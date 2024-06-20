/**
 *
 *  You can modify and use this source freely
 *  only for the development of application related Live2D.
 *
 *  (c) Live2D Inc. All rights reserved.
 */
#pragma once

#include "ModelSetting.h"
#include "util/Json.h"


static const char NAME[]="name";
static const char MODEL[]="model";
static const char _ID[]="id";
static const char TEXTURES[]="textures";
static const char INIT_PARAM[]="init_param";
static const char INIT_PARTS_VISIBLE[]="init_parts_visible";
static const char HIT_AREAS[]="hit_areas";
static const char EXPRESSIONS[]="expressions";
static const char POSE[]="pose";
static const char PHYSICS[]="physics";
static const char MOTION_GROUPS[]="motions";
static const char SOUND[]="sound";
static const char FADE_IN[]="fade_in";
static const char FADE_OUT[]="fade_out";
static const char VAL[]="val";
static const char LAYOUT[]="layout";
static const char _FILE[]="file";

class ModelSettingJson : public ModelSetting
{
private:
    live2d::Json* json;
    
    bool existModelName()       {return ! json->getRoot()[NAME].isNull() ;}
    bool existModelFile()       {return ! json->getRoot()[MODEL].isNull() ;}
    bool existTextureFiles()    {return ! json->getRoot()[TEXTURES].isNull() ;}
    bool existInitParam()       {return ! json->getRoot()[INIT_PARAM].isNull() ;}
    bool existInitPartsVisible(){return ! json->getRoot()[INIT_PARTS_VISIBLE].isNull();}
    bool existHitAreas()        {return ! json->getRoot()[HIT_AREAS].isNull() ;}
    bool existPhysicsFile()     {return ! json->getRoot()[PHYSICS].isNull() ;}
    bool existPoseFile()        {return ! json->getRoot()[POSE].isNull() ;}
    bool existExpressionFile()  {return ! json->getRoot()[EXPRESSIONS].isNull() ;}
    bool existMotionGroups()    {return ! json->getRoot()[MOTION_GROUPS].isNull() ;}
    bool existMotionGroup(const char* name)         {return ! json->getRoot()[MOTION_GROUPS][name].isNull() ;}
    bool existMotionSound(const char* name,int n)   {return ! json->getRoot()[MOTION_GROUPS][name][n][SOUND].isNull();}
    bool existMotionFadeIn(const char* name,int n)  {return ! json->getRoot()[MOTION_GROUPS][name][n][FADE_IN].isNull();}
    bool existMotionFadeOut(const char* name,int n) {return ! json->getRoot()[MOTION_GROUPS][name][n][FADE_OUT].isNull();}
    
public:
    ModelSettingJson(const char* buf,int size){
        json = live2d::Json::parseFromBytes( buf,size ) ;
    }
    ~ModelSettingJson(){
        delete json;
    }
	
    
    const char* getModelName()
    {
        if(!existModelName())return "";
        return json->getRoot()[NAME].toString().c_str();
    }
    
    
    const char* getModelFile()  
    {
        if(!existModelFile())return "";
        return json->getRoot()[MODEL].toString().c_str();
    }
    
    
    int getTextureNum()         
    {
        if(!existTextureFiles())return 0;
        return json->getRoot()[TEXTURES].size() ;
    }
    
    
    const char* getTextureFile(int n)  { return json->getRoot()[TEXTURES][n].toString().c_str(); }
    
    
    int getInitParamNum()       
    {
        if(!existInitParam())return 0;
        return json->getRoot()[INIT_PARAM].size();
    }
    
    
    float getInitParamValue(int n)  { return (float)json->getRoot()[INIT_PARAM][n][VAL].toDouble(); }
    const char* getInitParamID(int n) { return json->getRoot()[INIT_PARAM][n][_ID].toString().c_str(); }
    
    
    int getInitPartsVisibleNum()        
    {
        if(!existInitPartsVisible())return 0;
        return json->getRoot()[INIT_PARTS_VISIBLE].size();
    }
    
    
    float getInitPartsVisibleValue(int n) { return (float)json->getRoot()[INIT_PARTS_VISIBLE][n][VAL].toDouble(); }
    const char* getInitPartsVisibleID(int n) { return json->getRoot()[INIT_PARTS_VISIBLE][n][_ID].toString().c_str(); }
    
    
    
	int getHitAreasNum()        
    {
        if(!existHitAreas())return 0;
        return json->getRoot()[HIT_AREAS].size() ;
    }
	
	const char* getHitAreaID(int n)     {return json->getRoot()[HIT_AREAS][n][_ID].toString().c_str();}
	const char* getHitAreaName(int n)   {return json->getRoot()[HIT_AREAS][n][NAME].toString().c_str();}
	
    
	const char* getPhysicsFile()
    {
        if(!existPhysicsFile())return "";
        return json->getRoot()[PHYSICS].toString().c_str();
    }
    
    
	const char* getPoseFile()   
    {
        if(!existPoseFile())return "";
        return json->getRoot()[POSE].toString().c_str();
    }
    
	
    int getExpressionNum()
    {
        if(!existExpressionFile())return 0;
        return json->getRoot()[EXPRESSIONS].size();
    }
	
	
	const char* getExpressionFile(int n)
    {
        return json->getRoot()[EXPRESSIONS][n][_FILE].toString().c_str();
    }
	
	
    const char* getExpressionName(int n)
    {
        return json->getRoot()[EXPRESSIONS][n][NAME].toString().c_str();
    }
    
	
    
	int getMotionNum(const char* name)       
    {
        if(!existMotionGroup(name))return 0;
        return json->getRoot()[MOTION_GROUPS][name].size();
    }
    
    
	const char* getMotionFile(const char* name,int n)   
    {
        if(!existMotionGroup(name))return "";
        return json->getRoot()[MOTION_GROUPS][name][n][_FILE].toString().c_str();
    }
    
    
	const char* getMotionSound(const char* name,int n)  
    {
        if(!existMotionSound(name,n))return "";
        return json->getRoot()[MOTION_GROUPS][name][n][SOUND].toString().c_str();
    }
    
    
	int getMotionFadeIn(const char* name,int n)         
    {
        if(!existMotionFadeIn(name,n))return 1000;
        return json->getRoot()[MOTION_GROUPS][name][n][FADE_IN].toInt();
    }
    
    
	int getMotionFadeOut(const char* name,int n)        
    {
        if(!existMotionFadeOut(name,n))return 1000;
        return json->getRoot()[MOTION_GROUPS][name][n][FADE_OUT].toInt();
    }
    

    int getMotionGroupNum()
    {
        if ( ! existMotionGroups()) {
            return 0;
        }
        return json->getRoot()[MOTION_GROUPS].getKeys().size();
    }
    
    
    const char* getMotionGroupName(int n)
    {
        if ( ! existMotionGroups()) {
            return NULL;
        }
        return json->getRoot()[MOTION_GROUPS].getKeys()[n].c_str();
    }
	

	bool getLayout(std::map<std::string, float> & layout)
    {
		live2d::LDMap<live2d::LDString, live2d::Value* > * map=json->getRoot()[LAYOUT].getMap();
		if (map==NULL) {
			return false;
		}
		live2d::LDMap<live2d::LDString, live2d::Value* >::const_iterator map_ite;
		bool ret=false;
        for(map_ite=map->begin();map_ite!=map->end();map_ite++)
        {
            layout[(*map_ite).first.c_str()] = (float)(*map_ite).second->toDouble();
			ret=true;
        }
		return ret;
    }

};