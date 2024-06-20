/**
 *
 *  You can modify and use this source freely
 *  only for the development of application related Live2D.
 *
 *  (c) Live2D Inc. All rights reserved.
 */
#pragma once

#include <live2d/type/LDVector.h>
#include <math.h>
#include "MyLive2DAllocator.h"


class LAppModel;
class L2DViewMatrix;

class LAppLive2DManager{
private :
	
	LAppModel* currentModel;

	MyLive2DAllocator	myAllocator;
public:
    
    LAppLive2DManager() ;    
    ~LAppLive2DManager() ; 

    LAppModel* current();
    
	void loadModel(const char* path);
	void releaseModel();
    bool tapEvent(float x,float y) ;
    void setDrag(float x, float y);
};

