
#ifndef __LIVE2D_LDMATRIX44_H__
#define __LIVE2D_LDMATRIX44_H__


#ifndef __SKIP_DOC__


#include "../Live2D.h"
#include "../memory/LDObject.h"
#include "../type/LDPointF.h"
#include "../type/LDVector.h"
#include "../util/UtMath.h"

//--------- LIVE2D NAMESPACE ------------
namespace live2d
{
	class LDMatrix44 : public live2d::LDObject 
	{
	protected:
		float m[16];

	public:
		LDMatrix44(void);
		LDMatrix44(float *f);
		~LDMatrix44(void);

		void identity();
		float* getArray();
		void getCopyMatrix(float* matrix);
		void setMatrix(float *tr);
		void setMatrix(LDMatrix44 *matrix);
		//char* ToString();
		LDMatrix44* mult(LDMatrix44 *b, LDMatrix44 *dst, bool omit4thRow);
		void translate( float x, float y , float z );
		void scale( float scaleX ,float scaleY , float scaleZ );
		void rotateX( float thetaX );
		void rotateY( float thetaY );
		void rotateZ( float thetaZ );

	private:
		void mult_safe( float *a , float *b , float *dst, bool omit4thRow );
		void mult_fast( float *a , float *b , float *dst, bool omit4thRow );
	};

} //------------------------- LIVE2D NAMESPACE ------------

#endif		// __SKIP_DOC__

#endif		// __LIVE2D_LDMATRIX44_H__