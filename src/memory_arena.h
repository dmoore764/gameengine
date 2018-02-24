#pragma once
#include "stddef.h"

struct memory_arena
{
	void *memory;
	void *current;
	size_t size;
};

struct transient_arena
{
	memory_arena *arena;
	void *current;
};

void InitArena(memory_arena *arena, size_t size);
transient_arena GetTransientArena(memory_arena *arena);
void FreeTransientArena(transient_arena ta);

void *_PushData(memory_arena *arena, size_t size);

#define PUSH_ITEM(Arena, Type) (Type *)_PushData(Arena, sizeof(Type))
#define PUSH_ARRAY(Arena, Type, Count) (Type *)_PushData(Arena, sizeof(Type)*Count)

