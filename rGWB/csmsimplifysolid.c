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

static void i_simplify_loop(
                        struct csmloop_t *loop,
                        CSMBOOL is_inner_loop, struct csmloop_t *outer_loop,
                        const struct csmtolerance_t *tolerances,
                        CSMBOOL *changed,
                        CSMBOOL *loop_has_been_killed_opt)
{
    CSMBOOL there_are_changes;
    unsigned long no_iters;
    double tolerance_equal_coords;
    CSMBOOL loop_has_been_killed;
    
    tolerance_equal_coords = csmtolerance_equal_coords(tolerances);
    
    no_iters = 0;
    loop_has_been_killed = CSMFALSE;
    
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
            struct csmvertex_t *he_iterator_vertex;
            struct csmhedge_t *he_next;
            struct csmvertex_t *he_next_next_vertex;
            
            he_next = csmhedge_next(he_iterator);
            he_next_next_vertex = csmhedge_vertex(csmhedge_next(he_next));
            
            he_iterator_vertex = csmhedge_vertex(he_iterator);
            
            if (csmvertex_equal_coords(he_iterator_vertex, he_next_next_vertex, tolerance_equal_coords) == CSMTRUE)
            {
                struct csmhedge_t *he_next_mate;
            
                csmdebug_set_debug_screen(CSMTRUE);
                
                csmface_debug_print_info_debug(csmloop_lface(loop), CSMTRUE, NULL);
                csmdebug_show_face(csmloop_lface(loop), NULL);

                he_next_mate = csmopbas_mate(he_next);
                
                if (he_iterator == he_next)
                {
                    struct csmhedge_t *flout_hedge;
                    
                    assert(csmhedge_prev(he_iterator) == he_iterator);
                    
                    if (is_inner_loop == CSMTRUE)
                    {
                        struct csmhedge_t *he1_aux, *he2_aux;
                        
                        flout_hedge = csmloop_ledge(outer_loop);
                    
                        csmeuler_lmekr(flout_hedge, he_iterator, &he1_aux, &he2_aux);
                        csmeuler_lkev(&he_next, &he_next_mate, NULL, NULL, NULL, NULL);
                        csmeuler_lkev(&he1_aux, &he2_aux, NULL, NULL, NULL, NULL);
                    }
                    else
                    {
                        csmface_debug_print_info_debug(csmloop_lface(csmhedge_loop(he_next_mate)), CSMTRUE, NULL);
                        csmdebug_show_face(csmloop_lface(csmhedge_loop(he_next_mate)), NULL);
                        
                        csmeuler_lkef(&he_next_mate, &he_next);
                    }
                    
                    loop_has_been_killed = CSMTRUE;
                }
                else
                {
                    csmeuler_lkev(&he_next, &he_next_mate, NULL, NULL, NULL, NULL);
                    
                    csmface_debug_print_info_debug(csmloop_lface(loop), CSMTRUE, NULL);
                    csmdebug_show_face(csmloop_lface(loop), NULL);
                }
                
                there_are_changes = CSMTRUE;
            }
            else
            {
                he_iterator = he_next;
            }
            
            if (there_are_changes == CSMTRUE || loop_has_been_killed == CSMTRUE)
                break;
            
        }while (he_iterator != ledge);
        
    } while (there_are_changes == CSMTRUE && loop_has_been_killed == CSMFALSE);
    
    ASSIGN_OPTIONAL_VALUE(loop_has_been_killed_opt, loop_has_been_killed);
}

// ----------------------------------------------------------------------------------------------------

static void i_simplify_face_loop(struct csmface_t *face, const struct csmtolerance_t *tolerances, CSMBOOL *changed, CSMBOOL *face_has_been_deleted)
{
    struct csmloop_t *face_floops, *loop_iterator;
    struct csmloop_t *flout;
    
    assert_no_null(face_has_been_deleted);
    
    csmdebug_print_debug_info("Simpliying face %lu...\n", csmface_id(face));
    
    face_floops = csmface_floops(face);
    flout = csmface_flout(face);
    
    if (csmface_id(face) == 40)
    {
        if (csmdebug_debug_enabled() == CSMTRUE)
        {
            CSMBOOL state;
            
            state = csmdebug_get_debug_screen();
            csmdebug_set_debug_screen(CSMTRUE);
            
            csmface_debug_print_info_debug(face, CSMTRUE, NULL);
            csmdebug_show_face(face, NULL);
            
            csmdebug_set_debug_screen(state);
        }
    }
    
    loop_iterator = face_floops;
    
    while (loop_iterator != NULL)
    {
        struct csmloop_t *loop_next;

        loop_next = csmloop_next(loop_iterator);
        
        if (loop_iterator != flout)
        {
            i_simplify_loop(loop_iterator, CSMTRUE, flout, tolerances, changed, NULL);
        }
        
        loop_iterator = loop_next;
    }
    
    i_simplify_loop(flout, CSMFALSE, NULL, tolerances, changed, face_has_been_deleted);
    
    if (*face_has_been_deleted == CSMTRUE)
        csmdebug_print_debug_info("Face deleted\n");
}

// ----------------------------------------------------------------------------------------------------

static void i_simplify_all_face_loops(struct csmsolid_t *solid, const struct csmtolerance_t *tolerances, CSMBOOL *changed)
{
    CSMBOOL there_are_changes;
    unsigned long no_iters;
    
    assert_no_null(changed);

    csmdebug_print_debug_info("Simpliying faces...\n");
    no_iters = 0;
    
    do
    {
        struct csmhashtb_iterator(csmface_t) *face_iterator;
        
        assert(no_iters < 10000);
        no_iters++;
        
        face_iterator = csmsolid_face_iterator(solid);
        there_are_changes = CSMFALSE;
    
        while (csmhashtb_has_next(face_iterator, csmface_t) == CSMTRUE)
        {
            struct csmface_t *face;
        
            csmhashtb_next_pair(face_iterator, NULL, &face, csmface_t);
            i_simplify_face_loop(face, tolerances, changed, &there_are_changes);
            
            if (there_are_changes == CSMTRUE)
                break;
        }

        csmhashtb_free_iterator(&face_iterator, csmface_t);
        
    } while (there_are_changes == CSMTRUE);
}

// ----------------------------------------------------------------------------------------------------

static void i_delete_redundant_faces(struct csmsolid_t *solid, const struct csmtolerance_t *tolerances, CSMBOOL *changed)
{
    CSMBOOL there_are_changes;
    unsigned long no_iters;
    unsigned long no_deleted;
    
    assert_no_null(changed);
    
    csmdebug_print_debug_info("Deleting coplanar faces...\n");
    
    no_iters = 0;
    no_deleted = 0;
    
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
    
            if (csmface_are_coplanar_faces(face_he1, face_he2, tolerances, &same_sense) == CSMTRUE && same_sense == CSMTRUE)
            {
                if (csmface_flout(face_he1) == csmhedge_loop(he1) && csmface_flout(face_he2) == csmhedge_loop(he2))
                {
                    if (face_he1 != face_he2)
                    {
                        csmeuler_lkef(&he1, &he2);
                    }
                    else
                    {
                        struct csmloop_t *outer_loop, *new_loop;
                        double A, B, C, D, xc, yc, zc;
                        
                        outer_loop = csmface_flout(face_he1);
                        csmloop_face_equation(outer_loop, &A, &B, &C, &D, &xc, &yc, &zc);

                        csmdebug_show_face(face_he1, NULL);
                        
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
                    }
                    
                    csmface_redo_geometric_generated_data(face_he1);
                    
                    no_deleted++;
                    
                    there_are_changes = CSMTRUE;
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
    i_simplify_all_face_loops(solid, tolerances, &changed);
    i_delete_redundant_faces(solid, tolerances, &changed);
    i_delete_redundant_vertexs(solid, &changed);

    if (csmdebug_debug_enabled() == CSMTRUE)
    {
        csmdebug_print_debug_info("After simplification...\n");
        csmsolid_debug_print_debug_forced(solid);
    }
        
    if (changed == CSMTRUE)
        csmsolid_redo_geometric_face_data(solid);
}

