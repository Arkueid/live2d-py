/**
 *  MHPageHeaderTmp.h
 *
 *  一時インスタンス用のメモリ保持用クラス
 * 
 *  通常の確保・破棄を行うメモリ実装
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
	class MHBin ;


	//==========================================================
	// 	ページヘッダ クラス
	// 
	// 	Pageの先頭にPageHeaderが入り、以降にデータが並ぶ
	// 	PageHeaderのサイズは64-sizeof(AllocHeader)以下とする
	// 	chunkの先頭は、this + 64 - sizeof( AllocHeader )となり、ポインタは this + 64となる
	//==========================================================
	class MHPageHeaderTmp : public APageHeader 
	{
	public:
		// placement newを行う
		MHPageHeaderTmp( MemoryHolderTmp* holder , MHBin* bin , l2d_size_t pageSize , l2d_size_t chunkSize , l2d_size_t pageOffset ) ;

		static l2d_size_t  calcChunkCount( l2d_size_t  pageSize , l2d_size_t  chunkSize ) ;

		// 使用可能な最初の要素のインデックスを取り出し、使用中フラグを立てる
		int getFirstEmptyIndexAndSetFlag(MHBin* bin) ;

		void*  getPtr( int index ) ;

		AllocHeader*  getAllocHeaderPtr( int index ) ;

		int  getIndexOfPtr( void* ptr ) ;

		// 使用可能な最初の要素のインデックスを取り出し、使用中フラグを立てる
		void setFlag( int index , bool flag ) ;

		bool getFlag( int index ) ;

		// 開放する
		virtual void  free_exe( void* ptr ){ holder->free_exe( this , ptr ) ; }
		

	protected:
		~MHPageHeaderTmp(){}// placement newを使い/deleteはしない

	public:// 仮
		// []内は、32のバイト数と、64bitのバイト数
		//void*					vtable			// [4-8]
		MemoryHolderTmp*		holder ;		// [4-8]
		l2d_uint32				bitmask[3] ;	// [12]	32*3=最大96個のフラグを持つ但し、かならずしもPageに96個保持するとは限らない
												// 		使用中のものはフラグが立つ。
		l2d_uint16				emptyCount ;	// [2]
		l2d_uint16				chunkCount ;	// [2]	チャンクの合計数
		l2d_uint32				chunkSize ;		// [4]	一つのデータのサイズ（AllocHeaderを含む）
		l2d_uint32				pageSize ;		// [4]
		// ここまで 32-36
	
		MHPageHeaderTmp*		nextPage ;		// [4-8]
		l2d_uint8					pageOffset ;	// [1]	ページがアライメント調整される場合の調整バイト数（最大32）
		l2d_uint8					binNo ;			// [1]	MHBinの番号

		// 8-16

		// 64bit版を考えると最大で 64-8 = 56byte
	} ;

}
//--------- LIVE2D NAMESPACE ------------

#endif // __SKIP_DOC__
