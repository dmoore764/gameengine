#pragma once
#include <BulletDynamics/btBulletDynamicsCommon.h>
#include "game_math.h"

struct physics_world
{
	btBroadphaseInterface*					broadphase;
	btDefaultCollisionConfiguration*		collisionConfiguration;
	btCollisionDispatcher*					dispatcher;
	btSequentialImpulseConstraintSolver*	solver;
	btDiscreteDynamicsWorld*				dynamicsWorld;
};

enum physics_shapes
{
	SHAPE_PLANE_X			= 0,
	SHAPE_PLANE_Y			= 1,
	SHAPE_PLANE_Z			= 2,
	SHAPE_CAPSULE_X			= 3,
	SHAPE_CAPSULE_Y			= 4,
	SHAPE_CAPSULE_Z			= 5,
	SHAPE_CONE_X			= 6,
	SHAPE_CONE_Y			= 7, 
	SHAPE_CONE_Z			= 8, 
	SHAPE_SPHERE			= 9, 
	SHAPE_CYLINDER_X		= 10,
	SHAPE_CYLINDER_Y		= 11,
	SHAPE_CYLINDER_Z		= 12,
	SHAPE_BOX				= 13,
};

struct object_physics
{
	btCollisionShape* shape;
	btDefaultMotionState* motionState;
	btRigidBody* rigidBody;
	v3 offset;
};

void			physInitWorld(physics_world *world);
void			physCloseWorld(physics_world *world);
btQuaternion	physQuatToBTQuat(const quat &q);
btVector3		physVec3ToBTVec3(const v3 &v);
quat			physBTQuatToQuat(const btQuaternion &q);
v3				physBTVec3ToVec3(const btVector3 &v);

void			physConvertBTTrans(const btTransform &t, quat *q, v3 *v);
void			physAddRigidBodyToWorld(physics_world *world, btRigidBody *rigidBody);
void			physRemoveRigidBodyToWorld(physics_world *world, btRigidBody *rigidBody);
