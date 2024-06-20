/**
 *  UtOS_iphone.h
 *  
 *  Created by joe on 11/05/05.
 *  Copyright 2011 Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */

#ifndef __LIVE2D_UTOS_IPHONE_H__
#define __LIVE2D_UTOS_IPHONE_H__


#ifndef __SKIP_DOC__

//--------- LIVE2D NAMESPACE ------------
namespace live2d
{
	
	class UtOS_iPhone 
	{
	public:
			// 
		static void println( const char * format , va_list va ) ;
	
			//
		static long long getTimeMSec() ;
	
	private:
		UtOS_iPhone();// インスタンス化させない

		//Prevention of copy Constructor
		UtOS_iPhone( const UtOS_iPhone & ) ;
		UtOS_iPhone& operator=( const UtOS_iPhone & ) ;
		~UtOS_iPhone();
	
	
	};

}
//--------- LIVE2D NAMESPACE ------------

#endif // __SKIP_DOC__

#endif // __LIVE2D_UTOS_IPHONE_H__
