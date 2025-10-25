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
uniform vec4 baseColor;
uniform vec3 specColor;
uniform float materialRoughness;
uniform float materialMetallic;
uniform float globalMetallic;
uniform float heightScale;
uniform float aoStrength;
uniform float emissionStrength;

uniform sampler2D normalMap;
uniform sampler2D roughnessMap;
uniform sampler2D metalnessMap;
uniform sampler2D aoMap;
uniform sampler2D heightMap;
uniform sampler2D emissionMap;
uniform sampler2D environmentMap;
uniform sampler2D irradianceMap;
uniform sampler2D prefilterMap;
uniform sampler2D brdfLUT;

uniform int normalMapEnabled;
uniform int roughnessMapEnabled;
uniform int metalnessMapEnabled;
uniform int aoMapEnabled;
uniform int heightMapEnabled;
uniform int emissionMapEnabled;
uniform int roughnessMapInverse;
uniform int environmentEnabled;
uniform int irradianceEnabled;
uniform int prefilterEnabled;
uniform int brdfLUTEnabled;
uniform float iblDiffuseIntensity;
uniform float iblSpecularIntensity;
uniform float prefilterMaxLod;

//***************************************************
//            Color Space Conversion Functions
//***************************************************
// Bidirectional conversions for (inverse) tone mapping so that
// all BRDF math happens in linear space.
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

//***************************************************
//               Microfacet BRDF Helpers
//***************************************************
// Support functions for Cook-Torrance BRDF evaluation.
float saturate(float v){
	return clamp(v, 0.0, 1.0);
}

float distributionGGX(vec3 N, vec3 H, float roughness){
	float a = max(roughness, 0.001);
	float a2 = a * a;
	float NdotH = saturate(dot(N, H));
	float NdotH2 = NdotH * NdotH;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	return a2 / max(PI * denom * denom, 0.0001);
}

float geometrySchlickGGX(float NdotV, float roughness){
	float r = max(roughness, 0.001);
	float k = ( (r + 1.0) * (r + 1.0) ) / 8.0;
	return NdotV / (NdotV * (1.0 - k) + k);
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness){
	float NdotV = saturate(dot(N, V));
	float NdotL = saturate(dot(N, L));
	float ggx1 = geometrySchlickGGX(NdotV, roughness);
	float ggx2 = geometrySchlickGGX(NdotL, roughness);
	return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0){
	float oneMinusCos = clamp(1.0 - cosTheta, 0.0, 1.0);
	float factor = pow(oneMinusCos, 5.0);
	return F0 + (1.0 - F0) * factor;
}

// Tangent frame reconstruction for normal/parallax mapping.
mat3 computeTBN(vec3 N, vec3 pos, vec2 uv){
	vec3 dpdx = dFdx(pos);
	vec3 dpdy = dFdy(pos);
	vec2 duvdx = dFdx(uv);
	vec2 duvdy = dFdy(uv);

	vec3 T = dpdx * duvdy.y - dpdy * duvdx.y;
	vec3 B = dpdy * duvdx.x - dpdx * duvdy.x;
	float lenT = length(T);
	float lenB = length(B);
	if( lenT <= 1e-6 || lenB <= 1e-6 ) {
		vec3 up = abs(N.z) < 0.999?vec3(0,0,1):vec3(0,1,0);
		T = normalize(cross(up, N));
		B = cross(N, T);
	}
	else {
		T /= lenT;
		B /= lenB;
		T = normalize(T - N * dot(N, T));
		B = normalize(B - N * dot(N, B));
		if( dot(cross(T, B), N) < 0.0 )
			T = -T;
		B = normalize(cross(N, T));
	}
	return mat3(T, B, N);
}

// Cheap parallax offset using height map.
vec2 parallaxMapping(vec2 uv, vec3 V, mat3 TBN){
	vec3 viewDirTangent = normalize(transpose(TBN) * V);
	float height = texture(heightMap, uv).r;
	float heightOffset = height * heightScale - heightScale * 0.5;
	vec2 offset = viewDirTangent.xy / max(viewDirTangent.z, 0.001) * heightOffset;
	return uv - offset;
}

//***************************************************
//               Environment Sampling
//***************************************************
// Sample equirectangular environment textures for IBL fallback paths.
vec2 sphericalUV(vec3 dir){
	dir = normalize(dir);
	float phi = atan(dir.z, dir.x);
	float theta = acos(clamp(dir.y, -1.0, 1.0));
	float u = (phi + PI) / (2.0 * PI);
	float v = theta / PI;
	return vec2(u, v);
}

vec3 sampleEnvironment(vec3 dir){
	if( environmentEnabled<=0 ) return vec3(0);
	vec2 uv = sphericalUV(dir);
	return texture(environmentMap, uv).rgb;
}

vec3 sampleIrradiance(vec3 dir){
	if( irradianceEnabled>0 ) {
		vec2 uv = sphericalUV(dir);
		return texture(irradianceMap, uv).rgb;
	}
	return sampleEnvironment(dir);
}

vec3 samplePrefilter(vec3 dir, float rough){
	if( prefilterEnabled>0 ) {
		vec2 uv = sphericalUV(dir);
		float lod = rough * prefilterMaxLod;
		return textureLod(prefilterMap, uv, lod).rgb;
	}
	return sampleEnvironment(dir);
}

vec2 sampleBRDFLUT(float NdotV, float rough){
	if( brdfLUTEnabled>0 ) {
		return texture(brdfLUT, vec2(saturate(NdotV), saturate(rough))).rg;
	}
	return vec2(0.5, 0.5);
}

//***************************************************
//                Main Shading Path
//***************************************************
void main() {
	vec3 faceN = normalize( cross( dFdx(worldPos), dFdy(worldPos) ) );
	vec3 N = normalize(normal);
	vec3 toLight = lightPosition - worldPos;
	if( dot(N,faceN) <0 ) N = -N;
	vec4 albedo = vec4(1);
	if( diffTexEnabled>0 )
		albedo = texture( diffTex, texCoord );
	vec3 L = normalize(toLight);
	vec3 V = normalize(cameraPosition - worldPos);
	mat3 TBN = computeTBN(N, worldPos, texCoord);
	vec2 uv = texCoord;
	if( heightMapEnabled>0 )
		uv = parallaxMapping(uv, V, TBN);
	if( diffTexEnabled>0 )
		albedo = texture( diffTex, uv );
	vec3 albedoLinear = diffTexEnabled>0 ? inverseTonemap(albedo.rgb, mat3(1), 2.4) : vec3(1);
	vec3 baseLinear = baseColor.rgb;
	albedoLinear *= baseLinear;
	float alpha = baseColor.a * albedo.a;

	vec3 sampledNormal = N;
	if( normalMapEnabled>0 ) {
		vec3 tangentNormal = texture( normalMap, uv ).xyz * 2.0 - 1.0;
		sampledNormal = normalize( TBN * tangentNormal );
	}
	N = normalize(sampledNormal);
	vec3 H = normalize(L + V);
	vec3 radiance = lightColor / max(dot(toLight, toLight), 0.0001);
	float NdotL = saturate(dot(N, L));
	float NdotV = saturate(dot(N, V));
	float roughBase = saturate(materialRoughness * roughness);
	if( roughnessMapEnabled>0 ) {
		float roughSample = texture( roughnessMap, uv ).r;
		if( roughnessMapInverse>0 )
			roughSample = 1.0 - roughSample;
		roughBase = saturate(roughSample * roughBase);
	}
	float roughFinal = clamp(roughBase, 0.02, 1.0);

	float metallicValue = saturate(globalMetallic);
	if( metalnessMapEnabled>0 )
		metallicValue = saturate(texture( metalnessMap, uv ).r);
	else
		metallicValue = saturate(metallicValue + materialMetallic);

	vec3 F0 = mix(vec3(0.04) * specColor, albedoLinear, metallicValue);
	vec3 F = fresnelSchlick(saturate(dot(H, V)), F0);
	float D = distributionGGX(N, H, roughFinal);
	float G = geometrySmith(N, V, L, roughFinal);
	vec3 numerator = F * D * G;
	float denom = max(4.0 * NdotV * NdotL, 0.0001);
	vec3 specular = numerator / denom;
	vec3 kS = F;
	vec3 kD = (vec3(1.0) - kS) * (1.0 - metallicValue);
	vec3 diffuse = albedoLinear / PI;

	float ao = 1.0;
	if( aoMapEnabled>0 ) {
		float aoSample = texture( aoMap, uv ).r;
		ao = mix(1.0, aoSample, saturate(aoStrength));
	}

	vec3 ambient = (environmentEnabled>0 || irradianceEnabled>0)?vec3(0):0.03 * albedoLinear * ao;
	vec3 lightContribution = vec3(0);
	if( NdotL>0.0 && NdotV>0.0 ) {
		lightContribution = (kD * diffuse + specular) * radiance * NdotL;
		lightContribution *= ao;
	}

	vec3 emission = vec3(0);
	if( emissionMapEnabled>0 ) {
		vec3 emissionSample = inverseTonemap(texture( emissionMap, uv ).rgb, mat3(1), 2.4);
		emission = emissionSample * emissionStrength;
	}

	// IBL contribution (diffuse irradiance + split-sum specular).
	vec3 irradiance = sampleIrradiance(N);
	vec3 F_IBL = fresnelSchlick(saturate(NdotV), F0);
	vec3 kS_IBL = F_IBL;
	vec3 kD_IBL = (vec3(1.0) - kS_IBL) * (1.0 - metallicValue);
	vec3 diffuseIBL = irradiance * albedoLinear;
	vec3 R = reflect(-V, N);
	vec3 prefilteredColor = samplePrefilter(R, roughFinal);
	vec2 brdfSample = sampleBRDFLUT(NdotV, roughFinal);
	vec3 specularIBL = prefilteredColor * (F_IBL * brdfSample.x + vec3(brdfSample.y));
	vec3 iblContribution = kD_IBL * diffuseIBL * ao * iblDiffuseIntensity + specularIBL * iblSpecularIntensity * ao;

	vec3 colorLinear = ambient + lightContribution + emission + iblContribution;
	outColor = vec4(tonemap(colorLinear,mat3(1),2.4), alpha);
}
