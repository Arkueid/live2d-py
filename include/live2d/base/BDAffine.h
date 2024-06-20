/**
 *  BDAffine.h
 *
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */
#ifndef __LIVE2D_BD_AFFINE_ENT_H__
#define __LIVE2D_BD_AFFINE_ENT_H__


#ifndef __SKIP_DOC__

#include "../param/PivotManager.h"
#include "IBaseData.h"
#include "IBaseContext.h"

//------------ LIVE2D NAMESPACE ------------
namespace live2d
{
	class AffineEnt ;
	
	
	class BDAffine :public IBaseData 
	{
	public:
		BDAffine();
		virtual ~BDAffine();
		
	public:
		void initDirect(MemoryParam* memParam) ;
	
		virtual void readV2(BReader & br , MemoryParam* memParam ) ;
	
		virtual IBaseContext* init(ModelContext &mdc) ;
	
		virtual void setupInterpolate(ModelContext &mdc, IBaseContext* cdata ) ;
	
		virtual void setupTransform(ModelContext &mdc, IBaseContext* cdata ) ;

		virtual void transformPoints( ModelContext &mdc, IBaseContext* cdata 
				, l2d_pointf * srcPoints , l2d_pointf * dstPoints 
				, int numPoint , int pt_offset , int pt_step ) ;
		
		
		/***************************************************************
		@brief	変換の型を返す
		@return	IBaseData::TYPE_BD_AFFINE
		***************************************************************/
		virtual int getType()
		{
			return IBaseData::TYPE_BD_AFFINE ;
		}
		
	
#if L2D_DEBUG		
		virtual void dump() ;
#endif
	
	private:
		void getDirectionOnDst(ModelContext & mdc 
							   , IBaseData* targetToDst, IBaseContext* targetToDstContext
							   , l2d_pointf* srcOrigin , l2d_pointf* srcDir , l2d_pointf* retDir ) ;
	
	private:
		PivotManager*			pivotManager ;	
		LDVector<AffineEnt*>*	affines ;		
	
	};
	
	
	class BDAffineContext : public IBaseContext 
	{
	public:
		BDAffineContext( BDAffine *src ) ;
		virtual ~BDAffineContext() ;
		
	public:		
		int 			tmpBaseDataIndex ;		// IBaseDataのインデックス。不変なので一度取得すればよい
		AffineEnt*		interpolatedAffine ;	//
		AffineEnt*		transformedAffine ;		//
	} ;
}
// --------------------------- LIVE2D NAMESPACE ------------

#endif // __SKIP_DOC__

#endif	// __LIVE2D_BD_AFFINE_ENT_H__
