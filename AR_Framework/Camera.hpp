//
//  Camera.hpp
//  AR_Framework
//
//  Created by Hyun Joon Shin on 2021/09/06.
//

#ifndef Camera_hpp
#define Camera_hpp

#include "Tools/gl.hpp"

namespace AR {


struct Camera {
	vec3 position = vec3(0,0,10);
	vec3 center = vec3(0,0,0);
	float fov = 60/180.f*PI;
	float zNear = 0.1f;
	float zFar = 1000.f;
	vec2 viewport;
	
	
	mat4 projMat() const {
		return perspective( fov, viewport.x/viewport.y, zNear, zFar );
	}
	mat4 viewMat() const {
		return lookAt( position, center, vec3(0,1,0));
	}
	
};



}

#endif /* Camera_hpp */
