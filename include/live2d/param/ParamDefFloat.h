/**
 *  ParamDefFloat.h
 *
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */
#ifndef __LIVE2D_PARAM_DEF_FLOAT_H__
#define __LIVE2D_PARAM_DEF_FLOAT_H__


#include "../id/ParamID.h"
#include "../io/ISerializableV2.h"

//--------- LIVE2D NAMESPACE ------------
namespace live2d 
{ 
	
	class ParamDefFloat : public ISerializableV2
	{
	public:
		ParamDefFloat();
		virtual ~ParamDefFloat();
		
	public:
		virtual void readV2(BReader & br , MemoryParam* memParam ) ;
	
	public:
		
		inline float getMinValue(){ return minValue ; }
	
		
		inline float getMaxValue(){ return maxValue ; }
	
		
		inline float getDefaultValue(){ return defaultValue ; }
	
		
		inline ParamID* getParamID(){ return paramID ; }
		
	private:
		float 				minValue ;		// パラメータの最小値
		float 				maxValue ;		// パラメータの最大値
		float 				defaultValue;	// パラメータのデフォルト値
		
		ParamID *			paramID ;		//shared
	
	};

} 
//------------------------- LIVE2D NAMESPACE ------------
#endif	//__LIVE2D_PARAM_DEF_FLOAT_H__
