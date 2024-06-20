/**
 *  ALive2DModel.h
 *
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */
#ifndef __LIVE2D_ALIVE_2D_MODE_H__
#define __LIVE2D_ALIVE_2D_MODE_H__


#include "type/LDVector.h"
#include "Live2D.h"
#include "memory/LDObject.h"
#include "ModelContext.h"
#include "id/ParamID.h"
#include "id/DrawDataID.h"
#include "id/PartsDataID.h"

//------------ LIVE2D NAMESPACE ------------
namespace live2d
{ 
	class ModelImpl ;
	class DrawParam ;
	
	
	class ALive2DModel : public LDObject
	{
	public:
		static const int FILE_LOAD_EOF_ERROR = 1 ;			// データの末尾を読み込めない場合のエラー
		static const int FILE_LOAD_VERSION_ERROR = 2 ;		// ロードできないバージョンのファイル
		
		static int INSTANCE_COUNT ;	
	
	
	public:	
		ALive2DModel() ;
		virtual ~ALive2DModel() ;
	
		
	
		// パラメータの値を取得する
		float getParamFloat( const char * paramID ) ;
		
		// パラメータの値を設定する
		void setParamFloat( const char * paramID , float value , float weight = 1.0f ) ;
		
		// 現在の値に対して、重みをつけて値を加える。 weight=1のときは指定した値をそのまま加える。
		void addToParamFloat( const char * paramID , float value , float weight = 1.0f ) ;
	
		// 現在の値に対して、重みをつけて値を掛け合わせる。weight=1のときは指定した値をそのまま掛け合わせる。
		void multParamFloat( const char * paramID , float mult , float weight = 1.0f  ) ;
		
		
		inline int getParamIndex( const char * paramID )
		{
			return modelContext->getParamIndex(ParamID::getID(paramID)) ;
		}
	
		
		inline int getParamIndex( ParamID* paramID )
		{
			return modelContext->getParamIndex( paramID ) ;
		}	

		inline float getParamFloat( int paramIndex )
		{
			return modelContext->getParamFloat( paramIndex ) ;
		}
	
		inline void setParamFloat( int paramIndex , float value , float weight = 1.0f)
		{
			if(weight==1)modelContext->setParamFloat( paramIndex , value ) ;
			else modelContext->setParamFloat( paramIndex ,  modelContext->getParamFloat( paramIndex )*(1-weight) + value*weight ) ;
		}
	
		inline void addToParamFloat( int paramIndex, float value , float weight = 1.0f)
		{
			modelContext->setParamFloat( paramIndex , modelContext->getParamFloat( paramIndex ) + value*weight ) ;
		}
	
		inline void multParamFloat( int paramIndex , float mult , float weight = 1.0f)
		{
			modelContext->setParamFloat( paramIndex , modelContext->getParamFloat( paramIndex )*(1.0f + (mult - 1.0f)*weight ) ) ;
		}
	
		// 前回saveParamしたときのパラメータ値を復元する（saveParamされていなければ何もしない）
		void loadParam() ;

		// 現在のパラメータ値を一時的に記憶する。loadParam()で復元できる。
		void saveParam() ;
	
		// データ構造の変更、パーツ差し替えなどを行った場合に呼ぶ
		virtual void init() ;
	
		// パラメータ変更などを行った場合に呼ぶ
		virtual void update() ;

		// 描画命令（プラットフォームごとにオーバーライドされる）
		virtual void draw()  ;
		
		// パーツの不透明度を設定
		void setPartsOpacity( const char *partsID , float opacity ) ;
		void setPartsOpacity( int partsIndex , float opacity ) ;

		// パーツの不透明度を取得
		float getPartsOpacity( const char *partsID ) ;
		float getPartsOpacity( int partsIndex ) ;
		
		// パーツの表示グループを制御（非推奨）
		void setupPartsOpacityGroup_alphaImpl( const char* paramGroup[] , float deltaTimeSec ) ;
	
		// モデルのデータ構造を設定
		void setModelImpl(ModelImpl* m) ;

		// モデルのデータ構造を取得
		ModelImpl* getModelImpl() ;
	
		
		ModelContext * getModelContext(){ return modelContext ; }

		
		int getErrorFlags(){ return Live2D::getError() ; }
		
		// 新しく利用できるLive2D内部で管理されるテクスチャ番号を確保(Avatar用)
		virtual int generateModelTextureNo() ;
		
		// Live2D内部で管理されるテクスチャ番号を解放(Avatar用)
		virtual void releaseModelTextureNo(int no) ;
	
		// キャンバスの幅を取得
		float getCanvasWidth() ;

		// キャンバスの高さを取得
		float getCanvasHeight() ;
	
		
		virtual DrawParam* getDrawParam(){ return NULL ; }
	
		
		virtual int getDrawDataIndex( const char * drawDataID )
		{
			return modelContext->getDrawDataIndex( DrawDataID::getID(drawDataID)) ;
		}
	
		
		virtual IDrawData* getDrawData( int drawIndex )
		{
			return modelContext->getDrawData( drawIndex ) ;
		}
		
		// 描画用の点列を返す。
		virtual l2d_pointf* getTransformedPoints( int drawIndex , int*pointCount) ;
		virtual l2d_index* getIndexArray( int drawIndex , int*polygonCount) ;
	
		// 【非推奨】ZBufferを設定する
		// 通常、描画には必要ないのでZ値は設定されないが、フェードイン、アウトなどの透明度を描画する場合に必要。
		void updateZBuffer_TestImpl( float startZ , float stepZ )
		{
			modelContext->updateZBuffer_TestImpl( startZ , stepZ ) ;
		}
	
		
		inline int getPartsDataIndex( const char * partsID )
		{
			return modelContext->getPartsDataIndex(PartsDataID::getID(partsID)) ;
		}
		
		
		inline int getPartsDataIndex( PartsDataID* partsID )
		{
			return modelContext->getPartsDataIndex( partsID ) ;
		}

		
		void setPremultipliedAlpha(bool b);
		bool isPremultipliedAlpha();
		
		void setAnisotropy(int n);
		int getAnisotropy();


	#if L2D_ALPHA_IMPL_LAYOUT
		// 【仮実装】モデルの配置情報を返す
		// ポインタを直接返す.内容を更新すると直接反映される
		//{ x , y , anchorX , anchorY , scaleX , scaleY }
		float* getLayout_alphaImpl(){ return layout_alphaImpl ; }
	#endif
		
	#ifdef L2D_DEBUG
		void dump() ;
	#endif
	
	protected:
		//------------ static method ------------
		// 2013/04/02 beta2 エラーフラグを返すように修正
		static l2d_uint32 loadModel_exe( ALive2DModel *ret , const LDString & filepath ) ;
		static l2d_uint32 loadModel_exe( ALive2DModel *ret , const void * buf , int bufSize ) ;
	
	private:
		//Prevention of copy Constructor
		ALive2DModel( const ALive2DModel & ) ;				
		ALive2DModel& operator=( const ALive2DModel & ) ;
	
		
	protected:
		//------------ field ------------
		ModelImpl*			modelImpl ;						// モデルのデータ構造
		ModelContext*		modelContext ;					// モデルの状態			
//		int					errorFlags ;					// エラーフラグ(1.00beta2より破棄.Live2D::setError()に統合)
	
	#if L2D_ALPHA_IMPL_LAYOUT
		float layout_alphaImpl[6] ;//{ x , y , anchorX , anchorY , scaleX , scaleY }
	#endif
		
	};

} 
//------------ LIVE2D NAMESPACE ------------

#endif		// __LIVE2D_ALIVE_2D_MODE_H__
