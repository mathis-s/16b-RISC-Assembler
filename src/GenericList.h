#pragma once

#include "stddef.h"
#include "memory.h"
#include <stdlib.h>
#include <stdbool.h>

static const size_t DEFAULT_COUNT = 1;

typedef struct
{
    void* data;
    size_t memberSize;

    size_t count;
    size_t maxCount;

} GenericList;

GenericList GenericList_Create (size_t memberSize);
GenericList GenericList_CreateCopy (GenericList original);

void GenericList_Append (GenericList* this, void* member);

void* GenericList_At (GenericList* this, size_t index);

// Find an object in this List based on a generic comperator. Left side in comparator(void*, void*) 
// is object pointer in list, right side is always compareTo.
void* GenericList_Find (GenericList* this, bool (*comparator)(void*, void*), void* compareTo);

void GenericList_ForAll (GenericList* this, void (*func)(void*, void*), void* parameters);

void GenericList_Dispose (GenericList* this);