/**
 *  UtFile.h
 *
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */

#ifndef __LIVE2D_UTFILE_H__
#define __LIVE2D_UTFILE_H__


#include "../type/LDVector.h"


//--------- LIVE2D NAMESPACE ------------
namespace live2d 
{ 
	
	class UtFile 
	{
	public:
		// ファイルからバイナリデータをロードして返す。失敗した場合は NULL が返る。
		// 必ず UtFile::releaseLoadBuffer() で破棄する。
		static char* loadFile( const live2d::LDString &filepath , int * ret_bufsize ) ;

		// ファイルからバイナリデータをロードして返す。失敗した場合は NULL が返る。
		// 必ず UtFile::releaseLoadBuffer() で破棄する。
		static char* loadFile( const char * filepath , int * ret_bufsize ) ;
	
		// loadFile()でロードしたバッファを破棄する
		static void releaseLoadBuffer( char * buf ) ;
		
	private:
		UtFile();	
									
		//Prevention of copy Constructor
		UtFile( const UtFile & ) ;				
		UtFile& operator=( const UtFile & ) ; 	
	
		~UtFile();
	
	};
} 
//------------------------- LIVE2D NAMESPACE ------------

#endif		// __LIVE2D_UTFILE_H__
