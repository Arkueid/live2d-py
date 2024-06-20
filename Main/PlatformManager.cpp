/**
 *  You can modify and use this source freely
 *  only for the development of application related Live2D.
 *
 *  (c) Live2D Inc. All rights reserved.
 */

#include <GL/glew.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "PlatformManager.h"
#include "FileManager.h"
#include "util/UtDebug.h"
#include "LAppTextureDesc.h"
#include <Live2DModelOpenGL.h>
#include "Log.hpp"


using namespace live2d;
using namespace live2d::framework;

PlatformManager::PlatformManager(void)
{
}


PlatformManager::~PlatformManager(void)
{
}


unsigned char* PlatformManager::loadBytes(const char* path, size_t* size)
{
	unsigned char* data = FileManager::loadFile(path ,(int *)size);
	return data;
}

void PlatformManager::releaseBytes(void* data)
{
	FileManager::releaseBuffer(data);
}

ALive2DModel* PlatformManager::loadLive2DModel(const char* path)
{
	size_t size;
	unsigned char* buf = loadBytes(path, &size);
	
	//Create Live2D Model Instance
	ALive2DModel* live2DModel = Live2DModelOpenGL::loadModel(buf, (int)size);

    return live2DModel;
}

L2DTextureDesc* PlatformManager::loadTexture(ALive2DModel* model, int no, const char* path)
{
	unsigned int textureID;

	int width, height, channels;
	assert(textureCount <= 10);
	unsigned char* data = stbi_load(path, &width, &height, &channels, 0);

	assert(data != NULL);

	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &textureID);

	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	/*glTexImage2D(
	GL_TEXTURE_2D , 0 , GL_RGBA , width , height ,
	0 , GL_RGBA , GL_UNSIGNED_BYTE , image
	);*/
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);

	stbi_image_free(data);
	LAppTextureDesc* desc=new LAppTextureDesc(textureID);

	((live2d::Live2DModelOpenGL*)model)->setTexture(no, textureID);
	return desc;
}

void PlatformManager::log(const char* txt)
{
	Info("%s", txt);
}