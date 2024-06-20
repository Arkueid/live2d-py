/**
 *  DrawDataID.h
 *
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */
#ifndef __LIVE2D_DRAW_DATA_ID_H__
#define __LIVE2D_DRAW_DATA_ID_H__


#include "../Live2D.h"
#include "ID.h"

//------------ LIVE2D NAMESPACE ------------
namespace live2d
{ 
	
	class DrawDataID  : public ID
	{
	public:
		const static int L2D_DRAWDATA_ID_INITIAL_CAPACITY = 256 ;
	
	
	public:
	
		// IDを取得する
		// ・同一の値を持つ全てのIDが、同じポインタを指すことを保証する （同一の確認が　== 比較 で良い） \n
		// ・Live2D::dispose()時に解放される
		static DrawDataID * getID( const LDString & str ) ;

		static DrawDataID * getID(  const RefString& refStr ) ;
	
		static void releaseStored_notForClientCall() ;
	
	
	public:
		/****************************************************************************
		@~english

		@~japanese
		@brief		IDをC言語の文字列として取得
		@return		ID文字列
		****************************************************************************/
		const char * toChar(){ return id.c_str() ; }

		
	private:	
		DrawDataID() ;									
		DrawDataID( const char* str ) ;					
		DrawDataID( const RefString& refStr  ) ;		
		DrawDataID(const LDString & tmp_str);			
	
		//Prevention of copy Constructor
		DrawDataID( const DrawDataID & ) ;				
		DrawDataID& operator=( const DrawDataID & ) ;	

		virtual ~DrawDataID();
		
	private:	
		static LDVector<DrawDataID*>* idlist ;
		
	private:	
		LDString id ;
	};
} 
//------------ LIVE2D NAMESPACE ------------

#endif		// __LIVE2D_DRAW_DATA_ID_H__


