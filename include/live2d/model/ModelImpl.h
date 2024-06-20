/**
 *  ModelImpl.h
 *
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */
#ifndef __LIVE2D_MODEL_IMPL_H__
#define __LIVE2D_MODEL_IMPL_H__

#include <stdio.h>
#include "../io/ISerializableV2.h"

//------------ LIVE2D NAMESPACE ------------
namespace live2d
{ 
	class BReader ;
	class PartsData ;
	class IBaseData ;
	class ParamDefSet ;
	class MemoryParam ;
	
	
	class ModelImpl : public ISerializableV2
	{
	public:
		static int INSTANCE_COUNT ;
	
	public:
		ModelImpl(void) ;
		virtual ~ModelImpl(void) ;
	
	public:
		void initDirect() ;
		virtual void readV2(BReader & br , MemoryParam* memParam ) ;
	
		void reset() ;
		
		
		void addPartsData( PartsData * parts ){	partsDataList->push_back( parts , false ) ;	}
		
		
		LDVector<PartsData*> * getPartsDataList(){ 	return partsDataList ;	} 
	
		
		ParamDefSet* getParamDefSet(){ return paramDefSet ; }
		
		
		float getCanvasWidth(){ return (float)canvasWidth ; }
	
		
		float getCanvasHeight(){ return (float)canvasHeight ; }
			
#ifdef L2D_DEBUG
		void dump() ;
#endif
	
	private:
		// Prevention of copy Constructor
		ModelImpl( const ModelImpl & ) ;			
		ModelImpl& operator=( const ModelImpl & ) ; 
	
		
	private:
		ParamDefSet*			paramDefSet ;
		LDVector<PartsData*>*	partsDataList ;
		
		int						canvasWidth ;
		int						canvasHeight ;
	
		MemoryParam*			memoryManagement ;// 自身と同時に破棄されるオブジェクトのメモリを管理
		AMemoryHolder*			memoryHolderFixedMain ;
		AMemoryHolder*			memoryHolderFixedGPU ;
	};
	
} 
//------------ LIVE2D NAMESPACE ------------

#endif		// __LIVE2D_MODEL_IMPL_H__
