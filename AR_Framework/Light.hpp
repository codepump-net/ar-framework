//
//  Light.hpp
//  AR_Framework
//
//  Created by Hyun Joon Shin on 2021/09/23.
//

#ifndef Light_h
#define Light_h

#include "Tools/gl.hpp"

struct Light {
	float lightFactor = 4.f;
	vec3 position;
	vec3 color = vec3(1,1,1);
	virtual void setUniform( Program& prog ) const {
		prog.setUniform("lightPosition", position );
		prog.setUniform("lightColor", color*lightFactor );
	}
};

#endif /* Light_h */
