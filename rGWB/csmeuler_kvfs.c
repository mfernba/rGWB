//
//  csmeuler_kvfs.c
//  rGWB
//
//  Created by Manuel Fernández on 8/3/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmeuler_kvfs.inl"

#include "csmface.inl"
#include "csmhashtb.inl"
#include "csmloop.inl"
#include "csmsolid.tli"
#include "csmvertex.inl"

#ifdef __STANDALONE_DISTRIBUTABLE
#include "csmassert.inl"
#else
#include "cyassert.h"
#endif

// ----------------------------------------------------------------------------------------------------

void csmeuler_kvfs(struct csmsolid_t *solid)
{
    struct csmhashtb_iterator(csmface_t) *face_iterator;
    struct csmface_t *face;
    struct csmloop_t *floops;

    assert_no_null(solid);
    assert(csmhashtb_count(solid->sfaces, csmface_t) == 1);
    assert(csmhashtb_count(solid->sedges, csmedge_t) == 0);
    assert(csmhashtb_count(solid->svertexs, csmvertex_t) == 1);
    
    face_iterator = csmhashtb_create_iterator(solid->sfaces, csmface_t);
    assert(csmhashtb_has_next(face_iterator, csmface_t) == CSMTRUE);
    
    csmhashtb_next_pair(face_iterator, NULL, &face, csmface_t);
    assert(csmhashtb_has_next(face_iterator, csmface_t) == CSMFALSE);
    
    floops = csmface_floops(face);
    assert(csmloop_next(floops) == NULL);
    
    csmhashtb_free_iterator(&face_iterator, csmface_t);
    
    csmhashtb_clear(solid->sfaces, csmface_t, csmface_free);
    csmhashtb_clear(solid->svertexs, csmvertex_t, csmvertex_free);
}
