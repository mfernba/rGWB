//
//  csmeuler_laringmv.c
//  cysolidmodeling
//
//  Created by Manuel Fernández on 13/4/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmeuler_laringmv.inl"

#include "csmdebug.inl"
#include "csmloop.inl"
#include "csmface.inl"
#include "csmhedge.inl"
#include "csmvertex.inl"

#include "cyassert.h"

// ------------------------------------------------------------------------------------------

void csmeuler_laringmv(struct csmface_t *face1, struct csmface_t *face2)
{
    register struct csmloop_t *iterator_face1;
    unsigned long num_iteraciones;
    
    iterator_face1 = csmface_floops(face1);
    num_iteraciones = 0;
    
    do
    {
        struct csmloop_t *next_loop;
        
        assert(num_iteraciones < 10000);
        num_iteraciones++;
        
        next_loop = csmloop_next(iterator_face1);
        
        if (iterator_face1 != csmface_flout(face1))
        {
            struct csmhedge_t *ledge;
            struct csmvertex_t *vtx;
            double x, y, z;
            
            ledge = csmloop_ledge(iterator_face1);
            vtx = csmhedge_vertex(ledge);
            csmvertex_get_coordenadas(vtx, &x, &y, &z);
            
            if (csmface_is_point_interior_to_face(face1, x, y, z) == FALSO)
            {
                csmdebug_print_debug_info("\t***laringmv loop %lu to face %lu\n", csmloop_id(iterator_face1), csmface_id(face2));
                csmface_add_loop_while_removing_from_old(face2, iterator_face1);
            }
        }
        
        iterator_face1 = next_loop;
        
    } while (iterator_face1 != NULL);
}
