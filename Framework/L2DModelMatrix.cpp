/**
 *
 *  You can modify and use this source freely
 *  only for the development of application related Live2D.
 *
 *  (c) Live2D Inc. All rights reserved.
 */
#include "L2DModelMatrix.h"

using namespace std;

namespace live2d
{
	namespace framework
	{
		L2DModelMatrix::L2DModelMatrix(float w,float h)
		{
			width=w;
			height=h;

			
			if (width>height)
			{
				setHeight(height/width);
			}
			else
			{
				setWidth(2);
			}
			centerX(0);
			centerY(0);
		}

		
		
		void L2DModelMatrix::setCenterPosition(float x,float y)
		{
			centerX(x);
			centerY(y);
		}
		
		
		void L2DModelMatrix::bottom(float y)
		{
			float h = height * getScaleY();
			translateY( y - h);
		}
		
		
		void L2DModelMatrix::centerY(float y)
		{
			float h = height * getScaleY();
			translateY( y - h/2);
		}
		
		
		void L2DModelMatrix::right(float x)
		{
			float w = width * getScaleX();
			translateX(x - w);
		}
		
		
		void L2DModelMatrix::centerX(float x)
		{
			float w = width * getScaleX();
			translateX(x - w/2);
		}
		
		
		
		void L2DModelMatrix::setWidth(float w)
		{
			float scaleX = w/width;
			float scaleY = - scaleX ;
			scale(scaleX, scaleY);
		}
		
		
		
		void L2DModelMatrix::setHeight(float h)
		{
			float scaleX = h/height;
			float scaleY = - scaleX ;
			scale(scaleX, scaleY);
		}


		void L2DModelMatrix::setupLayout(map<string, float>& layout)
		{
			const char* KEY_WIDTH="width";
			const char* KEY_HEIGHT="height";
			const char* KEY_X="x";
			const char* KEY_Y="y";
			const char* KEY_CENTER_X="center_x";
			const char* KEY_CENTER_Y="center_y";
			const char* KEY_TOP="top";
			const char* KEY_BOTTOM="bottom";
			const char* KEY_LEFT="left";
			const char* KEY_RIGHT="right";

			map<string, float>::const_iterator ite;
		
			for (ite=layout.begin(); ite!=layout.end(); ite++) {
				string key=(*ite).first;
				float value=(*ite).second;
				if (key==KEY_WIDTH)setWidth(value);
				else if (key==KEY_HEIGHT )setHeight(value);
			}
		
			for (ite=layout.begin(); ite!=layout.end(); ite++) {
				string key=(*ite).first;
				float value=(*ite).second;
				if (key==KEY_X )setX(value);
				else if (key==KEY_Y )setY(value);
				else if (key==KEY_CENTER_X )centerX(value);
				else if (key==KEY_CENTER_Y)centerY(value);
				else if (key==KEY_TOP)top(value);
				else if (key==KEY_BOTTOM )bottom(value);
				else if (key==KEY_LEFT)left(value);
				else if (key==KEY_RIGHT )right(value);
			}
		}
		
	}
}