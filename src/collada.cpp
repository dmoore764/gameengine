#include "collada.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"

collada_source_array *ColladaGetSourceArrayByParamName(const char *name, collada_source_array *sources, int numSources)
{
	collada_source_array *result = NULL;
	for (int i = 0; i < numSources; i++)
	{
		if (strcmp(sources[i].params[0].name, name) == 0)
		{
			result = &sources[i];
			break;
		}
	}
	return result;
}

collada_source_array ColladaParseSourceArray(xmlnode *source, memory_arena *arena)
{
	collada_source_array result = {};
	result.id = XMLGetAttrByName(source, "id")->val;

	char *readPos = NULL;
	if (xmlnode *floatArray = XMLGetFirstChildWithTag(source, "float_array"))
	{
		result.type = COLLADA_FLOAT_ARRAY;
		result.count = atoi(XMLGetAttrByName(floatArray, "count")->val);
		result.values = PUSH_ARRAY(arena, float, result.count);
		readPos = floatArray->value;
	}
	else if (xmlnode *nameArray = XMLGetFirstChildWithTag(source, "Name_array"))
	{
		result.type = COLLADA_NAME_ARRAY;
		result.count = atoi(XMLGetAttrByName(nameArray, "count")->val);
		result.name_values = PUSH_ARRAY(arena, char, result.count*128);
		readPos = nameArray->value;
	}

	for (int i = 0; i < result.count; i++)
	{
		if (readPos[0] == ' ')
			readPos++;
		switch (result.type)
		{
			case COLLADA_FLOAT_ARRAY:
			{
				result.values[i] = atof(readPos);
			} break;
			case COLLADA_NAME_ARRAY:
			{
				char *firstSpace = strchr(readPos, ' ');
				if (firstSpace)
					sprintf(result.name_values + (128*i), "%.*s", (int)(firstSpace - readPos), readPos);
				else
					strcpy(result.name_values + (128*i), readPos);
			} break;
		}
		readPos = strchr(readPos, ' ');
	}

	xmlnode *techniqueCommon = XMLGetFirstChildWithTag(source, "technique_common");
	xmlnode *accessor = XMLGetFirstChildWithTag(techniqueCommon, "accessor");
	result.stride = atoi(XMLGetAttrByName(accessor, "stride")->val);

	xmlnode *param = XMLGetFirstChildWithTag(accessor, "param");
	while (param)
	{
		collada_param *currentParam = &result.params[result.numParams++];
		currentParam->name = XMLGetAttrByName(param, "name")->val;
		currentParam->type = XMLGetAttrByName(param, "type")->val;
		param = XMLGetNextChildWithTag(param, "param");
	}

	return result;
}

collada_poly_semantic ColladaParsePolySemantic(collada_geometry geometry, xmlnode *semantic)
{
	collada_poly_semantic result = {};
	result.semantic = XMLGetAttrByName(semantic, "semantic")->val;
	result.offset = atoi(XMLGetAttrByName(semantic, "offset")->val);

	if (strcmp(result.semantic, "VERTEX") == 0)
		result.source_index = 0;
	else if (strcmp(result.semantic, "NORMAL") == 0)
		result.source_index = 1;
	else
	{
		char *sourceId = XMLGetAttrByName(semantic, "source")->val + 1;

		for (int i = 0; i < geometry.numSources; i++)
		{
			if (strcmp(geometry.sources[i].id, sourceId) == 0)
			{
				result.source_index = i;
				break;
			}
		}

		result.set = atoi(XMLGetAttrByName(semantic, "set")->val);
	}

	return result;
}

collada_geometry ColladaParseGeometry(xmlnode *current, memory_arena *arena)
{
	collada_geometry result = {};
	result.id = XMLGetAttrByName(current, "id")->val;
	result.name = XMLGetAttrByName(current, "name")->val;

	xmlnode *mesh = XMLGetFirstChildWithTag(current, "mesh");
	xmlnode *currentSource = XMLGetFirstChildWithTag(mesh, "source");
	while (currentSource)
	{
		result.sources[result.numSources++] = ColladaParseSourceArray(currentSource, arena);
		currentSource = XMLGetNextChildWithTag(currentSource, "source");
	}

	xmlnode *polylist = XMLGetFirstChildWithTag(mesh, "polylist");
	if (!polylist)
		polylist = XMLGetFirstChildWithTag(mesh, "triangles");
	result.numPolys = atoi(XMLGetAttrByName(polylist, "count")->val);
	result.polys = PUSH_ARRAY(arena, collada_poly, result.numPolys);


	xmlnode *currentSemantic = XMLGetFirstChildWithTag(polylist, "input");
	while (currentSemantic)
	{
		result.semantics[result.numSemantics++] = ColladaParsePolySemantic(result, currentSemantic);
		currentSemantic = XMLGetNextChildWithTag(currentSemantic, "input");
	}
	int highestOffset = 0; //To determine how many integers per in the 'p' stream
	for (int i = 0; i < result.numSemantics; i++)
	{
		highestOffset = Max(result.semantics[i].offset, highestOffset);
	}
	result.vertexAttributeCount = highestOffset + 1;

	xmlnode *p = XMLGetFirstChildWithTag(polylist, "p");
	char *readPos = p->value;
	for (int i = 0; i < result.numPolys; i++)
	{
		for (int v = 0; v < 3; v++)
		{
			int attributes[10];
			for (int a = 0; a < result.vertexAttributeCount; a++)
			{
				if (readPos[0] == ' ')
					readPos++;
				attributes[a] = atoi(readPos);
				readPos = strchr(readPos, ' ');
			}
			for (int s = 0; s < result.numSemantics; s++)
			{
				int val = attributes[result.semantics[s].offset];
				if (result.semantics[s].semantic[0] == 'V')
					result.polys[i].vert[v].position_index = val;
				else if (result.semantics[s].semantic[0] == 'N')
					result.polys[i].vert[v].normal_index = val;
				else if (result.semantics[s].semantic[0] == 'T')
					result.polys[i].vert[v].map_index[result.semantics[s].set] = val;
				else if (result.semantics[s].semantic[0] == 'C')
					result.polys[i].vert[v].color_index[result.semantics[s].set] = val;
			}
		}
	}
	return result;
}



collada_controller ColladaParseController(xmlnode *controller, memory_arena *arena)
{
	collada_controller result = {};

	result.id = XMLGetAttrByName(controller, "id")->val;
	result.name = XMLGetAttrByName(controller, "name")->val;

	xmlnode *skin = XMLGetFirstChildWithTag(controller, "skin");
	result.forMesh = XMLGetAttrByName(skin, "source")->val + 1;

	xmlnode *bindShapeMatrix = XMLGetFirstChildWithTag(skin, "bind_shape_matrix");
	result.hasBindShapeMatrix = bindShapeMatrix;
	if (bindShapeMatrix)
	{
		char *startPos = bindShapeMatrix->value;
		for (int i = 0; i < 16; i++)
		{
			result.bindShapeMatrix[i] = atof(startPos);
			startPos = strchr(startPos, ' ');
			startPos++;
		}
	}

	xmlnode *currentSource = XMLGetFirstChildWithTag(skin, "source");
	while (currentSource)
	{
		result.sources[result.numSources++] = ColladaParseSourceArray(currentSource, arena);
		currentSource = XMLGetNextChildWithTag(currentSource, "source");
	}

	xmlnode *vertex_weights = XMLGetFirstChildWithTag(skin, "vertex_weights");
	result.numVerts = atoi(XMLGetAttrByName(vertex_weights, "count")->val);
	result.vertWeights = PUSH_ARRAY(arena, collada_vertex_weight, result.numVerts);

	xmlnode *vcount = XMLGetFirstChildWithTag(vertex_weights, "vcount");
	char *readPos = vcount->value;
	for (int i = 0; i < result.numVerts; i++)
	{
		if (readPos[0] == ' ')
			readPos++;
		result.vertWeights[i].numWeights = atoi(readPos);
		result.vertWeights[i].weights = PUSH_ARRAY(arena, collada_joint_weight, result.vertWeights[i].numWeights);
		readPos = strchr(readPos, ' ');
	}

	collada_source_array *weights = ColladaGetSourceArrayByParamName("WEIGHT", result.sources, result.numSources);

	xmlnode *v = XMLGetFirstChildWithTag(vertex_weights, "v");
	readPos = v->value;
	for (int i = 0; i < result.numVerts; i++)
	{
		collada_vertex_weight *vertWeight = &result.vertWeights[i];
		for (int j = 0; j < vertWeight->numWeights; j++)
		{
			if (readPos[0] == ' ')
				readPos++;
			collada_joint_weight *jw = &vertWeight->weights[j];
			jw->joint_index = atoi(readPos);
			readPos = strchr(readPos, ' ') + 1;
			jw->weight = weights->values[atoi(readPos)];
			readPos = strchr(readPos, ' ');
		}
	}
	
	return result;
}

collada_animation ColladaParseAnimation(xmlnode *animation, memory_arena *arena)
{
	collada_animation result = {};
	result.id = XMLGetAttrByName(animation, "id")->val;
	result.numSources = XMLNumChildrenWithTag(animation, "source");
	result.sources = PUSH_ARRAY(arena, collada_source_array, result.numSources);
	xmlnode *currentSource = XMLGetFirstChildWithTag(animation, "source");
	int current = 0;
	while (currentSource)
	{
		result.sources[current++] = ColladaParseSourceArray(currentSource, arena);
		currentSource = XMLGetNextChildWithTag(currentSource, "source");
	}

	xmlnode *channel = XMLGetFirstChildWithTag(animation, "channel");
	result.target = XMLGetAttrByName(channel, "target")->val;

	return result;
}


collada_node ColladaParseNode(xmlnode *node, memory_arena *arena)
{
	collada_node result = {};
	result.id = XMLGetAttrByName(node, "id")->val;
	result.name = XMLGetAttrByName(node, "name")->val;
	char *type = XMLGetAttrByName(node, "type")->val;

	result.numNodes = XMLNumChildrenWithTag(node, "node");
	result.nodes = PUSH_ARRAY(arena, collada_node, result.numNodes);
	int current = 0;
	xmlnode *childNode = XMLGetFirstChildWithTag(node, "node");
	while (childNode)
	{
		result.nodes[current++] = ColladaParseNode(childNode, arena);
		childNode = XMLGetNextChildWithTag(childNode, "node");
	}

	if (strcmp(type, "NODE") == 0)
		result.type = COLLADA_NODE_NODE;
	else if (strcmp(type, "JOINT") == 0)
		result.type = COLLADA_NODE_JOINT;

	switch (result.type)
	{
		case COLLADA_NODE_NODE:
		{
			xmlnode *translate = XMLGetFirstChildWithTag(node, "translate");
			char *readPos = translate->value;
			result.location.x = atof(readPos);
			readPos = strchr(readPos, ' ') + 1;
			result.location.y = atof(readPos);
			readPos = strchr(readPos, ' ') + 1;
			result.location.z = atof(readPos);

			xmlnode *rotate = XMLGetFirstChildWithTag(node, "rotate");

			for (int i = 0; i < 3; i++)
			{
				if (strncmp(rotate->value, "1 0 0", 5) == 0)
					result.rotationX = atof(rotate->value + 6);
				else if (strncmp(rotate->value, "0 1 0", 5) == 0)
					result.rotationY = atof(rotate->value + 6);
				else if (strncmp(rotate->value, "0 0 1", 5) == 0)
					result.rotationZ = atof(rotate->value + 6);

				rotate = XMLGetNextChildWithTag(rotate, "rotate");
			}

			xmlnode *scale = XMLGetFirstChildWithTag(node, "scale");

			readPos = scale->value;
			result.scale.x = atof(readPos);
			readPos = strchr(readPos, ' ') + 1;
			result.scale.y = atof(readPos);
			readPos = strchr(readPos, ' ') + 1;
			result.scale.z = atof(readPos);

		} break;

		case COLLADA_NODE_JOINT:
		{

		} break;
	}

	if (xmlnode *instanceController = XMLGetFirstChildWithTag(node, "instance_controller"))
	{
		result.contentType = COLLADA_INSTANCE_CONTROLLER;
		result.instanceController.controllerURL = XMLGetAttrByName(instanceController, "url")->val + 1;
		result.instanceController.boneURL = XMLGetFirstChildWithTag(instanceController, "skeleton")->value + 1;
	}
	else if (xmlnode *instanceLight = XMLGetFirstChildWithTag(node, "instance_light"))
		result.contentType = COLLADA_INSTANCE_LIGHT;
	else if (xmlnode *instanceCamera = XMLGetFirstChildWithTag(node, "instance_camera"))
		result.contentType = COLLADA_INSTANCE_CAMERA;
	else if (xmlnode *instanceGeometry = XMLGetFirstChildWithTag(node, "instance_geometry"))
	{
		result.contentType = COLLADA_INSTANCE_GEOMETRY;
		result.instanceGeometry.meshURL = XMLGetAttrByName(instanceGeometry, "url")->val + 1;
	}
	else
		result.contentType = COLLADA_GENERIC;

	return result;
}

collada_visual_scene ColladaParseVisualScene(xmlnode *scene, memory_arena *arena)
{
	collada_visual_scene result = {};
	result.id = XMLGetAttrByName(scene, "id")->val;
	result.name = XMLGetAttrByName(scene, "name")->val;

	result.numNodes = XMLNumChildrenWithTag(scene, "node");
	result.nodes = PUSH_ARRAY(arena, collada_node, result.numNodes);
	int current = 0;
	xmlnode *node = XMLGetFirstChildWithTag(scene, "node");
	while (node)
	{
		result.nodes[current++] = ColladaParseNode(node, arena);
		node = XMLGetNextChildWithTag(node, "node");
	}

	return result;
}

collada_node *ColladaGetSceneObjectByName(collada_visual_scene *scene, const char *name)
{
	collada_node *result = NULL;
	for (int i = 0; i < scene->numNodes; i++)
	{
		collada_node *node = &scene->nodes[i];
		if (strcmp(node->name, name) == 0)
		{
			result = node;
			break;
		}
	}
	return result;
}

collada_node *ColladaFindNodeByID(collada_node *node, const char *id)
{
	collada_node *result = NULL;
	if (strcmp(node->id, id) == 0)
		result = node;
	else
	{
		for (int i = 0; i < node->numNodes; i++)
		{
			collada_node *childResult = ColladaFindNodeByID(&node->nodes[i], id);
			if (childResult)
			{
				result = childResult;
				break;
			}
		}
	}
	return result;
}

collada_node *ColladaFindNodeByID(collada_visual_scene *scene, const char *id)
{
	collada_node *result = NULL;
	for (int i = 0; i < scene->numNodes; i++)
	{
		result = ColladaFindNodeByID(&scene->nodes[i], id);
		if (result)
			break;
	}
	return result;
}

collada_animation *ColladaFindAnimationByTarget(collada_file *file, const char *target)
{
	collada_animation *result = NULL;
	char transformTarget[128];
	sprintf(transformTarget, "%s/transform", target);

	for (int i = 0; i < file->numAnimations; i++)
	{
		if (strcmp(file->animations[i].target, transformTarget) == 0)
		{
			result = &file->animations[i];
			break;
		}
	}
	return result;
}

void ColladaPrintNode(collada_node *node, int indent)
{
	char *indexSpaces = "                                                                          ";
	printf("%.*s", indent*2, indexSpaces);
	printf("%s (%s)\n", node->name, node->type == COLLADA_NODE_NODE ? "node" : "joint");
	for (int n = 0; n < node->numNodes; n++)
	{
		ColladaPrintNode(&node->nodes[n], indent+1);
	}
}

collada_file ReadColladaFile(char *fileContents, memory_arena *arena)
{
	collada_file result;

	xmltokenizer t;
	t.at = fileContents;
	xmlnode *head = XMLParseNode(&t, arena);

	xmlnode *lib_geometries = XMLGetFirstChildWithTag(head, "library_geometries");
	if (lib_geometries)
	{
		result.numGeometries = XMLNumChildrenWithTag(lib_geometries, "geometry");
		int current = 0;
		result.geometries = PUSH_ARRAY(arena, collada_geometry, result.numGeometries);
		xmlnode *currentGeometry = XMLGetFirstChildWithTag(lib_geometries, "geometry");
		while (currentGeometry)
		{
			result.geometries[current++] = ColladaParseGeometry(currentGeometry, arena);
			currentGeometry = XMLGetNextChildWithTag(currentGeometry, "geometry");
		}
	}

	xmlnode *lib_animations = XMLGetFirstChildWithTag(head, "library_animations");
	if (lib_animations)
	{
		result.numAnimations = XMLNumChildrenWithTag(lib_animations, "animation");
		int current = 0;
		result.animations = PUSH_ARRAY(arena, collada_animation, result.numAnimations);
		xmlnode *currentAnimation = XMLGetFirstChildWithTag(lib_animations, "animation");
		while (currentAnimation)
		{
			result.animations[current++] = ColladaParseAnimation(currentAnimation, arena);
			currentAnimation = XMLGetNextChildWithTag(currentAnimation, "animation");
		}
	}

	xmlnode *lib_controllers = XMLGetFirstChildWithTag(head, "library_controllers");
	if (lib_controllers)
	{
		result.numControllers = XMLNumChildrenWithTag(lib_controllers, "controller");
		int current = 0;
		result.controllers = PUSH_ARRAY(arena, collada_controller, result.numControllers);
		xmlnode *currentController = XMLGetFirstChildWithTag(lib_controllers, "controller");
		while (currentController)
		{
			result.controllers[current++] = ColladaParseController(currentController, arena);
			currentController = XMLGetNextChildWithTag(currentController, "controller");
		}
	}

	xmlnode *lib_visual_scenes = XMLGetFirstChildWithTag(head, "library_visual_scenes");
	if (lib_visual_scenes)
	{
		result.numVisualScenes = XMLNumChildrenWithTag(lib_visual_scenes, "visual_scene");
		int current = 0;
		result.visualScenes = PUSH_ARRAY(arena, collada_visual_scene, result.numVisualScenes);
		xmlnode *currentVisualScene = XMLGetFirstChildWithTag(lib_visual_scenes, "visual_scene");
		while (currentVisualScene)
		{
			result.visualScenes[current++] = ColladaParseVisualScene(currentVisualScene, arena);
			currentVisualScene = XMLGetNextChildWithTag(currentVisualScene, "visual_scene");
		}
	}

	return result;
}
