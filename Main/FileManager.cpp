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

#include <fstream>
#include <filesystem>

// csmByte* LAppPal::LoadFileAsBytes(const string filePath, csmSizeInt* outSize)
// {
//     //filePath;//
//     const char* pathStr = filePath.c_str();
//     std::filesystem::path path = std::filesystem::u8path(filePath);

//     int size = 0;
//     if (std::filesystem::exists(path))
//     {
//         size = std::filesystem::file_size(path);
//         if (size == 0)
//         {
//             Info("Stat succeeded but file size is zero. path:%s", pathStr);
//             return NULL;
//         }
//     }
//     else
//     {
//         Info("Stat failed. errno:%d path:%s", errno, pathStr);
//         return NULL;
//     }

//     std::fstream file;
//     file.open(path, std::ios::in | std::ios::binary);
//     if (!file.is_open())
//     {
//         Info("File open failed. path:%s", pathStr);
//         return NULL;
//     }

//     char* buf = new char[size];
//     file.read(buf, size);
//     file.close();

//     if(outSize) *outSize = size;
    
//     return reinterpret_cast<csmByte*>(buf);
// }

unsigned char *FileManager::loadFile(const char *filepath, int *ret_bufsize)
{
	std::filesystem::path path = std::filesystem::u8path(filepath);

	unsigned int size = 0;
	if (std::filesystem::exists(path))
	{
	    size = std::filesystem::file_size(path);
	    if (size == 0)
	    {
	        Info("Stat succeeded but file size is zero. path:%s", filepath);
	        return NULL;
	    }
	}
	else
	{
	    Info("Stat failed. errno:%d path:%s", errno, filepath);
	    return NULL;
	}

	std::fstream file;
	file.open(path, std::ios::in | std::ios::binary);
	if (!file.is_open())
	{
	    Info("File open failed. path:%s", filepath);
	    return NULL;
	}

	char* buf = (char*)malloc(size);
	file.read(buf, size);
	file.close();

	if (ret_bufsize) *ret_bufsize = static_cast<int>(size);
	return reinterpret_cast<unsigned char*>(buf);
}

void FileManager::releaseBuffer(void *ptr)
{
	free(ptr);
}

void FileManager::loadTexture(const char *textureFilePath, unsigned int *textureID)
{
	int width, height, channels;

	int size;
	unsigned char* bytes = loadFile(textureFilePath, &size);
	Debug("%d", size);
	unsigned char *data = stbi_load_from_memory(bytes, size, &width, &height, &channels, 0);

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
	releaseBuffer(bytes);
}

void FileManager::getParentDir(const char *path, std::string *return_dir)
{
	(*return_dir) = path;
	(*return_dir) += "\\..\\";
}
