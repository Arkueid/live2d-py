/**
 *  BReader.h
 *
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */
#ifndef __LIVE2D_B_READER_H__
#define __LIVE2D_B_READER_H__


#ifndef __SKIP_DOC__


#include "../memory/LDObject.h"
#include "ByteBuffer.h"

#include "../type/LDVector.h"

//--------- LIVE2D NAMESPACE ------------
namespace live2d
{
	class AMemoryHolder ;
	
	
	class BReader : public live2d::LDObject 
	{
	public:
		static const int FLAG_READ_AS_USHORT_ARRAY	= 0x1 ;// int[]を、ロード時にushort[]として扱う
		static const int FLAG_MALLOC_ON_GPU			= 0x2 ;// GPU上にメモリを確保する
	
	
		// ロード時にオブジェクトを保管するテーブルのサイズ
		// 小さいとvectorのresizeが発生する。大きいとメモリを圧迫する
		// 使用メモリは sizeof(void*) * LOAD_OBJECT_INITIAL_CAPACITYとなる
		// 今後は、モデルの中にオブジェクト数を埋め込む方式に変更する
		static const int LOAD_OBJECT_INITIAL_CAPACITY = 10000 ; 
	
	
		// 開放の方法が確定せず安全ではないため必ず外部で開放するように変更
		//BReader( const char * buf , int length , bool deleteBufOnDestructor );
		BReader( const char * buf , int length );
		virtual ~BReader();
		
		//  ファイルフォーマットのバージョン 
		int getFormatVersion(){ return formatVersion ; }
	
		void setFormatVersion(int version){ formatVersion = version ; }
		
		void rollback( int byteLen ) ;// 指定のバイト数戻す
	
		// Flagsには以下を渡す
		//	FLAG_READ_AS_USHORT_ARRAY	= 0x1 ;// int[]を、ロード時にushort[]として扱う
		//	FLAG_MALLOC_ON_GPU			= 0x2 ;// GPU上にメモリを確保する
		void * readObject( MemoryParam * memParam , int cno = -1 , int flags = 0 ) ;
		bool readBit() ;
		int readNum(){ return bin.readNum() ; } ;// 可変長の長さを取得する(1..4バイトまでを長さに応じて使い分ける
		float readFloat(){	checkBits() ; return bin.readFloat() ;	}
		double readDouble(){ checkBits() ; return bin.readDouble() ;	}
		long long readLong(){ checkBits() ; return bin.readLong() ;	}
		int readInt() { checkBits() ; return bin.readInt() ;}
		bool readBoolean(){ checkBits() ; return bin.readBoolean() ;}
		char readByte(){ checkBits() ; return bin.readByte() ;}// 1byte分返す（javaのバイトと対応。charはJavaと型が異なるので名称から回避）
		short readShort(){ checkBits() ; return bin.readShort() ;	}
	
		//  （ポインタ）を返す。 外部でL2D_FREE( )が必要
		double* readArrayDouble(MemoryParam* memParam,int *ret_length){ checkBits() ; return bin.readArrayDouble(memParam, ret_length) ;	}// size, float* を戻す必要がある
		float* readArrayFloat(MemoryParam* memParam,int *ret_length){ checkBits() ; return bin.readArrayFloat(memParam, ret_length) ;	}// size, float* を戻す必要がある
	
		int * readArrayInt(MemoryParam* memParam,int *ret_length){ checkBits() ; return bin.readArrayInt(memParam, ret_length ) ;}
		unsigned short * readArrayIntAsUShort(MemoryParam* memParam,int *ret_length){ checkBits() ; return bin.readArrayIntAsUShort(memParam, ret_length ) ;}
	
		LDVector<double>* readArrayDoubleAsVector(MemoryParam* memParam){ checkBits() ; return bin.readArrayDoubleAsVector(memParam) ;	}// size, float* を戻す必要がある
		LDVector<float>* readArrayFloatAsVector(MemoryParam* memParam){ checkBits() ; return bin.readArrayFloatAsVector(memParam) ;	}// size, float* を戻す必要がある
		LDVector<int> * readArrayIntAsVector(MemoryParam* memParam){ checkBits() ; return bin.readArrayIntAsVector(memParam) ;}
		
		//  マルチバイト文字は未検証（bytesで長さを指定しているのでここでのエラーはでない） 
		live2d::LDString* readString(MemoryParam* memParam ){ checkBits() ; return bin.readString(memParam) ;	}
		RefString& readStringAsRef(){ checkBits() ; return bin.readStringAsRef() ;	}
	
		// cのポインタを返し、終端記号まで進める
		const char* readCStr(){ checkBits() ; return bin.readCStr() ; }
		char* getCurPtr(){ return bin.getCurPtr() ; }
		void setEndian( bool isBigEndian ) { bin.setEndian(isBigEndian) ; }
		void skip( int bytes ){ bin.skip( bytes ) ; }

		// 読み込みながらEndianを入れ替えていく場合にtrue
		// 一度読み込んだバッファを再度ロードする場合に二回目を同じEndianで扱えるようにする
		void setChangeEndian( bool change ) { bin.setChangeEndian( change ) ; }
	
	#if L2D_SAMPLE
		static void TEST_1() ;
		static void TEST_MODEL_LOAD() ;
	#endif
	
		
	private:
		void* read_array_ent( MemoryParam* memParam ,int targetClassNo, int restDimension) ;// 戻り値は、配列型 ( プリミティブもあるので、必ずしもObject[]ではない )
	
		void checkBits()
		{
			if( bitCount != 0 ) bitCount = 0 ;
		}
		
		int formatVersion ;
		
		ByteBuffer bin ;
		int bitCount ; //= 0 ;
		int bitBuff ;//= 0 ;// 現在のビットフラグを管理
	
		// TODO オブジェクトリストの管理を強化
		LDVector<void*> loadObjects ;// 生成されたオブジェクトは全て登録される。ロード結果なので中身は解放しない
		
	};
}
//------------------------- LIVE2D NAMESPACE -------------

#endif // __SKIP_DOC__

#endif		// __LIVE2D_B_READER_H__
