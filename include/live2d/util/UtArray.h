/**
 *  UtArray.h
 *
 * 
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */

#ifndef __LIVE2D_UTARRAY_H__
#define __LIVE2D_UTARRAY_H__


#ifndef __SKIP_DOC__


//------------ LIVE2D NAMESPACE ------------
namespace live2d
{
	class MemoryParam ;
	
	
	class UtArray 
	{
	public:
		// 
		static float * floatArray( MemoryParam* memParam ,int num , ...  ) ;
		// 
		static void dumpPoints( float * array , int w , int h ) ;
		
	private:
		UtArray();									// インスタンス化させない

		//Prevention of copy Constructor
		UtArray( const UtArray & ) ;				
		UtArray& operator=( const UtArray & ) ; 	
	
		// Destructor
		~UtArray();
		
	};
} 
//------------ LIVE2D NAMESPACE ------------

#endif // __SKIP_DOC__

#endif		// __LIVE2D_UTARRAY_H__


