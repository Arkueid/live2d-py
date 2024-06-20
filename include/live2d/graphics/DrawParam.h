/**
 * DrawParam.h
 *
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */
#ifndef __LIVE2D_DRAWPARAM_H__
#define __LIVE2D_DRAWPARAM_H__


#include "../Live2D.h"
#include "../memory/LDObject.h"
#include "../type/LDVector.h"

#include "../id/DrawDataID.h"

//------------ LIVE2D NAMESPACE ------------
namespace live2d
{
	class ClipContext;

	class TextureInfo :public LDObject{
	public:
		TextureInfo();
		float a;
		float r;
		float g;
		float b;

		float scale;
		float interpolate;

		int blendMode;
	};

	
	class DrawParam  : public live2d::LDObject 
	{
	public:
		static const int DEFAULT_FIXED_TEXTURE_COUNT = 32 ;

		static const int CLIPPING_PROCESS_NONE = 0;
		static const int CLIPPING_PROCESS_OVERWRITE_ALPHA = 1;	// マスク用前処理。アルファのみ上書き
		static const int CLIPPING_PROCESS_MULTIPLY_ALPHA = 2;	// マスク用前処理。アルファのみ乗算
		static const int CLIPPING_PROCESS_DRAW = 3;				// マスク用。DestのアルファをSrcのアルファであるかのように利用
		static const int CLIPPING_PROCESS_CLEAR_ALPHA = 4;		// マスクを掛ける前にアルファをクリア
	
	public:
		DrawParam();
		virtual ~DrawParam();
	
	public:
		//  描画のセットアップ。必要に応じてオーバーライドする。
		virtual void setupDraw(){} ;
		// 描画完了後の処理。必要に応じてオーバーライドする。
		virtual void cleanupDraw(){};
	
	
		
		virtual void drawTexture( int textureNo , int indexCount , int vertexCount , l2d_index * indexArray
				 , l2d_pointf * vertexArray , l2d_uvmapf * uvArray , float opacity, int colorCompositionType ) = 0 ;
	
	
		// 新しく利用できるModelのテクスチャ番号を確保
		virtual int generateModelTextureNo() ;
		
		// Modelのテクスチャ番号を生成
		virtual void releaseModelTextureNo(int no) ;
	
		// モデル描画の際、全体に掛け合わされる色設定。各色0..1 (1が標準の状態）
		virtual void setBaseColor( float alpha , float red , float green , float blue ) ;
	
		// カリング（片面描画）を行う場合はtrueをセットする。
		void setCulling( bool culling ){ this->culling = culling ; }
	
		//  配列は複製されるので元の配列は外で破棄して良い
		void setMatrix( float* _matrix4x4 )
		{
			for( int i = 0 ; i < 16 ; i++ )
			{
				matrix4x4[i] = _matrix4x4[i] ;
			}
		}
		float* getMatrix(){ return matrix4x4; }

		void setPremultipliedAlpha(bool enable);
		bool isPremultipliedAlpha();

		void setAnisotropy(int n);
		int getAnisotropy();

		void setTextureColor(int textureNo, float r, float g, float b, float a=1);
		void setTextureScale(int textureNo, float scale);
		void setTextureInterpolate(int textureNo, float interpolate);
		void setTextureBlendMode(int textureNo, int mode);

		bool enabledTextureInfo(int textureNo);

		void setpCurrentDrawDataID(DrawDataID* pDrawDataID);
		DrawDataID* getpCurrentDrawDataID();

		int getClippingProcess(){return clippingProcess;}
		void setClippingProcess(int v){ clippingProcess = v; }

		void setClipBufPre_clipContextForMask(ClipContext *clip);
		void setClipBufPre_clipContextForDraw(ClipContext *clip);

	protected:
		int 	fixedTexureCount ;
		
		float 	baseAlpha ;
		float 	baseRed ;
		float 	baseGreen ;
		float 	baseBlue ;
	
		bool	culling	;
		float 	matrix4x4[16] ;
		int		clippingProcess;
		
		bool 	premultipliedAlpha;
		int 	anisotropy;


		LDVector<TextureInfo*> 		texturesInfo;

		DrawDataID*	 pCurrentDrawDataID;//D3D用に現在の描画オブジェクトのインデックスを渡す必要があるので、追加。他プラットフォームでは使用せず。

		virtual bool enableDrawTexture(int textureNo, int vertexCount, l2d_index * indexArray,
			l2d_pointf * vertexArray, l2d_uvmapf * uvArray, float opacity, int colorCompositionType);



		ClipContext *clipBufPre_clipContextMask;// クリップMask用に描くときに設定される
		ClipContext *clipBufPre_clipContextDraw;// クリップDraw用に描くときに設定される

	private:
		//Prevention of copy Constructor
		DrawParam( const DrawParam & ) ;			
		DrawParam& operator=( const DrawParam & ) ;
	};
}
//------------ LIVE2D NAMESPACE ------------

#endif		// __LIVE2D_DRAWPARAM_H__

