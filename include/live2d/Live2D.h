/**
 *  Live2D.h
 *
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */

#ifndef __LIVE2D_H__
#define __LIVE2D_H__

//========================================================
//  Compile target
// 	（プロジェクトに定義しない場合#if内をコメントを外す)
//========================================================
#if !( defined(L2D_TARGET_PSP) \
		|| defined(L2D_TARGET_IPHONE)	   \
		|| defined(L2D_TARGET_IPHONE_ES2) 	\
		|| defined(L2D_TARGET_MAC_OSX) 		\
		|| defined(L2D_TARGET_ANDROID) 	\
		|| defined(L2D_TARGET_ANDROID_ES2) 	\
		|| defined(L2D_TARGET_QT) 			\
		|| defined(L2D_TARGET_D3D) 			\
		|| defined(L2D_TARGET_D3D11) 		\
		|| defined(L2D_TARGET_WIN_GL) 		\
		|| defined(L2D_TARGET_3DS)		  \
		|| defined(L2D_TARGET_PS3 ) 		\
		|| defined(L2D_TARGET_PS4 ) 		\
		|| defined(L2D_TARGET_VITA ) 		\
	)
		//#define L2D_TARGET_PSP
		//#define L2D_TARGET_IPHONE
		//#define L2D_TARGET_IPHONE_ES2
		//#define L2D_TARGET_MAC_OSX
		//#define L2D_TARGET_ANDROID
		//#define L2D_TARGET_ANDROID_ES2
		//#define L2D_TARGET_QT
		//#define L2D_TARGET_D3D
		//#define L2D_TARGET_D3D11
		//#define L2D_TARGET_WIN_GL
		//#define L2D_TARGET_3DS
		//#define L2D_TARGET_PS3
		//#define L2D_TARGET_PS4
		//#define L2D_TARGET_VITA
#endif

#if defined( L2D_TARGET_PSP )
#define __L2D_PLATFORM_STR__ ("PSP")
#elif defined( L2D_TARGET_IPHONE )
#define __L2D_PLATFORM_STR__ ("iPhone")
#elif defined( L2D_TARGET_IPHONE_ES2 )
#define __L2D_PLATFORM_STR__ ("iPhone ES2")
#elif defined( L2D_TARGET_MAC_OSX )
#define __L2D_PLATFORM_STR__ ("MacOSX")
#elif defined( L2D_TARGET_ANDROID )
#define __L2D_PLATFORM_STR__ ("Android")
#elif defined( L2D_TARGET_ANDROID_ES2 )
#define __L2D_PLATFORM_STR__ ("Android ES2")
#elif defined( L2D_TARGET_QT )
#define __L2D_PLATFORM_STR__ ("Qt")
#elif defined( L2D_TARGET_D3D )
#define __L2D_PLATFORM_STR__ ("DirectX9")
#elif defined( L2D_TARGET_D3D11 )
#define __L2D_PLATFORM_STR__ ("DirectX11")
#elif defined( L2D_TARGET_D3D_OPEN )
#define __L2D_PLATFORM_STR__ ("DirectX_OPEN")
#elif defined( L2D_TARGET_WIN_GL )
#define __L2D_PLATFORM_STR__ ("Windows OpenGL")
#elif defined( L2D_TARGET_3DS )
#define __L2D_PLATFORM_STR__ ("3DS")
#elif defined( L2D_TARGET_VITA )
#define __L2D_PLATFORM_STR__ ("Vita")
#elif defined( L2D_TARGET_PS3 )
#define __L2D_PLATFORM_STR__ ("PS3")
#elif defined( L2D_TARGET_PS4 )
#define __L2D_PLATFORM_STR__ ("PS4")
#endif


#if defined( L2D_TARGET_ANDROID ) || defined( L2D_TARGET_IPHONE )
#define L2D_GRAPHICS_OPENGL_ES1
#endif

#if defined( L2D_TARGET_ANDROID_ES2 ) || defined( L2D_TARGET_IPHONE_ES2 ) || defined( L2D_TARGET_WIN_GL )
#define L2D_GRAPHICS_OPENGL_ES2
#endif

//========================================================
//	Options
//========================================================
// Unity用に書き出すときは1にしないとテクスチャなどが崩れるかもしれない
#define L2D_BUILD_FOR_UNITY			0

//------------ Debug options ---------------
#define L2D_SAMPLE			0	// サンプルデータ、テストメソッドなどを利用する場合に 1
#define L2D_VERBOSE			1	// エラーメッセージなどをはきだす場合に 1

// デバッグ用フラグ。開発中のみ1
#if defined( L2D_TARGET_IPHONE ) || defined ( DEBUG )
#  define L2D_DEBUG			1
#elif defined( L2D_TARGET_3DS ) && defined ( NN_BUILD_DEBUG )
#  define L2D_DEBUG			1
#elif defined ( _DEBUG )
#  define L2D_DEBUG			1
#endif


#define L2D_TEMPORARY_DEBUG 0	// 一時的なテスト用

#define L2D_ALPHA_IMPL_LAYOUT 0 // モデルに拡大縮小を設定するテスト実装


//------------ Performance options ---------------
#define L2D_RANGE_CHECK		1	// 配列へのアクセス時に範囲チェックを行う場合は1（あまり頻度は高くない）
#define L2D_RANGE_CHECK_POINT	1	// 0..1区間に収まるべき点のチェックを行う場合は1　速度が低下する。
									// チェックをしないと配列に不正アクセスする可能性もある (70%->72%程度にアップする)
#define L2D_DEFORMER_EXTEND		1	// デフォーマを拡張する(SDK2.0)

#define L2D_FORCE_UPDATE	0	// パラメータの更新に関係なく、全てのオブジェクトをアップデートする場合に1にする

//------------ Texture options ------------
#if defined( L2D_TARGET_IPHONE ) || defined( L2D_TARGET_ANDROID )
#  define L2D_INVERT_TEXTURE 0
#elif defined( L2D_TARGET_IPHONE_ES2 ) || defined( L2D_TARGET_ANDROID_ES2 ) || defined( L2D_TARGET_WIN_GL)
	#if L2D_BUILD_FOR_UNITY
		#  define L2D_INVERT_TEXTURE 1
	#else
		#  define L2D_INVERT_TEXTURE 0
	#endif
#elif defined( L2D_TARGET_VITA ) || defined( L2D_TARGET_D3D11 ) || defined( L2D_TARGET_PS4 )
#  define L2D_INVERT_TEXTURE 0
#else
#  define L2D_INVERT_TEXTURE 1
#endif

//------------ Avatar options ------------
#define AVATAR_OPTION_A 1

//========================================================
//  Exception
//========================================================
#if defined( L2D_TARGET_PSP ) || defined ( L2D_TARGET_VITA ) || defined(L2D_TARGET_PS4 ) || defined(L2D_TARGET_3DS)	|| defined(L2D_TARGET_PS3 )
	#define L2D_THROW( c )	{}
#elif defined(L2D_TARGET_IPHONE_ES2)|| defined( L2D_TARGET_ANDROID_ES2 ) || defined( L2D_TARGET_ANDROID )
	#define L2D_THROW( c )	{}
#else
	#define L2D_THROW( c ) throw (c)
#endif

//========================================================
//  typedef
//========================================================
typedef float				l2d_uvmapf ;	// 通常 float 型のuv map値
typedef float				l2d_pointf ;	// 通常 float 型のpoint 値
typedef float				l2d_paramf ;	// 通常 float型のパラメータ値
typedef unsigned short		l2d_index ;		// インデックス配列の型
typedef signed short		l2d_order ;		// 描画順序の型(signed short)
typedef float				l2d_float ;		// 透明度など基本的な float 型

#if defined(L2D_TARGET_IPHONE_ES2) || defined(L2D_TARGET_MAC_OSX)
typedef unsigned long	   l2d_size_t ;
#elif defined(L2D_TARGET_PS4)
#include <stddef.h>
typedef  size_t	   l2d_size_t ;
#elif defined(L2D_TARGET_WIN_GL) || defined(L2D_TARGET_D3D) || defined(L2D_TARGET_D3D11)
typedef size_t	  l2d_size_t ;
#elif defined(L2D_TARGET_IPHONE)
#include <stdlib.h>
typedef size_t	  l2d_size_t ;
#else
typedef unsigned int		l2d_size_t ;	// size_t 互換, 64bitでは64bitにする（未対応）
#endif

// 基本型（LDxxx -> l2d_xxxに移行）
typedef bool				l2d_bool ;
typedef signed char			l2d_int8 ;
typedef unsigned char		l2d_uint8 ;

typedef signed short		l2d_int16 ;
typedef unsigned short		l2d_uint16 ;

typedef signed int			l2d_int32 ;
typedef unsigned int		l2d_uint32 ;

typedef signed long long	l2d_int64 ;
typedef unsigned long long	l2d_uint64 ;

// 旧基本型（互換性維持のため保持）
typedef signed char			LDint8 ;
typedef unsigned char		LDuint8 ;

typedef signed short		LDint16 ;
typedef unsigned short		LDuint16 ;

typedef signed int			LDint32 ;
typedef unsigned int		LDuint32 ;

typedef signed long long	LDint64 ;
typedef unsigned long long	LDuint64 ;


// ---------- 範囲外パラメータの扱い  ----------
#define L2D_OUTSIDE_PARAM_AVAILABLE 0	// パラメータが範囲外のとき描画するなら 1
//#define L2D_OUTSIDE_PARAM_BASE_AVAILABLE 1	// パラメータが範囲外のとき座標変換を有効にする 1



//========================================================
//	Compiler
//========================================================
#ifdef _MSC_VER
#pragma warning (disable : 4100)
#endif

#ifndef NULL
#  define NULL	0
#endif

//========================================================
//	Live2D
//========================================================
namespace live2d
{
	class LDAllocator;
}

#include "memory/UtMemory.h"


//--------- LIVE2D NAMESPACE ------------
namespace live2d
{
	
	class Live2D
	{
	public:
		//static const l2d_uint32 L2D_VERSION_NO 	= __L2D_VERSION_NO__ ;// ロードエラーを回避するよう修正

		//========================================================
		// 	エラー定数 Live2D::L2D_ERROR_XXX
		// 	Live2D.getError()で返される
		//========================================================
		//   0- エラー無し
		static const int L2D_NO_ERROR								= 0 ;

		// 1000- Live2D全般・初期化関連
		static const int L2D_ERROR_LIVE2D_INIT_FAILED				= 1000 ;
		static const int L2D_ERROR_FILE_LOAD_FAILED					= 1001 ;
		static const int L2D_ERROR_MEMORY_ERROR						= 1100 ;

		// 2000- モデル関連エラー
		static const int L2D_ERROR_MODEL_DATA_VERSION_MISMATCH		= 2000 ;	// このライブラリで未対応バージョンのデータ
		static const int L2D_ERROR_MODEL_DATA_EOF_ERROR				= 2001 ;	// 終端が不正

		// 3000- モーション関連


		static const int L2D_COLOR_BLEND_MODE_MULT			= 0;
		static const int L2D_COLOR_BLEND_MODE_ADD			= 1;
		static const int L2D_COLOR_BLEND_MODE_INTERPOLATE = 2;

		static int getClippingMaskBufferSize();
		static void setClippingMaskBufferSize(int size);


		// Live2Dのライブラリを初期化し利用可能な状態にします
		static void init( live2d::LDAllocator* allocator = NULL ) ;

		// Live2Dのライブラリをクローズし、全てのリソースを解放します
		// ただし、外部から設定された画像などのリソースは解放しません
		// 外部で適切に破棄する必要があります。
		static void dispose() ;

		// バージョン文字列取得
		static const char * getVersionStr() ;

		// バージョン取得
		static l2d_uint32 getVersionNo() ;

		// ライブラリビルド時の設定を確認する
		static l2d_bool getBuildOption_RANGE_CHECK_POINT() ;
		static l2d_bool getBuildOption_AVATAR_OPTION_A() ;

		// 頂点のダブルバッファの設定
		
		static void setVertexDoubleBufferEnabled(l2d_bool enabled)
		{
			flag_vertexDoubleBufferEnabled = enabled ;
		}


		
		static l2d_bool isVertexDoubleBufferEnabled()
		{
			return flag_vertexDoubleBufferEnabled ;
		}

		// エラー設定(内部処理)
		static void setError(l2d_uint32 errorNo) ;

		// エラーの取得
		static l2d_uint32 getError() ;

	#ifdef L2D_TARGET_PSP
		// PSPにおいてポリゴンの描画前に頂点配列をライトバックする場合に trueを設定(0.9.00b15)
		static void setPSPWritebackEnabled_alphaImpl(l2d_bool enabled)
		{
			flag_PSPWritebackEnabled_alphaImpl = enabled ;
		}

		// PSPにおいてポリゴンの描画前に頂点配列をライトバックする場合に true(0.9.00b15)
		// デフォルトは false
		static l2d_bool isPSPWritebackEnabled_alphaImpl()
		{
			return flag_PSPWritebackEnabled_alphaImpl ;
		}
	#endif


	private:
		Live2D(){}							// インスタンス化させない

		// Prevention of copy Constructor
		Live2D( const Live2D & ) ;
		Live2D& operator=( const Live2D & ) ;
		~Live2D(){}

	private:
		//--- field ---
		static l2d_bool flag_first_init ;
		static l2d_bool flag_vertexDoubleBufferEnabled ;// 0.9.00b14 DDTextureのポリゴンをダブルバッファする場合にtrue

	#ifdef L2D_TARGET_PSP
		static l2d_bool flag_PSPWritebackEnabled_alphaImpl ;// 0.9.00b15 PSPにおいてポリゴン頂点をライトバックする場合true
	#endif

		static l2d_uint32	lastError ;
		static l2d_int32	initCount ;	// Live2D::init()が複数回発生するのを防ぐ。Live2D::dispose()を行わずにinit()を複数回行った場合は無視する。

		static int clippingMaskBufferSize;
	};

}
//--------- LIVE2D NAMESPACE ------------


#endif		// __LIVE2D_H__
