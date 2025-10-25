//
//  gl.hpp
//  AR_Framework
//
//  Created by Hyun Joon Shin on 2021/09/06.
//

#ifndef gl_hpp
#define gl_hpp

#define NOMINMAX
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#else
#define GLEW_STATIC
#include <gl/glew.h>
#pragma comment (lib, "glew32s")
#pragma comment (lib, "opengl32")
#endif

//#define USE_GLM

#ifdef USE_GLM
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#else
#include <jm/jm.hpp>
#endif


#include <fstream>
#include <iostream>
#include <string>

namespace AR {

#ifdef USE_GLM
using namespace glm;
const float PI = 3.14159265358979f;
#else
using namespace jm;
#endif



#ifdef _MSC_VER
typedef wchar_t CHAR_T;
#include <windows.h>
inline std::wstring utf82Unicode(const std::string& s) {
	int len = MultiByteToWideChar(CP_UTF8,0,s.c_str(),-1,NULL,0);
	wchar_t* wstr = new wchar_t[len+1];
	len = MultiByteToWideChar(CP_UTF8,0,s.c_str(),-1,wstr,len);
	wstr[len]=L'\0';
	std::wstring ws(L""); ws+=wstr;
	delete [] wstr;
	return ws;
}
#else
typedef char CHAR_T;
#define utf82Unicode(X) (X)
#endif


inline std::string backToFrontSlash( const std::string& filename ) {
	std::string ret = filename;
	for( int i=0; i<ret.size(); i++ )
	if( ret[i]=='\\' ) ret[i] = '/';
	return ret;
}

inline std::string getFilenameFromAbsPath( const std::string& filename )
{
	size_t slashPos = filename.find_last_of('/');
	if( slashPos == std::string::npos ) return filename;
	if( slashPos == filename.length()-1 ) return "";
	return filename.substr(slashPos+1);
}

inline std::string getPath( const std::string& filename )
{
	size_t slashPos = filename.find_last_of('/');
	if( slashPos == std::string::npos ) return "";
	if( slashPos == filename.length()-1 ) return filename;
	return filename.substr(0,slashPos+1);
}

inline std::string readString( std::istream& ifs ) {
	std::string ret = "";
	std::getline(ifs, ret, '\"');
	std::getline(ifs, ret, '\"');
	return ret;
}


inline std::string loadText(const std::string& filename) {
	std::ifstream t(utf82Unicode(backToFrontSlash(filename)));
	if( !t.is_open() ) {
		std::cerr<<"[ERROR] Text file: "<<getFilenameFromAbsPath(filename)<<" is not found\n";
		return "";
	}
	std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
	return str;
}

inline unsigned long loadText( const std::string& fn, char*& buf ) {
	unsigned long len = 0;
	std::string filename = backToFrontSlash( fn );
	std::string str = loadText( filename );
	len = (unsigned long)str.length();
	buf = new char[len+1];
	strcpy( buf, str.c_str() );
	
	buf[len] = '\0';
	return len;
}

inline bool testFile( const std::string& fn ) {
	std::string filename = backToFrontSlash( fn );
	if( filename.back()=='/' ) return false;
	std::ifstream file( filename );
	if( !file.is_open() ) {
		FILE* file = fopen( filename.c_str(), "rb" );
		if( file ) {
			fclose( file );
			return 1;
		}
		return 0;
	}
	file.close();
	return 1;
}

inline std::string getExtension( const std::string& filename ) {
	size_t slashPos = filename.find_last_of('.');
	if( slashPos == std::string::npos ) return "";
	if( slashPos == filename.length()-1 ) return "";
	return filename.substr(slashPos+1);
}
inline std::string getName( const std::string& filename ) {
	size_t slashPos = filename.find_last_of('.');
	if( slashPos == std::string::npos ) return filename;
	if( slashPos == filename.length()-1 ) return filename.substr(0,slashPos);
	return filename.substr(0,slashPos);
}


inline void glErr( const std::string& message ) {
	GLint err = glGetError();
	if( err != GL_NO_ERROR ) {
		printf("%08X ", err );
		std::cerr<<"GL Error: "<<message<<std::endl;
	}
}


}

#endif /* gl_h */
