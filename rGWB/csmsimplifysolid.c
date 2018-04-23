//
//  csmsimplifysolid.c
//  rGWB
//
//  Created by Manuel Fernández on 19/4/18.
//  Copyright © 2018 Manuel Fernández. All rights reserved.
//

#include "csmsimplifysolid.inl"

#include "csmarrayc.inl"
#include "csmdebug.inl"
#include "csmedge.inl"
#include "csmedge.tli"
#include "csmeuler_lmekr.inl"
#include "csmeuler_lkef.inl"
#include "csmeuler_lkemr.inl"
#include "csmeuler_lkev.inl"
#include "csmeuler_lringmv.inl"
#include "csmface.inl"
#include "csmface_debug.inl"
#include "csmhashtb.inl"
#include "csmhedge.inl"
#include "csmloop.inl"
#include "csmmath.inl"
#include "csmopbas.inl"
#include "csmsolid.inl"
#include "csmsolid_debug.inl"
#include "csmtolerance.inl"
#include "csmvertex.inl"
#include "csmvertex.tli"

#ifdef __STANDALONE_DISTRIBUTABLE
#include "csmassert.inl"
#else
#include "cyassert.h"
#endif

// ----------------------------------------------------------------------------------------------------

static void i_delete_null_area_faces(struct csmsolid_t *solid, const struct csmtolerance_t *tolerances, CSMBOOL *changed)
{
    CSMBOOL there_are_changes;
    unsigned long no_iters;
    
    assert_no_null(changed);
    
    csmdebug_print_debug_info("Deleting null area faces...\n");
    
    no_iters = 0;
    
    do
    {
        struct csmhashtb_iterator(csmedge_t) *edge_iterator;
        
        assert(no_iters < 10000);
        no_iters++;
        
        edge_iterator = csmsolid_edge_iterator(solid);
        there_are_changes = CSMFALSE;
        
        while (csmhashtb_has_next(edge_iterator, csmedge_t) == CSMTRUE)
        {
            struct csmedge_t *edge;
            struct csmhedge_t *he1, *he2;
            struct csmface_t *face_he1, *face_he2;
            double face_he1_area, face_he2_area;
            
            csmhashtb_next_pair(edge_iterator, NULL, &edge, csmedge_t);
            
            he1 = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_POS);
            face_he1 = csmopbas_face_from_hedge(he1);
            face_he1_area = csmface_loop_area_in_face(face_he1, csmface_flout(face_he1));
            
            he2 = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_NEG);
            face_he2 = csmopbas_face_from_hedge(he2);
            face_he2_area = csmface_loop_area_in_face(face_he2, csmface_flout(face_he2));
            
            if (face_he1_area == 0. || face_he2_area == 0.)
            {
                if (face_he1_area == 0.)
                {
                    csmeuler_lkef(&he2, &he1);
                    csmface_redo_geometric_generated_data(face_he2);
                }
                else
                {
                    csmeuler_lkef(&he1, &he2);
                    csmface_redo_geometric_generated_data(face_he1);
                }
                
                there_are_changes = CSMTRUE;
                break;
            }
        }
        
        if (there_are_changes == CSMTRUE)
            *changed = CSMTRUE;
        
        csmhashtb_free_iterator(&edge_iterator, csmedge_t);
        
    } while (there_are_changes == CSMTRUE);
}

// ----------------------------------------------------------------------------------------------------

static void i_delete_overlaped_hedges_in_same_loop(struct csmloop_t *loop, CSMBOOL *changed)
{
    CSMBOOL there_are_changes;
    unsigned long no_iters;
    
    no_iters = 0;
    
    do
    {
        struct csmhedge_t *ledge, *he_iterator;
        
        assert(no_iters < 1000);
        no_iters++;
        
        ledge = csmloop_ledge(loop);
        he_iterator = ledge;

        there_are_changes = CSMFALSE;
        
        do
        {
            struct csmhedge_t *he_iterator_mate;
            
            he_iterator_mate = csmopbas_mate(he_iterator);
            
            if (csmhedge_loop(he_iterator_mate) == loop && csmhedge_next(he_iterator) == he_iterator_mate)
            {
                if (csmdebug_debug_enabled() == CSMTRUE)
                {
                    csmdebug_show_face(csmloop_lface(loop), NULL);
                    csmface_debug_print_info_debug(csmloop_lface(loop), CSMTRUE, NULL);
                }
                
                csmeuler_lkev(&he_iterator_mate, &he_iterator, NULL, NULL, NULL, NULL);
                
                there_are_changes = CSMTRUE;
                
                if (csmdebug_debug_enabled() == CSMTRUE)
                {
                    csmdebug_show_face(csmloop_lface(loop), NULL);
                    csmface_debug_print_info_debug(csmloop_lface(loop), CSMTRUE, NULL);
                }
            }
            else
            {
                he_iterator = csmhedge_next(he_iterator);
            }
            
            if (there_are_changes == CSMTRUE)
            {
                *changed = CSMTRUE;
                break;
            }
            
        } while (he_iterator != ledge);
        
    } while (there_are_changes == CSMTRUE);
}

// ----------------------------------------------------------------------------------------------------

static void i_delete_overlaped_hedges_in_same_face_loop(struct csmface_t *face, CSMBOOL *changed)
{
    struct csmloop_t *loop_iterator;
    struct csmloop_t *flout;
    
    loop_iterator = csmface_floops(face);
    flout = csmface_flout(face);
    
    while (loop_iterator != NULL)
    {
        struct csmloop_t *loop_next;

        loop_next = csmloop_next(loop_iterator);
        
        if (loop_iterator != flout)
            i_delete_overlaped_hedges_in_same_loop(loop_iterator, changed);
        
        loop_iterator = loop_next;
    }
    
    i_delete_overlaped_hedges_in_same_loop(flout, changed);
}

// ----------------------------------------------------------------------------------------------------

static void i_delete_redundant_faces(struct csmsolid_t *solid, const struct csmtolerance_t *tolerances, CSMBOOL *changed)
{
    CSMBOOL there_are_changes;
    unsigned long no_iters;
    
    assert_no_null(changed);
    
    csmdebug_print_debug_info("Deleting coplanar faces...\n");
    
    no_iters = 0;
    
    do
    {
        struct csmhashtb_iterator(csmedge_t) *edge_iterator;
        
        assert(no_iters < 10000);
        no_iters++;
        
        edge_iterator = csmsolid_edge_iterator(solid);
        there_are_changes = CSMFALSE;
        
        while (csmhashtb_has_next(edge_iterator, csmedge_t) == CSMTRUE)
        {
            struct csmedge_t *edge;
            struct csmhedge_t *he1, *he2;
            struct csmface_t *face_he1, *face_he2;
            CSMBOOL same_sense;
            
            csmhashtb_next_pair(edge_iterator, NULL, &edge, csmedge_t);
            
            he1 = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_POS);
            face_he1 = csmopbas_face_from_hedge(he1);
            
            he2 = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_NEG);
            face_he2 = csmopbas_face_from_hedge(he2);
            
            if (csmface_flout(face_he1) == csmhedge_loop(he1) && csmface_flout(face_he2) == csmhedge_loop(he2))
            {
                if (face_he1 == face_he2)
                {
                    struct csmloop_t *outer_loop, *new_loop;
                    double A, B, C, D, xc, yc, zc;
                    
                    there_are_changes = CSMTRUE;
                    
                    outer_loop = csmface_flout(face_he1);
                    csmloop_face_equation(outer_loop, &A, &B, &C, &D, &xc, &yc, &zc);

                    if (csmdebug_debug_enabled() == CSMTRUE)
                    {
                        //csmdebug_set_debug_screen(CSMTRUE);
                        csmsolid_debug_print_debug_forced(solid);
                        csmdebug_show_face(face_he1, NULL);
                    }
                    
                    csmeuler_lkemr(&he1, &he2, NULL, NULL, &new_loop);
                    
                    if (csmface_is_loop_contained_in_face_outer_loop(face_he1, new_loop, tolerances) == CSMFALSE)
                    {
                        double A_new, B_new, C_new, D_new, xc_new, yc_new, zc_new;
                        CSMBOOL parallel, same_sense;

                        csmloop_face_equation(new_loop, &A_new, &B_new, &C_new, &D_new, &xc_new, &yc_new, &zc_new);
                        
                        parallel = csmmath_unit_vectors_are_parallel_ex(A, B, C, A_new, B_new, C_new, tolerances, &same_sense);
                        assert(parallel == CSMTRUE);
                        
                        if (same_sense == CSMFALSE)
                            csmloop_revert_loop_orientation(new_loop);
                        
                        csmeuler_lringmv(new_loop, face_he1, CSMTRUE);
                        csmface_reorient_loops_in_face(face_he1, tolerances);
                    }
                    
                    if (csmdebug_debug_enabled() == CSMTRUE)
                        csmdebug_show_face(face_he1, NULL);
                }
                else if (csmface_are_coplanar_faces(face_he1, face_he2, tolerances, &same_sense) == CSMTRUE && same_sense == CSMTRUE)
                {
                    there_are_changes = CSMTRUE;
                    
                    if (csmdebug_debug_enabled() == CSMTRUE)
                    {
                        csmsolid_debug_print_debug_forced(solid);
                        csmdebug_show_face(face_he1, face_he2);
                    }
                
                    csmeuler_lkef(&he1, &he2);
                    
                    if (csmdebug_debug_enabled() == CSMTRUE)
                        csmdebug_show_face(face_he1, NULL);
                }
                
                if (there_are_changes == CSMTRUE)
                {
                    i_delete_overlaped_hedges_in_same_face_loop(face_he1, changed);
                    csmface_redo_geometric_generated_data(face_he1);
                    break;
                }
            }
        }
        
        if (there_are_changes == CSMTRUE)
            *changed = CSMTRUE;
        
        csmhashtb_free_iterator(&edge_iterator, csmedge_t);
        
    } while (there_are_changes == CSMTRUE);
}

// ----------------------------------------------------------------------------------------------------

static unsigned long i_num_uses_of_vertex(struct csmvertex_t *vertex)
{
    unsigned long no_uses;
    struct csmhedge_t *he, *he_iterator;
    unsigned long no_iterations;
    
    he = csmvertex_hedge(vertex);
    he_iterator = he;
    
    no_uses = 0;
    no_iterations = 0;
    
    do
    {
        assert(no_iterations < 1000);
        no_iterations++;
        
        no_uses++;
        he_iterator = csmhedge_next(csmopbas_mate(he_iterator));
        
    } while (he != he_iterator);
    
    return no_uses;
}

// ----------------------------------------------------------------------------------------------------

static void i_delete_redundant_vertexs(struct csmsolid_t *solid, CSMBOOL *changed)
{
    CSMBOOL there_are_changes;
    unsigned long no_iterations;
    
    assert_no_null(changed);
    
    csmdebug_print_debug_info("Deleting vertexs...\n");
    no_iterations = 0;
    
    do
    {
        struct csmhashtb_iterator(csmvertex_t) *vertex_iterator;
        
        assert(no_iterations < 100000);
        no_iterations++;
        
        vertex_iterator = csmsolid_vertex_iterator(solid);
        there_are_changes = CSMFALSE;
        
        while (csmhashtb_has_next(vertex_iterator, csmvertex_t) == CSMTRUE)
        {
            struct csmvertex_t *vertex;
            
            csmhashtb_next_pair(vertex_iterator, NULL, &vertex, csmvertex_t);
            
            if (i_num_uses_of_vertex(vertex) == 2)
            {
                struct csmhedge_t *he1, *he2;
                
                he1 = csmvertex_hedge(vertex);
                he2 = csmhedge_next(csmopbas_mate(he1));
                
                if (csmhedge_edge(he1) != csmhedge_edge(he2))
                {
                    struct csmhedge_t *he1_mate;
                    
                    he1_mate = csmopbas_mate(he1);
                    
                    csmeuler_lkev(&he1, &he1_mate, NULL, NULL, NULL, NULL);
                    there_are_changes = CSMTRUE;
                    
                    csmsolid_debug_print_debug(solid, CSMTRUE);
                    break;
                }
            }
        }
        
        if (there_are_changes == CSMTRUE)
            *changed = CSMTRUE;
        
        csmhashtb_free_iterator(&vertex_iterator, csmvertex_t);
        
    } while (there_are_changes == CSMTRUE);
}

// ----------------------------------------------------------------------------------------------------

void csmsimplifysolid_simplify(struct csmsolid_t *solid, const struct csmtolerance_t *tolerances)
{
    CSMBOOL changed;
    
    csmsolid_redo_geometric_face_data(solid);
    
    if (csmdebug_debug_enabled() == CSMTRUE)
    {
        csmdebug_print_debug_info("Before simplification...\n");
        csmsolid_debug_print_debug_forced(solid);
    }
        
    changed = CSMFALSE;
    i_delete_null_area_faces(solid, tolerances, &changed);
    i_delete_redundant_faces(solid, tolerances, &changed);
    i_delete_redundant_vertexs(solid, &changed);

    if (changed == CSMTRUE && csmdebug_debug_enabled() == CSMTRUE)
    {
        csmdebug_print_debug_info("After simplification...\n");
        csmsolid_debug_print_debug_forced(solid);
        
        csmdebug_show_viewer();
    }
    
    if (changed == CSMTRUE)
        csmsolid_redo_geometric_face_data(solid);
}

