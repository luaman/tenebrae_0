// gl_vidsdl.c -- sdl opengl video driver 


// dynamic library access
#include <dlfcn.h>

#include <SDL.h>
#include <SDL/SDL_opengl.h>

#include "quakedef.h"

viddef_t    vid;                // global video state
unsigned short  d_8to16table[256];

unsigned short	d_8to16table[256];
unsigned	d_8to24table[256];
unsigned char	d_15to8table[65536];
unsigned char d_8to8graytable[256];

// The original defaults
//#define    BASEWIDTH    320
//#define    BASEHEIGHT   200
// Much better for high resolution displays
#define    BASEWIDTH    (640)
#define    BASEHEIGHT   (480)

static qboolean mouse_avail;
static float   mouse_x, mouse_y;
static int mouse_oldbuttonstate = 0;

// No support for option menus
void (*vid_menudrawfn)(void) = NULL;
void (*vid_menukeyfn)(int key) = NULL;

static int scr_width, scr_height;

#define WARP_WIDTH              320
#define WARP_HEIGHT             200

static SDL_Surface *screen = NULL;

/*-----------------------------------------------------------------------*/

//int		texture_mode = GL_NEAREST;
//int		texture_mode = GL_NEAREST_MIPMAP_NEAREST;
//int		texture_mode = GL_NEAREST_MIPMAP_LINEAR;
int		texture_mode = GL_LINEAR;
//int		texture_mode = GL_LINEAR_MIPMAP_NEAREST;
//int		texture_mode = GL_LINEAR_MIPMAP_LINEAR;

int		texture_extension_number = 1;

float		gldepthmin, gldepthmax;

//cvar_t	gl_ztrick = {"gl_ztrick","1"}; PENTA: Removed 

const char *gl_vendor;
const char *gl_renderer;
const char *gl_version;
const char *gl_extensions;

void (*qglColorTableEXT) (int, int, int, int, int, const void*);
void (*qgl3DfxSetPaletteEXT) (GLuint *);

static float vid_gamma = 1.0;

qboolean is8bit = false;
qboolean isPermedia = false;
qboolean gl_mtexable = false;
qcardtype   gl_cardtype = GENERIC;
qboolean 	gl_var	= false; //PENTA: vertex array range is available
qboolean 	gl_texcomp = false; // JP: texture compression available

// <AWE> Two more extensions. Added already check for paletted texture to gl_vidnt.c.
//	 However any code for anisotropic texture filtering has still to be added to gl_vidnt.c.

qboolean 	gl_palettedtex = false; // <AWE> true for "gl_EXT_paletted_texture" [GL_Upload8_EXT].

qboolean	gl_texturefilteranisotropic = false; // <AWE> true for "GL_EXT_texture_filter_anisotropic".
GLfloat		gl_textureanisotropylevel = 2.0f; // <AWE> anistropic texture level [= 1.0f or max. value].//Penta?? Changed to 2.0 because 1.0 is just isotropic filtering
//cvar_t	gl_anisotropic = { "gl_anisotropic", "0", 1 }; // <AWE> On MacOSX X we use this var to store the state. 0 = off, 1 = on.

/*-----------------------------------------------------------------------*/



void    VID_SetPalette (unsigned char *palette)
{
    /*
    int i;
    SDL_Color colors[256];

    for ( i=0; i<256; ++i )
    {
        colors[i].r = *palette++;
        colors[i].g = *palette++;
        colors[i].b = *palette++;
    }
    SDL_SetColors(screen, colors, 0, 256);
    */
	byte	*pal;
	unsigned r,g,b;
	unsigned a;
	unsigned v;
	int     r1,g1,b1;
	int		j,k,l,m;
	unsigned short i;
	unsigned	*table;
	unsigned char *shade;
	FILE *f;
	char s[255];
	int dist, bestdist;

//
// 8 8 8 encoding
//
	pal = palette;
	table = d_8to24table;
	shade = d_8to8graytable;
	for (i=0 ; i<256 ; i++)
	{
		r = pal[0];
		g = pal[1];
		b = pal[2];
		pal += 3;
		
		//PENTA: fullbright colors
		//a = i;//(i >= 192) ? 255 : 0;
		a = 255;
		v = (a<<24) + (r<<0) + (g<<8) + (b<<16);
		*table++ = v;
		//PENA: Grayscale conversion for bump maps
		*shade++ = ((r+g+b)/3);
	}
	d_8to24table[255] &= 0xffffff;	// 255 is transparent

	for (i=0; i < (1<<15); i++) {
		/* Maps
		000000000000000
		000000000011111 = Red  = 0x1F
		000001111100000 = Blue = 0x03E0
		111110000000000 = Grn  = 0x7C00
		*/
		r = ((i & 0x1F) << 3)+4;
		g = ((i & 0x03E0) >> 2)+4;
		b = ((i & 0x7C00) >> 7)+4;
		pal = (unsigned char *)d_8to24table;
		for (v=0,k=0,bestdist=10000*10000; v<256; v++,pal+=4) {
			r1 = (int)r - (int)pal[0];
			g1 = (int)g - (int)pal[1];
			b1 = (int)b - (int)pal[2];
			dist = (r1*r1)+(g1*g1)+(b1*b1);
			if (dist < bestdist) {
				k=v;
				bestdist = dist;
			}
		}
		d_15to8table[i]=k;
	}
}

void    VID_ShiftPalette (unsigned char *palette)
{
  //VID_SetPalette(palette);
}

/*
<AWE> required for GL_Upload8_EXT ():
*/

void	CheckPalettedTexture (void)
{
    if (strstr (gl_extensions, "GL_EXT_paletted_texture"))
    {
        gl_palettedtex = true;
        Con_Printf ("Found GL_EXT_paletted_texture...\n");
    }
}

void CheckMultiTextureExtensions(void) 
{
	void *prjobj;

	if (strstr(gl_extensions, "GL_ARB_multitexture")) {
		qglActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)SDL_GL_GetProcAddress("glActiveTextureARB");
		qglClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC)SDL_GL_GetProcAddress("glClientActiveTextureARB");
		qglMultiTexCoord1fARB = (PFNGLMULTITEXCOORD1FARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord1fARB");
		qglMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord2fARB");
		qglMultiTexCoord2fvARB = (PFNGLMULTITEXCOORD2FVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord2fvARB");
		qglMultiTexCoord3fARB = (PFNGLMULTITEXCOORD3FARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord3fARB");
		qglMultiTexCoord3fvARB =(PFNGLMULTITEXCOORD3FVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord3fvARB");
		gl_mtexable = true;
	} else {
		Sys_Error ("No multitexturing found.\nProbably your 3d-card is not supported.\n");
	}	
}

/*
PENTA: If we don't have these extensions then we don't continue
(how would we ever draw bump maps is these simpel ext's aren't supported)
*/
void CheckDiffuseBumpMappingExtensions(void) 
{
	if (!strstr(gl_extensions, "GL_EXT_texture_env_combine") &&
		!strstr(gl_extensions, "GL_ARB_texture_env_combine") ) {
	  Sys_Error ("EXT_texture_env_combine not found.\nProbably your 3d-card is not supported.\n");
	}

	if (!strstr(gl_extensions, "GL_ARB_texture_env_dot3")) {
		Sys_Error ("ARB_texture_env_dot3 not found.\nProbably your 3d-card is not supported.\n");
	}

	if (!strstr(gl_extensions, "GL_ARB_texture_cube_map")) {
		Sys_Error ("ARB_texture_cube_map not found.\nProbably your 3d-card is not supported.\n");
	}

	//Just spit a warning user prob has gl-1.2 or something
	if (!strstr(gl_extensions, "GL_SGI_texture_edge_clamp") && 
		!strstr(gl_extensions, "GL_EXT_texture_edge_clamp")) {
		Con_Printf("Warning no edge_clamp extension found");
	}
}

/*
PENTA: if we don't have these we continue with less eficient specular
*/
void CheckSpecularBumpMappingExtensions(void) 
{
	if ( (strstr(gl_extensions, "GL_NV_register_combiners")) && (!COM_CheckParm ("-forcenonv")) ) {
		gl_cardtype = GEFORCE; // GEFORCE3 checked later
		/* Retrieve all NV_register_combiners routines. */
	  qglCombinerParameterfvNV =
		(PFNGLCOMBINERPARAMETERFVNVPROC)
		SDL_GL_GetProcAddress("glCombinerParameterfvNV");
	  qglCombinerParameterivNV =
		(PFNGLCOMBINERPARAMETERIVNVPROC)
		SDL_GL_GetProcAddress("glCombinerParameterivNV");
	  qglCombinerParameterfNV =
		(PFNGLCOMBINERPARAMETERFNVPROC)
		SDL_GL_GetProcAddress("glCombinerParameterfNV");
	  qglCombinerParameteriNV =
		(PFNGLCOMBINERPARAMETERINVPROC)
		SDL_GL_GetProcAddress("glCombinerParameteriNV");
	  qglCombinerInputNV =
		(PFNGLCOMBINERINPUTNVPROC)
		SDL_GL_GetProcAddress("glCombinerInputNV");
	  qglCombinerOutputNV =
		(PFNGLCOMBINEROUTPUTNVPROC)
		SDL_GL_GetProcAddress("glCombinerOutputNV");
	  qglFinalCombinerInputNV =
		(PFNGLFINALCOMBINERINPUTNVPROC)
		SDL_GL_GetProcAddress("glFinalCombinerInputNV");
	  qglGetCombinerInputParameterfvNV =
		(PFNGLGETCOMBINERINPUTPARAMETERFVNVPROC)
		SDL_GL_GetProcAddress("glGetCombinerInputParameterfvNV");
	  qglGetCombinerInputParameterivNV =
		(PFNGLGETCOMBINERINPUTPARAMETERIVNVPROC)
		SDL_GL_GetProcAddress("glGetCombinerInputParameterivNV");
	  qglGetCombinerOutputParameterfvNV =
		(PFNGLGETCOMBINEROUTPUTPARAMETERFVNVPROC)
		SDL_GL_GetProcAddress("glGetCombinerOutputParameterfvNV");
	  qglGetCombinerOutputParameterivNV =
		(PFNGLGETCOMBINEROUTPUTPARAMETERIVNVPROC)
		SDL_GL_GetProcAddress("glGetCombinerOutputParameterivNV");
	  qglGetFinalCombinerInputParameterfvNV =
		(PFNGLGETFINALCOMBINERINPUTPARAMETERFVNVPROC)
		SDL_GL_GetProcAddress("glGetFinalCombinerInputfvNV"); // <AWE> "glGetFinalCombinerInputfvNV" should be changed to: "glGetFinalCombinerInputParameterfvNV"
	  qglGetFinalCombinerInputParameterivNV =
		(PFNGLGETFINALCOMBINERINPUTPARAMETERIVNVPROC)
		SDL_GL_GetProcAddress("glGetFinalCombinerInputivNV"); // <AWE> "glGetFinalCombinerInputivNV" should be changed to: "glGetFinalCombinerInputParameterivNV"
	} else {
             //gl_nvcombiner = false;
	}
}

/*
PENTA: if we have these we draw optimized
*/
void CheckGeforce3Extensions(void) 
{
	int supportedTmu;
	glGetIntegerv(GL_MAX_ACTIVE_TEXTURES_ARB,&supportedTmu); 
	Con_Printf("%i texture units\n",supportedTmu);

	if (strstr(gl_extensions, "GL_EXT_texture3D")
		&& (supportedTmu >= 4)  && (!COM_CheckParm ("-forcegf2"))
		&& (gl_cardtype == GEFORCE)
		&& strstr(gl_extensions, "GL_NV_vertex_program1_1")
	        && strstr(gl_extensions, "GL_NV_vertex_array_range"))
	{
	        //Con_Printf("Using Geforce3 path.\n");
		gl_cardtype = GEFORCE3;

		qglTexImage3DEXT = (PFNGLTEXIMAGE3DEXT)SDL_GL_GetProcAddress("glTexImage3DEXT");

		//get vertex_program pointers
                qglAreProgramsResidentNV = (PFNGLAREPROGRAMSRESIDENTNVPROC)SDL_GL_GetProcAddress("glAreProgramsResidentNV");
                qglBindProgramNV = (PFNGLBINDPROGRAMNVPROC)SDL_GL_GetProcAddress("glBindProgramNV");
                qglDeleteProgramsNV = (PFNGLDELETEPROGRAMSNVPROC)SDL_GL_GetProcAddress("glDeleteProgramsNV");
                qglExecuteProgramNV = (PFNGLEXECUTEPROGRAMNVPROC)SDL_GL_GetProcAddress("glExecuteProgramNV");
                qglGenProgramsNV = (PFNGLGENPROGRAMSNVPROC)SDL_GL_GetProcAddress("glGenProgramsNV");
                qglGetProgramParameterdvNV = (PFNGLGETPROGRAMPARAMETERDVNVPROC)SDL_GL_GetProcAddress("glGetProgramParameterdvNV");
                qglGetProgramParameterfvNV = (PFNGLGETPROGRAMPARAMETERFVNVPROC)SDL_GL_GetProcAddress("glGetProgramParameterfvNV");
                qglGetProgramivNV = (PFNGLGETPROGRAMIVNVPROC)SDL_GL_GetProcAddress("glGetProgramivNV");
                qglGetProgramStringNV = (PFNGLGETPROGRAMSTRINGNVPROC)SDL_GL_GetProcAddress("glGetProgramStringNV");
                qglGetTrackMatrixivNV = (PFNGLGETTRACKMATRIXIVNVPROC)SDL_GL_GetProcAddress("glGetTrackMatrixivNV");
                qglGetVertexAttribdvNV = (PFNGLGETVERTEXATTRIBDVNVPROC)SDL_GL_GetProcAddress("glGetVertexAttribdvNV");
                qglGetVertexAttribfvNV = (PFNGLGETVERTEXATTRIBFVNVPROC)SDL_GL_GetProcAddress("glGetVertexAttribfvNV");
                qglGetVertexAttribivNV = (PFNGLGETVERTEXATTRIBIVNVPROC)SDL_GL_GetProcAddress("glGetVertexAttribivNV");
                qglGetVertexAttribPointervNV = (PFNGLGETVERTEXATTRIBPOINTERVNVPROC)SDL_GL_GetProcAddress("glGetVertexAttribPointervNV");
                qglGetVertexAttribPointervNV = (PFNGLGETVERTEXATTRIBPOINTERVNVPROC)SDL_GL_GetProcAddress("glGetVertexAttribPointerNV");
                qglIsProgramNV = (PFNGLISPROGRAMNVPROC)SDL_GL_GetProcAddress("glIsProgramNV");
                qglLoadProgramNV = (PFNGLLOADPROGRAMNVPROC)SDL_GL_GetProcAddress("glLoadProgramNV");
                qglProgramParameter4dNV = (PFNGLPROGRAMPARAMETER4DNVPROC)SDL_GL_GetProcAddress("glProgramParameter4dNV");
                qglProgramParameter4dvNV = (PFNGLPROGRAMPARAMETER4DVNVPROC)SDL_GL_GetProcAddress("glProgramParameter4dvNV");
                qglProgramParameter4fNV = (PFNGLPROGRAMPARAMETER4FNVPROC)SDL_GL_GetProcAddress("glProgramParameter4fNV");
                qglProgramParameter4fvNV = (PFNGLPROGRAMPARAMETER4FVNVPROC)SDL_GL_GetProcAddress("glProgramParameter4fvNV");
                qglProgramParameters4dvNV = (PFNGLPROGRAMPARAMETERS4DVNVPROC)SDL_GL_GetProcAddress("glProgramParameters4dvNV");
                qglProgramParameters4fvNV = (PFNGLPROGRAMPARAMETERS4FVNVPROC)SDL_GL_GetProcAddress("glProgramParameters4fvNV");
                qglRequestResidentProgramsNV = (PFNGLREQUESTRESIDENTPROGRAMSNVPROC)SDL_GL_GetProcAddress("glRequestResidentProgramsNV");
                qglTrackMatrixNV = (PFNGLTRACKMATRIXNVPROC)SDL_GL_GetProcAddress("glTrackMatrixNV");

		//default to trilinear filtering on gf3
		gl_filter_min = GL_LINEAR_MIPMAP_LINEAR;
		gl_filter_max = GL_LINEAR;

	} else {
               //gl_geforce3 = false;
	}
}

//program wide pointer to the allocated agp mem block
void *AGP_Buffer;

void CheckVertexArrayRange(void) 
{
/*
	if (strstr(gl_extensions, "GL_NV_vertex_array_range"))
	{
		//get VAR pointers
        qglFlushVertexArrayRangeNV = (PFNGLFLUSHVERTEXARRAYRANGENVPROC)SDL_GL_GetProcAddress("glFlushVertexArrayRangeNV");
        qglVertexArrayRangeNV = (PFNGLVERTEXARRAYRANGENVPROC)SDL_GL_GetProcAddress("glVertexArrayRangeNV");
        //wglAllocateMemoryNV = (PFNWGLALLOCATEMEMORYNVPROC)SDL_GL_GetProcAddress("wglAllocateMemoryNV");
        //wglFreeMemoryNV = (PFNWGLFREEMEMORYNVPROC)SDL_GL_GetProcAddress("wglFreeMemoryNV");

		if (wglAllocateMemoryNV != NULL) {
			AGP_Buffer = wglAllocateMemoryNV(AGP_BUFFER_SIZE, 0.0, 0.0, 0.85);
			if (AGP_Buffer) {
				Con_Printf("Using %ikb AGP mem.\n",AGP_BUFFER_SIZE/1024);
				gl_var = true;
			} else {
				Con_Printf("Failed to allocate %ikb AGP mem.\n",AGP_BUFFER_SIZE/1024);
				gl_var = false;
			}
		}
	} else {
*/
		gl_var = false;
//	}

}

/*
PA: if we have these we draw optimized
*/
void CheckRadeonExtensions(void) 
{
	int supportedTmu;
	glGetIntegerv(GL_MAX_ACTIVE_TEXTURES_ARB,&supportedTmu); 

	if (strstr(gl_extensions, "GL_EXT_texture3D")
		&& (supportedTmu >= 4)  && (!COM_CheckParm ("-forcegeneric"))
		&& strstr(gl_extensions, "GL_ATI_fragment_shader")
		&& strstr(gl_extensions, "GL_EXT_vertex_shader"))
	{
		Con_Printf("Using Radeon path.\n");
                gl_cardtype = RADEON;

		//get TEX3d poiters                    
		qglTexImage3DEXT = (PFNGLTEXIMAGE3DEXT)SDL_GL_GetProcAddress("glTexImage3DEXT");

		//default to trilinear filtering on Radeon
		gl_filter_min = GL_LINEAR_MIPMAP_LINEAR;
		gl_filter_max = GL_LINEAR;
                GL_CreateShadersRadeon();

	} 
}

void CheckParheliaExtensions(void) 
{
	int supportedTmu;
	glGetIntegerv(GL_MAX_ACTIVE_TEXTURES_ARB,&supportedTmu); 

	if (strstr(gl_extensions, "GL_EXT_texture3D")
		&& (supportedTmu >= 4)  && (!COM_CheckParm ("-forcegeneric"))
		&& strstr(gl_extensions, "GL_MTX_fragment_shader")
		&& strstr(gl_extensions, "GL_EXT_vertex_shader"))
	{
            gl_cardtype = PARHELIA;

	    //get TEX3d poiters                   wlgGetProcAddress
	    qglTexImage3DEXT = (PFNGLTEXIMAGE3DEXT)SDL_GL_GetProcAddress("glTexImage3DEXT");

	    //default to trilinear filtering on Parhelia
	    gl_filter_min = GL_LINEAR_MIPMAP_LINEAR;
	    gl_filter_max = GL_LINEAR;
            GL_CreateShadersParhelia();
	}
}

void CheckARBFragmentExtensions(void) 
{
	int supportedTmu;
	glGetIntegerv(GL_MAX_ACTIVE_TEXTURES_ARB,&supportedTmu); 

	if (strstr(gl_extensions, "GL_EXT_texture3D")
		&& (supportedTmu >= 6)  && (!COM_CheckParm ("-forcegeneric"))
                && (!COM_CheckParm ("-noarb"))
		&& strstr(gl_extensions, "GL_ARB_fragment_program")
		&& strstr(gl_extensions, "GL_ARB_vertex_program"))
	{
		gl_cardtype = ARB;

	    //get TEX3d poiters                   wlgGetProcAddress
	    qglTexImage3DEXT = (PFNGLTEXIMAGE3DEXT)SDL_GL_GetProcAddress("glTexImage3DEXT");

	    //default to trilinear filtering
	    gl_filter_min = GL_LINEAR_MIPMAP_LINEAR;
	    gl_filter_max = GL_LINEAR;
            GL_CreateShadersARB();
	}
}

void CheckAnisotropicExtension(void)
{
	if (strstr(gl_extensions, "GL_EXT_texture_filter_anisotropic") &&
				(COM_CheckParm ("-anisotropic"))) {
		Con_Printf("Anisotropic texture filter used\n");
		gl_texturefilteranisotropic = true;
	}
}

void CheckTextureCompressionExtension(void)
{
    if (strstr(gl_extensions, "GL_ARB_texture_compression") )
    {
        Con_Printf("Texture compression available\n");
        gl_texcomp = true;
    }
}

/*
===============
GL_Init
===============
*/
void GL_Init (void)
{
	int supportedTmu;

	gl_vendor = glGetString (GL_VENDOR);
	Con_Printf ("GL_VENDOR: %s\n", gl_vendor);
	gl_renderer = glGetString (GL_RENDERER);
	Con_Printf ("GL_RENDERER: %s\n", gl_renderer);

	gl_version = glGetString (GL_VERSION);
	Con_Printf ("GL_VERSION: %s\n", gl_version);
	gl_extensions = glGetString (GL_EXTENSIONS);
	Con_Printf ("GL_EXTENSIONS: %s\n", gl_extensions);

	Con_Printf ("%s %s\n", gl_renderer, gl_version);
        /*
    if (strnicmp(gl_renderer,"PowerVR",7)==0)
         fullsbardraw = true;

    if (strnicmp(gl_renderer,"Permedia",8)==0)
         isPermedia = true;
        */
        Con_Printf("Checking paletted texture\n");
	CheckPalettedTexture ();

        Con_Printf("Checking multitexture\n");
	CheckMultiTextureExtensions ();

        Con_Printf("Checking diffuse bumpmap extensions\n");
	CheckDiffuseBumpMappingExtensions(); 

        Con_Printf("Checking ARB extensions\n");
	CheckARBFragmentExtensions();
	if ( gl_cardtype != ARB )
	{
                Con_Printf("Checking GeForce 1/2/4-MX\n");
	CheckSpecularBumpMappingExtensions(); 
                Con_Printf("Checking GeForce 3/4\n");
	CheckGeforce3Extensions();
                Con_Printf("Checking Radeon 8500+\n");
	CheckRadeonExtensions();
                Con_Printf("Checking Parhelia\n");
		CheckParheliaExtensions();
	}
        Con_Printf("Checking VAR\n");
	CheckVertexArrayRange();
        Con_Printf("Checking AF\n");
	CheckAnisotropicExtension();
        Con_Printf("Checking TC\n");
	CheckTextureCompressionExtension();

	switch(gl_cardtype)
	{
		case GENERIC:
			Con_Printf("Using generic path.\n");
			break;
		case GEFORCE:
			Con_Printf("Using GeForce 1/2/4-MX path\n");
			break;
		case GEFORCE3:
			Con_Printf("Using GeForce 3/4 path\n");
			break;
		case RADEON:
			Con_Printf("Using Radeon path.\n");
			break;
		case PARHELIA:
			Con_Printf("Using Parhelia path.\n");
			break;
		case ARB:
			Con_Printf("Using ARB_fragment_program path.\n");
			break;
	}

	glGetIntegerv(GL_MAX_ACTIVE_TEXTURES_ARB,&supportedTmu); 
	Con_Printf("%i texture units\n",supportedTmu);

	//PENTA: enable mlook by default, people kept mailing me about how to do mlook
	Cbuf_AddText ("+mlook");

	glClearColor (0.5,0.5,0.5,0.5);

	glCullFace(GL_FRONT);
	glEnable(GL_TEXTURE_2D);

	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.666);

	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	glShadeModel (GL_FLAT);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

/*
=================
GL_BeginRendering

=================
*/
void GL_BeginRendering (int *x, int *y, int *width, int *height)
{
	extern cvar_t gl_clear;

	*x = *y = 0;
	*width = scr_width;
	*height = scr_height;

//    if (!wglMakeCurrent( maindc, baseRC ))
//		Sys_Error ("wglMakeCurrent failed");

//	glViewport (*x, *y, *width, *height);
}


void GL_EndRendering (void)
{
        glFlush ();
        SDL_GL_SwapBuffers ();
}

qboolean VID_Is8bit(void)
{
	return is8bit;
}

void VID_Init8bitPalette(void) 
{
	// Check for 8bit Extensions and initialize them.
	int i;
	void *prjobj;

	if ((prjobj = dlopen(NULL, RTLD_LAZY)) == NULL) {
		Con_Printf("Unable to open symbol list for main program.\n");
		return;
	}

	if (strstr(gl_extensions, "3DFX_set_global_palette") &&
		(qgl3DfxSetPaletteEXT = dlsym(prjobj, "gl3DfxSetPaletteEXT")) != NULL) {
		GLubyte table[256][4];
		char *oldpal;

		Con_SafePrintf("8-bit GL extensions enabled.\n");
		glEnable( GL_SHARED_TEXTURE_PALETTE_EXT );
		oldpal = (char *) d_8to24table; //d_8to24table3dfx;
		for (i=0;i<256;i++) {
			table[i][2] = *oldpal++;
			table[i][1] = *oldpal++;
			table[i][0] = *oldpal++;
			table[i][3] = 255;
			oldpal++;
		}
		qgl3DfxSetPaletteEXT((GLuint *)table);
		is8bit = true;

	} else if (strstr(gl_extensions, "GL_EXT_shared_texture_palette") &&
		(qglColorTableEXT = dlsym(prjobj, "glColorTableEXT")) != NULL) {
		char thePalette[256*3];
		char *oldPalette, *newPalette;

		Con_SafePrintf("8-bit GL extensions enabled.\n");
		glEnable( GL_SHARED_TEXTURE_PALETTE_EXT );
		oldPalette = (char *) d_8to24table; //d_8to24table3dfx;
		newPalette = thePalette;
		for (i=0;i<256;i++) {
			*newPalette++ = *oldPalette++;
			*newPalette++ = *oldPalette++;
			*newPalette++ = *oldPalette++;
			oldPalette++;
		}
		qglColorTableEXT(GL_SHARED_TEXTURE_PALETTE_EXT, GL_RGB, 256, GL_RGB, GL_UNSIGNED_BYTE, (void *) thePalette);
		is8bit = true;
	}
	
	dlclose(prjobj);
}

static void Check_Gamma (unsigned char *pal)
{
	float	f, inf;
	unsigned char	palette[768];
	int		i;

	if ((i = COM_CheckParm("-gamma")) == 0) {
		if ((gl_renderer && strstr(gl_renderer, "Voodoo")) ||
			(gl_vendor && strstr(gl_vendor, "3Dfx")))
			vid_gamma = 1;
		else
		        vid_gamma = 0.6; // default to 0.7 on non-3dfx hardware
		                         // PENTA: lowered to make things a little brighter
	} else
		vid_gamma = Q_atof(com_argv[i+1]);

	for (i=0 ; i<768 ; i++)
	{
		f = pow ( (pal[i]+1)/256.0 , vid_gamma );
		inf = f*255 + 0.5;
		if (inf < 0)
			inf = 0;
		if (inf > 255)
			inf = 255;
		palette[i] = inf;
	}

	memcpy (pal, palette, sizeof(palette));
}


void    VID_Init (unsigned char *palette)
{
    int pnum, chunk;
    byte *cache;
    int cachesize;
    Uint8 video_bpp;
    Uint16 video_w, video_h;
    Uint32 flags;
    const SDL_VideoInfo* info = NULL;
    char	gldir[MAX_OSPATH];

    // Load the SDL library
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_CDROM) < 0)
        Sys_Error("VID: Couldn't load SDL: %s", SDL_GetError());

    // Get Video info
    info=SDL_GetVideoInfo ();
    if( !info ) {
      Sys_Error ("Video query failed: %s\n", SDL_GetError ());
    }
    video_bpp = 32; // 32 bpp mandatory

    // Define some necessary OpenGL features  
    SDL_GL_SetAttribute (SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute (SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute (SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute (SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute (SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute (SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, 1);

   // Set up display mode (width and height)
    vid.width = BASEWIDTH;
    vid.height = BASEHEIGHT;
    vid.maxwarpwidth = WARP_WIDTH;
    vid.maxwarpheight = WARP_HEIGHT;
    vid.colormap = host_colormap;
    //vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));

    if ((pnum=COM_CheckParm("-winsize")))
    {
        if (pnum >= com_argc-2)
            Sys_Error("VID: -winsize <width> <height>\n");
        vid.width = Q_atoi(com_argv[pnum+1]);
        vid.height = Q_atoi(com_argv[pnum+2]);
        if (!vid.width || !vid.height)
            Sys_Error("VID: Bad window width/height\n");
    }
    if ((pnum=COM_CheckParm("-width")))
    {
        if (pnum >= com_argc-1)
            Sys_Error("VID: -width <width>\n");
        vid.width = Q_atoi(com_argv[pnum+1]);
        if (!vid.width)
            Sys_Error("VID: Bad window width\n");
    }
    if ((pnum=COM_CheckParm("-height")))
    {
        if (pnum >= com_argc-1)
            Sys_Error("VID: -height <height>\n");
        vid.height = Q_atoi(com_argv[pnum+1]);
        if (!vid.height)
            Sys_Error("VID: Bad window height\n");
    }
    // Set video width, height and flags

    flags = SDL_OPENGL;
    if ( !COM_CheckParm ("-window") )
        flags |= SDL_FULLSCREEN;

    // Initialize display 
    if (!(screen = SDL_SetVideoMode(vid.width, vid.height, video_bpp, flags)))
        Sys_Error("VID: Couldn't set video mode: %s\n", SDL_GetError());

    scr_width = vid.width;
    scr_height = vid.height;

    vid.conheight = vid.height;
    vid.conwidth = vid.width;

    /*
    if (vid.conheight > vid.height)
      vid.conheight = vid.height;
    if (vid.conwidth > vid.width)
      vid.conwidth = vid.width;
    */
    
    vid.aspect = ((float)vid.height / (float)vid.width) * (320.0 / 240.0);
    vid.numpages = 2;

    // Initialize OpenGL
    GL_Init ();
    
    sprintf (gldir, "%s/glquake", com_gamedir);
    Sys_mkdir (gldir);
 
    VID_SetPalette(palette);
    SDL_WM_SetCaption("tenebrae-sdl","tenebrae-sdl");

    // Check for 3DFX Extensions and initialize them.
    VID_Init8bitPalette();
    
    Con_SafePrintf ("Video mode %dx%d initialized.\n", vid.width, vid.height);
    
    vid.recalc_refdef = 1;				// force a surface cache flush    
    
    // initialize the mouse
    SDL_ShowCursor(0);
}

void    VID_Shutdown (void)
{
    SDL_Quit();
}

/*
================
D_BeginDirectRect
================
*/
void D_BeginDirectRect (int x, int y, byte *pbitmap, int width, int height)
{
  /*
    Uint8 *offset;


    if (!screen) return;
    if ( x < 0 ) x = screen->w+x-1;
    offset = (Uint8 *)screen->pixels + y*screen->pitch + x;
    while ( height-- )
    {
        memcpy(offset, pbitmap, width);
        offset += screen->pitch;
        pbitmap += width;
    }
  */
}


/*
================
D_EndDirectRect
================
*/
void D_EndDirectRect (int x, int y, int width, int height)
{
  /*
    if (!screen) return;
    if (x < 0) x = screen->w+x-1;
    SDL_UpdateRect(screen, x, y, width, height);
  */
}


/*
================
Sys_SendKeyEvents
================
*/

void Sys_SendKeyEvents(void)
{
    SDL_Event event;
    int sym, state;
     int modstate;

    while (SDL_PollEvent(&event))
    {
        switch (event.type) {

            case SDL_KEYDOWN:
            case SDL_KEYUP:
                sym = event.key.keysym.sym;
                state = event.key.state;
                modstate = SDL_GetModState();
                switch(sym)
                {
                   case SDLK_DELETE: sym = K_DEL; break;
                   case SDLK_BACKSPACE: sym = K_BACKSPACE; break;
                   case SDLK_F1: sym = K_F1; break;
                   case SDLK_F2: sym = K_F2; break;
                   case SDLK_F3: sym = K_F3; break;
                   case SDLK_F4: sym = K_F4; break;
                   case SDLK_F5: sym = K_F5; break;
                   case SDLK_F6: sym = K_F6; break;
                   case SDLK_F7: sym = K_F7; break;
                   case SDLK_F8: sym = K_F8; break;
                   case SDLK_F9: sym = K_F9; break;
                   case SDLK_F10: sym = K_F10; break;
                   case SDLK_F11: sym = K_F11; break;
                   case SDLK_F12: sym = K_F12; break;
                   case SDLK_BREAK:
                   case SDLK_PAUSE: sym = K_PAUSE; break;
                   case SDLK_UP: sym = K_UPARROW; break;
                   case SDLK_DOWN: sym = K_DOWNARROW; break;
                   case SDLK_RIGHT: sym = K_RIGHTARROW; break;
                   case SDLK_LEFT: sym = K_LEFTARROW; break;
                   case SDLK_INSERT: sym = K_INS; break;
                   case SDLK_HOME: sym = K_HOME; break;
                   case SDLK_END: sym = K_END; break;
                   case SDLK_PAGEUP: sym = K_PGUP; break;
                   case SDLK_PAGEDOWN: sym = K_PGDN; break;
                   case SDLK_RSHIFT:
                   case SDLK_LSHIFT: sym = K_SHIFT; break;
                   case SDLK_RCTRL:
                   case SDLK_LCTRL: sym = K_CTRL; break;
                   case SDLK_RALT:
                   case SDLK_LALT: sym = K_ALT; break;
                   case SDLK_KP0: 
                       if(modstate & KMOD_NUM) sym = K_INS; 
                       else sym = SDLK_0;
                       break;
                   case SDLK_KP1:
                       if(modstate & KMOD_NUM) sym = K_END;
                       else sym = SDLK_1;
                       break;
                   case SDLK_KP2:
                       if(modstate & KMOD_NUM) sym = K_DOWNARROW;
                       else sym = SDLK_2;
                       break;
                   case SDLK_KP3:
                       if(modstate & KMOD_NUM) sym = K_PGDN;
                       else sym = SDLK_3;
                       break;
                   case SDLK_KP4:
                       if(modstate & KMOD_NUM) sym = K_LEFTARROW;
                       else sym = SDLK_4;
                       break;
                   case SDLK_KP5: sym = SDLK_5; break;
                   case SDLK_KP6:
                       if(modstate & KMOD_NUM) sym = K_RIGHTARROW;
                       else sym = SDLK_6;
                       break;
                   case SDLK_KP7:
                       if(modstate & KMOD_NUM) sym = K_HOME;
                       else sym = SDLK_7;
                       break;
                   case SDLK_KP8:
                       if(modstate & KMOD_NUM) sym = K_UPARROW;
                       else sym = SDLK_8;
                       break;
                   case SDLK_KP9:
                       if(modstate & KMOD_NUM) sym = K_PGUP;
                       else sym = SDLK_9;
                       break;
                   case SDLK_KP_PERIOD:
                       if(modstate & KMOD_NUM) sym = K_DEL;
                       else sym = SDLK_PERIOD;
                       break;
                   case SDLK_KP_DIVIDE: sym = SDLK_SLASH; break;
                   case SDLK_KP_MULTIPLY: sym = SDLK_ASTERISK; break;
                   case SDLK_KP_MINUS: sym = SDLK_MINUS; break;
                   case SDLK_KP_PLUS: sym = SDLK_PLUS; break;
                   case SDLK_KP_ENTER: sym = SDLK_RETURN; break;
                   case SDLK_KP_EQUALS: sym = SDLK_EQUALS; break;
                }
                // If we're not directly handled and still above 255
                // just force it to 0
                if(sym > 255) sym = 0;
                Key_Event(sym, state);
                break;

            case SDL_MOUSEMOTION:
                if ( (event.motion.x != (vid.width/2)) ||
                     (event.motion.y != (vid.height/2)) ) {
                    mouse_x = event.motion.xrel*10;
                    mouse_y = event.motion.yrel*10;
                    if ( (event.motion.x < ((vid.width/2)-(vid.width/4))) ||
                         (event.motion.x > ((vid.width/2)+(vid.width/4))) ||
                         (event.motion.y < ((vid.height/2)-(vid.height/4))) ||
                         (event.motion.y > ((vid.height/2)+(vid.height/4))) ) {
                        SDL_WarpMouse(vid.width/2, vid.height/2);
                    }
                }
                break;

            case SDL_QUIT:
                CL_Disconnect ();
                Host_ShutdownServer(false);        
                Sys_Quit ();
                break;
            default:
                break;
        }
    }
}

void IN_Init (void)
{
    if ( COM_CheckParm ("-nomouse") )
        return;
    mouse_x = mouse_y = 0.0;
    mouse_avail = 1;
}

void IN_Shutdown (void)
{
    mouse_avail = 0;
}

void IN_Commands (void)
{
    int i;
    int mouse_buttonstate;
   
    if (!mouse_avail) return;
   
    i = SDL_GetMouseState(NULL, NULL);
    /* Quake swaps the second and third buttons */
    mouse_buttonstate = (i & ~0x06) | ((i & 0x02)<<1) | ((i & 0x04)>>1);
    for (i=0 ; i<3 ; i++) {
        if ( (mouse_buttonstate & (1<<i)) && !(mouse_oldbuttonstate & (1<<i)) )
            Key_Event (K_MOUSE1 + i, true);

        if ( !(mouse_buttonstate & (1<<i)) && (mouse_oldbuttonstate & (1<<i)) )
            Key_Event (K_MOUSE1 + i, false);
    }
    mouse_oldbuttonstate = mouse_buttonstate;
}

void IN_Move (usercmd_t *cmd)
{
    if (!mouse_avail)
        return;
   
    mouse_x *= sensitivity.value;
    mouse_y *= sensitivity.value;
   
    if ( (in_strafe.state & 1) || (lookstrafe.value && (in_mlook.state & 1) ))
        cmd->sidemove += m_side.value * mouse_x;
    else
        cl.viewangles[YAW] -= m_yaw.value * mouse_x;
    if (in_mlook.state & 1)
        V_StopPitchDrift ();
   
    if ( (in_mlook.state & 1) && !(in_strafe.state & 1)) {
        cl.viewangles[PITCH] += m_pitch.value * mouse_y;
        if (cl.viewangles[PITCH] > 80)
            cl.viewangles[PITCH] = 80;
        if (cl.viewangles[PITCH] < -70)
            cl.viewangles[PITCH] = -70;
    } else {
        if ((in_strafe.state & 1) && noclip_anglehack)
            cmd->upmove -= m_forward.value * mouse_y;
        else
            cmd->forwardmove -= m_forward.value * mouse_y;
    }
    mouse_x = mouse_y = 0.0;
}

/*
================
Sys_ConsoleInput
================
*/
char *Sys_ConsoleInput (void)
{
    return 0;
}


