/*
 *  MemoryParam.h
 *
 * メモリ確保用パラメータ
 * 
 * 各種メソッドにこのパラメータを伝播させてメモリ管理を制御する。
 * 
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */


#pragma once


#ifndef __SKIP_DOC__

#include "LDUnmanagedObject.h"
#include "fixed/MemoryHolderFixed.h"
#include "tmp/MemoryHolderTmp.h"
#include "LDAllocator.h"

//--------- LIVE2D NAMESPACE ------------
namespace live2d 
{


	class MemoryPage ;

	//----------------------------------------
	class MemoryStackEnt : public LDUnmanagedObject// ここだけはLDObjectを継承しない
	{
		friend class MemoryParam ;
	public:
		MemoryStackEnt() : nextEnt(NULL), allocType(0) , align(0) , holder(NULL)
		{
		}

		virtual ~MemoryStackEnt(){}

	private:
		MemoryStackEnt*		nextEnt ;

		int					allocType ;
		int					align ;
		AMemoryHolder*		holder ;// 開放しない
	} ;


	//----------------------------------------
	class MemoryParam : public LDObject// TmpをPlacement newにすることで、Tmpメモリ利用を可能に変更
	{
	public:

	//	MemoryParam();
		MemoryParam(AMemoryHolder* main , AMemoryHolder* gpu);
		
		virtual ~MemoryParam() ;

		// Allocの設定を指定する(戻り値は直前の設定）
		LDAllocator::Type  setAllocType( LDAllocator::Type  allocType )
		{
			LDAllocator::Type  ret = this->curAllocType	 ;
			this->curAllocType	= allocType ;
			this->curMemoryHolder	= getMemoryHolder( allocType ) ;	
			return ret ;
		}

		// Allocの設定を取り出す(戻り値は直前の設定）
		int  setAllocAlign( int align )
		{
			int ret = this->curAlign ;
			this->curAlign = align ;
			return ret ;
		}

		// Allocの設定を取り出す
		LDAllocator::Type getAllocType( )
		{
			return this->curAllocType ;
		}

		// Allocの設定を取り出す
		int  getAllocAlign( )
		{
			return curAlign ;
		}


		void* malloc_exe( l2d_size_t size  )
		{
			return curMemoryHolder->malloc_exe( size , curAlign ) ;
		}

		AMemoryHolder*  getCurMemoryHolder()
		{
			return curMemoryHolder ;
		}

		AMemoryHolder*  getMemoryHolder( LDAllocator::Type  allocType ) ;

		void clear() ;

		// 開放しないので外部で管理する
		void setMemoryHolderMain(AMemoryHolder*	h)
		{
			this->memoryHolderMain = h ;
		}

		// 開放しないので外部で管理する
		void setMemoryHolderGPU(AMemoryHolder*	h)
		{
			this->memoryHolderGPU = h ;
		}

		void checkMemory()
		{
			memoryHolderMain->healthCheck();
			memoryHolderGPU->healthCheck();
		}
	private:

		AMemoryHolder*			curMemoryHolder ;
		LDAllocator::Type		curAllocType ;
		int						curAlign ;

		AMemoryHolder*			memoryHolderMain ;	
		AMemoryHolder*			memoryHolderGPU ;	

	};


}
//--------- LIVE2D NAMESPACE ------------

#endif // __SKIP_DOC__
