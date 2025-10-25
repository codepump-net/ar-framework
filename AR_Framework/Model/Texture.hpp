//
//  Texture.hpp
//  AR_Framework
//
//  Created by Hyun Joon Shin on 2021/09/06.
//

#ifndef Texture_h
#define Texture_h

#include <stb_image.h>
#include <stb_image_resize.h>
#include "Tools/gl.hpp"
#include "Tools/Program.hpp"
#include "TriMesh.hpp"

namespace AR {

extern 	AutoBuildProgram __blitProgram__;
extern const char* __blit_vert;
extern const char* __blit_frag;


struct Texture {
	
	static std::tuple<GLenum,GLenum,GLenum> getTextureType( GLenum type, int nChannels, int sRGB=false ) {
		switch( type ) {
			case GL_UNSIGNED_SHORT:
				switch( nChannels ) {
					case 1:		return {GL_R16UI, GL_RED, type};
					case 2:		return {GL_RG16UI, GL_RG, type};
					case 4:		return {GL_RGBA16UI, GL_RGBA, type};
					case 3:
					default:	return {GL_RGB16UI, GL_RGB, type};
				}
				break;
			case GL_UNSIGNED_INT:
				switch( nChannels ) {
					case 1:		return {GL_R32UI, GL_RED, type};
					case 2:		return {GL_RG32UI, GL_RG, type};
					case 4:		return {GL_RGBA32UI, GL_RGBA, type};
					case 3:
					default:	return {GL_RGB32UI, GL_RGB, type};
				}
				break;
			case GL_FLOAT:
				switch( nChannels ) {
					case 1:		return {GL_R32F, GL_RED, type};
					case 2:		return {GL_RG32F, GL_RG, type};
					case 4:		return {GL_RGBA32F, GL_RGBA, type};
					case 3:
					default:	return {GL_RGB32F, GL_RGB, type};
				}
				break;
			case GL_UNSIGNED_BYTE:
			default:
				switch( nChannels ) {
					case 1:		return {GL_R8, GL_RED, type};
					case 2:		return {GL_RG8, GL_RG, type};
					case 4:		return {GLenum(sRGB?GL_SRGB8_ALPHA8:GL_RGBA8), GL_RGBA, type};
					case 3:
					default:	return {GLenum(sRGB?GL_SRGB8:GL_RGB8), GL_RGB, type};
				}
				break;
		}
	}

	static void setTexParam( GLuint minFilter = GL_LINEAR, GLuint wrap_s = GL_REPEAT, GLuint wrap_t = GL_REPEAT ) {
		float maxAniso;
//		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
//		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, maxAniso);
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t );
	}
	
	GLuint texID=0;
	GLsizei width=0, height=0, nChannels=0;
	GLsizei glW = 0, glH = 0, glN = 0;
	bool    texDataDirty = false;
	bool	ownBuf = false;
	GLenum dataType = GL_UNSIGNED_BYTE;
	bool hdr = false;
	std::string name = "";
	unsigned char* buf = nullptr;
	bool SRGB = true;
	GLuint wrap_t = GL_REPEAT;
	GLuint wrap_s = GL_REPEAT;
	GLuint inter= GL_LINEAR_MIPMAP_LINEAR;
	
	
	Texture()
	: texID(0), width(0), height(0), nChannels(0), dataType(GL_UNSIGNED_BYTE), hdr(false), SRGB(true),
	buf(nullptr), ownBuf(false), texDataDirty(false) {}
	Texture( const std::string& fn, bool sRGB=true )
	: texID(0), width(0), height(0), nChannels(0), dataType(GL_UNSIGNED_BYTE), hdr(false), SRGB(sRGB),
	buf(nullptr), ownBuf(false), texDataDirty(false) {
		load( fn, sRGB);
	}
	Texture( Texture&&a )
	: texID(a.texID), width(a.width), height(a.height), nChannels(a.nChannels), dataType(a.dataType), SRGB(a.SRGB), hdr(a.hdr),
	name(a.name), buf(a.buf), ownBuf(a.ownBuf), texDataDirty(a.texDataDirty) {
		a.texID = 0;
		a.ownBuf = false;
		a.buf = nullptr;
		a.nChannels = 0;
	}
	
	~Texture(){ clear(); }
	virtual void clear() {
		if( texID ) glDeleteTextures(1, &texID); texID = 0;
	}
	virtual void createGL() {
		if( glW==width && glH==height && glN==nChannels && texID>0 ) {
			if( texDataDirty && buf && glW>0 )
				update( buf );
			if( buf && ownBuf ) free(buf); buf = nullptr;
			return;
		}
		if( texID>0 ) clear();
		auto [internal,format,type] = getTextureType( dataType, nChannels, SRGB );
		GLint oldTex = Texture::getBinding();
		glGenTextures( 1, &texID );
		glBindTexture( GL_TEXTURE_2D, texID );
		setTexParam(inter,wrap_s,wrap_t);
		glTexImage2D( GL_TEXTURE_2D, 0, internal, width, height, 0, format, type, buf);
		glGenerateMipmap( GL_TEXTURE_2D );
		Texture::restoreBinding( oldTex );
		glW = width;
		glH = height;
		glN = nChannels;
		texDataDirty = false;
		if( buf && ownBuf ) free( buf ); buf = nullptr;
	}
	virtual void create( int w, int h, int n, GLuint type, void* data, bool sRGB=false ) {
		if( w!=width || h!=height || n!=nChannels || type!=dataType || sRGB!=SRGB) {
			
			width = w;
			height = h;
			dataType = type;
			nChannels = n;
			SRGB = sRGB;
			ownBuf = false;
			if( buf && ownBuf ) free( buf ); buf = nullptr;
			buf = (unsigned char*)data;
		}
		texDataDirty = true;
	}
	virtual void update( void* data ) {
		GLint oldTex = Texture::getBinding();
		glBindTexture( GL_TEXTURE_2D, texID );
		auto [internal,format,type] = getTextureType( dataType, nChannels, SRGB );
		glErr("Before SubImage2D\n");
		glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, width, height, format, type, data);
		glErr("SubImage2D\n");
		glGenerateMipmap( GL_TEXTURE_2D );
		Texture::restoreBinding( oldTex );
		texDataDirty = false;
	}
	
	virtual bool load( const std::string& fn, bool sRGB=false, int targetWidth=-1 ) {
		SRGB = sRGB;
		std::string filename = backToFrontSlash( fn );
		std::string extension = getExtension( filename );
		if( !testFile( filename ) ) return false;
		if( stbi_is_hdr( filename.c_str() ) ) {
			hdr  = true;
			SRGB = false;
			dataType = GL_FLOAT;
		}
		stbi_set_flip_vertically_on_load(true);
		
		if( buf && ownBuf ) free(buf); buf = nullptr;
		name = filename;
		printf("loading:%s ", getFilenameFromAbsPath(name).c_str());
		int w=0, h=0, n=0;
		stbi_convert_iphone_png_to_rgb(1);
		if( hdr )	buf = (unsigned char*)stbi_loadf( filename.c_str(), &w, &h, &n, 0);
		else		buf = stbi_load( filename.c_str(), &w, &h, &n, 0);
		printf("(%d x %d x %d)", w, h, n);
		
		int desiredTexWidth = w, desiredTexHeight = h;
		if( targetWidth>0 ) {
			desiredTexWidth = targetWidth;
			desiredTexHeight= int(h/float(w)*targetWidth);
		}
		
		int maxTexWidth = 0;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexWidth );
		if( maxTexWidth<desiredTexWidth ) {
			desiredTexHeight = int(h/float(w)*maxTexWidth);
			desiredTexWidth = maxTexWidth;
		}
		if( maxTexWidth<desiredTexHeight ) {
			desiredTexWidth = int(w/float(h)*maxTexWidth);
			desiredTexHeight = maxTexWidth;
		}
		if( w!=desiredTexWidth || h!=desiredTexHeight ) {
			int ww =desiredTexWidth, hh = desiredTexHeight;
			printf("--> (%d x %d)", ww, hh );
			void* temp;
			if( hdr )	{
				temp = (float*)malloc(ww*hh*n*sizeof(float));
				stbir_resize_float( (float*)buf, w, h, 0, (float*)temp, ww, hh, 0, n );
			}
			else {
				temp = (unsigned char*)malloc(ww*hh*n);
				int alphaMode = n>3?0:-1;
				int alphaChannel = n>3?3:-1;
				if( SRGB )		stbir_resize_uint8_srgb( buf, w, h, 0, (unsigned char*)temp, ww, hh, 0, n, alphaChannel, alphaMode );
				else			stbir_resize_uint8( buf, w, h, 0, (unsigned char*)temp, ww, hh, 0, n );
			}
			free( buf );
			buf = (unsigned char*)temp;
			w = ww;
			h = hh;
		}
		
		ownBuf = true;
		texDataDirty = true;
		width = w;
		height = h;
		dataType = hdr?GL_FLOAT:GL_UNSIGNED_BYTE;
		nChannels = n;
		printf("\n");
		return true;
	}
	virtual void bind( int slot ) {
		if( texID<1 || texDataDirty ) {
			glErr("Before Texture Create GL\n");
			if( nChannels>0 ) createGL();
			else return;
			glErr("Texture Create GL\n");
		}
		glActiveTexture( GL_TEXTURE0 + slot );
		glBindTexture( GL_TEXTURE_2D, texID );
	}
	virtual void bind( int slot, const Program& program, const std::string& name ) {
		bind( slot );
		program.setUniform( name, slot );
	}

	virtual void blit( float scale=1.f, bool sRGB=true ) {
		__blitProgram__.use();
		bind(0, __blitProgram__, "diffTex" );
		__blitProgram__.setUniform("modelMat",mat4(1));
		__blitProgram__.setUniform("scale", scale);
		__blitProgram__.setUniform("CSC", mat3(1) );
		__blitProgram__.setUniform("gamma", sRGB?2.4f:1.f );
		TriMesh::renderQuad( __blitProgram__ );
	}
	
	static GLint getBinding( int slot=-1 ) {
		GLint oldTex = 0;
		if( slot>=0 ) glActiveTexture( GL_TEXTURE0 + slot );
		glGetIntegerv( GL_TEXTURE_BINDING_2D, &oldTex );
		return oldTex;
	}
	static void restoreBinding( GLint oldTex, int slot=-1 ) {
		if( slot>=0 ) glActiveTexture( GL_TEXTURE0 + slot );
		glBindTexture( GL_TEXTURE_2D, oldTex );
	}
};


struct TextureLib {
	std::vector<Texture> textures;
	void clear() {
		textures.clear();
	}
	int searchTexture( const std::string& fn ) {
		std::string filename = backToFrontSlash( fn );
		for( auto i=0; i<textures.size(); i++ )
			if( filename.compare( textures[i].name ) == 0 )
				return int(i);
		return -1;
	}
	int testAndLoadTexture( const std::string& fn, bool sRGB ) {
		std::string filename = backToFrontSlash( fn );
		int ret = searchTexture( filename );
		if( ret<0 && testFile( filename ) ) {
			textures.emplace_back();
			textures.back().load( filename, sRGB );
			ret = int(textures.size()-1);
		}
		return ret;
	}
	size_t size() const {
		return textures.size();
	}
	Texture& operator[] (int i) {
		return textures[i];
	}
	const Texture& operator[] (int i) const {
		return textures[i];
	}
};



struct Texture3D: Texture {
	
	static void setTexParam( GLuint minFilter = GL_LINEAR, GLuint wrap_s = GL_REPEAT, GLuint wrap_t = GL_REPEAT, GLuint wrap_r = GL_REPEAT ) {
//		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
//		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, maxAniso);
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, wrap_r );
	}
	
	GLsizei depth=0, height=0, nChannels=0;
	GLsizei glD = 0;;
	GLuint wrap_r = GL_REPEAT;
	
	
	Texture3D() : Texture(), depth(0) {}
	Texture3D( const std::string& fn, bool sRGB=true ) {
		fprintf( stderr, "Texture3D doesnot support file loading\n");
	}
	Texture3D( Texture3D&&a ): Texture( std::move(a) ) { }
	
	virtual void createGL() {
		if( glW==width && glH==height && glN==nChannels && glD==depth && texID>0 ) {
			return;
		}
		if( texID>0 ) clear();
		auto [internal,format,type] = getTextureType( dataType, nChannels, SRGB );
		GLint oldTex = Texture::getBinding();
		glGenTextures( 1, &texID );
		glBindTexture( GL_TEXTURE_3D, texID );
		setTexParam(inter,wrap_s,wrap_t,wrap_r);
		glTexImage3D( GL_TEXTURE_3D, 0, internal, width, height, depth, 0, format, type, buf);
		glGenerateMipmap( GL_TEXTURE_3D );
		Texture::restoreBinding( oldTex );
		glW = width;
		glH = height;
		glN = nChannels;
		texDataDirty = false;
		if( buf && ownBuf ) free( buf ); buf = nullptr;
	}
	virtual void create( int w, int h, int d, int n, GLuint type, void* data, bool sRGB=false ) {
		if( w!=width || h!=height || d!=depth || n!=nChannels || type!=dataType || sRGB!=SRGB) {
			width = w;
			height = h;
			depth = d;
			dataType = type;
			nChannels = n;
			SRGB = sRGB;
			ownBuf = false;
			if( buf && ownBuf ) free( buf ); buf = nullptr;
			buf = (unsigned char*)data;
		}
		texDataDirty = true;
	}
	virtual void update( int k, void* data ) {
		glBindTexture( GL_TEXTURE_3D, texID );
		auto [internal,format,type] = getTextureType( dataType, nChannels, SRGB );
		glErr("Before SubImage2D\n");
		glTexSubImage3D( GL_TEXTURE_3D, 0, 0, 0, k, width, height, 1, format, type, data);
		glErr("SubImage3D\n");
		glGenerateMipmap( GL_TEXTURE_3D );
		texDataDirty = false;
	}
	virtual void bind( int slot ) {
		if( texID<1 || texDataDirty ) {
			glErr("Before Texture Create GL\n");
			if( nChannels>0 ) createGL();
			else return;
			glErr("Texture Create GL\n");
		}
		glActiveTexture( GL_TEXTURE0 + slot );
		glBindTexture( GL_TEXTURE_3D, texID );
	}
	virtual void bind( int slot, const Program& program, const std::string& name ) {
		bind( slot );
		program.setUniform( name, slot );
	}
};



}

#endif /* Texture_h */
