/**
* ClippingManagerOpenGL.h
*
*  Copyright(c) Live2D Inc. All rights reserved.
*  [[ CONFIDENTIAL ]]
*/
#ifndef __LIVE2D_CLIPPINGMANAGEROPENGL_H__
#define __LIVE2D_CLIPPINGMANAGEROPENGL_H__

#ifndef __SKIP_DOC__


#if defined(L2D_TARGET_WIN_GL) || defined(L2D_TARGET_ANDROID_ES2) || defined(L2D_TARGET_IPHONE_ES2)


#ifdef L2D_TARGET_ANDROID_ES2
#include <jni.h>
#include <errno.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif

#ifdef L2D_TARGET_IPHONE_ES2
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#endif

#ifdef L2D_TARGET_WIN_GL
#include <windows.h>
#include <GL/GL.h>
#endif

#include "../Live2D.h"
#include "../type/LDRectF.h"
#include "../type/LDPointF.h"
#include "../type/LDVector.h"
#include "../type/LDMatrix44.h"
#include "../type/LDMap.h"
#include "../memory/MemoryParam.h"


namespace live2d
{
	class ModelContext;
	class IDrawData;
	class IDrawContext;
	class DrawDataID;

	class DrawParam_OpenGLES2;
	class ClipContext;
	class TextureInfo;
	class ClippedDrawContext;
	class RenderTextureRes;


	class FrameBufferInfo
	{
	public:
		FrameBufferInfo();
		~FrameBufferInfo();

		int size;
		GLuint colorBuffer;
	};


	class ClippingManagerOpenGL : public LDObject
	{
	public:
		ClippingManagerOpenGL(MemoryParam *memParam);
		~ClippingManagerOpenGL(void);

		static TextureInfo *getChannelFlagAsColor(int channelNo);

		void init(MemoryParam *memParam, ModelContext &mc, LDVector<IDrawData*> *drawDataList, LDVector<IDrawContext*> *drawContextList);

		static GLuint getMaskRenderTexture();

		void setupClip(ModelContext &mc, DrawParam_OpenGLES2 *dp2);

		// リリース用メソッド（Live2D::diposeから呼ばれる)
		static void releaseStored_notForClientCall() ;

		static void releaseShader();

	public:
		static const int CHANNEL_COUNT = 4;// 実験時に1チャンネルの場合は1、RGBだけの場合は3、アルファも含める場合は4

		static const bool RENDER_TEXTURE_USE_MIPMAP = false;

		GLuint maskRenderTexture;

		LDVector<ClipContext*>* clipContextList ;

		GLuint getColorBuffer();


	private:
		ClipContext* findSameClip(LDVector<DrawDataID*> *clips);
		static void calcClippedDrawTotalBounds(ModelContext &mc, ClipContext *cc);

		void setupLayoutBounds(int usingClipCount);

		static void releaseUnusedRenderTextures();

		void initClipping();

	private:
		static LDMatrix44 *tmpModelToViewMatrix;
		static LDMatrix44 *tmpMatrix2;
		static LDMatrix44 *tmpMatrixForMask;
		static LDMatrix44 *tmpMatrixForDraw;

		static LDRectF *tmpBoundsOnModel;

		static int lastFrameCount ;
		static int totalTexCount ;
		static const int NOT_USED_FRAME = -100;

		static bool firstError_clipInNotUpdate ;

		static int curFrameNo;

		static LDVector<RenderTextureRes*> *maskTextures;

		static LDVector<TextureInfo*>* CHANNEL_COLORS;

		static GLuint colorBuffer;

	};

	class ClipContext : public LDObject
	{
	public:
		ClipContext(MemoryParam *memParam, ClippingManagerOpenGL *owner , ModelContext &mc , LDVector<DrawDataID*> *clips);
		ClipContext();
		~ClipContext(void);

		void addClippedDrawData(MemoryParam *memParam, DrawDataID *id, int drawDataIndex);
		ClippingManagerOpenGL* getClippingManager();

	public:
		// モデルのinit時
		LDVector<DrawDataID*> *clipIDList;	//初期化時のみ使う（使わない実装も可能）
		LDVector<int> *clippingMaskDrawIndexList;	//マスクを構成する描画オブジェクト群

		LDVector<ClippedDrawContext*> *clippedDrawContextList;


		// モデルの描画前処理時
		bool isUsing;// 現在の描画状態でマスクの準備が必要ならtrue

		// レイアウト
		int layoutChannelNo;// RGBAのいずれのチャンネルにこのクリップを配置するか(0:R , 1:G , 2:B , 3:A)
		LDRectF *layoutBounds;// マスク用チャンネルのどの領域にマスクを入れるか(View座標-1..1 => UVは0..1に直す)

		// このクリッピングで、クリッピングされる全ての描画オブジェクトの囲み矩形（毎回更新）
		LDRectF *allClippedDrawRect;

		float matrixForMask[16];
		float matrixForDraw[16];//

	private:
		ClippingManagerOpenGL *owner;
	};



	class ClippedDrawContext : public LDObject
	{
	public:
		ClippedDrawContext(DrawDataID *id, int drawDataIndex);
		~ClippedDrawContext(void);

	public:
		DrawDataID *drawDataID;
		int drawDataIndex;
	};


	class RenderTextureRes : public LDObject
	{
	public:
		GLuint tex ;
		int frameNo;

	public:
		RenderTextureRes(int frameNo, GLuint tex);
		~RenderTextureRes(void){};
	};
}


#endif		// __SKIP_DOC__

#endif		// L2D_TARGET_WIN_GL || L2D_TARGET_ANDROID_ES2 || L2D_TARGET_IPHONE_ES2

#endif		// __LIVE2D_CLIPPINGMANAGEROPENGL_H__
