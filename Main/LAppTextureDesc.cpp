/**
 *
 *  You can modify and use this source freely
 *  only for the development of application related Live2D.
 *
 *  (c) Live2D Inc. All rights reserved.
 */
#include <GL/glew.h>
#include "LAppTextureDesc.h"


LAppTextureDesc::LAppTextureDesc(unsigned int tex)
{
	data=tex;
}

LAppTextureDesc::~LAppTextureDesc()
{
	glDeleteTextures(1, &data);
}
