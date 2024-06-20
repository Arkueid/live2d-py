/**
 *  EyeBlinkMotion.h
 *
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */
#ifndef __LIVE2D_EYE_BLINK_MOTION_H__
#define __LIVE2D_EYE_BLINK_MOTION_H__


#include "../memory/LDObject.h"

#include "../type/LDVector.h"

#include "../ALive2DModel.h"


//--------- LIVE2D NAMESPACE ------------
namespace live2d 
{

	
	class EyeBlinkMotion : public live2d::LDObject 
	{

	public:
		// 眼の状態定数
		enum EYE_STATE{
			STATE_FIRST = 0 , 
			STATE_INTERVAL ,
			STATE_CLOSING ,// 閉じていく途中
			STATE_CLOSED , // 閉じている状態
			STATE_OPENING ,// 開いていく途中
		};

	public:
		// Constructor
		EyeBlinkMotion();

		// Destructor
		virtual ~EyeBlinkMotion();

	public:
		//  次回のまばたきモーションの時刻を設定
		long long calcNextBlink() ;

		//  インターバル時間の設定
		void setInterval( int blinkIntervalMsec) ;

		//  まばたきモーションの設定
		void setEyeMotion( int closingMotionMsec , int closedMotionMsec , int openingMotionMsec ) ;

		//  指定したモデルのパラメータ設定
		void setParam( live2d::ALive2DModel *model ) ;

	private:
		long long 			nextBlinkTime ;				// 次回眼パチする時刻（msec）
		
		int 				eyeState ;					// 現在の状態
		long long 			stateStartTime ;			// 現在のstateが開始した時刻

		bool 				closeIfZero;				// IDで指定された眼のパラメータが、0のときに閉じるなら true 、1の時に閉じるなら false
		
		live2d::LDString 	eyeID_L ;					// 左目のID
		live2d::LDString 	eyeID_R ;					// 右目のID

		int					blinkIntervalMsec ;			// 
		int 				closingMotionMsec ;			// 眼が閉じるまでの時間
		int 				closedMotionMsec  ;			// 閉じたままでいる時間
		int 				openingMotionMsec ;			// 眼が開くまでの時間

	};

}
//--------- LIVE2D NAMESPACE ------------


#endif		// __LIVE2D_EYE_BLINK_MOTION_H__
