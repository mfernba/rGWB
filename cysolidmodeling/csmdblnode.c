//
//  csmdblnode.c
//  cysolidmodeling
//
//  Created by Manuel Fernández on 8/3/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmdblnode.inl"

struct csmdblnode_t
{
    struct csmnode_t *node;
    struct csmdblnode_t *prev, *next;
};
