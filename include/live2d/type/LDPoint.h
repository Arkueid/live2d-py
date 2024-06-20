/**
 *  LDPoint.h
 *
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */

#ifndef __LIVE2D_LDPOINT_H__
#define __LIVE2D_LDPOINT_H__


#ifndef __SKIP_DOC__

#include "../memory/LDObject.h"


//--------- LIVE2D NAMESPACE ------------
namespace live2d 
{ 
	
	
	
	class LDPoint  : public live2d::LDObject 
	{
	public:
		LDPoint();
		LDPoint(int x , int y);
	
		virtual ~LDPoint();
	
	public:
		int x ;			//
		int y ;			//
	
	};
	
}
//------------------------- LIVE2D NAMESPACE ------------

#endif //__SKIP_DOC__

#endif	// __LIVE2D_LDPOINT_H__

