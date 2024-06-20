/*
 *  DEF.h
 * 
 *  ライブラリ内で利用する定数などを定義
 * 
 * 
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */


#ifndef __LIVE2D_DEF_H__
#define __LIVE2D_DEF_H__


#ifndef __SKIP_DOC__

#include "Live2D.h"

//------------ LIVE2D NAMESPACE ------------
namespace live2d
{ 
	// 定数定義クラス
	class DEF 
	{
	public:
		static const int VERTEX_TYPE_OFFSET0_STEP2 = 1 ;		// 
		static const int VERTEX_TYPE_OFFSET2_STEP5 = 2 ;		// 
		static const int VERTEX_TYPE_OFFSET0_STEP5 = 3 ;		// 
			
			
	// コンパイルターゲットごとに描画用配列のオフセットを変える

#if defined( L2D_TARGET_PSP	) // (texture x,y , vertex x,y,z )と並ぶ
		// 実行環境に依存するため定数
		static const int VERTEX_OFFSET = 2 ;							// texture x,y の後に並ぶ
		static const int VERTEX_STEP = 5 ;
		static const int VERTEX_TYPE = VERTEX_TYPE_OFFSET2_STEP5 ;
#elif defined( L2D_TARGET_D3D )
		static const int VERTEX_OFFSET = 0 ;							// texture x,y の後に並ぶ
		static const int VERTEX_STEP = 5 ;
		static const int VERTEX_TYPE = VERTEX_TYPE_OFFSET0_STEP5 ;
#elif defined( L2D_TARGET_VITA )
		static const int VERTEX_OFFSET = 0 ;							// x,y,z , u,v
		static const int VERTEX_STEP = 5 ;
		static const int VERTEX_TYPE = VERTEX_TYPE_OFFSET0_STEP5 ;
#elif defined( L2D_TARGET_PS3 )
		// 実行環境に依存するため定数
		static const int VERTEX_OFFSET = 0 ;							//  
		static const int VERTEX_STEP = 5 ;
		static const int VERTEX_TYPE = VERTEX_TYPE_OFFSET0_STEP5 ;
#else
		static const int VERTEX_OFFSET = 0 ;							// texture x,y の後に並ぶ
		static const int VERTEX_STEP = 2 ;
		static const int VERTEX_TYPE = VERTEX_TYPE_OFFSET0_STEP2 ;
#endif
	
		static const int MAX_INTERPOLATION = 5 ; 						// 最大の補間回数 2^(MAX..) で補間回数は増えるので 通常3 程度におさせる
		static const int PIVOT_TABLE_SIZE = 65 ; 						// 2^( 最大の補間の数 ) + 1
	
		static const float GOSA ;
	
	//	static bool FORCE_UPDATE ;
	
	private:
		DEF() ;			
		//Prevention of copy Constructor					
		DEF( const DEF & ) ;				
		DEF& operator=( const DEF & ) ; 
		~DEF() ;
		
	};

}
//------------ LIVE2D NAMESPACE ------------

#endif		//__SKIP_DOC__

#endif		// __LIVE2D_DEF_H__
