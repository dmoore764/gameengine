#include "hash.h"
#include "compiler_helper.h"
#include "string.h"

// http://www.exploringbinary.com/ten-ways-to-check-if-an-integer-is-a-power-of-two-in-c/
int IsPowerOfTwo(size_t x)
{
	return ((x != 0) && ((x & (~x + 1)) == x));
}

void InitHash(generic_hash *hash, size_t table_size, memory_arena *arena)
{
	if (!IsPowerOfTwo(table_size))
	{
		DBG("Hash tables must be a power of 2\n");
		assert(false);
	}

	hash->initialized = true;
	hash->table_size = table_size;

	if (arena)
	{
		hash->items = PUSH_ARRAY(arena, hash_item *, table_size);
		memset(hash->items, '\0', sizeof(hash_item *) * table_size);
	}
	else
		hash->items = (hash_item **)calloc(sizeof(hash_item *)*table_size, 1);
}

void _FreeHashItemsRecursive(hash_item* item) {
	if (item == NULL) {
		return;
	}

	// Recurse all the way down to the last item;
	if (item->next != NULL) {
		_FreeHashItemsRecursive(item->next);
	}

	// Delete the items on the way back up
	free(item);
}

void DestroyHash(generic_hash *hash, memory_arena *arena) {
	if (arena) {
		// ???
	} else {
		// Free items
		for (size_t i = 0; i < hash->table_size; ++i) {
			_FreeHashItemsRecursive(hash->items[i]);
		}
	}
	hash->initialized = false;
}

void *GetFromHash(generic_hash *hash, size_t index)
{
	assert(hash->initialized);
	int hash_index = index & (hash->table_size - 1);
	assert(hash_index >= 0 && hash_index < hash->table_size);

	hash_item *result = hash->items[hash_index];
	while (result)
	{
		if (result->index == index)
			return result->data;
		result = result->next;
	}
	return NULL;
}

uint32_t HashIndexForString(const char *str)
{
    uint32_t hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

void *GetFromHash(generic_hash *hash, const char *key)
{
	assert(hash->initialized);
	uint32_t hash_index = HashIndexForString(key) & (hash->table_size - 1);
	assert(hash_index >= 0 && hash_index < hash->table_size);

	hash_item *result = hash->items[hash_index];
	while (result)
	{
		if (strcmp(result->key, key) == 0)
			return result->data;
		result = result->next;
	}
	return NULL;
}

uint32_t HashIndexForIVec2(v2i location)
{
    int hash = 23;
	hash = hash * 31 + location.x;
	hash = hash * 31 + location.y;
    return hash;
}

void *GetFromHash(generic_hash *hash, v2i location)
{
	assert(hash->initialized);
	uint32_t hash_index = HashIndexForIVec2(location) & (hash->table_size - 1);
	assert(hash_index >= 0 && hash_index < hash->table_size);

	hash_item *result = hash->items[hash_index];
	while (result)
	{
		if (result->location == location)
			return result->data;
		result = result->next;
	}
	return NULL;
}

void RemoveFromHash(generic_hash *hash, size_t index)
{
	assert(hash->initialized);
	int hash_index = index & (hash->table_size - 1);
	assert(hash_index >= 0 && hash_index < hash->table_size);

	hash_item **result = &hash->items[hash_index];
	while (*result)
	{
		if ((*result)->index == index)
		{
			(*result) = ((*result)->next);
			return;
		}
		result = &((*result)->next);
	}
}

void RemoveFromHash(generic_hash *hash, const char *key)
{
	assert(hash->initialized);
	uint32_t hash_index = HashIndexForString(key) & (hash->table_size - 1);
	assert(hash_index >= 0 && hash_index < hash->table_size);

	hash_item **result = &hash->items[hash_index];
	while (*result)
	{
		if (strcmp((*result)->key, key) == 0)
		{
			(*result) = ((*result)->next);
			return;
		}
		result = &((*result)->next);
	}
}

void RemoveFromHash(generic_hash *hash, v2i location)
{
	assert(hash->initialized);
	uint32_t hash_index = HashIndexForIVec2(location) & (hash->table_size - 1);
	assert(hash_index >= 0 && hash_index < hash->table_size);

	hash_item **result = &hash->items[hash_index];
	while (*result)
	{
		if ((*result)->location == location)
		{
			(*result) = ((*result)->next);
			return;
		}
		result = &((*result)->next);
	}
}

void AddToHash(generic_hash *hash, void *data, size_t index, memory_arena *arena)
{
	assert(hash->initialized);
	hash_item *new_item;
	if (arena)
		new_item = PUSH_ITEM(arena, hash_item);
	else
		new_item = (hash_item *)calloc(sizeof(hash_item), 1);

	new_item->index = index;
	new_item->data = data;

	int hash_index = index & (hash->table_size - 1);
	assert(hash_index >= 0 && hash_index < hash->table_size);

	new_item->next = hash->items[hash_index];
	hash->items[hash_index] = new_item;
}

void AddToHash(generic_hash *hash, void *data, const char *key, memory_arena *arena)
{
	assert(hash->initialized);
	hash_item *new_item;
	if (arena)
	{
		new_item = PUSH_ITEM(arena, hash_item);
		new_item->key = PUSH_ARRAY(arena, char, strlen(key) + 1);
	}
	else
	{
		new_item = (hash_item *)calloc(sizeof(hash_item), 1);
		new_item->key = (char *)calloc(sizeof(char)* (strlen(key) + 1), 1);
	}

	strcpy(new_item->key, key);
	new_item->data = data;

	uint32_t hash_index = HashIndexForString(key) & (hash->table_size - 1);
	assert(hash_index >= 0 && hash_index < hash->table_size);

	new_item->next = hash->items[hash_index];
	hash->items[hash_index] = new_item;
}

void AddToHash(generic_hash *hash, void *data, v2i location, memory_arena *arena)
{
	assert(hash->initialized);
	hash_item *new_item;
	if (arena)
		new_item = PUSH_ITEM(arena, hash_item);
	else
		new_item = (hash_item *)calloc(sizeof(hash_item), 1);

	new_item->location = location;
	new_item->data = data;

	uint32_t hash_index = HashIndexForIVec2(location) & (hash->table_size - 1);
	assert(hash_index >= 0 && hash_index < hash->table_size);

	new_item->next = hash->items[hash_index];
	hash->items[hash_index] = new_item;
}

