/**
 *  PartsDataID.h
 * 
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */
#ifndef __LIVE2D_PARTS_DATA_ID_H__
#define __LIVE2D_PARTS_DATA_ID_H__


#include "../Live2D.h"
#include "ID.h"

//------------ LIVE2D NAMESPACE ------------
namespace live2d
{ 
	
	class PartsDataID  : public ID
	{
	public:
		const static int L2D_PARTS_ID_INITIAL_CAPACITY = 64 ;
		
	public:
		// IDを取得する 
		// ・同一の値を持つ全てのIDが、同じポインタを指すことを保証する （同一の確認が　== 比較 で良い） \n
		// ・Live2D::dispose()時に解放される
		static PartsDataID * getID(const live2d::LDString &str ) ;

		static PartsDataID * getID(  const RefString& refStr ) ;
	
		// リリース用メソッド（Live2D::diposeから呼ばれる)
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
		// Constructor
		PartsDataID() ;									
		PartsDataID( const char* str ) ;					
		PartsDataID(const live2d::LDString &str );		

		//Prevention of copy Constructor	
		PartsDataID( const RefString& refStr ) ;		
		PartsDataID( const PartsDataID & ) ;			
		PartsDataID& operator=( const PartsDataID & ) ; 

		// Destructor
		virtual ~PartsDataID();
		
	private:	
		//------------ static ------------
		static LDVector<PartsDataID*>*		idlist ;
		
	private:	
		//------------ this ------------
		live2d::LDString 					id ;// 生成時に複製されているため、Destructorで開放する
	};
}
//------------ LIVE2D NAMESPACE ------------

#endif		// __LIVE2D_PARTS_DATA_ID_H__
