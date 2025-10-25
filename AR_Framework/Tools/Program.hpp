//
//  Program.hpp
//  AR_Framework
//
//  Created by Hyun Joon Shin on 2021/09/06.
//


#ifndef Program_h
#define Program_h

#include "Tools/gl.hpp"
#include <vector>
#include <string>

namespace AR {

struct Program {
	GLuint programID = 0;
	GLuint vertShaderID = 0;
	GLuint fragShaderID = 0;
	GLuint geomShaderID = 0;
	
	static void printInfoProgramLog(GLuint obj) {
		int infologLength = 0, charsWritten  = 0;
		glGetProgramiv( obj, GL_INFO_LOG_LENGTH, &infologLength );
		if( infologLength <=0 ) return;
		char *infoLog = new char[infologLength];
		glGetProgramInfoLog( obj, infologLength, &charsWritten, infoLog );
		std::cerr<<"Program Info:"<<std::endl;
		std::cerr<<infoLog<<std::endl;
		delete [] infoLog;
	}
	
	static inline void printInfoShaderLog(GLuint obj) {
		int infologLength = 0, charsWritten  = 0;
		glGetShaderiv ( obj, GL_INFO_LOG_LENGTH, &infologLength );
		if( infologLength <=0 ) return;
		char *infoLog = new char[infologLength];
		glGetShaderInfoLog ( obj, infologLength, &charsWritten, infoLog );
		std::cerr<<"Shader Info:"<<std::endl;
		std::cerr<<infoLog<<std::endl;
		delete [] infoLog;
	}
	
	Program() : programID(0), vertShaderID(0), fragShaderID(0), geomShaderID(0) {}
	Program(Program&& a)
	: programID(a.programID), vertShaderID(a.vertShaderID), fragShaderID(a.fragShaderID), geomShaderID(a.geomShaderID) {
		a.fragShaderID = a.vertShaderID = a.geomShaderID = a.programID = 0;
	}
	void build( const char* vshaderSrc, const char* fshaderSrc, const char* gshaderSrc=nullptr ) {
		programID = glCreateProgram();
		
		vertShaderID = glCreateShader( GL_VERTEX_SHADER );
		glShaderSource( vertShaderID, 1, (const GLchar* const*)&vshaderSrc, nullptr );
		glCompileShader( vertShaderID );
		printInfoShaderLog( vertShaderID );
		
		fragShaderID = glCreateShader( GL_FRAGMENT_SHADER );
		glShaderSource( fragShaderID, 1, (const GLchar* const*)&fshaderSrc, nullptr );
		glCompileShader( fragShaderID );
		printInfoShaderLog( fragShaderID );
		
		if( gshaderSrc && strlen( gshaderSrc )>0 ) {
			geomShaderID = glCreateShader( GL_GEOMETRY_SHADER );
			glShaderSource( geomShaderID, 1, (const GLchar* const*)&gshaderSrc, nullptr );
			glCompileShader( geomShaderID );
			printInfoShaderLog( geomShaderID );
		}
		glAttachShader( programID, vertShaderID );
		glAttachShader( programID, fragShaderID );
		if( geomShaderID>0 ) {
			glAttachShader( programID, geomShaderID );
		}
		glLinkProgram( programID );
		glUseProgram( programID );
		printInfoProgramLog(programID);
	}
	void load( const std::string& vsFilename, const std::string& fsFilename, const std::string& gsFilename="" ) {
		char *vshaderSrc=nullptr, *fshaderSrc=nullptr, *gshaderSrc=nullptr;
		loadText( vsFilename, vshaderSrc );
		loadText( fsFilename, fshaderSrc );
		if( gsFilename.length()>0 )
			loadText( gsFilename, gshaderSrc );
		
		build(vshaderSrc, fshaderSrc, gshaderSrc );
		
		if( fshaderSrc ) delete [] fshaderSrc;
		if( vshaderSrc ) delete [] vshaderSrc;
		if( gshaderSrc ) delete [] gshaderSrc;
	}

	
	void clear() {
		if( programID )	glDeleteProgram( programID );
		if( vertShaderID )	glDeleteShader( vertShaderID );
		if( fragShaderID )	glDeleteShader( fragShaderID );
		if( geomShaderID )	glDeleteShader( geomShaderID );
		programID = vertShaderID = fragShaderID = geomShaderID = 0;
	}
	~Program() { clear(); }
	inline bool isUsable() const { return programID>0; }
	virtual void use() {
		glUseProgram( programID );
	}
	
	void setUniform( const std::string& name, float v ) const {
		glUniform1f( glGetUniformLocation( programID, name.c_str() ), v );
	}
	void setUniform( const std::string& name, int v ) const {
		glUniform1i( glGetUniformLocation( programID, name.c_str() ), v );
	}
	void setUniform( const std::string& name, const vec2& v ) const {
		glUniform2f( glGetUniformLocation( programID, name.c_str() ), v.x, v.y );
	}
	void setUniform( const std::string& name, const vec3& v ) const {
		glUniform3f( glGetUniformLocation( programID, name.c_str() ), v.x, v.y, v.z );
	}
	void setUniform( const std::string& name, const vec4& v ) const {
		glUniform4f( glGetUniformLocation( programID, name.c_str() ), v.x, v.y, v.z, v.w );
	}
	void setUniform( const std::string& name, const mat3& v ) const {
		glUniformMatrix3fv(glGetUniformLocation( programID, name.c_str() ), 1, GL_FALSE, value_ptr(v) );
	}
	void setUniform( const std::string& name, const mat4& v ) const {
		glUniformMatrix4fv(glGetUniformLocation( programID, name.c_str() ), 1, GL_FALSE, value_ptr(v) );
	}
	
	
	
	
	void setUniform( const std::string& name, const float* v, uint32_t count ) const {
		glUniform1fv( glGetUniformLocation( programID, name.c_str() ), count, v );
	}
	void setUniform( const std::string& name, const int* v, uint32_t count ) const {
		glUniform1iv( glGetUniformLocation( programID, name.c_str() ), count, v );
	}
	void setUniform( const std::string& name, const vec2* v, uint32_t count ) const {
		glUniform2fv( glGetUniformLocation( programID, name.c_str() ), count, (float*)v );
	}
	void setUniform( const std::string& name, const vec3* v, uint32_t count ) const {
		glUniform3fv( glGetUniformLocation( programID, name.c_str() ), count, (float*)v );
	}
	void setUniform( const std::string& name, const vec4* v, uint32_t count ) const {
		glUniform4fv( glGetUniformLocation( programID, name.c_str() ), count, (float*)v );
	}
	void setUniform( const std::string& name, const mat3* v, uint32_t count ) const {
		glUniformMatrix3fv(glGetUniformLocation( programID, name.c_str() ), count, GL_FALSE, (float*)v );
	}
	void setUniform( const std::string& name, const mat4* v, uint32_t count ) const {
		glUniformMatrix4fv(glGetUniformLocation( programID, name.c_str() ), count, GL_FALSE, (float*)v );
	}
	
	
	
	
	
	void setUniform( const std::string& name, const std::vector<float>& v ) const {
		glUniform1fv( glGetUniformLocation( programID, name.c_str() ), (uint32_t)v.size(), v.data() );
	}
	void setUniform( const std::string& name, const std::vector<int>& v ) const {
		glUniform1iv( glGetUniformLocation( programID, name.c_str() ), (uint32_t)v.size(), v.data() );
	}
	void setUniform( const std::string& name, const std::vector<vec2>& v ) const {
		glUniform2fv( glGetUniformLocation( programID, name.c_str() ), (uint32_t)v.size(), (float*)v.data() );
	}
	void setUniform( const std::string& name, const std::vector<vec3>& v ) const {
		glUniform3fv( glGetUniformLocation( programID, name.c_str() ), (uint32_t)v.size(), (float*)v.data() );
	}
	void setUniform( const std::string& name, const std::vector<vec4>& v ) const {
		glUniform4fv( glGetUniformLocation( programID, name.c_str() ), (uint32_t)v.size(), (float*)v.data() );
	}
	void setUniform( const std::string& name, const std::vector<mat3>& v ) const {
		glUniformMatrix3fv(glGetUniformLocation( programID, name.c_str() ), (uint32_t)v.size(), GL_FALSE, (float*)v.data() );
	}
	void setUniform( const std::string& name, const std::vector<mat4>& v ) const {
		glUniformMatrix4fv(glGetUniformLocation( programID, name.c_str() ), (uint32_t)v.size(), GL_FALSE, (float*)v.data() );
	}
};


struct AutoLoadProgram : Program {
	std::string vsFilename, fsFilename, gsFilename;
	AutoLoadProgram( const std::string& vShaderFilename,
					const std::string& fShaderFilename,
					const std::string& gShaderFilename="" )
	: vsFilename( vShaderFilename ), fsFilename( fShaderFilename ), gsFilename( gShaderFilename ) {}
	virtual void use() {
		if( !isUsable() ) {
			printf("Auto building: %s + %s\n", vsFilename.c_str(), fsFilename.c_str() );
			load( vsFilename, fsFilename, gsFilename );
		}
		Program::use();
	}
};

struct AutoBuildProgram : Program {
	const char* vertShaderSrc = nullptr;
	const char* fragShaderSrc = nullptr;
	const char* geomShaderSrc = nullptr;
	AutoBuildProgram( const char* vertSrc, const char* fragSrc, const char* geomSrc=nullptr )
	: vertShaderSrc(vertSrc), fragShaderSrc(fragSrc), geomShaderSrc(geomSrc) {}
	virtual void use(){
		if( !isUsable() ) build( vertShaderSrc, fragShaderSrc, geomShaderSrc );
		Program::use();
	}
};


}

#endif /* Program_h */
