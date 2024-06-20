/**
 *
 *  You can modify and use this source freely
 *  only for the development of application related Live2D.
 *
 *  (c) Live2D Inc. All rights reserved.
 */
#pragma once

#include <stdlib.h>
#include <math.h>
#include <string>
//Live2D lib
#include "ALive2DModel.h"
#include "util/UtSystem.h"

namespace live2d
{
	namespace framework
	{
		class L2DEyeBlink
		{
		public:
			
			enum EYE_STATE{
				STATE_FIRST = 0 ,
				STATE_INTERVAL ,
				STATE_CLOSING ,
				STATE_CLOSED , 
				STATE_OPENING
			};
			
			L2DEyeBlink();
			virtual ~L2DEyeBlink(){}
			
			l2d_int64 calcNextBlink() ;
			void setInterval( int blinkIntervalMsec) ;
			void setEyeMotion( int closingMotionMsec , int closedMotionMsec , int openingMotionMsec ) ;
			
			
			void setParam( live2d::ALive2DModel *model ) ;
			
		private:
			l2d_int64 nextBlinkTime ;
			int eyeState ;
			l2d_int64 stateStartTime ;
			bool closeIfZero;
			
			
			std::string eyeID_L ;
			std::string eyeID_R ;
			
			
			int blinkIntervalMsec ;
			int closingMotionMsec ;
			int closedMotionMsec  ;
			int openingMotionMsec ;
		};
	}
}