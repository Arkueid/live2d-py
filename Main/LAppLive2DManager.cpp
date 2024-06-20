/**
 *
 *  You can modify and use this source freely
 *  only for the development of application related Live2D.
 *
 *  (c) Live2D Inc. All rights reserved.
 */
#include "LAppLive2DManager.h"

#include "L2DViewMatrix.h"

//Live2DApplication
#include "LAppModel.h"
#include "LAppDefine.h"
#include "LAppModel.h"
#include "L2DMotionManager.h"

#include "PlatformManager.h"

#include "Live2DFramework.h"

using namespace live2d;
using namespace live2d::framework;


LAppLive2DManager::LAppLive2DManager()
	:currentModel(nullptr)
{
	live2d::Live2D::init( &myAllocator );
	Live2DFramework::setPlatformManager(new PlatformManager());
}


LAppLive2DManager::~LAppLive2DManager() 
{
	releaseModel();
	Live2D::dispose();
}

LAppModel* LAppLive2DManager::current()
{
	return currentModel;
}


void LAppLive2DManager::loadModel(const char* path)
{
	currentModel = new LAppModel();
	currentModel->load(path);
}

void LAppLive2DManager::releaseModel()
{
	delete currentModel;
}


void LAppLive2DManager::setDrag(float x, float y)
{
	currentModel->setDrag(x, y);
}

bool LAppLive2DManager::tapEvent(float x,float y)
{
	if(LAppDefine::DEBUG_LOG) UtDebug::print( "tapEvent\n");
	

	if(currentModel->hitTest(HIT_AREA_HEAD, x, y))
	{
			
		if(LAppDefine::DEBUG_LOG)UtDebug::print( "face\n");
		currentModel->setRandomExpression();
	}
	else if(currentModel->hitTest( HIT_AREA_BODY,x, y))
	{
		if(LAppDefine::DEBUG_LOG)UtDebug::print( "body\n");
		currentModel->startRandomMotion(MOTION_GROUP_TAP_BODY, PRIORITY_NORMAL );
	}
	    
    return true;
}
