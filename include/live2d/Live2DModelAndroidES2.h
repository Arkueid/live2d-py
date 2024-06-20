/*
 *  Live2DModelAndroidES2.h
 *
 *  外部から参照されるモデルデータ
 *
 *  内部の構造は ModelImpl
 *
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */
#ifndef __LIVE2D_L2DMODEL_ANDROIDES2_H__
#define __LIVE2D_L2DMODEL_ANDROIDES2_H__



#include "Live2D.h"				// プラットフォーム別設定の前で必ず呼び出すこと

#ifdef L2D_TARGET_ANDROID_ES2	// 以下をAndroidES2専用とする
#include "ALive2DModel.h"
#include "model/ModelImpl.h"
#include "graphics/DrawParam_OpenGLES2.h"


//--------- LIVE2D NAMESPACE ------------
namespace live2d
{

	class ModelContext ;


	
	class Live2DModelAndroidES2 : public live2d::ALive2DModel
	{
	public:
		// Constructor
		Live2DModelAndroidES2(void) ;

		// Destructor
		virtual ~Live2DModelAndroidES2(void) ;

	public:
		void setMatrix( float* matrix4x4 )
		{
			drawParamAndroid->setMatrix(matrix4x4);
		}
		
		// 更新
		virtual void update();

		// 描画
		virtual void draw() ;

		// テクスチャセット
		void setTexture( int textureNo , GLuint openGLTextureNo ) ;

		// モデルロード(ファイルパス指定)
		static Live2DModelAndroidES2 * loadModel( const live2d::LDString & filepath ) ;
		// モデルロード(バッファ指定)
		static Live2DModelAndroidES2 * loadModel( const void * buf , int bufSize ) ;

		// 新しく利用できるModelのテクスチャ番号を確保(Avatar用)
		virtual int generateModelTextureNo() ;

		// Modelのテクスチャ番号を生成(Avatar用)
		virtual void releaseModelTextureNo(int no) ;

		// 描画用パラメータを取得
		virtual live2d::DrawParam* getDrawParam(){ return drawParamAndroid ; }

	private:
		// Prevention of copy Constructor
		Live2DModelAndroidES2( const Live2DModelAndroidES2 & ) ;
		Live2DModelAndroidES2& operator=( const Live2DModelAndroidES2 & ) ;
	private:
		//---- field ----
		live2d::DrawParam_OpenGLES2 * drawParamAndroid ;

	};
}
//--------- LIVE2D NAMESPACE ------------


#endif//L2D_TARGET_ANDROID_ES2

#endif		// __LIVE2D_L2DMODEL_ANDROIDES2_H__
