/**
 *  AMotion.h
 *
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */
#ifndef __LIVE2D_A_MOTION_H__
#define __LIVE2D_A_MOTION_H__


#include "../memory/LDObject.h"

#include "../ALive2DModel.h"

//--------- LIVE2D NAMESPACE ------------
namespace live2d 
{

	class MotionQueueEnt ;

	
	class AMotion : public live2d::LDObject 
	{
	public:
		AMotion();
		virtual ~AMotion() ;

	public:
		//  パラメータ更新
		void updateParam( live2d::ALive2DModel * model , MotionQueueEnt *motionQueueEnt ) ;

	public:

		
		void setFadeIn( int fadeInMsec ){ this->fadeInMsec = fadeInMsec ; }

		
		void setFadeOut( int fadeOutMsec){ this->fadeOutMsec = fadeOutMsec ; }
		
		
		int getFadeOut(){ return this->fadeOutMsec ; }

		
		int getFadeIn(){ return this->fadeInMsec ; }

		
		void setWeight( float weight){ this->weight = weight ; }
		
		
		float getWeight(){ return this->weight ; }

		/*****************************************************************
		 * モーションの長さを返す（ループのときは -1） 
		 * ループではない場合は、オーバーライドする
		 * 
		 * 正の値の時は取得される時間で終了する
		 * -1のときは外部から停止命令が無い限り終わらない処理となる
		******************************************************************/
		virtual int getDurationMSec(){ return -1 ; }
		
		/*****************************************************************
		 * ループ１回分の長さを返す (0.9.00b8)
		 * 
		 * 主にモーションの処理内部で利用する
		 * 
		 * サブクラスLive2DMotionの場合は、mtnファイルで定義される一連のモーションの長さを返す
		 * ループしない場合は getDurationMSec()と同じ値を返す
		 * ループ一回分の長さが定義できない場合（プログラム的に動き続けるサブクラスなど）の場合は-1を返す
		******************************************************************/
		virtual int getLoopDurationMSec(){ return -1 ; }


		/*****************************************************************
		 * モーション再生の開始時刻を設定する
		 * 途中から再生する場合に使用する
		 *
		 * version 0.9.00より搭載
		******************************************************************/
		void setOffsetMSec( int offsetMsec ){ this->offsetMsec = offsetMsec ; }

		
		void reinit(){}

	protected:
		//  パラメータアップデート
		virtual void updateParamExe( live2d::ALive2DModel * model , long long timeMSec , float weight , MotionQueueEnt *motionQueueEnt ) = 0 ;

		//  イージングを取得
		float getEasing( float time , float totalTime , float accelerateTime ) ;

	protected:
		int 	fadeInMsec ;		// フェードインする場合。これが無いと、突然向きが変わったりする
		int 	fadeOutMsec ;		// フェードアウト時間
		float 	weight ;			// Modelに設定されている値と掛け合わせる度合い。１なら上書きする

		int 	offsetMsec ;		// 再生の開始時刻

	};

}
//--------- LIVE2D NAMESPACE ------------


#endif		// __LIVE2D_A_MOTION_H__
