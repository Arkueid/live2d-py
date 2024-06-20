/**
 *
 *  You can modify and use this source freely
 *  only for the development of application related Live2D.
 *
 *  (c) Live2D Inc. All rights reserved.
 */
#include "L2DMatrix44.h"

namespace live2d
{
	namespace framework
	{
		L2DMatrix44::L2DMatrix44()
		{
			identity() ;
		}

		
		void L2DMatrix44::identity(){
			for (int i = 0 ; i < 16 ; i++ )	tr[i] = ((i%5) == 0) ? 1.0f : 0.0f ; 
		}

		void L2DMatrix44::mul( float* a , float* b , float* dst)
		{
			float c[16] = {0,0,0,0 , 0,0,0,0 , 0,0,0,0 , 0,0,0,0 } ;
			int n = 4 ;
			int i,j,k;
			
			
			for(i=0;i<n;i++)
			{
				for(j=0;j<n;j++)
				{
					for(k=0;k<n;k++)
					{
						//c[i*4+j]+=a[i*4+k]*b[k*4+j];
						c[i+j*4]+=a[i+k*4]*b[k+j*4];
					}
				}
			}
			
			for(i =0 ; i < 16 ; i++ )
			{
				dst[i] = c[i] ;
			}
		}


		void L2DMatrix44::multTranslate( float x, float y )
		{
			float tr1[16] = { 1,0,0,0 , 0,1,0,0 , 0,0,1,0 , x,y,0,1 } ;
			mul( tr1 , tr , tr ) ;
		}


		void L2DMatrix44::translate( float x, float y )
		{
			tr[12]=x;
			tr[13]=y;
		}


		void L2DMatrix44::multScale( float scaleX,float scaleY )
		{
			float tr1[16] = { scaleX,0,0,0 , 0,scaleY,0,0 , 0,0,1,0 , 0,0,0,1 } ;
			mul( tr1 , tr , tr ) ;	
		}


		void L2DMatrix44::scale( float scaleX,float scaleY )
		{
			tr[0]=scaleX;
			tr[5]=scaleY;
		}


		float L2DMatrix44::transformX( float src )
		{
			return tr[0]*src + tr[12] ;
		}


		float L2DMatrix44::invertTransformX( float src )
		{
			return (src - tr[12]) / tr[0] ;
		}


		float L2DMatrix44::transformY( float src )
		{
			return tr[5]*src + tr[13] ;
		}


		float L2DMatrix44::invertTransformY( float src )
		{
			return (src - tr[13]) / tr[5] ;
		}


		void L2DMatrix44::setMatrix( float* _tr )
		{
			for (int i = 0 ; i < 16 ; i++ )	tr[i] = _tr[i] ;
		}

		void L2DMatrix44::append(L2DMatrix44* m)
		{
			mul(m->getArray(),tr,tr);
		}
	}
}
