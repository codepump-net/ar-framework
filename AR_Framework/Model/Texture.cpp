//
//  Texture.cpp
//  AR_Framework
//
//  Created by Hyun Joon Shin on 2021/09/06.
//

#include "Texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

namespace AR {

const char* __blit_vert = ""
"#version 410\n"
"layout(location=0) in vec3 inPosition;\n"
"layout(location=2) in vec2 inTexCoord;\n"
"uniform mat4 modelMat;\n"
"out vec2 texCoord;\n"
"void main(void) {\n"
"	vec4 world_Pos = modelMat* vec4( inPosition, 1. );\n"
"	gl_Position= world_Pos;\n"
"	texCoord = vec2( inTexCoord.x, 1.-inTexCoord.y );\n"
"}\n"
;

const char* __blit_frag = ""
"#version 410\n"
"out vec4 outColor;\n"
"in vec2 texCoord;\n"
"\n"
"uniform sampler2D diffTex;\n"
"uniform float scale = 1.f;\n"
"uniform mat3 CSC;\n"
"uniform float gamma;\n"
"\n"
"\n"
"//***************************************************\n"
"//            Color Space Conversion Functions\n"
"//***************************************************\n"
"float tonemap_sRGB(float u) {\n"
"	float u_ = abs(u);\n"
"	return  u_>0.0031308?( sign(u)*1.055*pow( u_,0.41667)-0.055):(12.92*u);\n"
"}\n"
"vec3 tonemap( vec3 rgb, mat3 csc, float gamma ){\n"
"	vec3 rgb_ = csc*rgb;\n"
"	if( abs( gamma-2.4) <0.01 ) // sRGB\n"
"		return vec3( tonemap_sRGB(rgb_.r), tonemap_sRGB(rgb_.g), tonemap_sRGB(rgb_.b) );\n"
"	return sign(rgb_)*pow( abs(rgb_), vec3(1./gamma) );\n"
"}\n"
"float inverseTonemap_sRGB(float u) {\n"
"	float u_ = abs(u);\n"
"	return u_>0.04045?(sign(u)*pow((u_+0.055)/1.055,2.4)):(u/12.92);\n"
"}\n"
"vec3 inverseTonemap( vec3 rgb, mat3 csc, float gamma ){\n"
"	if( abs( gamma-2.4) <0.01 ) // sRGB\n"
"		return csc*vec3( inverseTonemap_sRGB(rgb.r), inverseTonemap_sRGB(rgb.g), inverseTonemap_sRGB(rgb.b) );\n"
"	return csc*sign(rgb)*pow( abs(rgb), vec3(gamma) );\n"
"}\n"
"\n"
"\n"
"void main(void) {\n"
"	vec4 color = texture( diffTex, texCoord );\n"
"	color.rgb = color.rgb * scale;\n"
"	outColor = vec4( tonemap( color.rgb, CSC, gamma ), color.a );\n"
"}\n"
;


AutoBuildProgram __blitProgram__( __blit_vert, __blit_frag );

}

