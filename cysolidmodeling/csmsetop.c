// Boolean operations on solids...

#include "csmsetop.h"
#include "csmsetop.tli"

#include "csmdebug.inl"
#include "csmedge.inl"
#include "csmedge.tli"
#include "csmeuler_lkfmrh.inl"
#include "csmface.inl"
#include "csmloop.inl"
#include "csmloopglue.inl"
#include "csmnode.inl"
#include "csmopbas.inl"
#include "csmsetopcom.inl"
#include "csmsetop_procedges.inl"
#include "csmsetop_vtxfacc.inl"
#include "csmsetop_vtxvtx.inl"
#include "csmsolid.h"
#include "csmsolid.inl"
#include "csmvertex.tli"

#include "a_punter.h"
#include "cyassert.h"
#include "cypespy.h"

ArrEstructura(csmsetop_vtxvtx_inters_t);
ArrEstructura(csmsetop_vtxfacc_inters_t);
ArrEstructura(csmhedge_t);

// ------------------------------------------------------------------------------------------

static CYBOOL i_can_join(
                        struct csmhedge_t *hea, struct csmhedge_t *heb,
                        ArrEstructura(csmhedge_t) *loose_ends_A,
                        ArrEstructura(csmhedge_t) *loose_ends_B,
                        struct csmhedge_t **matching_loose_end_hea, struct csmhedge_t **matching_loose_end_heb)
{
    CYBOOL can_join;
    struct csmhedge_t *matching_loose_end_hea_loc, *matching_loose_end_heb_loc;
    unsigned long i, num_loose_ends;
    
    num_loose_ends = arr_NumElemsPunteroST(loose_ends_A, csmhedge_t);
    assert(num_loose_ends == arr_NumElemsPunteroST(loose_ends_B, csmhedge_t));
    assert_no_null(matching_loose_end_hea);
    assert_no_null(matching_loose_end_heb);
    
    can_join = FALSO;
    matching_loose_end_hea_loc = NULL;
    matching_loose_end_heb_loc = NULL;
    
    for (i = 0; i < num_loose_ends; i++)
    {
        struct csmhedge_t *loose_end_a, *loose_end_b;
        
        loose_end_a = arr_GetPunteroST(loose_ends_A, i, csmhedge_t);
        loose_end_b = arr_GetPunteroST(loose_ends_B, i, csmhedge_t);
        
        if (csmsetopcom_hedges_are_neighbors(hea, loose_end_a) == CIERTO
                && csmsetopcom_hedges_are_neighbors(heb, loose_end_b) == CIERTO)
        {
            can_join = CIERTO;
            
            matching_loose_end_hea_loc = loose_end_a;
            matching_loose_end_heb_loc = loose_end_b;
            
            arr_BorrarEstructuraST(loose_ends_A, i, NULL, csmhedge_t);
            arr_BorrarEstructuraST(loose_ends_B, i, NULL, csmhedge_t);
            break;
        }
    }
    
    if (can_join == FALSO)
    {
        matching_loose_end_hea_loc = NULL;
        matching_loose_end_heb_loc = NULL;
        
        arr_AppendPunteroST(loose_ends_A, hea, csmhedge_t);
        arr_AppendPunteroST(loose_ends_B, heb, csmhedge_t);
    }
    
    *matching_loose_end_hea = matching_loose_end_hea_loc;
    *matching_loose_end_heb = matching_loose_end_heb_loc;
    
    return can_join;
}

// ----------------------------------------------------------------------------------------------------

static void i_join_null_edges(
                        struct csmsolid_t *solid_A, ArrEstructura(csmedge_t) *set_of_null_edges_A,
                        struct csmsolid_t *solid_B, ArrEstructura(csmedge_t) *set_of_null_edges_B,
                        ArrEstructura(csmface_t) **set_of_null_faces_A,
                        ArrEstructura(csmface_t) **set_of_null_faces_B)
{
    ArrEstructura(csmface_t) *set_of_null_faces_A_loc, *set_of_null_faces_B_loc;
    ArrEstructura(csmhedge_t) *loose_ends_A, *loose_ends_B;
    unsigned long i, no_null_edges;
    unsigned long no_null_edges_deleted_A, no_null_edges_deleted_B;
    
    csmsetopcom_sort_edges_lexicographically_by_xyz(set_of_null_edges_A);
    csmsetopcom_sort_edges_lexicographically_by_xyz(set_of_null_edges_B);
    no_null_edges = arr_NumElemsPunteroST(set_of_null_edges_A, csmedge_t);
    assert(no_null_edges == arr_NumElemsPunteroST(set_of_null_edges_B, csmedge_t));
    assert(no_null_edges > 0);
    assert_no_null(set_of_null_faces_A);
    assert_no_null(set_of_null_faces_B);
    
    if (csmdebug_debug_enabled() == CIERTO)
    {
        csmsetopcom_print_set_of_null_edges(set_of_null_edges_A);
        csmsetopcom_print_set_of_null_edges(set_of_null_edges_B);
    }
    
    set_of_null_faces_A_loc = arr_CreaPunteroST(0, csmface_t);
    set_of_null_faces_B_loc = arr_CreaPunteroST(0, csmface_t);
    
    loose_ends_A = arr_CreaPunteroST(0, csmhedge_t);
    loose_ends_B = arr_CreaPunteroST(0, csmhedge_t);
    
    no_null_edges_deleted_A = 0;
    no_null_edges_deleted_B = 0;
    
    csmsolid_redo_geometric_generated_data(solid_A);
    csmsolid_print_debug(solid_A, CIERTO);
    csmsolid_print_debug(solid_B, CIERTO);

    for (i = 0; i < no_null_edges; i++)
    {
        struct csmedge_t *next_edge_A, *next_edge_B;
        struct csmhedge_t *he1_next_edge_A, *he2_next_edge_A, *he1_next_edge_B, *he2_next_edge_B;
        struct csmhedge_t *h1a, *h2a, *h1b, *h2b;
        
        next_edge_A = arr_GetPunteroST(set_of_null_edges_A, i - no_null_edges_deleted_A, csmedge_t);
        next_edge_B = arr_GetPunteroST(set_of_null_edges_B, i - no_null_edges_deleted_B, csmedge_t);
        
        he1_next_edge_A = csmedge_hedge_lado(next_edge_A, CSMEDGE_LADO_HEDGE_POS);
        he2_next_edge_B = csmedge_hedge_lado(next_edge_B, CSMEDGE_LADO_HEDGE_NEG);
     
        if (i_can_join(he1_next_edge_A, he2_next_edge_B, loose_ends_A, loose_ends_B, &h1a, &h2b) == CIERTO)
        {
            csmsetopcom_join_hedges(h1a, he1_next_edge_A);
            
            if (csmsetopcom_is_loose_end(csmopbas_mate(h1a), loose_ends_A) == FALSO)
                csmsetopcom_cut_he_setop(h1a, set_of_null_edges_A, set_of_null_faces_A_loc, &no_null_edges_deleted_A);
            
            csmsetopcom_join_hedges(h2b, he2_next_edge_B);
            
            if (csmsetopcom_is_loose_end(csmopbas_mate(h2b), loose_ends_B) == FALSO)
                csmsetopcom_cut_he_setop(h2b, set_of_null_edges_B, set_of_null_faces_B_loc, &no_null_edges_deleted_B);
            
            assert(no_null_edges_deleted_A == no_null_edges_deleted_B);
        }
        else
        {
            h1a = NULL;
            h2b = NULL;
        }

        he2_next_edge_A = csmedge_hedge_lado(next_edge_A, CSMEDGE_LADO_HEDGE_NEG);
        he1_next_edge_B = csmedge_hedge_lado(next_edge_B, CSMEDGE_LADO_HEDGE_POS);
     
        if (i_can_join(he2_next_edge_A, he1_next_edge_B, loose_ends_A, loose_ends_B, &h2a, &h1b) == CIERTO)
        {
            csmsetopcom_join_hedges(h2a, he2_next_edge_A);
            
            if (csmsetopcom_is_loose_end(csmopbas_mate(h2a), loose_ends_A) == FALSO)
                csmsetopcom_cut_he_setop(h2a, set_of_null_edges_A, set_of_null_faces_A_loc, &no_null_edges_deleted_A);

            csmsetopcom_join_hedges(h1b, he1_next_edge_B);
            
            if (csmsetopcom_is_loose_end(csmopbas_mate(h1b), loose_ends_B) == FALSO)
                csmsetopcom_cut_he_setop(h1b, set_of_null_edges_B, set_of_null_faces_B_loc, &no_null_edges_deleted_B);
            
            assert(no_null_edges_deleted_A == no_null_edges_deleted_B);
        }
        else
        {
            h2a = NULL;
            h1b = NULL;
        }
        
        if (h1a != NULL && h2a != NULL && h1b != NULL && h2b != NULL)
        {
            csmsetopcom_cut_he_setop(he1_next_edge_A, set_of_null_edges_A, set_of_null_faces_A_loc, &no_null_edges_deleted_A);
            csmsetopcom_cut_he_setop(he1_next_edge_B, set_of_null_edges_B, set_of_null_faces_B_loc, &no_null_edges_deleted_B);
            assert(no_null_edges_deleted_A == no_null_edges_deleted_B);
        }
        
        if (csmdebug_debug_enabled() == CIERTO)
        {
            csmsetopcom_print_set_of_null_edges(set_of_null_edges_A);
            csmsetopcom_print_set_of_null_edges(set_of_null_edges_B);
            
            csmsetopcom_print_debug_info_loose_ends(loose_ends_A);
            csmsetopcom_print_debug_info_loose_ends(loose_ends_B);
        }
        
        csmsolid_print_debug(solid_B, CIERTO);
    }
    
    csmdebug_print_debug_info("*** AFTER JOINING NULL EDGES\n");
    
    csmsolid_print_debug(solid_A, CIERTO);
    csmsolid_print_debug(solid_B, CIERTO);
    //csmdebug_show_viewer();
    
    *set_of_null_faces_A = set_of_null_faces_A_loc;
    *set_of_null_faces_B = set_of_null_faces_B_loc;
    
    assert(arr_NumElemsPunteroST(set_of_null_edges_A, csmedge_t) == 0);
    assert(arr_NumElemsPunteroST(set_of_null_edges_B, csmedge_t) == 0);
    assert(arr_NumElemsPunteroST(loose_ends_A, csmhedge_t) == 0);
    assert(arr_NumElemsPunteroST(loose_ends_B, csmhedge_t) == 0);
    arr_DestruyeEstructurasST(&loose_ends_A, NULL, csmhedge_t);
    arr_DestruyeEstructurasST(&loose_ends_B, NULL, csmhedge_t);
}

// ----------------------------------------------------------------------------------------------------
/*
static void i_exchange_loops_of_null_faces_if_needed(ArrEstructura(csmface_t) *set_of_null_faces, CYBOOL is_setop_union)
{
    unsigned long i, num_null_faces;
    
    num_null_faces = arr_NumElemsPunteroST(set_of_null_faces, csmface_t);
    
    for (i = 0; i < num_null_faces; i++)
    {
        struct csmface_t *null_face;
        struct csmloop_t *loop1, *loop2;
        
        null_face = arr_GetPunteroST(set_of_null_faces, i, csmface_t);
        
        loop1 = csmface_floops(null_face);
        assert_no_null(loop1);
        
        loop2 = csmloop_next(loop1);
        assert_no_null(loop2);
        assert(csmloop_next(loop2) == NULL);
        
        if (csmloop_is_bounded_by_vertex_with_mask_attrib(loop1, CSMVERTEX_MASK_SETOP_VTX_FAC_CLASS) == CIERTO)
        {
            assert(csmloop_is_bounded_by_vertex_with_mask_attrib(loop2, CSMVERTEX_MASK_SETOP_VTX_FAC_CLASS) == CIERTO);
            
            if (is_setop_union == CIERTO)
            {
            }
            else
            {
                struct csmloop_t *old_flout, *new_flout;
                
                old_flout = csmface_flout(null_face);
                new_flout = (old_flout == loop1) ? loop2: loop1;
            
                csmnode_set_ptr_next(CSMNODE(loop1), NULL);
                csmnode_set_ptr_prev(CSMNODE(loop1), NULL);
            
                csmnode_set_ptr_next(CSMNODE(loop2), NULL);
                csmnode_set_ptr_prev(CSMNODE(loop2), NULL);

                csmnode_set_ptr_next(CSMNODE(loop2), CSMNODE(loop1));
                csmnode_set_ptr_prev(CSMNODE(loop1), CSMNODE(loop2));
            
                csmface_set_floops(null_face, loop2);
                csmface_set_flout(null_face, new_flout);
            
                {
                    struct csmhedge_t *ledge_old_flout;
                    struct csmface_t *other_face;
                    struct csmloop_t *other_face_floops;
                
                    ledge_old_flout = csmloop_ledge(old_flout);
                    other_face = csmopbas_face_from_hedge(csmopbas_mate(ledge_old_flout));
                
                    other_face_floops = csmface_floops(other_face);
                    assert(csmloop_next(other_face_floops) == NULL);
                
                    csmface_add_loop_while_removing_from_old(null_face, other_face_floops);
                    csmface_add_loop_while_removing_from_old(other_face, old_flout);
                }
            }
        }
        else
        {
            assert(csmloop_is_bounded_by_vertex_with_mask_attrib(loop2, CSMVERTEX_MASK_SETOP_VTX_FAC_CLASS) == FALSO);
        }
    }
}
*/

// ----------------------------------------------------------------------------------------------------

static void i_convert_inner_loops_of_null_faces_to_faces(ArrEstructura(csmface_t) *set_of_null_faces)
{
    unsigned long i, no_null_faces;
    ArrEstructura(csmface_t) *inner_loops_as_faces;
    
    no_null_faces = arr_NumElemsPunteroST(set_of_null_faces, csmface_t);
    assert(no_null_faces > 0);
    
    inner_loops_as_faces = csmsetopcom_convert_inner_loops_of_null_faces_to_faces(set_of_null_faces);
    assert(no_null_faces == arr_NumElemsPunteroST(inner_loops_as_faces, csmface_t));

    for (i = 0; i < no_null_faces; i++)
    {
        struct csmface_t *face;
        
        face = arr_GetPunteroST(inner_loops_as_faces, i, csmface_t);
        arr_AppendPunteroST(set_of_null_faces, face, csmface_t);
    }
    
    arr_DestruyeEstructurasST(&inner_loops_as_faces, NULL, csmface_t);
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmsolid_t *, i_finish_set_operation, (
                        enum csmsetop_operation_t set_operation,
                        struct csmsolid_t *solid_A, ArrEstructura(csmface_t) *set_of_null_faces_A,
                        struct csmsolid_t *solid_B, ArrEstructura(csmface_t) *set_of_null_faces_B))
{
    struct csmsolid_t *result;
    unsigned long i, no_null_faces, half_no_null_faces;
    unsigned long face_desp_a, face_desp_b;
    
    no_null_faces = arr_NumElemsPunteroST(set_of_null_faces_A, csmface_t);
    assert(no_null_faces == arr_NumElemsPunteroST(set_of_null_faces_B, csmface_t));
    assert(no_null_faces > 0);
    
    /*
    csmsolid_print_debug(solid_B, FALSO);
    switch (set_operation)
    {
        case CSMSETOP_OPERATION_UNION:
            
            i_exchange_loops_of_null_faces_if_needed(set_of_null_faces_B, CIERTO);
            break;
            
        case CSMSETOP_OPERATION_DIFFERENCE:
        case CSMSETOP_OPERATION_INTERSECTION:

            i_exchange_loops_of_null_faces_if_needed(set_of_null_faces_B, FALSO);
            break;
            
        default_error();
    }
    csmsolid_print_debug(solid_B, FALSO);
    */
    
    i_convert_inner_loops_of_null_faces_to_faces(set_of_null_faces_A);
    i_convert_inner_loops_of_null_faces_to_faces(set_of_null_faces_B);
    no_null_faces = arr_NumElemsPunteroST(set_of_null_faces_A, csmface_t);
    assert(no_null_faces == arr_NumElemsPunteroST(set_of_null_faces_B, csmface_t));
    assert(no_null_faces % 2 == 0);
    
    csmsolid_redo_geometric_generated_data(solid_A);
    csmsetopcom_reintroduce_holes_in_corresponding_faces(set_of_null_faces_A);
    
    csmsolid_redo_geometric_generated_data(solid_B);
    csmsetopcom_reintroduce_holes_in_corresponding_faces(set_of_null_faces_B);

    no_null_faces = arr_NumElemsPunteroST(set_of_null_faces_A, csmface_t);
    assert(no_null_faces == arr_NumElemsPunteroST(set_of_null_faces_B, csmface_t));
    assert(no_null_faces > 0);
    assert(no_null_faces % 2 == 0);
    
    half_no_null_faces = no_null_faces / 2;
    
    switch (set_operation)
    {
        case CSMSETOP_OPERATION_UNION:
            
            face_desp_a = 0;
            face_desp_b = 0;
            break;
            
        case CSMSETOP_OPERATION_DIFFERENCE:
            
            face_desp_a = 0;
            face_desp_b = half_no_null_faces;

            csmsolid_revert(solid_B);
            break;
            
        case CSMSETOP_OPERATION_INTERSECTION:
            
            face_desp_a = half_no_null_faces;
            face_desp_b = half_no_null_faces;            
            break;
            
        default_error();
    }
    
    csmsolid_print_debug(solid_A, FALSO);
    csmsolid_print_debug(solid_B, FALSO);

    result = csmsolid_crea_vacio(0);
    
    for (i = 0; i < half_no_null_faces; i++)
    {
        struct csmface_t *face_from_solid_A, *face_from_solid_B;
        
        face_from_solid_A = arr_GetPunteroST(set_of_null_faces_A, i + face_desp_a, csmface_t);
        csmsetopcom_move_face_to_solid(0, face_from_solid_A, solid_A, result);
        
        face_from_solid_B = arr_GetPunteroST(set_of_null_faces_B, i + face_desp_b, csmface_t);
        csmsetopcom_move_face_to_solid(0, face_from_solid_B, solid_B, result);
    }
    
    csmsetopcom_cleanup_solid_setop(solid_A, solid_B, result);
    csmsolid_print_debug(result, FALSO);
    //csmsolid_print_debug(solid_A, FALSO);
    //csmsolid_print_debug(solid_B, FALSO);

    for (i = 0; i < half_no_null_faces; i++)
    {
        struct csmface_t *face_from_solid_A, *face_from_solid_B;
        
        face_from_solid_A = arr_GetPunteroST(set_of_null_faces_A, i + face_desp_a, csmface_t);
        face_from_solid_B = arr_GetPunteroST(set_of_null_faces_B, i + face_desp_b, csmface_t);
        
        csmeuler_lkfmrh(face_from_solid_A, &face_from_solid_B);
        csmloopglue_merge_face_loops(face_from_solid_A);
    }
    
    csmdebug_set_viewer_results(result, NULL);
    //csmsolid_print_debug(result, CIERTO);
    csmdebug_show_viewer();
    
    return result;
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmsolid_t *, i_set_operation_modifying_solids, (
                        enum csmsetop_operation_t set_operation,
                        struct csmsolid_t *solid_A, struct csmsolid_t *solid_B))
{
    struct csmsolid_t *result;
    ArrEstructura(csmsetop_vtxvtx_inters_t) *vv_intersections;
    ArrEstructura(csmsetop_vtxfacc_inters_t) *vf_intersections_A, *vf_intersections_B;
    ArrEstructura(csmedge_t) *set_of_null_edges_A, *set_of_null_edges_B;
    ArrEstructura(csmface_t) *set_of_null_faces_A, *set_of_null_faces_B;
    unsigned long no_null_edges;
    
    csmdebug_begin_context("SETOP");
    csmsolid_set_name(solid_A, "Solid A");
    csmsolid_set_name(solid_B, "Solid B");
    
    csmdebug_set_viewer_results(NULL, NULL);
    csmdebug_set_viewer_parameters(solid_A, solid_B);
    csmdebug_show_viewer();
    
    csmsolid_redo_geometric_generated_data(solid_A);
    csmsolid_clear_algorithm_vertex_mask(solid_A);
    
    csmsolid_redo_geometric_generated_data(solid_B);
    csmsolid_clear_algorithm_vertex_mask(solid_B);

    if (csmdebug_debug_enabled() == CIERTO)
    {
        csmsolid_print_debug(solid_A, CIERTO);
        csmsolid_print_debug(solid_B, CIERTO);
    }
    
    csmsetop_procedges_generate_intersections_on_both_solids(
                        solid_A, solid_B,
                        &vv_intersections,
                        &vf_intersections_A, &vf_intersections_B);
    
    set_of_null_edges_A = arr_CreaPunteroST(0, csmedge_t);
    set_of_null_edges_B = arr_CreaPunteroST(0, csmedge_t);
    
    csmsetop_vtxfacc_append_null_edges(vf_intersections_A, set_operation, CSMSETOP_A_VS_B, set_of_null_edges_A, set_of_null_edges_B);
    csmsetop_vtxfacc_append_null_edges(vf_intersections_B, set_operation, CSMSETOP_B_VS_A, set_of_null_edges_B, set_of_null_edges_A);
    
    csmsolid_print_debug(solid_A, CIERTO);
    
    csmsetop_vtxvtx_append_null_edges(vv_intersections, set_operation, set_of_null_edges_A, set_of_null_edges_B);
    
    no_null_edges = arr_NumElemsPunteroST(set_of_null_edges_A, csmedge_t);
    assert(no_null_edges == arr_NumElemsPunteroST(set_of_null_edges_B, csmedge_t));
    
    if (no_null_edges == 0)
    {
        result = csmsolid_crea_vacio(0);
    }
    else
    {
        i_join_null_edges(
                        solid_A, set_of_null_edges_A,
                        solid_B, set_of_null_edges_B,
                        &set_of_null_faces_A, &set_of_null_faces_B);
    
        result = i_finish_set_operation(
                        set_operation,
                        solid_A, set_of_null_faces_A,
                        solid_B, set_of_null_faces_B);
        
        csmsolid_set_name(result, "Result");
    }
    
    csmdebug_set_viewer_results(result, NULL);
    csmdebug_end_context();
    
    arr_DestruyeEstructurasST(&vv_intersections, csmsetop_vtxvtx_free_inters, csmsetop_vtxvtx_inters_t);
    arr_DestruyeEstructurasST(&vf_intersections_A, csmsetop_vtxfacc_free_inters, csmsetop_vtxfacc_inters_t);
    arr_DestruyeEstructurasST(&vf_intersections_B, csmsetop_vtxfacc_free_inters, csmsetop_vtxfacc_inters_t);
    
    return result;
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmsolid_t *, i_set_operation, (
                        enum csmsetop_operation_t set_operation,
                        const struct csmsolid_t *solid_A, const struct csmsolid_t *solid_B))
{
    struct csmsolid_t *result;
    struct csmsolid_t *solid_A_copy, *solid_B_copy;
    
    solid_A_copy = csmsolid_duplicate(solid_A);
    solid_B_copy = csmsolid_duplicate(solid_B);
    
    result = i_set_operation_modifying_solids(set_operation, solid_A_copy, solid_B_copy);
    
    csmsolid_free(&solid_A_copy);
    csmsolid_free(&solid_B_copy);
    
    return result;
}

// ------------------------------------------------------------------------------------------

struct csmsolid_t *csmsetop_difference_A_minus_B(const struct csmsolid_t *solid_A, const struct csmsolid_t *solid_B)
{
    enum csmsetop_operation_t set_operation;
    
    set_operation = CSMSETOP_OPERATION_DIFFERENCE;
    return i_set_operation(set_operation, solid_A, solid_B);
}

// ------------------------------------------------------------------------------------------

struct csmsolid_t *csmsetop_union_A_and_B(const struct csmsolid_t *solid_A, const struct csmsolid_t *solid_B)
{
    enum csmsetop_operation_t set_operation;
    
    set_operation = CSMSETOP_OPERATION_UNION;
    return i_set_operation(set_operation, solid_A, solid_B);
}

// ------------------------------------------------------------------------------------------

struct csmsolid_t *csmsetop_intersection_A_and_B(const struct csmsolid_t *solid_A, const struct csmsolid_t *solid_B)
{
    enum csmsetop_operation_t set_operation;
    
    set_operation = CSMSETOP_OPERATION_INTERSECTION;
    return i_set_operation(set_operation, solid_A, solid_B);
}
