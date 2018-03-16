#include "assets.h"
#include "collada.h"

void assLoad(assets *ass, memory_arena *arena)
{
	InitHash(&ass->boneModelHash, 64, arena);
	InitHash(&ass->basicMeshHash, 64, arena);

	collada_file bonetest = ReadColladaFile("../assets/meshes/bonetest.dae", arena);
	ass->worm = BoneModelGetFromColladaByName(&bonetest, "Worm", arena);
	AddToHash(&ass->boneModelHash, &ass->worm, "testAssetWorm");
	ass->sphere = BasicMeshGetFromColladaByName(&bonetest, "Sphere", arena);
	AddToHash(&ass->basicMeshHash, &ass->sphere, "testAssetSphere");

	collada_file axisplanes = ReadColladaFile("../assets/meshes/axisplanes.dae", arena);
	ass->xaxis = BasicMeshGetFromColladaByName(&axisplanes, "XAxis", arena);
	AddToHash(&ass->basicMeshHash, &ass->xaxis, "editorAssetXAxis");
	ass->yaxis = BasicMeshGetFromColladaByName(&axisplanes, "YAxis", arena);
	AddToHash(&ass->basicMeshHash, &ass->yaxis, "editorAssetYAxis");
	ass->zaxis = BasicMeshGetFromColladaByName(&axisplanes, "ZAxis", arena);
	AddToHash(&ass->basicMeshHash, &ass->zaxis, "editorAssetZAxis");

	collada_file physicsshapes = ReadColladaFile("../assets/meshes/physicsshapes.dae", arena);
	ass->physShapeRectangularPrism = BasicMeshGetFromColladaByName(&physicsshapes, "RectangularPrism", arena);
	AddToHash(&ass->basicMeshHash, &ass->physShapeRectangularPrism, "physShapeRectangularPrism");
	ass->physShapeSphere = BasicMeshGetFromColladaByName(&physicsshapes, "Sphere", arena);
	AddToHash(&ass->basicMeshHash, &ass->physShapeSphere, "physShapeSphere");
	ass->physShapeCone = BasicMeshGetFromColladaByName(&physicsshapes, "Cone", arena);
	AddToHash(&ass->basicMeshHash, &ass->physShapeCone, "physShapeCone");
	ass->physShapeCylinder = BasicMeshGetFromColladaByName(&physicsshapes, "Cylinder", arena);
	AddToHash(&ass->basicMeshHash, &ass->physShapeCylinder, "physShapeCylinder");
	ass->physShapeCapsuleBottom = BasicMeshGetFromColladaByName(&physicsshapes, "CapsuleBottom", arena);
	AddToHash(&ass->basicMeshHash, &ass->physShapeCapsuleBottom, "physShapeCapsuleBottom");
	ass->physShapeCapsuleMiddle = BasicMeshGetFromColladaByName(&physicsshapes, "CapsuleMiddle", arena);
	AddToHash(&ass->basicMeshHash, &ass->physShapeCapsuleMiddle, "physShapeCapsuleMiddle");
	ass->physShapeCapsuleTop = BasicMeshGetFromColladaByName(&physicsshapes, "CapsuleTop", arena);
	AddToHash(&ass->basicMeshHash, &ass->physShapeCapsuleTop, "physShapeCapsuleTop");
	ass->physShapePlane = BasicMeshGetFromColladaByName(&physicsshapes, "Plane", arena);
	AddToHash(&ass->basicMeshHash, &ass->physShapePlane, "physShapePlane");

	collada_file manipulator = ReadColladaFile("../assets/meshes/manipulator.dae", arena);
	ass->manipulatorX = BasicMeshGetFromColladaByName(&manipulator, "XAxis", arena);
	AddToHash(&ass->basicMeshHash, &ass->manipulatorX, "editorAssetManipulatorX");
	ass->manipulatorY = BasicMeshGetFromColladaByName(&manipulator, "YAxis", arena);
	AddToHash(&ass->basicMeshHash, &ass->manipulatorY, "editorAssetManipulatorY");
	ass->manipulatorZ = BasicMeshGetFromColladaByName(&manipulator, "ZAxis", arena);
	AddToHash(&ass->basicMeshHash, &ass->manipulatorZ, "editorAssetManipulatorZ");

	collada_file objectmarker = ReadColladaFile("../assets/meshes/nonvisualobject.dae", arena);
	ass->objmarker = BasicMeshGetFromColladaByName(&objectmarker, "Cube", arena);
	AddToHash(&ass->basicMeshHash, &ass->objmarker, "editorAssetObjectMarker");

	//Shader loading
	oglLoadShader(&ass->basicMeshColoredVerts, "../shaders/basic_mesh_colored_verts_vert.glsl", "../shaders/basic_mesh_colored_verts_frag.glsl");
	ass->basicMeshColoredVerts.vertexAttributes = ATTR_POSITION | ATTR_COLOR;

	oglLoadShader(&ass->hiddenMeshColoredVerts, "../shaders/hidden_mesh_colored_verts_vert.glsl", "../shaders/hidden_mesh_colored_verts_frag.glsl");
	ass->hiddenMeshColoredVerts.vertexAttributes = ATTR_POSITION | ATTR_COLOR;

	oglLoadShader(&ass->texturedTriangles, "../shaders/textured_vert.glsl", "../shaders/textured_frag.glsl");
	ass->texturedTriangles.vertexAttributes = ATTR_POSITION | ATTR_UV | ATTR_COLOR;

	oglLoadShader(&ass->basicMeshSolidColor, "../shaders/basic_mesh_solid_color_vert.glsl", "../shaders/basic_mesh_solid_color_frag.glsl");
	ass->basicMeshSolidColor.vertexAttributes = ATTR_POSITION;

	oglLoadShader(&ass->outlineDrawer, "../shaders/outline_drawer_vert.glsl", "../shaders/outline_drawer_frag.glsl");
	ass->outlineDrawer.vertexAttributes = ATTR_POSITION | ATTR_UV;

	oglLoadShader(&ass->boneModel, "../shaders/bone_model_vert.glsl", "../shaders/bone_model_frag.glsl");
	ass->boneModel.vertexAttributes = ATTR_POSITION | ATTR_NORMAL | ATTR_UV | ATTR_COLOR | ATTR_JOINT_WEIGHTS | ATTR_JOINT_INDICES;


	//Texture specification
	ass->uvTex.fileName = "../assets/textures/UVMap.png";
	ass->uvTex.flags = MAG_FILTERING_NEAREST | MIN_FILTERING_MIPMAP_LINEAR | CLAMP_TO_EDGE_S | CLAMP_TO_EDGE_T | TEX_2D;
	ass->uvTex.loadFlags = TEX_FROM_FILE | TEX_SHOULD_GEN_MIPMAPS;
}
