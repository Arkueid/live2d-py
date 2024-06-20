/**
 *
 *  You can modify and use this source freely
 *  only for the development of application related Live2D.
 *
 *  (c) Live2D Inc. All rights reserved.
 */
#include "L2DEyeBlink.h"

namespace live2d
{
	namespace framework
	{
		L2DEyeBlink::L2DEyeBlink()
		{
			
			eyeState = STATE_FIRST ;
			
			blinkIntervalMsec = 4000 ;
			
			closingMotionMsec = 100 ;
			closedMotionMsec  =  50 ;
			openingMotionMsec = 150 ;
			
			closeIfZero = true ;
			
			
			eyeID_L = "PARAM_EYE_L_OPEN" ;
			eyeID_R = "PARAM_EYE_R_OPEN" ;
		}
		
		
		l2d_int64 L2DEyeBlink::calcNextBlink()
		{
			
			l2d_int64 time = live2d::UtSystem::getTimeMSec() ;
			double r = (double)rand() / RAND_MAX ;
			return time + (l2d_int64)( r*( 2*blinkIntervalMsec - 1 ) ) ;
		}
		
		
		void L2DEyeBlink::setInterval( int blinkIntervalMsec)
		{
			this->blinkIntervalMsec = blinkIntervalMsec ;
		}
		
		
		void L2DEyeBlink::setEyeMotion( int closingMotionMsec , int closedMotionMsec , int openingMotionMsec )
		{
			this->closingMotionMsec = closingMotionMsec ;
			this->closedMotionMsec = closedMotionMsec ;
			this->openingMotionMsec = openingMotionMsec ;
		}
		
		
		
		void L2DEyeBlink::setParam( live2d::ALive2DModel *model )
		{
			l2d_int64 time = live2d::UtSystem::getTimeMSec() ;
			float eyeParamValue ;
			float t = 0 ;
			
			switch( this->eyeState )
			{
				case STATE_CLOSING:
					
					t = (float)(( time - stateStartTime ) / (double)closingMotionMsec ) ;
					if( t >= 1 )
					{
						t = 1 ;
						this->eyeState = STATE_CLOSED ;
						this->stateStartTime = time ;
					}
					eyeParamValue = 1 - t ;
					break ;
				case STATE_CLOSED:
					t = (float)(( time - stateStartTime ) / (double)closedMotionMsec) ;
					if( t >= 1 )
					{
						this->eyeState = STATE_OPENING ;
						this->stateStartTime = time ;
					}
					eyeParamValue = 0 ;
					break ;
				case STATE_OPENING:
					t = (float)(( time - stateStartTime ) / (double)openingMotionMsec ) ;
					if( t >= 1 )
					{
						t = 1 ;
						this->eyeState = STATE_INTERVAL ;
						this->nextBlinkTime = calcNextBlink() ;
					}
					eyeParamValue = t ;
					break ;
				case STATE_INTERVAL:
					
					if( this->nextBlinkTime < time )
					{
						this->eyeState = STATE_CLOSING ;
						this->stateStartTime = time ;
					}
					eyeParamValue = 1 ;
					break ;
				case STATE_FIRST:
				default:
					this->eyeState = STATE_INTERVAL ;
					this->nextBlinkTime = calcNextBlink() ;
					eyeParamValue = 1 ;
					break ;
			}
			
			if( ! closeIfZero ) eyeParamValue = -eyeParamValue ;
			
			
			model->setParamFloat( eyeID_L.c_str() , eyeParamValue ) ;
			model->setParamFloat( eyeID_R.c_str() , eyeParamValue ) ;
			
		}
	}
}