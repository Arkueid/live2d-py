/**
 *  You can modify and use this source freely
 *  only for the development of application related Live2D.
 *
 *  (c) Live2D Inc. All rights reserved.
 */
#pragma once

#include "IPlatformManager.h"

class PlatformManager:public live2d::framework::IPlatformManager
{
public:
	PlatformManager(void);
	~PlatformManager(void);

	unsigned char* loadBytes(const char* path,size_t* size);
	void releaseBytes(void* data);
	live2d::ALive2DModel* loadLive2DModel(const char* path);
	live2d::framework::L2DTextureDesc* loadTexture(live2d::ALive2DModel* model, int no, const char* path);
	void log(const char* txt);
};

