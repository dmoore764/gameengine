#pragma once
#include "game_math.h"
#include "opengl.h"
#include "memory_arena.h"
#include "collada.h"

struct basic_mesh
{
	vertex_array_object mesh;
};

struct bone_keyframe
{
	float timeStamp;
	v3 *transforms;
	quat *quaternions;
};

struct bone_animation
{
	float timeInSeconds;
	int numFrames;
	bone_keyframe *keyframes;
};

struct bone_joint
{
	char *name;
	m4 inverseBindTransform;

	int numChildren;
	int *childrenIndices;
};

#define MAX_JOINTS 100
struct bone_model
{
	vertex_array_object mesh;
	int numJoints;
	bone_joint joints[MAX_JOINTS];
	int rootIndex;
	bone_animation animation;
};

struct bone_model_instance
{
	bone_model *model;
	float currentTime;
	m4 *boneTransforms;
	m4 *boneTransformsNoInverseBind;
};


basic_mesh BasicMeshGetFromColladaByName(collada_file *file, const char *name, memory_arena *arena);
int BoneModelSetJointChildren(bone_joint *joints, int numJoints, collada_node *node, memory_arena *arena);
bone_model BoneModelGetFromColladaByName(collada_file *file, const char *name, memory_arena *arena);
bone_model_instance BoneModelInstanceNew(bone_model *model);
void BoneModelInstanceApplyPoseToJoint(m4 *boneTransforms, m4 *boneTransformsNoInverseBind, m4 *currentPose, bone_joint *joints, int jointIndex, m4 parentTransform);
void BoneModelInstanceUpdateTransforms(bone_model_instance *instance);
