//
//  csmface_debug.c
//  rGWB
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
#include "csmmath.inl"
#include "csmmath.tli"

// ----------------------------------------------------------------------------------------------------

void csmface_debug_print_info_debug(struct csmface_t *face, CSMBOOL assert_si_no_es_integro, unsigned long *num_holes_opc)
{
    unsigned long num_holes_loc;
    CSMBOOL compute_loop_area;
    double Xo, Yo, Zo, Ux, Uy, Uz, Vx, Vy, Vz;
    struct csmloop_t *loop_iterator;
    
    assert_no_null(face);
    
    csmdebug_print_debug_info(
                        "\tFace %lu (%g, %g, %g, %g) Setop Null face: %lu\n",
                        face->id,
                        face->A, face->B, face->C, face->D,
                        face->setop_is_null_face);
    
    if (csmmath_fabs(face->A) > 0. || csmmath_fabs(face->B) > 0. || csmmath_fabs(face->C) > 0.)
    {
        compute_loop_area = CSMTRUE;

        csmmath_plane_axis_from_implicit_plane_equation(
                        face->A, face->B, face->C, face->D,
                        &Xo, &Yo, &Zo,
                        &Ux, &Uy, &Uz, &Vx, &Vy, &Vz);
    }
    else
    {
        compute_loop_area = CSMFALSE;
    }
    
    loop_iterator = face->floops;
    num_holes_loc = 0;
    
    while (loop_iterator != NULL)
    {
        struct csmloop_t *next_loop;
        CSMBOOL is_outer_loop;
        double loop_area;
        
        is_outer_loop = IS_TRUE(face->flout == loop_iterator);
        
        if (compute_loop_area == CSMTRUE)
        {
            loop_area = csmloop_compute_area(
                    loop_iterator,
                    Xo, Yo, Zo,
                    Ux, Uy, Uz, Vx, Vy, Vz);
        }
        else
        {
            loop_area = 0.;
        }
        
        csmloop_debug_print_info_debug(
	                    loop_iterator,
                        is_outer_loop,
                        compute_loop_area, loop_area,
                        assert_si_no_es_integro);
        
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
