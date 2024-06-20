/**
 *
 *  You can modify and use this source freely
 *  only for the development of application related Live2D.
 *
 *  (c) Live2D Inc. All rights reserved.
 */
#pragma once

#include <physics/PhysicsHair.h>
#include "ALive2DModel.h"
#include "util/UtFile.h"
#include <string>
#include <vector>

namespace live2d
{
	namespace framework
	{
		class L2DPhysics
		{
		private:
			std::vector<live2d::PhysicsHair*> physicsObjects;
			std::vector<std::string*> paramIDs;
			l2d_int64 startTimeMSec;
			
		public:
			L2DPhysics(void);
			~L2DPhysics(void);
			
			void updateParam(live2d::ALive2DModel* model) ;
			
			
			static L2DPhysics* load(const void* buf , int size ) ;
			static L2DPhysics* load(const std::string & filepath );
		};
	}
}