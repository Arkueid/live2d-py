/*
 *  AvatarPartsItem.h
 *
 *  Copyright 2010 Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */
#ifndef __LIVE2D_AVATAR_PARTS_ITEM_H__
#define __LIVE2D_AVATAR_PARTS_ITEM_H__


#ifndef __SKIP_DOC__


#include <stdio.h>
#include "../Live2D.h"
#include "../io/ISerializableV2.h"
#include "../model/PartsData.h"

//------------ LIVE2D NAMESPACE ------------
namespace live2d
{ 
	class BReader ;
	class IBaseData ;
	class IDrawData ;
	class PartsDataID ;
	
	class AvatarPartsItem  : public ISerializableV2 
	{
	public:
		static int INSTANCE_COUNT;
	
	public:
		AvatarPartsItem() ;
		virtual ~AvatarPartsItem() ;
	
	public:
		virtual void readV2(BReader & br , MemoryParam* memParam ) ;
		void replacePartsData( PartsData* parts ) ;
	
	public:
		LDVector<IBaseData*>* getBaseDataList(){ return baseDataList ; }
		LDVector<IDrawData*>* getDrawDataList(){ return drawDataList ; }
	
	private:
		// Prevention of copy Constructor
		AvatarPartsItem( const AvatarPartsItem & ) ;
		AvatarPartsItem& operator=(const AvatarPartsItem &) ;
		
	private:
		PartsDataID * partsID ;					
		
		LDVector<IBaseData*>*	baseDataList ;	
		LDVector<IDrawData*>*	drawDataList ;	
	
	} ;
//------------ LIVE2D NAMESPACE ------------
}

#endif  // __SKIP_DOC__

#endif	// __LIVE2D_AVATAR_PARTS_ITEM_H__

