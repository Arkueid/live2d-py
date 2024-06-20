/*************************************************************************
 * Json
 * 
 * Ascii文字のみ対応した最小限の軽量JSONパーサ。
 * 仕様はJSONのサブセットとなる。
 * 
 * 設定ファイルなどのロード用
 * 
 * 未対応項目
 * ・日本語などの非ASCII文字
 * ・e による指数表現
 *
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 * version 1.0.2
 ***************************************************************************/
#ifndef __LIVE2D_JSON_H__
#define __LIVE2D_JSON_H__

#include "stdio.h"
#include "../Live2D.h"
#include "../memory/LDObject.h"
#include "../type/LDVector.h"
#include "../type/LDMap.h"
#include "../type/LDString.h"


//------------ LIVE2D NAMESPACE ------------
namespace live2d
{ 
	class Value;
	class Error;
	class NullValue;
		
	#define L2D_JSON_ERROR_TYPE_MISMATCH			"Error:type mismatch"
	#define L2D_JSON_ERROR_INDEX_OUT_OF_BOUNDS		"Error:index out of bounds"
	
	
	class Value : public live2d::LDObject 
	{
	private:
		friend class Array;

	public:
		static Value * 	ERROR_VALUE;			// 一時的な返り値として返すエラー。L2D_DELETEしないようにする。rootとして保持する場合はnew Error(とする
												// ERRORからERROR_VALUEに修正 ( DirectXにてWinGDI.hのERRORマクロと重複して問題になるため )
		static Value * 	NULL_VALUE;				// 一時的な返り値として返すNULL。L2D_DELETEしないようにする
		
	public:
		// Constructor
		Value(MemoryParam* memParam):memParam(memParam){}

		// Destructor
		virtual ~Value(){}

		// 
		virtual live2d::LDString toString( const live2d::LDString& defaultV = "" , const live2d::LDString& indent = "") const = 0;

		// 
		virtual const char* c_str( const live2d::LDString& defaultV = "" , const live2d::LDString& indent = ""){ return this->toString(defaultV, indent).c_str(); } 
		
		// 
		virtual int toInt( int defaultV = 0 ){ return defaultV; }

		// 
		virtual double toDouble( double defaultV = 0 ){ return defaultV; }

		// 
		virtual bool toBoolean( bool defaultV = false ){ return defaultV; }

		//
		virtual int size(){ return 0; }
		
		//
		virtual LDVector<Value*>* getVector( LDVector<Value*>* defaultV = NULL ){ return defaultV; }
		
		//
		virtual l2d_map<live2d::LDString , Value*>* getMap( l2d_map<live2d::LDString , Value*>* defaultV = NULL ){ return defaultV; }
		
		//
		virtual Value &operator[]( int index ) { return *(ERROR_VALUE->setError_notForClientCall(L2D_JSON_ERROR_TYPE_MISMATCH)); }
		
		//
		virtual Value &operator[]( const live2d::LDString& s ) { return *(ERROR_VALUE->setError_notForClientCall(L2D_JSON_ERROR_TYPE_MISMATCH)); }
		
		// 
		virtual LDVector<live2d::LDString>& getKeys()
		{ 
			return *dummyKeys2; 
		} 

		//
		virtual bool isError(){ return false; }
		
		// NULL
		virtual bool isNull(){ return false; }
		
		// 
		virtual bool isBool(){ return false; }

		// Number
		virtual bool isDouble(){ return false; }

		// 
		virtual bool isString(){ return false; }

		// 
		virtual bool isArray(){ return false; }

		// Object 
		virtual bool isMap(){ return false; }

		//
		virtual bool equals( const live2d::LDString& v ){ return false; }

		//
		virtual bool equals( const char* v ){ return false; }
		
		// 
		virtual bool equals( int v ){ return false; }

		// 
		virtual bool equals( double v ){ return false; }

		//
		virtual bool equals( bool v ){ return false; }
		
		// 
		virtual bool isStatic(){ return false; }// staticなら解放しない

		//
		virtual Value* setError_notForClientCall(const char* errorStr){ return ERROR_VALUE; }//Override by Error#setError

		// 初期化用メソッド（Live2D::initから呼ばれる)
		static void staticInit_notForClientCall();

		// リリース用メソッド（Live2D::diposeから呼ばれる)
		static void staticRelease_notForClientCall();

	protected:
		MemoryParam*	memParam ;

	private:
		static LDVector<live2d::LDString> *dummyKeys2 ;
	};

		
		
	
	class Json  : public live2d::LDObject 
	{
	public:
		static const int UTF8 = 0;			// UTF-8
		static const int SJIS = 1;			// 

	public:
		// Constructor
		Json(); 
		Json( const char * buf , int length , int encoding = UTF8 ); 

		// Destructor
		virtual ~Json();

	public:
		// 
		void release();

		// 
		bool parseFile( const char * filepath , int encoding = UTF8 );

		//
		bool parseBytes( const char * buf , int length , int encoding = UTF8 );

		// 
		Value& getRoot();
		
		//
		const char* getError(){return error;} 

		// 
		bool checkEOF(){ return (*root)[1].equals("EOF"); }

		// staticでJSONの解析結果を生成
		// buf は外部で管理（破棄）する必要がある
	public:
		// 
		static Json* parseFromBytes( const char * buf, int length, int encoding = UTF8 );

		//
		static Json* parseFromFile( const char * filepath, int encoding = UTF8 ); 

	protected:
		//
		bool parse_exe(const char* str, int len );

		// 
		live2d::LDString parseString( const char* str, int len, int _pos, int* ret_endpos );

		//
		Value *parseObject( const char* buf, int len, int _pos, int* ret_endpos );

		//
		Value *parseArray( const char* buf, int len, int _pos, int* ret_endpos );

		// 
		Value *parseValue( const char* buf, int len, int _pos, int* ret_endpos2 );
		
		
	private:
		int encoding;			// マルチバイト文字のエンコーディング
		int line_count;			// エラー報告用
		const char *error;		// パース時のエラー
		Value *root;			// パースされたルートデータ

		MemoryParam*	memoryManagement ;
		AMemoryHolder*	memoryHolderFixedMain ;
	}; 	
		

	
	class Double : public Value
	{
	public:
		// Constructor
		Double(double v) : Value(NULL) 
		{
			this->value = v; 
		}
		
		// Destructor
		virtual ~Double(){}

	public:
		// Number
		virtual bool isDouble(){ return true; }

		// 主にデバッグ用
		virtual live2d::LDString toString( const live2d::LDString &defaultV = "", const live2d::LDString& indent = "") const 
		{ 

#if defined( L2D_TARGET_D3D ) || defined( L2D_TARGET_D3D11 ) || defined ( L2D_TARGET_WIN_GL ) ||defined (L2D_TARGET_D3D_OPEN)
			char strbuf[32] = {'\0'};
			_snprintf_s(strbuf, 32, 32, "%f", this->value );
			return strbuf;

#else
			// string stream 未対応
			char strbuf[32] = {'\0'};
			snprintf(strbuf, 32, "%f", this->value );
			return strbuf;
#endif
		}

		// 
		virtual int toInt( int defaultV = 0 ){ return (int)this->value ; }

		// 
		virtual double toDouble( double defaultV = 0 ){ return this->value ; }

		//
		virtual bool equals( double v ){ return v == this->value ; }

		// 
		virtual bool equals( const live2d::LDString& v ){ return false ; }

		//
		virtual bool equals( const char* v ){ return false ; }
		
		// 
		virtual bool equals( int v ){ return false ; }

		
		// 
		virtual bool equals( bool v ){ return false ; }

	private:
		double value ;
	};


	
	class Boolean : public Value
	{
	private:
		friend class Value ;
		
	public:
		static Boolean* TRUE_VALUE;				//
		static Boolean* FALSE_VALUE;				//
		
	public:
		// Destructor
		virtual ~Boolean(){} 

		//
		virtual bool isBool(){ return true; }

		// 
		virtual bool toBoolean( bool defaultV = false ){ return boolValue; }

		// 
		virtual live2d::LDString toString( const live2d::LDString &defaultV = "", const live2d::LDString &indent = "") const
		{ 
			return boolValue ? "true" : "false";
		}

		// 
		virtual bool equals( bool v ){ return v == boolValue; }

		//
		virtual bool equals( const live2d::LDString& v ){ return false; }

		// 
		virtual bool equals( const char* v ){ return false; }

		// 
		virtual bool equals( int v ){ return false; }

		// 
		virtual bool equals( double v ){ return false; }

		// staticなら開放しない
		virtual bool isStatic(){ return true; }
		
	private:
		Boolean(bool v) : Value(NULL) { this->boolValue = v; }
		bool boolValue ;
	};
		

	
	class String : public Value
	{
	public:
		// Constructor
		String(MemoryParam* memParam , const live2d::LDString &s):Value(memParam){ this->str = s; }
		String(MemoryParam* memParam , const char *s):Value(memParam){ this->str = s; }

		// Destructor
		virtual ~String(){}

		// 
		virtual bool isString(){ return true; }

		// 
		virtual live2d::LDString toString( const live2d::LDString &defaultV = "", const live2d::LDString &indent = "") const
		{ 
			return str;
		}

		// 
		virtual bool equals( const live2d::LDString& v ){ return (str == v); }

		// 
		virtual bool equals( const char* v ){ return (str == v); }

		
		// 
		virtual bool equals( int v ){ return false; }

		//
		virtual bool equals( double v ){ return false; }

		//
		virtual bool equals( bool v ){ return false; }

	protected:
		live2d::LDString str;			// 
	};
		

	
	class Error : public String
	{
	private:
		friend class Value;			// 
		friend class Array;			// 
		friend class Json;			// 
		
	public:
		// 
		virtual bool isStatic(){ return isStatic_; }

		//
		virtual Value* setError_notForClientCall(const char *s){ this->str = s; return this; }

	protected:
		// Constructor
		Error(MemoryParam* memParam , const live2d::LDString &s, bool isStatic ) :  String(memParam,s), isStatic_(isStatic){}

		// Destructor
		virtual ~Error(){}

		virtual bool isError(){ return true; }
		
	protected:
		bool isStatic_;			// 
	};


	
	class NullValue : public Value
	{
	private:
		friend class Value;			//
		friend class Json;			// 

	public:
		// Destructor
		virtual ~NullValue(){}

	public:
		// NULLチェック
		virtual bool isNull(){ return true; }

		// 
		virtual live2d::LDString toString( const live2d::LDString &defaultV = "", const live2d::LDString &indent = "") const
		{ 
			return "NullValue" ;
		}

		// 
		virtual bool isStatic(){ return true; }

	private:
		// Constructor
		NullValue():Value(NULL){}
	};


	
	class Array : public Value 
	{
	public:
		// Constructor
		Array(MemoryParam* memParam) : Value(memParam) , array(memParam) {}
		
		// Destructor
		virtual ~Array() ;

	public:
		// Arrayチェック
		virtual bool isArray(){ return true; }
		
		// 
		virtual Value &operator[]( int index )
		{ 
			if( index < 0 || (int)array.size() <= index ) return *(ERROR_VALUE->setError_notForClientCall(L2D_JSON_ERROR_INDEX_OUT_OF_BOUNDS)) ;
			Value * v = array[ index ] ;
			
			if( v == NULL ) return *Value::NULL_VALUE ;
			return *v ;
		}


		// 
		virtual Value &operator[]( const live2d::LDString& s ) { return *(ERROR_VALUE->setError_notForClientCall(L2D_JSON_ERROR_TYPE_MISMATCH)) ; }

		// 主にデバッグ用
		virtual live2d::LDString toString( const live2d::LDString &defaultV = "", const live2d::LDString &indent = "") const
		{
			live2d::LDString ret = indent + "[\n"; 
			LDVector<Value*>::const_iterator ite = array.begin();
			for( ; ite != array.end() ; ite++ )
			{
				Value* v = (*ite);
				ret += indent + "	" + v->toString(indent+"	") + "\n";
			}
			ret += indent + "]\n";
			return ret;
		}

		// 
		void add( Value* v ){ array.push_back( v , false ); }
		
		// 
		virtual LDVector<Value*>* getVector( LDVector<Value*>* defaultV = NULL ){ return &array ; }

		// 
		virtual int size(){ return (int)array.size() ; }

	private:
		LDVector<Value*> array ;			//
	};

	
	class Map : public Value
	{
	public:
		// Constructor
		Map(MemoryParam* memParam) : Value(memParam) , keys(NULL){}

		// Destructor
		virtual ~Map();
		
	public:
		// Mapチェック 
		virtual bool isMap(){ return true; }

		// 
		virtual Value &operator[]( const live2d::LDString& s )
		{ 
			Value* ret = map[ s ];			
			if( ret == NULL )
			{
				return *Value::NULL_VALUE;
			}
			return *ret ;
		}

		// 
		virtual Value &operator[]( int index ) { return *(ERROR_VALUE->setError_notForClientCall(L2D_JSON_ERROR_TYPE_MISMATCH)); }

		// 
		virtual live2d::LDString toString( const live2d::LDString& defaultV = "", const live2d::LDString& indent = "") const
		{ 			
			live2d::LDString ret = indent + "{\n";
			l2d_map<live2d::LDString, Value*>::const_iterator ite = map.begin();
			while( ite != map.end() )
			{
				const live2d::LDString& key = (*ite).first;
				Value *v = (*ite).second;
				
				ret += indent + "	" + key + " : " + v->toString(indent+"	") + "\n"; 
				++ite;
			}
			ret += indent + "}\n";
			return ret ; 
		}
		
		// 
		virtual l2d_map<live2d::LDString, Value*> *getMap(l2d_map<live2d::LDString, Value*> *defaultV = NULL )
		{
			return &map; 
		}
		
		// 
		void put( live2d::LDString &key, Value *v )
		{
			map[ key ] = v;
		}
		
		// 
		virtual LDVector<live2d::LDString> &getKeys()
		{
			if( !keys )
			{
				keys = L2D_NEW(memParam)  LDVector<live2d::LDString>(memParam);
				l2d_map<live2d::LDString , Value*>::const_iterator ite = map.begin();
				while( ite != map.end() )
				{
					const live2d::LDString& key = (*ite).first;
					keys->push_back(key,true);
					++ite;
				}
			}
			return *keys;
		}

		// 
		virtual int size(){ return (int)keys->size(); }

	private:
		l2d_map<live2d::LDString, Value*>	map;			//
		LDVector<live2d::LDString> *keys;				//
	};
}
//------------ LIVE2D NAMESPACE ------------

#endif		// __LIVE2D_JSON_H__
