/*
Copyright (C) 2001-2002 Charles Hollemeersch

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

Generic post processing
Please give us decent rtt on opengl!!

	Post processing is in two fases for tenebrae
	Fase1:
		* Effects like heat haze, gravity warps, refractions, ... they are normal polygons
			rendered after all other polygons, they get acess to the rendered framebuffer
			for stuff like distortions, decolorations...
	Fase2:
		* Effects like glare, ... these get applied after the heat haze effects etc...

*/

#include "quakedef.h"
#include "gl_arbprograms.h"

static int frameBufferTexture;	// Full screen image of the current rendered view (next pow of 2)
static int smallBufferTexture1; // Textures used for downsample post process effects
static int smallBufferTexture2;
static int frameBufferWidth;
static int frameBufferHeight;
static int smallBufferWidth;
static int smallBufferHeight;
static int smallWidth;
static int smallHeight;
typedef struct {
	int vp;
	int fp;
} glProgram_t;


static glProgram_t downSample;
static glProgram_t blur;
static glProgram_t finalCompose;

static cvar_t	gl_lumiMin = {"gl_lumiMin","0.2"};
static cvar_t	gl_lumiMax = {"gl_lumiMax","0.6"};
static cvar_t	gl_glareScale = {"gl_glareScale","1"};
static cvar_t	gl_sceneScale = {"gl_sceneScale","1"};

cvar_t  gl_postprocess = {"gl_postprocess","2"};
cvar_t	post_brightness = {"post_brightness","1.1", true};
cvar_t	post_contrast = {"post_contrast","1.1", true};
cvar_t	post_saturation = {"post_saturation","1.2", true};

qboolean R_CardCanDoPostprocess( void ) {
	return (gl_cardtype == ARB);
}

int R_UsePostprocess( void ) {
	if ( !R_CardCanDoPostprocess() ) return 0;
	return gl_postprocess.value;
}

void R_InitPostProcess( void ) {
	void *rubbish;

	// These always need to be registered because they are used by the options menu
	Cvar_RegisterVariable (&post_brightness);	
	Cvar_RegisterVariable (&post_saturation);	
	Cvar_RegisterVariable (&post_contrast);	

	//We will never do postprocessing, save some texture memory
	if ( !R_CardCanDoPostprocess() ) {
		gl_postprocess.value = 0.0f;	
		return;
	}

	//Allocate textures
	frameBufferTexture = texture_extension_number;
	texture_extension_number++;
	smallBufferTexture1 = texture_extension_number;
	texture_extension_number++;
	smallBufferTexture2 = texture_extension_number;
	texture_extension_number++;

	for (frameBufferWidth = 1; frameBufferWidth < vid.width; frameBufferWidth<<=1);
	for (frameBufferHeight = 1; frameBufferHeight < vid.height; frameBufferHeight<<=1);

	smallWidth = vid.width/4;
	smallHeight = vid.height/4;

	for (smallBufferWidth = 1; smallBufferWidth < smallWidth; smallBufferWidth<<=1);
	for (smallBufferHeight = 1; smallBufferHeight < smallHeight; smallBufferHeight<<=1);

	//Upoad rubbish
	rubbish = malloc( frameBufferWidth* frameBufferHeight * 4 );
	memset( rubbish, 0, frameBufferWidth* frameBufferHeight * 4 );

	GL_Bind ( frameBufferTexture );
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA8, frameBufferWidth, frameBufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, rubbish);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

	GL_Bind ( smallBufferTexture1 );
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA8, smallBufferWidth, smallBufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, rubbish);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

	GL_Bind ( smallBufferTexture2 );
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA8, smallBufferWidth, smallBufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, rubbish);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

	//Load shaders
	downSample.vp = GL_LoadShader( GL_VERTEX_PROGRAM_ARB, "glprogs/downsample_vp.txt" );
	downSample.fp = GL_LoadShader( GL_FRAGMENT_PROGRAM_ARB, "glprogs/downsample_fp.txt" );
	
	blur.vp = GL_LoadShader( GL_VERTEX_PROGRAM_ARB, "glprogs/blur_vp.txt" );
	blur.fp = GL_LoadShader( GL_FRAGMENT_PROGRAM_ARB, "glprogs/blur_fp.txt" );
	
	finalCompose.vp = GL_LoadShader( GL_VERTEX_PROGRAM_ARB, "glprogs/final_vp.txt" );
	finalCompose.fp = GL_LoadShader( GL_FRAGMENT_PROGRAM_ARB, "glprogs/final_fp.txt" );

	Cvar_RegisterVariable (&gl_lumiMin);
	Cvar_RegisterVariable (&gl_lumiMax);	
	Cvar_RegisterVariable (&gl_sceneScale);	
	Cvar_RegisterVariable (&gl_glareScale);	
	Cvar_RegisterVariable (&gl_postprocess);
}

void R_PostProcessFase1( void ) {

	if (R_UsePostprocess() != 2) return;

	GL_Bind( frameBufferTexture );
	glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0, 0, frameBufferWidth, frameBufferHeight );
}

static void FullScreenQuad() {

	float texX = (float)vid.width / (float)frameBufferWidth;
	float texY = (float)vid.height / (float)frameBufferHeight;

	glBegin (GL_QUADS);
	glTexCoord2f(0,1);
	glVertex3f (0.1f, 1, 1);
	glTexCoord2f(1,1);
	glVertex3f (0.1f, -1, 1);
	glTexCoord2f(1,0);
	glVertex3f (0.1f, -1, -1);
	glTexCoord2f(0,0);
	glVertex3f (0.1f, 1, -1);
	glEnd ();
}

extern cshift_t cshift_slime;
extern cshift_t cshift_lava;

float SetupForScreenFlash( void ) {
	float blur = 1.0; // We modulate this down then invert at the end

	//No flashing
	if (!v_blend[3]) {
		glColor4f ( 1.0f, 1.0f, 1.0f, 1.0f );
		return 0.0f;
	}

	//Some fluids and
	if ( memcmp( &cl.cshifts[CSHIFT_CONTENTS], &cshift_slime, sizeof(cshift_t) ) == 0 ) {
		blur *= 0.25;
	} else if ( memcmp( &cl.cshifts[CSHIFT_CONTENTS], &cshift_lava, sizeof(cshift_t) ) == 0 ) {
		blur *= 0.5;
	}

	//Damage blurs view
	blur *= (1.0f-(cl.cshifts[CSHIFT_DAMAGE].percent / 150.0f));
	blur *= (1.0f-(cl.cshifts[CSHIFT_DAMAGE].percent / 150.0f));

	glColor4fv ( v_blend );
	return 1.0f-blur;
}

void R_PostProcessFase2( void ) {
	float blurFactor;
	if (!R_UsePostprocess()) return;

	GL_Bind( frameBufferTexture );
	glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0, 0, frameBufferWidth, frameBufferHeight );

	//Setup opengl state
	glDisable (GL_BLEND);
	glDisable (GL_DEPTH_TEST);

	glLoadIdentity ();
	glRotatef (-90,  1, 0, 0);	    // put Z going up
	glRotatef (90,  0, 0, 1);	    // put Z going up

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity ();
	glMatrixMode(GL_MODELVIEW);

	glEnable( GL_VERTEX_PROGRAM_ARB );
	glEnable( GL_FRAGMENT_PROGRAM_ARB );

	//Blurring is easy by just playing with the values a bit (they can go over 1.0)	
	blurFactor = SetupForScreenFlash();

	qglProgramEnvParameter4fARB( GL_VERTEX_PROGRAM_ARB, 0,
		1.0f / (float)frameBufferWidth, 1.0f / (float)frameBufferHeight, 0, 0 ); 
	qglProgramEnvParameter4fARB( GL_VERTEX_PROGRAM_ARB, 1,
		1.0f / (float)smallBufferWidth, 1.0f / (float)smallBufferHeight, 0, 0 ); 
	qglProgramEnvParameter4fARB( GL_VERTEX_PROGRAM_ARB, 2,
		(float)vid.width / (float)frameBufferWidth, (float)vid.height / (float)frameBufferHeight, 0, 0 ); 
	qglProgramEnvParameter4fARB( GL_VERTEX_PROGRAM_ARB, 3,
		(float)smallWidth / (float)smallBufferWidth, (float)smallHeight / (float)smallBufferHeight, 0, 0 ); 
	qglProgramEnvParameter4fARB( GL_FRAGMENT_PROGRAM_ARB, 0,
		gl_lumiMin.value, 1.0f / ( gl_lumiMax.value - gl_lumiMin.value ) , gl_sceneScale.value * (1-blurFactor), gl_glareScale.value ); 

	// Downscale and other fancy stuff here

	GL_Bind( frameBufferTexture );
	qglBindProgramARB( GL_VERTEX_PROGRAM_ARB, downSample.vp );
	qglBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, downSample.fp );
	FullScreenQuad();

	GL_Bind( smallBufferTexture1 );
	glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0, 0, smallBufferWidth, smallBufferHeight ); 

	// Blur
	GL_Bind( smallBufferTexture1 );
	qglBindProgramARB( GL_VERTEX_PROGRAM_ARB, blur.vp );
	qglBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, blur.fp );
	qglProgramLocalParameter4fARB( GL_VERTEX_PROGRAM_ARB, 0, 0.0f, 1.0f, 0.0f, 0.0f );
	FullScreenQuad();
	
	glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0, 0, smallBufferWidth, smallBufferHeight ); 

	qglProgramLocalParameter4fARB( GL_VERTEX_PROGRAM_ARB, 0, 1.0f, 0.0f, 0.0f, 0.0f );
	FullScreenQuad();

	glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0, 0, smallBufferWidth, smallBufferHeight ); 

	// Final compositing pass
	GL_Bind( frameBufferTexture );
	GL_SelectTexture(GL_TEXTURE1_ARB);
	GL_Bind( smallBufferTexture1 );
	GL_SelectTexture(GL_TEXTURE0_ARB);

	qglBindProgramARB( GL_VERTEX_PROGRAM_ARB, finalCompose.vp );
	qglBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, finalCompose.fp );
	qglProgramLocalParameter4fARB( GL_FRAGMENT_PROGRAM_ARB, 0,
		post_saturation.value, post_contrast.value, post_brightness.value, blurFactor * gl_sceneScale.value ); 

	FullScreenQuad();

	// Cleanup opengl state

	glDisable( GL_VERTEX_PROGRAM_ARB );
	glDisable( GL_FRAGMENT_PROGRAM_ARB );

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable (GL_ALPHA_TEST);
}

void R_DrawMenuPostProcess( void ) {
	float blurFactor;
	if (!R_UsePostprocess()) return;

	//Setup opengl state
	glDisable ( GL_ALPHA_TEST );

	glPushMatrix();
	glLoadIdentity ();
	glRotatef (-90,  1, 0, 0);	    // put Z going up
	glRotatef (90,  0, 0, 1);	    // put Z going up

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity ();

	glEnable( GL_VERTEX_PROGRAM_ARB );
	glEnable( GL_FRAGMENT_PROGRAM_ARB );

	qglProgramEnvParameter4fARB( GL_VERTEX_PROGRAM_ARB, 0,
		1.0f / (float)frameBufferWidth, 1.0f / (float)frameBufferHeight, 0, 0 ); 
	qglProgramEnvParameter4fARB( GL_VERTEX_PROGRAM_ARB, 1,
		1.0f / (float)smallBufferWidth, 1.0f / (float)smallBufferHeight, 0, 0 ); 
	qglProgramEnvParameter4fARB( GL_VERTEX_PROGRAM_ARB, 2,
		(float)vid.width / (float)frameBufferWidth, (float)vid.height / (float)frameBufferHeight, 0, 0 ); 
	qglProgramEnvParameter4fARB( GL_VERTEX_PROGRAM_ARB, 3,
		(float)smallWidth / (float)smallBufferWidth, (float)smallHeight / (float)smallBufferHeight, 0, 0 ); 
	qglProgramEnvParameter4fARB( GL_FRAGMENT_PROGRAM_ARB, 0,
		gl_lumiMin.value, 1.0f / ( gl_lumiMax.value - gl_lumiMin.value ) , 0.2 , gl_glareScale.value ); 

	// Final compositing pass
	GL_Bind( frameBufferTexture );
	GL_SelectTexture(GL_TEXTURE1_ARB);
	GL_Bind( smallBufferTexture1 );
	GL_SelectTexture(GL_TEXTURE0_ARB);

	qglBindProgramARB( GL_VERTEX_PROGRAM_ARB, finalCompose.vp );
	qglBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, finalCompose.fp );
	qglProgramLocalParameter4fARB( GL_FRAGMENT_PROGRAM_ARB, 0,
		post_saturation.value, post_contrast.value, post_brightness.value, gl_sceneScale.value*0.8 ); 

	FullScreenQuad();

	// Cleanup opengl state

	glDisable( GL_VERTEX_PROGRAM_ARB );
	glDisable( GL_FRAGMENT_PROGRAM_ARB );

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glColor4f(1.0,1.0,1.0,1.0);
	glEnable (GL_ALPHA_TEST);
}

int R_GetPostProcessTexture( void ) {
	return frameBufferTexture;
}

static float scale_matrix[16] = {
		0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0
};

float * R_GetPostProcessMatrix( void ) {
	scale_matrix[0] = (float)vid.width / (float)frameBufferWidth;
	scale_matrix[5] = (float)vid.height / (float)frameBufferHeight;
	return scale_matrix;
}