//
//  FrameBuffer.hpp
//  AR_Framework
//
//  Created by Hyun Joon Shin on 2021/10/29.
//

#ifndef FrameBuffer_hpp
#define FrameBuffer_hpp
#include "gl.hpp"
#include "texture.hpp"

struct _PREV_STATE_ {
	GLint  drawFboId, readFboId;
	GLint  viewport[4];
	GLint  scissor[4];
	GLint  cullMode = GL_BACK;
	GLint  cullFace = GL_FALSE;
	GLint  depthTest = GL_FALSE;
	GLint  scissorTest = GL_FALSE;
	void capture() {
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);
		glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFboId);
		glGetIntegerv(GL_VIEWPORT, viewport);
		glGetIntegerv(GL_SCISSOR_BOX, scissor);
		glGetIntegerv(GL_CULL_FACE_MODE, &cullMode );

		cullFace = glIsEnabled(GL_CULL_FACE);
		depthTest = glIsEnabled(GL_DEPTH_TEST);
		scissorTest = glIsEnabled(GL_SCISSOR_TEST);
	}
	void restore() {
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, drawFboId );
		glBindFramebuffer( GL_READ_FRAMEBUFFER, readFboId );
		glViewport( viewport[0], viewport[1], viewport[2], viewport[3] );
		glScissor( scissor[0], scissor[1], scissor[2], scissor[3] );
		glCullFace( cullMode );

		if( cullFace )	glEnable ( GL_CULL_FACE );
		else			glDisable( GL_CULL_FACE );
		if( depthTest )	glEnable ( GL_DEPTH_TEST );
		else			glDisable( GL_DEPTH_TEST );
		if(scissorTest) glEnable ( GL_SCISSOR_TEST );
		else			glDisable( GL_SCISSOR_TEST );
	}
};



struct Framebuffer : Texture {
	bool depthed = false;
	
	GLuint fbID=0, depthID=0;
	_PREV_STATE_ prevState;
	
	Framebuffer(): fbID(0), depthID(0) {}
	Framebuffer(Framebuffer&&a): Texture(std::forward<Texture>(a)), fbID(a.fbID), depthID(a.depthID) {
		a.depthID = a.fbID = 0;
	}
	void storeFramebufferState() {
		prevState.capture();
		glDisable( GL_SCISSOR_TEST );
	}
	void restoreFramebufferState() {
		prevState.restore();
	}
	virtual void create( int w, int h, GLenum type=GL_UNSIGNED_BYTE, int numChannels=4, bool withDepthBuffer=false ) {
		if( w == width && h == height && type == dataType && numChannels == nChannels
		   && depthed == withDepthBuffer && fbID>0 )
			return;

		dataType = type;
		width = w;
		height = h;
		nChannels = numChannels;
		depthed = withDepthBuffer;
		auto [internal,format,_type] = getTextureType( dataType, numChannels, false );
		clear();
		storeFramebufferState();

		GLint oldTex = 0;
		glGetIntegerv( GL_TEXTURE_BINDING_2D, &oldTex );
		glErr("Before create fbo texture");
		bool needBind = false;
		if( texID< 1) {
			glGenTextures( 1, &texID );
			glBindTexture( GL_TEXTURE_2D, texID );
			setTexParam( GL_LINEAR, wrap_s, wrap_t );
			glTexImage2D(GL_TEXTURE_2D, 0, internal, width, height, 0, format, _type, 0 );
			needBind = true;
		}
		if( depthed && depthID<1 ) {
			glGenTextures( 1, &depthID );
			glBindTexture(GL_TEXTURE_2D, depthID );
			setTexParam( GL_LINEAR, wrap_s, wrap_t );
			if( dataType == GL_FLOAT )
				glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height,
							 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
			else
				glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height,
							 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
			needBind = true;
		}
		if( fbID<1 ) {
			glGenFramebuffers( 1, &fbID );
			needBind = true;
		}
		if( needBind ) {
			glBindFramebuffer( GL_FRAMEBUFFER, fbID );
			glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texID, 0 );
			glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthID, 0 );
			std::vector<GLenum> drawBuffers = {GL_COLOR_ATTACHMENT0};
			glDrawBuffers( 1, drawBuffers.data() );
			glErr("glDrawBuffers");
			GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
			if( status !=GL_FRAMEBUFFER_COMPLETE )
				fprintf( stderr, "FBO is not completed!!\n" );
		}
		glBindTexture( GL_TEXTURE_2D, oldTex );
		restoreFramebufferState();
	}
	virtual void use(bool setViewport=true) {
		storeFramebufferState();
		glBindFramebuffer(GL_FRAMEBUFFER, fbID);
		if( setViewport ) glViewport(0,0,width,height);
	}
	virtual void unuse() {
		restoreFramebufferState();
	}
	virtual void clear() {
		if( fbID>0   ) glDeleteFramebuffers( 1, &fbID ); fbID=0;
		if( depthID>0 ) glDeleteTextures( 1, &depthID ); depthID=0;
		Texture::clear();
	}
	
	virtual void bindDepth( int slot ) {
		if( depthID<1 ) return;
		glActiveTexture( GL_TEXTURE0 + slot );
		glBindTexture( GL_TEXTURE_2D, depthID );
	}
	virtual void bindDepth( int slot, const Program& program, const std::string& name ) {
		bindDepth( slot );
		program.setUniform( name, slot );
	}

	template<typename T> T* readPixels() {
		T* buf = nullptr;
		GLuint format = GL_RGB;
		switch( nChannels ) {
			case 1: format = GL_RED; break;
			case 4: format = GL_RGBA; break;
			default:
			case 3: format = GL_RGB; break;
		}
		GLint  readFboId;
		glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFboId);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbID );

		if( std::is_same<T,float>::value ) {
			buf = new T[width*height*nChannels];
			glReadPixels(0, 0, width, height, format, GL_FLOAT, buf);
		}
		else if( std::is_same<T,int>::value ) {
			buf = new T[width*height*nChannels];
			glReadPixels(0, 0, width, height, format, GL_INT, buf);
		}
		else if( std::is_same<T,unsigned int>::value ) {
			buf = new T[width*height*nChannels];
			glReadPixels(0, 0, width, height, format, GL_UNSIGNED_INT, buf);
		}
		else if( std::is_same<T,unsigned char>::value ) {
			buf = new T[width*height*nChannels];
			glReadPixels(0, 0, width, height, format, GL_UNSIGNED_BYTE, buf);
		}
		glBindFramebuffer(GL_READ_FRAMEBUFFER, readFboId );
		return buf;
	}

	~Framebuffer() {
		clear();
	}
};

#endif /* FrameBuffer_hpp */
