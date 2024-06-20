/**
 *
 *  You can modify and use this source freely
 *  only for the development of application related Live2D.
 *
 *  (c) Live2D Inc. All rights reserved.
 */
#include "L2DPhysics.h"
#include "util/UtSystem.h"
#include "util/Json.h"

using namespace std;

namespace live2d
{
	namespace framework
	{
		L2DPhysics::L2DPhysics(void)
		{
			startTimeMSec = live2d::UtSystem::getUserTimeMSec() ;
		}
		
		
		L2DPhysics::~L2DPhysics(void)
		{
			for (unsigned int i = 0; i < physicsObjects.size(); i++)
			{
				delete physicsObjects[i];
			}
			physicsObjects.clear();
			
			for (unsigned int i = 0; i < paramIDs.size(); i++)
			{
				delete paramIDs[i];
			}
			paramIDs.clear();
		}
		
		
		
		void L2DPhysics::updateParam(live2d::ALive2DModel* model)
		{
			l2d_int64 timeMSec = live2d::UtSystem::getUserTimeMSec() - startTimeMSec  ;
			
			for (unsigned int i = 0; i < physicsObjects.size(); i++)
			{
				physicsObjects[i]->update(model, timeMSec);
			}
		}
		
		
		L2DPhysics* L2DPhysics::load(const string & filepath )
		{
			int size = 0 ;
			
			char * buf = UtFile::loadFile(filepath.c_str() , &size ) ;
			if( buf == NULL ){
				UtDebug::error( "load file failed : file : %s @L2DPhysics#load()" , filepath.c_str() ) ;
				return NULL ;
			}
			
			L2DPhysics* ret = load( buf , size ) ;
			
			if( buf ) UtFile::releaseLoadBuffer( buf ) ;
			
			return ret ;
		}
		
		
		
		L2DPhysics* L2DPhysics::load(const void* buf ,int size)
		{
			
			L2DPhysics* ret = new L2DPhysics();
			
			live2d::Json* json = live2d::Json::parseFromBytes( (const char*)buf , size ) ;
			live2d::Value& root = json->getRoot();
			
			
			live2d::Value& params = root["physics_hair"];
			int paramNum = params.size();
			
			for (int i = 0; i < paramNum; i++)
			{
				live2d::Value& param = params[i];
				
				PhysicsHair* physics=new PhysicsHair();
				
				live2d::Value& setup = param["setup"];
				
				float length = (float)setup["length"].toDouble();
				float resist = (float)setup["regist"].toDouble();
				float mass = (float)setup["mass"].toDouble();
				physics->setup(length,resist,mass);
				
				
				live2d::Value& srcList = param["src"];
				
				int srcNum = srcList.size();
				for (int j = 0; j < srcNum; j++)
				{
					live2d::Value& src = srcList[j];
					LDString pid = src["id"].toString();//param ID
					PhysicsHair::Src type=PhysicsHair::SRC_TO_X;
					LDString typeStr=src["ptype"].toString();
					if(typeStr=="x")
					{
						type=PhysicsHair::SRC_TO_X;
					}
					else if(typeStr=="y")
					{
						type=PhysicsHair::SRC_TO_Y;
					}
					else if(typeStr=="angle")
					{
						type=PhysicsHair::SRC_TO_G_ANGLE;
					}else{
						live2d::UtDebug::error("live2d", "Invalid parameter:PhysicsHair.Src");
					}
					
					float scale = (float)src["scale"].toDouble();
					float weight = (float)src["weight"].toDouble();
					string* tmp=new string(pid.c_str());
					ret->paramIDs.push_back(tmp);
					physics->addSrcParam( type,tmp->c_str(), scale, weight);
				}
				
				
				live2d::Value& targetList = param["targets"];
				int targetNum = targetList.size();
				for (int j = 0; j < targetNum; j++) {
					live2d::Value& target = targetList[j];
					LDString pid = target["id"].toString();//param ID
					PhysicsHair::Target type=PhysicsHair::TARGET_FROM_ANGLE;
					LDString typeStr=target["ptype"].toString();
					if(typeStr=="angle")
					{
						type=PhysicsHair::TARGET_FROM_ANGLE;
					}
					else if(typeStr=="angle_v")
					{
						type=PhysicsHair::TARGET_FROM_ANGLE_V;
					}else{
						live2d::UtDebug::error("live2d", "Invalid parameter:PhysicsHair.Target");
					}
					
					float scale = (float)target["scale"].toDouble();
					float weight = (float)target["weight"].toDouble();
					
					string* tmp=new string(pid.c_str());
					ret->paramIDs.push_back(tmp);
					physics->addTargetParam( type,tmp->c_str()	, scale, weight);
				}
				ret->physicsObjects.push_back(physics);
			}
			delete json;
			
			return ret;
		}
	}
}