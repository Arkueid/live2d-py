/**
 *  Live2DMotion.h
 *
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */
#ifndef __LIVE2D_MOTION_H__
#define __LIVE2D_MOTION_H__


#include "../Live2D.h"
#include "../memory/LDObject.h"
#include "../ALive2DModel.h"
#include "../type/LDVector.h"
#include "AMotion.h"

//--------- LIVE2D NAMESPACE ------------
namespace live2d
{

	class Motion ;
	class MotionQueueEnt ;
	class MemoryParam ;

	
	class Live2DMotion : public live2d::AMotion
	{
	public:
		// Constructor
		Live2DMotion();

		// Destructor
		virtual ~Live2DMotion();
		
	public:
		//  パラメータ更新
		virtual void updateParamExe( live2d::ALive2DModel * model , long long timeMSec , float weight , MotionQueueEnt *motionQueueEnt) ;

		//  ループ設定
		void setLoop( bool _loop ){ this->loop = _loop ; }

		//  ループするか?
		bool isLoop( ){ return this->loop ; }
		
		//  ループ時にフェードインが有効かどうかを設定する。デフォルトでは有効。
		void setLoopFadeIn( bool _loopFadeIn ){ this->loopFadeIn = _loopFadeIn ; }
		
		//  ループ時のフェードインが有効かどうかを取得
		bool isLoopFadeIn( ){ return this->loopFadeIn ; }
		
		//  モーションの長さを返す。ループの時は-1
		virtual int getDurationMSec() ;
		
		
		//  mtnファイルで定義されている一連のモーションの長さを返す
		virtual int getLoopDurationMSec()
		{
			return loopDurationMSec ; 
		}

		//  モーションデータをロード(2byte文字非対応)
		static Live2DMotion * loadMotion( const live2d::LDString & filepath ) ;
		//  モーションデータをロード(2byte文字非対応)
		static Live2DMotion * loadMotion( const void * buf , int bufSize ) ;
		
		void dump() ;
		

		//SDK2.0
		// パラメータごとのフェードの設定
		//void setParamFadeIn(const char* paramID,int value);
		//void setParamFadeOut(const char* paramID,int value);
		void setParamFadeIn(LDString paramID,int value);
		void setParamFadeOut(LDString paramID,int value);
		
		int getParamFadeIn(const char* paramID);
		int getParamFadeOut(const char* paramID);


	private:
		live2d::LDVector<Motion*>* 	motionsPtr ;			//  モーションリスト

		float 						srcFps ;				//  ロードしたファイルのFPS。記述が無ければデフォルト値15fpsとなる
		int 						maxLength ;				//  

		int 						loopDurationMSec ;		//  mtnファイルで定義される一連のモーションの長さ
		bool 						loop ;					//  ループするか?
		bool 						loopFadeIn;				//  ループ時にフェードインが有効かどうかのフラグ。初期値では有効。
		int 						objectNoForDebug ;		//  デバッグ用
		
		float 						lastWeight ;
		live2d::MemoryParam*		memoryManagement ;
		live2d::AMemoryHolder*		memoryHolderFixedMain ;
	};




#ifndef __SKIP_DOC__

	/***************************************************************************
	一つのパラメータについてのアクション定義
	***************************************************************************/
	class Motion : public live2d::LDObject
	{
	public:
		static const int MOTION_TYPE_PARAM = 0 ;
		static const int MOTION_TYPE_PARTS_VISIBLE = 1 ;
		static const int MOTION_TYPE_PARAM_FADEIN = 2 ;//SDK2.0
		static const int MOTION_TYPE_PARAM_FADEOUT = 3 ;//SDK2.0
		
		// 順序変更不可（ < で比較しているため ）
		static const int MOTION_TYPE_LAYOUT_X = 100 ;
		static const int MOTION_TYPE_LAYOUT_Y = 101 ;
		static const int MOTION_TYPE_LAYOUT_ANCHOR_X = 102 ;
		static const int MOTION_TYPE_LAYOUT_ANCHOR_Y = 103 ;
		static const int MOTION_TYPE_LAYOUT_SCALE_X = 104 ;
		static const int MOTION_TYPE_LAYOUT_SCALE_Y = 105 ;
		
	public:
		Motion(live2d::MemoryParam* memParam) ;
		virtual ~Motion() ;
		
	public:
		int getParamIndex( live2d::ALive2DModel* model ) ;

	public:
		live2d::LDString * 			paramIDStr ;				//  パラメータID文字列
		
		live2d::ParamID* 			cached_paramID ;			//  複数のモデルでモーションを使いまわすとIndexのキャッシュができない
		int  						cached_paramIndex ;			//
		live2d::ALive2DModel* 		cached_model_ofParamIndex ;

		live2d::LDVector<float> 	values ;
		
		int 						motionType ;				//  モーションタイプ

		//SDK2.0
		int 	fadeInMsec ;		// このパラメータのフェードインする時間。無効値のときはモーション全体の設定を優先する
		int 	fadeOutMsec ;		
	};

#endif // __SKIP_DOC__

}
//--------- LIVE2D NAMESPACE ------------

#endif		// __LIVE2D_MOTION_H__
