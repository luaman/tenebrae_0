/*
Copyright (C) 2001-2002 Charles Hollemeersch
NV_fragment_progam version (C) 2003 Jarno Paananen

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

Same as gl_bumpmap.c but NV3x optimized as they can't really cope with
the ARB path.
These routines require 5 texture units, vertex shader and pixel shader

All lights require 1 pass:
1 diffuse + specular with optional light filter

*/

#include "quakedef.h"

// NV_vertex_program is already in glquake.h

// NV_fragment_program
#ifndef GL_NV_fragment_program
#define GL_FRAGMENT_PROGRAM_NV            0x8870
#define GL_MAX_TEXTURE_COORDS_NV          0x8871
#define GL_MAX_TEXTURE_IMAGE_UNITS_NV     0x8872
#define GL_FRAGMENT_PROGRAM_BINDING_NV    0x8873
#define GL_PROGRAM_ERROR_STRING_NV        0x8874
#define GL_MAX_FRAGMENT_PROGRAM_LOCAL_PARAMETERS_NV 0x8868
#endif

#ifndef GL_NV_fragment_program
#define GL_NV_fragment_program 1
#ifdef GL_GLEXT_PROTOTYPES
extern void APIENTRY qglProgramNamedParameter4fNV(GLuint id, GLsizei len, const GLubyte *name, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void APIENTRY qglProgramNamedParameter4dNV(GLuint id, GLsizei len, const GLubyte *name, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern void APIENTRY qglProgramNamedParameter4fvNV(GLuint id, GLsizei len, const GLubyte *name, const GLfloat v[]);
extern void APIENTRY qglProgramNamedParameter4dvNV(GLuint id, GLsizei len, const GLubyte *name, const GLdouble v[]);
extern void APIENTRY qglGetProgramNamedParameterfvNV(GLuint id, GLsizei len, const GLubyte *name, GLfloat *params);
extern void APIENTRY qglGetProgramNamedParameterdvNV(GLuint id, GLsizei len, const GLubyte *name, GLdouble *params);
#endif
typedef void (APIENTRY * PFNGLPROGRAMNAMEDPARAMETER4FNVPROC) (GLuint id, GLsizei len, const GLubyte *name, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (APIENTRY * PFNGLPROGRAMNAMEDPARAMETER4DNVPROC) (GLuint id, GLsizei len, const GLubyte *name, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (APIENTRY * PFNGLPROGRAMNAMEDPARAMETER4FVNVPROC) (GLuint id, GLsizei len, const GLubyte *name, const GLfloat v[]);
typedef void (APIENTRY * PFNGLPROGRAMNAMEDPARAMETER4DVNVPROC) (GLuint id, GLsizei len, const GLubyte *name, const GLdouble v[]);
typedef void (APIENTRY * PFNGLGETPROGRAMNAMEDPARAMETERFVNVPROC) (GLuint id, GLsizei len, const GLubyte *name, GLfloat *params);
typedef void (APIENTRY * PFNGLGETPROGRAMNAMEDPARAMETERDVNVPROC) (GLuint id, GLsizei len, const GLubyte *name, GLdouble *params);
#endif

static char vertexprogram[] =
"!!VP1.1\n"
"OPTION NV_position_invariant;\n"
"DP4   o[TEX3].x, c[4], v[OPOS];\n"
"DP4   o[TEX3].y, c[5], v[OPOS];\n"
"DP4   o[TEX3].z, c[6], v[OPOS];\n"
"DP4   o[TEX3].w, c[7], v[OPOS];\n"
"MOV   o[TEX0], v[TEX0];\n"
"MOV   o[TEX1], v[TEX1];\n"
"MOV   o[TEX2], v[TEX2];\n"
"MOV   o[COL0], v[COL0];\n"
"END";

static char vertexprogram2[] =
"!!VP1.1\n"
"OPTION NV_position_invariant;\n"
"DP4   o[TEX3].x, c[4], v[OPOS];\n"
"DP4   o[TEX3].y, c[5], v[OPOS];\n"
"DP4   o[TEX3].z, c[6], v[OPOS];\n"
"DP4   o[TEX3].w, c[7], v[OPOS];\n"
"DP4   o[TEX4].x, c[8], v[OPOS];\n"
"DP4   o[TEX4].y, c[9], v[OPOS];\n"
"DP4   o[TEX4].z, c[10], v[OPOS];\n"
"DP4   o[TEX4].w, c[11], v[OPOS];\n"
"MOV   o[TEX0], v[TEX0];\n"
"MOV   o[TEX1], v[TEX1];\n"
"MOV   o[TEX2], v[TEX2];\n"
"MOV   o[COL0], v[COL0];\n"
"END";

// H0 = normal
// H1 = light vec, half vec
// H2 = temp (x = diff dot, y = self shadow)
// H3 = temp

// Use normalization cube
static char fragmentprogram[] =
"!!FP1.0\n"
"TEX H0, f[TEX0], TEX0, 2D;\n"
"TEX H1, f[TEX1], TEX3, CUBE;\n"
"MADH H0.xyz, H0, 2, -1;\n"
"MADH H1.xyz, H1, 2, -1;\n"
"DP3H_SAT H2.x, H0, H1;\n"
"TEX H3, f[TEX0], TEX1, 2D;\n"
"MULH H3, H3, H2.x;\n"
"MULH H3, H3, f[COL0];\n"
"MULH_SAT H2.y, H1.z, 8;\n"
"MULH H2.x, H2.x, H2.y;\n"
"TEX H1, f[TEX2], TEX3, CUBE;\n"
"MADH H1.xyz, H1, 2, -1;\n"
"DP3H_SAT H2.z, H0, H1;\n"
"POWH H2.z, H2.z, 16;\n"
"MULX H2.z, H2.z, H0.w;\n"
"MADX H3, H3, H2.y, H2.z;\n"
"TEX H1, f[TEX3], TEX2, 3D;\n"
"MULX_SAT o[COLH], H3, H1;\n"
"END";

static char fragmentprogram2[] =
"!!FP1.0\n"
"TEX H0, f[TEX0], TEX0, 2D;\n"
"TEX H1, f[TEX1], TEX3, CUBE;\n"
"MADH H0.xyz, H0, 2, -1;\n"
"MADH H1.xyz, H1, 2, -1;\n"
"DP3H_SAT H2.x, H0, H1;\n"
"TEX H3, f[TEX0], TEX1, 2D;\n"
"MULH H3, H3, H2.x;\n"
"MULH H3, H3, f[COL0];\n"
"MULH_SAT H2.y, H1.z, 8;\n"
"MULH H2.x, H2.x, H2.y;\n"
"TEX H1, f[TEX2], TEX3, CUBE;\n"
"MADH H1.xyz, H1, 2, -1;\n"
"DP3H_SAT H2.z, H0, H1;\n"
"POWH H2.z, H2.z, 16;\n"
"MULX H2.z, H2.z, H0.w;\n"
"MADX H3, H3, H2.y, H2.z;\n"
"TEX H1, f[TEX3], TEX2, 3D;\n"
"TEX H2, f[TEX4], TEX4, CUBE;\n"
"MULX H3, H3, H2;\n"
"MULX_SAT o[COLH], H3, H1;\n"
"END";

static GLuint fragment_programs[2];
static GLuint vertex_programs[2];


//#define NV3xDEBUG

#if defined(NV3xDEBUG) && defined(_WIN32)
static void checkerror()
{
    GLuint error = glGetError();
    if ( error != GL_NO_ERROR )
    {
        int line;
        const char* err;
        
        glGetIntegerv(GL_PROGRAM_ERROR_POSITION_NV, &line);
        err = (const char*)glGetString(GL_PROGRAM_ERROR_STRING_NV);
        _asm { int 3 };
    }
}
#else

#define checkerror() do { } while(0)

#endif


void GL_CreateShadersNV3x()
{
#if !defined(__APPLE__) && !defined (MACOSX)
    // NV_vertex_program entry points

    // NV_fragment_program entry points
    // These happen to be already queried by the GF3/4 path

    SAFE_GET_PROC( qglActiveStencilFaceEXT, PFNGLACTIVESTENCILFACEEXTPROC, "glActiveStencilFaceEXT");
#endif /* !__APPLE__ && !MACOSX */

    glEnable(GL_VERTEX_PROGRAM_NV);
    checkerror();

    qglGenProgramsNV(2, &vertex_programs[0]);
    checkerror();

    qglLoadProgramNV( GL_VERTEX_PROGRAM_NV, vertex_programs[0],
  		      strlen(vertexprogram), (const GLubyte *) vertexprogram);
    checkerror();

    qglLoadProgramNV( GL_VERTEX_PROGRAM_NV, vertex_programs[1],
  		      strlen(vertexprogram2), (const GLubyte *) vertexprogram2);
    checkerror();

    // Track the texture unit 2 matrix in registers 4-7
    qglTrackMatrixNV( GL_VERTEX_PROGRAM_NV, 4, GL_TEXTURE2_ARB, GL_IDENTITY_NV );

    // Track the texture unit 3 matrix in registers 8-11
    qglTrackMatrixNV( GL_VERTEX_PROGRAM_NV, 8, GL_TEXTURE3_ARB, GL_IDENTITY_NV );

    glDisable(GL_VERTEX_PROGRAM_NV);
    checkerror();

    glEnable(GL_FRAGMENT_PROGRAM_NV);
    checkerror();

    qglGenProgramsNV(2, &fragment_programs[0]);
    checkerror();

    qglLoadProgramNV( GL_FRAGMENT_PROGRAM_NV, fragment_programs[0],
  		      strlen(fragmentprogram), (const GLubyte *) fragmentprogram);
    checkerror();

    qglLoadProgramNV( GL_FRAGMENT_PROGRAM_NV, fragment_programs[1],
  		      strlen(fragmentprogram2), (const GLubyte *) fragmentprogram2);
    checkerror();

    glDisable(GL_FRAGMENT_PROGRAM_NV);
    checkerror();
}

void GL_DisableDiffuseShaderNV3x()
{
    //tex 0 = normal map
    //tex 1 = color map
    //tex 2 = attenuation 3d texture
    //tex 3 = normalization cubemap
    //tex 4 = (light filter, depends on light settings)

    glDisable(GL_FRAGMENT_PROGRAM_NV);
    glDisable(GL_VERTEX_PROGRAM_NV);

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

void GL_EnableDiffuseSpecularShaderNV3x(qboolean world, vec3_t lightOrig)
{
    float invrad = 1/currentshadowlight->radius;

    //tex 0 = normal map
    //tex 1 = color map
    //tex 2 = attenuation 3d texture
    //tex 3 = normalization cubemap
    //tex 4 = (light filter, depends on light settings)

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
    glEnable(GL_VERTEX_PROGRAM_NV);
    checkerror();
    glEnable(GL_FRAGMENT_PROGRAM_NV);
    checkerror();

    GL_SelectTexture(GL_TEXTURE3_ARB);
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, normcube_texture_object);

    if (currentshadowlight->filtercube)
    {
        // the funny thing about the NV fragment program is that you don't need
        // to enable/disable textures (and actually can't for textures > 3 )
        // so we happily mix texture matrices and the actual textures :)
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();
        GL_SetupCubeMapMatrix(world);

        GL_SelectTexture(GL_TEXTURE4_ARB);
        glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, currentshadowlight->filtercube);

	qglBindProgramNV( GL_VERTEX_PROGRAM_NV, vertex_programs[1] );
	checkerror();
	qglBindProgramNV( GL_FRAGMENT_PROGRAM_NV, fragment_programs[1] );
	checkerror();
    }
    else
    {

	qglBindProgramNV( GL_VERTEX_PROGRAM_NV, vertex_programs[0] );
	checkerror();
	qglBindProgramNV( GL_FRAGMENT_PROGRAM_NV, fragment_programs[0] );
	checkerror();
    }

    GL_SelectTexture(GL_TEXTURE0_ARB);
}

void R_DrawWorldNV3xDiffuseSpecular(lightcmd_t *lightCmds) 
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



void R_DrawBrushNV3xDiffuseSpecular(entity_t *e)
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


void R_DrawAliasFrameNV3xDiffuseSpecular (aliashdr_t *paliashdr, aliasframeinstant_t *instant)
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

void R_DrawWorldBumpedNV3x()
{
    if (!currentshadowlight->visible)
        return;

    glDepthMask (0);
    glShadeModel (GL_SMOOTH);

    GL_AddColor();
    glColor3fv(&currentshadowlight->color[0]);

    GL_EnableDiffuseSpecularShaderNV3x(true,currentshadowlight->origin);
    R_DrawWorldNV3xDiffuseSpecular(currentshadowlight->lightCmds);
    GL_DisableDiffuseShaderNV3x();

    glColor3f (1,1,1);
    glDisable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask (1);
}

void R_DrawBrushBumpedNV3x(entity_t *e)
{
    GL_AddColor();
    glColor3fv(&currentshadowlight->color[0]);

    GL_EnableDiffuseSpecularShaderNV3x(false,((brushlightinstant_t *)e->brushlightinstant)->lightpos);
    R_DrawBrushNV3xDiffuseSpecular(e);
    GL_DisableDiffuseShaderNV3x();
}

void R_DrawAliasBumpedNV3x(aliashdr_t *paliashdr, aliasframeinstant_t *instant)
{
    GL_AddColor();
    glColor3fv(&currentshadowlight->color[0]);

    GL_EnableDiffuseSpecularShaderNV3x(false,instant->lightinstant->lightpos);
    R_DrawAliasFrameNV3xDiffuseSpecular(paliashdr,instant);
    GL_DisableDiffuseShaderNV3x();
}
