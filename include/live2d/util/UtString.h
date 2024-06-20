/**
 *  UtString.h
 *
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */

#ifndef __LIVE2D_UTSTRING_H__
#define __LIVE2D_UTSTRING_H__



#ifndef __SKIP_DOC__

#include "../type/LDVector.h"



//--------- LIVE2D NAMESPACE ------------
namespace live2d 
{ 
	class UtString 
	{
	public:
		
		// 戻り値が複製されるのでオーバーヘッドは大きい。デバッグなどで使う
		static live2d::LDString toString( const char * msg , ... ) ;
	
		// textがstartWordで始まっている場合にtrueを返す
		static bool startsWith( const char * text , const char * startWord ) ;
	
		// mtnで定義されるfloat文字を解釈する。指数表示には未対応
		static float strToFloat( const char* str , int len , int pos , int* ret_endpos ) ;
	
	private:
		UtString();									
	
		//Prevention of copy Constructor
		UtString( const UtString & ) ;				
		UtString& operator=( const UtString & ) ; 	
		
		~UtString();
	
	};
	
}
//------------------------- LIVE2D NAMESPACE ------------

#endif // __SKIP_DOC__

#endif		// __LIVE2D_UTSTRING_H__
