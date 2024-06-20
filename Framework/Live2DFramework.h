/**
 *
 *  You can modify and use this source freely
 *  only for the development of application related Live2D.
 *
 *  (c) Live2D Inc. All rights reserved.
 */
#pragma once

#include "IPlatformManager.h"

namespace live2d
{
	namespace framework
	{
		class Live2DFramework 
		{
		private:
			static IPlatformManager* platformManager;
		public:
			static IPlatformManager* getPlatformManager();
			static void setPlatformManager(IPlatformManager* platformManager);
		};
	}
}