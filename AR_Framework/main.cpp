//
//  main.cpp
//  AR_Framework
//
//  Created by Hyun Joon Shin on 2021/09/06.
//

#define NANOUI_IMPLEMENTATION

#include <iostream>
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
	renderer->ui->add(new nanoSliderF(0,0,200,"Light Int.",0.5,10,lightFactor,true));
}

void renderFunc( Program& prog ) {
	light.lightFactor = lightFactor;
	light.setUniform(prog);
	for( auto& mesh: meshSet ) {
		int textureID = mesh.material.diffTexID;
		if( textureID>=0 ){
			Texture& tex = texLib[textureID];
			tex.bind( 0, prog, "diffTex" );
		}
//		prog.setUniform( "roughness", mesh.material.roughness);
		prog.setUniform( "roughness", roughness);
		prog.setUniform( "diffTexEnabled", textureID>=0?1:0 );
		prog.setUniform( "modelMat", mesh.modelMat );
		mesh.render( prog );
	}
}

void dropFunc( const std::string& fn ) {
	loadFile( fn );
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
