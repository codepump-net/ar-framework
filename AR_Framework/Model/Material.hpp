//
//  Material.hpp
//  AR_Framework
//
//  Created by Hyun Joon Shin on 2021/09/06.
//

#ifndef Material_hpp
#define Material_hpp

#include "Tools/gl.hpp"

namespace AR {

#ifdef USE_GLM
using namespace glm;
#else
using namespace jm;
#endif

struct Material {
	vec4 diffColor = vec4(1,.3f,0,1);
	vec3 specColor = vec3(1);
	vec3 refIndex  = vec3(1.48f);
	float roughness = 0.3f;
	int  diffTexID = -1;
	int  specTexID = -1;
	int  bumpMapID = -1;
	int  normMapID = -1;
	int  emissionMapID = -1;
	int  roughnessMapID = -1;
	int  opacityMapID = -1;
	int  metalnessMapID = -1;
	int  ambOccMatID = -1;
	bool roughnessMapInverse = false;
	std::string name;
};

}

#endif /* Material_hpp */
