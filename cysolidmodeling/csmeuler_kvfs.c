//
//  csmeuler_kvfs.c
//  cysolidmodeling
//
//  Created by Manuel Fernández on 8/3/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmeuler_kvfs.inl"

#include "csmloop.inl"
#include "csmnode.inl"
#include "csmface.inl"
#include "csmsolid.tli"
#include "csmvertex.inl"

#include "cyassert.h"

// ----------------------------------------------------------------------------------------------------

void csmeuler_kvfs(struct csmsolid_t *solido)
{
    struct csmloop_t *floops;
    
    assert_no_null(solido);
    assert_no_null(solido->sfaces);
    assert(csmface_next(solido->sfaces) == NULL);
    assert(solido->sedges == NULL);
    assert_no_null(solido->svertexs);
    assert(csmvertex_next(solido->svertexs) == NULL);
    
    floops = csmface_floops(solido->sfaces);
    assert(csmloop_next(floops) == NULL);
    
    csmnode_free_node_list(&solido->sfaces, csmface_t);
    csmnode_free_node_list(&solido->svertexs, csmvertex_t);
}
