/**
 *
 *  You can modify and use this source freely
 *  only for the development of application related Live2D.
 *
 *  (c) Live2D Inc. All rights reserved.
 */

#pragma once

//Live2D Framework
#include "L2DPhysics.h"
#include "L2DPose.h"
#include "L2DModelMatrix.h"
#include "L2DMotionManager.h"
#include "L2DEyeBlink.h"
#include "L2DMotionManager.h"
#include "L2DExpressionMotion.h"
#include "L2DTargetPoint.h"
#include "L2DTextureDesc.h"

//Live2D Lib
#include <util/UtSystem.h>
#include <motion/Live2DMotion.h>
#include <motion/MotionQueueManager.h>

#include <string>
#include <map>

namespace live2d
{
	namespace framework
	{
		class L2DBaseModel
		{
		protected:
			live2d::ALive2DModel* live2DModel;
			std::vector<L2DTextureDesc*> textures;
			L2DMotionManager*	mainMotionMgr;
			L2DMotionManager*	expressionMgr;
			L2DEyeBlink*			eyeBlink;
			L2DModelMatrix*		modelMatrix;
			L2DPhysics*			physics;
			L2DPose*				pose;
			L2DTargetPoint* dragMgr;

			std::map< std::string, AMotion* >		motions;
			std::map< std::string, AMotion*>		expressions;
			
			l2d_int64	startTimeMSec ;
			
			bool		initialized;
			bool		updating;
			float	alpha;
			float	accAlpha;
			bool		lipSync;			
			float	lastLipSyncValue;	
			float	dragX;
			float	dragY;
			float	accelX;
			float	accelY;
			float	accelZ;
			bool	debugMode;

		public:
			L2DBaseModel();
			virtual ~L2DBaseModel(void);
			
			virtual void setDrag(float x,float y);
			virtual void setAccel(float x,float y,float z);
			
			virtual void releaseTextures();
			virtual void releaseMotions();
			virtual void releaseExpressions();
			
			L2DModelMatrix* getModelMatrix(){return modelMatrix;}
			
			
			virtual bool isInitialized() { return initialized;  }
			virtual void setInitialized(bool v) { initialized=v;  }
			
			
			virtual bool isUpdating() { return updating; }
			virtual void setUpdating(bool v) { updating=v; }
			
			virtual void setAlpha(float a){alpha=a;}
			virtual float getAlpha(){return alpha;}

			virtual void loadModelData(const char* path);
			virtual void loadTexture(int no,const char* path);
			virtual AMotion* loadMotion(const char* name,const char* path);
			virtual void loadExpression(const char* name,const char* path);
			virtual void loadPose( const char* path );
			virtual void loadPhysics( const char* path );

			virtual bool hitTestSimple(const char drawID[],float testX,float testY);

			live2d::ALive2DModel* getLive2DModel(){return live2DModel;}
		};
	}
}