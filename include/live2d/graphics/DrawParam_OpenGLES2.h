/**
 * DrawParam_OpenGLES2.h
 *
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */
#ifndef __LIVE2D_DRAWPARAM_OPENGLES2_H__
#define __LIVE2D_DRAWPARAM_OPENGLES2_H__


#ifndef __SKIP_DOC__

#include "../Live2D.h"
#include "../type/LDMap.h"
#include "../type/LDVector.h"

#ifdef L2D_GRAPHICS_OPENGL_ES2

#include "DrawParam.h"


#ifdef L2D_TARGET_ANDROID_ES2
#include <jni.h>
#include <errno.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif

#ifdef L2D_TARGET_IPHONE_ES2
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#endif

#ifdef L2D_TARGET_WIN_GL
#include <windows.h>
#include <GL/gl.h>
#endif

//------------ LIVE2D NAMESPACE ------------
namespace live2d
{
	class ClippingManagerOpenGL;
	class ClipContext;

	
	class DrawParam_OpenGLES2 : public DrawParam
	{
	public:
		DrawParam_OpenGLES2();
		DrawParam_OpenGLES2(bool drawParamForSetupClip);

		virtual ~DrawParam_OpenGLES2();

	public:
		virtual void drawTexture( int textureNo  , int indexCount , int vertexCount , l2d_index * indexArray
								 , l2d_pointf * vertexArray , l2d_uvmapf * uvArray , float opacity, int colorCompositionType) ;

		void setTexture(int modelTextureNo, GLuint textureNo);

		LDVector<GLuint>* getTextures(){ return &textures; }

		//  新しく利用できるModelのテクスチャ番号を確保(Avatar用）
		virtual int generateModelTextureNo() ;

		//  Modelのテクスチャ番号を生成(Avatar用）
		virtual void releaseModelTextureNo(int no) ;

		virtual void setupDraw();

		ClipContext *getClipBufPre_clipContextMask();
		ClipContext *getClipBufPre_clipContextDraw();

	private:
		// Prevention of copy Constructor
		DrawParam_OpenGLES2( const DrawParam_OpenGLES2 & ) ;
		DrawParam_OpenGLES2& operator=( const DrawParam_OpenGLES2 & ) ;

	private:
		static const int DUMMY_GL_TEXTURE_NO = 9999 ;	//  generateした番号に立てるフラグ

	private:
		live2d::LDVector<GLuint>		textures ;				//  Destructorでテクスチャの解放は行わない。基本的に外部で行う。

		static bool 				first_initGLFunc ;


#ifdef L2D_TARGET_ANDROID_ES2
	public:
		static void setExtShaderMode( bool extMdoe , bool extPAMode = false ) ;
		static void reloadShader();
#endif

#ifdef L2D_TARGET_IPHONE_ES2

#endif

#ifdef L2D_TARGET_WIN_GL
	public:
		static void initGLFunc() ;
		static bool initGLFuncSuccess ;
	private:
		static void* my_wglGetProcAddress( const char* name ) ;//for Windows
		int ___checkError___(const char* str) ;
#endif

	protected:
		virtual bool enableDrawTexture(int textureNo, int vertexCount, l2d_index * indexArray,
			l2d_pointf * vertexArray, l2d_uvmapf * uvArray, float opacity, int colorCompositionType);

	};

}
//------------ LIVE2D NAMESPACE ------------

#endif		// L2D_GRAPHICS_OPENGL_ES2
#endif		// __SKIP_DOC__
#endif		// __LIVE2D_DRAWPARAM_OPENGLES2_H__
