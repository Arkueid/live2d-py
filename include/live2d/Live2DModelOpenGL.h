/*
 *  Live2DModelOpenGL.h
 *
 *  外部から参照されるモデルデータ
 * 
 *  内部の構造は ModelImpl
 * 
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */


#ifndef __LIVE2D_L2DMODEL_OPENGL_H__
#define __LIVE2D_L2DMODEL_OPENGL_H__

#include "Live2D.h"

#if  defined(L2D_TARGET_IPHONE_ES2) 	\
		|| defined(L2D_TARGET_ANDROID_ES2) 	\
		|| defined(L2D_TARGET_WIN_GL) 	

#include "ALive2DModel.h"
#include "model/ModelImpl.h"

#include "graphics/DrawParam_OpenGLES2.h"


//--------- LIVE2D NAMESPACE ------------
namespace live2d 
{


	class ModelContext ;


	
	class Live2DModelOpenGL : public live2d::ALive2DModel
	{
	public:
		// Constructor
		Live2DModelOpenGL(void) ;

		// Destructor
		virtual ~Live2DModelOpenGL(void) ;

	public:
		
		// 更新
		virtual void update();
		
		// 描画
		virtual void draw() ;

		// テクスチャのセット
		void setTexture( int textureNo , unsigned int openGLTextureNo ) ;
		
		// モデルのロード
		static Live2DModelOpenGL * loadModel( const live2d::LDString & filepath ) ;
		// モデルのロード
		static Live2DModelOpenGL * loadModel( const void * buf , int bufSize ) ;

		// 新しく利用できるModelのテクスチャ番号を確保(Avatar用)
		virtual int generateModelTextureNo() ;
		
		// Modelのテクスチャ番号を生成(Avatar用)
		virtual void releaseModelTextureNo(int no) ;
		
		// 描画用パラメータを取得
		virtual live2d::DrawParam* getDrawParam(){ return drawParamOpenGL; }

		void setMatrix( float*  matrix ) ;

		void setTextureColor(int textureNo, float r, float g, float b, float scale);

	private:
		//Prevention of copy Constructor
		Live2DModelOpenGL( const Live2DModelOpenGL & ) ;
		Live2DModelOpenGL& operator=( const Live2DModelOpenGL & ) ;
		
	private:
		//---- field ----
		live2d::DrawParam_OpenGLES2* 		drawParamOpenGL ;		//

	};
}
//--------- LIVE2D NAMESPACE ------------

#endif		//L2D_TARGET_WIN_GL

#endif		// __LIVE2D_L2DMODEL_OPENGL_H__
