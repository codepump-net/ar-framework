//
//  main.cpp
//  AR_Framework
//
//  Created by Hyun Joon Shin on 2021/09/06.
//

#define NANOUI_IMPLEMENTATION

#include <iostream>
#include <algorithm>
#include <cctype>
#include <cmath>
#include "Renderer.hpp"
#include "FileLoader.hpp"
#include "Light.hpp"
#include <GLFW/glfw3.h>
#pragma comment (lib, "glfw3")


AutoLoadProgram renderProg("render.vert","render.frag");


MeshSet meshSet;
TextureLib texLib;
Range3 range;
Renderer* renderer = nullptr;
Light light;

float roughness = 0.5f;
float lightFactor = .5f;
float metallic = 0.0f;
float heightScale = 0.05f;
float aoStrength = 1.0f;
float emissionStrength = 1.0f;
float iblDiffuseIntensity = 0.3f;
float iblSpecularIntensity = 1.0f;

Texture environmentMapTex;
Texture irradianceMapTex;
Texture prefilterMapTex;
Texture brdfLUTTex;
bool environmentMapLoaded = false;
bool irradianceMapLoaded = false;
bool prefilterMapLoaded = false;
bool brdfLUTLoaded = false;
float prefilterMaxLod = 0.0f;

void loadFile( const std::string& fn, bool clearPrev=true ) {
	if( clearPrev ) {
		meshSet.clear();
		texLib.clear();
		range = Range3();
	}
	range += loadMesh( backToFrontSlash(fn), meshSet, texLib);
	printf("Range: %f %f\n", range.minVal.z, range.maxVal.z);
	renderer->setSceneBound(range.minVal, range.maxVal);
	vec3 sceneSize = (range.maxVal - range.minVal);
	vec3 sceneCenter = (range.maxVal + range.minVal)/2.f;
	light.position= sceneCenter + length(sceneSize)*2.f*normalize(vec3(0.5,0.7,1));
	light.color = powf(length(sceneSize)*2.f,2.f)*vec3(1);
}


void initFunc() {
	renderer->ui->add(new nanoSliderF(0,0,200,"Roughness",0,1,roughness));
	renderer->ui->add(new nanoSliderF(0,0,200,"Metallic",0,1,metallic));
	// renderer->ui->add(new nanoSliderF(0,0,200,"Parallax Height",0,0.1f,heightScale));
	// renderer->ui->add(new nanoSliderF(0,0,200,"IBL Diffuse",0,2,iblDiffuseIntensity));
	// renderer->ui->add(new nanoSliderF(0,0,200,"IBL Specular",0,4,iblSpecularIntensity));
	renderer->ui->add(new nanoSliderF(0,0,200,"Light Int.",0.5,10,lightFactor,true));
}

// Precompute LOD range for split-sum prefilter sampling.
static float computePrefilterMaxLod(const Texture& tex) {
	int w = tex.width;
	int h = tex.height;
	if( w<=0 || h<=0 ) return 0.0f;
	int size = std::max(w,h);
	return float(std::max(0, int(std::log2(float(size)))));
}

static std::string toLowerString(std::string s) {
	std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
	return s;
}

// Centralised loader that routes dropped HDR assets to the right IBL target.
void loadEnvironmentTexture( const std::string& fn ) {
	std::string path = backToFrontSlash(fn);
	std::string filename = getFilenameFromAbsPath(path);
	std::string lower = toLowerString(filename);
	if( lower.find("irr") != std::string::npos ) {
		if( irradianceMapTex.load(path, false) ) {
			irradianceMapLoaded = true;
			irradianceMapTex.createGL();
			printf("Loaded irradiance map: %s\n", filename.c_str());
		}
		return;
	}
	if( lower.find("pref") != std::string::npos || lower.find("rough") != std::string::npos ) {
		if( prefilterMapTex.load(path, false) ) {
			prefilterMapLoaded = true;
			prefilterMapTex.createGL();
			prefilterMaxLod = computePrefilterMaxLod(prefilterMapTex);
			printf("Loaded prefiltered environment map: %s (max LOD %.2f)\n", filename.c_str(), prefilterMaxLod);
		}
		return;
	}
	if( lower.find("brdf") != std::string::npos ) {
		if( brdfLUTTex.load(path, false) ) {
			brdfLUTLoaded = true;
			brdfLUTTex.createGL();
			printf("Loaded BRDF LUT: %s\n", filename.c_str());
		}
		return;
	}
	if( environmentMapTex.load(path, false) ) {
		environmentMapLoaded = true;
		environmentMapTex.createGL();
		printf("Loaded environment map: %s\n", filename.c_str());
	}
}

// Utility to bind optional textures while advancing slot counters.
static int bindOptionalTexture(int slot, int texID, const char* uniformName, Program& prog) {
	if( texID < 0 ) return slot;
	texLib[texID].bind(slot, prog, uniformName);
	return slot + 1;
}

// Upload all scene lighting uniforms and IBL resources.
static void setLightingUniforms(Program& prog) {
	light.lightFactor = lightFactor;
	light.setUniform(prog);
	prog.setUniform( "globalMetallic", metallic );
	prog.setUniform( "heightScale", heightScale );
	prog.setUniform( "aoStrength", aoStrength );
	prog.setUniform( "emissionStrength", emissionStrength );
	prog.setUniform( "iblDiffuseIntensity", iblDiffuseIntensity );
	prog.setUniform( "iblSpecularIntensity", iblSpecularIntensity );
	prog.setUniform( "environmentEnabled", environmentMapLoaded?1:0 );
	prog.setUniform( "irradianceEnabled", irradianceMapLoaded?1:0 );
	prog.setUniform( "prefilterEnabled", prefilterMapLoaded?1:0 );
	prog.setUniform( "brdfLUTEnabled", brdfLUTLoaded?1:0 );
	prog.setUniform( "prefilterMaxLod", prefilterMaxLod );

	int iblSlot = 8;
	if( environmentMapLoaded ) environmentMapTex.bind( iblSlot++, prog, "environmentMap" );
	if( irradianceMapLoaded ) irradianceMapTex.bind( iblSlot++, prog, "irradianceMap" );
	if( prefilterMapLoaded ) prefilterMapTex.bind( iblSlot++, prog, "prefilterMap" );
	if( brdfLUTLoaded ) brdfLUTTex.bind( iblSlot++, prog, "brdfLUT" );
}

// Push per-material BRDF parameters and bind supporting textures.
static void uploadMaterial(const Material& mat, Program& prog) {
	prog.setUniform( "baseColor", mat.diffColor );
	prog.setUniform( "specColor", mat.specColor );
	prog.setUniform( "materialRoughness", mat.roughness );
	prog.setUniform( "roughnessMapInverse", mat.roughnessMapInverse?1:0 );
	prog.setUniform( "materialMetallic", 0.0f );	// per-material metalness map overrides this.
}

void renderFunc( Program& prog ) {
	setLightingUniforms(prog);

	for( auto& mesh: meshSet ) {
		const Material& mat = mesh.material;

		// Bind colour / attribute texture set.
		int texSlot = 0;
		prog.setUniform( "diffTexEnabled", mat.diffTexID>=0?1:0 );
		if( mat.diffTexID>=0 )
			texLib[mat.diffTexID].bind( texSlot++, prog, "diffTex" );
		int slot = texSlot;
		slot = bindOptionalTexture(slot, mat.normMapID, "normalMap", prog);
		slot = bindOptionalTexture(slot, mat.roughnessMapID, "roughnessMap", prog);
		slot = bindOptionalTexture(slot, mat.metalnessMapID, "metalnessMap", prog);
		slot = bindOptionalTexture(slot, mat.ambOccMatID, "aoMap", prog);
		slot = bindOptionalTexture(slot, mat.bumpMapID, "heightMap", prog);
		bindOptionalTexture(slot, mat.emissionMapID, "emissionMap", prog);

		prog.setUniform( "normalMapEnabled", mat.normMapID>=0?1:0 );
		prog.setUniform( "roughnessMapEnabled", mat.roughnessMapID>=0?1:0 );
		prog.setUniform( "metalnessMapEnabled", mat.metalnessMapID>=0?1:0 );
		prog.setUniform( "aoMapEnabled", mat.ambOccMatID>=0?1:0 );
		prog.setUniform( "heightMapEnabled", mat.bumpMapID>=0?1:0 );
		prog.setUniform( "emissionMapEnabled", mat.emissionMapID>=0?1:0 );
		prog.setUniform( "roughness", roughness);

		uploadMaterial(mat, prog);

		prog.setUniform( "modelMat", mesh.modelMat );
		mesh.render( prog );
	}
}

void dropFunc( const std::string& fn ) {
	std::string ext = toLowerString( getExtension(fn) );
	if( ext == "hdr" || ext == "exr" ) {
		loadEnvironmentTexture( fn );
	}
	else {
		loadFile( fn );
	}
}

	


int main(int argc, const char * argv[]) {
	if ( !glfwInit() )  {
		printf("FAil\n");
		exit(EXIT_FAILURE);
	}
	
#ifdef __APPLE__
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
	glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, true );
	glfwWindowHint( GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE );
#endif
	
	glfwWindowHint( GLFW_SAMPLES, 32 );
	GLFWwindow* window = glfwCreateWindow( 800, 600, "Hello", NULL, NULL );
	glfwMakeContextCurrent( window );
#ifndef __APPLE__
	glewInit();
#endif

	renderer = new Renderer(window);
	renderer->initFunc = initFunc;
	renderer->renderFunc = renderFunc;
	renderer->dropFunc = dropFunc;

	while ( !glfwWindowShouldClose( window ) ) {
		int fw, fh, ww, wh;
		glfwGetFramebufferSize( window, &fw, &fh );
		glfwGetWindowSize( window, &ww, &wh );


		renderer->render( fw, fh );
		renderer->renderUI(ww,wh,fw,fh);
		glFlush();
		glFinish();
		
		glfwSwapBuffers( window );
		glfwPollEvents();
	}
	glfwDestroyWindow( window );
	glfwTerminate();
	return 0;
}
