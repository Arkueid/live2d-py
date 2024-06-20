/**
 *  ParamDefSet.h
 *
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */
#ifndef __LIVE2D_PARAMDEFSET_H__
#define __LIVE2D_PARAMDEFSET_H__



#include "../io/ISerializableV2.h"
#include "ParamDefFloat.h"

//--------- LIVE2D NAMESPACE ------------
namespace live2d
{ 
	
	class ParamDefSet : public ISerializableV2
	{
	public:
		ParamDefSet();
		virtual ~ParamDefSet();

	public:
		void initDirect(MemoryParam* memParam) ;
	
		
		LDVector<ParamDefFloat*>* getParamDefFloatList(){ return paramDefList ; }
	
		
		virtual void readV2(BReader & br , MemoryParam* memParam ) ;
	
	private:
		LDVector<ParamDefFloat*>*	paramDefList ;		// パラメータ定義リスト
	
	};
}
//------------------------- LIVE2D NAMESPACE ------------

#endif		// __LIVE2D_PARAMDEFSET_H__
