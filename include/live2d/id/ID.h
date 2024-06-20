/**
 * ID.h
 *
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */
#ifndef __LIVE2D_ID_H__
#define __LIVE2D_ID_H__


#include "../io/ISerializableV2.h"
#include "../memory/LDObject.h"

//--------- LIVE2D NAMESPACE ------------
namespace live2d
{ 
	
	class ID : public live2d::LDObject 
	{
	public:
		ID();
		virtual ~ID();
		
	public:
		//  リリース用メソッド（Live2D::diposeから呼ばれる)
		static void staticInit_notForClientCall() ;
		
		//  リリース用メソッド（Live2D::diposeから呼ばれる)
		static void staticRelease_notForClientCall() ;
	
	protected:
		static MemoryParam*				memParam ;
		static live2d::AMemoryHolder*	memoryHolderFixedMain ;
	
	};

} 
//------------------------- LIVE2D NAMESPACE ------------


#endif		// __LIVE2D_ID_H__
