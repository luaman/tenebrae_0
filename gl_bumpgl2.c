/*
Copyright (C) 2001-2002 Charles Hollemeersch
GLSlang version (C) 2003 Jarno Paananen

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

Same as gl_bumpmap.c but for GL Shading language extension
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

#if  !defined(GL_ARB_vertex_shader) && !defined(GL_ARB_fragment_shader) && !defined(GL_ARB_shader_objects)

typedef int GLhandleARB;
typedef char GLcharARB;

#define GL_PROGRAM_OBJECT_ARB				0x8B40
#define GL_OBJECT_TYPE_ARB                  0x8B4E
#define GL_OBJECT_SUBTYPE_ARB               0x8B4F

#define GL_SHADER_OBJECT_ARB				0x8B48
#define GL_FLOAT_VEC2_ARB                   0x8B50
#define GL_FLOAT_VEC3_ARB                   0x8B51   
#define GL_FLOAT_VEC4_ARB                   0x8B52   
#define GL_INT_VEC2_ARB                     0x8B53   
#define GL_INT_VEC3_ARB                     0x8B54   
#define GL_INT_VEC4_ARB                     0x8B55   
#define GL_BOOL_ARB                         0x8B56   
#define GL_BOOL_VEC2_ARB                    0x8B57   
#define GL_BOOL_VEC3_ARB                    0x8B58   
#define GL_BOOL_VEC4_ARB                    0x8B59   
#define GL_FLOAT_MAT2_ARB                   0x8B5A   
#define GL_FLOAT_MAT3_ARB                   0x8B5B   
#define GL_FLOAT_MAT4_ARB                   0x8B5C 

#define GL_VERTEX_SHADER_ARB		        	 0x8B31  
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB     0x8B4A
#define GL_MAX_VERTEX_ATTRIBS_ARB                0x8869
#define GL_MAX_TEXTURE_IMAGE_UNITS_ARB           0x8872
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB    0x8B4C 
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB  0x8B4D
#define GL_MAX_TEXTURE_COORDS_ARB                0x8871

#define GL_VERTEX_ATTRIB_ARRAY_ENABLED_ARB       0x8622
#define GL_VERTEX_ATTRIB_ARRAY_SIZE_ARB          0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE_ARB        0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE_ARB          0x8625
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED_ARB    0x886A
#define GL_CURRENT_VERTEX_ATTRIB_ARB             0x8626
#define GL_VERTEX_ATTRIB_ARRAY_POINTER_ARB       0x8645

#define GL_FRAGMENT_SHADER_ARB                   0x8B30
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB   0x8B49

#define GL_MAX_VARYING_FLOATS_ARB			     0x8B4B

#define GL_VERTEX_PROGRAM_POINT_SIZE_ARB         0x8642
#define GL_VERTEX_PROGRAM_TWO_SIDE_ARB           0x8643

#define GL_OBJECT_DELETE_STATUS_ARB              0x8B80
#define GL_OBJECT_COMPILE_STATUS_ARB             0x8B81
#define GL_OBJECT_LINK_STATUS_ARB                0x8B82
#define GL_OBJECT_VALIDATE_STATUS_ARB            0x8B83
#define GL_OBJECT_INFO_LOG_LENGTH_ARB            0x8B84
#define GL_OBJECT_ATTACHED_OBJECTS_ARB           0x8B85
#define GL_OBJECT_ACTIVE_UNIFORMS_ARB            0x8B86
#define GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB  0x8B87
#define GL_OBJECT_SHADER_SOURCE_LENGTH_ARB       0x8B88
#define GL_OBJECT_ACTIVE_ATTRIBUTE_MAX_LENGTH_ARB    0x8B8A
#define GL_OBJECT_ACTIVE_ATTRIBUTES_ARB              0x8B89




//  *************************************************
//     Rest of these support the old GL2 or VERTEX_ARRAY code
//  ************************************************

// Keep for Vertex Array code
#define GL_VERTEX_ARRAY_OBJECT_GL2			0x40004
#define GL_VERTEX_ARRAY_FORMAT_OBJECT_GL2	0x40008
#define GL_PAD_ARRAY_GL2				    0x80000
#define GL_ALL_INDEX_ARRAY_GL2				0x80001
#define GL_TEXTURE_COORD0_ARRAY_GL2			0x80002
#define GL_TEXTURE_COORD1_ARRAY_GL2			0x80003
#define GL_TEXTURE_COORD2_ARRAY_GL2			0x80004
#define GL_TEXTURE_COORD3_ARRAY_GL2			0x80005
#define GL_TEXTURE_COORD4_ARRAY_GL2			0x80006
#define GL_TEXTURE_COORD5_ARRAY_GL2			0x80007
#define GL_TEXTURE_COORD6_ARRAY_GL2			0x80008
#define GL_TEXTURE_COORD7_ARRAY_GL2			0x80009
#define GL_USER_ATTRIBUTE_ARRAY0_GL2		0x8000A
#define GL_USER_ATTRIBUTE_ARRAY1_GL2		0x8000B
#define GL_USER_ATTRIBUTE_ARRAY2_GL2		0x8000C
#define GL_USER_ATTRIBUTE_ARRAY3_GL2		0x8000D
#define GL_USER_ATTRIBUTE_ARRAY4_GL2		0x8000E
#define GL_USER_ATTRIBUTE_ARRAY5_GL2		0x8000F
#define GL_USER_ATTRIBUTE_ARRAY6_GL2		0x80010
#define GL_USER_ATTRIBUTE_ARRAY7_GL2		0x80011
#define GL_USER_ATTRIBUTE_ARRAY8_GL2		0x80012
#define GL_USER_ATTRIBUTE_ARRAY9_GL2		0x80013
#define GL_USER_ATTRIBUTE_ARRAY10_GL2		0x80014
#define GL_USER_ATTRIBUTE_ARRAY11_GL2		0x80015
#define GL_USER_ATTRIBUTE_ARRAY12_GL2		0x80016
#define GL_USER_ATTRIBUTE_ARRAY13_GL2		0x80017
#define GL_USER_ATTRIBUTE_ARRAY14_GL2		0x80018
#define GL_USER_ATTRIBUTE_ARRAY15_GL2		0x80019


#endif // If shader_object, fragment_shader, and vertex_shader is not defined

#ifndef GL_ARB_shader_objects
#define GL_ARB_shader_objects 1

#ifndef GL_ARB_fragment_shader
#define GL_ARB_fragment_shader 1

#ifndef GL_ARB_vertex_shader
#define GL_ARB_vertex_shader 1

// Taken from ARB_vertex_program
#ifndef GL_ARB_vertex_program
#define GL_ARB_vertex_program 1
typedef void (APIENTRY * PFNGLVERTEXATTRIB4FVARBPROC)	(GLuint index, const GLfloat *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB3FVARBPROC)	(GLuint index, const GLfloat *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB2FVARBPROC)	(GLuint index, const GLfloat *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB1FVARBPROC)	(GLuint index, const GLfloat *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4FARBPROC)		(GLuint index, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void (APIENTRY * PFNGLVERTEXATTRIB3FARBPROC)		(GLuint index, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void (APIENTRY * PFNGLVERTEXATTRIB2FARBPROC)		(GLuint index, GLfloat v0, GLfloat v1);
typedef void (APIENTRY * PFNGLVERTEXATTRIB1FARBPROC)		(GLuint index, GLfloat v0);

typedef void (APIENTRY * PFNGLVERTEXATTRIB4DVARBPROC)	(GLuint index, const GLdouble *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB3DVARBPROC)	(GLuint index, const GLdouble *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB2DVARBPROC)	(GLuint index, const GLdouble *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB1DVARBPROC)	(GLuint index, const GLdouble *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4DARBPROC)		(GLuint index, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3);
typedef void (APIENTRY * PFNGLVERTEXATTRIB3DARBPROC)		(GLuint index, GLdouble v0, GLdouble v1, GLdouble v2);
typedef void (APIENTRY * PFNGLVERTEXATTRIB2DARBPROC)		(GLuint index, GLdouble v0, GLdouble v1);
typedef void (APIENTRY * PFNGLVERTEXATTRIB1DARBPROC)		(GLuint index, GLdouble v0);

typedef void (APIENTRY * PFNGLVERTEXATTRIB4SVARBPROC)	(GLuint index, const GLshort *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB3SVARBPROC)	(GLuint index, const GLshort *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB2SVARBPROC)	(GLuint index, const GLshort *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB1SVARBPROC)	(GLuint index, const GLshort *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4SARBPROC)		(GLuint index, GLshort v0, GLshort v1, GLshort v2, GLshort v3);
typedef void (APIENTRY * PFNGLVERTEXATTRIB3SARBPROC)		(GLuint index, GLshort v0, GLshort v1, GLshort v2);
typedef void (APIENTRY * PFNGLVERTEXATTRIB2SARBPROC)		(GLuint index, GLshort v0, GLshort v1);
typedef void (APIENTRY * PFNGLVERTEXATTRIB1SARBPROC)		(GLuint index, GLshort v0);

typedef void (APIENTRY * PFNGLVERTEXATTRIB4NUBARBPROC)	(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4UBVARBPROC)	(GLuint index, const GLubyte *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4USVARBPROC)	(GLuint index, const GLushort *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4UIVARBPROC)	(GLuint index, const GLuint *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4IVARBPROC)	(GLuint index, const GLint *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4BVARBPROC)	(GLuint index, const GLbyte *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4NBVARBPROC)	(GLuint index, const GLbyte *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4NSVARBPROC)	(GLuint index, const GLshort *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4NIVARBPROC)	(GLuint index, const GLint *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4NUBVARBPROC)	(GLuint index, const GLubyte *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4NUSVARBPROC)	(GLuint index, const GLushort *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4NUIVARBPROC)	(GLuint index, const GLuint *v);

typedef void (APIENTRY * PFNGLVERTEXATTRIBPOINTERARBPROC)	(GLuint index, GLint size, GLenum type, GLboolean normalize, GLsizei stride, const GLvoid *pointer);
typedef void (APIENTRY * PFNGLENABLEVERTEXATTRIBARRAYARBPROC)	(GLuint index);
typedef void (APIENTRY * PFNGLDISABLEVERTEXATTRIBARRAYARBPROC)	(GLuint index);
typedef void (APIENTRY * PFNGLGETVERTEXATTRIBDVARBPROC) (GLuint index, GLenum pname, GLdouble *params);

typedef void (APIENTRY * PFNGLGETVERTEXATTRIBFVARBPROC) (GLuint index, GLenum pname, GLfloat *params);
typedef void (APIENTRY * PFNGLGETVERTEXATTRIBIVARBPROC) (GLuint index, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLGETVERTEXATTRIBPOINTERVARBPROC) (GLuint index, GLenum pname, void **pointer);
#endif
typedef void (APIENTRY * PFNGLBINDATTRIBLOCATIONARBPROC) (GLhandleARB programObj, GLuint index, const GLcharARB *name);
typedef void (APIENTRY * PFNGLBINDARRAYGL2PROC) (GLhandleARB shaderObject, GLenum array, const GLcharARB *name, GLint length);
typedef GLhandleARB (APIENTRY * PFNGLCREATESHADEROBJECTARBPROC) (GLenum shaderType);
typedef GLhandleARB (APIENTRY * PFNGLCREATEPROGRAMOBJECTARBPROC) ();
typedef void (APIENTRY * PFNGLDELETEOBJECTARBPROC) (GLhandleARB obj);

typedef void (APIENTRY * PFNGLSHADERSOURCEARBPROC) (GLhandleARB shaderObj, GLsizei count, const GLcharARB **string, const GLint *length);
typedef void (APIENTRY * PFNGLGETSHADERSOURCEARBPROC) (GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *source);

typedef void (APIENTRY * PFNGLCOMPILESHADERARBPROC) (GLhandleARB shaderObj);
typedef void (APIENTRY * PFNGLDETACHOBJECTARBPROC) (GLhandleARB containerObj, GLhandleARB attachedObj);
typedef void (APIENTRY * PFNGLATTACHOBJECTARBPROC) (GLhandleARB containerObject, GLhandleARB obj);
typedef void (APIENTRY * PFNGLUSEPROGRAMOBJECTARBPROC) (GLhandleARB programObj);
typedef void (APIENTRY * PFNGLGETINFOLOGARBPROC) (GLhandleARB obj,GLsizei maxLength, GLsizei *length, GLcharARB *infoLog);
typedef void (APIENTRY * PFNGLGETATTACHEDOBJECTSARBPROC) (GLhandleARB containerObj, GLsizei maxCount, GLsizei *count,
                          GLhandleARB *obj);
typedef void (APIENTRY * PFNGLLINKPROGRAMARBPROC) (GLhandleARB programObj);

typedef void (APIENTRY * PFNGLUNIFORM1FARBPROC) (GLint location, GLfloat v0);
typedef void (APIENTRY * PFNGLUNIFORM2FARBPROC) (GLint location, GLfloat v0, GLfloat v1);
typedef void (APIENTRY * PFNGLUNIFORM3FARBPROC) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void (APIENTRY * PFNGLUNIFORM4FARBPROC) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);

typedef void (APIENTRY * PFNGLUNIFORM1IARBPROC) (GLint location, GLint v0);
typedef void (APIENTRY * PFNGLUNIFORM2IARBPROC) (GLint location, GLint v0, GLint v1);
typedef void (APIENTRY * PFNGLUNIFORM3IARBPROC) (GLint location, GLint v0, GLint v1, GLint v2);
typedef void (APIENTRY * PFNGLUNIFORM4IARBPROC) (GLint location, GLint v0, GLint v1, GLint v2, GLint v3);



typedef void (APIENTRY * PFNGLUNIFORM1FVARBPROC) (GLint location, GLsizei count, GLfloat *value);
typedef void (APIENTRY * PFNGLUNIFORM2FVARBPROC) (GLint location, GLsizei count, GLfloat *value);
typedef void (APIENTRY * PFNGLUNIFORM3FVARBPROC) (GLint location, GLsizei count, GLfloat *value);
typedef void (APIENTRY * PFNGLUNIFORM4FVARBPROC) (GLint location, GLsizei count, GLfloat *value);

typedef void (APIENTRY * PFNGLUNIFORM1IVARBPROC) (GLint location, GLsizei count, GLint *value);
typedef void (APIENTRY * PFNGLUNIFORM2IVARBPROC) (GLint location, GLsizei count, GLint *value);
typedef void (APIENTRY * PFNGLUNIFORM3IVARBPROC) (GLint location, GLsizei count, GLint *value);
typedef void (APIENTRY * PFNGLUNIFORM4IVARBPROC) (GLint location, GLsizei count, GLint *value);


typedef void (APIENTRY * PFNGLUNIFORMMATRIX2FVARBPROC) (GLint location, GLuint count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRY * PFNGLUNIFORMMATRIX3FVARBPROC) (GLint location, GLuint count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRY * PFNGLUNIFORMMATRIX4FVARBPROC) (GLint location, GLuint count, GLboolean transpose, const GLfloat *value);
typedef GLint (APIENTRY * PFNGLGETUNIFORMLOCATIONARBPROC) (GLhandleARB programObj, const GLcharARB *name);
typedef void (APIENTRY * PFNGLGETACTIVEATTRIBARBPROC) (GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, 
													   GLenum *type, GLcharARB *name);
typedef void (APIENTRY * PFNGLGETACTIVEUNIFORMARBPROC) (GLhandleARB programObj, GLuint index, GLsizei maxLength,
														GLsizei *length, GLint *size, GLenum *type, GLcharARB *name);
typedef void (APIENTRY * PFNGLGETUNIFORMFVARBPROC) (GLhandleARB programObj, GLint location, GLfloat *params);
typedef void (APIENTRY * PFNGLGETUNIFORMIVARBPROC) (GLhandleARB programObj, GLint location, GLint *params);
typedef void (APIENTRY * PFNGLGETATTRIBLOCATIONARBPROC) (GLhandleARB programObj, const GLcharARB *name);
typedef void (APIENTRY * PFNGLVALIDATEPROGRAMARBPROC) (GLhandleARB programObj);

typedef void (APIENTRY * PFNGLGETOBJECTPARAMETERFVARBPROC)(GLhandleARB obj, GLenum pname, GLfloat *params);
typedef void (APIENTRY * PFNGLGETOBJECTPARAMETERIVARBPROC)(GLhandleARB obj, GLenum pname, GLint *params);


typedef GLhandleARB (APIENTRY * PFNGLGETHANDLEARBPROC) (GLenum pname);
typedef GLhandleARB (APIENTRY * PFNGLCREATEVERTEXARRAYOBJECTGL2PROC) (GLhandleARB formatObject, GLsizei count);
typedef void (APIENTRY * PFNGLLOADVERTEXARRAYDATAGL2PROC) (GLhandleARB object, GLuint start, GLsizei count, GLvoid *data, GLenum preserve);
typedef GLhandleARB (APIENTRY * PFNGLSTARTVERTEXARRAYFORMATGL2PROC) ();
typedef void (APIENTRY * PFNGLADDELEMENTGL2PROC) (GLhandleARB formatObject, GLenum array, GLsizei size, GLenum type);
typedef void (APIENTRY * PFNGLENABLEVERTEXARRAYOBJECTGL2PROC) (GLhandleARB object);
typedef void (APIENTRY * PFNGLDISABLEVERTEXARRAYOBJECTGL2PROC) (GLhandleARB object);
typedef void (APIENTRY * PFNGLDRAWINDEXEDARRAYSGL2PROC) (GLenum mode, GLuint first, GLsizei count);

#endif  // End for #ifndef GL_ARB_vertex_shader
#endif  // End for #ifndef GL_ARB_fragment_shader
#endif  // End for #ifndef GL_ARB_shader_objects

PFNGLCREATEPROGRAMOBJECTARBPROC qglCreateProgramObjectARB;
PFNGLCREATESHADEROBJECTARBPROC qglCreateShaderObjectARB;
PFNGLDELETEOBJECTARBPROC qglDeleteObjectARB;
PFNGLDETACHOBJECTARBPROC qglDetachObjectARB;
PFNGLATTACHOBJECTARBPROC qglAttachObjectARB;

PFNGLSHADERSOURCEARBPROC qglShaderSourceARB;
PFNGLCOMPILESHADERARBPROC qglCompileShaderARB;
PFNGLLINKPROGRAMARBPROC qglLinkProgramARB;
PFNGLGETINFOLOGARBPROC qglGetInfoLogARB;
PFNGLUSEPROGRAMOBJECTARBPROC qglUseProgramObjectARB;

PFNGLGETOBJECTPARAMETERIVARBPROC qglGetObjectParameterivARB;
PFNGLGETOBJECTPARAMETERFVARBPROC qglGetObjectParameterfvARB;
PFNGLGETUNIFORMLOCATIONARBPROC qglGetUniformLocationARB;

PFNGLUNIFORM1FARBPROC qglUniform1fARB;
PFNGLUNIFORM2FARBPROC qglUniform2fARB;
PFNGLUNIFORM3FARBPROC qglUniform3fARB;
PFNGLUNIFORM4FARBPROC qglUniform4fARB;

PFNGLUNIFORM1IARBPROC qglUniform1iARB;
PFNGLUNIFORM2IARBPROC qglUniform2iARB;
PFNGLUNIFORM3IARBPROC qglUniform3iARB;
PFNGLUNIFORM4IARBPROC qglUniform4iARB;

PFNGLUNIFORM1FVARBPROC qglUniform1fvARB;
PFNGLUNIFORM2FVARBPROC qglUniform2fvARB;
PFNGLUNIFORM3FVARBPROC qglUniform3fvARB;
PFNGLUNIFORM4FVARBPROC qglUniform4fvARB;

PFNGLUNIFORM1IVARBPROC qglUniform1ivARB;
PFNGLUNIFORM2IVARBPROC qglUniform2ivARB;
PFNGLUNIFORM3IVARBPROC qglUniform3ivARB;
PFNGLUNIFORM4IVARBPROC qglUniform4ivARB;

static GLhandleARB programs[2];
static GLhandleARB fragment_shaders[2];
static GLhandleARB vertex_shaders[2];

static GLcharARB* vertex_programs[2] =
{
    "hardware/bump.vert",
    "hardware/bump2.vert"
};

static GLcharARB* fragment_programs[2] =
{
    "hardware/bump.frag",
    "hardware/bump2.frag"
};


#define GL2DEBUG

#if defined(GL2DEBUG) && defined(_WIN32)
static void checkerror()
{
    GLuint error = glGetError();
    if ( error != GL_NO_ERROR )
    {
        int line;
        const char* err;

        err = gluErrorString(error);
        Con_Printf("GL2: %s\n", err);
	//        glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &line);
        //err = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
//        _asm { int 3 };
    }
}
#else

#define checkerror() do { } while(0)

#endif

void printlog(GLhandleARB obj)
{
    int blen = 0;	/* length of buffer to allocate */
    int slen = 0;	/* strlen actually written to buffer */
    GLcharARB *infoLog;

    qglGetObjectParameterivARB(obj, GL_OBJECT_INFO_LOG_LENGTH_ARB, &blen);
    if (blen > 1) {
	if ((infoLog = (GLcharARB*)malloc(blen)) == NULL) {
	    printf("ERROR: Could not allocate InfoLog buffer\n");
	    exit(1);
	}
	qglGetInfoLogARB(obj, blen, &slen, infoLog);
	Con_Printf("GL2: %s\n", infoLog);
	free(infoLog);
    }
}

void GL_CreateShadersGL2()
{
    GLint len;
    int i;

    SAFE_GET_PROC( qglCreateProgramObjectARB, PFNGLCREATEPROGRAMOBJECTARBPROC, "glCreateProgramObjectARB");
    SAFE_GET_PROC( qglCreateShaderObjectARB, PFNGLCREATESHADEROBJECTARBPROC, "glCreateShaderObjectARB");
    SAFE_GET_PROC( qglDeleteObjectARB, PFNGLDELETEOBJECTARBPROC, "glDeleteObjectARB");
    SAFE_GET_PROC( qglDetachObjectARB, PFNGLDETACHOBJECTARBPROC, "glDetachObjectARB");
    SAFE_GET_PROC( qglAttachObjectARB, PFNGLATTACHOBJECTARBPROC, "glAttachObjectARB");

    SAFE_GET_PROC( qglShaderSourceARB, PFNGLSHADERSOURCEARBPROC, "glShaderSourceARB");
    SAFE_GET_PROC( qglCompileShaderARB, PFNGLCOMPILESHADERARBPROC, "glCompileShaderARB");
    SAFE_GET_PROC( qglLinkProgramARB, PFNGLLINKPROGRAMARBPROC, "glLinkProgramARB");
    SAFE_GET_PROC( qglGetInfoLogARB, PFNGLGETINFOLOGARBPROC, "glGetInfoLogARB");
    SAFE_GET_PROC( qglUseProgramObjectARB, PFNGLUSEPROGRAMOBJECTARBPROC, "glUseProgramObjectARB");

    SAFE_GET_PROC( qglGetObjectParameterivARB, PFNGLGETOBJECTPARAMETERIVARBPROC, "glGetObjectParameterivARB");
    SAFE_GET_PROC( qglGetObjectParameterfvARB, PFNGLGETOBJECTPARAMETERFVARBPROC, "glGetObjectParameterfvARB");
    SAFE_GET_PROC( qglGetUniformLocationARB, PFNGLGETUNIFORMLOCATIONARBPROC, "glGetUniformLocationARB");

    SAFE_GET_PROC( qglUniform1fARB, PFNGLUNIFORM1FARBPROC, "glUniform1fARB");
    SAFE_GET_PROC( qglUniform2fARB, PFNGLUNIFORM2FARBPROC, "glUniform2fARB");
    SAFE_GET_PROC( qglUniform3fARB, PFNGLUNIFORM3FARBPROC, "glUniform3fARB");
    SAFE_GET_PROC( qglUniform4fARB, PFNGLUNIFORM4FARBPROC, "glUniform4fARB");

    SAFE_GET_PROC( qglUniform1iARB, PFNGLUNIFORM1IARBPROC, "glUniform1iARB");
    SAFE_GET_PROC( qglUniform2iARB, PFNGLUNIFORM2IARBPROC, "glUniform2iARB");
    SAFE_GET_PROC( qglUniform3iARB, PFNGLUNIFORM3IARBPROC, "glUniform3iARB");
    SAFE_GET_PROC( qglUniform4iARB, PFNGLUNIFORM4IARBPROC, "glUniform4iARB");

    SAFE_GET_PROC( qglUniform1fvARB, PFNGLUNIFORM1FVARBPROC, "glUniform1fvARB");
    SAFE_GET_PROC( qglUniform2fvARB, PFNGLUNIFORM2FVARBPROC, "glUniform2fvARB");
    SAFE_GET_PROC( qglUniform3fvARB, PFNGLUNIFORM3FVARBPROC, "glUniform3fvARB");
    SAFE_GET_PROC( qglUniform4fvARB, PFNGLUNIFORM4FVARBPROC, "glUniform4fvARB");

    SAFE_GET_PROC( qglUniform1ivARB, PFNGLUNIFORM1IVARBPROC, "glUniform1ivARB");
    SAFE_GET_PROC( qglUniform2ivARB, PFNGLUNIFORM2IVARBPROC, "glUniform2ivARB");
    SAFE_GET_PROC( qglUniform3ivARB, PFNGLUNIFORM3IVARBPROC, "glUniform3ivARB");
    SAFE_GET_PROC( qglUniform4ivARB, PFNGLUNIFORM4IVARBPROC, "glUniform4ivARB");

    if ( strstr(gl_extensions, "GL_ATI_pn_triangles") )
    {
        SAFE_GET_PROC( qglPNTrianglesiATI, PFNGLPNTRIANGLESIATIPROC, "glPNTrianglesiATI");
        SAFE_GET_PROC( qglPNTrianglesfATI, PFNGLPNTRIANGLESFATIPROC, "glPNTrianglesfATI");
    }

    for ( i = 0; i < 2; i++ )
    {
        char* shader;
        int status;
        GLint loc, value;

	programs[i] = qglCreateProgramObjectARB();
	checkerror();
	vertex_shaders[i] = qglCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	checkerror();
	fragment_shaders[i] = qglCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
	checkerror();

        shader = COM_LoadTempFile(vertex_programs[i]);
     	if (!shader)
        {
		//this is serious we need shader to render stuff
		Sys_Error("GL2: %s not found\n", vertex_programs[i]);
	}

	len = strlen(shader);
        qglShaderSourceARB(vertex_shaders[i], 1, (const GLcharARB**)&shader, &len);
	checkerror();

        shader = COM_LoadTempFile(fragment_programs[i]);
     	if (!shader)
        {
		//this is serious we need shader to render stuff
		Sys_Error("GL2: %s not found\n", fragment_programs[i]);
	}
	len = strlen(shader);
	qglShaderSourceARB(fragment_shaders[i], 1, (const GLcharARB**)&shader, &len);
	checkerror();
    
	qglCompileShaderARB(vertex_shaders[i]);
	checkerror();
	qglGetObjectParameterivARB(vertex_shaders[i], GL_OBJECT_COMPILE_STATUS_ARB, &status);
        checkerror();
        printlog(vertex_shaders[i]);

	qglCompileShaderARB(fragment_shaders[i]);
	checkerror();
	qglGetObjectParameterivARB(fragment_shaders[i], GL_OBJECT_COMPILE_STATUS_ARB, &status);
        checkerror();
        printlog(fragment_shaders[i]);

	qglAttachObjectARB(programs[i], vertex_shaders[i]);
	checkerror();
	qglAttachObjectARB(programs[i], fragment_shaders[i]);
	checkerror();

	qglDeleteObjectARB(vertex_shaders[i]);
	checkerror();
	qglDeleteObjectARB(fragment_shaders[i]);
	checkerror();

	qglLinkProgramARB(programs[i]);
	checkerror();

	qglUseProgramObjectARB(programs[i]);
        // link textures to units...
        qglUniform1iARB(qglGetUniformLocationARB(programs[i], "normalmap"), 0);
	checkerror();
        qglUniform1iARB(qglGetUniformLocationARB(programs[i], "colormap"), 1);
	checkerror();
        qglUniform1iARB(qglGetUniformLocationARB(programs[i], "attenuation"), 2);
	checkerror();
        if ( i == 1 )
        {
            qglUniform1iARB(qglGetUniformLocationARB(programs[i], "filter"), 3);
	    checkerror();
        }
    }
    qglUseProgramObjectARB(0);
}

void GL_DisableDiffuseShaderGL2()
{
    //tex 0 = normal map
    //tex 1 = color map
    //tex 2 = attenuation 3d texture
    //tex 3 = (light filter, depends on light settings)

    qglUseProgramObjectARB(0);

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

void GL_EnableDiffuseSpecularShaderGL2(qboolean world, vec3_t lightOrig)
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

    if (currentshadowlight->filtercube)
    {
	GL_SelectTexture(GL_TEXTURE3_ARB);
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();

        glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, currentshadowlight->filtercube);
        GL_SetupCubeMapMatrix(world);

	qglUseProgramObjectARB(programs[1]);
	checkerror();
    }
    else
    {
	qglUseProgramObjectARB(programs[0]);
	checkerror();
    }

    GL_SelectTexture(GL_TEXTURE0_ARB);
}

void R_DrawWorldGL2DiffuseSpecular(lightcmd_t *lightCmds) 
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



void R_DrawBrushGL2DiffuseSpecular(entity_t *e)
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


void R_DrawAliasFrameGL2DiffuseSpecular (aliashdr_t *paliashdr, aliasframeinstant_t *instant)
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

void R_DrawWorldBumpedGL2()
{
    if (!currentshadowlight->visible)
        return;

    glDepthMask (0);
    glShadeModel (GL_SMOOTH);

    GL_AddColor();
    glColor3fv(&currentshadowlight->color[0]);

    GL_EnableDiffuseSpecularShaderGL2(true,currentshadowlight->origin);
    R_DrawWorldGL2DiffuseSpecular(currentshadowlight->lightCmds);
    GL_DisableDiffuseShaderGL2();

    glColor3f (1,1,1);
    glDisable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask (1);
}

void R_DrawBrushBumpedGL2(entity_t *e)
{
    GL_AddColor();
    glColor3fv(&currentshadowlight->color[0]);

    GL_EnableDiffuseSpecularShaderGL2(false,((brushlightinstant_t *)e->brushlightinstant)->lightpos);
    R_DrawBrushGL2DiffuseSpecular(e);
    GL_DisableDiffuseShaderGL2();
}

void R_DrawAliasBumpedGL2(aliashdr_t *paliashdr, aliasframeinstant_t *instant)
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

    GL_EnableDiffuseSpecularShaderGL2(false,instant->lightinstant->lightpos);
    R_DrawAliasFrameGL2DiffuseSpecular(paliashdr,instant);
    GL_DisableDiffuseShaderGL2();

    if ( gl_truform.value )
    {
        glDisable(GL_PN_TRIANGLES_ATI);
    }
}
