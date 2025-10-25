//
//  TriMesh.hpp
//  AR_Framework
//
//  Created by Hyun Joon Shin on 2021/09/06.
//

#ifndef TriMesh_hpp
#define TriMesh_hpp

#include "Tools/gl.hpp"
#include "Tools/Program.hpp"
#include <vector>
#include <tuple>
#include "Material.hpp"

namespace AR {

struct MeshData {
	std::vector<vec3> verts;
	std::vector<vec3> norms;
	std::vector<vec2> tcoords;
	std::vector<uvec3> tris;
	
	MeshData() {}
	MeshData(MeshData& a)
	: verts(a.verts), norms(a.norms), tcoords(a.tcoords), tris(a.tris){}
	MeshData(MeshData&& a)
	: verts(std::move(a.verts)), norms(std::move(a.norms)),
	tcoords(std::move(a.tcoords)), tris(std::move(a.tris)){}
	MeshData& operator = (MeshData&& a) {
		verts = std::move(a.verts);
		norms = std::move(a.norms);
		tcoords = std::move(a.tcoords);
		tris = std::move(a.tris);
		return *this;
	}
	MeshData& operator = (const MeshData& a) {
		verts = a.verts;
		norms = a.norms;
		tcoords = a.tcoords;
		tris = a.tris;
		return *this;
	}
	virtual void clear() {
		verts.clear();
		norms.clear();
		tcoords.clear();
		tris.clear();
	}
	static void computeNormals( const std::vector<vec3>& vertices, const std::vector<ivec3>& ftris, std::vector<vec3>& normals  ) {
		normals.resize( vertices.size(), {0,0,0} );
		for( auto t: ftris ) {
			vec3 v1 = vertices[t.y]-vertices[t.x];
			vec3 v2 = vertices[t.z]-vertices[t.x];
			vec3 n = cross( v1, v2 );
			normals[t.x]+=n;
			normals[t.y]+=n;
			normals[t.z]+=n;
		}
		for( auto& n: normals ) n = normalize( n );
	}
	static void computeNormals( const std::vector<vec3>& vertices, const std::vector<uvec3>& ftris, std::vector<vec3>& normals  ) {
		normals.resize( vertices.size(), {0,0,0} );
		for( auto t: ftris ) {
			vec3 v1 = vertices[t.y]-vertices[t.x];
			vec3 v2 = vertices[t.z]-vertices[t.x];
			vec3 n = cross( v1, v2 );
			normals[t.x]+=n;
			normals[t.y]+=n;
			normals[t.z]+=n;
		}
		for( auto& n: normals ) n = normalize( n );
	}
	virtual void build(const std::vector<vec3>& vertices,
					   const std::vector<vec2>& txcoords,
					   const std::vector<ivec3>& ftris,
					   const std::vector<ivec3>& ttris ) {
		
		std::vector<vec3> normals;
		computeNormals( vertices, ftris, normals );
		
		if( ttris.size()>0 && ttris[0].x>=0 && tcoords.size()>vertices.size() ) {
			verts  .resize( tcoords.size() );
			norms  .resize( tcoords.size() );
			tcoords = txcoords;
			tris   .resize( ttris.size() );
			for( auto i=0; i<ttris.size(); i++ ) {
				verts[ttris[i].x] = vertices[ftris[i].x];
				verts[ttris[i].y] = vertices[ftris[i].y];
				verts[ttris[i].z] = vertices[ftris[i].z];
				norms[ttris[i].x] = normals[ftris[i].x];
				norms[ttris[i].y] = normals[ftris[i].y];
				norms[ttris[i].z] = normals[ftris[i].z];
				tris [i] = ttris[i];
			}
		}
		else {
			verts = vertices;
			norms = normals;
			tris   .resize( ftris.size() );
			if( ttris.size()>0 && ttris[0].x>=0 ) {
				tcoords.resize( vertices.size() );
				for( auto i=0; i<ftris.size(); i++ ) {
					tcoords[ftris[i].x] = txcoords[ttris[i].x];
					tcoords[ftris[i].y] = txcoords[ttris[i].y];
					tcoords[ftris[i].z] = txcoords[ttris[i].z];
					tris[i] = ftris[i];
				}
			}
			else if( txcoords.size() == vertices.size() ){
				tcoords = txcoords;
			}
			else {
				for( auto i=0; i<ftris.size(); i++ ) {
					tris[i] = ftris[i];
				}
			}
		}
	}
	virtual void build( const std::tuple<std::vector<vec3>,std::vector<vec2>,std::vector<ivec3>,
					   std::vector<ivec3>, std::string>& data ) {
		build(std::get<0>(data),std::get<1>(data),std::get<2>(data),std::get<3>(data) );
	}

	
	static MeshData createSphere(float r=1);
	static MeshData createQuad();
	static MeshData createTriangle();
};

struct TriMesh {
	MeshData data;
	bool dataDirty = false;

	GLuint vao = 0, vBuf = 0, eBuf = 0, tBuf = 0, nBuf = 0;
	GLsizei nTris = 0, nVerts = 0;
	
	mat4 modelMat = mat4(1);
	mat3 texMat = mat3(1);
	bool visible = true;
	Material material;
		
	TriMesh()
	: vao(0), vBuf(0), eBuf(0), tBuf(0), nBuf(0), nTris(0), nVerts(0), modelMat(1), texMat(1), material(Material()), visible(true) {}
	
	TriMesh(TriMesh&&a)
	: vao(a.vao), vBuf(a.vBuf), eBuf(a.eBuf), nBuf(a.nBuf), tBuf(a.tBuf), nTris(a.nTris), nVerts(a.nVerts),
	modelMat(a.modelMat), texMat(a.texMat), material(a.material), visible(a.visible),
	data(std::move(a.data)), dataDirty(true) {
		a.dataDirty = false;
		a.vao	= 0;
		a.eBuf	= 0;
		a.tBuf	= 0;
		a.vBuf	= 0;
		a.nBuf	= 0;
	}
	
	virtual void setData( MeshData&& d ) {
		data = std::move(d);
		dataDirty = true;
	}
	virtual void setData( const MeshData& d ) {
		data = d;
		dataDirty = true;
	}
	
	virtual void clear() {
		if( vao ) glDeleteVertexArrays(1, &vao); vao = 0;
		if( vBuf ) glDeleteBuffers( 1, &vBuf ); vBuf = 0;
		if( nBuf ) glDeleteBuffers( 1, &nBuf ); nBuf = 0;
		if( tBuf ) glDeleteBuffers( 1, &tBuf ); tBuf = 0;
		if( eBuf ) glDeleteBuffers( 1, &eBuf ); eBuf = 0;
		data.clear();
		nTris = 0;
		nVerts = 0;
	}
	virtual void createMeshGL() {
		
		if( data.tris.size() == nTris && data.verts.size() == nVerts && vao>0 && eBuf>0 ) {
			printf("Updating mesh\n");
			glBindBuffer( GL_ARRAY_BUFFER, vBuf);
			glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(vec3) * nVerts, data.verts.data() );
			if( data.norms.size()>0 ) {
				glBindBuffer(GL_ARRAY_BUFFER, nBuf);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * nVerts, data.norms.data() );
			}
			
			if( data.tcoords.size()>0 ) {
				glBindBuffer(GL_ARRAY_BUFFER, tBuf);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec2) * nVerts, data.tcoords.data() );
			}
			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, eBuf);
			glBufferSubData( GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(uvec3) * nTris, data.tris.data()  );
		}
		else {
			if( vao ) glDeleteVertexArrays(1, &vao);
			if( vBuf ) glDeleteBuffers(1, &vBuf);
			if( nBuf ) glDeleteBuffers(1, &nBuf);
			if( tBuf ) glDeleteBuffers(1, &tBuf);
			if( eBuf ) glDeleteBuffers(1, &eBuf);
			nTris  = GLsizei(data.tris.size());
			nVerts = GLsizei(data.verts.size());
			
			glGenVertexArrays(1, &vao );
			glBindVertexArray( vao );
			
			glGenBuffers(1, &vBuf);
			glBindBuffer( GL_ARRAY_BUFFER, vBuf);
			glBufferData( GL_ARRAY_BUFFER, sizeof(vec3) * nVerts, data.verts.data(), GL_STATIC_DRAW );
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
			
			if( data.norms.size()>0 ) {
				glGenBuffers(1, &nBuf);
				glBindBuffer(GL_ARRAY_BUFFER, nBuf);
				glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * nVerts, data.norms.data(), GL_STATIC_DRAW);
				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
			}
			
			if( data.tcoords.size()>0 ) {
				glGenBuffers(1, &tBuf);
				glBindBuffer(GL_ARRAY_BUFFER, tBuf);
				glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * nVerts, data.tcoords.data(), GL_STATIC_DRAW);
				glEnableVertexAttribArray(2);
				glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
			}
			glGenBuffers(1, &eBuf);
			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, eBuf);
			glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(uvec3) * nTris, data.tris.data(), GL_STATIC_DRAW );
			glBindVertexArray( 0 );
			
		}
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
		glBindBuffer( GL_ARRAY_BUFFER, 0 );
		data.clear();
		dataDirty = false;
	}
	virtual void render( const Program& program, const mat4& modelMat_=mat4(1) ) {
		if( !visible ) return;
		
		if( vBuf<1 || eBuf<1 || dataDirty ) {
			if( data.verts.size()<1 ) {
				clear();
				return;
			}
			else createMeshGL();
			glErr("Create MeshGL");
		}
		glBindVertexArray( vao );
		program.setUniform( "modelMat", modelMat_*modelMat );
		glErr("set uniform modelMat");
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, eBuf);
		glErr("bind element array buffer");
		glDrawElements(GL_TRIANGLES, nTris*3, GL_UNSIGNED_INT, 0);
		glErr("Draw elements");
		
		glBindVertexArray( 0 );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
		glBindBuffer( GL_ARRAY_BUFFER, 0 );
	}
	
	virtual void build(const std::vector<vec3>& vertices,
					   const std::vector<vec2>& tcoords,
					   const std::vector<ivec3>& ftris,
					   const std::vector<ivec3>& ttris ) {
		data.build(vertices, tcoords, ftris, ttris );
		dataDirty = true;
	}
	virtual void build( const std::tuple<std::vector<vec3>,
					   std::vector<vec2>,
					   std::vector<ivec3>,
					   std::vector<ivec3>,std::string>& dataSet ) {
		data.build( dataSet );
		dataDirty = true;
	}
	
	
	~TriMesh() {
		clear();
	}

	void createTriangle() {
		clear();
		setData( MeshData::createTriangle() );
		dataDirty = true;
	}
	
	void createQuad() {
		clear();
		setData( MeshData::createQuad() );
		dataDirty = true;
	}
	
	void createSphere(float r=1) {
		clear();
		setData( MeshData::createSphere(r) );
		dataDirty = true;
	}

	
	static TriMesh staticQuad;
	static TriMesh staticSphere;
	static void renderQuad( const Program& prog, const mat4& mat=mat4(1) ) {
		if( staticQuad.nVerts<1 ) staticQuad.createQuad();
		staticQuad.render( prog, mat );
	}
	static void renderSphere( const Program& prog, const mat4& mat=mat4(1) ) {
		if( staticSphere.nVerts<1 ) staticSphere.createSphere();
		staticSphere.render( prog, mat );
	}
};

}

#endif /* TriMesh_hpp */
