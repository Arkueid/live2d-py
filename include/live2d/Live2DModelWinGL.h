/**
 *  Live2DModelWinGL.h
 *
 *  外部から参照されるモデルデータ
 * 
 *  内部の構造は ModelImpl
 * 
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */


#ifndef __LIVE2D_L2DMODEL_WINGL_H__
#define __LIVE2D_L2DMODEL_WINGL_H__

#include "Live2D.h"
#ifdef L2D_TARGET_WIN_GL


#include "ALive2DModel.h"
#include "model/ModelImpl.h"
#include "graphics/DrawParam_OpenGLES2.h"

//--------- LIVE2D NAMESPACE ------------
namespace live2d 
{


	class ModelContext ;


	
	class Live2DModelWinGL : public live2d::ALive2DModel
	{
	public:
		// Constructor
		Live2DModelWinGL(void) ;

		// Destructor
		virtual ~Live2DModelWinGL(void) ;

	public:
		
		// 更新
		virtual void update();
		
		// 描画
		virtual void draw() ;

		// テクスチャのセット
		void setTexture( int textureNo , GLuint openGLTextureNo ) ;
		
		// モデルのロード
		static Live2DModelWinGL * loadModel( const live2d::LDString & filepath ) ;
		// モデルのロード
		static Live2DModelWinGL * loadModel( const void * buf , int bufSize ) ;

		// 新しく利用できるModelのテクスチャ番号を確保(Avatar用)
		virtual int generateModelTextureNo() ;
		
		// Modelのテクスチャ番号を生成(Avatar用)
		virtual void releaseModelTextureNo(int no) ;
		
		// 描画用パラメータを取得
		virtual live2d::DrawParam* getDrawParam(){ return drawParamWinGL ; }

		void setMatrix( float*  matrix ) { this->drawParamWinGL->setMatrix(matrix) ; }
	private:
		//Prevention of copy Constructor
		Live2DModelWinGL( const Live2DModelWinGL & ) ;
		Live2DModelWinGL& operator=( const Live2DModelWinGL & ) ;
		
	private:
		//---- field ----
		live2d::DrawParam_OpenGLES2* 		drawParamWinGL;		//

	};
}
//--------- LIVE2D NAMESPACE ------------

#endif		//L2D_TARGET_WIN_GL

#endif		// __LIVE2D_L2DMODEL_WINGL_H__
