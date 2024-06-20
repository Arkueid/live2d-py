/**
 *
 *  You can modify and use this source freely
 *  only for the development of application related Live2D.
 *
 *  (c) Live2D Inc. All rights reserved.
 */
#include "L2DTargetPoint.h"
#include "util/UtSystem.h"

namespace live2d
{
	namespace framework
	{
		L2DTargetPoint::L2DTargetPoint()
		{
			this->faceTargetX   = 0;
			this->faceTargetY   = 0;
			this->faceX         = 0;
			this->faceY         = 0;
			this->faceVX        = 0.0f;
			this->faceVY        = 0.0f;
			this->lastTimeSec   = 0;
		}
		
		
		
		void L2DTargetPoint::update()
		{
			
			
			const float FACE_PARAM_MAX_V = 40.0 / 10 ;
			const float MAX_V =  FACE_PARAM_MAX_V * 1.0f / FRAME_RATE ;
			
			static l2d_int64 lastTimeSec = 0 ;
			if( lastTimeSec == 0 )
			{
				lastTimeSec = UtSystem::getUserTimeMSec() ;
				return ;
			}
			l2d_int64 curTimeSec = UtSystem::getUserTimeMSec() ;
			
			float deltaTimeWeight = (float)(curTimeSec - lastTimeSec)*FRAME_RATE/1000.0f ;
			lastTimeSec = curTimeSec ;
			
			
			const float TIME_TO_MAX_SPEED = 0.15f ;
			const float FRAME_TO_MAX_SPEED = TIME_TO_MAX_SPEED * FRAME_RATE  ;//sec*frame/sec
			const float MAX_A = deltaTimeWeight * MAX_V / FRAME_TO_MAX_SPEED ;
			
			
			
			float dx = (faceTargetX - faceX) ;
			float dy = (faceTargetY - faceY) ;
			
			if( dx == 0 && dy == 0 ) return ;
			
			
			float d = L2D_SQRT( dx*dx + dy*dy ) ;
			
			
			float vx = MAX_V * dx / d ;
			float vy = MAX_V * dy / d ;
			
			
			float ax = vx - faceVX ;
			float ay = vy - faceVY ;
			
			float a = L2D_SQRT( ax*ax + ay*ay ) ;
			
			
			if( a < -MAX_A || a > MAX_A )
			{
				ax *= MAX_A / a ;
				ay *= MAX_A / a ;
			}
			
			
			faceVX += ax ;
			faceVY += ay ;
			
			
			
			
			
			{
				
				//            2  6           2               3
				//      sqrt(a  t  + 16 a h t  - 8 a h) - a t
				// v = --------------------------------------
				//                    2
				//                 4 t  - 2
				//(t=1)
				
				
				
				float max_v = 0.5f * ( L2D_SQRT( MAX_A*MAX_A + 16*MAX_A * d - 8*MAX_A * d ) - MAX_A ) ;
				float cur_v = L2D_SQRT( faceVX*faceVX + faceVY*faceVY ) ;
				
				if( cur_v > max_v )
				{
					
					faceVX *= max_v / cur_v ;
					faceVY *= max_v / cur_v ;
				}
			}
			
			faceX += faceVX ;
			faceY += faceVY ;
			
			return;
		}
		
		
		void L2DTargetPoint::set( float x, float y )
		{
			this->faceTargetX   = x;
			this->faceTargetY   = y;
		}
	}
}