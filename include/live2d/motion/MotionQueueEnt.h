/**
 *  MotionQueueEnt.h
 *
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */
#ifndef __LIVE2D_MOTION_QUEUE_ENT_H__
#define __LIVE2D_MOTION_QUEUE_ENT_H__


#ifndef __SKIP_DOC__

#include "../memory/LDObject.h"
#include "../ALive2DModel.h"

#include "../type/LDVector.h"


//--------- LIVE2D NAMESPACE ------------
namespace live2d
{
	class AMotion ;
	class Live2DMotion ;

	/********************************************************************************
	@brief	MotionQueueManagerで再生している各モーションの管理クラス
	
	MotionQueueManager* mqm = ... \n
	int motionQueueEntNo = mqm->startMotion(...)\n
	MotionQueueEnt ent = mqm->getMotionQueueEnt(motionQueueEntNo)
	********************************************************************************/
	class MotionQueueEnt : public live2d::LDObject 
	{
		// フレンド
		friend class MotionQueueManager ;
		friend class AMotion ;
		friend class Live2DMotion ;
		friend class Live2DMotionBin ;
	public:
		MotionQueueEnt();
		virtual ~MotionQueueEnt(void);

	public:
		//  フェードアウト開始
		void startFadeout(long long fadeOutMsec)  ;

		//  終了判定
		bool isFinished() { return finished ; }

		//  開始時間取得
		long long getStartTimeMSec(){ return startTimeMSec ; }

		//  フェードイン開始時間取得
		long long getFadeInStartTimeMSec(){ return fadeInStartTimeMSec ; }

		//  フェードイン終了時間取得
		long long getEndTimeMSec(){ return endTimeMSec ; }

		//  開始時間設定
		void setStartTimeMSec(long long t){ this->startTimeMSec = t ; }

		//  フェードイン開始時間設定
		void setFadeInStartTimeMSec(long long t){ this->fadeInStartTimeMSec = t ; }

		//  フェードイン終了時間設定
		void setEndTimeMSec(long long t){ this->endTimeMSec = t ; }

		//  終了判定設定
		void setFinished( bool f ){ this->finished = f ; }

		//  モーションが有効か返す
		bool isAvailable(){ return available ; }

		//  有効化を設定
		void setAvailable( bool v ){ this->available = v ; }

		//---- state for callback(Unity etc) ----
		//  状態設定
		void setState( long long time , float weight ){ this->state_time = time ; this->state_weight = weight ; }

		//  時間状態取得
		long long getState_time(){ return this->state_time ; }

		//  "重み"の状態取得
		float getState_weight(){ return this->state_weight ; }
		
	private:
		// パブリックな設定値
		bool			autoDelete;					//  自動削除
		AMotion* 		motion;						//  モーション

		bool 			available;					//  有効化フラグ
		bool 			finished ;					//  終了フラグ
		bool 			started ;					//  開始フラグ（0.9.00以降）
		long long 		startTimeMSec ;				//  開始時刻
		long long 		fadeInStartTimeMSec ;		//  フェードイン開始時刻（ループの時は初回のみ）

		// 終了予定時刻
		// 　-1の場合は終了予定時刻なし（未初期化、または　ループを意味する）
		// 	非loopの場合は endTimeMSec = ( startTimeMSec + 1回分の時間 )
		// 	別モーションの割込時 MotionQueueManagerから startFadeout()が呼ばれる
		//		newEndTimeMSec = (割込時刻+fadeOutMSec) ;
		//	if( endTimeMSec<0 /*ループ時*/ || newEndTimeMSec < endTimeMSec ){
		//			endTimeMSec = newEndTimeMSec 
		// 終了予定時刻を過ぎた場合はモーションが破棄される
		long long endTimeMSec ;

		// ---- インスタンスの識別番号
		static int static_motionQueueEntNo ;// インスタンスごとに一意の値を持つ識別番号の次の値
		int motionQueueEntNo ;// インスタンスごとに一意の値を持つ識別番号（new時に設定）(0.8.09追加）

	private:
		//---- state for callback(Unity etc) ----
		long long   	state_time ;				//  時刻の状態
		float	   	state_weight ;				//  重みの状態
	};

}
//--------- LIVE2D NAMESPACE ------------

#endif // __SKIP_DOC__

#endif	// __LIVE2D_MOTION_QUEUE_ENT_H__
