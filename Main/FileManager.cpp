/**
 *
 *  You can modify and use this source freely
 *  only for the development of application related Live2D.
 *
 *  (c) Live2D Inc. All rights reserved.
 */

#include <GL/glew.h>
#include <stb_image.h>

#include "FileManager.h"
#include "MyLive2DAllocator.h"
#include <stdio.h>
#include <live2d/type/LDString.h>
#include "Log.hpp"


unsigned char* FileManager::loadFile(const char* filepath,int* ret_bufsize)
{
		FILE *fp; 
		unsigned char * buf;
	
		
		if ( fopen_s( &fp , filepath, "rb") ) //return nonzero if error
		{
			Error("file open error %s!!" , filepath );
			return NULL;
		}
	
		
		fseek(fp, 0, SEEK_END );
		int size = ftell(fp);
	
		buf = (unsigned char*)malloc( size );
		L2D_ASSERT_S( buf != 0 , "malloc( %d ) is NULL @ fileload %s" , size , filepath ) ;
		
	
		fseek(fp, 0, SEEK_SET);
	
		
		int loaded = (int)fread(buf, sizeof(char), size, fp);
		fclose(fp); 
	
		
		if (loaded != size)
		{
			Error("file load error / loaded size is wrong / %d != %d" , loaded , size );
	
	
			return NULL;
		}
	
		*ret_bufsize = size ;
		return buf;

}


void FileManager::releaseBuffer(void* ptr)
{
	free(ptr);
}



void FileManager::loadTexture(const char * textureFilePath, unsigned int* textureID)
{
	int width, height, channels;
	unsigned char* data = stbi_load(textureFilePath, &width, &height, &channels, 0);

	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, textureID);

	glBindTexture(GL_TEXTURE_2D, *textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	/*glTexImage2D(
	GL_TEXTURE_2D , 0 , GL_RGBA , width , height ,
	0 , GL_RGBA , GL_UNSIGNED_BYTE , image
	);*/
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);

	stbi_image_free(data);
}



void FileManager::getParentDir(const char* path , std::string* return_dir){
	(*return_dir) = path ;
	(*return_dir) += "\\..\\" ;
}
