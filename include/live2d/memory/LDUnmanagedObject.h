/**
 *  LDUnmanagedObject.h
 * 
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */
#pragma once


#include "../Live2D.h"


namespace live2d { //--------- LIVE2D NAMESPACE ------------


	class MemoryParam ;


	
	class LDUnmanagedObject
	{
	public:
		static int instanceCount ;

		LDUnmanagedObject(void) ;
		virtual ~LDUnmanagedObject(void) ;

		static void* operator new(l2d_size_t size) ;
		static void operator delete(void* ptr ) ;

		static void* operator new[](l2d_size_t size) ;
		static void operator delete[](void* ptr ) ;

		//placement new
		static void* operator new(l2d_size_t size, void* ptr) ;
		static void operator delete(void* ptr, void* ptr2) ;//dummy
	
		//--- debug file & line 
		static void* operator new(l2d_size_t size , const char* filename , int line ) ;
		static void operator delete(void* ptr, const char* filename , int line ) ;

		static void* operator new[](l2d_size_t size , const char* filename , int line ) ;
		static void operator delete[](void* ptr , const char* filename , int line ) ;

	public:
	#ifdef L2D_DEBUG
		static int object_instanceCount ;
		static int object_totalCount ;
	#endif

	};

} //------------------------- LIVE2D NAMESPACE ------------
