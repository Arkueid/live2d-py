/**
 *  MHBin.h
 *
 * 一時インスタンス用のメモリ保持用クラス
 * 
 * 通常の確保・破棄を行うメモリ実装
 * 
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */
#pragma once


#ifndef __SKIP_DOC__

#include "../../Live2D.h"
#include "../LDObject.h"
#include "../AMemoryHolder.h"
#include "../APageHeader.h"

//--------- LIVE2D NAMESPACE ------------
namespace live2d 
{

	class MemoryHolderTmp ;
	class MHPageHeaderTmp ;


	//==========================================================
	// 　Bin（ビン）クラス
	// 　32 , 64 , 128等バイトサイズごとにページデータを格納する
	//==========================================================
	class MHBin
	{
	public:
		MHBin() ;

		void init( l2d_uint16 binNo , l2d_size_t _chunkSize , l2d_size_t _pageSize ) ;
		
		l2d_size_t	getChunkSize( l2d_size_t  malloc_size ) ;

	public:// 仮
		l2d_size_t					chunkSize ;			// １つのチャンクサイズ。AllocHeaderを含む。0の時は自由なサイズ
		l2d_size_t					pageSize ;			// ページ全体のサイズ。AllocHeaderを含む。0の時は自由なサイズ
		l2d_uint16				pageChunkCount ;	// １つのページに入るチャンク数
		l2d_uint16				binNo ;				// 自身のBin番号（index）
		l2d_uint32				bitmask[3] ;		// 使用可能なビットを1とする(使用中という意味ではない)。0は使用不可

		MHPageHeaderTmp*		filledPages ;		// 全部使用中
		MHPageHeaderTmp*		curPages ;			// 使用可能ページ
	} ;

}
//--------- LIVE2D NAMESPACE ------------

#endif // __SKIP_DOC__
