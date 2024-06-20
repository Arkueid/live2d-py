/**
 *  ModelContext.h
 * 
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */
#ifndef __LIVE2D_MODEL_CONTEXT_H__
#define __LIVE2D_MODEL_CONTEXT_H__


#include "Live2D.h"
#include "memory/LDObject.h"
#include "type/LDVector.h"
#include "DEF.h"

#if defined(L2D_TARGET_WIN_GL) || defined(L2D_TARGET_ANDROID_ES2) || defined(L2D_TARGET_IPHONE_ES2)
#include "graphics/ClippingManagerOpenGL.h"
#endif

#if L2D_VERBOSE
#include "util/UtDebug.h"
#endif

//------------ LIVE2D NAMESPACE ------------
namespace live2d
{ 
	
	class ParamID ;
	class BaseDataID ;
	class PartsDataID ;
	class DrawDataID ;
	class IBaseData ;
	class IDrawData ;
	class PartsData ;
	class ALive2DModel ;
	class DrawParam ;
	class IBaseContext ;
	class IDrawContext ;
	class PartsDataContext ;
		
	#define _PARAM_FLOAT_MIN_ (-1000000) 
	#define _PARAM_FLOAT_MAX_ ( 1000000) 
	
	
	class ModelContext : public live2d::LDObject 
	{
	public:
		static const unsigned short NOT_USED_ORDER ; 
		static const unsigned short NO_NEXT ; 
	
	
	public:
		ModelContext(ALive2DModel *model) ;
		virtual ~ModelContext(void) ;
	
	public:
		// 解放
		void release() ;
		
		// 初期化。setup()の前に一度だけ呼ぶ。構造が変わった場合も呼ぶ。
		void init() ;
	
		// メモリを一括管理するためのMemoryParamを返す
		MemoryParam*  getMemoryParam(){ return memoryManagement ; }
	
		// 初期化のバージョン情報取得（再初期化されるとキャッシュが無効になるため）
		inline int getInitVersion() const { return initVersion ; }
	
		inline bool requireSetup() const { return needSetup ; }
		
		// 現在のパラメータ値で描画用のデータを生成する
		bool update() ;
	
		// クリッピング用のマスクを生成
		void preDraw(DrawParam &dp) ;
	
		// 描画
		void draw(DrawParam &dp) ;
	
	
				
		inline bool isParamUpdated( int paramIndex ) const { 
# if L2D_FORCE_UPDATE
			return true ;
# else
			return (*updatedFloatParamFlagsPtr)[ paramIndex ] == PARAM_UPDATED ; 
# endif
		}

		// paramIDからアクセス用のparamIndexを取得
		int getParamIndex( ParamID * paramID )   ;

		// BaseDataIDからアクセス用の baseIndex を取得する
		int getBaseDataIndex( BaseDataID * baseID ) ;

		// PartsDataIDからアクセス用の partsIndex を取得する
		int getPartsDataIndex( PartsDataID * partsID ) ;

		// drawDataIDからアクセス用の drawDataIndex を取得する
		int getDrawDataIndex( DrawDataID * drawDataID ) ;
	
		inline unsigned short * getTmpPivotTableIndicesRef(){ return tmpPivotTableIndices_short ; }
		inline float * getTmpT_ArrayRef(){ return tmpT_array ; }
		
	
		// floatパラメータ追加
		int addFloatParam( ParamID * id , l2d_paramf value , l2d_paramf min , l2d_paramf max ) ;
	
	
		void setBaseData( unsigned int baseDataIndex , IBaseData * baseData ) ;
	
	
		void setParamFloat( unsigned int paramIndex , l2d_paramf value ) ;
	
		float getParamMax( unsigned int paramIndex ){ return (*floatParamMaxListPtr)[paramIndex] ; }
		float getParamMin( unsigned int paramIndex ){ return (*floatParamMinListPtr)[paramIndex] ; }
		
		// 前回saveParamしたパラメータ値を復元（saveParamされていなければ何もしない）
		void loadParam() ;

		// 現在のパラメータ値を保存
		void saveParam() ;
	
		// パーツの不透明度をセット
		void setPartsOpacity( int partIndex , float opacity ) ;
	
		// パーツの不透明度取得
		float getPartsOpacity( int partIndex ) ;
		
	
		
		inline IBaseData* getBaseData( unsigned int baseDataIndex )
		{

	# if L2D_RANGE_CHECK		
			if( baseDataIndex >= baseDataListPtr->size() )
			{
	
				L2D_THROW( "out of range ModelDrawContext@getBaseData()" ) ;
			}
	# endif
			return (*baseDataListPtr)[ baseDataIndex ] ;// 範囲外は未定義（外部でチェックする）
		}

		
		inline IDrawData* getDrawData( unsigned int drawDataIndex )
		{
	# if L2D_RANGE_CHECK		
			if( drawDataIndex >= (*drawDataListPtr).size() )
			{
	
				L2D_THROW( "out of range ModelDrawContext@getBaseData()" ) ;
			}
	# endif
			return (*drawDataListPtr)[ drawDataIndex ] ;// 範囲外は未定義（外部でチェックする）
		}
	
		
		inline IBaseContext* getBaseContext( unsigned int baseDataIndex )
		{
			return (*baseContextListPtr)[ baseDataIndex ] ;// 範囲外は未定義（外部でチェックする）
		}

		
		inline IDrawContext* getDrawContext( unsigned int drawDataIndex )
		{
			return (*drawContextListPtr)[ drawDataIndex ] ;// 範囲外は未定義（外部でチェックする）
		}

		
		inline PartsDataContext* getPartsContext( unsigned int partsDataIndex )
		{
			return (*partsContextListPtr)[ partsDataIndex ] ;// 範囲外は未定義（外部でチェックする）
		}

		
		inline int getBaseDataCount(){	return (int)baseDataListPtr->size() ; }

		
		inline int getDrawDataCount(){	return (int)drawDataListPtr->size() ; }

		
		inline int getPartsDataCount(){	return (int)partsDataListPtr->size() ; }
		
		// partsデータを取得します
		inline l2d_paramf getParamFloat( unsigned int paramIndex )
		{

	# if L2D_RANGE_CHECK
			if( paramIndex >= floatParamListPtr->size() )
			{
	
				L2D_THROW( "out of range ModelDrawContext@getParamFloat()" ) ;
			}
	# endif
			return (*floatParamListPtr)[ paramIndex ] ;
		}
		
		// DirectXデバイスロスト時に呼び出す
		void deviceLost( ) ;
		
		/*
		 * ZBufferを設定する
		 *
		 * 通常描画には必要ないのでZ値は設定されないが、
		 * フェードイン・アウトなどの透明度を描画する場合に使います。
		 * update()とdraw()の間で呼び出して下さい。
		 * 
		 * startZ	一番背面にあるポリゴンのZ値
		 * stepZ	Z値を更新するステップ幅
		 * 
		 */
		void updateZBuffer_TestImpl( float startZ , float stepZ ) ;
	
	#if L2D_SAMPLE
		void DUMP_PARAMS() ;
	#endif
	
	private:
		enum { PARAM_NOT_UPDATED = 0,	PARAM_UPDATED	};
		
		//Prevention of copy Constructor
		ModelContext( const ModelContext & ) ;				
		ModelContext& operator=( const ModelContext & ) ; 	

		void initValue();
		
	private:	
		bool 							needSetup ;						// 最初のsetupがすんでいないだけ場合 true
		ALive2DModel * 					model ;							// 
	
	
		int 							initVersion ;					// initを行ったバージョン。このバージョンが異なる場合は、キャッシュされたindex値などが無効とみなす
		// ------------ パラメータ値（float） ------------
		LDVector<ParamID *>*			floatParamIDListPtr ;			// 解放しない
		LDVector<l2d_paramf>*			floatParamListPtr ;				// float 型のパラメータ値テーブル（paramIndexでアクセスする）
		LDVector<l2d_paramf>*			lastFloatParamListPtr ;			// 前回upate時のパラメータ値テーブル（paramIndexでアクセスする）0.9.00b19 2011/10/27
		
		//-- 
		LDVector<l2d_paramf>*			floatParamMinListPtr ;			// float 型のパラメータ、最小値テーブル（paramIndexでアクセスする）
		LDVector<l2d_paramf>*			floatParamMaxListPtr ;			// float 型のパラメータ、最大値テーブル（paramIndexでアクセスする）
		
		LDVector<l2d_paramf>*			savedFloatParamListPtr ;		// saveParam() により保存された状態のfloat 型のパラメータ値テーブル（paramIndexでアクセスする）
	
		// 更新されたパラメータのフラグを立てる  updatedFloatParamFlags[paramIndex] = 0 or 1(updated)
		LDVector<char>*					updatedFloatParamFlagsPtr ;		// boolフラグは良くないのでunsigned char
		
		// ------------ setup効率化のための ------------
		// 座標変換 
		// 	予め依存関係の無い順序で並べておくことで、setup時のOnDemandBuilderを不要にする
		// 	indexでアクセスするためinit()以外で順序変更はしない
		LDVector<IBaseData*>*			baseDataListPtr ;				// 解放しない(Model以下に所属）
		
		// 	visibleの drawDataのみをリスト化する。setup時のオーダーなどの制御ではこの index を用いる
		LDVector<IDrawData*>*			drawDataListPtr ;				// 解放しない(Model以下に所属）
	
		LDVector<PartsData*>*			partsDataListPtr ;				// 解放しない(Model以下に所属）
		
		// --- context data (描画用の演算結果のリスト)---
		LDVector<IBaseContext*>*		baseContextListPtr ;			// 解放する
		LDVector<IDrawContext*>*		drawContextListPtr ;			// 解放する
		LDVector<PartsDataContext*>*	partsContextListPtr ;			// 解放する
	
		// オーダー管理用（末尾にメモ）
		// 添え字は同じ , 0は番兵
		LDVector<unsigned short>*		orderList_firstDrawIndexPtr ;	// order 1000 同じオーダーの最初の sub listのindex
		LDVector<unsigned short>*		orderList_lastDrawIndexPtr  ;	// order 1000 同じオーダーの最後の sub listのindex
		
		// 添え字は drawDataList と同じ
		LDVector<unsigned short>*		nextList_drawIndexPtr ;			// drawDataのindexと一致する。初期値NO_NEXT
		
		// ----------- 補間の補助用一次データ（マルチスレッド未対応） -----------
		unsigned short 					tmpPivotTableIndices_short [DEF::PIVOT_TABLE_SIZE] ;	// 現在補間しようとしている要素の各ピボット番号を記録する(最後は-1：番兵)
		float 							tmpT_array[DEF::MAX_INTERPOLATION*2] ;					// 少し大き目に確保
	
		MemoryParam*					memoryManagement ;
		AMemoryHolder*					memoryHolderFixedMain ;
		AMemoryHolder*					memoryHolderFixedGPU;

#if defined(L2D_TARGET_WIN_GL) || defined(L2D_TARGET_ANDROID_ES2) || defined(L2D_TARGET_IPHONE_ES2)
		ClippingManagerOpenGL *clipManager;
#endif
		
	};
}
//------------ LIVE2D NAMESPACE ------------

#endif		// __LIVE2D_MODEL_CONTEXT_H__
