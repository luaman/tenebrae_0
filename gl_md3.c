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

  Md3 support
*/

#include "quakedef.h"

#define MD3_VERSION			15
#define	MD3_XYZ_SCALE		(1.0/64)

typedef struct md3Frame_s {
	vec3_t		bounds[2];
	vec3_t		localOrigin;
	float		radius;
	char		name[16];
} md3Frame_t;

typedef struct {
	char			name[MAX_QPATH];
	int				shaderIndex;
} md3Shader_t;

typedef struct {
	int		ident;				// 

	char	name[MAX_QPATH];	// polyset name

	int		flags;
	int		numFrames;			// all surfaces in a model should have the same

	int		numShaders;			// all surfaces in a model should have the same
	int		numVerts;

	int		numTriangles;
	int		ofsTriangles;

	int		ofsShaders;			// offset from start of md3Surface_t
	int		ofsSt;				// texture coords are common for all frames
	int		ofsXyzNormals;		// numVerts * numFrames

	int		ofsEnd;				// next surface follows
} md3Surface_t;

typedef struct {
	int			ident;
	int			version;

	char		name[MAX_QPATH];

	int			flags;

	int			numFrames;
	int			numTags;			
	int			numSurfaces;

	int			numSkins;

	int			ofsFrames;
	int			ofsTags;
	int			ofsSurfaces;
	int			ofsEnd;
} md3Header_t;

typedef struct {
	short		xyz[3];
	unsigned short normal;
} md3XyzNormal_t;

typedef struct {
	int			indexes[3];
} md3Triangle_t;

typedef struct {
	float		s;
	float		t;
} md3St_t;


int findneighbourmd3_old(int index, int edgei, int numtris, mtriangle_t *triangles) {
	int i, j;
	mtriangle_t *current = &triangles[index];

	for (i=0; i<numtris; i++) {
		if (i == index) continue;
		for (j=0; j<3; j++) {      
			if (((current->vertindex[edgei] == triangles[i].vertindex[j]) 
			    && (current->vertindex[(edgei+1)%3] == triangles[i].vertindex[(j+1)%3]))
				||
			   ((current->vertindex[edgei] == triangles[i].vertindex[(j+1)%3]) 
			    && (current->vertindex[(edgei+1)%3] == triangles[i].vertindex[j])))
			{
				triangles[i].neighbours[j] = index;
				return i;
			}
  
		}
	}
	return -1;
}
/*
Yet another hack.  Some models seem to have edges shared between three triangles, this is obviously
a strange thing to have, we resolve it simply by throwing away that shared egde and giving all
triangles a "-1" neighbour for that edge.  This will give some unneeded fins for some edges of some models
but this number is generally verry low (< 3 edges per model) and only on a few models.
*/
int findneighbourmd3(int index, int edgei, int numtris, mtriangle_t *triangles) {
	int i, j, v1, v0, found,foundj = 0;
	mtriangle_t *current = &triangles[index];
	mtriangle_t *t;
	qboolean	dup;

	v0 = current->vertindex[edgei];
	v1 = current->vertindex[(edgei+1)%3];

	//XYZ
	found = -1;
	dup = false;
	for (i=0; i<numtris; i++) {
		if (i == index) continue;
		t = &triangles[i];

		for (j=0; j<3; j++) {      
			if (((current->vertindex[edgei] == triangles[i].vertindex[j]) 
			    && (current->vertindex[(edgei+1)%3] == triangles[i].vertindex[(j+1)%3]))
				||
			   ((current->vertindex[edgei] == triangles[i].vertindex[(j+1)%3]) 
			    && (current->vertindex[(edgei+1)%3] == triangles[i].vertindex[j])))
			{
				//no edge for this model found yet?
				if (found == -1) {
					found = i;
					foundj = j;
				}
				//the three edges story
				else
					dup = true;
			}
  
		}
	}

	//normal edge, setup neighbour pointers
	if (!dup) {
		if (found != -1)
			triangles[found].neighbours[foundj] = index;
		return found;
	}
	//naughty egde let no-one have the neighbour
	Con_Printf("%s: warning: open edge added\n",loadname);
	return -1;
}

void TangentForTrimd3(mtriangle_t *tri, vec3_t norm, ftrivertx_t *verts, fstvert_t *texcos, vec3_t res) {

	vec3_t	vec1, vec2, dirv, tz;
	float	delta1, delta2, t;
	vec3_t	*v[3];
	float	st[3][2];
	int		j;

	for (j=0; j<3; j++) {
		v[j] = &verts[tri->vertindex[j]].v;

		st[j][0] = texcos[tri->vertindex[j]].s;
		st[j][1] = texcos[tri->vertindex[j]].t;
	}

	vec1[0] = *v[1][0] - *v[0][0];
	vec1[1] = *v[1][1] - *v[0][1];
	vec1[2] = *v[1][2] - *v[0][2];
	delta1 = st[1][0] - st[0][0];
      
	vec2[0] = *v[2][0] - *v[0][0];
	vec2[1] = *v[2][1] - *v[0][1];
	vec2[2] = *v[2][2] - *v[0][2];
	delta2 = st[2][0] - st[0][0];

	if  ((!delta1) && (!delta2)) Con_Printf("%s: warning: Degenerate tangent space\n",loadname);


	dirv[0] = (delta1 * vec2[0] - vec1[0] * delta2);
	dirv[1] = (delta1 * vec2[1] - vec1[1] * delta2);
	dirv[2] = (delta1 * vec2[2] - vec1[2] * delta2);

	VectorNormalize(dirv);

	VectorCopy(norm,tz);
	t = dirv[0]*tz[0]+dirv[1]*tz[1]+dirv[2]*tz[2];
	res[0] = dirv[0]-t*tz[0];
	res[1] = dirv[1]-t*tz[1];
	res[2] = dirv[2]-t*tz[2];
}

/*
=================
Mod_LoadMd3Model

PENTA: Very similar to LoadAliasModel
=================
*/
#define	LL(x) x=LittleLong(x)

// <AWE> added following macros
#if defined (__APPLE__) || defined (MACOSX)
#define min(A,B)	((A) < (B) ? (A) : (B))
#define max(A,B)	((A) > (B) ? (A) : (B))
#endif /* __APPLE__ || MACOSX */

void Mod_LoadMd3Model (model_t *mod, void *buffer)
{
	int					i, j, k, l;
	md3Header_t			*pinmodel;
	int					size;
	int					start, end, total;
	md3Surface_t		*surf;
	vec3_t				md3scale = {MD3_XYZ_SCALE, 	MD3_XYZ_SCALE, 	MD3_XYZ_SCALE};
	vec3_t				md3origin = {0.0f, 0.0f, 0.0f};
	ftrivertx_t			*verts, *v;
	md3XyzNormal_t		*xyz;
	mtriangle_t			*tris;
	md3Triangle_t		*tri;
	md3St_t				*st;
	fstvert_t			*texcoords;
	md3Frame_t			*frame;
	plane_t				*norms;
	int					*indecies;
	vec3_t				v1, v2, normal;
	vec3_t				triangle[3];
	vec3_t				*tangents;
	md3Shader_t			*shader;
	byte				fake[16];
	char				shadername[MAX_QPATH];

	start = Hunk_LowMark ();

	//Con_Printf("Loading md3 from %s\n",mod->name);
	pinmodel = (md3Header_t *)buffer;

//
// endian-adjust and copy the data, starting with the md3 header
//


	LL(pinmodel->version);
	if (pinmodel->version != MD3_VERSION)
		Sys_Error ("%s has wrong version number (%i should be %i)",
				 mod->name, pinmodel->version, MD3_VERSION);

	//swap header
	LL(pinmodel->numFrames);
	LL(pinmodel->numTags);			
	LL(pinmodel->numSurfaces);

	LL(pinmodel->numSkins);

	LL(pinmodel->ofsFrames);
	LL(pinmodel->ofsTags);
	LL(pinmodel->ofsSurfaces);
	LL(pinmodel->ofsEnd);

	if ( pinmodel->numFrames < 1 ) {
		Con_Printf( "LoadMd3Model: %s has no frames\n", mod->name );
		return;
	}

	if ( pinmodel->numFrames > MAXALIASFRAMES) {
		Sys_Error ("LoadMd3Model: %s has too many frames",mod->name);
	}

	// swap all the frames
    frame = (md3Frame_t *) ( (byte *)pinmodel + pinmodel->ofsFrames );
    for ( i = 0 ; i < pinmodel->numFrames ; i++, frame++) {
    	frame->radius = LittleFloat( frame->radius );
        for ( j = 0 ; j < 3 ; j++ ) {
            frame->bounds[0][j] = LittleFloat( frame->bounds[0][j] );
            frame->bounds[1][j] = LittleFloat( frame->bounds[1][j] );
	    	frame->localOrigin[j] = LittleFloat( frame->localOrigin[j] );
        }
	}

	// swap all the surfaces
	surf = (md3Surface_t *) ( (byte *)pinmodel + pinmodel->ofsSurfaces );

	if (pinmodel->numSurfaces > 1) {
		Con_Printf("%s: warning: Model with multiple surfaces\n",mod->name);
	}

	for ( i = 0 ; i < pinmodel->numSurfaces ; i++) {

        LL(surf->ident);
        LL(surf->flags);
        LL(surf->numFrames);
        LL(surf->numShaders);
        LL(surf->numTriangles);
        LL(surf->ofsTriangles);
        LL(surf->numVerts);
        LL(surf->ofsShaders);
        LL(surf->ofsSt);
        LL(surf->ofsXyzNormals);
        LL(surf->ofsEnd);
		
		if ( surf->numVerts > MAXALIASVERTS)
			Sys_Error ("LoadMd3Model: %s has too many vertices",mod->name);

		if (surf->numTriangles <= 0)
			Sys_Error ("LoadMd3Model: %s has no triangles", mod->name);

		if (surf->numTriangles > MAXALIASTRIS)
			Sys_Error ("LoadMd3Model: %s has too many triangles",mod->name);

		// swap all the triangles
		tri = (md3Triangle_t *) ( (byte *)surf + surf->ofsTriangles );
		for ( j = 0 ; j < surf->numTriangles ; j++, tri++ ) {
			LL(tri->indexes[0]);
			LL(tri->indexes[1]);
			LL(tri->indexes[2]);
		}

		// swap all the ST
        st = (md3St_t *) ( (byte *)surf + surf->ofsSt );
        for ( j = 0 ; j < surf->numVerts ; j++, st++ ) {
            st->s = LittleFloat( st->s);
            st->t = LittleFloat( st->t);
        }

		// swap all the XyzNormals
        xyz = (md3XyzNormal_t *) ( (byte *)surf + surf->ofsXyzNormals );
        for ( j = 0 ; j < surf->numVerts * surf->numFrames ; j++, xyz++ ) 
		{
            xyz->xyz[0] = LittleShort( xyz->xyz[0] );
            xyz->xyz[1] = LittleShort( xyz->xyz[1] );
            xyz->xyz[2] = LittleShort( xyz->xyz[2] );

            xyz->normal = LittleShort( xyz->normal );
        }


		// find the next surface
		surf = (md3Surface_t *)( (byte *)surf + surf->ofsEnd );
	}

	//point to first surface again
	surf = (md3Surface_t *) ( (byte *)pinmodel + pinmodel->ofsSurfaces );

//
// We have now a working version of the md3 in the "*buffer" now convert that to an "alias" model
// this conversion is not to bad sice the I changed the way alias models work to make them more
// quake3 friendly, the only thing that remains is that we only use the first surface of the  model.
//
	//Alocate hunk mem for the header and the frame info (not the actual frame vertices)
	size = 	sizeof (aliashdr_t) + (pinmodel->numFrames-1) * sizeof (maliasframedesc_t);
	pheader = Hunk_AllocName (size, mod->name);

	//Convert the header to the old header
	pheader->ident = pinmodel->ident;
	pheader->version = pinmodel->version;
	VectorCopy(md3scale,pheader->scale);
	VectorCopy(md3origin,pheader->scale_origin);
	pheader->boundingradius = 100; //This seems not used anymore by quake
	VectorCopy(md3origin,pheader->eyeposition);//This seems not used anymore by quake
	pheader->numskins = 1; //Hacked value
	pheader->skinwidth = 255;//Hacked value
	pheader->skinheight = 255;//Hacked value
	pheader->numverts = surf->numVerts;
	pheader->numtris = surf->numTriangles;
	pheader->numframes = pinmodel->numFrames;
	pheader->synctype = ST_SYNC;
	pheader->flags = 0;//Hacked value
	pheader->size = 1;//All right, the unofficial specs say the average size of triangles, so we just put something there
	pheader->numposes = pinmodel->numFrames;
	pheader->poseverts = surf->numVerts;

	mod->mins[0] = mod->mins[1] = mod->mins[2] =  99999.0;
	mod->maxs[0] = mod->maxs[1] = mod->maxs[2] = -99999.0; 
	mod->flags = 0;
	mod->type = mod_alias;
	mod->numframes = pheader->numframes;
	mod->synctype = pheader->synctype;

	//Convert the frames
	frame = (md3Frame_t *) ( (byte *)pinmodel + pinmodel->ofsFrames );
	for (i=0; i<pheader->numframes; i++, frame++) {
		strcpy (pheader->frames[i].name, frame->name);

		//setup correct bounding box
		for (j=0; j<3; j++) {
			mod->mins[j] = min(mod->mins[j],frame->bounds[0][j]);
			mod->maxs[j] = max(mod->maxs[j],frame->bounds[1][j]);
		}

		pheader->frames[i].firstpose = i;
		pheader->frames[i].numposes = 1;
		pheader->frames[i].frame = i;
		pheader->frames[i].interval = 0.1f;

		//bounding box for frame
		pheader->frames[i].bboxmin.v[0] = (byte)frame->bounds[0][0]/pheader->scale[0];
		pheader->frames[i].bboxmin.v[1] = (byte)frame->bounds[0][1]/pheader->scale[1];
		pheader->frames[i].bboxmin.v[2] = (byte)frame->bounds[0][2]/pheader->scale[2];
		pheader->frames[i].bboxmax.v[0] = (byte)frame->bounds[1][0]/pheader->scale[0];
		pheader->frames[i].bboxmax.v[1] = (byte)frame->bounds[1][1]/pheader->scale[1];
		pheader->frames[i].bboxmax.v[2] = (byte)frame->bounds[1][2]/pheader->scale[2];
	}
	//Con_Printf("%s: %f,%f,%f %f,%f,%f\n",loadname,mod->mins[0],mod->mins[1],mod->mins[2],mod->maxs[0],mod->maxs[1],mod->maxs[2]);

	//calculate radius
	mod->radius = max(Length(mod->mins),Length(mod->maxs));

	//Convert the vertices
	verts = Hunk_Alloc (pheader->numposes * pheader->poseverts * sizeof(ftrivertx_t) );
	pheader->posedata = (byte *)verts - (byte *)pheader;
	xyz = (md3XyzNormal_t *) ( (byte *)surf + surf->ofsXyzNormals );
	for (i=0; i<pheader->numposes; i++) {
		for (j=0; j<pheader->poseverts; j++) {
			k = i*pheader->poseverts+j;
			verts[k].v[0] = xyz[k].xyz[0]*MD3_XYZ_SCALE;
			verts[k].v[1] = xyz[k].xyz[1]*MD3_XYZ_SCALE;
			verts[k].v[2] = xyz[k].xyz[2]*MD3_XYZ_SCALE;
			verts[k].lightnormalindex = xyz[k].normal;
		}
	}

	//Convert the triangles
	tris = Hunk_Alloc (pheader->numtris * sizeof(mtriangle_t));
	pheader->triangles = (byte *)tris - (byte *)pheader;
	tri = (md3Triangle_t *) ( (byte *)surf + surf->ofsTriangles );
	for (i=0; i<pheader->numtris; i++) {
		for (j=0; j<3; j++) {
			tris[i].vertindex[j] = tri[i].indexes[j];
			tris[i].neighbours[j] = -1;
		}
		tris[i].facesfront = true; //doesn't matter
	}

	//Setup connectivity
	for (i=0; i<pheader->numtris; i++)
		for (j=0 ; j<3 ; j++) {
			//none found yet 
			if (tris[i].neighbours[j] == -1) {
				tris[i].neighbours[j] = findneighbourmd3(i, j, pheader->numtris, tris);
			}
		}

	//Calculate plane equatations
	norms = Hunk_Alloc (pheader->numtris * pheader->numposes * sizeof(plane_t));
	pheader->planes = (byte *)norms - (byte *)pheader;
	for (i=0; i<pheader->numposes; i++) {
		for (j=0; j<pheader->numtris ; j++) {

			//make 3 vec3_t's of this triangle's vertices
			for (k=0; k<3; k++) {
				v = &verts[i*pheader->poseverts + tris[j].vertindex[k]];
				for (l=0; l<3; l++)
					triangle[k][l] = v->v[l];		
			}

			//calculate their normal
			VectorSubtract(triangle[0], triangle[1], v1);
			VectorSubtract(triangle[2], triangle[1], v2);
			CrossProduct(v2,v1, normal);		
			VectorScale(normal, 1/Length(normal), norms[i*pheader->numtris+j].normal);
			
			//distance of plane eq
			norms[i*pheader->numtris+j].dist = DotProduct(triangle[0],norms[i*pheader->numtris+j].normal);
		}
	}

	//Convert texcoords for triangles
	texcoords = Hunk_Alloc (pheader->poseverts * sizeof(fstvert_t));
	pheader->texcoords = (byte *)texcoords - (byte *)pheader;
	st = (md3St_t *) ( (byte *)surf + surf->ofsSt );
	for (i=0; i<pheader->poseverts ; i++) {
		texcoords[i].s = st[i].s;
		texcoords[i].t = st[i].t;
	}

	//Create index lists
	indecies = Hunk_Alloc (pheader->numtris * sizeof(int) * 3);
	pheader->indecies = (byte *)indecies - (byte *)pheader;
	for (i=0 ; i<pheader->numtris ; i++) {
		for (j=0 ; j<3 ; j++) {
			//Throw vertex index into our index array
			(*indecies) = tris[i].vertindex[j];
			indecies++;
		}
	}

	//Calculate tangents for vertices
	tangents = Hunk_Alloc (pheader->poseverts * pheader->numposes * sizeof(vec3_t));
	pheader->tangents = (byte *)tangents - (byte *)pheader;
	//for all frames
	for (i=0; i<pheader->numposes; i++) {

		//set temp to zero
		for (j=0; j<pheader->poseverts; j++) {
			tangents[i*pheader->poseverts+j][0] = 0;
			tangents[i*pheader->poseverts+j][1] = 0;
			tangents[i*pheader->poseverts+j][2] = 0;
			numNormals[j] = 0;
		}

		
		//for all tris
		for (j=0; j<pheader->numtris; j++) {
			vec3_t tangent;
			TangentForTrimd3(&tris[j],norms[i*pheader->numtris+j].normal,
						  &verts[i*pheader->poseverts],texcoords,tangent);
			//for all vertices in the tri
			for (k=0; k<3; k++) {
				l = tris[j].vertindex[k];
				VectorAdd(tangents[i*pheader->poseverts+l],tangent,
							tangents[i*pheader->poseverts+l]); 
				numNormals[l]++;
			}
		}
		
		//calculate average
		for (j=0; j<pheader->poseverts; j++) {
			if (!numNormals[j]) continue;
			tangents[i*pheader->poseverts+j][0] = tangents[i*pheader->poseverts+j][0]/numNormals[j];
			tangents[i*pheader->poseverts+j][1] = tangents[i*pheader->poseverts+j][1]/numNormals[j];
			tangents[i*pheader->poseverts+j][2] = tangents[i*pheader->poseverts+j][2]/numNormals[j];
			VectorNormalize(tangents[i*pheader->poseverts+j]);
		}
	}


	//Load skins
	fake[0] = 0;
	shader = (md3Shader_t *) ( (byte *)surf + surf->ofsShaders );
	COM_FileBase (shader->name, shadername);
	pheader->gl_texturenum[0][0] =
	pheader->gl_texturenum[0][1] =
	pheader->gl_texturenum[0][2] =
	pheader->gl_texturenum[0][3] = GL_LoadTexture (shadername, 4, 4, &fake[0], true, false, true);
	//Con_Printf("Load shader %s\n",shader->name);


//
// move the complete, relocatable alias model to the cache
//	
	end = Hunk_LowMark ();
	total = end - start;
	
	Cache_Alloc (&mod->cache, total, loadname);
	if (!mod->cache.data)
		return;
	memcpy (mod->cache.data, pheader, total);

	Hunk_FreeToLowMark (start);
}