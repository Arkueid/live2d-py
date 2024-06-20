/**
 *  LDString.h
 * 
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */

#ifndef __LIVE2D_LD_STRING_H__
#define __LIVE2D_LD_STRING_H__


#include "../memory/LDObject.h"
#include "../io/RefString.h"
# include <string.h>

// 旧版との互換性維持のための宣言。今後使用中止予定
#define l2d_string			live2d::LDString


//--------- LIVE2D NAMESPACE ------------
namespace live2d 
{ 
	
	class LDString  : public live2d::LDObject 
	{
	public:
		// Constructor
		LDString();
		LDString( const char * s , MemoryParam* memParam = NULL ) ;
		LDString( const char * s , int length , MemoryParam* memParam = NULL) ;
		LDString( const RefString& refStr , MemoryParam* memParam = NULL) ;
		LDString( const LDString & s , MemoryParam* memParam = NULL) ;
		LDString( const char * s , int length , bool useptr , MemoryParam* memParam = NULL) ;// useptr==trueなら引数のポインタを保持する（ポインタはDestructorでL2D_DELETEされる）
	
		// Destructor
		~LDString();
	
	
	public:
	
		//  =演算子のオーバーロード
		LDString& operator=( const LDString & s ) ; 
		LDString& operator=( const char* c ) ;
	
		//  ==演算子のオーバーロード	
		bool operator==( const RefString & s ) const ;
		bool operator==( const LDString & s ) const ;
		bool operator==( const char* c ) const ;
	
		//  <演算子のオーバーロード
		bool operator<( const LDString & s ) const ;
		bool operator<( const char* c ) const ;
	
		//  >演算子のオーバーロード
		bool operator>( const LDString & s ) const ;
		bool operator>( const char* c ) const ;
	
		bool equals( const RefString& refStr ) const 
		{
			if( refStr.length != this->_length ) return false ;// サイズ違い
			const char *c1 = this->_ptr ;
			const char *c2 = refStr.ptr_not_zero_end ;
			// 文字違い（逆順なのはPARAMの比較の特性）
			for( int i = this->_length -1 ; i >= 0 ; --i )
			{
				if( c1[i] != c2[i] ) return false ;
			}
			return true ;
		}
	
		// メモリを浪費するので、基本的にデバッグ等の特定用途で用いる
		LDString operator+( const LDString & s ) const ;
		LDString operator+( const char* s ) const ;
	
		//  +=演算子のオーバーロード
		LDString& operator+=( const LDString & s ) ;
		LDString& operator+=( const char* s ) ;
	
		// 
		LDString& append( const char* p , int length ) ;
		LDString& append( int count , const char p ) ;
	
		//  長さ取得
		unsigned int length() const {	return _length ; }
	
		//  サイズ取得
		int size() const {	return _length ; }
	
		//  C言語文字列としてのポインタ取得
		const char* c_str() const 
		{ 
			return _ptr ; 
		}
	
		//  現在のポインタをクリアする
		void clear() ;
	
		//  ハッシュコード取得
		int getHashcode()
		{
			if( _hashcode == -1 ) _hashcode = calcHashcode( this->_ptr , this->_length ) ;
			return _hashcode ;
		}
	
	protected:
		void copy( const char* ptr , int length ) ;// Constructor等で用いる

		void init( const char * s , int length , bool useptr ) ;// useptr==trueなら引数のポインタを保持する（ポインタはDestructorでL2D_DELETEされる）
		int calcHashcode( const char* p1 , int length ) ;
	
	private:
		static const int DEFAULT_SIZE = 10 ;
	
	private:
		char * 				_ptr ;					// C文字列のポインタ
		int 				_length ;				// 半角文字数（メモリ確保は最後に0が入るため_length+1）
		int 				_hashcode ;				//
	
		int 				instanceNo ;			// インスタンス番号
		static int 			totalInstanceNo ;		// 通算のインスタンス番号
	
		MemoryParam* 		memParam ;				//
	};

} 
//------------------------- LIVE2D NAMESPACE -----------

#endif	// __LIVE2D_LD_STRING_H__
