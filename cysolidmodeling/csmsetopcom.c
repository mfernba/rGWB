// Commons operations for Set Operators...

#include "csmsetopcom.inl"

#include "csmdebug.inl"
#include "csmedge.inl"
#include "csmedge.tli"
#include "csmface.inl"
#include "csmhedge.inl"
#include "csmloop.inl"
#include "csmeuler_laringmv.inl"
#include "csmeuler_lkef.inl"
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
#include "csmsolid.tli"
#include "csmtolerance.inl"
#include "csmvertex.inl"
#include "csmvertex.tli"

#include "cyassert.h"
#include "a_punter.h"

// ----------------------------------------------------------------------------------------------------

static CYBOOL i_equals_vertices(const struct csmvertex_t *vertex1, const struct csmvertex_t *vertex2)
{
    if (vertex1 == vertex2)
        return CIERTO;
    else
        return FALSO;
}

// ----------------------------------------------------------------------------------------------------

static bool i_equals_vertices2(const struct csmvertex_t *vertex1, const struct csmvertex_t *vertex2)
{
    return i_equals_vertices(vertex1, vertex2) == CIERTO ? true: false;
}

// ----------------------------------------------------------------------------------------------------

void csmsetopcom_append_vertex_if_not_exists(struct csmvertex_t *vertex, ArrEstructura(csmvertex_t) *set_of_on_vertices)
{
    if (arr_ExisteEstructuraST(set_of_on_vertices, csmvertex_t, vertex, struct csmvertex_t, i_equals_vertices2, NULL) == FALSO)
        arr_AppendPunteroST(set_of_on_vertices, vertex, csmvertex_t);
}

// ----------------------------------------------------------------------------------------------------

static CYBOOL i_is_hedge_on_inner_loop(struct csmhedge_t *hedge)
{
    struct csmloop_t *hedge_loop;
    struct csmface_t *face_loop;
    
    hedge_loop = csmhedge_loop(hedge);
    face_loop = csmloop_lface(hedge_loop);
    
    if (csmface_flout(face_loop) != hedge_loop)
        return CIERTO;
    else
        return FALSO;
}

// ----------------------------------------------------------------------------------------------------

static CYBOOL i_is_hedge_on_hedge_cycle_with_a_side_on_a_hole(struct csmhedge_t *hedge)
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
        
        if (i_is_hedge_on_inner_loop(iterator) == CIERTO || i_is_hedge_on_inner_loop(iterator_mate) == CIERTO)
            return CIERTO;
        else
            iterator = csmhedge_next(iterator);
        
    } while (iterator != hedge);
    
    return FALSO;
}

// ----------------------------------------------------------------------------------------------------

CYBOOL csmsetopcom_is_edge_on_cycle_of_edges_with_a_side_on_a_hole(struct csmedge_t *edge)
{
    struct csmhedge_t *hedge_pos, *hedge_neg;
    
    hedge_pos = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_POS);
    hedge_neg = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_NEG);
    
    if (i_is_hedge_on_hedge_cycle_with_a_side_on_a_hole(hedge_pos) == CIERTO
            || i_is_hedge_on_hedge_cycle_with_a_side_on_a_hole(hedge_neg) == CIERTO)
    {
        return CIERTO;
    }
    else
    {
        return FALSO;
    }
}

// ----------------------------------------------------------------------------------------------------

CYBOOL csmsetopcom_hedges_are_neighbors(struct csmhedge_t *he1, struct csmhedge_t *he2)
{
    struct csmface_t *face_he1, *face_he2;
    
    face_he1 = csmopbas_face_from_hedge(he1);
    face_he2 = csmopbas_face_from_hedge(he2);
    
    if (face_he1 != face_he2)
    {
        return FALSO;
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
            return CIERTO;
        else if (he1 == he2_edge_he1 && he2 == he1_edge_he2)
            return CIERTO;
        else
            return FALSO;
    }
}

// ----------------------------------------------------------------------------------------------------

static enum comparac_t i_compare_coords(double coord1, double coord2, double tolerance)
{
    switch (csmmath_compare_doubles(coord1, coord2, tolerance))
    {
        case CSMMATH_VALUE1_LESS_THAN_VALUE2:
            
            return comparac_PRIMERO_MENOR;
            
        case CSMMATH_EQUAL_VALUES:
            
            return comparac_IGUALES;
            
        case CSMMATH_VALUE1_GREATER_THAN_VALUE2:
            
            return comparac_PRIMERO_MAYOR;
            
        default_error();
    }
}

// ----------------------------------------------------------------------------------------------------

static enum comparac_t i_compare_edges_by_coord(const struct csmedge_t *edge1, const struct csmedge_t *edge2)
{
    enum comparac_t comparacion;
    const struct csmhedge_t *he1_edge1, *he1_edge2;
    const struct csmvertex_t *vertex1, *vertex2;
    double x1, y1, z1, x2, y2, z2;
    double tolerance;
    
    he1_edge1 = csmedge_hedge_lado_const(edge1, CSMEDGE_LADO_HEDGE_POS);
    vertex1 = csmhedge_vertex_const(he1_edge1);
    csmvertex_get_coordenadas(vertex1, &x1, &y1, &z1);
    
    he1_edge2 = csmedge_hedge_lado_const(edge2, CSMEDGE_LADO_HEDGE_POS);
    vertex2 = csmhedge_vertex_const(he1_edge2);
    csmvertex_get_coordenadas(vertex2, &x2, &y2, &z2);

    tolerance = csmtolerance_equal_coords();
    
    comparacion = i_compare_coords(x1, x2, tolerance);
    
    if (comparacion == comparac_IGUALES)
    {
        comparacion = i_compare_coords(y1, y2, tolerance);
        
        if (comparacion == comparac_IGUALES)
            comparacion = i_compare_coords(z1, z2, tolerance);
    }
    
    return comparacion;
}

// ----------------------------------------------------------------------------------------------------

void csmsetopcom_sort_edges_lexicographically_by_xyz(ArrEstructura(csmedge_t) *set_of_null_edges)
{
    arr_QSortPunteroST(set_of_null_edges, i_compare_edges_by_coord, csmedge_t);
}

// ----------------------------------------------------------------------------------------------------

void csmsetopcom_print_set_of_null_edges(const ArrEstructura(csmedge_t) *set_of_null_edges)
{
    unsigned long i, num_null_edges;
    
    num_null_edges = arr_NumElemsPunteroST(set_of_null_edges, csmedge_t);
    
    csmdebug_print_debug_info("Set of null edges:\n");
    
    for (i = 0; i < num_null_edges; i++)
    {
        const struct csmedge_t *edge;
        const struct csmhedge_t *he1, *he2;
        const struct csmvertex_t *vertex1;
        double x, y, z;
        
        edge = arr_GetPunteroST(set_of_null_edges, i, csmedge_t);
        assert_no_null(edge);
        
        he1 = csmedge_hedge_lado_const(edge, CSMEDGE_LADO_HEDGE_POS);
        he2 = csmedge_hedge_lado_const(edge, CSMEDGE_LADO_HEDGE_NEG);
        
        vertex1 = csmhedge_vertex_const(he1);
        csmvertex_get_coordenadas(vertex1, &x, &y, &z);
        
        csmdebug_print_debug_info(
                "\t[%lu] (%5.3g, %5.3g, %5.3g)\t[he1, loop, face] = (%lu, %lu, %lu)\t[he2, loop, face] = (%lu, %lu, %lu)) \n",
                csmedge_id(edge), x, y, z,
                csmhedge_id(he1), csmloop_id(csmhedge_loop((struct csmhedge_t *)he1)), csmface_id(csmopbas_face_from_hedge((struct csmhedge_t *)he1)),
                csmhedge_id(he2), csmloop_id(csmhedge_loop((struct csmhedge_t *)he2)), csmface_id(csmopbas_face_from_hedge((struct csmhedge_t *)he2)));
    }
}

// ----------------------------------------------------------------------------------------------------

CYBOOL csmsetopcom_is_loose_end(struct csmhedge_t *hedge, ArrEstructura(csmhedge_t) *loose_ends)
{
    unsigned long i, no_loose_end;
    
    no_loose_end = arr_NumElemsPunteroST(loose_ends, csmhedge_t);
    
    for (i = 0; i < no_loose_end; i++)
    {
        if (arr_GetPunteroST(loose_ends, i, csmhedge_t) == hedge)
            return CIERTO;
    }
    
    return FALSO;
}

// ----------------------------------------------------------------------------------------------------

void csmsetopcom_print_debug_info_loose_ends(const ArrEstructura(csmhedge_t) *loose_ends)
{
    unsigned long i, no_loose_end;
    
    csmdebug_print_debug_info("Loose ends [");
    
    no_loose_end = arr_NumElemsPunteroST(loose_ends, csmhedge_t);
    
    for (i = 0; i < no_loose_end; i++)
    {
        const struct csmhedge_t *hedge;
        
        hedge = arr_GetPunteroConstST(loose_ends, i, csmhedge_t);
        assert_no_null(hedge);
        
        if (i > 0)
            csmdebug_print_debug_info(", ");
        
        csmdebug_print_debug_info("%lu", csmhedge_id(hedge));
    }
    
    csmdebug_print_debug_info("]\n");
}


// ----------------------------------------------------------------------------------------------------

void csmsetopcom_join_hedges(struct csmhedge_t *he1, struct csmhedge_t *he2)
{
    struct csmface_t *old_face, *new_face;
    struct csmhedge_t *he1_next, *he1_next_next;
    
    old_face = csmopbas_face_from_hedge(he1);
    
    if (csmhedge_loop(he1) == csmhedge_loop(he2))
    {
        struct csmhedge_t *he1_prev, *he1_prev_prev;
        
        he1_prev = csmhedge_prev(he1);
        he1_prev_prev = csmhedge_prev(he1);
        
        if (he1_prev_prev != he2)
        {
            struct csmhedge_t *he2_next;
            
            he2_next = csmhedge_next(he2);
            csmeuler_lmef(he1, he2_next, &new_face, NULL, NULL);
            
            if (csmdebug_debug_enabled() == CIERTO)
            {
                csmdebug_print_debug_info("(SAME LOOP) joining edges (%lu, %lu) with LMEF, new face %lu.\n", csmhedge_id(he1), csmhedge_id(he2), csmface_id(new_face));
                //csmsolid_print_debug(csmface_fsolid(new_face), CIERTO);
            }
        }
        else
        {
            if (csmdebug_debug_enabled() == CIERTO)
                csmdebug_print_debug_info("(SAME LOOP) joining edges (%lu, %lu). Already connected.\n", csmhedge_id(he1), csmhedge_id(he2));
            
            new_face = NULL;
        }
    }
    else
    {
        struct csmhedge_t *he2_next;
        
        new_face = NULL;
        
        he2_next = csmhedge_next(he2);
        csmeuler_lmekr(he1, he2_next, NULL, NULL);
        
        if (csmdebug_debug_enabled() == CIERTO)
            csmdebug_print_debug_info("(DIFFERENT LOOP) joining edges (%lu, %lu) with LMEKR, lmekr he1 with %lu\n", csmhedge_id(he1), csmhedge_id(he2), csmhedge_id(he2_next));
    }
    
    he1_next = csmhedge_next(he1);
    he1_next_next = csmhedge_next(he1_next);
    
    if (he1_next_next != he2)
    {
        struct csmloop_t *old_face_floops;
        struct csmface_t *second_new_face;

        if (csmdebug_debug_enabled() == CIERTO)
            csmdebug_print_debug_info("Split(): joining edges (%lu, %lu) with LMEF between (%lu, %lu)\n", csmhedge_id(he1), csmhedge_id(he2), csmhedge_id(he2), csmhedge_id(he1_next));
        
        csmeuler_lmef(he2, he1_next, &second_new_face, NULL, NULL);

        if (csmdebug_debug_enabled() == CIERTO)
        {
            csmdebug_print_debug_info("\tFace %lu created\n", csmface_id(second_new_face));
            //csmsolid_print_debug(csmface_fsolid(second_new_face), CIERTO);
        }
        
        old_face_floops = csmface_floops(old_face);
        
        if (new_face != NULL && csmloop_next(old_face_floops) != NULL)
            csmeuler_laringmv(old_face, new_face);
    }
}

// ----------------------------------------------------------------------------------------------------

static bool i_is_same_edge_by_ptr(const struct csmedge_t *edge1, const struct csmedge_t *edge2)
{
    return edge1 == edge2;
}

// ----------------------------------------------------------------------------------------------------

static void i_cut_he(
                    struct csmhedge_t *hedge,
                    CYBOOL do_lmekr_he1_he2,
                    ArrEstructura(csmedge_t) *set_of_null_edges,
                    ArrEstructura(csmface_t) *set_of_null_faces,
                    unsigned long *no_null_edges_deleted)
{
    unsigned long idx;
    struct csmsolid_t *solid;
    struct csmedge_t *edge;
    struct csmhedge_t *he1_edge, *he2_edge;
    
    assert_no_null(no_null_edges_deleted);
    
    solid = csmopbas_solid_from_hedge(hedge);
    
    edge = csmhedge_edge(hedge);
    he1_edge = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_POS);
    he2_edge = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_NEG);

    idx = arr_BuscarEstructuraST(set_of_null_edges, csmedge_t, edge, struct csmedge_t, i_is_same_edge_by_ptr);
    assert(idx != ULONG_MAX);
    arr_BorrarEstructuraST(set_of_null_edges, idx, NULL, csmedge_t);
    (*no_null_edges_deleted)++;
    
    if (csmhedge_loop(he1_edge) == csmhedge_loop(he2_edge))
    {
        struct csmface_t *null_face;
        
        null_face = csmopbas_face_from_hedge(hedge);
        arr_AppendPunteroST(set_of_null_faces, null_face, csmface_t);
        
        if (csmdebug_debug_enabled() == CIERTO)
        {
            csmdebug_print_debug_info("(CUTTING HE)  (%lu, %lu) with LKEMR\n", csmhedge_id(he1_edge), csmhedge_id(he2_edge));
            //csmsolid_print_debug(csmopbas_solid_from_hedge(hedge), CIERTO);
        }
        
        if (do_lmekr_he1_he2 == CIERTO)
            csmeuler_lkemr(&he1_edge, &he2_edge, NULL, NULL);
        else
            csmeuler_lkemr(&he2_edge, &he1_edge, NULL, NULL);
        
        //if (csmdebug_debug_enabled() == CIERTO)
            //csmsolid_print_debug(solid, CIERTO);
    }
    else
    {
        if (csmdebug_debug_enabled() == CIERTO)
            csmdebug_print_debug_info("(CUTTING HE)  (%lu, %lu) with LKEF\n", csmhedge_id(he1_edge), csmhedge_id(he2_edge));
        
        csmeuler_lkef(&he1_edge, &he2_edge);
        
        //if (csmdebug_debug_enabled() == CIERTO)
            //csmsolid_print_debug(solid, CIERTO);
    }
}

// ----------------------------------------------------------------------------------------------------

void csmsetopcom_cut_he_split(
                    struct csmhedge_t *hedge,
                    ArrEstructura(csmedge_t) *set_of_null_edges,
                    ArrEstructura(csmface_t) *set_of_null_faces,
                    unsigned long *no_null_edges_deleted)
{
    CYBOOL do_lmekr_he1_he2;
    
    do_lmekr_he1_he2 = CIERTO;
    i_cut_he(hedge, do_lmekr_he1_he2, set_of_null_edges, set_of_null_faces, no_null_edges_deleted);
}

// ----------------------------------------------------------------------------------------------------

void csmsetopcom_cut_he_setop(
                    struct csmhedge_t *hedge,
                    ArrEstructura(csmedge_t) *set_of_null_edges,
                    ArrEstructura(csmface_t) *set_of_null_faces,
                    unsigned long *no_null_edges_deleted)
{
    CYBOOL do_lmekr_he1_he2;
    
    do_lmekr_he1_he2 = FALSO;
    i_cut_he(hedge, do_lmekr_he1_he2, set_of_null_edges, set_of_null_faces, no_null_edges_deleted);
}

// ----------------------------------------------------------------------------------------------------

ArrEstructura(csmface_t) *csmsetopcom_convert_inner_loops_of_null_faces_to_faces_solid_below(ArrEstructura(csmface_t) *set_of_null_faces)
{
    ArrEstructura(csmface_t) *set_of_null_faces_below;
    unsigned long i, no_null_faces;
    
    no_null_faces = arr_NumElemsPunteroST(set_of_null_faces, csmface_t);
    assert(no_null_faces > 0);
    
    set_of_null_faces_below = arr_CreaPunteroST(0, csmface_t);
    
    for (i = 0; i < no_null_faces; i++)
    {
        struct csmface_t *null_face;
        struct csmloop_t *floops, *loop_to_move;
        struct csmface_t *new_face;
        
        null_face = arr_GetPunteroST(set_of_null_faces, i, csmface_t);
        
        floops = csmface_floops(null_face);
        loop_to_move = floops;
        
        do
        {
            struct csmloop_t *next_loop;

            next_loop = csmloop_next(loop_to_move);
            
            if (loop_to_move != csmface_flout(null_face))
            {
                csmeuler_lmfkrh(loop_to_move, &new_face);
                arr_AppendPunteroST(set_of_null_faces_below, new_face, csmface_t);
            }
            
            loop_to_move = next_loop;
                
        } while (loop_to_move != NULL);
    }
    
    return set_of_null_faces_below;
}

// ----------------------------------------------------------------------------------------------------

static CYBOOL i_is_face_originated_by_hole(struct csmface_t *face)
{
    struct csmloop_t *floops;
    
    floops = csmface_floops(face);
    
    if (csmloop_next(floops) != NULL)
    {
        return FALSO;
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
            
            if (csmvertex_has_mask_attrib(vertex, CSMVERTEX_MASK_VERTEX_ON_HOLE_LOOP) == FALSO)
                return FALSO;
            
        } while (hedge_iterator != loop_ledge);
        
        return CIERTO;
    }
}

// ----------------------------------------------------------------------------------------------------

void csmsetopcom_reintroduce_holes_in_corresponding_faces(ArrEstructura(csmface_t) *set_of_null_faces)
{
    unsigned long no_iters;
    CYBOOL did_delete_faces;
    
    no_iters = 0;
    
    do
    {
        unsigned long i, no_null_faces;

        assert(no_iters < 1000000);
        no_iters++;
        
        no_null_faces = arr_NumElemsPunteroST(set_of_null_faces, csmface_t);
        assert(no_null_faces > 0);
        
        did_delete_faces = FALSO;
        
        for (i = 0; i < no_null_faces && did_delete_faces == FALSO; i++)
        {
            struct csmface_t *face_i;
            
            face_i = arr_GetPunteroST(set_of_null_faces, i, csmface_t);
            
            if (i_is_face_originated_by_hole(face_i) == CIERTO)
            {
                unsigned long j;
                struct csmloop_t *floops_face_i;
                
                floops_face_i = csmface_floops(face_i);
            
                for (j = 0; j < no_null_faces && did_delete_faces == FALSO; j++)
                {
                    if (i != j)
                    {
                        struct csmface_t *face_j;
                    
                        face_j = arr_GetPunteroST(set_of_null_faces, j, csmface_t);
                    
                        if (csmface_is_loop_contained_in_face(face_j, floops_face_i) == CIERTO)
                        {
                            did_delete_faces = CIERTO;
                            
                            csmeuler_lkfmrh(face_j, &face_i);
                            arr_BorrarEstructuraST(set_of_null_faces, i, NULL, csmface_t);
                        }
                    }
                }
            }
        }
        
    } while (did_delete_faces == CIERTO);
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
        
        if (csmdebug_debug_enabled() == CIERTO)
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
        case CSMMATH_VALUE1_LESS_THAN_VALUE2:
            
            return CSMSETOP_CLASSIFY_RESP_SOLID_OUT;
            
        case CSMMATH_EQUAL_VALUES:
            
            return CSMSETOP_CLASSIFY_RESP_SOLID_ON;
            
        case CSMMATH_VALUE1_GREATER_THAN_VALUE2:
            
            return CSMSETOP_CLASSIFY_RESP_SOLID_IN;
            
        default_error();
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_revert_loop_orientation(struct csmloop_t *loop)
{
    register struct csmhedge_t *loop_ledge, *he_iterator;
    struct csmvertex_t *prev_vertex;
    unsigned long no_iters;
        
    loop_ledge = csmloop_ledge(loop);
    he_iterator = loop_ledge;
    no_iters = 0;
            
    do
    {
        struct csmhedge_t *he_iter_prv, *he_iter_nxt;
        
        assert(no_iters < 10000);
        no_iters++;
        
        he_iter_prv = csmhedge_prev(he_iterator);
        he_iter_nxt = csmhedge_next(he_iterator);
        
        csmhedge_set_next(he_iterator, he_iter_prv);
        csmhedge_set_prev(he_iterator, he_iter_nxt);
        
        he_iterator = he_iter_nxt;
    }
    while (he_iterator != loop_ledge);
    
    prev_vertex = csmhedge_vertex(csmhedge_prev(he_iterator));
    
    do
    {
        struct csmvertex_t *vertex_aux;
        
        vertex_aux = csmhedge_vertex(he_iterator);
        
        csmhedge_set_vertex(he_iterator, prev_vertex);
        csmvertex_set_hedge(prev_vertex, he_iterator);
        
        prev_vertex = vertex_aux;
        
        he_iterator = csmhedge_next(he_iterator);
    }
    while (he_iterator != loop_ledge);
}

// ----------------------------------------------------------------------------------------------------

void csmsetopcom_revert_solid(struct csmsolid_t *solid)
{
    struct csmhashtb_iterator(csmface_t) *face_iterator;
    
    assert_no_null(solid);

    face_iterator = csmhashtb_create_iterator(solid->sfaces, csmface_t);
    
    while (csmhashtb_has_next(face_iterator, csmface_t) == CIERTO)
    {
        struct csmface_t *face;
        struct csmloop_t *loop_iterator;
        
        csmhashtb_next_pair(face_iterator, NULL, &face, csmface_t);
        loop_iterator = csmface_floops(face);
        
        while (loop_iterator != NULL)
        {
            i_revert_loop_orientation(loop_iterator);
            loop_iterator = csmloop_next(loop_iterator);
        }
    }
    
    csmhashtb_free_iterator(&face_iterator, csmface_t);
}

// ----------------------------------------------------------------------------------------------------

void csmsetopcom_cleanup_solid(struct csmsolid_t *origin_solid, struct csmsolid_t *destination_solid)
{
    struct csmhashtb_iterator(csmface_t) *face_iterator;
    
    assert_no_null(destination_solid);

    face_iterator = csmhashtb_create_iterator(destination_solid->sfaces, csmface_t);
    
    while (csmhashtb_has_next(face_iterator, csmface_t) == CIERTO)
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
    
    while (csmhashtb_has_next(face_iterator, csmface_t) == CIERTO)
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
                    struct csmsolid_t *origin_solid;
                    
                    if (csmsolid_contains_edge(origin_solid_A, edge) == CIERTO)
                    {
                        assert(csmsolid_contains_edge(origin_solid_B, edge) == FALSO);
                        origin_solid = origin_solid_A;
                    }
                    else
                    {
                        assert(csmsolid_contains_edge(origin_solid_B, edge) == CIERTO);
                        origin_solid = origin_solid_B;
                    }
                    
                    if (csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_POS) == he_iterator)
                        csmsolid_move_edge_to_solid(origin_solid, edge, destination_solid);
                    
                    if (csmvertex_hedge(vertex) == he_iterator)
                        csmsolid_move_vertex_to_solid(origin_solid, vertex, destination_solid);
                }
                else
                {
                    struct csmsolid_t *origin_solid;
                    
                    assert(csmvertex_hedge(vertex) == he_iterator);
                    
                    if (csmsolid_contains_vertex(origin_solid_A, vertex) == CIERTO)
                    {
                        assert(csmsolid_contains_vertex(origin_solid_B, vertex) == FALSO);
                        origin_solid = origin_solid_A;
                    }
                    else
                    {
                        assert(csmsolid_contains_vertex(origin_solid_B, vertex) == CIERTO);
                        origin_solid = origin_solid_B;
                    }
                    
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
