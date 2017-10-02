//
//  sm4mem.c
//  solidmodeling4
//
//  Created by Manuel Fernandez on 1/2/15.
//  Copyright (c) 2015 manueru. All rights reserved.
//

#include "csmmem.inl"

#include "csmassert.inl"

#include <stdlib.h>
#include <string.h>

static const int i_MEMORY_CREATION_MARK = 0xAB;

// ---------------------------------------------------------------------------------

void *csmmem_dontuse_malloc(size_t n_bytes)
{
    void *memory;
    
    assert(n_bytes > 0);
    
    memory = malloc(n_bytes);
    memset(memory, i_MEMORY_CREATION_MARK, n_bytes);
    
    assert(memory != NULL);
    
    return memory;
}

// ---------------------------------------------------------------------------------

void csmmem_dontuse_free(void **data, size_t bytes)
{
    void *data_loc;
    
    assert(data != NULL);
    
    data_loc = *data;
    *data = NULL;
    
    free(data_loc);
}

// ---------------------------------------------------------------------------------

void *csmmem_dontuse_dereference_memory(void **data)
{
    void *data_loc;
    
    assert(data != NULL);
    
    data_loc = *data;
    *data = NULL;
    
    return data_loc;
}

// ---------------------------------------------------------------------------------

void *csmmem_dontuse_dereference_optional_memory(void **data)
{
    if (data == NULL || *data == NULL)
    {
        return NULL;
    }
    else
    {
        void *data_loc;
    
        data_loc = *data;
        *data = NULL;
        
        return data_loc;
    }
}





