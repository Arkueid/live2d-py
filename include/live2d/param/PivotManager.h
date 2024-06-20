/**
 *  PivotManager.h
 *
 * 
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */
#ifndef __LIVE2D_PIVOT_MANAGER_H__
#define __LIVE2D_PIVOT_MANAGER_H__


#ifndef __SKIP_DOC__

#include "../Live2D.h"
#include "../ModelContext.h"
#include "../io/ISerializableV2.h"

//------------ LIVE2D NAMESPACE ------------
namespace live2d
{ 
	class ParamPivots ;
	class ParamID ;
	
	
	
	class PivotManager : public ISerializableV2 
	{
	public:
		PivotManager();
		virtual ~PivotManager();

	public:
		virtual void readV2(BReader & br , MemoryParam* memParam ) ;
		void initDirect(MemoryParam*	memParam) ;
		int calcPivotValue(ModelContext & mdc , bool *ret_paramOutside );
		void calcPivotIndexies( unsigned short* array64 , float * tmpT_array  , int interpolateCount);
		bool checkParamUpdated( ModelContext & mdc ) const ;
	
	public:
		
		int getParamCount() const { return (int)paramPivotTable->size() ; } 
		
		
		LDVector<ParamPivots*>* getParamPivotTableRef() const { return paramPivotTable ; }
	
	
	
// ---------------------------------サンプル用-------------------------------------
#if L2D_SAMPLE
		void DUMP_PIVOT_STR( int pivotArrayNo );
#endif
//--------------------------------------------------------------------------------
		
	private: 
		//Prevention of copy Constructor 
		PivotManager( const PivotManager & ) ;				
		PivotManager& operator=( const PivotManager & ) ; 	
	
		
	private:  
		LDVector<ParamPivots*>*		paramPivotTable;		// パラメータ(paramID)ごとのピボットを設定する
	
	};
}
//------------ LIVE2D NAMESPACE ------------

#endif // __SKIP_DOC__

#endif // __LIVE2D_PIVOT_MANAGER_H__
