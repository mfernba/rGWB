//
//  csmeuler_laringmv.c
//  cysolidmodeling
//
//  Created by Manuel Fernández on 13/4/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmeuler_laringmv.inl"

#include "csmloop.inl"
#include "csmface.inl"
#include "csmhedge.inl"

#include "cyassert.h"

// ------------------------------------------------------------------------------------------

void csmeuler_laringmv(struct csmface_t *face1, struct csmface_t *face2)
{
    struct csmloop_t *flout_face1;
    register struct csmloop_t *iterator_face1;
    unsigned long num_iteraciones;
    
    flout_face1 = csmface_flout(face1);
    iterator_face1 = csmface_floops(face1);
    num_iteraciones = 0;
    
    do
    {
        struct csmloop_t *next_loop;
        
        assert(num_iteraciones < 10000);
        num_iteraciones++;
        
        next_loop = csmloop_next(iterator_face1);
        
        if (csmface_is_loop_contained_in_face(face1, iterator_face1) == FALSO)
            csmface_add_loop_while_removing_from_old(face2, iterator_face1);
            
        iterator_face1 = next_loop;
        
    } while (iterator_face1 != NULL);
}
