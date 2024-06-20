/*
 *  Live2DModelIPhoneES2.h
 *
 *  外部から参照されるモデルデータ
 * 
 *  内部の構造は ModelImpl
 * 
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */
#ifndef __LIVE2D_L2DMODE_IPHONEES2_H__
#define __LIVE2D_L2DMODE_IPHONEES2_H__

#include "Live2D.h"
#ifdef L2D_TARGET_IPHONE_ES2


#include "ALive2DModel.h"
#include "model/ModelImpl.h"
#include "graphics/DrawParam_OpenGLES2.h"

//--------- LIVE2D NAMESPACE ------------
namespace live2d 
{

	class ModelContext ;


	
	class Live2DModelIPhoneES2 : public live2d::ALive2DModel
	{
	public:
		// Constructor
		Live2DModelIPhoneES2(void) ;

		// Destructor
		virtual ~Live2DModelIPhoneES2(void) ;

	public:
		
		// 更新
		virtual void update();
		
		// 描画
		virtual void draw() ;

		// テクスチャのセット
		void setTexture( int textureNo , GLuint openGLTextureNo ) ;
		
		// モデルロード
		static Live2DModelIPhoneES2 * loadModel( const live2d::LDString & filepath ) ;

		// モデルロード
		static Live2DModelIPhoneES2 * loadModel( const void * buf , int bufSize ) ;

		// 新しく利用できるModelのテクスチャ番号を確保(Avatar用)
		virtual int generateModelTextureNo() ;
		
		// Modelのテクスチャ番号を生成(Avatar用)
		virtual void releaseModelTextureNo(int no) ;
		
		
		virtual live2d::DrawParam* getDrawParam(){ return drawParamIPhone ; }

		void setMatrix( float* matrix4x4 );

	private:
		// Prevention of copy Constructor
		Live2DModelIPhoneES2( const Live2DModelIPhoneES2 & ) ;				
		Live2DModelIPhoneES2& operator=( const Live2DModelIPhoneES2 & ) ; 	
		
	private:
		//---- field ----
		live2d::DrawParam_OpenGLES2 * 		drawParamIPhone ;				// iPhone用描画パラメータ格納用

	};
}
//--------- LIVE2D NAMESPACE ------------

#endif//L2D_TARGET_IPHONE_ES2

#endif		// __LIVE2D_L2DMODE_IPHONEES2_H__
