/*
 *  RefString.h
 *
 *  ロード中に使用する文字列参照
 *  破棄してはならない
 *
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */
 
#ifndef __LIVE2D_REF_STRING__
#define __LIVE2D_REF_STRING__


#ifndef __SKIP_DOC__

//--------- LIVE2D NAMESPACE ------------
namespace live2d 
{ 

	class RefString
	{
	public:
		RefString();
		RefString(const char* ptr , int length )
		{
			this->ptr_not_zero_end = ptr ;
			this->length = length ;
		}
		~RefString();


		void set(const char* ptr , int length )
		{
			this->ptr_not_zero_end = ptr ;
			this->length = length ;
		}

	public:
		const char*		ptr_not_zero_end ;// 終端が\0とは限らない
		int				length ;
	};

}
//--------- LIVE2D NAMESPACE ------------

#endif // __SKIP_DOC__

#endif // __LIVE2D_REF_STRING__
