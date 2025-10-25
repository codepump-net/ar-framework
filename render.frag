#version 410 core
out vec4 outColor;
const float PI = 3.1415926535;
in vec3 normal;
in vec3 worldPos;
in vec2 texCoord;

uniform vec2 viewport;
uniform sampler2D diffTex;
uniform int  diffTexEnabled=0;
uniform vec3 lightColor;
uniform vec3 lightPosition;
uniform vec3 cameraPosition;
uniform float roughness;

//***************************************************
//            Color Space Conversion Functions
//***************************************************
float tonemap_sRGB(float u) {
	float u_ = abs(u);
	return  u_>0.0031308?( sign(u)*1.055*pow( u_,0.41667)-0.055):(12.92*u);
}
vec3 tonemap( vec3 rgb, mat3 csc, float gamma ){
	vec3 rgb_ = csc*rgb;
	if( abs( gamma-2.4) <0.01 ) // sRGB
		return vec3( tonemap_sRGB(rgb_.r), tonemap_sRGB(rgb_.g), tonemap_sRGB(rgb_.b) );
	return sign(rgb_)*pow( abs(rgb_), vec3(1./gamma) );
}
float inverseTonemap_sRGB(float u) {
	float u_ = abs(u);
	return u_>0.04045?(sign(u)*pow((u_+0.055)/1.055,2.4)):(u/12.92);
}
vec3 inverseTonemap( vec3 rgb, mat3 csc, float gamma ){
	if( abs( gamma-2.4) <0.01 ) // sRGB
		return csc*vec3( inverseTonemap_sRGB(rgb.r), inverseTonemap_sRGB(rgb.g), inverseTonemap_sRGB(rgb.b) );
	return csc*sign(rgb)*pow( abs(rgb), vec3(gamma) );
}

void main() {
	vec3 faceN = normalize( cross( dFdx(worldPos), dFdy(worldPos) ) );
	vec3 N = normalize(normal);
	vec3 toLight = lightPosition-worldPos;
	vec3 w_i = normalize( toLight );
	vec3 w_o = normalize( cameraPosition - worldPos );
	if( dot(N,faceN) <0 ) N = -N;
	vec4 albedo = vec4(1);
	if( diffTexEnabled>0 )
		albedo = texture( diffTex, texCoord );
	vec3 Li = lightColor/dot(toLight,toLight);
	vec4 color;
	color.rgb = albedo.rgb * Li;
	color.a = albedo.a;
	outColor = vec4(tonemap(color.rgb,mat3(1),2.4),color.a);
}

