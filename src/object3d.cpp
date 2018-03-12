#include "object3d.h"
#include "compiler_helper.h"
#include "string.h"

void o3dUpdate(object3d *obj, float deltaT)
{
	switch (obj->type)
	{
		case object3d_types::NON_VISUAL:
		{
		} break;

		case object3d_types::BONE_MODEL:
		{
			bone_model_instance *animated_model = &obj->boneModelInstance;
			animated_model->currentTime += deltaT;
			if (animated_model->currentTime > animated_model->model->animation.keyframes[animated_model->model->animation.numFrames-1].timeStamp)
				animated_model->currentTime = animated_model->model->animation.keyframes[0].timeStamp;
			BoneModelInstanceUpdateTransforms(animated_model);
		} break;

		case object3d_types::BASIC_GEOMETRY:
		{
		} break;
	}

	switch (obj->physicsType)
	{
		case object3d_physics::NONE:
		{
		} break;

		case object3d_physics::DYNAMIC:
		{
			btTransform trans;
			obj->physics.rigidBody->getMotionState()->getWorldTransform(trans);
			physConvertBTTrans(trans, &obj->rotation, &obj->position);
			obj->position = obj->position - RotateByQuat(obj->physics.offset, obj->rotation);
		} break;
		
		case object3d_physics::KINETIC:
		{
			//update the physics system here?
		} break;
	}
}

void o3dCalculateModelMat(object3d *obj)
{
	obj->modelMat = RotationPositionToM4(obj->rotation, obj->position);
	/*obj->modelMat = QuatToM4(obj->rotation);
	obj->modelMat.col[3].x = obj->position.x;
	obj->modelMat.col[3].y = obj->position.y;
	obj->modelMat.col[3].z = obj->position.z;*/
}

void o3dSetModelMatToBone(object3d *obj, object3d *target, const char *boneName)
{
	assert(target->type == object3d_types::BONE_MODEL);
	bone_model *model = target->boneModelInstance.model;
	int jointIndex = -1;
	for (int i = 0; i < model->numJoints; i++)
	{
		if (strcmp(model->joints[i].name, boneName) == 0)
		{
			jointIndex = i;
			break;
		}
	}
	obj->modelMat = target->modelMat * target->boneModelInstance.boneTransformsNoInverseBind[jointIndex];
}

void o3dSetDynamicShape(object3d *obj, physics_shapes shape, float mass, float restitution, v3 dimensions, v3 offset)
{
	obj->physicsType = object3d_physics::DYNAMIC;
	obj->physics.offset = offset;
	btVector3 inertia(0,0,0);
	switch (shape)
	{
		case SHAPE_PLANE_X:
		{
			obj->physics.shape = new btStaticPlaneShape(btVector3(1,0,0), 0);
		} break;
		case SHAPE_PLANE_Y:
		{
			obj->physics.shape = new btStaticPlaneShape(btVector3(0,1,0), 0);
			offset.y = offset.x;
			offset.x = 0;
		} break;
		case SHAPE_PLANE_Z:
		{
			obj->physics.shape = new btStaticPlaneShape(btVector3(0,0,1), 0);
			offset.z = offset.x;
			offset.x = 0;
		} break;

		case SHAPE_CAPSULE_X:
		{
			obj->physics.shape = new btCapsuleShapeX(dimensions.x, dimensions.y);
			obj->physics.shape->calculateLocalInertia(mass, inertia);
		} break;
		case SHAPE_CAPSULE_Y:
		{
			obj->physics.shape = new btCapsuleShape(dimensions.x, dimensions.y);
			obj->physics.shape->calculateLocalInertia(mass, inertia);
		} break;
		case SHAPE_CAPSULE_Z:
		{
			obj->physics.shape = new btCapsuleShapeZ(dimensions.x, dimensions.y);
			obj->physics.shape->calculateLocalInertia(mass, inertia);
		} break;

		case SHAPE_CONE_X:
		{
			obj->physics.shape = new btConeShapeX(dimensions.x, dimensions.y);
			obj->physics.shape->calculateLocalInertia(mass, inertia);
		} break;
		case SHAPE_CONE_Y:
		{
			obj->physics.shape = new btConeShape(dimensions.x, dimensions.y);
			obj->physics.shape->calculateLocalInertia(mass, inertia);
		} break;
		case SHAPE_CONE_Z:
		{
			obj->physics.shape = new btConeShapeZ(dimensions.x, dimensions.y);
			obj->physics.shape->calculateLocalInertia(mass, inertia);
		} break;

		case SHAPE_SPHERE:
		{
			obj->physics.shape = new btSphereShape(dimensions.x);
			obj->physics.shape->calculateLocalInertia(mass, inertia);
		} break;

		case SHAPE_CYLINDER_X:
		{
			btVector3 halfExtents = physVec3ToBTVec3(V3(dimensions.y, dimensions.x, dimensions.x));
			obj->physics.shape = new btCylinderShapeX(halfExtents);
			obj->physics.shape->calculateLocalInertia(mass, inertia);
		} break;
		case SHAPE_CYLINDER_Y:
		{
			btVector3 halfExtents = physVec3ToBTVec3(V3(dimensions.x, dimensions.y, dimensions.x));
			obj->physics.shape = new btCylinderShape(halfExtents);
			obj->physics.shape->calculateLocalInertia(mass, inertia);
		} break;
		case SHAPE_CYLINDER_Z:
		{
			btVector3 halfExtents = physVec3ToBTVec3(V3(dimensions.x, dimensions.x, dimensions.y));
			obj->physics.shape = new btCylinderShapeZ(halfExtents);
			obj->physics.shape->calculateLocalInertia(mass, inertia);
		} break;

		case SHAPE_BOX:
		{
			btVector3 halfExtents = physVec3ToBTVec3(dimensions);
			obj->physics.shape = new btBoxShape(halfExtents);
			obj->physics.shape->calculateLocalInertia(mass, inertia);
		} break;

		default: break;
	}

	obj->physics.motionState = new btDefaultMotionState(btTransform(physQuatToBTQuat(obj->rotation), physVec3ToBTVec3(obj->position + RotateByQuat(offset, obj->rotation))));
	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass, obj->physics.motionState, obj->physics.shape, inertia);
	rigidBodyCI.m_restitution = restitution;
	rigidBodyCI.m_friction = 0.1f;
	rigidBodyCI.m_rollingFriction = 0.1f;
	obj->physics.rigidBody = new btRigidBody(rigidBodyCI);
}

void o3dRemovePhysics(object3d *obj)
{
	switch (obj->physicsType)
	{
		case object3d_physics::NONE:
		{
		} break;

		case object3d_physics::DYNAMIC:
		{
			delete obj->physics.rigidBody;
			delete obj->physics.motionState;
			delete obj->physics.shape;
		} break;

		case object3d_physics::KINETIC:
		{
			//whatever is necessary for a kinetic object
		} break;
	}

	obj->physics.rigidBody = NULL;
	obj->physics.motionState = NULL;
	obj->physics.shape = NULL;
	obj->physicsType = object3d_physics::NONE;
}
