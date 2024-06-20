/*
 *  APageHeader.h
 *
 *  メモリ保持のための抽象クラス
 * 
 *  通常のメモリ管理と、寿命管理型のメモリ管理を抽象化
 * 
 * 
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */

#pragma once


#ifndef __SKIP_DOC__

#include "LDUnmanagedObject.h"
//#include "../memory/MemoryPage.h"
#include "LDAllocator.h"


//--------- LIVE2D NAMESPACE ------------
namespace live2d 
{


	//==========================================================
	//==========================================================
	class APageHeader // placement newを使い、deleteしない
	//class APageHeader  : public LDUnmanagedObject // メモリ管理対象外
	{
	public:
		const static int ENTRY_OFFSET = 64 ;// ページの最初のデータへのオフセット（chunkは-4(32bit) -8(64bit))

		APageHeader(){}

		
		virtual void  free_exe( void* ptr ) = 0 ;


	protected:
		~APageHeader(){}// placement newを使い/deleteはしない

	};


}
//--------- LIVE2D NAMESPACE ------------

#endif // __SKIP_DOC__
