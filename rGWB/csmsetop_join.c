//
//  csmsetop_join.c
//  rGWB
//
//  Created by Manuel Fernández on 6/3/18.
//  Copyright © 2018 Manuel Fernández. All rights reserved.
//

#include "csmsetop_join.inl"

#include "csmarrayc.h"
#include "csmdebug.inl"
#include "csmedge.inl"
#include "csmedge.tli"
#include "csmeuler_lkef.inl"
#include "csmeuler_lmef.inl"
#include "csmface.inl"
#include "csmhashtb.inl"
#include "csmhedge.inl"
#include "csmloop.inl"
#include "csmopbas.inl"
#include "csmsetopcom.inl"
#include "csmsolid.inl"
#include "csmstring.inl"
#include "csmvertex.inl"

#ifdef __STANDALONE_DISTRIBUTABLE
#include "csmassert.inl"
#else
#include "cyassert.h"
#endif

static CSMBOOL i_EXHAUSTIVE_DEBUG = CSMFALSE;

// ------------------------------------------------------------------------------------------

static CSMBOOL i_can_join(
                        struct csmhedge_t *hea, struct csmhedge_t *heb,
                        csmArrayStruct(csmhedge_t) *loose_ends_A,
                        csmArrayStruct(csmhedge_t) *loose_ends_B,
                        struct csmhedge_t **matching_loose_end_hea, struct csmhedge_t **matching_loose_end_heb)
{
    CSMBOOL can_join;
    struct csmhedge_t *matching_loose_end_hea_loc, *matching_loose_end_heb_loc;
    unsigned long i, num_loose_ends;
    
    num_loose_ends = csmarrayc_count_st(loose_ends_A, csmhedge_t);
    assert(num_loose_ends == csmarrayc_count_st(loose_ends_B, csmhedge_t));
    assert_no_null(matching_loose_end_hea);
    assert_no_null(matching_loose_end_heb);
    
    can_join = CSMFALSE;
    matching_loose_end_hea_loc = NULL;
    matching_loose_end_heb_loc = NULL;
    
    for (i = 0; i < num_loose_ends; i++)
    {
        struct csmhedge_t *loose_end_a, *loose_end_b;
        
        loose_end_a = csmarrayc_get_st(loose_ends_A, i, csmhedge_t);
        loose_end_b = csmarrayc_get_st(loose_ends_B, i, csmhedge_t);
        
        if (csmsetopcom_hedges_are_neighbors(hea, loose_end_a) == CSMTRUE
                && csmsetopcom_hedges_are_neighbors(heb, loose_end_b) == CSMTRUE)
        {
            can_join = CSMTRUE;
            
            matching_loose_end_hea_loc = loose_end_a;
            matching_loose_end_heb_loc = loose_end_b;
            
            csmarrayc_delete_element_st(loose_ends_A, i, csmhedge_t, NULL);
            csmarrayc_delete_element_st(loose_ends_B, i, csmhedge_t, NULL);
            break;
        }
    }
    
    if (can_join == CSMFALSE)
    {
        matching_loose_end_hea_loc = NULL;
        matching_loose_end_heb_loc = NULL;
        
        csmarrayc_append_element_st(loose_ends_A, hea, csmhedge_t);
        csmarrayc_append_element_st(loose_ends_B, heb, csmhedge_t);
        
        if (csmdebug_debug_enabled() == CSMTRUE)
            csmdebug_print_debug_info("Pushed loose end pair: (%lu, %lu)\n", csmhedge_id(hea), csmhedge_id(heb));
    }
    
    *matching_loose_end_hea = matching_loose_end_hea_loc;
    *matching_loose_end_heb = matching_loose_end_heb_loc;
    
    return can_join;
}

// ----------------------------------------------------------------------------------------------------

static CSMBOOL i_there_are_only_null_edges_that_cannot_be_matched(csmArrayStruct(csmedge_t) *set_of_null_edges)
{
    unsigned long i, no_null_edges;
    
    no_null_edges = csmarrayc_count_st(set_of_null_edges, csmedge_t);
    
    for (i = 0; i < no_null_edges; i++)
    {
        unsigned long j;
        struct csmedge_t *edge_i;
        struct csmhedge_t *he1;
        
        edge_i = csmarrayc_get_st(set_of_null_edges, i, csmedge_t);
        he1 = csmedge_hedge_lado(edge_i, CSMEDGE_LADO_HEDGE_POS);
        
        for (j = 0; j < no_null_edges; j++)
        {
            struct csmedge_t *edge_j;
            struct csmhedge_t *he2;
            
            edge_j = csmarrayc_get_st(set_of_null_edges, j, csmedge_t);
            he2 = csmedge_hedge_lado(edge_j, CSMEDGE_LADO_HEDGE_NEG);
            
            if (csmsetopcom_hedges_are_neighbors(he1, he2) == CSMTRUE)
                return CSMTRUE;
        }
    }
    
    
    return CSMFALSE;
}

// ----------------------------------------------------------------------------------------------------

static void i_print_null_faces(const struct csmsolid_t *solid, const csmArrayStruct(csmface_t) *set_of_null_faces)
{
    unsigned long i, no_null_faces;
    
    no_null_faces = csmarrayc_count_st(set_of_null_faces, csmface_t);
    
    csmdebug_print_debug_info("***Null faces solid: %s\n", csmsolid_get_name(solid));
    
    for (i = 0; i < no_null_faces; i++)
    {
        const struct csmface_t *null_face;
        
        null_face = csmarrayc_get_const_st(set_of_null_faces, i, csmface_t);
        csmdebug_print_debug_info("Null face: %lu\n", csmface_id(null_face));
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_cut_he(
                    struct csmhedge_t *hedge,
                    csmArrayStruct(csmedge_t) *set_of_null_edges,
                    csmArrayStruct(csmface_t) *set_of_null_faces,
                    CSMBOOL is_solid_A,
                    unsigned long *no_null_edges_deleted,
                    CSMBOOL *null_face_created_opt)
{
    CSMBOOL null_face_created_loc;

    UNREFERENCED(is_solid_A);
    
    csmsetopcom_cut_he(hedge, set_of_null_edges, set_of_null_faces, no_null_edges_deleted, &null_face_created_loc);
    
    if (null_face_created_loc == CSMTRUE)
    {
        unsigned long no_null_faces;
        struct csmface_t *null_face;
        
        no_null_faces = csmarrayc_count_st(set_of_null_faces, csmface_t);
        assert(no_null_faces > 0);
        
        null_face = csmarrayc_get_st(set_of_null_faces, no_null_faces - 1, csmface_t);
        csmface_mark_setop_null_face(null_face);
    }
    
    ASSIGN_OPTIONAL_VALUE(null_face_created_opt, null_face_created_loc);
}

// ----------------------------------------------------------------------------------------------------

static void i_cut_he_solid_A(
                    struct csmhedge_t *hedge,
                    csmArrayStruct(csmedge_t) *set_of_null_edges,
                    csmArrayStruct(csmface_t) *set_of_null_faces,
                    unsigned long *no_null_edges_deleted,
                    CSMBOOL *null_face_created_opt)
{
    CSMBOOL is_solid_A;
    
    is_solid_A = CSMTRUE;
    i_cut_he(hedge, set_of_null_edges, set_of_null_faces, is_solid_A, no_null_edges_deleted, null_face_created_opt);
}

// ----------------------------------------------------------------------------------------------------

static void i_cut_he_solid_B(
                    struct csmhedge_t *hedge,
                    csmArrayStruct(csmedge_t) *set_of_null_edges,
                    csmArrayStruct(csmface_t) *set_of_null_faces,
                    unsigned long *no_null_edges_deleted,
                    CSMBOOL *null_face_created_opt)
{
    CSMBOOL is_solid_A;
    
    is_solid_A = CSMFALSE;
    i_cut_he(hedge, set_of_null_edges, set_of_null_faces, is_solid_A, no_null_edges_deleted, null_face_created_opt);
}

// ----------------------------------------------------------------------------------------------------

static void i_append_null_edges_to_debug_view(csmArrayStruct(csmedge_t) *set_of_null_edges)
{
    unsigned long i, no_null_edges;
    
    no_null_edges = csmarrayc_count_st(set_of_null_edges, csmedge_t);
    
    csmdebug_clear_debug_points();
    
    for (i = 0; i < no_null_edges; i++)
    {
        struct csmedge_t *null_edge;
        double x1, y1, z1, x2, y2, z2;
        char *description;
        
        null_edge = csmarrayc_get_st(set_of_null_edges, i, csmedge_t);
        assert(csmedge_setop_is_null_edge(null_edge) == CSMTRUE);
        
        csmedge_vertex_coordinates(null_edge, &x1, &y1, &z1, NULL, &x2, &y2, &z2, NULL);
        
        description = csmstring_duplicate("ne");
        csmdebug_append_debug_point(x1, y1, z1, &description);
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_validate_edges_belong_to_solid(struct csmsolid_t *solid, csmArrayStruct(csmedge_t) *set_of_null_edges)
{
    unsigned long i, no_null_edges;
    
    no_null_edges = csmarrayc_count_st(set_of_null_edges, csmedge_t);
    
    for (i = 0; i < no_null_edges; i++)
    {
        struct csmedge_t *edge;
        struct csmhedge_t *he1;
        
        edge = csmarrayc_get_st(set_of_null_edges, i, csmedge_t);
        
        he1 = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_POS);
        assert(solid == csmopbas_solid_from_hedge(he1));
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_replace_loose_end(
                        struct csmhedge_t *he_to_replace, struct csmhedge_t *he_to_replacement,
                        struct csmhedge_t **he_destination_face, CSMBOOL *he_has_been_replaced, 
                        csmArrayStruct(csmhedge_t) *loose_ends)
{
    unsigned long idx;
    
    assert(csmhedge_setop_is_loose_end(he_to_replacement) == CSMFALSE);
    assert_no_null(he_destination_face);
    assert_no_null(he_has_been_replaced);
    
    if (csmarrayc_contains_element_st(loose_ends, csmhedge_t, he_to_replace, struct csmhedge_t, csmhedge_equal_id, &idx) == CSMTRUE)
    {
        csmhedge_setop_set_loose_end(he_to_replacement, CSMTRUE);
        csmarrayc_set_st(loose_ends, idx, he_to_replacement, csmhedge_t);
    }

    if (he_to_replace == *he_destination_face)
    {
        *he_destination_face = he_to_replacement;
        *he_has_been_replaced = CSMTRUE;
    }
}

// ----------------------------------------------------------------------------------------------------

static CSMBOOL i_make_reachable_end_vertex_he1_occur_in_face_of_he2(
                        struct csmhedge_t *he1, struct csmhedge_t **he2,
                        CSMBOOL *he2_has_been_replaced,
                        csmArrayStruct(csmhedge_t) *loose_ends)
{
    struct csmvertex_t *he1_end_vertex;
    struct csmface_t *he2_face;
    
    assert_no_null(he2);
    assert_no_null(he2_has_been_replaced);

    he1_end_vertex = csmhedge_vertex(csmopbas_mate(he1));
    he2_face = csmopbas_face_from_hedge(*he2);

    if (csmface_is_vertex_used_by_hedge_on_face(he2_face, he1_end_vertex) == CSMFALSE)
    {
        *he2_has_been_replaced = CSMFALSE;
        return CSMFALSE;
    }
    else
    {
        unsigned long no_iters;

        csmdebug_print_debug_info("[End vertex he1 in face he2] Moving %lu to face of %lu\n", csmhedge_id(he1), csmhedge_id(*he2));
        
        no_iters = 0;
        *he2_has_been_replaced = CSMFALSE;

        while (csmopbas_face_from_hedge(he1) != he2_face)
        {
            struct csmhedge_t *he1_next, *he1_next_mate, *he1_next_next;
            
            assert(no_iters < 10000);
            no_iters++;
        
            he1_next = csmhedge_next(he1);
            he1_next_mate = csmopbas_mate(he1_next);
        
            he1_next_next = csmhedge_next(he1_next);
        
            csmeuler_lmef(he1, he1_next_next, NULL, NULL, NULL);
            assert(he1_next == csmhedge_next(he1));
            
            if (csmhedge_setop_is_loose_end(he1_next_mate) == CSMTRUE)
            {
                struct csmhedge_t *he_mate_null_edge_replacement;
                
                he_mate_null_edge_replacement = csmopbas_mate(csmhedge_prev(he1));
                i_replace_loose_end(he1_next_mate, he_mate_null_edge_replacement, he2, he2_has_been_replaced, loose_ends);
            }

            if (csmhedge_setop_is_loose_end(he1_next) == CSMTRUE)
            {
                struct csmhedge_t *he_mate_null_edge_replacement;
                
                he_mate_null_edge_replacement = csmhedge_prev(he1);
                i_replace_loose_end(he1_next, he_mate_null_edge_replacement, he2, he2_has_been_replaced, loose_ends);
            }

            csmeuler_lkef(&he1_next_mate, &he1_next);

            if (*he2_has_been_replaced == CSMTRUE)
                break;
        }

        return csmsetopcom_hedges_are_neighbors(he1, *he2);
    }
}

// ----------------------------------------------------------------------------------------------------

static CSMBOOL i_make_reachable_start_vertex_he1_occur_in_face_of_he2(
                        struct csmhedge_t *he1, struct csmhedge_t **he2,
                        CSMBOOL *he2_has_been_replaced,
                        csmArrayStruct(csmhedge_t) *loose_ends)
{
    struct csmvertex_t *he1_start_vertex;
    struct csmface_t *he2_face;
    
    assert_no_null(he2);
    assert_no_null(he2_has_been_replaced);

    he1_start_vertex = csmhedge_vertex(he1);
    he2_face = csmopbas_face_from_hedge(*he2);

    if (csmface_is_vertex_used_by_hedge_on_face(he2_face, he1_start_vertex) == CSMFALSE)
    {
        *he2_has_been_replaced = CSMFALSE;
        return CSMFALSE;
    }
    else
    {
        unsigned long no_iters;
        
        csmdebug_print_debug_info("[Start vertex he1 in face he2] Moving %lu to face of %lu\n", csmhedge_id(he1), csmhedge_id(*he2));
        
        no_iters = 0;
        *he2_has_been_replaced = CSMFALSE;
        
        while (csmopbas_face_from_hedge(he1) != he2_face)
        {
            struct csmhedge_t *he1_prev, *he1_prev_mate, *he1_next;
        
            assert(no_iters < 10000);
            no_iters++;
            
            he1_prev = csmhedge_prev(he1);
            he1_prev_mate = csmopbas_mate(he1_prev);
        
            he1_next = csmhedge_next(he1);
        
            csmeuler_lmef(he1_prev, he1_next, NULL, NULL, NULL);
            assert(he1_prev == csmhedge_prev(he1));
            
            if (csmhedge_setop_is_loose_end(he1_prev_mate) == CSMTRUE)
            {
                struct csmhedge_t *he_mate_null_edge_replacement;
                
                he_mate_null_edge_replacement = csmopbas_mate(csmhedge_next(he1));
                i_replace_loose_end(he1_prev_mate, he_mate_null_edge_replacement, he2, he2_has_been_replaced, loose_ends);
            }

            if (csmhedge_setop_is_loose_end(he1_prev) == CSMTRUE)
            {
                struct csmhedge_t *he_mate_null_edge_replacement;
                
                he_mate_null_edge_replacement = csmhedge_next(he1);
                i_replace_loose_end(he1_prev, he_mate_null_edge_replacement, he2, he2_has_been_replaced, loose_ends);
            }

            csmeuler_lkef(&he1_prev_mate, &he1_prev);

            if (*he2_has_been_replaced == CSMTRUE)
                break;
        }

        return csmsetopcom_hedges_are_neighbors(he1, *he2);
    }
}

// ----------------------------------------------------------------------------------------------------

static CSMBOOL i_could_make_hedges_reachable(
                        struct csmhedge_t *he1, struct csmhedge_t **he2,
                        CSMBOOL *he2_has_been_replaced,
                        csmArrayStruct(csmhedge_t) *loose_ends)
{
    assert_no_null(he2);
    assert_no_null(he2_has_been_replaced);

    *he2_has_been_replaced = CSMFALSE;

    if (csmhedge_edge(he1) == csmhedge_edge(*he2))
    {   
        return CSMFALSE;
    }
    else
    {
        enum csmedge_lado_hedge_t side_he1, side_he2;
        
        side_he1 = csmedge_hedge_side(csmhedge_edge(he1), he1);
        side_he2 = csmedge_hedge_side(csmhedge_edge(*he2), *he2);
        
        if (side_he1 == side_he2)
        {
            return CSMFALSE;
        }
        else if (i_make_reachable_end_vertex_he1_occur_in_face_of_he2(he1, he2, he2_has_been_replaced, loose_ends) == CSMTRUE)
        {
            return CSMTRUE;
        }
        else if (i_make_reachable_start_vertex_he1_occur_in_face_of_he2(he1, he2, he2_has_been_replaced, loose_ends) == CSMTRUE)
        {
            return CSMTRUE;
        }
        else
        {
            return CSMFALSE;
        }
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_remove_loose_ends_by_idx(unsigned long idx1, unsigned long idx2, csmArrayStruct(csmhedge_t) *loose_ends)
{
    unsigned long idx_min, idx_max;
    
    idx_max = CSMMATH_MAX(idx1, idx2);
    csmarrayc_delete_element_st(loose_ends, idx_max, csmhedge_t, NULL);
    
    idx_min = CSMMATH_MIN(idx1, idx2);
    csmarrayc_delete_element_st(loose_ends, idx_min, csmhedge_t, NULL);
}

// ----------------------------------------------------------------------------------------------------

static void i_join_pendant_loose_ends_by_modifying_topology(
                        struct csmsolid_t *solid_A, csmArrayStruct(csmedge_t) *set_of_null_edges_A, csmArrayStruct(csmhedge_t) *loose_ends_A,
                        struct csmsolid_t *solid_B, csmArrayStruct(csmedge_t) *set_of_null_edges_B, csmArrayStruct(csmhedge_t) *loose_ends_B,
                        const struct csmtolerance_t *tolerances,
                        unsigned long *no_null_edges_deleted_A, unsigned long *no_null_edges_deleted_B,
                        csmArrayStruct(csmface_t) *set_of_null_faces_A,
                        csmArrayStruct(csmface_t) *set_of_null_faces_B)
{
    unsigned long no_iters;
    CSMBOOL there_are_changes;
    
    assert_no_null(no_null_edges_deleted_A);
    assert_no_null(no_null_edges_deleted_B);
    
    no_iters = 0;
    there_are_changes = CSMFALSE;
    
    do
    {
        unsigned long i, no_loose_ends;
        
        no_loose_ends = csmarrayc_count_st(loose_ends_A, csmhedge_t);
        assert(no_loose_ends == csmarrayc_count_st(loose_ends_B, csmhedge_t));
        assert(csmarrayc_count_st(set_of_null_faces_A, csmface_t) == csmarrayc_count_st(set_of_null_faces_B, csmface_t));
        //assert(*no_null_edges_deleted_A == *no_null_edges_deleted_B);
        assert(no_iters < 10000);
        no_iters++;
        
        if (csmdebug_debug_enabled() == CSMTRUE)
        {
            csmdebug_print_debug_info("[i_join_pendant_loose_ends_by_modifying_topology] Iteration: %lu\n", no_iters);
            
            csmsetopcom_print_set_of_null_edges(set_of_null_edges_A, loose_ends_A);
            csmsetopcom_print_set_of_null_edges(set_of_null_edges_B, loose_ends_B);
            
            csmsetopcom_print_debug_info_loose_ends(loose_ends_A);
            csmsetopcom_print_debug_info_loose_ends(loose_ends_B);
            
            csmdebug_print_debug_info("*** joining loose ends by modifying topology\n");
            csmsolid_print_debug(solid_A, CSMTRUE);
            csmsolid_print_debug(solid_B, CSMTRUE);
            
            //csmdebug_show_viewer();
        }
        
        there_are_changes = CSMFALSE;
        
        for (i = 0; i < no_loose_ends; i++)
        {
            struct csmhedge_t *loose_end_a_i, *loose_end_b_i;
            unsigned long j;
            
            loose_end_a_i = csmarrayc_get_st(loose_ends_A, i, csmhedge_t);
            loose_end_b_i = csmarrayc_get_st(loose_ends_B, i, csmhedge_t);
            
            for (j = 0; j < no_loose_ends; j++)
            {
                if (i != j)
                {
                    struct csmhedge_t *loose_end_a_j, *loose_end_b_j;
                    CSMBOOL reachable_a, reachable_b;
                    CSMBOOL all_hedges_reachable;
                    CSMBOOL he2_has_been_replaced_A, he2_has_been_replaced_B;
                    
                    loose_end_a_j = csmarrayc_get_st(loose_ends_A, j, csmhedge_t);
                    loose_end_b_j = csmarrayc_get_st(loose_ends_B, j, csmhedge_t);
                    
                    reachable_a = csmsetopcom_hedges_are_neighbors(loose_end_a_i, loose_end_a_j);
                    reachable_b = csmsetopcom_hedges_are_neighbors(loose_end_b_i, loose_end_b_j);
                    
                    if (reachable_a == CSMTRUE && reachable_b == CSMTRUE)
                    {
                        all_hedges_reachable = CSMTRUE;
                        he2_has_been_replaced_A = CSMFALSE;
                        he2_has_been_replaced_B = CSMFALSE;
                    }
                    else if (reachable_a == CSMTRUE && reachable_b == CSMFALSE)
                    {
                        csmdebug_print_debug_info("Analyzing edges of solid B\n");

                        all_hedges_reachable = i_could_make_hedges_reachable(loose_end_b_i, &loose_end_b_j, &he2_has_been_replaced_B, loose_ends_B);
                        he2_has_been_replaced_A = CSMFALSE;
                    }
                    else if (reachable_a == CSMFALSE && reachable_b == CSMTRUE)
                    {
                        csmdebug_print_debug_info("Analyzing edges of solid A\n");

                        all_hedges_reachable = i_could_make_hedges_reachable(loose_end_a_i, &loose_end_a_j, &he2_has_been_replaced_A, loose_ends_A);
                        he2_has_been_replaced_B = CSMFALSE;
                    }
                    else
                    {
                        all_hedges_reachable = CSMFALSE;
                        he2_has_been_replaced_A = CSMFALSE;
                        he2_has_been_replaced_B = CSMFALSE;
                    }
                    
                    if (all_hedges_reachable == CSMTRUE)
                    {
                        assert(csmsetopcom_hedges_are_neighbors(loose_end_a_i, loose_end_a_j) == CSMTRUE);
                        assert(csmsetopcom_hedges_are_neighbors(loose_end_b_i, loose_end_b_j) == CSMTRUE);
                        
                        i_remove_loose_ends_by_idx(i, j, loose_ends_A);
                        i_remove_loose_ends_by_idx(i, j, loose_ends_B);
                        
                        csmdebug_print_debug_info("loose_end_a_i, loose_end_a_j\n");
                        csmsetopcom_join_hedges(loose_end_a_i, loose_end_a_j, tolerances);
            
                        if (csmsetopcom_is_loose_end(csmopbas_mate(loose_end_a_i), loose_ends_A) == CSMFALSE)
                            i_cut_he_solid_A(loose_end_a_i, set_of_null_edges_A, set_of_null_faces_A, no_null_edges_deleted_A, NULL);

                        if (csmsetopcom_is_loose_end(csmopbas_mate(loose_end_a_j), loose_ends_A) == CSMFALSE)
                            i_cut_he_solid_A(loose_end_a_j, set_of_null_edges_A, set_of_null_faces_A, no_null_edges_deleted_A, NULL);

                        csmdebug_print_debug_info("loose_end_b_i, loose_end_b_j\n");
                        csmsetopcom_join_hedges(loose_end_b_i, loose_end_b_j, tolerances);
            
                        if (csmsetopcom_is_loose_end(csmopbas_mate(loose_end_b_i), loose_ends_B) == CSMFALSE)
                            i_cut_he_solid_B(loose_end_b_i, set_of_null_edges_B, set_of_null_faces_B, no_null_edges_deleted_B, NULL);
                        
                        if (csmsetopcom_is_loose_end(csmopbas_mate(loose_end_b_j), loose_ends_B) == CSMFALSE)
                            i_cut_he_solid_B(loose_end_b_j, set_of_null_edges_B, set_of_null_faces_B, no_null_edges_deleted_B, NULL);
                        
                        //if (csmdebug_debug_enabled() == CSMTRUE)
                            //csmdebug_show_viewer();
                        
                        there_are_changes = CSMTRUE;
                        break;
                    }
                    else
                    {
                        if (he2_has_been_replaced_A == CSMTRUE || he2_has_been_replaced_B == CSMTRUE)
                        {
                            there_are_changes = CSMTRUE;
                            break;
                        }
                    }
                }
            }
            
            if (there_are_changes == CSMTRUE)
                break;
        }
        
    } while (there_are_changes == CSMTRUE);
    
    if (csmdebug_debug_enabled() == CSMTRUE)
    {
        if (csmarrayc_count_st(loose_ends_A, csmhedge_t) > 0 || csmarrayc_count_st(loose_ends_B, csmhedge_t) > 0)
        {
            csmsetopcom_print_set_of_null_edges(set_of_null_edges_A, loose_ends_A);
            csmsetopcom_print_set_of_null_edges(set_of_null_edges_B, loose_ends_B);
            
            csmsetopcom_print_debug_info_loose_ends(loose_ends_A);
            csmsetopcom_print_debug_info_loose_ends(loose_ends_B);
            
            //csmdebug_show_viewer();
        }
    }
}

// ----------------------------------------------------------------------------------------------------

static CSMBOOL i_null_face_has_only_two_loops(struct csmface_t *null_face)
{
    struct csmloop_t *loop1, *loop2;

    loop1 = csmface_floops(null_face);
    assert_no_null(loop1);

    loop2 = csmloop_next(loop1);
    assert_no_null(loop2);
    
    if (csmloop_next(loop2) == NULL)
        return CSMTRUE;
    else
        return CSMFALSE;
}

// ------------------------------------------------------------------------------------------

static CSMBOOL i_null_faces_are_correct(csmArrayStruct(csmface_t) *set_of_null_faces)
{
    unsigned long i, no_null_faces;

    no_null_faces = csmarrayc_count_st(set_of_null_faces, csmface_t);

    for (i = 0; i < no_null_faces; i++)
    {
        struct csmface_t *null_face;
        
        null_face = csmarrayc_get_st(set_of_null_faces, i, csmface_t);

        if (i_null_face_has_only_two_loops(null_face) == CSMFALSE)
            return CSMFALSE;
    }

    return CSMTRUE;
}

// ----------------------------------------------------------------------------------------------------

void csmsetop_join_null_edges(
                        struct csmsolid_t *solid_A, csmArrayStruct(csmedge_t) *set_of_null_edges_A,
                        struct csmsolid_t *solid_B, csmArrayStruct(csmedge_t) *set_of_null_edges_B,
                        const struct csmtolerance_t *tolerances,
                        csmArrayStruct(csmface_t) **set_of_null_faces_A,
                        csmArrayStruct(csmface_t) **set_of_null_faces_B,
                        CSMBOOL *did_join_all_null_edges)
{
    csmArrayStruct(csmface_t) *set_of_null_faces_A_loc, *set_of_null_faces_B_loc;
    CSMBOOL did_join_all_null_edges_loc;
    csmArrayStruct(csmhedge_t) *loose_ends_A, *loose_ends_B;
    unsigned long i, no_null_edges;
    unsigned long no_null_edges_deleted_A, no_null_edges_deleted_B;
    unsigned long no_null_edges_pendant;
    
    assert_no_null(did_join_all_null_edges);
    
    i_validate_edges_belong_to_solid(solid_A, set_of_null_edges_A);
    i_validate_edges_belong_to_solid(solid_B, set_of_null_edges_B);
    
    csmsetopcom_sort_pair_edges_lexicographically_by_xyz(set_of_null_edges_A, set_of_null_edges_B, tolerances);
    no_null_edges = csmarrayc_count_st(set_of_null_edges_A, csmedge_t);
    assert(no_null_edges == csmarrayc_count_st(set_of_null_edges_B, csmedge_t));
    assert(no_null_edges > 0);
    assert_no_null(set_of_null_faces_A);
    assert_no_null(set_of_null_faces_B);
    
    set_of_null_faces_A_loc = csmarrayc_new_st_array(0, csmface_t);
    set_of_null_faces_B_loc = csmarrayc_new_st_array(0, csmface_t);
    
    loose_ends_A = csmarrayc_new_st_array(0, csmhedge_t);
    loose_ends_B = csmarrayc_new_st_array(0, csmhedge_t);
    
    no_null_edges_deleted_A = 0;
    no_null_edges_deleted_B = 0;
    
    if (csmdebug_debug_enabled() == CSMTRUE)
    {
        csmsolid_redo_geometric_face_data(solid_A);
        csmsolid_print_debug(solid_A, CSMTRUE);
        
        csmsolid_redo_geometric_face_data(solid_B);
        csmsolid_print_debug(solid_B, CSMTRUE);
        
        i_append_null_edges_to_debug_view(set_of_null_edges_A);
        csmdebug_show_viewer();
        csmdebug_clear_debug_points();
        csmdebug_clear_segments();
    }
    
    //csmdebug_block_print_solid();
    
    for (i = 0; i < no_null_edges; i++)
    {
        CSMBOOL null_face_created_h1a, null_face_created_h2b;
        CSMBOOL null_face_created_h2a, null_face_created_h1b;
        CSMBOOL null_face_created_h12a, null_face_created_h12b;
        struct csmedge_t *next_edge_A, *next_edge_B;
        struct csmhedge_t *he1_next_edge_A, *he2_next_edge_A, *he1_next_edge_B, *he2_next_edge_B;
        struct csmhedge_t *h1a, *h2a, *h1b, *h2b;
        
        if (csmdebug_debug_enabled() == CSMTRUE)
        {
            csmdebug_print_debug_info("Join_null_Edges Iteration: %lu\n", i);
            
            csmsetopcom_print_set_of_null_edges(set_of_null_edges_A, loose_ends_A);
            csmsetopcom_print_set_of_null_edges(set_of_null_edges_B, loose_ends_B);
            
            csmsetopcom_print_debug_info_loose_ends(loose_ends_A);
            csmsetopcom_print_debug_info_loose_ends(loose_ends_B);
            
            //csmsetopcom_print_debug_info_faces_null_edges("Solid A", set_of_null_edges_A);
            //csmsetopcom_print_debug_info_faces_null_edges("Solid B", set_of_null_edges_B);
        }

        null_face_created_h1a = CSMFALSE;
        null_face_created_h2b = CSMFALSE;
        null_face_created_h2a = CSMFALSE;
        null_face_created_h1b = CSMFALSE;
        null_face_created_h12a = CSMFALSE;
        null_face_created_h12b = CSMFALSE;
        
        next_edge_A = csmarrayc_get_st(set_of_null_edges_A, i - no_null_edges_deleted_A, csmedge_t);
        next_edge_B = csmarrayc_get_st(set_of_null_edges_B, i - no_null_edges_deleted_B, csmedge_t);
        
        he1_next_edge_A = csmedge_hedge_lado(next_edge_A, CSMEDGE_LADO_HEDGE_POS);
        he2_next_edge_B = csmedge_hedge_lado(next_edge_B, CSMEDGE_LADO_HEDGE_NEG);
     
        if (i_can_join(he1_next_edge_A, he2_next_edge_B, loose_ends_A, loose_ends_B, &h1a, &h2b) == CSMTRUE)
        {
            csmdebug_print_debug_info("Joining h1a, he1_next_edge_A: Iter %lu (%lu, %lu)\n", i, csmhedge_id(h1a), csmhedge_id(he1_next_edge_A));
            csmsetopcom_join_hedges(h1a, he1_next_edge_A, tolerances);
            
            if (csmsetopcom_is_loose_end(csmopbas_mate(h1a), loose_ends_A) == CSMFALSE)
                i_cut_he_solid_A(h1a, set_of_null_edges_A, set_of_null_faces_A_loc, &no_null_edges_deleted_A, &null_face_created_h1a);
            
            csmdebug_print_debug_info("Joining h2b, he2_next_edge_B: Iter %lu (%lu, %lu)\n", i, csmhedge_id(h2b), csmhedge_id(he2_next_edge_B));
            csmsetopcom_join_hedges(h2b, he2_next_edge_B, tolerances);
            
            if (csmsetopcom_is_loose_end(csmopbas_mate(h2b), loose_ends_B) == CSMFALSE)
                i_cut_he_solid_B(h2b, set_of_null_edges_B, set_of_null_faces_B_loc, &no_null_edges_deleted_B, &null_face_created_h2b);

            if (i_EXHAUSTIVE_DEBUG == CSMTRUE)
            {
                if (csmdebug_debug_enabled() == CSMTRUE || csmdebug_debug_visual_enabled() == CSMTRUE)
                    csmdebug_show_viewer();
            }
            
            assert(no_null_edges_deleted_A == no_null_edges_deleted_B);
        }
        else
        {
            h1a = NULL;
            h2b = NULL;
        }

        he2_next_edge_A = csmedge_hedge_lado(next_edge_A, CSMEDGE_LADO_HEDGE_NEG);
        he1_next_edge_B = csmedge_hedge_lado(next_edge_B, CSMEDGE_LADO_HEDGE_POS);
     
        if (i_can_join(he2_next_edge_A, he1_next_edge_B, loose_ends_A, loose_ends_B, &h2a, &h1b) == CSMTRUE)
        {
            csmdebug_print_debug_info("Joining h2a, he2_next_edge_A: Iter %lu (%lu, %lu)\n", i, csmhedge_id(h2a), csmhedge_id(he2_next_edge_A));
            csmsetopcom_join_hedges(h2a, he2_next_edge_A, tolerances);
            
            if (csmsetopcom_is_loose_end(csmopbas_mate(h2a), loose_ends_A) == CSMFALSE)
                i_cut_he_solid_A(h2a, set_of_null_edges_A, set_of_null_faces_A_loc, &no_null_edges_deleted_A, &null_face_created_h2a);

            csmdebug_print_debug_info("Joining h1b, he1_next_edge_B: Iter %lu (%lu, %lu)\n", i, csmhedge_id(h1b), csmhedge_id(he1_next_edge_B));
            csmsetopcom_join_hedges(h1b, he1_next_edge_B, tolerances);
            
            if (csmsetopcom_is_loose_end(csmopbas_mate(h1b), loose_ends_B) == CSMFALSE)
                i_cut_he_solid_B(h1b, set_of_null_edges_B, set_of_null_faces_B_loc, &no_null_edges_deleted_B, &null_face_created_h1b);
         
            if (i_EXHAUSTIVE_DEBUG == CSMTRUE)
            {
                if (csmdebug_debug_enabled() == CSMTRUE || csmdebug_debug_visual_enabled() == CSMTRUE)
                    csmdebug_show_viewer();
            }
            
            assert(no_null_edges_deleted_A == no_null_edges_deleted_B);
        }
        else
        {
            h2a = NULL;
            h1b = NULL;
        }
        
        if (h1a != NULL && h2a != NULL && h1b != NULL && h2b != NULL)
        {
            csmdebug_print_debug_info("Joining h1a, h2a, h1b, h2b: Iter %lu\n", i);

            i_cut_he_solid_A(he1_next_edge_A, set_of_null_edges_A, set_of_null_faces_A_loc, &no_null_edges_deleted_A, &null_face_created_h12a);
            i_cut_he_solid_B(he1_next_edge_B, set_of_null_edges_B, set_of_null_faces_B_loc, &no_null_edges_deleted_B, &null_face_created_h12b);
            assert(no_null_edges_deleted_A == no_null_edges_deleted_B);
        }
        
        /*
        if (csmdebug_debug_enabled() == CSMTRUE)
        {
            if (null_face_created_h1a == CSMTRUE || null_face_created_h1b || null_face_created_h12a == CSMTRUE
                    || null_face_created_h1b == CSMTRUE || null_face_created_h2b == CSMTRUE || null_face_created_h12b == CSMTRUE)
            {
                csmdebug_unblock_print_solid();
                    csmdebug_print_debug_info("*** AFTER NULL FACES\n");
                    csmsolid_print_debug(solid_A, CSMTRUE);
                    csmsolid_print_debug(solid_B, CSMTRUE);
                csmdebug_block_print_solid();
                
                csmsetopcom_print_set_of_null_edges(set_of_null_edges_A, loose_ends_A);
                csmsetopcom_print_set_of_null_edges(set_of_null_edges_B, loose_ends_B);
                csmsetopcom_print_debug_info_loose_ends(loose_ends_A);
                csmsetopcom_print_debug_info_loose_ends(loose_ends_B);
                
                //csmdebug_show_viewer();
            }
        }
        */
    }
    
    i_join_pendant_loose_ends_by_modifying_topology(
                        solid_A, set_of_null_edges_A, loose_ends_A,
                        solid_B, set_of_null_edges_B, loose_ends_B,
                        tolerances,
                        &no_null_edges_deleted_A, &no_null_edges_deleted_B,
                        set_of_null_faces_A_loc,
                        set_of_null_faces_B_loc);
    
    csmdebug_unblock_print_solid();
    
    if (csmdebug_debug_enabled() == CSMTRUE)
    {
        csmdebug_set_viewer_parameters(solid_A, solid_B);
        csmdebug_show_viewer();
        
        i_print_null_faces(solid_A, set_of_null_faces_A_loc);
        i_print_null_faces(solid_B, set_of_null_faces_B_loc);
    }
    
    csmsetopcom_postprocess_join_edges(solid_A);
    csmsetopcom_postprocess_join_edges(solid_B);
    
    csmdebug_print_debug_info("*** AFTER JOINING NULL EDGES\n");
    csmsolid_print_debug(solid_A, CSMTRUE);
    csmsolid_print_debug(solid_B, CSMTRUE);
    //csmdebug_show_viewer();
    
    no_null_edges_pendant = csmarrayc_count_st(set_of_null_edges_A, csmedge_t);
    assert(no_null_edges_pendant == csmarrayc_count_st(set_of_null_edges_B, csmedge_t));
    
    if (no_null_edges_pendant == 0)
    {
        did_join_all_null_edges_loc = CSMTRUE;
        
        assert(csmarrayc_count_st(loose_ends_A, csmhedge_t) == 0);
        assert(csmarrayc_count_st(loose_ends_B, csmhedge_t) == 0);
    }
    else
    {
        CSMBOOL null_edges_that_cannot_be_matched_A, null_edges_that_cannot_be_matched_B;
        
        if (csmdebug_debug_enabled() == CSMTRUE)
        {
            csmdebug_print_debug_info("*****Pendant null edges!!!! Maybe the only ones in its faces\n");
        
            csmsetopcom_print_set_of_null_edges(set_of_null_edges_A, loose_ends_A);
            csmsetopcom_print_set_of_null_edges(set_of_null_edges_B, loose_ends_B);
            
            csmsetopcom_print_debug_info_loose_ends(loose_ends_A);
            csmsetopcom_print_debug_info_loose_ends(loose_ends_B);
        }
        
        /*
         Some null edges are the only ones in its faces, so nothing can be done with them.
         Maybe a non-manifold result (i.e. an edge like a line in one face connected to a hole generated by the other solid)
         
         */
        null_edges_that_cannot_be_matched_A = i_there_are_only_null_edges_that_cannot_be_matched(set_of_null_edges_A);
        null_edges_that_cannot_be_matched_B = i_there_are_only_null_edges_that_cannot_be_matched(set_of_null_edges_B);
     
        if (null_edges_that_cannot_be_matched_A == CSMTRUE || null_edges_that_cannot_be_matched_B == CSMTRUE)
            did_join_all_null_edges_loc = CSMFALSE;
        else
            did_join_all_null_edges_loc = CSMTRUE;
        
        /*
         if (csmdebug_get_treat_improper_solid_operations_as_errors() == CSMTRUE)
            assert(null_edges_that_cannot_be_matched_A == CSMTRUE || null_edges_that_cannot_be_matched_B == CSMTRUE);
         */
    }
    
    if (did_join_all_null_edges_loc == CSMTRUE)
    {
        if (i_null_faces_are_correct(set_of_null_faces_A_loc) == CSMFALSE || i_null_faces_are_correct(set_of_null_faces_B_loc) == CSMFALSE)
        {
            did_join_all_null_edges_loc = CSMFALSE;

            csmarrayc_free_st(&set_of_null_faces_A_loc, csmface_t, NULL);
            set_of_null_faces_A_loc = csmarrayc_new_st_array(0, csmface_t);

            csmarrayc_free_st(&set_of_null_faces_B_loc, csmface_t, NULL);
            set_of_null_faces_B_loc = csmarrayc_new_st_array(0, csmface_t);
        }
    }
    
    *set_of_null_faces_A = set_of_null_faces_A_loc;
    *set_of_null_faces_B = set_of_null_faces_B_loc;
    *did_join_all_null_edges = did_join_all_null_edges_loc;
    
    csmarrayc_free_st(&loose_ends_A, csmhedge_t, NULL);
    csmarrayc_free_st(&loose_ends_B, csmhedge_t, NULL);
}

