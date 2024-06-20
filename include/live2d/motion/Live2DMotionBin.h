/**
 *  Live2DMotionBin.h
 *
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */

#pragma once


#include "../Live2D.h"
#include "../ALive2DModel.h"
#include "../type/LDVector.h"
#include <vector>
#include "AMotion.h"


//--------- LIVE2D NAMESPACE ------------
namespace live2d
{
	class MotionBin ;
	class MotionQueueEnt ;



	
	class Live2DMotionBin : public AMotion{
	public:

		// loadMotionでバイナリを渡した場合に、第３引数をどのようにするかを指定する
		typedef enum {
			// デストラクト時に元データを破棄しない
			DO_NOTHING_ON_DESTRUCT ,		// 外部で破棄
			DUPLICATE_AND_DESTRUCT ,		// 渡したデータを複製して使う（渡したデータには何もしない）

			// デストラクト時に指定の方式で破棄する
			FREE_ON_DESTRUCT ,				// free()で破棄
			DELETE_ARRAY_ON_DESTRUCT , 		// delete[]で破棄
			DELETE_NORMAL_ON_DESTRUCT  , 	// 通常のdelete で破棄
			
			L2D_FREE_ON_DESTRUCT ,			// Live2D独自のメモリ管理方式で破棄
			UTFILE_RELEASE_ON_DESTRUCT		// UtFile::releaseLoadBuffer()で破棄
		} BufType ;



		Live2DMotionBin();
		virtual ~Live2DMotionBin();
		
		virtual void updateParamExe( live2d::ALive2DModel * model , long long timeMSec , float weight , MotionQueueEnt *motionQueueEnt) ;


		void setLoop( bool _loop ){ this->loop = _loop ; }
		bool isLoop( ){ return this->loop ; }
		
		/**
		 * モーションの長さを返す（ループのときは -1）
		 * -1のときは終わらない処理となる
		 */
		virtual int getDurationMSec() ;
		
		
		/** 
		 * mtnファイルで定義される一連のモーションの長さ（＝ループ１回分の長さ）を返す (0.9.00b8)
		 */
		virtual int getLoopDurationMSec(){ 
			return loopDurationMSec ; // mtnファイルで定義される一連のモーションの長さ
		}

		/*
		 * モーションデータをロードする
		 * 
		 * ２バイト文字は未対応
		 */
		static Live2DMotionBin * loadMotion( const l2d_string & filepath ) ;

		/*
		 * モーションデータをバイト列からロードする
		 *
		 * bufType  bufのポインタをどのように破棄するかポインタ確保の方式にあわせて以下から指定する。
		 *
		 *   BufType::DO_NOTHING_ON_DESTRUCT ,		// デストラクト時に破棄しない（外部で破棄）
		 *	 BufType::DUPLICATE_AND_DESTRUCT ,		// 渡したデータを複製して使う（渡したデータには何もしない）
		 *	 BufType::FREE_ON_DESTRUCT ,			// デストラクト時にfree()で破棄
		 *	 BufType::DELETE_ARRAY_ON_DESTRUCT , 	// デストラクト時にdelete[]で破棄
		 *	 BufType::DELETE_NORMAL_ON_DESTRUCT  , // デストラクト時に通常のdelete で破棄
		 *	 BufType::UTFILE_RELEASE_ON_DESTRUCT	// デストラクト時にUtFile::releaseLoadBuffer()で破棄
		 *
		 * deleteBufOnDestructor bufをLive2DMotionBinのDestructorで破棄する場合にtrue
		 */
		static Live2DMotionBin * loadMotion( 
						const void * buf ,					// モーションファイルをロードしたポインタ
						int bufSize ,						// モーションのデータサイズ(バイト）
						Live2DMotionBin::BufType  bufType	// bufの使い方、破棄の仕方を指定する
					) ;
		


		void dump() ;
		


	private:
		const void *				buf ;// ロードされたバイナリ。deleteBufOnDestructorがtrueの場合に破棄

		Live2DMotionBin::BufType	bufType ;// bufをどのように扱うか（enum BufType で指定する）
	//	bool						deleteBufOnDestructor ;// Destructorでポインタを破棄する場合にtrue

		//---- 
		MotionBin**					motionPtrArray ;
		int							paramCount ;//count of MotionBin

		// ---- ロードデータ
		float 						srcFps ;// ロードしたファイルのFPS。記述が無ければデフォルト値15fpsとなる
		int 						maxLength ;

		int 						loopDurationMSec ;// mtnファイルで定義される一連のモーションの長さ
		bool 						loop ;
		int 						objectNoForDebug ;
		
		float 						lastWeight ;
		live2d::MemoryParam*		memoryManagement ;
		live2d::AMemoryHolder*		memoryHolderFixedMain ;

	};




#ifndef __SKIP_DOC__

	//=================================================
	//  	一つのパラメータについてのアクションを定義するクラス
	//=================================================
	class MotionBin : public live2d::LDObject
	{
	public:
		static const int MOTION_TYPE_PARAM = 0 ;
		static const int MOTION_TYPE_PARTS_VISIBLE = 1 ;
		
		// 順序変更不可（ < で比較しているため ）
		static const int MOTION_TYPE_LAYOUT_X = 100 ;
		static const int MOTION_TYPE_LAYOUT_Y = 101 ;
		static const int MOTION_TYPE_LAYOUT_ANCHOR_X = 102 ;
		static const int MOTION_TYPE_LAYOUT_ANCHOR_Y = 103 ;
		static const int MOTION_TYPE_LAYOUT_SCALE_X = 104 ;
		static const int MOTION_TYPE_LAYOUT_SCALE_Y = 105 ;
		
		MotionBin(void);
		virtual ~MotionBin(void);
		
		int getParamIndex( live2d::ALive2DModel* model ) ;

	public:
		//--- fields ---
		l2d_string * 			paramIDStr ;

		// 複数のモデルでモーションを使いまわすとIndexのキャッシュができない
		live2d::ParamID* 		cached_paramID ;
		int  					cached_paramIndex ;//
		live2d::ALive2DModel* 	cached_model_ofParamIndex ;

	//	l2d_vector<float> 		values ;

		// データの持ち方を、float[] short[]型の二種類使い分ける
		// 通常はデータサイズを小さくするため min,max間をshort(65536段階16bit)に分解する
		// 値のレンジが広い場合（ min,max = -1000,1000のような場合）は
		// より精度の高いfloat[](仮数部23bit)を用いて直接値を設定する

		// min,maxの間でshort値を設定する場合は true
		bool					isShortArray ;

		void*					valuePtr ;// float[]またはshort[]を指す。ファイルのバイナリを直接参照するので破棄しない
		float					minValue ;
		float					maxValue ;
		
		int						valueCount ;
		
		int 					motionType ;
	};

#endif // __SKIP_DOC__

}
//--------- LIVE2D NAMESPACE ------------
