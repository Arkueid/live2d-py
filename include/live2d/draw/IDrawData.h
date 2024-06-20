/**
 *  IDrawData.h
 *
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */

#ifndef __LIVE2D_IDRAWDATA_H__
#define __LIVE2D_IDRAWDATA_H__


#ifndef __SKIP_DOC__

#include "../Live2D.h"
#include "../io/ISerializableV2.h"
#include "../id/BaseDataID.h"
#include "IDrawContext.h"
#include <stdio.h>


//------------ LIVE2D NAMESPACE ------------
namespace live2d
{
	class DrawDataID ;
	class ModelContext ;
	class PivotManager ;
	class DrawParam ;

	
	class IDrawData : public ISerializableV2
	{
	public:
		static const int BASE_INDEX_NOT_INIT 	= -2 ;
		static const int DEFAULT_ORDER 			= 500 ;
		static const int TYPE_DD_TEXTURE 		= 2 ;
	public:	
		IDrawData();
		virtual ~IDrawData();
	
		virtual void readV2(BReader & br , MemoryParam* memParam ) ;
		
		// オーバーライド不要
		inline BaseDataID * getTargetBaseDataID(){ return targetBaseDataID ; } 
		inline void setTargetBaseDataID(BaseDataID *id){ targetBaseDataID = id ; } 
	
		bool needTransform()
		{ 
			return ( targetBaseDataID 
					&& (targetBaseDataID != BaseDataID::DST_BASE_ID() ) ) ;
		}
	
		//  描画ID取得		
		inline DrawDataID * getDrawDataID(){ return drawDataID ; } 

		//  描画IDをセット
		inline void setDrawDataID( DrawDataID *id ){ drawDataID = id ; } 
	
		//  不透明度取得	
		inline float getOpacity( ModelContext &mdc , IDrawContext* cdata ){ return cdata->interpolatedOpacity ; }
	
		//  描画オーダー取得	
		inline int getDrawOrder(ModelContext &mdc, IDrawContext* cdata){ return cdata->interpolatedDrawOrder; }

		// クリッピングマスクの取得(SDK2.1)
		inline LDVector<DrawDataID*>* getClipIDList(){ return clipIDList; }
		
		inline void setDrawOrder(LDVector<int>* orders )
		{
			for( int i = (int)orders->size()-1 ; i >= 0  ; --i )
			{
				int order = (*orders)[i] ;

				if( order < totalMinOrder ) 		totalMinOrder = order ;
				else if( order > totalMaxOrder ) 	totalMaxOrder = order ;// 初期値 を持っている場合は else ifでよい
			}
		} 
	
		
		inline static int getTotalMinOrder(){ return totalMinOrder ; } 
		inline static int getTotalMaxOrder(){ return totalMaxOrder ; } 
		
		// 各処理（setupInterpolation/draw）で毎回やる必要のない処理を最初に行う
		// 各種パラメータが設定された後に呼び出す
		virtual IDrawContext* init(ModelContext &mdc) = 0;
		
		// drawの前段階として、補間を行う
		virtual void setupInterpolate(ModelContext &mdc , IDrawContext* cdata ) ;
		
		// drawの前段階として、変形を行う
		virtual void setupTransform(ModelContext &mdc , IDrawContext* cdata ) ;
	
		// クリッピングなどの前処理描画
		virtual void preDraw(DrawParam& dp, ModelContext& mdc, IDrawContext* cdata) = 0;

		// 描画を行う
		virtual void draw( DrawParam & dp , ModelContext &mdc , IDrawContext* cdata ) = 0 ;
	
		// IDrawDataの型を返す。TYPE_DD_TEXTUREなど
		virtual int getType() ;
		
		// デバイスロスト時にデータを破棄する
		virtual void deviceLost( IDrawContext* drawContext ) {}
	
		virtual void setZ_TestImpl( ModelContext &mdc , IDrawContext* _cdata , float z ){}
	
#ifdef L2D_DEBUG
		virtual void dump() = 0 ;
#endif
	
	private:
		//Prevention of copy Constructor
		IDrawData( const IDrawData & ) ;
		IDrawData& operator=( const IDrawData & ) ; 	
	
	private:
		static int			totalMinOrder ;		//  実行時のデータ確保用(transient)
		static int			totalMaxOrder ;		//  実行時のデータ確保用(transient)
	
	protected:
		PivotManager *		pivotManager ;
		int 				averageDrawOrder ;	//  描画順を変更しない方式で描く場合（__L2D_VERSION_STR__ 801以降は使わない）
		LDVector<DrawDataID*>*	clipIDList;				//  SDK2.1 クリッピングする描画オブジェクトのID

	private:
		DrawDataID *		drawDataID ;		//  解放しない
		BaseDataID *		targetBaseDataID ;	//  解放しない
		LDVector<int>*		pivotDrawOrder ;	//  補間用のpivotを全て持つ頂点配列 [numPivots]
		LDVector<float>*	pivotOpacity ;		//  補間用のpivotを全て持つ頂点配列 [numPivots]
	
		bool				dirty ;				//  更新されてinit()を呼び出す必要がある場合（初期状態も）
	};
}
//------------ LIVE2D NAMESPACE ------------

#endif // __SKIP_DOC__

#endif	// __LIVE2D_IDRAWDATA_H__
