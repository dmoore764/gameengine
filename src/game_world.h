#pragma once

#include "object3d.h"
#include "physics.h"
#include "assets.h"
#include "memory_arena.h"

struct object3d_list
{
	int maxNumObjects;
	object3d objects[1000];

	object3d *operator[](int i);
};

struct game_world
{
	assets ass;
	object3d_list o3ds;
	physics_world physWorld;
};

int gwNewObject3DHandle(game_world *game, object3d **obj);
void gwRemovePhysicsFromObject(game_world *game, object3d *obj);
void gwDeleteObject3D(game_world *game, int objHandle);
void gwClearObjects(game_world *game);
void gwInit(game_world *game, memory_arena *arena);
void gwClose(game_world *game);
