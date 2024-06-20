/*
 *  MHPageHeaderFixed.h
 *
 *	一括破棄のみ対応したメモリページ
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */

#pragma once


#ifndef __SKIP_DOC__

#include "../../Live2D.h"
#include "../LDUnmanagedObject.h"
#include "../APageHeader.h"
#include "../AMemoryHolder.h"


//--------- LIVE2D NAMESPACE ------------
namespace live2d 
{

	class MHPageHeaderFixed : public APageHeader // MemoryManagement対象外
	{
	public:
		MHPageHeaderFixed();

		int getPageNo(){ return pageNo ; }
		int getPageAmount(){ return pageAmount ; }

		void* getStartPtr(){ return (char*)this + ENTRY_OFFSET - sizeof(AllocHeader) ; }

		virtual void  free_exe( void* ptr ) ;

	protected:
		~MHPageHeaderFixed(){}// placement newを使い/deleteはしない

	public:// ---　以下、パブリック参照を許可 ---
		MHPageHeaderFixed*	nextPage ;		// [4-8]隣のページ

	//	char*				startPtr ;		// [4-8]ここに記憶する（確保後は固定）
		char*				endPtr ;		// [4-8]ポインタの終端（確保後は固定、このアドレス自体は範囲外）
		l2d_uint32			size ;			// [4-8]このページのメモリサイズ（確保後は固定）

		char*				curPtr ;		// [4-8]次に確保できるポインタ(AllocHeaderの先頭になる)（可変）
		l2d_uint32			rest ;			// [4-8]このページの残りサイズ（可変）
		l2d_uint32			pageNo ;		// [4]このページの通し番号（ID）


		//--- static ---
		static int			pageAmount ;	// 全体のページ数（開放しても減らさない）
	};



}
//--------- LIVE2D NAMESPACE ------------

#endif // __SKIP_DOC__
