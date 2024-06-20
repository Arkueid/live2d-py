/**
 *  MemoryHolderFixed.h
 *
 *  固定寿命インスタンス用のメモリ保持用クラス
 * 
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */

#pragma once


#ifndef __SKIP_DOC__

#include "../LDObject.h"
#include "../AMemoryHolder.h"

//--------- LIVE2D NAMESPACE ------------
namespace live2d 
{

	class MHPageHeaderFixed ;

	class MemoryHolderFixed : public AMemoryHolder
	{
	public:

		MemoryHolderFixed(LDAllocator::Type allocType , const char* holderName , l2d_size_t  pageSize = defaultPageSize );
		virtual ~MemoryHolderFixed() ;

		virtual void* malloc_exe( l2d_size_t size , int align) ;
		
		virtual void free_exe( APageHeader* header , void* ptr ) ;
		virtual void clear() ;

		virtual void healthCheck() ;

		static void setDefaultPageSize( l2d_size_t size ) { defaultPageSize = size ; } 

		void dumpPages(MHPageHeaderFixed* c) ;
		void checkPages(MHPageHeaderFixed* c) ;

	private:
		char* createNewPage( l2d_size_t size , int align ) ;

		char* getAvailablePtr( MHPageHeaderFixed* page , l2d_size_t size , int align ) ;

		void clear_exe( MHPageHeaderFixed* page ) ;

	private:
		MHPageHeaderFixed*			curPages ;		// 未使用領域が残るList
		MHPageHeaderFixed*			filledPages ;	// 全て使い切ったList
		LDAllocator::Type			allocType ;
		const char*					holderName ;	// MemoryHolderの識別名
		int							holderNo ;		// MemoryHolderの何番目のインスタンスか
		l2d_size_t						pageSize ;		// 新規作成時のPageのサイズ
		static l2d_size_t				defaultPageSize ;
	};

}
//--------- LIVE2D NAMESPACE ------------

#endif // __SKIP_DOC__
