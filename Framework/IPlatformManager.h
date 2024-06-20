/**
 *
 *  You can modify and use this source freely
 *  only for the development of application related Live2D.
 *
 *  (c) Live2D Inc. All rights reserved.
 */
#pragma once

#include "ALive2DModel.h"
#include "L2DTextureDesc.h"

namespace live2d
{
	namespace framework
	{
		class IPlatformManager 
		{
		public:
			virtual ~IPlatformManager(){}
			virtual unsigned char* loadBytes(const char* path,size_t* size)=0;
			virtual void releaseBytes(void* data)=0;
			virtual live2d::ALive2DModel* loadLive2DModel(const char* path)=0;
			virtual L2DTextureDesc* loadTexture(live2d::ALive2DModel* model, int no, const char* path)=0;
			virtual void log(const char* txt)=0;
		};
	}
}