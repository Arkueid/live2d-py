/**
 *  LDObject.h
 *
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */

#ifndef __LIVE2D_LDOBJECT_H__
#define __LIVE2D_LDOBJECT_H__

#include "../Live2D.h"


namespace live2d { //--------- LIVE2D NAMESPACE ------------


	class MemoryParam ;
	class AMemoryHolder ;

	
	class LDObject
	{
	public:

		LDObject(void) ;
		virtual ~LDObject(void) ;

		// --- L2D_MEM_TMPにメモリを配置 ---
		static void* operator new(l2d_size_t size) ;
		static void operator delete(void* ptr ) ;
		
		static void* operator new[](l2d_size_t size) ;
		static void operator delete[](void* ptr ) ;

		//placement new
		static void* operator new(l2d_size_t size, void* ptr) ;
		static void operator delete(void* ptr, void* ptr2) ;//dummy

		//--- memory managed ---
		static void* operator new(l2d_size_t size , MemoryParam* memParam ) ;
		static void operator delete(void* ptr, MemoryParam* memParam ) ;

		//--- debug file & line 
		static void* operator new(l2d_size_t size , MemoryParam* memParam , const char* filename , int line ) ;
		static void operator delete(void* ptr, MemoryParam* memParam , const char* filename , int line ) ;

	public:
	#ifdef L2D_DEBUG
		static int object_instanceCount ;
		static int object_totalCount ;
	#endif //L2D_DEBUG

	};

} //------------------------- LIVE2D NAMESPACE ------------

#endif	//__LIVE2D_LDOBJECT_H__
