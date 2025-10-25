//
//  FileLoader.cpp
//  AR_Framework
//
//  Created by Hyun Joon Shin on 2021/09/06.
//

#include "FileLoader.hpp"
#include "Model/Texture.hpp"
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>
#ifdef _DEBUG
#pragma comment (lib,"IrrXMLd")
#pragma comment (lib,"zlibstaticd")
#pragma comment (lib,"assimp-vc142-mtd")
#else
#pragma comment (lib,"IrrXML")
#pragma comment (lib,"zlibstatic")
#pragma comment (lib,"assimp-vc142-mt")
#endif

using namespace std;
namespace AR
{

inline vec3 toVec3( const aiVector3D& v ) { return vec3( v.x, v.y, v.z ); }
inline vec4 toVec4( const aiColor4D& c ) { return vec4( c.r, c.g, c.b, c.a ); }
inline vec4 toVec4( const aiColor4D& c, float shin ) { return vec4( c.r, c.g, c.b, shin ); }
inline mat4 toMat4( const aiMatrix4x4& m ) { return mat4( m.a1, m.b1, m.c1, m.d1, m.a2, m.b2, m.c2, m.d2, m.a3, m.b3, m.c3, m.d3, m.a4, m.b4, m.c4, m.d4); }


const vec3 DEF_COLOR_DIFFUSE		= vec3(1,.3,.0);
const vec3 DEF_COLOR_SPECULAR		= vec3(1);
const vec3 DEF_COLOR_EMMISIVE		= vec3(0);
const vec3 DEF_COLOR_TRANSPARENCY	= vec3(0);
const vec3 DEF_COLOR_REFLECTIVE		= vec3(1);

const float DEF_REFLECTIVITY		= 1;
const float DEF_OPACITY				= 1;
const float DEF_SHININESS			= 100;
const float DEF_STRENGTH			= 1;
const float DEF_REFRACTIVE_INDEX	= 1.48f;

vec3 get(const aiMaterial* mtl, const char* pKey,unsigned int type, unsigned int idx, const vec3 def) {
	aiColor4D tmp = aiColor4D(def.r,def.g,def.b, 1);
	mtl->Get( pKey, type, idx, tmp);
	return toVec4( tmp );
}

float get(const aiMaterial* mtl, const char* pKey,unsigned int type, unsigned int idx, float def) {
	float tmp = def;
	mtl->Get( pKey, type, idx, tmp);
	return tmp;
}

static void convertMaterial( Material& mat, const aiMaterial* mtl, const std::string& path, TextureLib& texLib ) {
	aiColor4D tmp;
	aiString file;
	aiGetMaterialString(mtl,AI_MATKEY_NAME,&file);
	mat.name = std::string(file.C_Str());

	vec3 diffuse		= get( mtl, AI_MATKEY_COLOR_DIFFUSE, DEF_COLOR_DIFFUSE );
	vec3 specular		= get( mtl, AI_MATKEY_COLOR_SPECULAR, DEF_COLOR_SPECULAR );
	vec3 emissive		= get( mtl, AI_MATKEY_COLOR_EMISSIVE, DEF_COLOR_EMMISIVE );
	vec3 transparency	= get( mtl, AI_MATKEY_COLOR_TRANSPARENT, DEF_COLOR_TRANSPARENCY );
	vec3 reflective		= get( mtl, AI_MATKEY_COLOR_REFLECTIVE, DEF_COLOR_REFLECTIVE );

	float reflectivity	= get( mtl, AI_MATKEY_REFLECTIVITY, DEF_REFLECTIVITY );
	float opacity		= get( mtl, AI_MATKEY_OPACITY, DEF_OPACITY );
	float shininess		= get( mtl, AI_MATKEY_SHININESS, DEF_SHININESS );
	float strength		= get( mtl, AI_MATKEY_SHININESS_STRENGTH, DEF_STRENGTH );
	float refractiveIdx	= get( mtl, AI_MATKEY_REFRACTI, DEF_REFRACTIVE_INDEX );

	mat.roughness = powf(2/(shininess+2),0.25);
	mat.specColor = specular * strength * reflectivity * reflective;
	mat.diffColor = vec4( diffuse, opacity );
	mat.refIndex  = vec3( refractiveIdx );
	
	if( mtl->GetTexture(aiTextureType_DIFFUSE, 0, &file) == AI_SUCCESS ) {
		mat.diffTexID = texLib.testAndLoadTexture( path+std::string(file.C_Str()), true );
	}
	if( mtl->GetTexture(aiTextureType_SPECULAR, 0, &file) == AI_SUCCESS ) {
		mat.specTexID = texLib.testAndLoadTexture( path+std::string(file.C_Str()), true );
	}
	if( mtl->GetTexture(aiTextureType_EMISSIVE, 0, &file) == AI_SUCCESS ) {
		mat.emissionMapID = texLib.testAndLoadTexture( path+std::string(file.C_Str()), true );
	}
	if( mtl->GetTexture(aiTextureType_HEIGHT, 0, &file) == AI_SUCCESS ) {
		mat.bumpMapID = texLib.testAndLoadTexture( path+std::string(file.C_Str()), true );
	}
	if( mtl->GetTexture(aiTextureType_NORMALS, 0, &file) == AI_SUCCESS ) {
		mat.normMapID = texLib.testAndLoadTexture( path+std::string(file.C_Str()), false );
	}
	if( mtl->GetTexture(aiTextureType_SHININESS, 0, &file) == AI_SUCCESS ) {
		mat.roughnessMapID = texLib.testAndLoadTexture( path+std::string(file.C_Str()), true );
		mat.roughnessMapInverse = true;
	}
	if( mtl->GetTexture(aiTextureType_OPACITY, 0, &file) == AI_SUCCESS ) {
		mat.opacityMapID = texLib.testAndLoadTexture( path+std::string(file.C_Str()), true );
	}
	if( mtl->GetTexture(aiTextureType_DISPLACEMENT, 0, &file) == AI_SUCCESS ) {
		mat.bumpMapID = texLib.testAndLoadTexture( path+std::string(file.C_Str()), true );
	}
	if( mtl->GetTexture(aiTextureType_LIGHTMAP, 0, &file) == AI_SUCCESS ) {
//		mat.normMapID = texLib.testAndLoadTexture( path+std::string(file.C_Str()), false );
	}
	if( mtl->GetTexture(aiTextureType_REFLECTION, 0, &file) == AI_SUCCESS ) {
		mat.specTexID = texLib.testAndLoadTexture( path+std::string(file.C_Str()), true );
	}
	if( mtl->GetTexture(aiTextureType_BASE_COLOR, 0, &file) == AI_SUCCESS ) {
		mat.diffTexID = texLib.testAndLoadTexture( path+std::string(file.C_Str()), true );
	}
	if( mtl->GetTexture(aiTextureType_NORMAL_CAMERA, 0, &file) == AI_SUCCESS ) {
		mat.normMapID = texLib.testAndLoadTexture( path+std::string(file.C_Str()), false );
	}
	if( mtl->GetTexture(aiTextureType_EMISSION_COLOR, 0, &file) == AI_SUCCESS ) {
		mat.emissionMapID = texLib.testAndLoadTexture( path+std::string(file.C_Str()), true );
	}
	if( mtl->GetTexture(aiTextureType_METALNESS, 0, &file) == AI_SUCCESS ) {
		mat.metalnessMapID = texLib.testAndLoadTexture( path+std::string(file.C_Str()), false );
	}
	if( mtl->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &file) == AI_SUCCESS ) {
		mat.roughnessMapID = texLib.testAndLoadTexture( path+std::string(file.C_Str()), false );
		mat.roughnessMapInverse = false;
	}
	if( mtl->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &file) == AI_SUCCESS ) {
		mat.ambOccMatID = texLib.testAndLoadTexture( path+std::string(file.C_Str()), false );
	}
}



Range3 convertMesh( aiMesh* mesh, aiMaterial** materials, const std::string& path, std::vector<TriMesh>& tris, TextureLib& texLib, const mat4& mat ) {
	char filename[1024];
	ofstream fout( filename );
	tris.emplace_back();
	TriMesh& obj = tris.back();
	
	obj.data.verts.resize( mesh->mNumVertices );
	for( size_t t = 0; t < mesh->mNumVertices; ++t) {
		vec3 v = toVec3( mesh->mVertices[t] );
		obj.data.verts[t] = vec3(mat*vec4(v,1));
	}
	
	Range3 rr = {obj.data.verts[0],obj.data.verts[0]};
	for( size_t t=0; t<mesh->mNumVertices; t++ ) {
		rr+=  obj.data.verts[t];
	}
	for (size_t t = 0; t < mesh->mNumFaces; ++t) {
		const aiFace* face = &mesh->mFaces[t];
		for( int k=0; k<int(face->mNumIndices)-2; k++ ) // This only act for vertices
		obj.data.tris.push_back( uvec3( face->mIndices[0],
									   face->mIndices[k+1], face->mIndices[k+2] ));
	}
	if( mesh->HasNormals() ) {
		obj.data.norms.resize( mesh->mNumVertices );
		for( size_t t = 0; t < mesh->mNumVertices; ++t)
		obj.data.norms[t] = toVec3( mesh->mNormals[t] );
	}
	else {
		MeshData::computeNormals( obj.data.verts, obj.data.tris, obj.data.norms );
	}
	// Since GLUK only support single texture coord per vertices.
	if( mesh->HasTextureCoords(0) ) {
		obj.data.tcoords.resize( mesh->mNumVertices );
		for( size_t t = 0; t < mesh->mNumVertices; ++t)
		obj.data.tcoords[t] = toVec3( mesh->mTextureCoords[0][t] );
	}
	
	convertMaterial( obj.material, materials[mesh->mMaterialIndex], path, texLib );
	obj.dataDirty = true;
	return rr;
}


Range3 convertMeshRecursive( aiNode* node, aiMesh** meshes, aiMaterial** materials, const std::string& path, MeshSet& meshSet, TextureLib& texLib, const mat4& parentMat ) {
	Range3 range;
	mat4 mat = parentMat * toMat4( node->mTransformation );
	for( size_t i=0; i<node->mNumMeshes; i++ ) {
		range += convertMesh( meshes[node->mMeshes[i]], materials, path, meshSet, texLib, mat );
	}
	for( size_t i=0; i<node->mNumChildren; i++ ) {
		range+=convertMeshRecursive( node->mChildren[i], meshes, materials, path, meshSet, texLib, mat);
	}
	return range;
}



Range3 loadMesh(const std::string& fn, MeshSet& set, TextureLib& texLib ){
	Range3 range;
	std::string path = getPath( fn );
	Assimp::Logger::LogSeverity severity = Assimp::Logger::NORMAL;
	Assimp::DefaultLogger::create("",severity, aiDefaultLogStream_STDOUT);
	
	aiLogStream stream = aiGetPredefinedLogStream(aiDefaultLogStream_STDOUT,NULL);
	aiAttachLogStream(&stream);
	const aiScene* scene = aiImportFile(fn.c_str(),0);
	if( !scene ) return range;
	range = convertMeshRecursive( scene->mRootNode, scene->mMeshes, scene->mMaterials, path, set, texLib, mat4(1));
//	for( size_t i=0; i<scene->mNumMeshes; i++ ) {
//		Range3 rr = convertMesh( set, scene->mMeshes[i], scene->mMaterials, path, texLib, mat4(1) );
//		range+= rr;
//	}
	//	Object* root = recursiveConvertGraph( scene->mRootNode, meshes );
	aiReleaseImport(scene);
	Assimp::DefaultLogger::kill();
	return range;
}

}


