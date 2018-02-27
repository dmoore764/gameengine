#include "memory_arena.h"
#include "stdlib.h"
#include "compiler_helper.h"

void InitArena(memory_arena *arena, size_t size)
{
	arena->size = size;
	arena->memory = calloc(size, 1);
	uint32_t *data = (uint32_t *)arena->memory;
	for (int i = 0; i < size >> 2; i++)
	{
		*data = 0xdeadbeef;
		data++;
	}
	
	//align to 4 bytes
	size_t addr = (size_t)arena->memory + 32;
	arena->current = (void *)(addr - (addr % 32));
}

transient_arena GetTransientArena(memory_arena *arena)
{
	transient_arena result;
	result.arena = arena;
	result.current = arena->current;
	return result;
}

void FreeTransientArena(transient_arena ta)
{
	ta.arena->current = ta.current;
}

void *_PushData(memory_arena *arena, size_t size)
{
	assert((char *)arena->current + size + 32 < (char *)arena->memory + arena->size);
	size_t addr = (size_t)arena->current + 32;
	void *result = (void *)(addr - (addr % 32));
	arena->current = (void *)((char *)result + size);
	return result;
}
