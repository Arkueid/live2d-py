/**
 *  FileFormat2.h
 *
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */
#include "../io/BReader.h"

#ifndef __LIVE2D_FILE_FORMAT2_H__
#define __LIVE2D_FILE_FORMAT2_H__


#ifndef __SKIP_DOC__

#include "../memory/LDObject.h"

//--------- LIVE2D NAMESPACE ------------
namespace live2d
{ 
	class ClassDef ;
	
	
	class FileFormat2 : public live2d::LDObject 
	{
	public:
		//  新しいバージョンを加えた場合は、必ず最新版にも設定する
		static const int LIVE2D_FORMAT_VERSION_V2_6_INTIAL 		= 6;	// 最初のバージョン
		static const int LIVE2D_FORMAT_VERSION_V2_7_OPACITY 	= 7;	// 半透明に対応
		static const int LIVE2D_FORMAT_VERSION_V2_8_TEX_OPTION 	= 8;	// テクスチャにオプションを付ける
		static const int LIVE2D_FORMAT_VERSION_V2_9_AVATAR_PARTS= 9;	// アバターのパーツを処理する
		static const int LIVE2D_FORMAT_VERSION_V2_10_SDK2	   = 10;	//SDK2.0
		static const int LIVE2D_FORMAT_VERSION_V2_11_SDK2_1 	= 11;	//SDK2.1

		// ---- 対応可能な最新版のファイル ---- Live2D#BUILD_NO 1000
#if defined( L2D_TARGET_PS4 ) || defined( L2D_TARGET_D3D ) || defined( L2D_TARGET_D3D11 ) || defined( L2D_TARGET_IPHONE_ES2 ) || defined( L2D_TARGET_ANDROID_ES2 ) || defined( L2D_TARGET_WIN_GL )
		static const int LIVE2D_FORMAT_VERSION_AVAILABLE	 	= LIVE2D_FORMAT_VERSION_V2_11_SDK2_1 ;
#else
		static const int LIVE2D_FORMAT_VERSION_AVAILABLE	 	= LIVE2D_FORMAT_VERSION_V2_10_SDK2 ;
#endif
	
		// --- EOF のフラグ
		static const int LIVE2D_FORMAT_EOF_VALUE = 0x88888888;			// 最初のバージョン
	
		// このバージョンで、クラスのロード方法を使い分ける(Live2DFormat.LIVE2D_FORMAT_VERSIONもある)
	//	static const int FILE_FORMAT_VERSION = 0 ;
		static const int NULL_NO	= 0;		// 
		static const int ARRAY_NO	= 23;		// 
		static const int OBJECT_REF = 33;		// 読み込み済みオブジェクトの参照
	
	public:
		FileFormat2();
		virtual ~FileFormat2();
		
	public:
		//  指定したクラス取得
		static ClassDef * getClass( int classNo ) ;
		
		static void * newInstance(MemoryParam* memParam , int classNo) ;

		static bool isSerializable(int classNo) ;

		//  指定のクラス番号がプリミティブ型ならtrue
		static bool isPrimitive(int classNo){ return ( 2 <= classNo && classNo <= 9 ) ; }
	
		
		static bool isPrimitiveDouble(int classNo){ return classNo == 30 ; } // 決め打ち(変更不可)

		
		static bool isPrimitiveFloat(int classNo){ return classNo == 31 ; }// 決め打ち(変更不可)

		
		static bool isPrimitiveInt(int classNo){ return classNo == 29 ; }// 決め打ち(変更不可)
	
	};
} 
//------------------------- LIVE2D NAMESPACE ------------

#endif // __SKIP_DOC__

#endif		// __LIVE2D_FILE_FORMAT2_H__
