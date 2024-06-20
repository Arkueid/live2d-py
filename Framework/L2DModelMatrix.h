/**
 *
 *  You can modify and use this source freely
 *  only for the development of application related Live2D.
 *
 *  (c) Live2D Inc. All rights reserved.
 */

#pragma once

#include "L2DMatrix44.h"
#include <map>
#include <string>

namespace live2d
{
	namespace framework
	{
		
		class L2DModelMatrix : public L2DMatrix44
		{
		private:
			float    width;
			float    height;
			
		public:
			L2DModelMatrix(float w,float h);
			~L2DModelMatrix(){}
			
			void setWidth(float w);
			void setHeight(float h);
			
			void setPosition(float x,float y){translate(x, y);}
			void setCenterPosition(float x,float y);
			
			void top(float y){setY(y);}
			void bottom(float y);
			void left(float x){setX(x);}
			void right(float x);
			
			void centerX(float x);
			void setX(float x){translateX(x);}
			void centerY(float y);
			void setY(float y){translateY(y);}
			void setupLayout(std::map<std::string, float>& layout);
		};
	}
}