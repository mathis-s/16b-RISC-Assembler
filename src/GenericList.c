#include "GenericList.h"
#include "Util.h"

GenericList GenericList_Create (size_t memberSize)
{
    GenericList l;

    l.data = xmalloc(DEFAULT_COUNT * memberSize);
    l.count = 0;
    l.maxCount = DEFAULT_COUNT;
    l.memberSize = memberSize;

    return l;
}

GenericList GenericList_CreateCopy (GenericList original)
{
    GenericList copy;

    copy.data = xmalloc(original.memberSize * original.maxCount);
    copy.count = original.count;
    copy.maxCount = original.maxCount;
    copy.memberSize = original.memberSize;

    memcpy(copy.data, original.data, original.memberSize * original.count);

    return copy;
}

void GenericList_Append (GenericList* this, void* member)
{
    if (this->count == this->maxCount)
        this->data = realloc(this->data, (this->maxCount *= 2) * this->memberSize);
    
    memcpy(this->data + this->count++ * this->memberSize, member, this->memberSize);
}

void* GenericList_At (GenericList* this, size_t index)
{
    return this->data + index * this->memberSize;
}

// Find an object in this List based on a generic comparator. Left side in comparator(void*, void*) 
// is object pointer in list, right side is always compareTo.
void* GenericList_Find (GenericList* this, bool (*comparator)(void*, void*), void* compareTo)
{
    for (size_t i = 0; i < this->count; i++)
    {
        if (comparator(this->data + this->memberSize * i, compareTo))
            return this->data + this->memberSize * i;
    }
    return NULL;
}

// Runs func for all elements in this list. The first parameter for func
// is the pointer to the current list element, the second is for optional
// parameters
void GenericList_ForAll (GenericList* this, void (*func)(void*, void*), void* parameters)
{
    for (size_t i = 0; i < this->count; i++)
        func(this->data + i * this->memberSize, parameters);
    
}

void GenericList_Dispose (GenericList* this)
{
    free(this->data);
    this->count = 0;
    this->maxCount = 0;
    this->memberSize = 0;
    this->data = NULL;
}