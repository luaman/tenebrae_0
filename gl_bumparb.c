/*
Copyright (C) 2001-2002 Charles Hollemeersch
ARB_fragment_progam version (C) 2002-2003 Jarno Paananen

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

PENTA: the whole file is freakin penta...

Same as gl_bumpmap.c but for ARB vertex and fragment program extensions
These routines require 4 texture units, vertex shader and pixel shader

All lights require 1 pass:
1 diffuse + specular with optional light filter

*/

#include "quakedef.h"

#ifndef GL_ATI_pn_triangles
// PN_triangles_ATI
#define GL_PN_TRIANGLES_ATI                       0x87F0
#define GL_MAX_PN_TRIANGLES_TESSELATION_LEVEL_ATI 0x87F1
#define GL_PN_TRIANGLES_POINT_MODE_ATI            0x87F2
#define GL_PN_TRIANGLES_NORMAL_MODE_ATI           0x87F3
#define GL_PN_TRIANGLES_TESSELATION_LEVEL_ATI     0x87F4
#define GL_PN_TRIANGLES_POINT_MODE_LINEAR_ATI     0x87F5
#define GL_PN_TRIANGLES_POINT_MODE_CUBIC_ATI      0x87F6
#define GL_PN_TRIANGLES_NORMAL_MODE_LINEAR_ATI    0x87F7
#define GL_PN_TRIANGLES_NORMAL_MODE_QUADRATIC_ATI 0x87F8

typedef void (APIENTRY *PFNGLPNTRIANGLESIATIPROC)(GLenum pname, GLint param);
typedef void (APIENTRY *PFNGLPNTRIANGLESFATIPROC)(GLenum pname, GLfloat param);
#endif

// actually in gl_bumpradeon (duh...)
extern PFNGLPNTRIANGLESIATIPROC qglPNTrianglesiATI;
extern PFNGLPNTRIANGLESFATIPROC qglPNTrianglesfATI;

#include "gl_arbprograms.h"
 glVertexAttrib1sARBPROC qglVertexAttrib1sARB = NULL;
 glVertexAttrib1fARBPROC qglVertexAttrib1fARB = NULL;
 glVertexAttrib1dARBPROC qglVertexAttrib1dARB = NULL;
 glVertexAttrib2sARBPROC qglVertexAttrib2sARB = NULL;
 glVertexAttrib2fARBPROC qglVertexAttrib2fARB = NULL;
 glVertexAttrib2dARBPROC qglVertexAttrib2dARB = NULL;
 glVertexAttrib3sARBPROC qglVertexAttrib3sARB = NULL;
 glVertexAttrib3fARBPROC qglVertexAttrib3fARB = NULL;
 glVertexAttrib3dARBPROC qglVertexAttrib3dARB = NULL;
 glVertexAttrib4sARBPROC qglVertexAttrib4sARB = NULL;
 glVertexAttrib4fARBPROC qglVertexAttrib4fARB = NULL;
 glVertexAttrib4dARBPROC qglVertexAttrib4dARB = NULL;
 glVertexAttrib4NubARBPROC qglVertexAttrib4NubARB = NULL;
 glVertexAttrib1svARBPROC qglVertexAttrib1svARB = NULL;
 glVertexAttrib1fvARBPROC qglVertexAttrib1fvARB = NULL;
 glVertexAttrib1dvARBPROC qglVertexAttrib1dvARB = NULL;
 glVertexAttrib2svARBPROC qglVertexAttrib2svARB = NULL;
 glVertexAttrib2fvARBPROC qglVertexAttrib2fvARB = NULL;
 glVertexAttrib2dvARBPROC qglVertexAttrib2dvARB = NULL;
 glVertexAttrib3svARBPROC qglVertexAttrib3svARB = NULL;
 glVertexAttrib3fvARBPROC qglVertexAttrib3fvARB = NULL;
 glVertexAttrib3dvARBPROC qglVertexAttrib3dvARB = NULL;
 glVertexAttrib4bvARBPROC qglVertexAttrib4bvARB = NULL;
 glVertexAttrib4svARBPROC qglVertexAttrib4svARB = NULL;
 glVertexAttrib4ivARBPROC qglVertexAttrib4ivARB = NULL;
 glVertexAttrib4ubvARBPROC qglVertexAttrib4ubvARB = NULL;
 glVertexAttrib4usvARBPROC qglVertexAttrib4usvARB = NULL;
 glVertexAttrib4uivARBPROC qglVertexAttrib4uivARB = NULL;
 glVertexAttrib4fvARBPROC qglVertexAttrib4fvARB = NULL;
 glVertexAttrib4dvARBPROC qglVertexAttrib4dvARB = NULL;
 glVertexAttrib4NbvARBPROC qglVertexAttrib4NbvARB = NULL;
 glVertexAttrib4NsvARBPROC qglVertexAttrib4NsvARB = NULL;
 glVertexAttrib4NivARBPROC qglVertexAttrib4NivARB = NULL;
 glVertexAttrib4NubvARBPROC qglVertexAttrib4NubvARB = NULL;
 glVertexAttrib4NusvARBPROC qglVertexAttrib4NusvARB = NULL;
 glVertexAttrib4NuivARBPROC qglVertexAttrib4NuivARB = NULL;
 glVertexAttribPointerARBPROC qglVertexAttribPointerARB = NULL;
 glEnableVertexAttribArrayARBPROC qglEnableVertexAttribArrayARB = NULL;
 glDisableVertexAttribArrayARBPROC qglDisableVertexAttribArrayARB = NULL;
 glProgramStringARBPROC qglProgramStringARB = NULL;
 glBindProgramARBPROC qglBindProgramARB = NULL;
 glDeleteProgramsARBPROC qglDeleteProgramsARB = NULL;
 glGenProgramsARBPROC qglGenProgramsARB = NULL;
 glProgramEnvParameter4dARBPROC qglProgramEnvParameter4dARB = NULL;
 glProgramEnvParameter4dvARBPROC qglProgramEnvParameter4dvARB = NULL;
 glProgramEnvParameter4fARBPROC qglProgramEnvParameter4fARB = NULL;
 glProgramEnvParameter4fvARBPROC qglProgramEnvParameter4fvARB = NULL;
 glProgramLocalParameter4dARBPROC qglProgramLocalParameter4dARB = NULL;
 glProgramLocalParameter4dvARBPROC qglProgramLocalParameter4dvARB = NULL;
 glProgramLocalParameter4fARBPROC qglProgramLocalParameter4fARB = NULL;
 glProgramLocalParameter4fvARBPROC qglProgramLocalParameter4fvARB = NULL;
 glGetProgramEnvParameterdvARBPROC qglGetProgramEnvParameterdvARB = NULL;
 glGetProgramEnvParameterfvARBPROC qglGetProgramEnvParameterfvARB = NULL;
 glGetProgramLocalParameterdvARBPROC qglGetProgramLocalParameterdvARB = NULL;
 glGetProgramLocalParameterfvARBPROC qglGetProgramLocalParameterfvARB = NULL;
 glGetProgramivARBPROC qglGetProgramivARB = NULL;
 glGetProgramStringARBPROC qglGetProgramStringARB = NULL;
 glGetVertexAttribdvARBPROC qglGetVertexAttribdvARB = NULL;
 glGetVertexAttribfvARBPROC qglGetVertexAttribfvARB = NULL;
 glGetVertexAttribivARBPROC qglGetVertexAttribivARB = NULL;
 glGetVertexAttribPointervARBPROC qglGetVertexAttribPointervARB = NULL;
 glIsProgramARBPROC qglIsProgramARB = NULL;

static GLuint gl_programs[16];
static int numGlPrograms = 0;

#define ARBDEBUG

#if defined(ARBDEBUG) && defined(_WIN32)
static void checkerror()
{
    GLuint error = glGetError();
    if ( error != GL_NO_ERROR )
    {
        int line;
        const char* err;
        
        glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &line);
        err = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
        _asm { int 3 };
    }
}
#else

#define checkerror() do { } while(0)

#endif

static GLuint LoadProgram( int target, const char *fileName ) {
	int identifier;
	byte *buffer = COM_LoadTempFile (fileName);
	
	if (!buffer) {
		Con_Printf("Can't load gl program from: %s\n",fileName);
		return 0;
	}
	
	qglGenProgramsARB(1, &identifier);
	qglBindProgramARB( target, identifier );
	checkerror();
	qglProgramStringARB( target, GL_PROGRAM_FORMAT_ASCII_ARB, strlen(buffer),  buffer );
	checkerror();

	return identifier;
}

void GL_CreateShadersARB()
{
#if !defined(__APPLE__) && !defined (MACOSX)
    SAFE_GET_PROC(qglVertexAttrib1sARB,glVertexAttrib1sARBPROC,"glVertexAttrib1sARB");
    SAFE_GET_PROC(qglVertexAttrib1fARB,glVertexAttrib1fARBPROC,"glVertexAttrib1fARB");
    SAFE_GET_PROC(qglVertexAttrib1dARB,glVertexAttrib1dARBPROC,"glVertexAttrib1dARB");
    SAFE_GET_PROC(qglVertexAttrib2sARB,glVertexAttrib2sARBPROC,"glVertexAttrib2sARB");
    SAFE_GET_PROC(qglVertexAttrib2fARB,glVertexAttrib2fARBPROC,"glVertexAttrib2fARB");
    SAFE_GET_PROC(qglVertexAttrib2dARB,glVertexAttrib2dARBPROC,"glVertexAttrib2dARB");
    SAFE_GET_PROC(qglVertexAttrib3sARB,glVertexAttrib3sARBPROC,"glVertexAttrib3sARB");
    SAFE_GET_PROC(qglVertexAttrib3fARB,glVertexAttrib3fARBPROC,"glVertexAttrib3fARB");
    SAFE_GET_PROC(qglVertexAttrib3dARB,glVertexAttrib3dARBPROC,"glVertexAttrib3dARB");
    SAFE_GET_PROC(qglVertexAttrib4sARB,glVertexAttrib4sARBPROC,"glVertexAttrib4sARB");
    SAFE_GET_PROC(qglVertexAttrib4fARB,glVertexAttrib4fARBPROC,"glVertexAttrib4fARB");
    SAFE_GET_PROC(qglVertexAttrib4dARB,glVertexAttrib4dARBPROC,"glVertexAttrib4dARB");
    SAFE_GET_PROC(qglVertexAttrib4NubARB,glVertexAttrib4NubARBPROC,"glVertexAttrib4NubARB");
    SAFE_GET_PROC(qglVertexAttrib1svARB,glVertexAttrib1svARBPROC,"glVertexAttrib1svARB");
    SAFE_GET_PROC(qglVertexAttrib1fvARB,glVertexAttrib1fvARBPROC,"glVertexAttrib1fvARB");
    SAFE_GET_PROC(qglVertexAttrib1dvARB,glVertexAttrib1dvARBPROC,"glVertexAttrib1dvARB");
    SAFE_GET_PROC(qglVertexAttrib2svARB,glVertexAttrib2svARBPROC,"glVertexAttrib2svARB");
    SAFE_GET_PROC(qglVertexAttrib2fvARB,glVertexAttrib2fvARBPROC,"glVertexAttrib2fvARB");
    SAFE_GET_PROC(qglVertexAttrib2dvARB,glVertexAttrib2dvARBPROC,"glVertexAttrib2dvARB");
    SAFE_GET_PROC(qglVertexAttrib3svARB,glVertexAttrib3svARBPROC,"glVertexAttrib3svARB");
    SAFE_GET_PROC(qglVertexAttrib3fvARB,glVertexAttrib3fvARBPROC,"glVertexAttrib3fvARB");
    SAFE_GET_PROC(qglVertexAttrib3dvARB,glVertexAttrib3dvARBPROC,"glVertexAttrib3dvARB");
    SAFE_GET_PROC(qglVertexAttrib4bvARB,glVertexAttrib4bvARBPROC,"glVertexAttrib4bvARB");
    SAFE_GET_PROC(qglVertexAttrib4svARB,glVertexAttrib4svARBPROC,"glVertexAttrib4svARB");
    SAFE_GET_PROC(qglVertexAttrib4ivARB,glVertexAttrib4ivARBPROC,"glVertexAttrib4ivARB");
    SAFE_GET_PROC(qglVertexAttrib4ubvARB,glVertexAttrib4ubvARBPROC,"glVertexAttrib4ubvARB");
    SAFE_GET_PROC(qglVertexAttrib4usvARB,glVertexAttrib4usvARBPROC,"glVertexAttrib4usvARB");
    SAFE_GET_PROC(qglVertexAttrib4uivARB,glVertexAttrib4uivARBPROC,"glVertexAttrib4uivARB");
    SAFE_GET_PROC(qglVertexAttrib4fvARB,glVertexAttrib4fvARBPROC,"glVertexAttrib4fvARB");
    SAFE_GET_PROC(qglVertexAttrib4dvARB,glVertexAttrib4dvARBPROC,"glVertexAttrib4dvARB");
    SAFE_GET_PROC(qglVertexAttrib4NbvARB,glVertexAttrib4NbvARBPROC,"glVertexAttrib4NbvARB");
    SAFE_GET_PROC(qglVertexAttrib4NsvARB,glVertexAttrib4NsvARBPROC,"glVertexAttrib4NsvARB");
    SAFE_GET_PROC(qglVertexAttrib4NivARB,glVertexAttrib4NivARBPROC,"glVertexAttrib4NivARB");
    SAFE_GET_PROC(qglVertexAttrib4NubvARB,glVertexAttrib4NubvARBPROC,"glVertexAttrib4NubvARB");
    SAFE_GET_PROC(qglVertexAttrib4NusvARB,glVertexAttrib4NusvARBPROC,"glVertexAttrib4NusvARB");
    SAFE_GET_PROC(qglVertexAttrib4NuivARB,glVertexAttrib4NuivARBPROC,"glVertexAttrib4NuivARB");
    SAFE_GET_PROC(qglVertexAttribPointerARB,glVertexAttribPointerARBPROC,"glVertexAttribPointerARB");
    SAFE_GET_PROC(qglEnableVertexAttribArrayARB,glEnableVertexAttribArrayARBPROC,"glEnableVertexAttribArrayARB");
    SAFE_GET_PROC(qglDisableVertexAttribArrayARB,glDisableVertexAttribArrayARBPROC,"glDisableVertexAttribArrayARB");
    SAFE_GET_PROC(qglProgramStringARB,glProgramStringARBPROC,"glProgramStringARB");
    SAFE_GET_PROC(qglBindProgramARB,glBindProgramARBPROC,"glBindProgramARB");
    SAFE_GET_PROC(qglDeleteProgramsARB,glDeleteProgramsARBPROC,"glDeleteProgramsARB");
    SAFE_GET_PROC(qglGenProgramsARB,glGenProgramsARBPROC,"glGenProgramsARB");
    SAFE_GET_PROC(qglProgramEnvParameter4dARB,glProgramEnvParameter4dARBPROC,"glProgramEnvParameter4dARB");
    SAFE_GET_PROC(qglProgramEnvParameter4dvARB,glProgramEnvParameter4dvARBPROC,"glProgramEnvParameter4dvARB");
    SAFE_GET_PROC(qglProgramEnvParameter4fARB,glProgramEnvParameter4fARBPROC,"glProgramEnvParameter4fARB");
    SAFE_GET_PROC(qglProgramEnvParameter4fvARB,glProgramEnvParameter4fvARBPROC,"glProgramEnvParameter4fvARB");
    SAFE_GET_PROC(qglProgramLocalParameter4dARB,glProgramLocalParameter4dARBPROC,"glProgramLocalParameter4dARB");
    SAFE_GET_PROC(qglProgramLocalParameter4dvARB,glProgramLocalParameter4dvARBPROC,"glProgramLocalParameter4dvARB");
    SAFE_GET_PROC(qglProgramLocalParameter4fARB,glProgramLocalParameter4fARBPROC,"glProgramLocalParameter4fARB");
    SAFE_GET_PROC(qglProgramLocalParameter4fvARB,glProgramLocalParameter4fvARBPROC,"glProgramLocalParameter4fvARB");
    SAFE_GET_PROC(qglGetProgramEnvParameterdvARB,glGetProgramEnvParameterdvARBPROC,"glGetProgramEnvParameterdvARB");
    SAFE_GET_PROC(qglGetProgramEnvParameterfvARB,glGetProgramEnvParameterfvARBPROC,"glGetProgramEnvParameterfvARB");
    SAFE_GET_PROC(qglGetProgramLocalParameterdvARB,glGetProgramLocalParameterdvARBPROC,"glGetProgramLocalParameterdvARB");
    SAFE_GET_PROC(qglGetProgramLocalParameterfvARB,glGetProgramLocalParameterfvARBPROC,"glGetProgramLocalParameterfvARB");
    SAFE_GET_PROC(qglGetProgramivARB,glGetProgramivARBPROC,"glGetProgramivARB");
    SAFE_GET_PROC(qglGetProgramStringARB,glGetProgramStringARBPROC,"glGetProgramStringARB");
    SAFE_GET_PROC(qglGetVertexAttribdvARB,glGetVertexAttribdvARBPROC,"glGetVertexAttribdvARB");
    SAFE_GET_PROC(qglGetVertexAttribfvARB,glGetVertexAttribfvARBPROC,"glGetVertexAttribfvARB");
    SAFE_GET_PROC(qglGetVertexAttribivARB,glGetVertexAttribivARBPROC,"glGetVertexAttribivARB");
    SAFE_GET_PROC(qglGetVertexAttribPointervARB,glGetVertexAttribPointervARBPROC,"glGetVertexAttribPointervARB");
    SAFE_GET_PROC(qglIsProgramARB,glIsProgramARBPROC,"glIsProgramARB");

    if ( strstr(gl_extensions, "GL_ATI_pn_triangles") )     {         SAFE_GET_PROC( qglPNTrianglesiATI, PFNGLPNTRIANGLESIATIPROC, "glPNTrianglesiATI");
        SAFE_GET_PROC( qglPNTrianglesfATI, PFNGLPNTRIANGLESFATIPROC, "glPNTrianglesfATI");
    }
#endif /* !__APPLE__ && !MACOSX */

    glEnable(GL_VERTEX_PROGRAM_ARB);
    checkerror();

	gl_programs[numGlPrograms++] = LoadProgram( GL_VERTEX_PROGRAM_ARB, "glprogs/light_vp_1.txt" );
	gl_programs[numGlPrograms++] = LoadProgram( GL_VERTEX_PROGRAM_ARB, "glprogs/light_vp_1.txt" );

    glDisable(GL_VERTEX_PROGRAM_ARB);
    checkerror();

    glEnable(GL_FRAGMENT_PROGRAM_ARB);
    checkerror();

	gl_programs[numGlPrograms++] = LoadProgram( GL_FRAGMENT_PROGRAM_ARB, "glprogs/light_fp_1.txt" );
	gl_programs[numGlPrograms++] = LoadProgram( GL_FRAGMENT_PROGRAM_ARB, "glprogs/light_fp_1.txt" );

    glDisable(GL_FRAGMENT_PROGRAM_ARB);
    checkerror();

}

int GL_LoadShader( int target, const char *fileName ) {
	
	glEnable(target);
	checkerror();

	gl_programs[numGlPrograms++] = LoadProgram( target, fileName );

	glDisable(target);
	checkerror();

	return gl_programs[numGlPrograms-1];
}


void GL_DisableDiffuseShaderARB()
{
    //tex 0 = normal map
    //tex 1 = color map
    //tex 2 = attenuation 3d texture
    //tex 3 = (light filter, depends on light settings)

    glDisable(GL_FRAGMENT_PROGRAM_ARB);
    glDisable(GL_VERTEX_PROGRAM_ARB);

    GL_SelectTexture(GL_TEXTURE1_ARB);

    GL_SelectTexture(GL_TEXTURE2_ARB);
    glPopMatrix();

    if (currentshadowlight->filtercube)
    {
	GL_SelectTexture(GL_TEXTURE3_ARB);
	glPopMatrix();
    }
    glMatrixMode(GL_MODELVIEW);

    GL_SelectTexture(GL_TEXTURE0_ARB);
}

void GL_EnableDiffuseSpecularShaderARB(qboolean world, vec3_t lightOrig)
{
    // No need to enable/disable textures as the fragment program
    // extension uses the information contained in the program anyway
    float invrad = 1/currentshadowlight->radius;

    //tex 0 = normal map
    //tex 1 = color map
    //tex 2 = attenuation 3d texture
    //tex 3 = (light filter, depends on light settings)

    GL_SelectTexture(GL_TEXTURE2_ARB);
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glLoadIdentity();

    glBindTexture(GL_TEXTURE_3D, atten3d_texture_object);

    glTranslatef(0.5,0.5,0.5);
    glScalef(0.5,0.5,0.5);
    glScalef(invrad, invrad, invrad);
    glTranslatef(-lightOrig[0], -lightOrig[1], -lightOrig[2]);

    glGetError();
    glEnable(GL_VERTEX_PROGRAM_ARB);
    checkerror();
    glEnable(GL_FRAGMENT_PROGRAM_ARB);
    checkerror();

    if (currentshadowlight->filtercube) {
		GL_SelectTexture(GL_TEXTURE3_ARB);
		glMatrixMode(GL_TEXTURE);
		glPushMatrix();
		glLoadIdentity();

		glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, currentshadowlight->filtercube);
		GL_SetupCubeMapMatrix(world);

		qglBindProgramARB( GL_VERTEX_PROGRAM_ARB, gl_programs[1] );
		checkerror();
		qglBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, gl_programs[3] );
		checkerror();
    } else {
		qglBindProgramARB( GL_VERTEX_PROGRAM_ARB, gl_programs[0] );
		checkerror();
		qglBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, gl_programs[2] );
		checkerror();
    }

    GL_SelectTexture(GL_TEXTURE0_ARB);
}

void R_DrawWorldARBDiffuseSpecular(lightcmd_t *lightCmds) 
{
    int command, num, i;
    int lightPos = 0;
    vec3_t lightOr;
    msurface_t *surf;
    float               *v;
    float* lightP;
    vec3_t lightDir;
    vec3_t tsH,H;

    texture_t   *t;//XYZ

    //support flickering lights
    VectorCopy(currentshadowlight->origin,lightOr);

    while (1)
    {
        command = lightCmds[lightPos++].asInt;
        if (command == 0) break; //end of list

        surf = lightCmds[lightPos++].asVoid;

        if (surf->visframe != r_framecount) {
            lightPos+=(4+surf->polys->numverts*(2+3));
            continue;
        }

        num = surf->polys->numverts;
        lightPos+=4;//skip color

        //XYZ
        t = R_TextureAnimation (surf->texinfo->texture);

        GL_SelectTexture(GL_TEXTURE0_ARB);
        GL_Bind(t->gl_texturenum+1);
        GL_SelectTexture(GL_TEXTURE1_ARB);
        GL_Bind(t->gl_texturenum);

        glBegin(command);
        //v = surf->polys->verts[0];
        v = (float *)(&globalVertexTable[surf->polys->firstvertex]);
        for (i=0; i<num; i++, v+= VERTEXSIZE)
        {
            //skip attent texture coord.
            lightPos += 2;

            lightP = &lightCmds[lightPos].asFloat;
            lightPos += 3;

            VectorCopy(lightP, lightDir);
            VectorNormalize(lightDir);

            //calculate local H vector and put it into tangent space

            //r_origin = camera position
            VectorSubtract(r_refdef.vieworg,v,H);
            VectorNormalize(H);

            //put H in tangent space first since lightDir (precalc) is already in tang space
            if (surf->flags & SURF_PLANEBACK)
            {
                tsH[2] = -DotProduct(H,surf->plane->normal);
            }
            else
            { 
                tsH[2] = DotProduct(H,surf->plane->normal);
            }

            tsH[1] = -DotProduct(H,surf->texinfo->vecs[1]);
            tsH[0] = DotProduct(H,surf->texinfo->vecs[0]);

            VectorAdd(lightDir,tsH,tsH);

            // diffuse
            qglMultiTexCoord2fARB(GL_TEXTURE0_ARB, v[3], v[4]);
            
            qglMultiTexCoord3fvARB(GL_TEXTURE1_ARB, lightP);

            // half vector for specular
            qglMultiTexCoord3fvARB(GL_TEXTURE2_ARB,&tsH[0]);

            glVertex3fv(&v[0]);
        }
        glEnd();
    }

    GL_SelectTexture(GL_TEXTURE0_ARB);
}



void R_DrawBrushARBDiffuseSpecular(entity_t *e)
{
    model_t     *model = e->model;
    msurface_t *surf;
    glpoly_t    *poly;
    int         i, j, count;
    brushlightinstant_t *ins = e->brushlightinstant;
    float       *v;
    texture_t   *t;//XYZ        

    count = 0;

    surf = &model->surfaces[model->firstmodelsurface];
    for (i=0; i<model->nummodelsurfaces; i++, surf++)
    {
        if (!ins->polygonVis[i]) continue;

        poly = surf->polys;

        //XYZ
        t = R_TextureAnimation (surf->texinfo->texture);

        GL_SelectTexture(GL_TEXTURE0_ARB);
        GL_Bind(t->gl_texturenum+1);
        GL_SelectTexture(GL_TEXTURE1_ARB);
        GL_Bind(t->gl_texturenum);

        glBegin(GL_TRIANGLE_FAN);
        //v = poly->verts[0];
        v = (float *)(&globalVertexTable[poly->firstvertex]);
        for (j=0 ; j<poly->numverts ; j++, v+= VERTEXSIZE)
        {       
            qglMultiTexCoord2fARB(GL_TEXTURE0_ARB, v[3], v[4]);
            qglMultiTexCoord3fvARB(GL_TEXTURE1_ARB,&ins->tslights[count+j][0]);
            qglMultiTexCoord3fvARB(GL_TEXTURE2_ARB,&ins->tshalfangles[count+j][0]);
            glVertex3fv(v);
        }
        glEnd();
        count+=surf->numedges;
    }   
}


void R_DrawAliasFrameARBDiffuseSpecular (aliashdr_t *paliashdr, aliasframeinstant_t *instant)
{

    mtriangle_t *tris;
    fstvert_t   *texcoords;
    int anim;
    int                 *indecies;
    aliaslightinstant_t *linstant = instant->lightinstant;

    tris = (mtriangle_t *)((byte *)paliashdr + paliashdr->triangles);
    texcoords = (fstvert_t *)((byte *)paliashdr + paliashdr->texcoords);

    //bind normal map
    anim = (int)(cl.time*10) & 3;

    GL_SelectTexture(GL_TEXTURE0_ARB);
    GL_Bind(paliashdr->gl_texturenum[currententity->skinnum][anim]+1);
    GL_SelectTexture(GL_TEXTURE1_ARB);
    GL_Bind(paliashdr->gl_texturenum[currententity->skinnum][anim]);
        
    indecies = (int *)((byte *)paliashdr + paliashdr->indecies);

    glVertexPointer(3, GL_FLOAT, 0, instant->vertices);
    glEnableClientState(GL_VERTEX_ARRAY);
    glNormalPointer(GL_FLOAT, 0, instant->normals);
    glEnableClientState(GL_NORMAL_ARRAY);

    qglClientActiveTextureARB(GL_TEXTURE0_ARB);
    glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    qglClientActiveTextureARB(GL_TEXTURE1_ARB);
    glTexCoordPointer(3, GL_FLOAT, 0, linstant->tslights);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    qglClientActiveTextureARB(GL_TEXTURE2_ARB);
    glTexCoordPointer(3, GL_FLOAT, 0, linstant->tshalfangles);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);


    //glDrawElements(GL_TRIANGLES,paliashdr->numtris*3,GL_UNSIGNED_INT,indecies);
    glDrawElements(GL_TRIANGLES,linstant->numtris*3,GL_UNSIGNED_INT,&linstant->indecies[0]);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    qglClientActiveTextureARB(GL_TEXTURE0_ARB);
    GL_SelectTexture(GL_TEXTURE0_ARB);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void R_DrawWorldBumpedARB()
{
    if (!currentshadowlight->visible)
        return;

    glDepthMask (0);
    glShadeModel (GL_SMOOTH);

    GL_AddColor();
    glColor3fv(&currentshadowlight->color[0]);

    GL_EnableDiffuseSpecularShaderARB(true,currentshadowlight->origin);
    R_DrawWorldARBDiffuseSpecular(currentshadowlight->lightCmds);
    GL_DisableDiffuseShaderARB();

    glColor3f (1,1,1);
    glDisable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask (1);
}

void R_DrawBrushBumpedARB(entity_t *e)
{
    GL_AddColor();
    glColor3fv(&currentshadowlight->color[0]);

    GL_EnableDiffuseSpecularShaderARB(false,((brushlightinstant_t *)e->brushlightinstant)->lightpos);
    R_DrawBrushARBDiffuseSpecular(e);
    GL_DisableDiffuseShaderARB();
}

void R_DrawAliasBumpedARB(aliashdr_t *paliashdr, aliasframeinstant_t *instant)
{
    if ( gl_truform.value )
    {
        glEnable(GL_PN_TRIANGLES_ATI);
	qglPNTrianglesiATI(GL_PN_TRIANGLES_POINT_MODE_ATI, GL_PN_TRIANGLES_POINT_MODE_CUBIC_ATI);
	qglPNTrianglesiATI(GL_PN_TRIANGLES_NORMAL_MODE_ATI, GL_PN_TRIANGLES_NORMAL_MODE_QUADRATIC_ATI);
	qglPNTrianglesiATI(GL_PN_TRIANGLES_TESSELATION_LEVEL_ATI, gl_truform_tesselation.value);
    }

    GL_AddColor();
    glColor3fv(&currentshadowlight->color[0]);

    GL_EnableDiffuseSpecularShaderARB(false,instant->lightinstant->lightpos);
    R_DrawAliasFrameARBDiffuseSpecular(paliashdr,instant);
    GL_DisableDiffuseShaderARB();

    if ( gl_truform.value )
    {
        glDisable(GL_PN_TRIANGLES_ATI);
    }
}
