#include "game_world.h"
#include "compiler_helper.h"
#include "memory.h"
#include "assets.h"

object3d *object3d_list::operator[](int i)
{
	assert(i >= 0 && i < ArrayCount(objects));
	return &objects[i];
}

int gwNewObject3DHandle(game_world *game, object3d **obj)
{
	int result = -1;
	for (int i = 0; i < ArrayCount(game->o3ds.objects); i++)
	{
		if (!(game->o3ds.objects[i].flags & O3D_IN_USE))
		{
			result = i;
			if (i > game->o3ds.maxNumObjects - 1)
				game->o3ds.maxNumObjects = i+1;

			object3d *newObj = game->o3ds[i];
			memset(newObj, '\0', sizeof(object3d));
			newObj->rotation.w = 1;
			*obj = newObj;

			newObj->flags = O3D_IN_USE | O3D_ACTIVE | O3D_VISIBLE;

			break;
		}
	}
	return result;
}

void gwRemovePhysicsFromObject(game_world *game, object3d *obj)
{
	if (obj->physics.rigidBody)
	{
		physRemoveRigidBodyToWorld(&game->physWorld, obj->physics.rigidBody);
	}
	o3dRemovePhysics(obj);
}

void gwDeleteObject3D(game_world *game, int objHandle)
{
	object3d *obj = game->o3ds[objHandle];
	if (obj->flags & O3D_IN_USE)
	{
		gwRemovePhysicsFromObject(game, obj);
		memset(obj, '\0', sizeof(object3d));
		if (game->o3ds.maxNumObjects - 1 == objHandle)
		{
			game->o3ds.maxNumObjects = 0;
			for (int i = objHandle - 1; i >= 0; i--)
			{
				if (game->o3ds[i]->flags & O3D_IN_USE)
				{
					game->o3ds.maxNumObjects = i + 1;
					break;
				}
			}
		}
	}
}

void gwClearObjects(game_world *game)
{
	for (int i = 0; i < game->o3ds.maxNumObjects; i++)
	{
		gwDeleteObject3D(game, i);
	}
}

void gwInit(game_world *game, memory_arena *arena)
{
	memset(&game->o3ds, '\0', sizeof(object3d_list));
	physInitWorld(&game->physWorld);
	assLoad(&game->ass, arena);
}

void gwClose(game_world *game)
{
	gwClearObjects(game);
	physCloseWorld(&game->physWorld);
}
