/*
 *  LDAllocator.h
 *
 *  Live2Dで使用するメモリを確保・破棄するためのクラス
 *
 *  カスタマイズしたい場合は、サブクラスを作り、malloc, freeをオーバーライドし、
 *  Live2Dの初期化時に登録する
 *
 *  Live2D::init( live2d::LDAllocator*  allocator ) ;
 *
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */

#ifndef __LIVE2D_LDALLOCATOR_H__
#define __LIVE2D_LDALLOCATOR_H__

//#include "../Live2D.h"

//--------- LIVE2D NAMESPACE ------------
namespace live2d { 


	
	class LDAllocator // LDObjectを継承しない
	{
	public:
		typedef enum {
			MAIN = 0 ,
			GPU 
#ifdef L2D_TARGET_PS4
			,SHADER,
#endif
		} Type ;

	public:
		LDAllocator();
		virtual ~LDAllocator();

		
		virtual void* pageAlloc( unsigned int size , LDAllocator::Type  allocType ) = 0 ;

		
		
		virtual void pageFree( void* ptr , LDAllocator::Type  allocType ) = 0 ;


		
		virtual void init(){}

		
		virtual void dispose(){}

	};

} 
//------------------------- LIVE2D NAMESPACE ------------

#endif
