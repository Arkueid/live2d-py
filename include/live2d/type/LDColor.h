/**
 *  LDColor.h
 *
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */

#ifndef __LIVE2D_LDCOLOR_H__
#define __LIVE2D_LDCOLOR_H__


#ifndef __SKIP_DOC__

#include "../memory/LDObject.h"


//--------- LIVE2D NAMESPACE ------------
namespace live2d
{ 
	
	
	class LDColor   : public live2d::LDObject 
	{
	public:
		LDColor();
		LDColor(int color , bool useAlpha);
	
		virtual ~LDColor();
		
	private:
		int color;			
	};
}
//------------------------- LIVE2D NAMESPACE ------------

#endif // __SKIP_DOC__

#endif	// __LIVE2D_LDCOLOR_H__
