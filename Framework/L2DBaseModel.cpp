/**
 *
 *  You can modify and use this source freely
 *  only for the development of application related Live2D.
 *
 *  (c) Live2D Inc. All rights reserved.
 */
#include "L2DBaseModel.h"
#include "Live2DFramework.h"

namespace live2d
{
	namespace framework
	{
		L2DBaseModel::L2DBaseModel()
			:mainMotionMgr(NULL),expressionMgr(NULL),eyeBlink(NULL),modelMatrix(NULL),physics(NULL),pose(NULL),
			initialized(false),updating(false),alpha(1),accAlpha(0),lipSync(false),lastLipSyncValue(0),
			dragX(0),dragY(0),debugMode(false),live2DModel(NULL)
		{
			
		    mainMotionMgr=new L2DMotionManager();
			expressionMgr=new L2DMotionManager();
			startTimeMSec=UtSystem::getUserTimeMSec();

			eyeBlink=new L2DEyeBlink();

			
			dragMgr=new L2DTargetPoint();
		}


		L2DBaseModel::~L2DBaseModel(void)
		{
			if(live2DModel)delete live2DModel;
		    if(physics)delete physics;
		    if(modelMatrix)delete modelMatrix;
			if(pose)delete pose;
			if(eyeBlink)delete eyeBlink;
			if(dragMgr)delete dragMgr;
			
			releaseTextures();
			releaseMotions();
			releaseExpressions();
		}
		
		void L2DBaseModel::releaseTextures()
		{
			for (int i = 0; i < textures.size(); i++)
			{
				delete textures[i];
			}
			textures.clear();
		}

		void L2DBaseModel::releaseMotions()
		{
			if (mainMotionMgr!=NULL)	delete mainMotionMgr;
			
			std::map<std::string,AMotion* >::const_iterator map_ite;
			for(map_ite=motions.begin();map_ite!=motions.end();map_ite++)
			{
				delete (*map_ite).second;
			}
			motions.clear();
		}
		
		
		void L2DBaseModel::releaseExpressions()
		{
			if (expressionMgr!=NULL)delete expressionMgr;
			
			std::map<std::string,AMotion* >::const_iterator map_ite;
			for(map_ite=expressions.begin();map_ite!=expressions.end();map_ite++)
			{
				delete (*map_ite).second;
			}
			expressions.clear();
		}
		
		
		void L2DBaseModel::setAccel(float x,float y,float z)
		{
			accelX=x;
			accelY=y;
			accelZ=z;
		}

		void L2DBaseModel::loadModelData(const char* path)
		{
			IPlatformManager* pm=Live2DFramework::getPlatformManager();
			

			if(debugMode)
			{
				pm->log( "Load model : ");
				pm->log( path);
			}

			live2DModel = pm->loadLive2DModel(path);
			live2DModel->saveParam();

			if(Live2D::getError()!=Live2D::L2D_NO_ERROR)
			{
				pm->log("Error : Failed to loadModelData().");
				return;
			}

			modelMatrix=new L2DModelMatrix(live2DModel->getCanvasWidth(),live2DModel->getCanvasHeight());
			modelMatrix->setWidth(2);
			modelMatrix->setCenterPosition(0, 0);
			
		}

		void L2DBaseModel::loadTexture(int no,const char path[])
		{
			IPlatformManager* pm=Live2DFramework::getPlatformManager();
			
			L2DTextureDesc* desc=pm->loadTexture(live2DModel,no,path);

			textures.push_back(desc);
		}

		void L2DBaseModel::loadExpression(const char* name,const char* path)
		{
			if (expressions[name]!=NULL)
			{
				delete expressions[name];
				expressions[name]=NULL;
			}

			IPlatformManager* pm=Live2DFramework::getPlatformManager();
			
			size_t size;
			void* data=pm->loadBytes(path, &size);;
	
			L2DExpressionMotion* motion = L2DExpressionMotion::loadJson(data,(int)size);
	
			pm->releaseBytes(data);

			expressions[name]= motion ;
		}

		void L2DBaseModel::loadPose( const char* path )
		{
			IPlatformManager* pm=Live2DFramework::getPlatformManager();
			
			size_t size;
			void* data= pm->loadBytes(path, &size);
			
			pose= L2DPose::load(data,(int)size) ;

			pm->releaseBytes(data);
		}

		void L2DBaseModel::loadPhysics( const char* path )
		{
			IPlatformManager* pm=Live2DFramework::getPlatformManager();

			size_t size;
			void* data= pm->loadBytes(path, &size);
			
			physics= L2DPhysics::load(data,(int)size) ;

			pm->releaseBytes(data);
		}

		bool L2DBaseModel::hitTestSimple(const char drawID[],float testX,float testY)
		{
			int drawIndex=live2DModel->getDrawDataIndex(drawID);
			if(drawIndex<0)return false;
			int count=0;
			float* points=live2DModel->getTransformedPoints(drawIndex,&count);
			
			float left=live2DModel->getCanvasWidth();
			float right=0;
			float top=live2DModel->getCanvasHeight();
			float bottom=0;
			
			for (int j = 0; j < count; j++)
			{
				float x = points[DEF::VERTEX_OFFSET+j*DEF::VERTEX_STEP];
				float y = points[DEF::VERTEX_OFFSET+j*DEF::VERTEX_STEP+1];
				if(x<left)left=x;	// Min x
				if(x>right)right=x;	// Max x
				if(y<top)top=y;		// Min y
				if(y>bottom)bottom=y;// Max y
			}
			float tx=modelMatrix->invertTransformX(testX);
			float ty=modelMatrix->invertTransformY(testY);
			
			return ( left <= tx && tx <= right && top <= ty && ty <= bottom ) ;
		}

		AMotion* L2DBaseModel::loadMotion(const char name[],const char path[])
		{
			IPlatformManager* pm=Live2DFramework::getPlatformManager();
	
			size_t size;
			void* data = pm->loadBytes(path, &size);
	
			AMotion* motion=Live2DMotion::loadMotion(data,(int)size);

			pm->releaseBytes(data);

			if ( name !=NULL)
			{
				if (motions[name]!=NULL)
				{
					delete motions[name];
				}
				motions[name] = motion; 
			}

			return motion;
		}

		void L2DBaseModel::setDrag(float x,float y)
		{
			dragMgr->set(x, y);
		}
	}
}




