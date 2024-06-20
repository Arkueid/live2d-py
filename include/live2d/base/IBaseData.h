/**
 *  IBaseData.h
 * 
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */
#ifndef __LIVE2D_I_BASE_DATA_H__
#define __LIVE2D_I_BASE_DATA_H__


#ifndef __SKIP_DOC__

#include "../Live2D.h"
#include "../io/ISerializableV2.h"
#include "../id/BaseDataID.h"


//------------ LIVE2D NAMESPACE ------------
namespace live2d
{
	class ModelContext ;
	class BaseDataID ;
	class IBaseContext ;
	class PivotManager ;

	
	class IBaseData : public ISerializableV2
	{
	public:
		static const int BASE_INDEX_NOT_INIT = -2 ;
	
		// getType()で取得するタイプの定数
		static const int TYPE_BD_AFFINE = 1 ;
		static const int TYPE_BD_BOX_GRID = 2 ;
		
	public:
		IBaseData();
		virtual ~IBaseData(void);
		
	public:
			
		inline void setTargetBaseDataID(BaseDataID *id){ targetBaseDataID = id ; } 
	
		
		inline void setBaseDataID(BaseDataID *id){ baseDataID = id ; } 
		
		
		inline BaseDataID * getTargetBaseDataID() const { return targetBaseDataID ; }
	
		
		inline BaseDataID * getBaseDataID() const { return baseDataID ; }
	
		
		bool needTransform()
		{
			return ( targetBaseDataID 
					&& (targetBaseDataID != BaseDataID::DST_BASE_ID()) ) ;
		}
		
		//  各処理（setupInterpolation/draw）で毎回やる必要のない処理を最初に行う
		//  各種パラメータが設定された後に呼び出す
		virtual IBaseContext* init(ModelContext &mdc) = 0;

		//  
		virtual void readV2(BReader & br , MemoryParam* memParam ) ;
	
		//  現在のパラメータを元に、補間を行う
		virtual void setupInterpolate(ModelContext &mdc , IBaseContext* cdata ) = 0;
	
		//-------------------------------------
		/*
		 * 現在の座標系を元に、変換を行う
		 *
		 * この座標変換自体を変換し、local -> dst を直接行えるようにする
		 * tr1 : local-> local2
		 * tr2 :		 local2 -> dst
		 */
		virtual void setupTransform(ModelContext &mdc, IBaseContext* cdata ) = 0;
	
		/*
		 * 点列を変換する
		 * 
		 * numPointは x,y を合わせた数。numPoint*stepのサイズの配列が渡される必要がある
		 * 
		 * @param pt_offset , pt_step // 変換対象の配列が x,y 以外の要素を持つ場合に、 pt_offset , pt_stepを考慮して設定する
		 * 		{x,y }　の場合は pt_offset = 0 , pt_step = 2
		 * 		{texture x,y , vertext x,y,z }　の場合は pt_offset = 2 , pt_step = 5
		 * 		※配列の途中から処理するというオフセットではないので注意
		 */
		virtual void transformPoints(ModelContext &mdc, IBaseContext* cdata , l2d_pointf * srcPoints , l2d_pointf * dstPoints 
				, int numPoint , int pt_offset , int pt_step ) = 0;
	
		/*
		 * 変換の型を返す
		 * IBaseData::TYPE_BD_AFFINE ;
		 * IBaseData::TYPE_BD_BOX_GRID ;
		 * 
		 */
		virtual int getType() = 0 ;
		
#ifdef L2D_DEBUG
		virtual void dump() = 0 ;
#endif

	//SDK2.0
	protected:
		void readV2_opacity(BReader & br , MemoryParam* memParam) ;
		void interpolateOpacity(ModelContext & mdc, PivotManager * pivotMgr , IBaseContext * _data, bool* ret_paramOutside);

	private:
		//Prevention of copy Constructor
		IBaseData( const IBaseData & ) ;				
		IBaseData& operator=( const IBaseData & ) ; 
	
	private:
		BaseDataID* 			baseDataID ;			// 自身のID( 破棄しない）
		BaseDataID* 			targetBaseDataID ;		// 親のデフォーマ( 破棄しない）
	
		bool 					dirty ;					// 更新されてinit()を呼び出す必要がある場合（初期状態も）

		// SDK 2.0 (.moc ) LIVE2D_FORMAT_VERSION_V2_10_SDK2
		// 補間用のpivotを全て持つ不透明度 [numPivots]
		// 下位互換のため null の場合があり、その場合は 1.0（不透明とみなす）
		LDVector<float>*				  pivotOpacity;
	};
}
//------------ LIVE2D NAMESPACE ------------

#endif // __SKIP_DOC__

#endif	// __LIVE2D_I_BASE_DATA_H__
