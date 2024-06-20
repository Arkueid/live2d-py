/**
 *  LDRect.h
 *
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */
#ifndef __LIVE2D_LD_RECT_H__
#define __LIVE2D_LD_RECT_H__


#ifndef __SKIP_DOC__

#include "../memory/LDObject.h"

//--------- LIVE2D NAMESPACE ------------
namespace live2d
{ 
	
	
	
	class LDRect  : public live2d::LDObject 
	{
	public:
		// Constructor
		LDRect();
		LDRect(int x , int y , int w , int h );
	
		// Destructor
		virtual ~LDRect();
		
	public:
		//  中央X座標取得
		int getCenterX(){ return x + width/2 ; } 
	
		//  中央Y座標取得
		int getCenterY(){ return y + height/2 ; }
	
		//  右端取得
		int getRight(){ return x + width ; }
	
		//  一番下取得
		int geBottom(){ return y + height ; }
		
	public:
		int x;			// 左端となるX座標
		int y;			// 上端となるY座標
		int width;		// 矩形の幅
		int height;		// 矩形の高さ
		
	};

}
//------------------------- LIVE2D NAMESPACE ------------

#endif // __SKIP_DOC__

#endif	// __LIVE2D_LD_RECT_H__
