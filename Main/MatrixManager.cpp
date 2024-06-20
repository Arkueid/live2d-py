/**
 *
 *  You can modify and use this source freely
 *  only for the development of application related Live2D.
 *
 *  (c) Live2D Inc. All rights reserved.
 */
#include "MatrixManager.h"

#include "LAppDefine.h"
#include "Log.hpp"


MatrixManager::MatrixManager()
{
}


MatrixManager::~MatrixManager()
{
}

void MatrixManager::init()
{
	viewMatrix.identity();
	viewMatrix.setMaxScale(VIEW_MAX_SCALE);
	viewMatrix.setMinScale(VIEW_MIN_SCALE);


	viewMatrix.setMaxScreenRect(
		VIEW_LOGICAL_MAX_LEFT,
		VIEW_LOGICAL_MAX_RIGHT,
		VIEW_LOGICAL_MAX_BOTTOM,
		VIEW_LOGICAL_MAX_TOP
	);
}

void MatrixManager::setDeviceSize( int width , int height )
{
	Info("set scene size : %d , %d" , width , height ) ;

	float ratio=(float)height/width;
	float left = VIEW_LOGICAL_LEFT;
	float right = VIEW_LOGICAL_RIGHT;
	float bottom = -ratio;
	float top = ratio;

	float screenW=abs(left-right);
	viewMatrix.setScreenRect(left, right, bottom, top);

	deviceToScreen.identity() ;
	deviceToScreen.multTranslate(-width/2.0f,-height/2.0f );
	deviceToScreen.multScale( screenW/width , -screenW/width );
}

float MatrixManager::transformDeviceToViewX(float deviceX)
{
	float screenX = deviceToScreen.transformX( deviceX );	
	return  viewMatrix.invertTransformX(screenX);			
}


float MatrixManager::transformDeviceToViewY(float deviceY)
{
	float screenY = deviceToScreen.transformY( deviceY );	
	return  viewMatrix.invertTransformY(screenY);			
}

