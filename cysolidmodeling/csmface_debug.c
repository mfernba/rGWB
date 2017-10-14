//
//  csmface_debug.c
//  cysolidmodeling
//
//  Created by Manuel Fernandez on 14/10/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmface_debug.inl"
#include "csmface.tli"

#include "csmassert.inl"
#include "csmdebug.inl"
#include "csmloop.inl"
#include "csmloop_debug.inl"

// ----------------------------------------------------------------------------------------------------

void csmface_debug_print_info_debug(struct csmface_t *face, CSMBOOL assert_si_no_es_integro, unsigned long *num_holes_opc)
{
    unsigned long num_holes_loc;
    struct csmloop_t *loop_iterator;
    
    assert_no_null(face);
    
    csmdebug_print_debug_info(
                        "\tFace %lu (%g, %g, %g, %g) Setop Null face: %lu\n",
                        face->id,
                        face->A, face->B, face->C, face->D,
                        face->setop_is_null_face);
    
    loop_iterator = face->floops;
    num_holes_loc = 0;
    
    while (loop_iterator != NULL)
    {
        struct csmloop_t *next_loop;
        CSMBOOL is_outer_loop;
        
        is_outer_loop = IS_TRUE(face->flout == loop_iterator);
        csmloop_debug_print_info_debug(loop_iterator, is_outer_loop, assert_si_no_es_integro);
        
        if (is_outer_loop == CSMFALSE)
            num_holes_loc++;
        
        next_loop = csmloop_next(loop_iterator);
        
        if (assert_si_no_es_integro == CSMTRUE)
        {
            assert(csmloop_lface(loop_iterator) == face);
            
            if (next_loop != NULL)
                assert(csmloop_prev(next_loop) == loop_iterator);
        }
        
        loop_iterator = next_loop;
    }
    
    ASSIGN_OPTIONAL_VALUE(num_holes_opc, num_holes_loc);
}
