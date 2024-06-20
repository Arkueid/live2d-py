/*
 *  IContextData.h
 *  
 *  ImmutableのDrawData、BaseDataの描画用のデータを保持する。
 *  
 *  Created by joe on 11/04/29.
 *  Copyright 2011 Live2D Inc. All rights reserved.
 */

#ifndef __LIVE2D_I_DRAW_DATA_H__
#define __LIVE2D_I_DRAW_DATA_H__


#ifndef __SKIP_DOC__

#include "../memory/LDObject.h"

//------------ LIVE2D NAMESPACE ------------
namespace live2d
{ 
	class IDrawData;
	class ClipContext;
	
	class IDrawContext : public live2d::LDObject 
	{
	public:
		IDrawContext(IDrawData* src) ;
		virtual ~IDrawContext(){};
	
		void* getSrcPtr(){ return this->srcPtr ; } 
	
		//  パーツ番号セット
		void setPartsIndex( int p ){ this->partsIndex = p ; }
	
		//  パーツ番号取得
		int getPartsIndex() { return this->partsIndex ; }
	
		//  利用できるか取得　TODO 更新
		bool isAvailable(){ return available && ! paramOutside ; }
	
	public:
		int 			interpolatedDrawOrder ;		// 描画順の補間結果
		float 			interpolatedOpacity ;		// 不透明度の補間結果
		
		bool			paramOutside ;				// 補間結果として、パラメータが範囲外のときに true 
		
		float			partsOpacity ;				// パーツの不透明度
		bool			available ;					// TODO 更新
	
		float			baseOpacity;				// SDK2.0 所属するデフォーマの累積透明度

		ClipContext		*clipBufPre_clipContext;	// SDK2.1 マスクの設定

	private:
		IDrawData* 		srcPtr ;					
		int 			partsIndex ;				
	};
}
//------------ LIVE2D NAMESPACE ------------

#endif // __SKIP_DOC__

#endif	// __LIVE2D_I_DRAW_DATA_H__

