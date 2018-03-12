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
}
