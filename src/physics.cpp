#include "physics.h"

void physInitWorld(physics_world *world)
{
	world->broadphase = new btDbvtBroadphase();
	world->collisionConfiguration = new btDefaultCollisionConfiguration();
	world->dispatcher = new btCollisionDispatcher(world->collisionConfiguration);
	world->solver = new btSequentialImpulseConstraintSolver;
	world->dynamicsWorld = new btDiscreteDynamicsWorld(world->dispatcher, world->broadphase, world->solver, world->collisionConfiguration);
	world->dynamicsWorld->setGravity(btVector3(0, 0, -9.8f));
}

void physCloseWorld(physics_world *world)
{
	delete world->dynamicsWorld;
    delete world->solver;
    delete world->dispatcher;
    delete world->collisionConfiguration;
    delete world->broadphase;

	memset(world, '\0', sizeof(physics_world));
}

btQuaternion physQuatToBTQuat(const quat &q)
{
	btQuaternion result(q.x, q.y, q.z, q.w);
	return result;
}

btVector3 physVec3ToBTVec3(const v3 &v)
{
	btVector3 result(v.x, v.y, v.z);
	return result;
}

quat physBTQuatToQuat(const btQuaternion &q)
{
	quat result;
	btVector3 axis = q.getAxis();
	result = CreateQuatRAD(q.getAngle(), V3(axis.getX(), axis.getY(), axis.getZ()));
	return result;
}

v3 physBTVec3ToVec3(const btVector3 &v)
{
	v3 result;
	result.x = v.getX();
	result.y = v.getY();
	result.z = v.getZ();
	return result;
}

void physConvertBTTrans(const btTransform &t, quat *q, v3 *v)
{
	btVector3 origin = t.getOrigin();
	*v = physBTVec3ToVec3(origin);
	btQuaternion rotation = t.getRotation();
	*q = physBTQuatToQuat(rotation);
}

void physAddRigidBodyToWorld(physics_world *world, btRigidBody *rigidBody)
{
	world->dynamicsWorld->addRigidBody(rigidBody);
}

void physRemoveRigidBodyToWorld(physics_world *world, btRigidBody *rigidBody)
{
	world->dynamicsWorld->removeRigidBody(rigidBody);
}
