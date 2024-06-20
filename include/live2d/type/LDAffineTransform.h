/**
 *  LDAffineTransform.h
 *
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */

#pragma once


#ifndef __SKIP_DOC__

#include "../Live2D.h"
#include "../memory/LDObject.h"

//------------ LIVE2D NAMESPACE ------------
namespace live2d
{ 

	class LDAffineTransform : public live2d::LDObject 
	{
	public:

		LDAffineTransform(void);
		LDAffineTransform(float m1, float m2, float m3, float m4, float m5,float m6);
		virtual ~LDAffineTransform(void);

		void setFactor(float *fmat);
		void factorize(float* ret) const ;
		void getMatrix(float* ret) const;//

		//  二つのアフィン変換を補間する
		static void interpolate(LDAffineTransform &aa1, LDAffineTransform &aa2, float t, LDAffineTransform &ret) ;

		//  点列を変換する
		void transform( float *src , float *dst , int numPoint ) ;

	#if L2D_SAMPLE
		void DUMP() const ;
		static void TEST_AFFINE() ;
	#endif


	private:
		//Prevention of copy Constructor
		LDAffineTransform( const LDAffineTransform & ) ;
		LDAffineTransform& operator=( const LDAffineTransform & ) ;
		
		void update() ;

		static const int MODE_UNKNOWN = -1;
		static const int MODE_IDENTITY = 0;
		static const int MODE_TRANSLATION = 1;
		static const int MODE_UNIFORM_SCALE = 2;
		static const int MODE_GENERAL_SCALE = 4;
		static const int MODE_MASK_SCALE = (MODE_UNIFORM_SCALE |MODE_GENERAL_SCALE);
		static const int MODE_FLIP = 64;

		static const int MODE_QUADRANT_ROTATION = 8;
		static const int MODE_GENERAL_ROTATION = 16;
		static const int MODE_MASK_ROTATION = (MODE_QUADRANT_ROTATION |MODE_GENERAL_ROTATION);
		static const int MODE_GENERAL_TRANSFORM = 32;

		static const int STATE_IDENTITY = 0;
		static const int STATE_TRANSLATE = 1;
		static const int STATE_SCALE = 2;
		static const int STATE_SHEAR = 4;

		//------------ field ------------
		float m00;
		float m10;
		float m01;
		float m11;
		float m02;
		float m12;

		int state ;
		int mode ;

	};


} //------------ LIVE2D NAMESPACE ------------

#endif //__SKIP_DOC__
