/**
 *
 *  You can modify and use this source freely
 *  only for the development of application related Live2D.
 *
 *  (c) Live2D Inc. All rights reserved.
 */
#pragma once

namespace live2d
{
	namespace framework
	{
		class L2DMatrix44
		{
		protected:
			float tr[16];
			
		public:
			L2DMatrix44();
			virtual ~L2DMatrix44(){}
			
			
			void identity() ;
			
			
			float* getArray(){ return tr; }
			
			
			void setMatrix( float* _tr );
			
			
			float getScaleX(){return tr[0] ;}
			
			
			float getScaleY(){return tr[5] ;}
			
			float getTranslateX(){return tr[12] ;}
			
			float getTranslateY(){return tr[13] ;}
			
			
			float transformX( float src );
			
			
			float transformY( float src );
			
			
			float invertTransformX( float src );
			
			
			float invertTransformY( float src );
			
			
			void multTranslate( float shiftX, float shiftY );
			void translate( float shiftX, float shiftY );
			void translateX( float shiftX ) {tr[12]=shiftX;}
			void translateY( float shiftY ) {tr[13]=shiftY;}
			
			
			void multScale( float scaleX,float scaleY );
			void scale( float scaleX,float scaleY );
			
			
			static void mul( float* a, float* b, float* dst );
			
			void append(L2DMatrix44* m);
		};
	}
}