//
//  Renderer.hpp
//  AR_Framework
//
//  Created by Hyun Joon Shin on 2021/09/06.
//

#ifndef Renderer_h
#define Renderer_h

#define GLFW_INCLUDE_NONE
#define NANOVG_GL3
#include <functional>
#include "Camera.hpp"
#include "Model/TriMesh.hpp"
#include "Model/Texture.hpp"
#include "Model/Material.hpp"
#include <GLFW/glfw3.h>
#include <nanoUI.hpp>

using namespace AR;

extern AutoLoadProgram renderProg;

struct Renderer {
	vec2 mousePt;
	bool initialized = false;
	Camera camera;
	GLFWwindow* window = nullptr;
	vec4 clearColor = vec4(.3,.3,.5,1);
	std::function<void()> initFunc = [](){};
	std::function<void(const std::string& fn)> dropFunc = [](const std::string&){};
	std::function<void(int key)> keyFunc = [](int key){};
	std::function<void(int button)> pushFunc = [](int button){};
	std::function<void(int button)> pullFunc = [](int button){};
	std::function<void(Program& prog)> renderFunc = [](Program&){};

	NVGcontext* vg = NULL;
	nanoGroup* ui;


	Renderer( GLFWwindow* win ): window(win) {
		s_renderers().push_back(this);
		int w, h;
		glfwGetFramebufferSize(win, &w, &h);
		camera.viewport = vec2(w,h);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		
		glfwSetCursorPosCallback(window, s_cursorCallback );
		glfwSetMouseButtonCallback(window, s_buttonCallback );
		glfwSetScrollCallback(window, s_scrollCallback);
		glfwSetDropCallback(window, s_dropFunction );
		glfwSetKeyCallback(window, s_keyCallback );
		
//		vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_DEBUG);
		vg = nvgCreateGL3(0);
		vg = nanoWidget::nanoUIInit("");
		ui = new nanoGroup(0,0,200,0);
	}
	void cursorCallback( const vec2& pt ) {
		if( nanoWidget::nanoUIMove(window, pt.x, pt.y, *ui) ) return;
		if( glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_1 ) ) {
			vec3 d = camera.position - camera.center;
			float dist = length(d);
			float theta = atan2f(d.x,d.z);
			float phi   = atan2f(d.y,length(vec2(d.x,d.z)));
			theta -= (pt.x-mousePt.x)/camera.viewport.x*PI;
			phi   += (pt.y-mousePt.y)/camera.viewport.y*PI;
			d = rotate( theta, vec3(0,1,0) )*rotate(phi,vec3(-1,0,0)) * vec4(0,0,dist,0);
			camera.position = camera.center + d;
		}
		mousePt = pt;
	}
	void buttonCallback( int button, int action, int mods ) {
		if( nanoWidget::nanoUIButton(window, button, action, mods, *ui) ) return;
		double x, y;
		glfwGetCursorPos(window, &x, &y);
		mousePt = vec2(x,y);
		if( action == GLFW_PRESS ) pushFunc( button );
		else if( action == GLFW_RELEASE) pullFunc( button );
	}
	void scrollCallback( double x, double y ) {
		vec3 d = camera.position - camera.center;
		float dist = length(d);
		dist *= powf(0.99f,float(y));
		camera.position = camera.center + dist*normalize(d);
	}
	void keyCallback( int key, int scan, int action, int mods ) {
		if( action == GLFW_PRESS ) keyFunc( key );
	}
	void dropFunction( int n,const char* fns[] ) {
		if( n>0 ) dropFunc( fns[0] );
	}
	void setSceneBound( const vec3& minVal, const vec3& maxVal, float distFactor = 1.5 ) {
		camera.center = (minVal+maxVal)/2.f;
		vec3 sz3 = abs( maxVal-minVal );
		float dist = max(sz3.x/camera.viewport.x*camera.viewport.y,sz3.y)*distFactor;
		camera.position = camera.center+dist*vec3(0,0,1);
		camera.zNear = dist*0.1f;
		camera.zFar = dist*10.f;
	}
	void render( int w, int h ) {
		if( !initialized ) {
			initFunc();
			initialized = true;
		}
		camera.viewport = vec2(w,h);
		glViewport(0,0,w,h);
		glEnable(GL_DEPTH_TEST);
		glClearColor(clearColor.r,clearColor.g,clearColor.b,clearColor.a);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		renderProg.use();
		renderProg.setUniform( "viewport", camera.viewport );
		renderProg.setUniform( "zNear", camera.zNear );
		renderProg.setUniform( "zFar", camera.zFar );
		renderProg.setUniform( "cameraPosition", camera.position );
		renderProg.setUniform( "viewMat", camera.viewMat() );
		renderProg.setUniform( "projMat", camera.projMat() );
		renderFunc( renderProg );
	}
	void renderUI( int ww, int wh, int fw, int fh ) {
		glViewport(0,0,fw,fh);
		nvgBeginFrame(vg, ww, wh, 1);
		ui->render(vg);
		nvgEndFrame(vg);
	}
	
	
	
	static void s_cursorCallback( GLFWwindow* window, double x, double y ) {
		Renderer* renderer = find( window );
		if( renderer ) renderer->cursorCallback( vec2(x,y) );
	}
	static void s_buttonCallback( GLFWwindow* window, int button, int action, int mods ) {
		Renderer* renderer = find( window );
		if( renderer ) renderer->buttonCallback( button, action, mods );
	}
	static void s_scrollCallback( GLFWwindow* window, double x, double y ) {
		Renderer* renderer = find( window );
		if( renderer ) renderer->scrollCallback( x, y );
	}
	static void s_keyCallback( GLFWwindow* window, int key, int scan, int action, int mods ) {
		Renderer* renderer = find( window );
		if( renderer ) renderer->keyCallback( key, scan, action, mods );
	}
	static void s_dropFunction( GLFWwindow* window, int n,const char* fns[] ) {
		Renderer* renderer = find( window );
		if( renderer ) renderer->dropFunction( n, fns );
	}
	static Renderer* find( GLFWwindow* win ) {
		auto& renderers = s_renderers();
		for( auto& renderer: renderers )
			if( renderer->window==win ) return renderer;
		return nullptr;
	}
	static std::vector<Renderer*>& s_renderers() {
		static std::vector<Renderer*> renderers;
		return renderers;
	};
	
};

#endif /* Renderer_h */
