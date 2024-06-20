/**
 *
 *  You can modify and use this source freely
 *  only for the development of application related Live2D.
 *
 *  (c) Live2D Inc. All rights reserved.
 */
#pragma once

//Live2D lib
#include "ALive2DModel.h"
#include "motion/AMotion.h"
#include "motion/MotionQueueManager.h"

namespace live2d
{
	namespace framework
	{
		class L2DMotionManager : public MotionQueueManager
		{
			typedef MotionQueueManager super;
		public:
			L2DMotionManager();
			virtual ~L2DMotionManager(){}
			
		private:
			int currentPriority;
			int reservePriority;
			
		public:
			
			int getCurrentPriority(){return currentPriority;}
			
			
			int getReservePriority(){return reservePriority;}
			
			
			void setReservePriority( int val ){reservePriority=val;}
			
			
			int startMotionPrio( live2d::AMotion* motion, bool isDelete, int priority );
			
			bool updateParam(live2d::ALive2DModel* model);
			
			bool reserveMotion(int priority);
		};
	}
}
