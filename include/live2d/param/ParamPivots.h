/**
 * ParamPivots.h
 *
 *
 * 補間可能な IDrawData / IBaseDataで利用される
 * パラメータごとに、ピボットをとる値を設定する。
 *
 * 例） ANGLE_X ( 0 ) => pivots { -30 , 0 , 30 }
 *
 * エディタ以外では静的な値となる。
 *
 * 但し、描画用の一時データを保持することもある。
 *
 * 
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */
#ifndef __LIVE2D_PARAMPIVOTS_H__
#define __LIVE2D_PARAMPIVOTS_H__


#ifndef __SKIP_DOC__


#include "../Live2D.h"
#include "../type/LDVector.h"
#include "../ModelContext.h"
#include "../io/ISerializableV2.h"


//------------ LIVE2D NAMESPACE ------------
namespace live2d
{
	class ParamID ;
	
	/**********************************************************************************************
	@brief	パラメータのキーの管理クラス
	**********************************************************************************************/
	class ParamPivots : public ISerializableV2 
	{
	public:
		static const int PARAM_INDEX_NOT_INIT = -2 ;	// 値が存在しない場合が -1を取るので -2
	
	public:
		ParamPivots();
		virtual ~ParamPivots();

	public:
		virtual void readV2(BReader & br , MemoryParam* memParam ) ;
	
	public:
		
		inline int getParamIndex( int initVersion )
		{ 
			if( this->indexInitVersion != initVersion ) 
			{
				_paramIndex = PARAM_INDEX_NOT_INIT ;
			}
	
			return _paramIndex ;
		}

		
		inline void setParamIndex_(int index , int initVersion )
		{ 
			this->_paramIndex = index ;
			this->indexInitVersion = initVersion ;
		}
	
		
		inline ParamID * getParamID() const { return paramID ; }
	
		
		inline void setParamID(ParamID * v){ paramID = v ; }
	
		
		inline int getPivotCount(){ return pivotCount ; }
	
	
		
		inline l2d_paramf * getPivotValue(){ return pivotValue ; } 
	
		
		inline void setPivotValue(int _pivotCount , l2d_paramf * _values)
		{
			this->pivotCount = _pivotCount ;
			this->pivotValue = _values ; 
		} 
	
		
		inline int getTmpPivotIndex(){ return tmpPivotIndex ; } 
	
		
		inline void setTmpPivotIndex(int v){ tmpPivotIndex = v ; } 
	
		
		inline float getTmpT(){ return tmpT ; } 
	
		
		inline void setTmpT(float t){ tmpT = t ; } 
	
		
#if L2D_SAMPLE
		void DUMP() ;
#endif
	
	
	private:
		//Prevention of copy Constructor
		ParamPivots( const ParamPivots & ) ;			
		ParamPivots& operator=( const ParamPivots & ) ;
	
		
	private:
		//---- Field ----
		int				pivotCount ;
		ParamID*		paramID ;			// パラメータIDは解放しなくてよい
		l2d_paramf*		pivotValue ;		// ld_paramfの配列[] 複数の値をピボットに持つ（外部で管理するためdelete不要)
		
		// ---- 計算時の一時情報
		int				_paramIndex ;		// 初期化前はPARAM_INDEX_NOT_INIT
		int				indexInitVersion ;	// indexをキャッシュした時の modelDrawContextのバージョン
		
		int				tmpPivotIndex ;		// 一時ピボット
		float			tmpT ;				// tmpPivotIndex , tmpPivotIndex+1 の内分比　（0の場合は内分しない）
	};
} 
//------------ LIVE2D NAMESPACE ------------

#endif // __SKIP_DOC__

#endif		// __LIVE2D_PARAMPIVOTS_H__
