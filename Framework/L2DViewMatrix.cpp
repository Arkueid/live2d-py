/**
 *
 *  You can modify and use this source freely
 *  only for the development of application related Live2D.
 *
 *  (c) Live2D Inc. All rights reserved.
 */
#include "L2DViewMatrix.h"

namespace live2d
{
	namespace framework
	{
		void L2DViewMatrix::adjustTranslate( float shiftX, float shiftY )
		{
			if( tr[0]*maxLeft  + (tr[12] + shiftX) > screenLeft ) shiftX = screenLeft - tr[0]*maxLeft - tr[12] ;
			if( tr[0]*maxRight + (tr[12] + shiftX) < screenRight  ) shiftX = screenRight  - tr[0]*maxRight - tr[12] ;
			
			if( tr[5]*maxTop   + (tr[13] + shiftY) < screenTop ) shiftY = screenTop - tr[5]*maxTop - tr[13] ;
			if( tr[5]*maxBottom+ (tr[13] + shiftY) > screenBottom    ) shiftY = screenBottom    - tr[5]*maxBottom- tr[13] ;
			
			
			float tr1[] = { 1,0,0,0 , 0,1,0,0 , 0,0,1,0 , shiftX,shiftY,0,1 } ;
			mul( tr1 , tr , tr ) ;
		}
		
		
		void L2DViewMatrix::adjustScale( float cx , float cy , float scale )
		{
			float MAX_SCALE = getMaxScale() ;
			float MIN_SCALE = getMinScale() ;
			
			float targetScale = scale * tr[0] ;//
			if( targetScale < MIN_SCALE )
			{
				if( tr[0] > 0 ) scale = MIN_SCALE / tr[0] ;
			}
			else if( targetScale > MAX_SCALE )
			{
				if( tr[0] > 0 ) scale = MAX_SCALE / tr[0] ;
			}
			
			float tr1[16] = { 1,0,0,0 , 0,1,0,0 , 0,0,1,0 , cx,cy,0,1 } ;
			float tr2[16] = { scale,0,0,0 , 0,scale,0,0 , 0,0,1,0 , 0,0,0,1 } ;
			float tr3[16] = { 1,0,0,0 , 0,1,0,0 , 0,0,1,0 ,-cx,-cy,0,1 } ;
			
			mul( tr3 , tr , tr ) ;
			mul( tr2 , tr , tr ) ;
			mul( tr1 , tr , tr ) ;
		}
		
		
		
		void L2DViewMatrix::setScreenRect(float left, float right, float bottom, float top)
		{
			screenLeft=left;
			screenRight=right;
			screenTop=top;
			screenBottom=bottom;
		}
		
		
		
		void L2DViewMatrix::setMaxScreenRect(float left, float right, float bottom, float top)
		{
			maxLeft=left;
			maxRight=right;
			maxTop=top;
			maxBottom=bottom;
		}
	}
}