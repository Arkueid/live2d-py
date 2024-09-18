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

#include <functional>

struct Parameter
{
	std::string id;
	float value;
	float maxValue;
	float minValue;
	float defaultValue;
};

using OnMotionStartCallback = std::function<void(const char*, int)>;
using OnMotionFinishCallback = std::function<void(void)>;

class LAppModel : public live2d::framework::L2DBaseModel
{
public:

	LAppModel();
	~LAppModel(void);

	void load(const char *path);

	void update();
	void draw();
	void resize(int w, int h);
	void setCenter(float x, float y);
	void setScale(float scale);

	int startMotion(const char name[], int no, int priority, OnMotionStartCallback s_call = NULL, OnMotionFinishCallback f_call = NULL);
	int startRandomMotion(const char name[], int priority, OnMotionStartCallback s_call = NULL, OnMotionFinishCallback f_call = NULL);

	void setExpression(const char name[]);
	void setRandomExpression();

	void preloadMotionGroup(const char name[]);

	std::string hitTest(float testX, float testY);

	bool isMotionFinished();

	void setEyeBlinkEnable(bool enable);

	void setParameterValue(const char* paramId, float value, float weight);
	
	void addParameterValue(const char* paramId, float value);

	int getParameterCount();

	Parameter getParameter(int index);


private:
	ModelSetting *modelSetting;
	std::string modelHomeDir;

	live2d::framework::L2DMatrix44 _projection;

	OnMotionFinishCallback currentFinishCallback;

	float _offx;
	float _offy;
	float _scale;
	int _ww;
	int _wh;
	bool _eyeBlink;
};
