// Commons operations for Set Operators...

#include "csmsetopcom.inl"

#include "csmassert.inl"
#include "csmarrayc.inl"
#include "csmdebug.inl"
#include "csmedge.inl"
#include "csmedge.tli"
#include "csmface.inl"
#include "csmface_debug.inl"
#include "csmhedge.inl"
#include "csmloop.inl"
#include "csmloopglue.inl"
#include "csmeuler_laringmv.inl"
#include "csmeuler_lkef.inl"
#include "csmeuler_lkev.inl"
#include "csmeuler_lkemr.inl"
#include "csmeuler_lmef.inl"
#include "csmeuler_lmekr.inl"
#include "csmeuler_lkfmrh.inl"
#include "csmeuler_lmfkrh.inl"
#include "csmhashtb.inl"
#include "csmmath.inl"
#include "csmopbas.inl"
#include "csmsetop.tli"
#include "csmsolid.inl"
#include "csmsolid_debug.inl"
#include "csmsolid.tli"
#include "csmtolerance.inl"
#include "csmvertex.inl"
#include "csmvertex.tli"


// ----------------------------------------------------------------------------------------------------

static CSMBOOL i_equals_vertices(const struct csmvertex_t *vertex1, const struct csmvertex_t *vertex2)
{
    if (vertex1 == vertex2)
        return CSMTRUE;
    else
        return CSMFALSE;
}

// ----------------------------------------------------------------------------------------------------

void csmsetopcom_append_vertex_if_not_exists(struct csmvertex_t *vertex, csmArrayStruct(csmvertex_t) *set_of_on_vertices)
{
    if (csmarrayc_contains_element_st(set_of_on_vertices, csmvertex_t, vertex, struct csmvertex_t, i_equals_vertices, NULL) == CSMFALSE)
        csmarrayc_append_element_st(set_of_on_vertices, vertex, csmvertex_t);
}

// ----------------------------------------------------------------------------------------------------

static CSMBOOL i_is_hedge_on_inner_loop(struct csmhedge_t *hedge)
{
    struct csmloop_t *hedge_loop;
    struct csmface_t *face_loop;
    
    hedge_loop = csmhedge_loop(hedge);
    face_loop = csmloop_lface(hedge_loop);
    
    if (csmface_flout(face_loop) != hedge_loop)
        return CSMTRUE;
    else
        return CSMFALSE;
}

// ----------------------------------------------------------------------------------------------------

static CSMBOOL i_is_hedge_on_hedge_cycle_with_a_side_on_a_hole(struct csmhedge_t *hedge)
{
    register struct csmhedge_t *iterator;
    unsigned long no_iters;
    
    iterator = hedge;
    no_iters = 0;
    
    do
    {
        struct csmhedge_t *iterator_mate;

        assert(no_iters < 100000);
        no_iters++;

        iterator_mate = csmopbas_mate(iterator);
        
        if (i_is_hedge_on_inner_loop(iterator) == CSMTRUE || i_is_hedge_on_inner_loop(iterator_mate) == CSMTRUE)
            return CSMTRUE;
        else
            iterator = csmhedge_next(iterator);
        
    } while (iterator != hedge);
    
    return CSMFALSE;
}

// ----------------------------------------------------------------------------------------------------

CSMBOOL csmsetopcom_is_edge_on_cycle_of_edges_with_a_side_on_a_hole(struct csmedge_t *edge)
{
    struct csmhedge_t *hedge_pos, *hedge_neg;
    
    hedge_pos = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_POS);
    hedge_neg = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_NEG);
    
    if (i_is_hedge_on_hedge_cycle_with_a_side_on_a_hole(hedge_pos) == CSMTRUE
            || i_is_hedge_on_hedge_cycle_with_a_side_on_a_hole(hedge_neg) == CSMTRUE)
    {
        return CSMTRUE;
    }
    else
    {
        return CSMFALSE;
    }
}

// ----------------------------------------------------------------------------------------------------

CSMBOOL csmsetopcom_hedges_are_neighbors(struct csmhedge_t *he1, struct csmhedge_t *he2)
{
    struct csmface_t *face_he1, *face_he2;
    
    face_he1 = csmopbas_face_from_hedge(he1);
    face_he2 = csmopbas_face_from_hedge(he2);
    
    if (face_he1 != face_he2)
    {
        return CSMFALSE;
    }
    else
    {
        struct csmedge_t *edge_he1, *edge_he2;
        struct csmhedge_t *he1_edge_he1, *he2_edge_he1;
        struct csmhedge_t *he1_edge_he2, *he2_edge_he2;
        
        edge_he1 = csmhedge_edge(he1);
        he1_edge_he1 = csmedge_hedge_lado(edge_he1, CSMEDGE_LADO_HEDGE_POS);
        he2_edge_he1 = csmedge_hedge_lado(edge_he1, CSMEDGE_LADO_HEDGE_NEG);
        
        edge_he2 = csmhedge_edge(he2);
        he1_edge_he2 = csmedge_hedge_lado(edge_he2, CSMEDGE_LADO_HEDGE_POS);
        he2_edge_he2 = csmedge_hedge_lado(edge_he2, CSMEDGE_LADO_HEDGE_NEG);
        
        if (he1 == he1_edge_he1 && he2 == he2_edge_he2)
            return CSMTRUE;
        else if (he1 == he2_edge_he1 && he2 == he1_edge_he2)
            return CSMTRUE;
        else
            return CSMFALSE;
    }
}

// ----------------------------------------------------------------------------------------------------

static enum csmcompare_t i_compare_edges_by_coord(
                        const struct csmedge_t *edge1, const struct csmedge_t *edge2,
                        const struct csmtolerance_t *tolerances)
{
    const struct csmhedge_t *he1_edge1, *he1_edge2;
    const struct csmvertex_t *vertex1, *vertex2;
    double x1, y1, z1, x2, y2, z2;
    
    he1_edge1 = csmedge_hedge_lado_const(edge1, CSMEDGE_LADO_HEDGE_POS);
    vertex1 = csmhedge_vertex_const(he1_edge1);
    csmvertex_get_coordenadas(vertex1, &x1, &y1, &z1);
    
    he1_edge2 = csmedge_hedge_lado_const(edge2, CSMEDGE_LADO_HEDGE_POS);
    vertex2 = csmhedge_vertex_const(he1_edge2);
    csmvertex_get_coordenadas(vertex2, &x2, &y2, &z2);

    return csmmath_compare_coords_xyz(
                        x1, y1, z1,
                        x2, y2, z2,
                        csmtolerance_equal_coords(tolerances));
}

// ----------------------------------------------------------------------------------------------------

void csmsetopcom_sort_edges_lexicographically_by_xyz(csmArrayStruct(csmedge_t) *set_of_null_edges, const struct csmtolerance_t *tolerances)
{
    csmarrayc_qsort_st_1_extra(
                        set_of_null_edges, csmedge_t,
                        tolerances, struct csmtolerance_t,
                        i_compare_edges_by_coord);
}

// ----------------------------------------------------------------------------------------------------

void csmsetopcom_print_set_of_null_edges(const csmArrayStruct(csmedge_t) *set_of_null_edges, csmArrayStruct(csmhedge_t) *loose_ends)
{
    unsigned long i, num_null_edges;
    
    num_null_edges = csmarrayc_count_st(set_of_null_edges, csmedge_t);
    
    csmdebug_print_debug_info("Set of null edges:\n");
    
    for (i = 0; i < num_null_edges; i++)
    {
        struct csmedge_t *edge;
        struct csmhedge_t *he1, *he2;
        const struct csmvertex_t *vertex1;
        double x, y, z;
        CSMBOOL is_loose_he1, is_loose_he2;
        
        edge = csmarrayc_get_st(set_of_null_edges, i, csmedge_t);
        assert_no_null(edge);
        
        he1 = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_POS);
        he2 = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_NEG);
        
        is_loose_he1 = csmsetopcom_is_loose_end(he1, loose_ends);
        is_loose_he2 = csmsetopcom_is_loose_end(he2, loose_ends);
        
        vertex1 = csmhedge_vertex_const(he1);
        csmvertex_get_coordenadas(vertex1, &x, &y, &z);

        csmdebug_print_debug_info("\t[");
        
        if (is_loose_he1 == CSMFALSE && is_loose_he2 == CSMFALSE)
        {
            csmdebug_print_debug_info("_,_");
        }
        else
        {
            if (is_loose_he1 == CSMTRUE)
                csmdebug_print_debug_info("L");
            else
                csmdebug_print_debug_info("M");

            csmdebug_print_debug_info(",");
            
            if (is_loose_he2 == CSMTRUE)
                csmdebug_print_debug_info("L");
            else
                csmdebug_print_debug_info("M");
        }
        
        csmdebug_print_debug_info("]");
        
        csmdebug_print_debug_info(
                "[%lu] (%5.3g, %5.3g, %5.3g)\t[he1, loop, face] = (%lu, %lu, %lu)\t[he2, loop, face] = (%lu, %lu, %lu)) \n",
                csmedge_id(edge), x, y, z,
                csmhedge_id(he1), csmloop_id(csmhedge_loop((struct csmhedge_t *)he1)), csmface_id(csmopbas_face_from_hedge((struct csmhedge_t *)he1)),
                csmhedge_id(he2), csmloop_id(csmhedge_loop((struct csmhedge_t *)he2)), csmface_id(csmopbas_face_from_hedge((struct csmhedge_t *)he2)));
    }
}

// ----------------------------------------------------------------------------------------------------

CSMBOOL csmsetopcom_is_loose_end(struct csmhedge_t *hedge, csmArrayStruct(csmhedge_t) *loose_ends)
{
    unsigned long i, no_loose_end;
    
    no_loose_end = csmarrayc_count_st(loose_ends, csmhedge_t);
    
    for (i = 0; i < no_loose_end; i++)
    {
        if (csmarrayc_get_st(loose_ends, i, csmhedge_t) == hedge)
            return CSMTRUE;
    }
    
    return CSMFALSE;
}

// ----------------------------------------------------------------------------------------------------

void csmsetopcom_print_debug_info_loose_ends(const csmArrayStruct(csmhedge_t) *loose_ends)
{
    unsigned long i, no_loose_end;
    
    csmdebug_print_debug_info("Loose ends [");
    
    no_loose_end = csmarrayc_count_st(loose_ends, csmhedge_t);
    
    for (i = 0; i < no_loose_end; i++)
    {
        const struct csmhedge_t *hedge;
        
        hedge = csmarrayc_get_const_st(loose_ends, i, csmhedge_t);
        assert_no_null(hedge);
        
        if (i > 0)
            csmdebug_print_debug_info(", ");
        
        csmdebug_print_debug_info("%lu", csmhedge_id(hedge));
    }
    
    csmdebug_print_debug_info("]\n");
}

// ----------------------------------------------------------------------------------------------------

static CSMBOOL i_is_same_edge_by_ptr(const struct csmedge_t *edge1, const struct csmedge_t *edge2)
{
    if (edge1 == edge2)
        return CSMTRUE;
    else
        return CSMFALSE;
}

// ----------------------------------------------------------------------------------------------------

void csmsetopcom_join_hedges(
                        struct csmhedge_t *he1, struct csmhedge_t *he2,
                        const struct csmtolerance_t *tolerances)
{
    struct csmsolid_t *he1_solid;
    struct csmface_t *old_face, *new_face;
    struct csmhedge_t *he1_next, *he1_next_next;
    CSMBOOL original_loop_is_a_hole;
    
    old_face = csmopbas_face_from_hedge(he1);
    he1_solid = csmface_fsolid(old_face);
    
    {
        csmdebug_print_debug_info(
                        "******\njoin_hedges (%lu [NE: %lu], %lu [NE: %lu]). Solid before operation: \n",
                        csmhedge_id(he1), csmedge_id(csmhedge_edge(he1)),
                        csmhedge_id(he2), csmedge_id(csmhedge_edge(he2)));
        
        csmsolid_debug_print_debug(he1_solid, CSMTRUE);
    }
    
    if (csmhedge_loop(he1) == csmhedge_loop(he2))
    {
        struct csmface_t *he1_face, *he2_face;
        struct csmhedge_t *he1_prev, *he1_prev_prev;
        
        he1_face = csmopbas_face_from_hedge(he1);
        he2_face = csmopbas_face_from_hedge(he2);
        assert(he1_face == he2_face);
        
        if (csmface_flout(he1_face) != csmhedge_loop(he1))
            original_loop_is_a_hole = CSMTRUE;
        else
            original_loop_is_a_hole = CSMFALSE;
        
        he1_prev = csmhedge_prev(he1);
        he1_prev_prev = csmhedge_prev(he1_prev);
        
        if (he1_prev_prev != he2)
        {
            struct csmhedge_t *he2_next;
            
            he2_next = csmhedge_next(he2);
            csmeuler_lmef(he1, he2_next, &new_face, NULL, NULL);
            
            if (csmdebug_debug_enabled() == CSMTRUE)
            {
                csmdebug_print_debug_info(
                        "(SAME LOOP) joining edges (%lu [NE: %lu], %lu [NE: %lu]) with LMEF, new face %lu.\n",
                        csmhedge_id(he1), csmedge_id(csmhedge_edge(he1)),
                        csmhedge_id(he2), csmedge_id(csmhedge_edge(he2)),
                        csmface_id(new_face));
                
                csmface_debug_print_info_debug(new_face, CSMTRUE, NULL);
                csmsolid_debug_print_debug(he1_solid, CSMTRUE);
            }
        }
        else
        {
            if (csmdebug_debug_enabled() == CSMTRUE)
            {
                csmdebug_print_debug_info(
                        "(SAME LOOP) joining edges (%lu [NE: %lu], %lu [NE: %lu]). Already connected.\n",
                        csmhedge_id(he1), csmedge_id(csmhedge_edge(he1)),
                        csmhedge_id(he2), csmedge_id(csmhedge_edge(he2)));
            }
            
            new_face = NULL;
        }
    }
    else
    {
        struct csmhedge_t *he2_next;
        
        new_face = NULL;
        
        he2_next = csmhedge_next(he2);
        csmeuler_lmekr(he1, he2_next, NULL, NULL);
        
        if (csmdebug_debug_enabled() == CSMTRUE)
        {
            csmdebug_print_debug_info(
                        "(DIFFERENT LOOP) joining edges (%lu [NE: %lu], %lu [NE: %lu]) with LMEKR, lmekr he1 with %lu\n",
                        csmhedge_id(he1), csmedge_id(csmhedge_edge(he1)),
                        csmhedge_id(he2), csmedge_id(csmhedge_edge(he2)),
                        csmhedge_id(he2_next));
        }
        
        original_loop_is_a_hole = CSMFALSE;
    }
    
    he1_next = csmhedge_next(he1);
    he1_next_next = csmhedge_next(he1_next);
    
    if (he1_next_next != he2)
    {
        struct csmloop_t *old_face_floops;
        struct csmface_t *second_new_face;

        if (csmdebug_debug_enabled() == CSMTRUE)
        {
            csmdebug_print_debug_info(
                        "Split(): joining edges (%lu [NE: %lu], %lu [NE: %lu]) with LMEF between (%lu, %lu)\n",
                        csmhedge_id(he1), csmedge_id(csmhedge_edge(he1)),
                        csmhedge_id(he2), csmedge_id(csmhedge_edge(he2)),
                        csmhedge_id(he2), csmhedge_id(he1_next));
            
            csmsolid_debug_print_debug(he1_solid, CSMTRUE);
        }
        
        csmeuler_lmef(he2, he1_next, &second_new_face, NULL, NULL);

        if (csmdebug_debug_enabled() == CSMTRUE)
        {
            csmface_debug_print_info_debug(second_new_face, CSMTRUE, NULL);

            csmdebug_print_debug_info("\tFace %lu created\n", csmface_id(second_new_face));
            csmsolid_debug_print_debug(csmface_fsolid(second_new_face), CSMTRUE);
        }
        
        old_face_floops = csmface_floops(old_face);
        
        if (new_face != NULL && csmloop_next(old_face_floops) != NULL)
            csmeuler_laringmv(old_face, new_face, tolerances);
        
         if (new_face != NULL && original_loop_is_a_hole == CSMTRUE)
        {
            CSMBOOL did_move_some_loop;
            
            csmeuler_laringmv_from_face1_to_2_if_fits_in_face(new_face, old_face, tolerances, &did_move_some_loop);
            
            if (did_move_some_loop == CSMTRUE && csmface_floops(new_face) == NULL)
                csmsolid_remove_face(he1_solid, &new_face);
        }
    }
}

// ----------------------------------------------------------------------------------------------------

void csmsetopcom_cut_he(
                    struct csmhedge_t *hedge,
                    csmArrayStruct(csmedge_t) *set_of_null_edges,
                    csmArrayStruct(csmface_t) *set_of_null_faces,
                    unsigned long *no_null_edges_deleted,
                    CSMBOOL *null_face_created_opt)
{
    CSMBOOL null_face_created_loc;
    CSMBOOL contains_element;
    unsigned long idx;
    struct csmsolid_t *solid;
    struct csmedge_t *edge;
    struct csmhedge_t *he1_edge, *he2_edge;
    struct csmloop_t *he1_loop, *he2_loop;
    
    assert_no_null(no_null_edges_deleted);
    
    solid = csmopbas_solid_from_hedge(hedge);
    
    edge = csmhedge_edge(hedge);
    he1_edge = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_POS);
    he2_edge = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_NEG);

    contains_element = csmarrayc_contains_element_st(set_of_null_edges, csmedge_t, edge, struct csmedge_t, i_is_same_edge_by_ptr, &idx);
    assert(contains_element == CSMTRUE);
    csmarrayc_delete_element_st(set_of_null_edges, idx, csmedge_t, NULL);
    (*no_null_edges_deleted)++;
    
    he1_loop = csmhedge_loop(he1_edge);
    he2_loop = csmhedge_loop(he2_edge);
    
    if (he1_loop == he2_loop)
    {
        struct csmface_t *null_face;
        
        null_face = csmopbas_face_from_hedge(hedge);
        csmarrayc_append_element_st(set_of_null_faces, null_face, csmface_t);
        
        if (csmdebug_debug_enabled() == CSMTRUE)
        {
            csmdebug_print_debug_info("***NULL FACE***\n");
            csmface_debug_print_info_debug(null_face, CSMTRUE, NULL);
            
            csmdebug_print_debug_info("(CUTTING HE)  (%lu, %lu) with LKEMR\n", csmhedge_id(he1_edge), csmhedge_id(he2_edge));
            csmsolid_debug_print_debug(csmopbas_solid_from_hedge(hedge), CSMTRUE);
        }
        
        csmeuler_lkemr(&he1_edge, &he2_edge, NULL, NULL);
        
        if (csmdebug_debug_enabled() == CSMTRUE)
            csmsolid_debug_print_debug(solid, CSMTRUE);
        
        null_face_created_loc = CSMTRUE;
    }
    else
    {
        if (csmdebug_debug_enabled() == CSMTRUE)
        {
            csmdebug_print_debug_info("(CUTTING HE)  (%lu, %lu) with LKEF. ", csmhedge_id(he1_edge), csmhedge_id(he2_edge));
            csmdebug_print_debug_info("Deleting face he2: %lu\n", csmface_id(csmopbas_face_from_hedge(he2_edge)));
        }
        
        csmeuler_lkef(&he1_edge, &he2_edge);
        null_face_created_loc = CSMFALSE;
        
        if (csmdebug_debug_enabled() == CSMTRUE)
            csmsolid_debug_print_debug(solid, CSMTRUE);
    }
    
    ASSIGN_OPTIONAL_VALUE(null_face_created_opt, null_face_created_loc);
}

// ----------------------------------------------------------------------------------------------------

void csmsetopcom_postprocess_join_edges(struct csmsolid_t *solid)
{
    CSMBOOL there_are_changes;
    
    do
    {
        struct csmhashtb_iterator(csmface_t) *face_iterator;
    
        face_iterator = csmsolid_face_iterator(solid);
        there_are_changes = CSMFALSE;
    
        while (csmhashtb_has_next(face_iterator, csmface_t) == CSMTRUE)
        {
            struct csmface_t *face;
            struct csmloop_t *loop_iterator;
            
            csmhashtb_next_pair(face_iterator, NULL, &face, csmface_t);
            loop_iterator = csmface_floops(face);
            
            while (loop_iterator != NULL)
            {
                struct csmloop_t *next_loop;
                
                next_loop = csmloop_next(loop_iterator);
                
                if (csmloop_setop_convert_loop_in_face(loop_iterator) == CSMTRUE)
                {
                    struct csmface_t *new_face;
                    
                    there_are_changes = CSMTRUE;
                    
                    csmloop_set_setop_convert_loop_in_face(loop_iterator, CSMFALSE);
                    csmeuler_lmfkrh(loop_iterator, &new_face);
                    csmface_redo_geometric_generated_data(new_face);
                }
                
                loop_iterator = next_loop;
            }
            
            if (csmface_floops(face) == NULL)
                csmsolid_remove_face(solid, &face);
            
            if (there_are_changes == CSMTRUE)
                break;
        }
        
        csmhashtb_free_iterator(&face_iterator, csmface_t);
        
    } while (there_are_changes == CSMTRUE);
}

// ----------------------------------------------------------------------------------------------------

void csmsetopcom_remove_null_edges(csmArrayStruct(csmedge_t) *set_of_null_edges)
{
    unsigned long i, no_null_edges;
    unsigned long no_deleted;
    
    no_null_edges = csmarrayc_count_st(set_of_null_edges, csmedge_t);
    no_deleted = 0;
    
    for (i = 0; i < no_null_edges; i++)
    {
        unsigned long idx;
        struct csmedge_t *null_edge;
        struct csmhedge_t *he1, *he2;
        
        idx = i - no_deleted;
        null_edge = csmarrayc_get_st(set_of_null_edges, idx, csmedge_t);
        
        he1 = csmedge_hedge_lado(null_edge, CSMEDGE_LADO_HEDGE_POS);
        he2 = csmedge_hedge_lado(null_edge, CSMEDGE_LADO_HEDGE_NEG);
        
        if (csmopbas_face_from_hedge(he1) != csmopbas_face_from_hedge(he2))
        {
            csmeuler_lkev(&he2, &he1, NULL, NULL, NULL, NULL);
            
            csmarrayc_delete_element_st(set_of_null_edges, idx, csmedge_t, NULL);
            no_deleted++;
        }
    }
}

// ----------------------------------------------------------------------------------------------------

csmArrayStruct(csmface_t) *csmsetopcom_convert_inner_loops_of_null_faces_to_faces(csmArrayStruct(csmface_t) *set_of_null_faces)
{
    csmArrayStruct(csmface_t) *set_of_null_faces_below;
    unsigned long i, no_null_faces;
    
    no_null_faces = csmarrayc_count_st(set_of_null_faces, csmface_t);
    assert(no_null_faces > 0);
    
    set_of_null_faces_below = csmarrayc_new_st_array(0, csmface_t);
    
    for (i = 0; i < no_null_faces; i++)
    {
        struct csmface_t *null_face;
        struct csmloop_t *floops, *loop_to_move;
        struct csmface_t *new_face;
        
        null_face = csmarrayc_get_st(set_of_null_faces, i, csmface_t);
        
        floops = csmface_floops(null_face);
        loop_to_move = floops;
        
        do
        {
            struct csmloop_t *next_loop;

            next_loop = csmloop_next(loop_to_move);
            
            if (loop_to_move != csmface_flout(null_face))
            {
                csmeuler_lmfkrh(loop_to_move, &new_face);
                csmarrayc_append_element_st(set_of_null_faces_below, new_face, csmface_t);
            }
            
            loop_to_move = next_loop;
                
        } while (loop_to_move != NULL);
    }
    
    return set_of_null_faces_below;
}

// ----------------------------------------------------------------------------------------------------

static CSMBOOL i_is_face_originated_by_hole(struct csmface_t *face)
{
    struct csmloop_t *floops;
    
    floops = csmface_floops(face);
    
    if (csmloop_next(floops) != NULL)
    {
        return CSMFALSE;
    }
    else
    {
        struct csmhedge_t *loop_ledge, *hedge_iterator;
        unsigned long no_iters;
        
        loop_ledge = csmloop_ledge(floops);
        hedge_iterator = loop_ledge;
        no_iters = 0;
        
        do
        {
            struct csmvertex_t *vertex;
            
            assert(no_iters < 100000);
            no_iters++;
            
            vertex = csmhedge_vertex(hedge_iterator);
            
            if (csmvertex_has_mask_attrib(vertex, CSMVERTEX_MASK_VERTEX_ON_HOLE_LOOP) == CSMFALSE)
                return CSMFALSE;
            
        } while (hedge_iterator != loop_ledge);
        
        return CSMTRUE;
    }
}

// ----------------------------------------------------------------------------------------------------

void csmsetopcom_reintroduce_holes_in_corresponding_faces(
                        csmArrayStruct(csmface_t) *set_of_null_faces,
                        const struct csmtolerance_t *tolerances)
{
    unsigned long no_iters;
    CSMBOOL did_delete_faces;
    
    no_iters = 0;
    
    do
    {
        unsigned long i, no_null_faces;

        assert(no_iters < 1000000);
        no_iters++;
        
        no_null_faces = csmarrayc_count_st(set_of_null_faces, csmface_t);
        assert(no_null_faces > 0);
        
        did_delete_faces = CSMFALSE;
        
        for (i = 0; i < no_null_faces && did_delete_faces == CSMFALSE; i++)
        {
            struct csmface_t *face_i;
            
            face_i = csmarrayc_get_st(set_of_null_faces, i, csmface_t);
            
            if (i_is_face_originated_by_hole(face_i) == CSMTRUE)
            {
                unsigned long j;
                struct csmloop_t *floops_face_i;
                
                floops_face_i = csmface_floops(face_i);
            
                for (j = 0; j < no_null_faces && did_delete_faces == CSMFALSE; j++)
                {
                    if (i != j)
                    {
                        struct csmface_t *face_j;
                    
                        face_j = csmarrayc_get_st(set_of_null_faces, j, csmface_t);
                    
                        if (csmface_is_loop_contained_in_face(face_j, floops_face_i, tolerances) == CSMTRUE)
                        {
                            did_delete_faces = CSMTRUE;
                            
                            csmeuler_lkfmrh(face_j, &face_i);
                            csmarrayc_delete_element_st(set_of_null_faces, i, csmface_t, NULL);
                        }
                    }
                }
            }
        }
        
    } while (did_delete_faces == CSMTRUE);
}

// ----------------------------------------------------------------------------------------------------

static CSMBOOL i_face_equal_ptr(const struct csmface_t *face1, const struct csmface_t *face2)
{
    if (face1 == face2)
        return CSMTRUE;
    else
        return CSMFALSE;
}

// ----------------------------------------------------------------------------------------------------

void csmsetopcom_introduce_holes_in_in_component_null_faces_if_proceed(
                        struct csmsolid_t *solid,
                        const struct csmtolerance_t *tolerances,
                        csmArrayStruct(csmface_t) *set_of_null_faces)
{
    unsigned long num_null_faces;
    CSMBOOL there_are_changes;
    
    num_null_faces = csmarrayc_count_st(set_of_null_faces, csmface_t);
    assert(num_null_faces > 0);
    assert(num_null_faces % 2 == 0);

    there_are_changes = CSMFALSE;
    
    do
    {
        struct csmhashtb_iterator(csmface_t) *face_iterator;
    
        face_iterator = csmsolid_face_iterator(solid);
        there_are_changes = CSMFALSE;
    
        while (csmhashtb_has_next(face_iterator, csmface_t) == CSMTRUE)
        {
            struct csmface_t *face;
            struct csmloop_t *face_floops;
            
            csmhashtb_next_pair(face_iterator, NULL, &face, csmface_t);
            face_floops = csmface_floops(face);
            
            if (csmloop_next(face_floops) == NULL
                    && csmloop_setop_loop_was_a_hole(face_floops) == CSMTRUE
                    && csmarrayc_contains_element_st(set_of_null_faces, csmface_t, face, struct csmface_t, i_face_equal_ptr, NULL) == CSMFALSE)
            {
                unsigned long idx_first_in_face, i;
                CSMBOOL did_remove_face;
                
                idx_first_in_face = num_null_faces / 2 ;
                did_remove_face = CSMFALSE;
                
                for (i = idx_first_in_face; i < num_null_faces; i++)
                {
                    struct csmface_t *null_face;
                    
                    null_face = csmarrayc_get_st(set_of_null_faces, i, csmface_t);
                    
                    if (csmface_are_coplanar_faces(face, null_face) == CSMTRUE)
                    {
                        CSMBOOL did_move_some_loop;
                        
                        csmeuler_laringmv_from_face1_to_2_if_fits_in_face(face, null_face, tolerances, &did_move_some_loop);
                        
                        if (did_move_some_loop == CSMTRUE)
                        {
                            assert(csmface_floops(face) == NULL);
                            
                            there_are_changes = CSMTRUE;
                            
                            csmloop_set_setop_loop_was_a_hole(face_floops, CSMFALSE);
                            csmsolid_remove_face(solid, &face);
                        }
                    }
                    
                    if (there_are_changes == CSMTRUE)
                        break;
                }
            }
            
            if (there_are_changes == CSMTRUE)
                break;
        }
        
        csmhashtb_free_iterator(&face_iterator, csmface_t);
        
    } while (there_are_changes == CSMTRUE);
}

// ----------------------------------------------------------------------------------------------------

static CSMBOOL i_is_loop_filled_by_face(struct csmloop_t *loop, struct csmface_t **opposed_face_opt)
{
    CSMBOOL is_filled_by_face;
    struct csmface_t *opposed_face_loc;
    struct csmhedge_t *lhedge, *he_iterator;
    struct csmloop_t *opposed_loop;
    unsigned long no_iters;
    
    is_filled_by_face = CSMTRUE;
    
    lhedge = csmloop_ledge(loop);
    he_iterator = lhedge;
    opposed_loop = NULL;
    opposed_face_loc = NULL;
    no_iters = 0;
    
    do
    {
        struct csmhedge_t *he_iterator_mate;
        struct csmloop_t *he_iterator_mate_loop;
        
        assert(no_iters < 10000);
        no_iters++;
    
        he_iterator_mate = csmopbas_mate(he_iterator);
        he_iterator_mate_loop = csmhedge_loop(he_iterator_mate);
        
        if (opposed_loop == NULL)
        {
            opposed_loop = he_iterator_mate_loop;
        }
        else if (he_iterator_mate_loop != opposed_loop)
        {
            is_filled_by_face = CSMFALSE;
            break;
        }
        
        he_iterator = csmhedge_next(he_iterator);
        
    } while (he_iterator != lhedge);
    
    if (is_filled_by_face == CSMTRUE)
    {
        opposed_face_loc = csmloop_lface(opposed_loop);
        assert(csmface_has_holes(opposed_face_loc) == CSMFALSE);
    }
    else
    {
        opposed_face_loc = NULL;
    }
    
    ASSIGN_OPTIONAL_VALUE(opposed_face_opt, opposed_face_loc);
    
    return is_filled_by_face;
}

// ----------------------------------------------------------------------------------------------------

static void i_remove_hole_filled_by_face(struct csmsolid_t *solid, struct csmface_t *face)
{
    struct csmloop_t *loop1, *loop2;
    struct csmhedge_t *he, *he_next;
    unsigned long no_iters;
    struct csmvertex_t *vertex;
    
    if (csmdebug_debug_enabled() == CSMTRUE)
    {
        csmdebug_print_debug_info("i_remove_hole_filled_by_face(): Before deleting hedges\n");
        csmface_debug_print_info_debug(face, CSMTRUE, NULL);
    }
    
    loop1 = csmface_floops(face);
    loop2 = csmloop_next(loop1);
    assert(csmloop_next(loop2) == NULL);
    
    he = csmloop_ledge(loop1);
    no_iters = 0;
    
    do
    {
        struct csmhedge_t *he_mate;
        struct csmedge_t *he_edge;
        
        assert(no_iters < 10000);
        no_iters++;
        
        he_next = csmhedge_next(he);
        
        he_edge = csmhedge_edge(he);
        he_mate = csmopbas_mate(he);
        
        csmeuler_lkev(&he, &he_mate, NULL, NULL, NULL, NULL);
        
        he = he_next;
        
    } while (csmhedge_edge(he) != NULL);
    
    if (csmdebug_debug_enabled() == CSMTRUE)
    {
        csmdebug_print_debug_info("i_remove_hole_filled_by_face(): After deleting hedges\n");
        csmface_debug_print_info_debug(face, CSMTRUE, NULL);
    }

    vertex = csmhedge_vertex(he);
    
    csmopbas_delhe(&he, NULL, &he);
    assert(he == NULL);
    
    csmface_remove_loop(face, &loop1);
    csmface_remove_loop(face, &loop2);
    csmsolid_remove_face(solid, &face);
    csmsolid_remove_vertex(solid, &vertex);
}

// ----------------------------------------------------------------------------------------------------

void csmsetopcom_delete_holes_filled_by_faces(struct csmsolid_t *solid, const struct csmtolerance_t *tolerances)
{
    struct csmhashtb_iterator(csmface_t) *face_iterator;

    face_iterator = csmsolid_face_iterator(solid);

    while (csmhashtb_has_next(face_iterator, csmface_t) == CSMTRUE)
    {
        struct csmface_t *face;
        struct csmloop_t *face_floops, *loop_iterator;
        
        csmhashtb_next_pair(face_iterator, NULL, &face, csmface_t);
        
        face_floops = csmface_floops(face);
        loop_iterator = face_floops;
        
        do
        {
            struct csmloop_t *next_loop;
            
            next_loop = csmloop_next(loop_iterator);
            
            if (loop_iterator != csmface_flout(face))
            {
                struct csmface_t *opposed_face;
                
                if (i_is_loop_filled_by_face(loop_iterator, &opposed_face) == CSMTRUE)
                {
                    csmface_add_loop_while_removing_from_old(opposed_face, loop_iterator);
                    i_remove_hole_filled_by_face(solid, opposed_face);
                }
            }
            
            loop_iterator = next_loop;
            
        } while (loop_iterator != NULL);
    }
    
    csmhashtb_free_iterator(&face_iterator, csmface_t);
}

// ----------------------------------------------------------------------------------------------------

void csmsetopcom_merge_faces_inside_faces(struct csmsolid_t *solid, const struct csmtolerance_t *tolerances)
{
    CSMBOOL there_are_changes;
    
    there_are_changes = CSMFALSE;
    
    do
    {
        struct csmhashtb_iterator(csmface_t) *outer_face_iterator;
    
        outer_face_iterator = csmsolid_face_iterator(solid);
        there_are_changes = CSMFALSE;
    
        while (csmhashtb_has_next(outer_face_iterator, csmface_t) == CSMTRUE)
        {
            struct csmface_t *outer_face;
            struct csmhashtb_iterator(csmface_t) *inner_face_iterator;
            
            csmhashtb_next_pair(outer_face_iterator, NULL, &outer_face, csmface_t);
            
            inner_face_iterator = csmsolid_face_iterator(solid);
            
            while (csmhashtb_has_next(inner_face_iterator, csmface_t) == CSMTRUE)
            {
                struct csmface_t *inner_face;
                
                csmhashtb_next_pair(inner_face_iterator, NULL, &inner_face, csmface_t);
                
                if (outer_face != inner_face
                        && csmface_should_analyze_intersections_between_faces(outer_face, inner_face) == CSMTRUE
                        && csmface_are_coplanar_faces(outer_face, inner_face) == CSMTRUE
                        && csmface_has_holes(inner_face) == CSMFALSE)
                {
                    struct csmloop_t *inner_face_floops;
                    
                    inner_face_floops = csmface_floops(inner_face);
                    
                    if (csmface_is_loop_contained_in_face(outer_face, inner_face_floops, tolerances) == CSMTRUE)
                    {
                        if (csmdebug_debug_enabled() == CSMTRUE)
                        {
                            csmface_debug_print_info_debug(outer_face, CSMTRUE, NULL);
                            csmface_debug_print_info_debug(inner_face, CSMTRUE, NULL);
                            
                            csmface_mark_setop_null_face(outer_face);
                            csmface_mark_setop_null_face(inner_face);
                            csmdebug_show_viewer();
                        }
                        
                        csmeuler_lkfmrh(outer_face, &inner_face);
                        there_are_changes = CSMTRUE;
                    }
                }
                
                if (there_are_changes == CSMTRUE)
                    break;
            }
            
            csmhashtb_free_iterator(&inner_face_iterator, csmface_t);
            
            if (there_are_changes == CSMTRUE)
                break;
        }
        
        csmhashtb_free_iterator(&outer_face_iterator, csmface_t);
        
    } while (there_are_changes == CSMTRUE);
}

// ----------------------------------------------------------------------------------------------------

void csmsetopcom_move_face_to_solid(
                        unsigned long recursion_level,
                        struct csmface_t *face, struct csmsolid_t *face_solid,
                        struct csmsolid_t *destination_solid)
{
    assert(recursion_level < 10000);
    
    if (csmface_fsolid(face) != destination_solid)
    {
        register struct csmloop_t *loop_iterator;
    
        assert(csmface_fsolid(face) == face_solid);
        
        if (csmdebug_debug_enabled() == CSMTRUE)
        {
            csmdebug_print_debug_info("Moving face %lu (solid %p) to solid %p\n", csmface_id(face), csmface_fsolid(face), destination_solid);
            assert(csmface_fsolid(face) == face_solid);
        }
        
        csmsolid_move_face_to_solid(face_solid, face, destination_solid);
        
        loop_iterator = csmface_floops(face);
        
        while (loop_iterator != NULL)
        {
            register struct csmhedge_t *loop_ledge, *he_iterator;
            unsigned long no_iters;
            
            loop_ledge = csmloop_ledge(loop_iterator);
            he_iterator = loop_ledge;
            no_iters = 0;
            
            do
            {
                struct csmhedge_t *he_mate_iterator;
                struct csmface_t *he_mate_iterator_face;
                struct csmsolid_t *he_mate_iterator_face_solid;
                
                assert(no_iters < 10000);
                no_iters++;
                
                he_mate_iterator = csmopbas_mate(he_iterator);
                he_mate_iterator_face = csmopbas_face_from_hedge(he_mate_iterator);
                he_mate_iterator_face_solid = csmface_fsolid(he_mate_iterator_face);
                
                if (he_mate_iterator_face_solid != destination_solid)
                    csmsetopcom_move_face_to_solid(recursion_level + 1, he_mate_iterator_face, he_mate_iterator_face_solid, destination_solid);
                
                he_iterator = csmhedge_next(he_iterator);
            }
            while (he_iterator != loop_ledge);
            
            loop_iterator = csmloop_next(loop_iterator);
        }
    }
}

// ----------------------------------------------------------------------------------------------------

enum csmsetop_classify_resp_solid_t csmsetopcom_classify_value_respect_to_plane(double value, double tolerance)
{
    switch (csmmath_compare_doubles(value, 0., tolerance))
    {
        case CSMCOMPARE_FIRST_LESS:
            
            return CSMSETOP_CLASSIFY_RESP_SOLID_OUT;
            
        case CSMCOMPARE_EQUAL:
            
            return CSMSETOP_CLASSIFY_RESP_SOLID_ON;
            
        case CSMCOMPARE_FIRST_GREATER:
            
            return CSMSETOP_CLASSIFY_RESP_SOLID_IN;
            
        default_error();
    }
}

// ----------------------------------------------------------------------------------------------------

struct csmface_t *csmsetopcom_face_for_hedge_sector(struct csmhedge_t *hedge, struct csmhedge_t *hedge_next)
{
    struct csmface_t *common_face;
    struct csmface_t *hedge_face, *hedge_next_face;
    
    hedge_face = csmopbas_face_from_hedge(hedge);
    hedge_next_face = csmopbas_face_from_hedge(hedge_next);
    
    if (hedge_face == hedge_next_face)
    {
        common_face = hedge_face;
    }
    else
    {
        struct csmhedge_t *mate_next_hedge;
        struct csmface_t *mate_next_hedge_face;
        
        mate_next_hedge = csmopbas_mate(hedge_next);
        mate_next_hedge_face = csmopbas_face_from_hedge(mate_next_hedge);
        
        if (hedge_face == mate_next_hedge_face)
        {
            common_face = hedge_face;
        }
        else
        {
            struct csmhedge_t *mate_he;
            struct csmface_t *mate_he_face;
        
            mate_he = csmopbas_mate(hedge);
            mate_he_face = csmopbas_face_from_hedge(mate_he);
            
            if (mate_he_face == hedge_next_face)
            {
                common_face = mate_he_face;
            }
            else
            {
                assert(mate_he_face == mate_next_hedge_face);
                common_face = mate_he_face;
            }
        }
    }
    
    return common_face;
}

// ----------------------------------------------------------------------------------------------------

void csmsetopcom_cleanup_solid(struct csmsolid_t *origin_solid, struct csmsolid_t *destination_solid)
{
    struct csmhashtb_iterator(csmface_t) *face_iterator;
    
    assert_no_null(destination_solid);

    face_iterator = csmhashtb_create_iterator(destination_solid->sfaces, csmface_t);
    
    while (csmhashtb_has_next(face_iterator, csmface_t) == CSMTRUE)
    {
        struct csmface_t *face;
        struct csmloop_t *loop_iterator;
        
        csmhashtb_next_pair(face_iterator, NULL, &face, csmface_t);
        loop_iterator = csmface_floops(face);
        
        while (loop_iterator != NULL)
        {
            register struct csmhedge_t *loop_ledge, *he_iterator;
            unsigned long no_iters;
        
            loop_ledge = csmloop_ledge(loop_iterator);
            he_iterator = loop_ledge;
            no_iters = 0;
            
            do
            {
                struct csmedge_t *edge;
                struct csmvertex_t *vertex;
                
                assert(no_iters < 10000);
                no_iters++;
                
                edge = csmhedge_edge(he_iterator);
                vertex = csmhedge_vertex(he_iterator);
                
                if (edge != NULL)
                {
                    if (csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_POS) == he_iterator)
                        csmsolid_move_edge_to_solid(origin_solid, edge, destination_solid);
                    
                    if (csmvertex_hedge(vertex) == he_iterator)
                        csmsolid_move_vertex_to_solid(origin_solid, vertex, destination_solid);
                }
                else
                {
                    assert(csmvertex_hedge(vertex) == he_iterator);
                    csmsolid_move_vertex_to_solid(origin_solid, vertex, destination_solid);
                }
                
                he_iterator = csmhedge_next(he_iterator);
            }
            while (he_iterator != loop_ledge);
            
            loop_iterator = csmloop_next(loop_iterator);
        }
    }
    
    csmhashtb_free_iterator(&face_iterator, csmface_t);
}

// ----------------------------------------------------------------------------------------------------

void csmsetopcom_cleanup_solid_setop(
                        struct csmsolid_t *origin_solid_A, struct csmsolid_t *origin_solid_B,
                        struct csmsolid_t *destination_solid)
{
    struct csmhashtb_iterator(csmface_t) *face_iterator;
    
    assert_no_null(destination_solid);
    
    face_iterator = csmhashtb_create_iterator(destination_solid->sfaces, csmface_t);
    
    while (csmhashtb_has_next(face_iterator, csmface_t) == CSMTRUE)
    {
        struct csmface_t *face;
        struct csmloop_t *loop_iterator;
        
        csmhashtb_next_pair(face_iterator, NULL, &face, csmface_t);
        loop_iterator = csmface_floops(face);
        
        while (loop_iterator != NULL)
        {
            register struct csmhedge_t *loop_ledge, *he_iterator;
            unsigned long no_iters;
        
            loop_ledge = csmloop_ledge(loop_iterator);
            he_iterator = loop_ledge;
            no_iters = 0;
            
            do
            {
                struct csmsolid_t *origin_solid;
                struct csmedge_t *edge;
                struct csmvertex_t *vertex;
                
                assert(no_iters < 10000);
                no_iters++;
                
                origin_solid = csmopbas_solid_aux_from_hedge(he_iterator);
                
                edge = csmhedge_edge(he_iterator);
                vertex = csmhedge_vertex(he_iterator);
                
                if (csmvertex_hedge(vertex) == he_iterator)
                {
                    assert(origin_solid == origin_solid_A || origin_solid == origin_solid_B);
                    csmsolid_move_vertex_to_solid(origin_solid, vertex, destination_solid);
                }
                
                if (edge != NULL)
                {
                    if (csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_POS) == he_iterator)
                    {
                        assert(origin_solid == origin_solid_A || origin_solid == origin_solid_B);
                        csmsolid_move_edge_to_solid(origin_solid, edge, destination_solid);
                    }
                }

                he_iterator = csmhedge_next(he_iterator);
            }
            while (he_iterator != loop_ledge);
            
            loop_iterator = csmloop_next(loop_iterator);
        }
    }
    
    csmhashtb_free_iterator(&face_iterator, csmface_t);
}
