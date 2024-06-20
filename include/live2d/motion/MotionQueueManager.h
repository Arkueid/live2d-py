/**
 *  MotionQueueManager.h
 *
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */
#ifndef __LIVE2D_MOTION_QUEUE_MANAGER_H__
#define __LIVE2D_MOTION_QUEUE_MANAGER_H__


#include "../memory/LDObject.h"
#include "../ALive2DModel.h"

#include "../type/LDVector.h"
#include "AMotion.h"

//--------- LIVE2D NAMESPACE ------------
namespace live2d
{
	class Live2DMotion ;
	class MotionQueueEnt ;
	class MemoryParam ;


	
	class MotionQueueManager : public live2d::LDObject 
	{
	public:
		MotionQueueManager();
		virtual ~MotionQueueManager();
		
		// 指定のモーションを開始する
		// 同じタイプのモーションが既にある場合は、既存のモーションに終了フラグを立て、フェードアウトを開始させる。
		int startMotion( AMotion * motion , bool autoDelete ) ;
		
		// モデルのパラメータを設定、更新する（動きを反映する）
		bool updateParam( live2d::ALive2DModel * model ) ;

		//  全てのモーションが終了しているか
		bool isFinished() ;
		
		// 引数で指定したモーションが終了しているかを返す。(0.8.09追加）
		bool isFinished(int motionQueueEntNo ) ;


		//  全てのモーションを停止する
		void stopAllMotions() ;

		//  モーションの開始・終了をダンプする
		void setMotionDebugMode( bool f ){ this->motionDebugMode = f ; }

		//  MotionQueueEntを取得
		MotionQueueEnt* getMotionQueueEnt( int entNo ) ;
		
	#if L2D_VERBOSE
		void DUMP() ;
	#endif

	private:
		live2d::LDVector<MotionQueueEnt *> *motions ;

		bool motionDebugMode ;// モーションの再生・停止をダンプするフラグ

		live2d::MemoryParam*  memoryManagement ;
		live2d::AMemoryHolder*	memoryHolderFixedMain ;
	};

}
//--------- LIVE2D NAMESPACE ------------

#endif		// __LIVE2D_MOTION_QUEUE_MANAGER_H__
