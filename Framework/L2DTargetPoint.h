/**
 *
 *  You can modify and use this source freely
 *  only for the development of application related Live2D.
 *
 *  (c) Live2D Inc. All rights reserved.
 */
#pragma once


//Live2D lib
#include "Live2D.h"
#include "util/UtMath.h"

namespace live2d
{
	namespace framework
	{
		class L2DTargetPoint
		{
		private:
			float faceTargetX;      
			float faceTargetY;      
			float faceX;            
			float faceY;            
			float faceVX;           
			float faceVY;           
			l2d_int64 timeSec;
			l2d_int64 lastTimeSec;
			
		public:
			static const int FRAME_RATE = 30;
			
		public:
			L2DTargetPoint();
			
			virtual ~L2DTargetPoint(){}
			
			void update();
			
			float getX(){return this->faceX;}
			
			float getY(){return this->faceY;}
			
			void set( float x, float y );
		};
	}
}

