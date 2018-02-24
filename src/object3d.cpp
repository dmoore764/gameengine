#include "object3d.h"
#include "compiler_helper.h"
#include "string.h"

void Object3DUpdate(object3d *obj, float deltaT)
{
	switch (obj->type)
	{
		case OBJECT3D_BONE_MODEL:
		{
			bone_model_instance *animated_model = &obj->boneModelInstance;
			animated_model->currentTime += deltaT;
			if (animated_model->currentTime > animated_model->model->animation.keyframes[animated_model->model->animation.numFrames-1].timeStamp)
				animated_model->currentTime = animated_model->model->animation.keyframes[0].timeStamp;
			BoneModelInstanceUpdateTransforms(animated_model);
		} break;

		case OBJECT3D_BASIC_GEOMETRY:
		{
		} break;
	}
}

void Object3DCalculateModelMat(object3d *obj)
{
	obj->modelMat = QuatToM4(obj->rotation);
	obj->modelMat.col[3].x = obj->position.x;
	obj->modelMat.col[3].y = obj->position.y;
	obj->modelMat.col[3].z = obj->position.z;
}

void Object3DSetModelMatToBone(object3d *obj, object3d *target, const char *boneName)
{
	assert(target->type == OBJECT3D_BONE_MODEL);
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
