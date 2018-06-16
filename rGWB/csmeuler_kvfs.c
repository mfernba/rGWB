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
#include "csmsolid.inl"

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

    assert(csmsolid_num_faces(solid) == 1);
    assert(csmsolid_num_edges(solid) == 0);
    assert(csmsolid_num_vertexs(solid) == 1);
    
    face_iterator = csmsolid_face_iterator(solid);
    
    csmhashtb_next_pair(face_iterator, NULL, &face, csmface_t);
    assert(csmhashtb_has_next(face_iterator, csmface_t) == CSMFALSE);
    
    floops = csmface_floops(face);
    assert(csmloop_next(floops) == NULL);
    
    csmhashtb_free_iterator(&face_iterator, csmface_t);
 
    csmsolid_clear_face_table(solid);
    csmsolid_clear_vertex_table(solid);
}
