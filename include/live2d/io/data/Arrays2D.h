/**
 *  Arrays2D.h
 *
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */
#ifndef __LIVE2D_ARRAYS2D_H__
#define __LIVE2D_ARRAYS2D_H__


#ifndef __SKIP_DOC__

#include "../../memory/LDObject.h"

//--------- LIVE2D NAMESPACE ------------
namespace live2d
{ 
	
	/**********************************************************************
	@brief		多次元配列を戻り値にするためのクラス
	
	展開後は、単純なポインタとして扱う
	Destructorでは、中身は削除されない!!
	**********************************************************************/
	class Arrays2D : public live2d::LDObject 
	{
	public:
		Arrays2D( void** ptr , int size1 , int size2 );
		virtual ~Arrays2D();
	
	private:
		void ** 		ptr ;
		int 			size1 ;
		int 			size2 ; 	
	};
	

}
//------------------------- LIVE2D NAMESPACE ------------

#endif // __SKIP_DOC__

#endif		// __LIVE2D_ARRAYS2D_H__
