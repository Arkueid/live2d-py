/**
 *  DrawProfileCocos2D.h
 *
 *  Created by Live2D on 2013/08/02.
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */

#ifndef __Live2D__DrawProfileCocos2D__
#define __Live2D__DrawProfileCocos2D__
#include "../Live2D.h"

#if defined(L2D_TARGET_IPHONE_ES2) || defined(L2D_TARGET_ANDROID_ES2) || defined(L2D_TARGET_WIN_GL)
//------------ LIVE2D NAMESPACE ------------
namespace live2d
{
	
	class DrawProfileCocos2D {
	public:
		static void preDraw ( );
		static void postDraw ( );
	};
	
}
//------------ LIVE2D NAMESPACE ------------

#endif //L2D_TARGET_IPHONE_ES2


#endif /* defined(__Live2D__DrawProfileCocos2D__) */
