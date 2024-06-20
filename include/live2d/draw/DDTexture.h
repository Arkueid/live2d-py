/**
 *  DDTexture.h
 *
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */
#ifndef __LIVE2D_DDTEXTURE_H__
#define __LIVE2D_DDTEXTURE_H__


#ifndef __SKIP_DOC__

#include "../Live2D.h"
#include "IDrawData.h"
#include "../param/PivotManager.h"
#include "IDrawContext.h"

#ifdef L2D_TARGET_D3D
# include <d3dx9.h>
# include "../graphics/DrawParam_D3D.h"
#endif

#ifdef L2D_TARGET_D3D11
#include <d3d11.h>
# include "../graphics/DrawParam_D3D11.h"
#endif

#ifdef L2D_TARGET_PS4
#include <gnmx.h>
#endif

//------------ LIVE2D NAMESPACE ------------
namespace live2d
{ 
	class DDTextureContext ;
	
	
	
	class DDTexture : public IDrawData
	{
	public:
		static const int OPTION_FLAG_BARCODE_KANOJO_COLOR_CONVERT = 0x1 ;// バーコードカノジョで色変を行う要素の場合 
		
		static const int MASK_COLOR_COMPOSITION = 0x1E ;//011110 
	
		static const int COLOR_COMPOSITION_NORMAL  = 0 ;// 通常
		static const int COLOR_COMPOSITION_SCREEN  = 1 ;// スクリーン
		static const int COLOR_COMPOSITION_MULTIPLY= 2 ;// 乗算
	
		static int INSTANCE_COUNT ;
	
	public:
		DDTexture() ;
		virtual ~DDTexture(void) ;
		
	public:
		virtual void readV2(BReader & br , MemoryParam* memParam ) ;

		void initDirect(MemoryParam* memParam) ;
	
		//  指定したテクスチャ番号にセット
		void setTextureNo(int no){ this->textureNo = no ; }
	
		//  テクスチャ番号取得
		int getTextureNo(){ return this->textureNo ; }
		
		//  uv配列 [ numPts*2 ]
		l2d_uvmapf * getUvMap(){ return this->uvmap ; }
		
		//  点数を返す
		int getNumPoints(){ return this->numPts ; }
	
		//  ポリゴン数を返す
		int getNumPolygons(){ return this->numPolygons ; }
		
		virtual IDrawContext* init(ModelContext &mdc) ;
		
		virtual void setupInterpolate(ModelContext &mdc , IDrawContext* cdata ) ;
		
		virtual void setupTransform(ModelContext &mdc , IDrawContext* cdata ) ;
	
		virtual void preDraw(DrawParam& dp, ModelContext& mdc, IDrawContext* cdata) ;

		virtual void draw( DrawParam & dp , ModelContext &mdc , IDrawContext* cdata ) ;
	
	#ifdef L2D_TARGET_D3D
	
		void setupBufD3D( DrawParam_D3D& dpD3D , ModelContext &mdc , DDTextureContext* cdata ) ;
	
		virtual void deviceLost( IDrawContext* drawContext ) ;
	#endif
	#ifdef L2D_TARGET_D3D11

		void setupBufD3D(DrawParam_D3D11& dpD3D, ModelContext &mdc, DDTextureContext* cdata);

		virtual void deviceLost(IDrawContext* drawContext);
	#endif

		//  テクスチャタイプ取得
		virtual int getType(){ return TYPE_DD_TEXTURE ; }// 
	
		int getOptionFlag(){ return this->optionFlag ; }
	
		virtual void setZ_TestImpl( ModelContext &mdc , IDrawContext* _cdata , float z ) ;
	
		l2d_index*  getIndexArray(int* polygonCount);

	#if AVATAR_OPTION_A
		//  色取得	
		int getOption_KanojoColor(){ return this->optionKanojoColor ; }
	
	#endif
	
	#ifdef L2D_DEBUG
		virtual void dump() ;
	#endif
	
	private:
		// Prevention of copy Constructor
		DDTexture( const DDTexture & ) ;				
		DDTexture& operator=( const DDTexture & ) ;		
	
	private:
		int						textureNo ;				// 使用するテクスチャ番号（モデルの管理するテクスチャ番号）
		int						numPts ;				// 頂点の数（PSPのsceGumDrawArray では 65535までになる）
		int						numPolygons ;			// ポリゴンの数
		int						optionFlag ;			// オプション設定
		
		// --- 必要に応じて設定されるオプション値
	#if AVATAR_OPTION_A
		int						optionKanojoColor ;		// Avatarのオプション（optionFlag & DDMorph.OPTION_FLAG_BARCODE_KANOJO_COLOR_CONVERTが立っている場合）
	#endif
		
		l2d_index *				indexArray ;			// 頂点インデックス配列 [numPolygons*3]
	
		//  points ( Pivot配列 )のルール ( 例  xnum=3 , ynum=4 , znum=2 のパラメータの場合 )
		//  [x0,y0,z0],[x1,y0,z0],[x2,y0,z0]  , [x0,y1,z0],[x1,y1,z0],[x2,y1,z0].. となる
		
		LDVector<l2d_paramf*>*	pivotPoints ;			// 補間用のpivotを全て持つ頂点配列 [numPivots][numPts*2]
		l2d_uvmapf *			uvmap ;					// uv配列 [ numPts*2 ]
		int						colorCompositionType ;	// 色合成のタイプ
		bool					culling ;				// 1.0beta3
	};
	
	
	class DDTextureContext : public IDrawContext 
	{
	public:
		DDTextureContext(IDrawData* src) ;
		virtual ~DDTextureContext() ;
		
	public:		
		// 変換済みの点列を返す 
		// pointCount は点の数が返る（点（x,y）がpointCount個並ぶので、pointCount*2の配列となる）
		l2d_pointf* getTransformedPoints( int* pointCount ) ;
	
	public:		
		int 				tmpBaseDataIndex ;		// IBaseDataのインデックス。不変なので一度取得すればよい
		l2d_pointf * 		interpolatedPoints ;	// 補間された点列 	PSPの場合は、[numPts*5]　/ OpenGLの場合は [numPts*2]( texture x,y , 補間・変形された後の点 x,y , z=0 ) 
		l2d_pointf * 		transformedPoints ;		// 変形された点列
		l2d_pointf * 		drawPoints ;			// 描画用の点列（ダブルバッファ用の場合はスワップして使われる）0.9.00b14
		
		unsigned char 		not_updated_count ;		// 更新されなかった回数をカウントする\n１回目と２回目で意味を持つ。\ndouble bufferのとき二回更新されなければそのまま前の値を使える
		
	#ifdef L2D_TARGET_D3D
		LPDIRECT3DVERTEXBUFFER9 		pUvBuf ; // Uv配列
		LPDIRECT3DINDEXBUFFER9 			pIndexBuf ; // インデックス配列
	#endif

	#ifdef L2D_TARGET_D3D11
		ID3D11Buffer* pUvBuf;		// テクスチャuvバッファ
		ID3D11Buffer* pIndexBuf;	// インデックスバッファ
	#endif

	#ifdef L2D_TARGET_PS4
		bool initializedBuffer;
		float* color;//透明度含めた掛け合わせる色。float[4]。シェーダ用
		sce::Gnm::Buffer buffers[2];//position , textureUV 
	#endif
	} ;
} 
//------------ LIVE2D NAMESPACE ------------

#endif // __SKIP_DOC__

#endif	// __LIVE2D_DDTEXTURE_H__

