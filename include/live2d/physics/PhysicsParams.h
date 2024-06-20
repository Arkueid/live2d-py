/**
 *  PhysicsParams.h
 *
 *  Created on: 2010/11/29
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */
#ifndef __LIVE2D_PHYSICS_PARAMS_H__
#define __LIVE2D_PHYSICS_PARAMS_H__



#ifndef __SKIP_DOC__


#include "PhysicsHair.h"
#include "../memory/LDObject.h"


//--------- LIVE2D NAMESPACE ------------
namespace live2d 
{

	
	class IPhysicsSrc : public live2d::LDObject 
	{
	public:
		IPhysicsSrc(const char* _paramID , float _scale , float _weight ) ;
		virtual ~IPhysicsSrc();

	public:
		virtual void updateSrc(live2d::ALive2DModel * model , PhysicsHair &hair ) ;

	protected:
		const char * 		paramID ;		// パラメータID（解放しない）
		float 				scale ;			// 拡大率
		float 				weight ;		// 重み
	};


	
	class PhysicsSrc : public IPhysicsSrc 
	{
	public:
		PhysicsSrc( PhysicsHair::Src srcType , const char*_paramID , float _scale , float _weight ) ;
		virtual ~PhysicsSrc();

		virtual void updateSrc(live2d::ALive2DModel * model , PhysicsHair &hair ) ;

	protected:
		PhysicsHair::Src srcType;
	} ;


	
	class IPhysicsTarget : public live2d::LDObject 
	{
	public:
		IPhysicsTarget(const char* _paramID , float _scale , float _weight ) ;
		virtual ~IPhysicsTarget();

	public:
		virtual void updateTarget(live2d::ALive2DModel * model , PhysicsHair &hair ) ;

	protected:
		const char* 		paramID ;		// 解放しない
		float 				scale ;			// 拡大率
		float 				weight ;		// 重み
	};


	
	class PhysicsTarget : public IPhysicsTarget 
	{
	public:
		PhysicsTarget( PhysicsHair::Target targetType , const char*_paramID , float _scale , float _weight ) ;
		virtual ~PhysicsTarget();

	public:
		virtual void updateTarget(live2d::ALive2DModel * model , PhysicsHair &hair ) ;

	protected:
		PhysicsHair::Target 	targetType ;
	} ;

}
//--------- LIVE2D NAMESPACE ------------

#endif // __SKIP_DOC__

#endif // __LIVE2D_PHYSICS_PARAMS_H__
