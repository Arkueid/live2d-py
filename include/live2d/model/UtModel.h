/**
 *  UtModel.h
 *
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */
#ifndef __LIVE2D_UT_MODEL_H__
#define __LIVE2D_UT_MODEL_H__


#ifndef __SKIP_DOC__

#include "../Live2D.h"


//------------ LIVE2D NAMESPACE ------------
namespace live2d
{ 
	class ALive2DModel ;
		
	class UtModel
	{
	public:
	

	private:
		UtModel();		
				
		// Prevention of copy Constructor					
		UtModel( const UtModel & ) ;				
		UtModel& operator=( const UtModel & ) ;	

		~UtModel();									
	};
}
//------------ LIVE2D NAMESPACE ------------

#endif // __SKIP_DOC__

#endif		// __LIVE2D_UT_MODEL_H__

