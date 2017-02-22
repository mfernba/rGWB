//
//  sm4dblist.h
//  solidmodeling4
//
//  Created by Manuel Fernandez on 1/2/15.
//  Copyright (c) 2015 manueru. All rights reserved.
//
//  An implementation of Martti Mäntylä Geometric Workbench
//
//  Basic memory management module.
//

#include <stddef.h>

void *cypespy_dontuse_malloc(size_t n_bytes);
#define MALLOC(type) (type *)cypespy_dontuse_malloc(sizeof(type))

void cypespy_dontuse_free(void **data, size_t bytes);
#define FREE_PP(data, type)\
(\
    (type **)data == data,\
    cypespy_dontuse_free((void **)data, sizeof(data))\
)

void *cypespy_dontuse_dereference_memory(void **data);
#define ASIGNA_PUNTERO_PP_NO_NULL(data, type)\
(\
    ((type **)data == data),\
    (type *)cypespy_dontuse_dereference_memory((void **)data)\
)

void *cypespy_dontuse_dereference_optional_memory(void **data);
#define ASIGNA_PUNTERO_PP(data, type)\
(\
    ((type **)data == data),\
    (type *)cypespy_dontuse_dereference_optional_memory((void **)data)\
)
