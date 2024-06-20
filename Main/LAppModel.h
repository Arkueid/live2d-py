/**
 *
 *  You can modify and use this source freely
 *  only for the development of application related Live2D.
 *
 *  (c) Live2D Inc. All rights reserved.
 */
#pragma once

#include "L2DBaseModel.h"
#include <vector>
#include <string>

#include "ModelSetting.h"
#include "L2DViewMatrix.h"
#include "LAppWavFileHandler.hpp"


class LAppModel : public live2d::framework::L2DBaseModel
{
public:
	typedef void (*OnStartMotionCallback)(const char*, int);
	typedef void (*OnFinishMotionCallback)(void);

    LAppModel();
    ~LAppModel(void);
    
	void load(const char* path) ;

    void update();
    void draw();
	void resize(int w, int h);
	void setCenter(float x, float y);
	void setScale(float scale);
	
    int startMotion(const char name[],int no,int priority, OnStartMotionCallback s_call=NULL, OnFinishMotionCallback f_call=NULL);
	int startRandomMotion(const char name[],int priority, OnStartMotionCallback s_call = NULL, OnFinishMotionCallback f_call = NULL);
	
	void setExpression(const char name[]);
	void setRandomExpression();

	void setLipSyncN(float n);
	
	void preloadMotionGroup(const char name[]);
    
	std::string hitTest(float testX,float testY);

	bool isMotionFinished();

private:
	ModelSetting* modelSetting;
	std::string			modelHomeDir;

	OnFinishMotionCallback currentFinishCallback;

	live2d::framework::L2DMatrix44 _projection;
	LAppWavFileHandler _wavHandler;

	float _offx;
	float _offy;
	float _scale;
	int _ww;
	int _wh;
	float _lipSyncN;
};






