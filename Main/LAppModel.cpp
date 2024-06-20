/**
 *
 *  You can modify and use this source freely
 *  only for the development of application related Live2D.
 *
 *  (c) Live2D Inc. All rights reserved.
 */

#include <string>


//Live2D Application
#include "LAppModel.h"
#include "LAppDefine.h"

#include "FileManager.h"
#include "ModelSettingJson.h"
#include <util/UtSystem.h>

#include <L2DStandardID.h>

#include <sstream> 
#include <Live2DModelOpenGL.h>
#include "Log.hpp"
#include "L2DViewMatrix.h"


using namespace std;
using namespace live2d;
using namespace live2d::framework;


LAppModel::LAppModel()
	:L2DBaseModel(),modelSetting(NULL),currentFinishCallback(NULL),_offx(0.0f),_offy(0.0f),_scale(1.0f),_ww(800),_wh(600),_lipSyncN(1.0f)
{
	lipSync = true;
	if (LAppDefine::DEBUG_LOG)
	{
		mainMotionMgr->setMotionDebugMode(true);
	}
}


LAppModel::~LAppModel(void)
{
	delete modelSetting;
}

void LAppModel::load(const char* _path)
{
	std::string path = _path;
	Info( "load model : %s",path.c_str());	
    updating=true;
    initialized=false;
    
	int size ;

	unsigned char* data = FileManager::loadFile( path.c_str(), &size);

    modelSetting = new ModelSettingJson( (char*)data , size );

	FileManager::releaseBuffer(data);
	
	FileManager::getParentDir( path.c_str(), &modelHomeDir);

    Info( "create model : %s", modelSetting->getModelName());	
    updating=true;
    initialized=false;

   
    if( strcmp( modelSetting->getModelFile() , "" ) != 0 )
    {        
        string mocPath=modelSetting->getModelFile();
		mocPath =modelHomeDir + mocPath;

        loadModelData(mocPath.c_str());
        
		int len=modelSetting->getTextureNum();
		for (int i=0; i<len; i++)
		{
			string texturePath=modelSetting->getTextureFile(i);
			texturePath=modelHomeDir+texturePath;
			loadTexture(i, texturePath.c_str());
		}

		live2DModel->setPremultipliedAlpha(false);
    }
	
	if (live2DModel==NULL) {

		return;
	}

     //Expression
	if (modelSetting->getExpressionNum() > 0)
	{
		int len=modelSetting->getExpressionNum();
		for (int i=0; i<len; i++)
		{
			string name=modelSetting->getExpressionName(i);
			string file=modelSetting->getExpressionFile(i);
			file=modelHomeDir+file;
			loadExpression(name.c_str(),file.c_str());
		}
	}

	int cnt = 0;
	
	//Physics
	if( strcmp( modelSetting->getPhysicsFile(), "" ) != 0 )
    {
		string path=modelSetting->getPhysicsFile();
		path=modelHomeDir+path;
        loadPhysics(path.c_str());
    }
	
	//Pose
	if( strcmp( modelSetting->getPoseFile() , "" ) != 0 )
    {
		string path=modelSetting->getPoseFile();
		path=modelHomeDir+path;
        loadPose(path.c_str());
    }
	
	if (eyeBlink==NULL)
	{
		eyeBlink=new L2DEyeBlink();
	}
	
	//Layout
	map<string, float> layout;
	modelSetting->getLayout(layout);
	modelMatrix->setupLayout(layout);
	
	for ( int i = 0; i < modelSetting->getInitParamNum(); i++)
	{
		live2DModel->setParamFloat(modelSetting->getInitParamID(i), modelSetting->getInitParamValue(i));
	}

	for ( int i = 0; i < modelSetting->getInitPartsVisibleNum(); i++)
	{
		live2DModel->setPartsOpacity(modelSetting->getInitPartsVisibleID(i), modelSetting->getInitPartsVisibleValue(i));
	}
	
	live2DModel->saveParam();

	preloadMotionGroup(MOTION_GROUP_IDLE);

	mainMotionMgr->stopAllMotions();
	
    updating=false;
    initialized=true;
}


void LAppModel::preloadMotionGroup(const char group[])
{
    int len = modelSetting->getMotionNum( group );
    for (int i = 0; i < len; i++)
	{
		std::stringstream ss;
		
		//ex) idle_0
		ss << group << "_" <<  i;
		
		string name=ss.str();
		string path=modelSetting->getMotionFile(group,i);
		path=modelHomeDir+path;

		Info("load motion name:%s ",name.c_str());
        
		AMotion* motion=loadMotion(name.c_str(),path.c_str());
    }
}


void LAppModel::update()
{
	dragMgr->update();
	dragX=dragMgr->getX();
	dragY=dragMgr->getY();

	//-----------------------------------------------------------------
	live2DModel->loadParam();
	if(mainMotionMgr->isFinished())
	{
		if (currentFinishCallback)
		{
			currentFinishCallback();
			currentFinishCallback = NULL;
		}
	}
	else
	{
		bool update = mainMotionMgr->updateParam(live2DModel);
		
		if( ! update){
			
			eyeBlink->setParam(live2DModel);
		}
	}
	live2DModel->saveParam();
	//-----------------------------------------------------------------
	
	
	if(expressionMgr!=NULL)expressionMgr->updateParam(live2DModel);
	
	live2DModel->addToParamFloat( PARAM_ANGLE_X, dragX *  30 , 1 );
	live2DModel->addToParamFloat( PARAM_ANGLE_Y, dragY *  30 , 1 );
	live2DModel->addToParamFloat( PARAM_ANGLE_Z, (dragX*dragY) * -30 , 1 );
	
	
	live2DModel->addToParamFloat( PARAM_BODY_ANGLE_X    , dragX * 10 , 1 );
	
	
	live2DModel->addToParamFloat( PARAM_EYE_BALL_X, dragX  , 1 );
	live2DModel->addToParamFloat( PARAM_EYE_BALL_Y, dragY  , 1 );
	
	
	LDint64	 timeMSec = UtSystem::getUserTimeMSec() - startTimeMSec  ;
	double t = (timeMSec / 1000.0) * 2 * 3.14159  ;//2*Pi*t
	
	live2DModel->addToParamFloat( PARAM_ANGLE_X,	(float) (15 * sin( t/ 6.5345 )) , 0.5f);
	live2DModel->addToParamFloat( PARAM_ANGLE_Y,	(float) ( 8 * sin( t/ 3.5345 )) , 0.5f);
	live2DModel->addToParamFloat( PARAM_ANGLE_Z,	(float) (10 * sin( t/ 5.5345 )) , 0.5f);
	live2DModel->addToParamFloat( PARAM_BODY_ANGLE_X,	(float) ( 4 * sin( t/15.5345 )) , 0.5f);
	live2DModel->setParamFloat  ( PARAM_BREATH,	(float) (0.5f + 0.5f * sin( t/3.2345 )),1);
	
	
	if(physics!=NULL)physics->updateParam(live2DModel);

	
	if(lipSync)
	{
		float value = 0.0f;
		float ldt = timeMSec / 1000.0;
		_wavHandler.Update(ldt);
		value = _wavHandler.GetRms() * _lipSyncN;
		live2DModel->setParamFloat(PARAM_MOUTH_OPEN_Y, value ,0.8f);
	}
	
	
	if(pose!=NULL)pose->updateParam(live2DModel);

	live2DModel->update();
}


int LAppModel::startMotion(const char group[],int no,int priority, OnStartMotionCallback s_call, OnFinishMotionCallback f_call)
{
	if (priority==PRIORITY_FORCE)
	{
		mainMotionMgr->setReservePriority(priority);
	}
	else if (! mainMotionMgr->reserveMotion(priority))
	{
		Info("can't start motion.\n");
		return -1;
	}
	
	const char* fileName = modelSetting->getMotionFile(group, no);
	std::stringstream ss;

	ss << group << "_" << no;

	string name = ss.str();
	AMotion* motion = motions[name.c_str()];
	bool autoDelete = false;
	bool hasMotion = false;

	if (strcmp(fileName, "") == 0 || strcmp("Error:type mismatch", fileName) == 0)
	{
		goto label;
	}	

	hasMotion = true;
	
	if ( motion == NULL )
	{
		
		string path=fileName;
		path=modelHomeDir+path;
		motion = loadMotion(NULL,path.c_str());
		
		autoDelete = true;
	}

	currentFinishCallback = f_call;
	
	motion->setFadeIn(  modelSetting->getMotionFadeIn(group,no)  );
	motion->setFadeOut( modelSetting->getMotionFadeOut(group,no) );


label:
	const char* soundFile = modelSetting->getMotionSound(group, no);
	if (strcmp(soundFile, "") != 0 && strcmp("Error:type mismatch", soundFile) != 0)
	{
		string path = soundFile;
		path = modelHomeDir + path;
		LDint64	 timeMSec = UtSystem::getUserTimeMSec() - startTimeMSec;
		_wavHandler.Start(path, (float)(timeMSec/1000.0));
		Info("lip sync: %s", path.c_str());
	}

    Info("start motion ( %s : %d )", group, no);
	if (s_call)
	{
		s_call(group, no);
	}

	if (!hasMotion)
	{
		if (f_call)
			f_call();
		currentFinishCallback = NULL;
		mainMotionMgr->setReservePriority(PRIORITY_NONE);
		return 0;
	}

	return mainMotionMgr->startMotionPrio(motion,autoDelete,priority);
}


int LAppModel::startRandomMotion(const char name[],int priority, OnStartMotionCallback s_call, OnFinishMotionCallback f_call)
{
	if(modelSetting->getMotionNum(name)==0)return -1;
    int no = rand() % modelSetting->getMotionNum(name); 
    
    return startMotion(name,no,priority,s_call,f_call);
}



void LAppModel::draw()
{
	live2d::Live2DModelOpenGL *model = (live2d::Live2DModelOpenGL*)live2DModel;
	_projection.identity();

	if (model->getCanvasWidth() > 1.0f && _ww < _wh)
	{
		// ����L����ǥ��k�L������ɥ��˱�ʾ�����H��ǥ�κ᥵������scale���������
		modelMatrix->setWidth(2.0f);
		_projection.scale(1.0f, static_cast<float>(_ww) / static_cast<float>(_wh));
	}
	else
	{
		_projection.scale(static_cast<float>(_wh) / static_cast<float>(_ww), 1.0f);
	}

	_projection.multScale(_scale, _scale);

	_projection.translate(_offx, _offy);

	_projection.mul(_projection.getArray(), modelMatrix->getArray(), _projection.getArray());

	model->setMatrix(_projection.getArray());

	live2DModel->draw();
}

void LAppModel::resize(int w, int h)
{
	_ww = w;
	_wh = h;
}

void LAppModel::setCenter(float x, float y)
{
	_offx = x;
	_offy = y;
}

void LAppModel::setScale(float scale)
{
	_scale = scale;
}

std::string LAppModel::hitTest(float testX,float testY)
{
	if(alpha<1)return "";

	int len=modelSetting->getHitAreasNum();
	for (int i = 0; i < len; i++)
	{
		const char* drawID=modelSetting->getHitAreaID(i);
		if (hitTestSimple(drawID, testX, testY))
		{
			return drawID;
		}
	}
	return "";
}

bool LAppModel::isMotionFinished()
{
	return mainMotionMgr->isFinished();
}


void LAppModel::setExpression(const char expressionID[])
{
	AMotion* motion = expressions[expressionID] ;
	Info( "expression[%s]\n" , expressionID ) ;
	if( motion != NULL )
	{
		expressionMgr->startMotion(motion, false) ;
	}
	else
	{
		Info( "expression[%s] is null \n" , expressionID ) ;
	}
}


void LAppModel::setRandomExpression()
{
	int no=rand()%expressions.size();
	map<string,AMotion* >::const_iterator map_ite;
	int i=0;
	for(map_ite=expressions.begin();map_ite!=expressions.end();map_ite++)
	{
		if (i==no)
		{
			string name=(*map_ite).first;
			setExpression(name.c_str());
			return;
		}
		i++;
	}
}

void LAppModel::setLipSyncN(float n)
{
	_lipSyncN = n;
}
