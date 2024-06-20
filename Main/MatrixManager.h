/**
 *
 *  You can modify and use this source freely
 *  only for the development of application related Live2D.
 *
 *  (c) Live2D Inc. All rights reserved.
 */
#pragma once

#include "L2DViewMatrix.h"
#include "L2DTargetPoint.h"

#include "LAppModel.h"

class MatrixManager
{
private:
	live2d::framework::L2DViewMatrix				viewMatrix;
	live2d::framework::L2DMatrix44 				deviceToScreen;

public:
	MatrixManager();
	~MatrixManager();

	void init();

	float transformDeviceToViewX(float deviceX);
	float transformDeviceToViewY(float deviceY);

	void setDeviceSize( int width , int height ) ;
};




