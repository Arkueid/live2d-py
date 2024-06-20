/*
 *  IContextData.h
 *  
 *  ImmutableのDrawData、BaseDataの描画用のデータを保持する。
 *  
 *  Created by Live2D on 11/04/29.
 *  Copyright 2011 Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */
#ifndef __IBASE_CONTEXT__
#define __IBASE_CONTEXT__


#ifndef __SKIP_DOC__

#include "../memory/LDObject.h"


//------------ LIVE2D NAMESPACE ------------
namespace live2d
{ 
	class IBaseData ;
		
	class IBaseContext : public live2d::LDObject 
	{
	public:
		IBaseContext(IBaseData* src) ;
		virtual ~IBaseContext(){}

		void* getSrcPtr(){ return this->srcPtr ; } 
		
		void setPartsIndex( int p ){ this->partsIndex = p ; }
		int getPartsIndex() { return this->partsIndex ; }

		bool isOutsideParam() {	return this->outsideParam;	}
		void setOutsideParam( bool outsideParam ) {	this->outsideParam = outsideParam ;	}

		bool isAvailable()
		{
			return available && ! outsideParam ;
		}

		void setAvailable(bool available)
		{
			this->available = available;
		}

		/*
		 * 基準面から自身までの倍率の総乗を返す。
		 * BDAffineの場合は、自身のスケールと親までのスケールの積になる
		 */
		float getTotalScale()
		{
			return totalScale ;
		}
	
		void setTotalScale_notForClient(float totalScale)
		{
			this->totalScale = totalScale ;
		}

		float getInterpolatedOpacity(){	return interpolatedOpacity ;}

		void setInterpolatedOpacity(float interpolatedOpacity){this->interpolatedOpacity = interpolatedOpacity ;}

		float getTotalOpacity(){return totalOpacity ;}
	
		void setTotalOpacity(float totalOpacity){this->totalOpacity = totalOpacity ;}


	private:
		IBaseData* srcPtr ;
		int partsIndex ;
		
		bool outsideParam ;
		bool available ;

		
	protected:
		// 基準面から自身までの倍率の総乗
		// BDAffineの scaleX,YはXYの意味を適切に使えないので scaleX のみ用いる。
		// scaleXと親のtotalScaleを掛けたものが自身のtotalScaleとなる
		// BDBoxGridのように全て点で変換を定義する場合は、倍率は意味を持たない。
		// BDAffine親、 BDAffine子の場合に倍率を反映するために用いる
		float		totalScale;

		// デフォーマの透明度(SDK 2.0以降)
		float		interpolatedOpacity;		// 不透明度の補間結果
		float		totalOpacity;				// 基準面から自身までの不透明度の累積（自身を含む）

	};
}
//------------ LIVE2D NAMESPACE ------------

#endif // __SKIP_DOC__

#endif // __IBASE_CONTEXT__
