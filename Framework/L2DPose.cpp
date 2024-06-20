/**
 *
 *  You can modify and use this source freely
 *  only for the development of application related Live2D.
 *
 *  (c) Live2D Inc. All rights reserved.
 */
#include "L2DPose.h"

namespace live2d
{
	namespace framework
	{
		L2DPose::L2DPose() :lastModel(NULL),lastTime(0)
		{
		}
		
		
		
		void L2DPose::updateParam(ALive2DModel* model)
		{
			
			if( model != lastModel )
			{
				
				initParam(model);
			}
			lastModel = model;
			
			l2d_int64  curTime = UtSystem::getTimeMSec();
			float deltaTimeSec = ( (lastTime == 0 ) ? 0 : ( curTime - lastTime )/1000.0f);
			lastTime = curTime;
			
			
			if (deltaTimeSec < 0) deltaTimeSec = 0;
			
			int offset=0;
			for (unsigned int i = 0; i < groupRows.size(); i++)
			{
				int rowCount=groupRows[i];
				normalizePartsOpacityGroup(model,deltaTimeSec,offset,rowCount);
				offset+=rowCount;
			}
			copyOpacityOtherParts(model);
		}
		
		
		
		void L2DPose::initParam(ALive2DModel* model)
		{
			int offset=0;
			for (unsigned int i=0; i<groupRows.size(); i++) {
				int rowCount=groupRows[i];
				for (int j = offset; j < offset+rowCount; j++)
				{
					partsGroups[j].initIndex(model);
					int partsIndex = partsGroups[j].partsIndex ;
					int paramIndex = partsGroups[j].paramIndex ;
					
					if(partsIndex<0)continue;
					
					model->setPartsOpacity(partsIndex , (j==offset ? 1.0f : 0.0f) ) ;
					model->setParamFloat(paramIndex , (j==offset ? 1.0f : 0.0f) ) ;
					
					for (unsigned int k = 0; k < partsGroups[j].link.size(); k++)
					{
						partsGroups[j].link[k].initIndex(model);
					}
				}
				offset+=rowCount;
			}
			
		}
		
		
		
		void L2DPose::normalizePartsOpacityGroup( ALive2DModel* model , float deltaTimeSec ,int offset ,int rowCount)
		{
			int visibleParts = -1 ;
			float visibleOpacity = 1.0f ;
			
			float CLEAR_TIME_SEC = 0.5f ;
			float phi = 0.5f ;
			float maxBackOpacity = 0.15f ;
			
			
			
			for (int i = offset ; i < offset + rowCount; i++ )
			{
				int partsIndex = partsGroups[i].partsIndex;
				int paramIndex = partsGroups[i].paramIndex;
				
				if( model->getParamFloat( paramIndex ) != 0 )
				{
					if( visibleParts >= 0 )
					{
						break ;
					}
					
					visibleParts = i ;
					visibleOpacity = model->getPartsOpacity(partsIndex) ;
					
					
					visibleOpacity += deltaTimeSec / CLEAR_TIME_SEC ;
					if( visibleOpacity > 1 )
					{
						visibleOpacity = 1 ;
					}
				}
			}
			
			if( visibleParts < 0 )
			{
				visibleParts = 0 ;
				visibleOpacity = 1 ;
			}
			
			
			for (int i = offset ; i < offset + rowCount ; i++ )
			{
				int partsIndex = partsGroups[i].partsIndex;
				
				
				if( visibleParts == i )
				{
					model->setPartsOpacity(partsIndex , visibleOpacity ) ;
				}
				
				else
				{
					float opacity = model->getPartsOpacity(partsIndex) ;
					float a1 ;
					if( visibleOpacity < phi )
					{
						a1 = visibleOpacity*(phi-1)/phi + 1 ; 
					}
					else
					{
						a1 = (1-visibleOpacity)*phi/(1-phi) ; 
					}
					
					
					float backOp = (1-a1)*(1-visibleOpacity) ;
					if( backOp > maxBackOpacity )
					{
						a1 = 1 - maxBackOpacity/( 1- visibleOpacity ) ;
					}
					
					if( opacity > a1 )
					{
						opacity = a1 ;
					}
					model->setPartsOpacity(partsIndex , opacity ) ;
				}
			}
		}
		
		
		
		void L2DPose::copyOpacityOtherParts(ALive2DModel* model)
		{
			for (unsigned int i_group = 0; i_group < partsGroups.size(); i_group++)
			{
				L2DPartsParam &partsParam = partsGroups[i_group];
				
				if(partsParam.link.size()==0)continue;
				
				int partsIndex = partsGroups[i_group].partsIndex;
				
				float opacity = model->getPartsOpacity( partsIndex );
				
				for (unsigned int i_link = 0; i_link < partsParam.link.size(); i_link++)
				{
					L2DPartsParam &linkParts = partsParam.link[i_link];
					
					int linkPartsIndex = linkParts.partsIndex;
					
					if(linkPartsIndex < 0)continue;
					model->setPartsOpacity(linkPartsIndex, opacity);
				}
			}
		}
		
		
		/**
		 * JSONファイルから読み込む
		 * 仕様についてはマニュアル参照。JSONスキーマの形式の仕様がある。
		 * @param buf
		 * @return
		 */
		L2DPose* L2DPose::load(const void* buf ,int size)
		{
			L2DPose* ret = new L2DPose();
			
			Json* json = Json::parseFromBytes( (const char*)buf , size ) ;
			
			Value& root = json->getRoot() ;
			
			
			Value& poseListInfo = root["parts_visible"];
			int poseNum = poseListInfo.size();
			
			for (int i_pose = 0; i_pose < poseNum; i_pose++)
			{
				Value& poseInfo = poseListInfo[i_pose];
				
				
				Value& idListInfo = poseInfo["group"];
				int idNum = idListInfo.size();
				int rowCount=0;
				for (int i_group = 0; i_group < idNum; i_group++)
				{
					Value& partsInfo=idListInfo[i_group];
					L2DPartsParam parts;
					LDString paramID=partsInfo["id"].toString();
					parts.partsID=paramID;
					
					
					if(partsInfo["link"].isNull())
					{
						
					}
					else
					{
						Value &linkListInfo = partsInfo["link"];
						int linkNum = linkListInfo.size();
						
						for (int i_link = 0; i_link< linkNum; i_link++)
						{
							L2DPartsParam linkParts;
							LDString linkID=linkListInfo[i_link].toString();
							linkParts.partsID=linkID;
							parts.link.push_back(linkParts);
						}
					}
					ret->partsGroups.push_back(parts);
					rowCount++;
				}
				ret->groupRows.push_back(rowCount);
				
			}
			
			delete json;
			return ret;
		}
	}
}