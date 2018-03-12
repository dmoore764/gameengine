#include "mesh.h"
#include "string.h"
#include "compiler_helper.h"
#include "stdio.h"

basic_mesh BasicMeshGetFromColladaByName(collada_file *file, const char *name, memory_arena *arena)
{
	basic_mesh result;
	collada_node *instance = ColladaGetSceneObjectByName(&file->visualScenes[0], name);
	assert(instance);
	assert(instance->contentType == COLLADA_INSTANCE_GEOMETRY);

	collada_geometry *geometry = NULL;
	for (int i = 0; i < file->numGeometries; i++)
	{
		if (strcmp(file->geometries[i].id, instance->instanceGeometry.meshURL) == 0)
		{
			geometry = &file->geometries[i];
			break;
		}
	}
	assert(geometry);

	basic_vertex *verts = (basic_vertex *)calloc(sizeof(basic_vertex) * geometry->numPolys * 3, 1);

	int currentVert = 0;
	for (int i = 0; i < geometry->numPolys; i++)
	{
		for (int v = 0; v < 3; v++)
		{
			basic_vertex *cur = &verts[currentVert++];
			cur->color = MAKE_COLOR(255, 255, 255, 255);
			for (int s = 0; s < geometry->numSemantics; s++)
			{
				if (geometry->semantics[s].semantic[0] == 'V')
				{
					int index = geometry->polys[i].vert[v].position_index;
					collada_source_array *source = &geometry->sources[geometry->semantics[s].source_index];
					cur->position.x = source->values[index*source->stride + 0];
					cur->position.y = source->values[index*source->stride + 1];
					cur->position.z = source->values[index*source->stride + 2];
				}
				else if (geometry->semantics[s].semantic[0] == 'N') //Normal
				{
					int index = geometry->polys[i].vert[v].normal_index;
					collada_source_array *source = &geometry->sources[geometry->semantics[s].source_index];
					cur->normal.x = source->values[index*source->stride + 0];
					cur->normal.y = source->values[index*source->stride + 1];
					cur->normal.z = source->values[index*source->stride + 2];
				}
				else if (geometry->semantics[s].semantic[0] == 'T') //Texcoord
				{
					if (geometry->semantics[s].set == 0)
					{
						int index = geometry->polys[i].vert[v].map_index[0];
						collada_source_array *source = &geometry->sources[geometry->semantics[s].source_index];
						cur->uv.x = source->values[index*source->stride + 0];
						cur->uv.y = 1 - source->values[index*source->stride + 1];
					}
				}
				else if (geometry->semantics[s].semantic[0] == 'C') //Color
				{
					if (geometry->semantics[s].set == 0)
					{
						int index = geometry->polys[i].vert[v].color_index[0];
						collada_source_array *source = &geometry->sources[geometry->semantics[s].source_index];
						float r = source->values[index*source->stride + 0];
						float g = source->values[index*source->stride + 1];
						float b = source->values[index*source->stride + 2];
						cur->color = MAKE_COLOR((uint8_t)(r * 255.0f), (uint8_t)(g * 255.0f), (uint8_t)(b * 255.0f), 255);
					}
				}
			}
		}
	}

	int *originalIndices = (int *)alloca(sizeof(int)*currentVert);
	for (int i = 0; i < currentVert; i++)
		originalIndices[i] = -1;
	int *remappedIndices = (int *)alloca(sizeof(int)*currentVert);

	int *indexData = (int *)alloca(sizeof(int)*currentVert);

	//check for duplicate vertices
	int currentUnique = 0;
	for (int i = 0; i < currentVert; i++)
	{
		if (originalIndices[i] != -1) //don't look for duplicates of a duplicate
			continue;

		//map the first occurance of a vertex to the next slot
		remappedIndices[i] = currentUnique;

		//then map all the next occurances to the same slot, and note the original vertex
		for (int j = i + 1; j < currentVert; j++)
		{
			if (originalIndices[j] != -1) //if we already found a duplicate, skip
				continue;

			basic_vertex *vA = &verts[i];
			basic_vertex *vB = &verts[j];

			if (memcmp(vA, vB, sizeof(skeletal_vertex)) == 0)
			{
				originalIndices[j] = i;
				remappedIndices[j] = currentUnique;
			}
		}

		currentUnique++;
	}

	//create the index buffer
	for (int i = 0; i < currentVert; i++)
	{
		indexData[i] = remappedIndices[i];
	}

	//create the new vertex buffer
	basic_vertex *newVerts = (basic_vertex *)malloc(sizeof(basic_vertex)*currentUnique);

	for (int i = 0; i < currentVert; i++)
	{
		if (originalIndices[i] == -1)
		{
			newVerts[remappedIndices[i]] = verts[i];
		}
	}
	free(verts);

	oglCreateVAOWithData(&result.mesh, BASIC_VERTEX, currentUnique, newVerts, currentVert, indexData);
	free(newVerts);

	return result;
}

int BoneModelSetJointChildren(bone_joint *joints, int numJoints, collada_node *node, memory_arena *arena)
{
	int jointId = -1;
	for (int i = 0; i < numJoints; i++)
	{
		if (strcmp(node->id, joints[i].name) == 0)
		{
			jointId = i;
			break;
		}
	}
	bone_joint *joint = &joints[jointId];
	joint->numChildren = 0;
	for (int i = 0; i < node->numNodes; i++)
	{
		if (node->nodes[i].contentType == COLLADA_GENERIC)
			joint->numChildren++;
	}
	joint->childrenIndices = PUSH_ARRAY(arena, int, joint->numChildren);

	int currentChild = 0;
	for (int i = 0; i < node->numNodes; i++)
	{
		if (node->nodes[i].contentType == COLLADA_GENERIC)
		{
			joint->childrenIndices[currentChild++] = BoneModelSetJointChildren(joints, numJoints, &node->nodes[i], arena);
		}
	}

	return jointId;
}

bone_model BoneModelGetFromColladaByName(collada_file *file, const char *name, memory_arena *arena)
{
	bone_model result;
	collada_node *instance = ColladaGetSceneObjectByName(&file->visualScenes[0], name);
	assert(instance); //assert for input correctness
	assert(instance->contentType == COLLADA_INSTANCE_CONTROLLER);

	collada_controller *controller = NULL;
	for (int i = 0; i < file->numControllers; i++)
	{
		if (strcmp(file->controllers[i].id, instance->instanceController.controllerURL) == 0)
		{
			controller = &file->controllers[i];
			break;
		}
	}
	assert(controller); //assert for api/collada file correctness

	m4 bindShapeMatrix;
	if (controller->hasBindShapeMatrix)
	{
		bindShapeMatrix = Transpose(M4(controller->bindShapeMatrix));
	}
	else
	{
		bindShapeMatrix = M4();
	}

	collada_geometry *geometry = NULL;
	for (int i = 0; i < file->numGeometries; i++)
	{
		if (strcmp(file->geometries[i].id, controller->forMesh) == 0)
		{
			geometry = &file->geometries[i];
			break;
		}
	}
	assert(geometry); //same as for controller

	skeletal_vertex *verts = (skeletal_vertex *)calloc(sizeof(skeletal_vertex) * geometry->numPolys * 3, 1);

	int currentVert = 0;
	for (int i = 0; i < geometry->numPolys; i++)
	{
		for (int v = 0; v < 3; v++)
		{
			skeletal_vertex *cur = &verts[currentVert++];
			cur->color = MAKE_COLOR(205,55,155,255);
			for (int s = 0; s < geometry->numSemantics; s++)
			{
				if (geometry->semantics[s].semantic[0] == 'V')
				{
					int index = geometry->polys[i].vert[v].position_index;
					collada_source_array *source = &geometry->sources[geometry->semantics[s].source_index];

					v4 pos;
					pos.x = source->values[index*source->stride + 0];
					pos.y = source->values[index*source->stride + 1];
					pos.z = source->values[index*source->stride + 2];
					pos.w = 1.0f;

					//pos = bindShapeMatrix * pos;

					cur->position.x = pos.x;//source->values[index*source->stride + 0];
					cur->position.y = pos.y;//source->values[index*source->stride + 1];
					cur->position.z = pos.z;//source->values[index*source->stride + 2];

					//use this index to get the joint weight?
					collada_vertex_weight *vertWeight = &controller->vertWeights[index];
					for (int w = 0; w < vertWeight->numWeights; w++)
					{
						collada_joint_weight *jw = &vertWeight->weights[w];
						//keep top three weights
						if (jw->weight > cur->weights[0])
						{
							cur->weights[2] = cur->weights[1];
							cur->weights[1] = cur->weights[0];
							cur->jointIndices[2] = cur->jointIndices[1];
							cur->jointIndices[1] = cur->jointIndices[0];
							cur->weights[0] = jw->weight;
							cur->jointIndices[0] = jw->joint_index;
						}
						else if (jw->weight > cur->weights[1])
						{
							cur->weights[2] = cur->weights[1];
							cur->jointIndices[2] = cur->jointIndices[1];
							cur->weights[1] = jw->weight;
							cur->jointIndices[1] = jw->joint_index;
						}
						else if (jw->weight > cur->weights[2])
						{
							cur->weights[2] = jw->weight;
							cur->jointIndices[2] = jw->joint_index;
						}
					}
					float total = cur->weights[0] + cur->weights[1] + cur->weights[2];
					cur->weights[0] /= total;
					cur->weights[1] /= total;
					cur->weights[2] /= total;
				}
				else if (geometry->semantics[s].semantic[0] == 'N') //Normal
				{
					int index = geometry->polys[i].vert[v].normal_index;
					collada_source_array *source = &geometry->sources[geometry->semantics[s].source_index];
					cur->normal.x = source->values[index*source->stride + 0];
					cur->normal.y = source->values[index*source->stride + 1];
					cur->normal.z = source->values[index*source->stride + 2];
				}
				else if (geometry->semantics[s].semantic[0] == 'T') //Texcoord
				{
					if (geometry->semantics[s].set == 0)
					{
						int index = geometry->polys[i].vert[v].map_index[0];
						collada_source_array *source = &geometry->sources[geometry->semantics[s].source_index];
						cur->uv.x = source->values[index*source->stride + 0];
						cur->uv.y = 1 - source->values[index*source->stride + 1];
					}
				}
			}
		}
	}

	int *originalIndices = (int *)alloca(sizeof(int)*currentVert);
	for (int i = 0; i < currentVert; i++)
		originalIndices[i] = -1;
	int *remappedIndices = (int *)alloca(sizeof(int)*currentVert);

	int *indexData = (int *)alloca(sizeof(int)*currentVert);

	//check for duplicate vertices
	int currentUnique = 0;
	for (int i = 0; i < currentVert; i++)
	{
		if (originalIndices[i] != -1) //don't look for duplicates of a duplicate
			continue;

		//map the first occurance of a vertex to the next slot
		remappedIndices[i] = currentUnique;

		//then map all the next occurances to the same slot, and note the original vertex
		for (int j = i + 1; j < currentVert; j++)
		{
			if (originalIndices[j] != -1) //if we already found a duplicate, skip
				continue;

			skeletal_vertex *vA = &verts[i];
			skeletal_vertex *vB = &verts[j];

			if (memcmp(vA, vB, sizeof(skeletal_vertex)) == 0)
			{
				originalIndices[j] = i;
				remappedIndices[j] = currentUnique;
			}
		}

		currentUnique++;
	}

	//create the index buffer
	for (int i = 0; i < currentVert; i++)
	{
		indexData[i] = remappedIndices[i];
	}

	//create the new vertex buffer
	skeletal_vertex *newVerts = (skeletal_vertex *)malloc(sizeof(skeletal_vertex)*currentUnique);

	for (int i = 0; i < currentVert; i++)
	{
		if (originalIndices[i] == -1)
		{
			newVerts[remappedIndices[i]] = verts[i];
		}
	}
	free(verts);

	oglCreateVAOWithData(&result.mesh, SKELETAL_VERTEX, currentUnique, newVerts, currentVert, indexData);
	free(newVerts);

	/*for (int i = 0; i < currentVert; i++)
	{
		skeletal_vertex *v = &verts[i];
		printf("Vert %d\n", i);
		printf("pos: %.2f, %.2f, %.2f\n", v->position.x, v->position.y, v->position.z);
		printf("normal: %.2f, %.2f, %.2f\n", v->normal.x, v->normal.y, v->normal.z);
		printf("uv: %.2f, %.2f\n", v->uv.x, v->uv.y);
		printf("weights: %.2f, %.2f, %.2f\n", v->weights[0], v->weights[1], v->weights[2]);
		printf("indices: %d, %d, %d\n", v->jointIndices[0], v->jointIndices[1], v->jointIndices[2]);
	}*/

	//Set up bones
	
	collada_source_array *joints = ColladaGetSourceArrayByParamName("JOINT", controller->sources, controller->numSources);
	assert(joints);
	collada_source_array *inverseBindTransforms = ColladaGetSourceArrayByParamName("TRANSFORM", controller->sources, controller->numSources);
	assert(inverseBindTransforms);
	result.numJoints = joints->count;

	for (int i = 0; i < result.numJoints; i++)
	{
		result.joints[i].name = PUSH_ARRAY(arena, char, strlen(joints->name_values + 128*i) + 1);
		strcpy(result.joints[i].name, joints->name_values + 128*i);
		result.joints[i].inverseBindTransform = Transpose(M4(inverseBindTransforms->values + (i*16))) * bindShapeMatrix;
	}

	char *rootBoneName = instance->instanceController.boneURL;
	collada_node *rootNode = ColladaFindNodeByID(&file->visualScenes[0], rootBoneName);
	assert(rootNode);
	result.rootIndex = BoneModelSetJointChildren(result.joints, result.numJoints, rootNode, arena);

	collada_animation *rootNodeAnimation = ColladaFindAnimationByTarget(file, rootBoneName);
	collada_source_array *rootNodeFrames = ColladaGetSourceArrayByParamName("TIME", rootNodeAnimation->sources, rootNodeAnimation->numSources);
	result.animation.numFrames = rootNodeFrames->count;
	result.animation.keyframes = PUSH_ARRAY(arena, bone_keyframe, result.animation.numFrames);

	for (int i = 0; i < result.animation.numFrames; i++)
	{
		result.animation.keyframes[i].timeStamp = rootNodeFrames->values[i];
		result.animation.keyframes[i].transforms = PUSH_ARRAY(arena, v3, result.numJoints);
		result.animation.keyframes[i].quaternions = PUSH_ARRAY(arena, quat, result.numJoints);
	}

	for (int i = 0; i < result.numJoints; i++)
	{
		collada_animation *anim = ColladaFindAnimationByTarget(file, result.joints[i].name);
		collada_source_array *transforms = ColladaGetSourceArrayByParamName("TRANSFORM", anim->sources, anim->numSources);
		for (int f = 0; f < result.animation.numFrames; f++)
		{
			m4 boneSpaceTransform = Transpose(M4(transforms->values + (f*16)));
			result.animation.keyframes[f].transforms[i] = V3(boneSpaceTransform.col[3].x, boneSpaceTransform.col[3].y, boneSpaceTransform.col[3].z);
			result.animation.keyframes[f].quaternions[i] = M4ToQuat(boneSpaceTransform);
		}
	}

	return result;
}

bone_model_instance BoneModelInstanceNew(bone_model *model)
{
	bone_model_instance result;
	result.model = model;
	result.currentTime = model->animation.keyframes[0].timeStamp;
	result.boneTransforms = (m4 *)malloc(sizeof(m4)*model->numJoints);
	result.boneTransformsNoInverseBind = (m4 *)malloc(sizeof(m4)*model->numJoints);
	return result;
}

void BoneModelInstanceApplyPoseToJoint(m4 *boneTransforms, m4 *boneTransformsNoInverseBind, m4 *currentPose, bone_joint *joints, int jointIndex, m4 parentTransform)
{
	bone_joint *joint = &joints[jointIndex];
	m4 currentLocalTransform = currentPose[jointIndex];
	m4 currentTransform = parentTransform*currentLocalTransform;
	for (int i = 0; i < joint->numChildren; i++)
	{
		BoneModelInstanceApplyPoseToJoint(boneTransforms, boneTransformsNoInverseBind, currentPose, joints, joint->childrenIndices[i], currentTransform);
	}
	boneTransformsNoInverseBind[jointIndex] = currentTransform;
	boneTransforms[jointIndex] = currentTransform*joint->inverseBindTransform;
}

void BoneModelInstanceUpdateTransforms(bone_model_instance *instance)
{
	m4 *currentPose = (m4 *)alloca(sizeof(m4)*instance->model->numJoints);
	bone_keyframe *keyframe0 = &instance->model->animation.keyframes[0];
	bone_keyframe *keyframe1 = &instance->model->animation.keyframes[instance->model->animation.numFrames-1];
	for (int i = 0; i < instance->model->animation.numFrames; i++)
	{
		if (instance->currentTime >= instance->model->animation.keyframes[i].timeStamp)
		{
			keyframe0 = &instance->model->animation.keyframes[i];
			if (i < instance->model->animation.numFrames - 1)
				keyframe1 = &instance->model->animation.keyframes[i+1];
		}
	}
	float progression = (instance->currentTime - keyframe0->timeStamp) / (keyframe1->timeStamp - keyframe0->timeStamp);

	for (int i = 0; i < instance->model->numJoints; i++)
	{
		bone_joint *joint = &instance->model->joints[i];
		if (progression < 0.01f)
		{
			currentPose[i] = QuatToM4(keyframe0->quaternions[i]);
			currentPose[i].col[3].x = keyframe0->transforms[i].x;
			currentPose[i].col[3].y = keyframe0->transforms[i].y;
			currentPose[i].col[3].z = keyframe0->transforms[i].z;
		}
		else if (progression > 0.99f)
		{
			currentPose[i] = QuatToM4(keyframe1->quaternions[i]);
			currentPose[i].col[3].x = keyframe1->transforms[i].x;
			currentPose[i].col[3].y = keyframe1->transforms[i].y;
			currentPose[i].col[3].z = keyframe1->transforms[i].z;
		}
		else
		{
			quat interpolatedQuat = Nlerp(keyframe0->quaternions[i], keyframe1->quaternions[i], progression);
			currentPose[i] = QuatToM4(interpolatedQuat);
			currentPose[i].col[3].x = Lerp(keyframe0->transforms[i].x, keyframe1->transforms[i].x, progression);
			currentPose[i].col[3].y = Lerp(keyframe0->transforms[i].y, keyframe1->transforms[i].y, progression);
			currentPose[i].col[3].z = Lerp(keyframe0->transforms[i].z, keyframe1->transforms[i].z, progression);
		}
	}

	BoneModelInstanceApplyPoseToJoint(instance->boneTransforms, instance->boneTransformsNoInverseBind, currentPose, instance->model->joints, instance->model->rootIndex, M4());
}
