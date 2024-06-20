/**
 *
 *  You can modify and use this source freely
 *  only for the development of application related Live2D.
 *
 *  (c) Live2D Inc. All rights reserved.
 */
#include "Live2DFramework.h"

namespace live2d
{
	namespace framework
	{
		IPlatformManager* Live2DFramework::platformManager=0;
		
		IPlatformManager* Live2DFramework::getPlatformManager()
		{
			return platformManager;
		}

		void Live2DFramework::setPlatformManager(IPlatformManager* pm)
		{
			platformManager=pm;
		}
	}
}