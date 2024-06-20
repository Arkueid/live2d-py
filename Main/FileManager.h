/**
 *
 *  You can modify and use this source freely
 *  only for the development of application related Live2D.
 *
 *  (c) Live2D Inc. All rights reserved.
 */

#include <string>


class FileManager 
{
public:
	static unsigned char* loadFile(const char* path,int* size);
	static void releaseBuffer(void* ptr);
	static void loadTexture(const char * textureFilePath, unsigned int *textureID);
	
	static void getParentDir( const char* path , std::string* return_dir ) ;
};

