/**
 *  LDPointF.h
 *
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */
#ifndef __LIVE2D_LDPOINTF_H__
#define __LIVE2D_LDPOINTF_H__


#ifndef __SKIP_DOC__

#include "../Live2D.h"
#include "../memory/LDObject.h"

//--------- LIVE2D NAMESPACE ------------
namespace live2d 
{ 
	
	class LDPointF  : public live2d::LDObject 
	{
	public:
		LDPointF();
		LDPointF( l2d_pointf x , l2d_pointf y ) ;
	
		virtual ~LDPointF();
		
	public:
		l2d_pointf x;		// X
		l2d_pointf y;		// Y
	};

} //------------------------- LIVE2D NAMESPACE ------------

#endif // __SKIP_DOC__

#endif		// __LIVE2D_LDPOINTF_H__
