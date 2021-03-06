//
//  csmeuler_laringmv.c
//  rGWB
//
//  Created by Manuel Fernández on 13/4/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmeuler_laringmv.inl"

#include "csmarrayc.h"
#include "csmdebug.inl"
#include "csmface.inl"
#include "csmhedge.inl"
#include "csmloop.inl"
#include "csmvertex.inl"

#ifdef RGWB_STANDALONE_DISTRIBUTABLE
#include "csmassert.inl"
#else
#include "cyassert.h"
#endif

// ------------------------------------------------------------------------------------------

void csmeuler_laringmv(
                    struct csmface_t *face1, struct csmface_t *face2,
                    const struct csmtolerance_t *tolerances)
{
    struct csmloop_t *iterator_face1;
    unsigned long no_iterations;
    struct csmloop_t *flout_face1;
    const csmArrayStruct(csmloop_t) *face1_inner_loops_with_area;
    
    csmface_redo_geometric_generated_data(face1);
    
    iterator_face1 = csmface_floops(face1);
    no_iterations = 0;
    
    flout_face1 = csmface_flout(face1);
    face1_inner_loops_with_area = csmface_get_inner_loops_with_area(face1);
    
    do
    {
        struct csmloop_t *next_loop;
        
        assert(no_iterations < 100000);
        no_iterations++;
        
        next_loop = csmloop_next(iterator_face1);
        
        if (iterator_face1 != flout_face1)
        {
            struct csmhedge_t *ledge;
            struct csmvertex_t *vtx;
            double x, y, z;
            
            ledge = csmloop_ledge(iterator_face1);
            vtx = csmhedge_vertex(ledge);
            csmvertex_get_coords(vtx, &x, &y, &z);
            
            if (csmface_is_point_interior_to_face_optimized_laringmv(face1, face1_inner_loops_with_area, x, y, z, tolerances) == CSMFALSE)
            {
                csmloop_set_setop_loop_was_a_hole(iterator_face1, CSMTRUE);
                
                csmdebug_print_debug_info("\t***laringmv loop %lu to face %lu\n", csmloop_id(iterator_face1), csmface_id(face2));
                csmface_add_loop_while_removing_from_old(face2, iterator_face1);
            }
        }
        
        iterator_face1 = next_loop;
        
    } while (iterator_face1 != NULL);
    
    csmarrayc_free_const_st(&face1_inner_loops_with_area, csmloop_t);
}

// ------------------------------------------------------------------------------------------

void csmeuler_laringmv_from_face1_to_2_if_fits_in_face(
                    struct csmface_t *face1, struct csmface_t *face2,
                    const struct csmtolerance_t *tolerances,
                    CSMBOOL *did_move_some_loop_opt)
{
    CSMBOOL did_move_some_loop_loc;
    struct csmloop_t *iterator_face1;
    unsigned long no_iterations;
    
    csmface_redo_geometric_generated_data(face1);
    
    iterator_face1 = csmface_floops(face1);
    no_iterations = 0;
    did_move_some_loop_loc = CSMFALSE;
    
    do
    {
        struct csmloop_t *next_loop;
        
        assert(no_iterations < 100000);
        no_iterations++;
        
        next_loop = csmloop_next(iterator_face1);
        
        {
            struct csmhedge_t *ledge;
            struct csmvertex_t *vtx;
            double x, y, z;
            
            ledge = csmloop_ledge(iterator_face1);
            vtx = csmhedge_vertex(ledge);
            csmvertex_get_coords(vtx, &x, &y, &z);
            
            if (csmface_is_point_interior_to_face(face2, x, y, z, tolerances) == CSMTRUE)
            {
                csmdebug_print_debug_info("\t***laringmv2 loop %lu to face %lu\n", csmloop_id(iterator_face1), csmface_id(face2));
                
                csmloop_set_setop_convert_loop_in_face(iterator_face1, CSMTRUE);
                csmface_add_loop_while_removing_from_old(face2, iterator_face1);
                
                did_move_some_loop_loc = CSMTRUE;
            }
        }
        
        iterator_face1 = next_loop;
        
    } while (iterator_face1 != NULL);
    
    ASSIGN_OPTIONAL_VALUE(did_move_some_loop_opt, did_move_some_loop_loc);
}

