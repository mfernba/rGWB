// Boolean operations on solids...

#include "csmsetop.h"
#include "csmsetop.tli"

#include "csmarrayc.inl"
#include "csmassert.inl"
#include "csmbbox.inl"
#include "csmdebug.inl"
#include "csmedge.inl"
#include "csmedge.tli"
#include "csmeuler_lkfmrh.inl"
#include "csmeuler_lmfkrh.inl"
#include "csmface.inl"
#include "csmhashtb.inl"
#include "csmhedge.inl"
#include "csmloop.inl"
#include "csmloopglue.inl"
#include "csmmaterial.inl"
#include "csmmath.inl"
#include "csmmem.inl"
#include "csmnode.inl"
#include "csmopbas.inl"
#include "csmsetop_join.inl"
#include "csmsetop_procedges.inl"
#include "csmsetop_vtxfacc.inl"
#include "csmsetop_vtxvtx.inl"
#include "csmsetopcom.inl"
#include "csmsolid.h"
#include "csmsolid.inl"
#include "csmsolid_debug.inl"
#include "csmstring.inl"
#include "csmtolerance.inl"
#include "csmvertex.inl"
#include "csmvertex.tli"

static const unsigned long i_NUM_MAX_PERTURBATIONS = 10;

// ----------------------------------------------------------------------------------------------------

static struct csmloop_t *i_get_in_component_of_null_face(struct csmface_t *null_face)
{
    struct csmloop_t *loop1, *loop2;
    struct csmloop_t *flout;

    loop1 = csmface_floops(null_face);
    assert_no_null(loop1);
    
    loop2 = csmloop_next(loop1);
    assert_no_null(loop2);
    assert(csmloop_next(loop2) == NULL);
    
    flout = csmface_flout(null_face);
    
    if (flout == loop1)
    {
        return loop2;
    }
    else
    {
        assert(flout == loop2);
        return loop1;
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_convert_holes_attached_to_in_componente_in_faces(struct csmface_t *null_face)
{
    struct csmloop_t *in_component;
    struct csmhedge_t *ledge, *he_iterator;
    unsigned long no_iterations;
    
    in_component = i_get_in_component_of_null_face(null_face);
    
    ledge = csmloop_ledge(in_component);
    he_iterator = csmloop_ledge(in_component);
    no_iterations = 0;
    
    do
    {
        struct csmhedge_t *hedge_iterator_mate;
        struct csmloop_t *hedge_iterator_mate_loop;
        struct csmface_t *hedge_iterator_mate_face;
        
        assert(no_iterations < 10000);
        no_iterations++;
        
        hedge_iterator_mate = csmopbas_mate(he_iterator);
        
        hedge_iterator_mate_loop = csmhedge_loop(hedge_iterator_mate);
        hedge_iterator_mate_face = csmloop_lface(hedge_iterator_mate_loop);
        
        if (hedge_iterator_mate_loop != csmface_flout(hedge_iterator_mate_face))
        {
            csmdebug_print_debug_info("in_component...(): Null face %lu, converting loop %lu to face.\n", csmface_id(null_face), csmloop_id(hedge_iterator_mate_loop));
            csmeuler_lmfkrh(hedge_iterator_mate_loop, NULL);
        }
        
        he_iterator = csmhedge_next(he_iterator);
        
    } while (he_iterator != ledge);
}

// ----------------------------------------------------------------------------------------------------

static void i_convert_holes_attached_to_in_component_of_null_faces_in_faces(csmArrayStruct(csmface_t) *set_of_null_faces)
{
    unsigned long i, num_null_faces;
    
    num_null_faces = csmarrayc_count_st(set_of_null_faces, csmface_t);
    
    for (i = 0; i < num_null_faces; i++)
    {
        struct csmface_t *null_face;
        
        null_face = csmarrayc_get_st(set_of_null_faces, i, csmface_t);
        i_convert_holes_attached_to_in_componente_in_faces(null_face);
    }
}

// ----------------------------------------------------------------------------------------------------

static CSMBOOL i_is_out_component_of_null_face_attached_to_itself(struct csmface_t *null_face, struct csmloop_t **loop_attached_to_out_component_opt)
{
    CSMBOOL is_out_component_connected_to_itself;
    struct csmloop_t *out_component;
    struct csmhedge_t *ledge, *ledge_mate, *iterator_mate;
    unsigned long no_iterations;
    struct csmloop_t *loop_attached_to_out_component_loc;
    
    out_component = csmface_flout(null_face);
    
    is_out_component_connected_to_itself = CSMTRUE;
    ledge = csmloop_ledge(out_component);
    ledge_mate = csmopbas_mate(ledge);
    iterator_mate = ledge_mate;
    no_iterations = 0;
    
    do
    {
        struct csmhedge_t *iterator_mate_mate;
        struct csmloop_t *iterator_mate_mate_loop;
        
        assert(no_iterations < 10000);
        no_iterations++;
        
        iterator_mate_mate = csmopbas_mate(iterator_mate);
        iterator_mate_mate_loop = csmhedge_loop(iterator_mate_mate);
        
        if (iterator_mate_mate_loop != out_component)
        {
            is_out_component_connected_to_itself = CSMFALSE;
            break;
        }
        
        iterator_mate = csmhedge_next(iterator_mate);
    }
    while (iterator_mate != ledge_mate);
    
    if (is_out_component_connected_to_itself == CSMTRUE)
    {
        struct csmface_t *loop_face;
        
        loop_face = csmopbas_face_from_hedge(ledge_mate);
        loop_attached_to_out_component_loc = csmhedge_loop(ledge_mate);
        
        if (csmface_flout(loop_face) != loop_attached_to_out_component_loc)
        {
            is_out_component_connected_to_itself = CSMFALSE;
            loop_attached_to_out_component_loc = NULL;
        }
    }
    else
    {
        loop_attached_to_out_component_loc = NULL;
    }
    
    ASSIGN_OPTIONAL_VALUE(loop_attached_to_out_component_opt, loop_attached_to_out_component_loc);
            
    return is_out_component_connected_to_itself;
}

// ----------------------------------------------------------------------------------------------------

static CSMBOOL i_is_same_face_ptr(const struct csmface_t *face1, const struct csmface_t *face2)
{
    if (face1 == face2)
        return CSMTRUE;
    else
        return CSMFALSE;
}

// ----------------------------------------------------------------------------------------------------

static void i_convert_faces_attached_to_out_component_of_null_faces_in_faces_if_out_component_is_connected_to_itself(
                        struct csmsolid_t *solid,
                        const struct csmtolerance_t *tolerances,
                        csmArrayStruct(csmface_t) *set_of_null_faces)
{
    unsigned long i, num_null_faces;
    
    num_null_faces = csmarrayc_count_st(set_of_null_faces, csmface_t);
    
    for (i = 0; i < num_null_faces; i++)
    {
        struct csmface_t *null_face;
        struct csmloop_t *loop_attached_to_out_component;
        
        null_face = csmarrayc_get_st(set_of_null_faces, i, csmface_t);

        if (i_is_out_component_of_null_face_attached_to_itself(null_face, &loop_attached_to_out_component) == CSMTRUE)
        {
            if (csmloop_next(loop_attached_to_out_component) == NULL
                    && csmloop_prev(loop_attached_to_out_component) == NULL)
            {
                struct csmhashtb_iterator(csmface_t) *face_iterator;
                struct csmface_t *loop_attached_to_out_component_face;
                CSMBOOL has_been_converted_in_hole;
                
                face_iterator = csmsolid_face_iterator(solid);
                
                loop_attached_to_out_component_face = csmloop_lface(loop_attached_to_out_component);
                csmface_redo_geometric_generated_data(loop_attached_to_out_component_face);
                
                has_been_converted_in_hole = CSMFALSE;
                
                while (csmhashtb_has_next(face_iterator, csmface_t) == CSMTRUE && has_been_converted_in_hole == CSMFALSE)
                {
                    struct csmface_t *face;
                    
                    csmhashtb_next_pair(face_iterator, NULL, &face, csmface_t);
                    
                    if (face != loop_attached_to_out_component_face
                            && csmarrayc_contains_element_st(set_of_null_faces, csmface_t, face, struct csmface_t, i_is_same_face_ptr, NULL) == CSMFALSE)
                    {
                        csmface_redo_geometric_generated_data(face);
                        
                        if (csmface_are_coplanar_faces(face, loop_attached_to_out_component_face, tolerances, NULL) == CSMTRUE
                                && csmface_is_loop_contained_in_face(face, loop_attached_to_out_component, tolerances) == CSMTRUE)
                        {
                            csmeuler_lkfmrh(face, &loop_attached_to_out_component_face);
                            has_been_converted_in_hole = CSMTRUE;
                        }
                    }
                }
                
                
                csmhashtb_free_iterator(&face_iterator, csmface_t);
            }
        }
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_convert_inner_loops_of_null_faces_to_faces(csmArrayStruct(csmface_t) *set_of_null_faces)
{
    unsigned long i, no_null_faces;
    csmArrayStruct(csmface_t) *inner_loops_as_faces;
    
    no_null_faces = csmarrayc_count_st(set_of_null_faces, csmface_t);
    assert(no_null_faces > 0);
    
    inner_loops_as_faces = csmsetopcom_convert_inner_loops_of_null_faces_to_faces(set_of_null_faces);
    assert(no_null_faces == csmarrayc_count_st(inner_loops_as_faces, csmface_t));

    for (i = 0; i < no_null_faces; i++)
    {
        struct csmface_t *face;
        
        face = csmarrayc_get_st(inner_loops_as_faces, i, csmface_t);
        csmarrayc_append_element_st(set_of_null_faces, face, csmface_t);
    }
    
    csmarrayc_free_st(&inner_loops_as_faces, csmface_t, NULL);
}

// ------------------------------------------------------------------------------------------

static void i_assign_result_material(
                        const struct csmsolid_t *solid_A, const struct csmsolid_t *solid_B,
                        struct csmsolid_t *result)
{
    const struct csmmaterial_t *material_A, *material_B;
    const struct csmmaterial_t *material;
    
    material_A = csmsolid_get_material(solid_A);
    material_B = csmsolid_get_material(solid_B);

    if (material_A != NULL)
        material = material_A;
    else
        material = material_B;
    
    if (material != NULL)
    {
        struct csmmaterial_t *material_copy;
        
        material_copy = csmmaterial_copy(material);
        csmsolid_set_visualization_material(result, &material_copy);
    }
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmsolid_t *, i_finish_set_operation, (
                        enum csmsetop_operation_t set_operation,
                        struct csmsolid_t *solid_A, csmArrayStruct(csmface_t) *set_of_null_faces_A,
                        struct csmsolid_t *solid_B, csmArrayStruct(csmface_t) *set_of_null_faces_B,
                        const struct csmtolerance_t *tolerances))
{
    struct csmsolid_t *result;
    unsigned long i, no_null_faces, half_no_null_faces;
    unsigned long face_desp_a, face_desp_b;
    
    no_null_faces = csmarrayc_count_st(set_of_null_faces_A, csmface_t);
    assert(no_null_faces == csmarrayc_count_st(set_of_null_faces_B, csmface_t));
    assert(no_null_faces > 0);
    
    i_convert_holes_attached_to_in_component_of_null_faces_in_faces(set_of_null_faces_A);
    i_convert_faces_attached_to_out_component_of_null_faces_in_faces_if_out_component_is_connected_to_itself(solid_A, tolerances, set_of_null_faces_A);
    
    i_convert_holes_attached_to_in_component_of_null_faces_in_faces(set_of_null_faces_B);
    i_convert_faces_attached_to_out_component_of_null_faces_in_faces_if_out_component_is_connected_to_itself(solid_B, tolerances, set_of_null_faces_B);

    //csmdebug_show_viewer();
    
    i_convert_inner_loops_of_null_faces_to_faces(set_of_null_faces_A);
    i_convert_inner_loops_of_null_faces_to_faces(set_of_null_faces_B);
    no_null_faces = csmarrayc_count_st(set_of_null_faces_A, csmface_t);
    assert(no_null_faces == csmarrayc_count_st(set_of_null_faces_B, csmface_t));
    assert(no_null_faces % 2 == 0);
    
    csmsolid_redo_geometric_face_data(solid_A);
    csmsetopcom_reintroduce_holes_in_corresponding_faces(set_of_null_faces_A, tolerances);
    csmsetopcom_introduce_holes_in_in_component_null_faces_if_proceed(solid_A, tolerances, set_of_null_faces_A);
    
    csmsolid_redo_geometric_face_data(solid_B);
    csmsetopcom_reintroduce_holes_in_corresponding_faces(set_of_null_faces_B, tolerances);
    csmsetopcom_introduce_holes_in_in_component_null_faces_if_proceed(solid_B, tolerances, set_of_null_faces_B);

    no_null_faces = csmarrayc_count_st(set_of_null_faces_A, csmface_t);
    assert(no_null_faces == csmarrayc_count_st(set_of_null_faces_B, csmface_t));
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
    
    csmsolid_debug_print_debug(solid_A, CSMFALSE);
    csmsolid_debug_print_debug(solid_B, CSMFALSE);

    csmsolid_prepare_for_cleanup(solid_A);
    csmsolid_prepare_for_cleanup(solid_B);
    
    result = csmsolid_crea_vacio(0);
    i_assign_result_material(solid_A, solid_B, result);
    csmsolid_set_name(result, "Result");
    
    for (i = 0; i < half_no_null_faces; i++)
    {
        struct csmface_t *face_from_solid_A, *face_from_solid_B;
        
        face_from_solid_A = csmarrayc_get_st(set_of_null_faces_A, i + face_desp_a, csmface_t);
        csmsetopcom_move_face_to_solid(0, face_from_solid_A, solid_A, result);
        
        face_from_solid_B = csmarrayc_get_st(set_of_null_faces_B, i + face_desp_b, csmface_t);
        csmsetopcom_move_face_to_solid(0, face_from_solid_B, solid_B, result);
    }
    
    csmsetopcom_cleanup_solid_setop(solid_A, solid_B, result);
    csmsolid_finish_cleanup(solid_A);
    csmsolid_finish_cleanup(solid_B);
    csmsolid_finish_cleanup(result);    
    
    csmsolid_debug_print_debug(result, CSMFALSE);
    //csmsolid_debug_print_debug(solid_A, CSMFALSE);
    //csmsolid_debug_print_debug(solid_B, CSMFALSE);

    csmsetopcom_correct_non_manifold_edges_null_faces(set_of_null_faces_A);
    csmsetopcom_correct_non_manifold_edges_null_faces(set_of_null_faces_B);
    
    for (i = 0; i < half_no_null_faces; i++)
    {
        struct csmface_t *face_from_solid_A, *face_from_solid_B;
        
        face_from_solid_A = csmarrayc_get_st(set_of_null_faces_A, i + face_desp_a, csmface_t);
        face_from_solid_B = csmarrayc_get_st(set_of_null_faces_B, i + face_desp_b, csmface_t);
        
        csmeuler_lkfmrh(face_from_solid_A, &face_from_solid_B);
        csmloopglue_merge_face_loops(face_from_solid_A, tolerances);
    }
    
    csmsolid_redo_geometric_face_data(result);

    csmdebug_print_debug_info("After merging face loops...\n");
    csmsolid_debug_print_debug(result, CSMFALSE);

    csmsetopcom_correct_faces_after_joining_null_edges(result, tolerances);
    
    csmsolid_clear_algorithm_data(result);
    csmsolid_redo_geometric_face_data(result);
    csmdebug_set_viewer_results(result, NULL);
    
    return result;
}

// ------------------------------------------------------------------------------------------

static CSMBOOL i_is_solid_A_contained_in_solid_B(
                        const struct csmsolid_t *solid_A, const struct csmsolid_t *solid_B,
                        const struct csmtolerance_t *tolerances)
{
    CSMBOOL is_solid_A_contained_in_solid_B;
    const struct csmbbox_t *bbox_solid_A, *bbox_solid_B;
    
    bbox_solid_A = csmsolid_get_bbox(solid_A);
    bbox_solid_B = csmsolid_get_bbox(solid_B);
    
    if (csmbbox_intersects_with_other_bbox(bbox_solid_A, bbox_solid_B) == CSMFALSE)
    {
        is_solid_A_contained_in_solid_B = CSMFALSE;
    }
    else
    {
        struct csmhashtb_iterator(csmvertex_t) *vertex_iterator_A;
    
        is_solid_A_contained_in_solid_B = CSMTRUE;
        vertex_iterator_A = csmsolid_vertex_iterator_const(solid_A);
        
        while (csmhashtb_has_next(vertex_iterator_A, csmvertex_t) == CSMTRUE)
        {
            struct csmvertex_t *vertex_A;
            double x, y, z;
            
            csmhashtb_next_pair(vertex_iterator_A, NULL, &vertex_A, csmvertex_t);
            csmvertex_get_coordenadas(vertex_A, &x, &y, &z);
            
            if (csmsolid_does_solid_contain_point(solid_B, x, y, z, tolerances) == CSMFALSE)
            {
                is_solid_A_contained_in_solid_B = CSMFALSE;
                break;
            }
        }
        
        csmhashtb_free_iterator(&vertex_iterator_A, csmvertex_t);
    }
    
    return is_solid_A_contained_in_solid_B;
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmsolid_t *, i_generate_op_result_with_no_null_edges,  (
                        enum csmsetop_operation_t set_operation,
                        const struct csmsolid_t *solid_A, const struct csmsolid_t *solid_B,
                        const struct csmtolerance_t *tolerances))
{
    struct csmsolid_t *solid_res;

    switch (set_operation)
    {
        case CSMSETOP_OPERATION_UNION:
        {
            if (i_is_solid_A_contained_in_solid_B(solid_A, solid_B, tolerances) == CSMTRUE)
            {
                solid_res = csmsolid_duplicate(solid_B);
            }
            else if (i_is_solid_A_contained_in_solid_B(solid_B, solid_A, tolerances) == CSMTRUE)
            {
                solid_res = csmsolid_duplicate(solid_A);
            }
            else
            {
                solid_res = csmsolid_crea_vacio(0);
                
                csmsolid_merge_solids(solid_res, solid_A);
                csmsolid_merge_solids(solid_res, solid_B);
            }
            break;
        }
            
        case CSMSETOP_OPERATION_DIFFERENCE:
        {
            if (i_is_solid_A_contained_in_solid_B(solid_A, solid_B, tolerances) == CSMTRUE)
                solid_res = csmsolid_crea_vacio(0);
            else
                solid_res = csmsolid_duplicate(solid_A);
            break;
        }
            
        case CSMSETOP_OPERATION_INTERSECTION:
        {
            if (i_is_solid_A_contained_in_solid_B(solid_A, solid_B, tolerances) == CSMTRUE)
                solid_res = csmsolid_duplicate(solid_A);
            else if (i_is_solid_A_contained_in_solid_B(solid_B, solid_A, tolerances) == CSMTRUE)
                solid_res = csmsolid_duplicate(solid_B);
            else
                solid_res = csmsolid_crea_vacio(0);
            break;
        }
            
        default_error();
    }
    
    return solid_res;
}

// ------------------------------------------------------------------------------------------

static enum csmsetop_opresult_t i_set_operation_modifying_solids_internal(
                        enum csmsetop_operation_t set_operation,
                        const struct csmsolid_t *original_solid_A, struct csmsolid_t *solid_A,
                        const struct csmsolid_t *original_solid_B, struct csmsolid_t *solid_B,
                        const struct csmtolerance_t *tolerances,
                        struct csmsolid_t **solid_res)
{
    enum csmsetop_opresult_t result;
    struct csmsolid_t *solid_res_loc;
    csmArrayStruct(csmsetop_vtxvtx_inters_t) *vv_intersections;
    csmArrayStruct(csmsetop_vtxfacc_inters_t) *vf_intersections_A, *vf_intersections_B;
    CSMBOOL did_find_non_manifold_operand;
    
    csmsolid_redo_geometric_face_data(solid_A);
    csmsolid_clear_algorithm_data(solid_A);
    
    csmsolid_redo_geometric_face_data(solid_B);
    csmsolid_clear_algorithm_data(solid_B);

    if (csmdebug_debug_enabled() == CSMTRUE)
    {
        csmsolid_debug_print_debug(solid_A, CSMTRUE);
        csmsolid_debug_print_debug(solid_B, CSMTRUE);
    }
    
    csmsetop_procedges_generate_intersections_on_both_solids(
                        solid_A, solid_B,
                        tolerances,
                        &vv_intersections,
                        &vf_intersections_A, &vf_intersections_B,
                        &did_find_non_manifold_operand);
    
    if (did_find_non_manifold_operand == CSMTRUE)
    {
        result = CSMSETOP_OPRESULT_NON_MANIFOLD_OPERAND;
        solid_res_loc = NULL;
    }
    else
    {
        csmArrayStruct(csmedge_t) *set_of_null_edges_A, *set_of_null_edges_B;
        CSMBOOL improper_intersection_detected;
        
        set_of_null_edges_A = csmarrayc_new_st_array(0, csmedge_t);
        set_of_null_edges_B = csmarrayc_new_st_array(0, csmedge_t);
        
        csmsetop_vtxfacc_append_null_edges(vf_intersections_A, set_operation, CSMSETOP_A_VS_B, tolerances, set_of_null_edges_A, set_of_null_edges_B);
        csmsetop_vtxfacc_append_null_edges(vf_intersections_B, set_operation, CSMSETOP_B_VS_A, tolerances, set_of_null_edges_B, set_of_null_edges_A);
        csmsetop_vtxvtx_append_null_edges(vv_intersections, set_operation, tolerances, set_of_null_edges_A, set_of_null_edges_B, &improper_intersection_detected);
        
        if (improper_intersection_detected == CSMTRUE)
        {
            result = CSMSETOP_OPRESULT_IMPROPER_INTERSECTIONS;
            solid_res_loc = NULL;
        }
        else
        {
            unsigned long no_null_edges;
            
            no_null_edges = csmarrayc_count_st(set_of_null_edges_A, csmedge_t);
            assert(no_null_edges == csmarrayc_count_st(set_of_null_edges_B, csmedge_t));
            
            if (no_null_edges == 0)
            {
                result = CSMSETOP_OPRESULT_OK;
                solid_res_loc = i_generate_op_result_with_no_null_edges(set_operation, original_solid_A, original_solid_B, tolerances);
            }
            else
            {
                csmArrayStruct(csmface_t) *set_of_null_faces_A, *set_of_null_faces_B;
                CSMBOOL did_join_all_null_edges_loc;
                unsigned long no_null_faces;
                
                csmsetop_join_null_edges(
                            solid_A, set_of_null_edges_A,
                            solid_B, set_of_null_edges_B,
                            tolerances,
                            &set_of_null_faces_A, &set_of_null_faces_B,
                            &did_join_all_null_edges_loc);
                
                no_null_faces = csmarrayc_count_st(set_of_null_faces_A, csmface_t);
                assert(no_null_faces == csmarrayc_count_st(set_of_null_faces_B, csmface_t));
                
                if (did_join_all_null_edges_loc == CSMFALSE || no_null_faces == 0)
                {
                    result = CSMSETOP_OPRESULT_IMPROPER_INTERSECTIONS;
                    solid_res_loc = NULL;
                }
                else
                {
                    result = CSMSETOP_OPRESULT_OK;
                    
                    solid_res_loc = i_finish_set_operation(
                                set_operation,
                                solid_A, set_of_null_faces_A,
                                solid_B, set_of_null_faces_B,
                                tolerances);
                }
                
                csmarrayc_free_st(&set_of_null_faces_A, csmface_t, NULL);
                csmarrayc_free_st(&set_of_null_faces_B, csmface_t, NULL);
            }
        }
        
        csmarrayc_free_st(&set_of_null_edges_A, csmedge_t, NULL);
        csmarrayc_free_st(&set_of_null_edges_B, csmedge_t, NULL);
    }
    
    *solid_res = solid_res_loc;
    
    csmarrayc_free_st(&vv_intersections, csmsetop_vtxvtx_inters_t, csmsetop_vtxvtx_free_inters);
    csmarrayc_free_st(&vf_intersections_A, csmsetop_vtxfacc_inters_t, csmsetop_vtxfacc_free_inters);
    csmarrayc_free_st(&vf_intersections_B, csmsetop_vtxfacc_inters_t, csmsetop_vtxfacc_free_inters);
    
    return result;
}

// ------------------------------------------------------------------------------------------

static enum csmsetop_opresult_t i_set_operation(
                        enum csmsetop_operation_t set_operation,
                        const struct csmsolid_t *solid_A, const struct csmsolid_t *solid_B,
                        struct csmsolid_t **solid_res)
{
    enum csmsetop_opresult_t result;
    struct csmsolid_t *solid_res_loc;
    struct csmtolerance_t *tolerances;
    unsigned long no_perturbations;
    double perturbartion_amount;
    CSMBOOL apply_perturbation;

    tolerances = csmtolerance_new();
    csmsolid_redo_geometric_face_data((struct csmsolid_t *)solid_A);
    csmsolid_redo_geometric_face_data((struct csmsolid_t *)solid_B);
    
    no_perturbations = 0;
    
    apply_perturbation = CSMFALSE;
    perturbartion_amount = 1.;
    
    solid_res_loc = NULL;
    
    do
    {
        struct csmsolid_t *solid_A_copy, *solid_B_copy;
    
        if (csmdebug_debug_enabled() == CSMTRUE)
            csmdebug_print_debug_info(">>> Perturbation pass %lu\n", no_perturbations);
        
        solid_A_copy = csmsolid_duplicate(solid_A);
        csmsolid_set_name(solid_A_copy, "Solid A");
        
        solid_B_copy = csmsolid_duplicate(solid_B);
        csmsolid_set_name(solid_B_copy, "Solid B");

        if (solid_res_loc != NULL)
            csmsolid_free(&solid_res_loc);
        
        if (apply_perturbation == CSMTRUE)
        {
            csmsolid_general_transform(
                        solid_B_copy,
                        perturbartion_amount, 0., 0., 0.,
                        0., perturbartion_amount, 0., 0.,
                        0., 0., perturbartion_amount, 0.);
        }

        csmdebug_clear_debug_points();
        csmdebug_set_viewer_results(NULL, NULL);
        csmdebug_set_viewer_parameters(solid_A_copy, solid_B_copy);
        csmdebug_show_viewer();
        
        csmdebug_begin_context("SETOP");
        {
            result = i_set_operation_modifying_solids_internal(
                        set_operation,
                        solid_A, solid_A_copy,
                        solid_B, solid_B_copy,
                        tolerances,
                        &solid_res_loc);
        }
        csmdebug_end_context();

        switch (result)
        {
            case CSMSETOP_OPRESULT_OK:
            case CSMSETOP_OPRESULT_NON_MANIFOLD_OPERAND:
                break;
                
            case CSMSETOP_OPRESULT_IMPROPER_INTERSECTIONS:
            
                if (csmdebug_get_treat_improper_solid_operations_as_errors() == CSMTRUE)
                    apply_perturbation = CSMFALSE;
                else
                    apply_perturbation = CSMTRUE;
                break;
                
            default_error();
        }
        
        if (apply_perturbation == CSMTRUE)
        {
            perturbartion_amount += csmtolerance_perturbation_increment(tolerances);
            no_perturbations++;
            
            if (csmdebug_debug_enabled() == CSMTRUE)
            {
                char *text;
                
                text = copiafor_codigo2("Perturbation %lu, amount: %lf\n", no_perturbations, perturbartion_amount);
                csmdebug_print_debug_info(text);
                
                csmstring_free(&text);
            }
        }
        
        csmsolid_free(&solid_A_copy);
        csmsolid_free(&solid_B_copy);
        
    } while (result != CSMSETOP_OPRESULT_OK && apply_perturbation == CSMTRUE && no_perturbations < i_NUM_MAX_PERTURBATIONS);

    if (csmdebug_get_treat_improper_solid_operations_as_errors() == CSMTRUE)
        assert(no_perturbations < i_NUM_MAX_PERTURBATIONS);
    
    if (result == CSMSETOP_OPRESULT_OK && csmdebug_debug_enabled() == CSMTRUE)
    {
        csmdebug_clear_debug_points();
        csmsolid_debug_print_debug(solid_res_loc, CSMTRUE);
        
        csmdebug_set_viewer_results(solid_res_loc, NULL);
        csmdebug_show_viewer();
    }
    
    *solid_res = solid_res_loc;
    
    csmtolerance_free(&tolerances);
    
    return result;
}

// ------------------------------------------------------------------------------------------

enum csmsetop_opresult_t csmsetop_difference_A_minus_B(const struct csmsolid_t *solid_A, const struct csmsolid_t *solid_B, struct csmsolid_t **solid_res)
{
    enum csmsetop_operation_t set_operation;
    
    set_operation = CSMSETOP_OPERATION_DIFFERENCE;
    return i_set_operation(set_operation, solid_A, solid_B, solid_res);
}

// ------------------------------------------------------------------------------------------

enum csmsetop_opresult_t csmsetop_union_A_and_B(const struct csmsolid_t *solid_A, const struct csmsolid_t *solid_B, struct csmsolid_t **solid_res)
{
    enum csmsetop_operation_t set_operation;
    
    set_operation = CSMSETOP_OPERATION_UNION;
    return i_set_operation(set_operation, solid_A, solid_B, solid_res);
}

// ------------------------------------------------------------------------------------------

enum csmsetop_opresult_t csmsetop_intersection_A_and_B(const struct csmsolid_t *solid_A, const struct csmsolid_t *solid_B, struct csmsolid_t **solid_res)
{
    enum csmsetop_operation_t set_operation;
    
    set_operation = CSMSETOP_OPERATION_INTERSECTION;
    return i_set_operation(set_operation, solid_A, solid_B, solid_res);
}
