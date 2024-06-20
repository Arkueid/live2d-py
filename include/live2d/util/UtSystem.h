/**
 *  UtSystem.h
 *
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */

#ifndef __LIVE2D_UTSYSTEM_H__
#define __LIVE2D_UTSYSTEM_H__

#include "../Live2D.h"

//--------- LIVE2D NAMESPACE ------------
namespace live2d
{ 
	
	
	
	class UtSystem 
	{
	public:
		// ビッグエンディアンかどうか
		static bool isBigEndian() ;
	
		// システム時刻（ミリ秒）の取得
		static l2d_int64 getTimeMSec() ;
	
		//-------------------------------------------------------------
		// ユーザ定義時刻 ( 0.8.15以降 )
		// モーションなどをシステム時刻で行うと、計算処理の途中でシステム時刻が進み
		// 違う時刻で動きを計算する可能性がある。
		// ユーザ時刻では、描画の開始前に明示的に時刻を設定し、描画開始から描画終了まで
		// 同じ「ユーザ時刻」で処理が完了するように同期する。
		// 一度もユーザ時刻を設定していない場合は、getTimeMSec()と同じ値が返る
		//-------------------------------------------------------------
		// ユーザ定義の時刻を取得
		static l2d_int64 getUserTimeMSec() ;
	
		// ユーザ定義時刻を設定
		static void setUserTimeMSec(l2d_int64 t) ;
		
		// ユーザ定義時刻に、現在時刻をセット.
		// setUserTimeMSec( getTimeMSec() )と同じ効果
		static l2d_int64 updateUserTimeMSec() ;
	

		// ユーザ定義時刻をリセット
		static void resetUserTimeMSec();
	

		static void exit(int code) ;
		
	private:
		UtSystem();		
		
		// Prevention of copy Constructor							
		UtSystem( const UtSystem & ) ;				
		UtSystem& operator=( const UtSystem & ) ; 	
		
		~UtSystem();
			
		
	private:
		static const int 	USER_TIME_AUTO = -1 ;	// UserタイムをSystem時刻に追従させる
		static long long 	userTimeMSec ;			//
		
	};
}
//------------------------- LIVE2D NAMESPACE ------------

#endif		// __LIVE2D_UTSYSTEM_H__
