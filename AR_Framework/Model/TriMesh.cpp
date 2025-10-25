//
//  TriMesh.cpp
//  AR_Framework
//
//  Created by Hyun Joon Shin on 2021/09/06.
//

#include "TriMesh.hpp"

namespace AR {

const int SPHERE_STACK= 63;
const int SPHERE_SLICE= 65;

TriMesh TriMesh::staticQuad;
TriMesh TriMesh::staticSphere;


MeshData MeshData::createSphere(float r) {
	MeshData data;
	int stacks = SPHERE_STACK;
	int slices = SPHERE_SLICE+1;
	data.verts.push_back(vec3(0,1,0)*r);
	data.norms.push_back(vec3(0,1,0));
	data.tcoords.push_back(vec2(0.5,0));
	for( int y = 1; y<stacks; y++ ) {
		float phi = PI/2 - y*PI/(stacks);
		for( int x = 0; x<slices; x++ ) {
			float theta = x*2*PI/(slices-1);
			data.verts.push_back  (vec3( cosf(phi)*sinf(theta), sinf(phi), cosf(phi)*cosf(theta) )*r );
			data.norms.push_back  (vec3( cosf(phi)*sinf(theta), sinf(phi), cosf(phi)*cosf(theta) ) );
			data.tcoords.push_back(vec2(x/(float)(slices-1),y/(float)stacks));
		}
	}
	data.verts.push_back  (vec3(0,-1,0)*r);
	data.norms.push_back  (vec3(0,-1,0));
	data.tcoords.push_back(vec2(0.5,1));
	
	for( int x = 0; x<slices-1; x++ )
	data.tris.push_back(uvec3(0,x+1,x+2));
	for( int y = 0; y<stacks-2; y++ ) {
		for (int x = 0; x < slices-1; x++) {
			data.tris.push_back(uvec3(1 + y * slices + x, 1 + (y+1) * slices + x, 1 + (y+1) * slices + x+1 ));
			data.tris.push_back(uvec3(1 + y * slices + x, 1 + (y+1) * slices + x +1, 1 + y * slices + x+1 ));
		}
	}
	int y = stacks-2;
	int last = 1+(stacks-1)*slices;
	for( int x = 0; x<slices-1; x++ )
	data.tris.push_back(uvec3(1+y*slices+x, last, 1+y*slices+x+1 ));
	return data;
}

MeshData MeshData::createQuad() {
	MeshData data;
	data.verts = {{-1, 1, 0}, {-1, -1, 0}, {1, -1, 0}, {1, 1, 0}};
	data.norms = {{0,0,1},{0,0,1},{0,0,1},{0,0,1}};
	data.tcoords= {{0,0},{0,1},{1,1},{1,0}};
	data.tris = {{0, 1, 2}, {0, 2, 3}};
	return data;
}
MeshData MeshData::createTriangle() {
	MeshData data;
	data.verts = {{0, 1, 0}, {-1, -1, 0}, {1, -1, 0}};
	data.norms = {{0,0,1},{0,0,1},{0,0,1}};
	data.tcoords= {{0.5,0},{0,1},{1,1}};
	data.tris = {{0, 2, 1}};
	return data;
}


}

