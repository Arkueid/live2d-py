/**
 *  ISerializable.h
 *
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */
#ifndef __LIVE2D_I_SERIALIZABLE_V2_H__
#define __LIVE2D_I_SERIALIZABLE_V2_H__

#include "BReader.h"

#include "../memory/LDObject.h"

//--------- LIVE2D NAMESPACE ------------
namespace live2d 
{ 
	class MemoryParam ;
	
	
	class ISerializableV2 : public LDObject 
	{
	public:
		ISerializableV2();
		virtual ~ISerializableV2();
		
	public:
		virtual void readV2(BReader &br , MemoryParam* memParam ) = 0 ;
	};
} //------------------------- LIVE2D NAMESPACE ------------

#endif		//__LIVE2D_I_SERIALIZABLE_V2_H__
