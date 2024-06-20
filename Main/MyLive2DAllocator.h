/**
 *
 *  You can modify and use this source freely
 *  only for the development of application related Live2D.
 *
 *  (c) Live2D Inc. All rights reserved.
 */

// Live2D
#include <stdlib.h>

#include	<live2d/Live2D.h>
#include	<live2d/util/UtDebug.h>




class MyLive2DAllocator : public live2d::LDAllocator{
public:

	
	MyLive2DAllocator() {
	}

	virtual ~MyLive2DAllocator() {
	}

	//---------------------------------------------------------------------------
	
	//
	
	//---------------------------------------------------------------------------
	virtual void init(){ }

	//---------------------------------------------------------------------------
	
	//
	
	//---------------------------------------------------------------------------
	virtual void dispose(){ }


	//---------------------------------------------------------------------------
	// malloc
	//
	
	//---------------------------------------------------------------------------
	virtual void* pageAlloc( unsigned int size , LDAllocator::Type  allocType ){
		void* ptr ;
		switch( allocType ){
		case LDAllocator::MAIN:	
			ptr = ::malloc(size) ;
			break ;

		case LDAllocator::GPU:	
			ptr = ::malloc(size) ;
			break ;

		default:				
			L2D_DEBUG_MESSAGE( "Alloc type not implemented %d" , allocType ) ;
			ptr = ::malloc(size) ;
			break ;
		}

		L2D_ASSERT_S( ptr != NULL , "MyAllocator#malloc failed (size= %d)" , size ) ;
		return ptr ;
	}

	//---------------------------------------------------------------------------
	// free
	//---------------------------------------------------------------------------
	virtual void pageFree( void* ptr , LDAllocator::Type  allocType ){
		::free(ptr);
	}

} ;

