// Split operation...

#include "csmsplit.h"

#include "csmdebug.inl"
#include "csmedge.inl"
#include "csmedge.tli"
#include "csmface.inl"
#include "csmloop.inl"
#include "csmhashtb.inl"
#include "csmhedge.inl"
#include "csmeuler_lmef.inl"
#include "csmeuler_lmekr.inl"
#include "csmeuler_lmev.inl"
#include "csmeuler_lmfkrh.inl"
#include "csmeuler_lkef.inl"
#include "csmeuler_lkemr.inl"
#include "csmeuler_lkfmrh.inl"
#include "csmeuler_laringmv.inl"
#include "csmmath.inl"
#include "csmmath.tli"
#include "csmopbas.inl"
#include "csmsolid.h"
#include "csmsolid.inl"
#include "csmsolid.tli"
#include "csmsetopcom.inl"
#include "csmtolerance.inl"
#include "csmvertex.inl"
#include "csmvertex.tli"

#include "a_punter.h"
#include "cyassert.h"
#include "cypespy.h"
#include "defmath.tlh"

ArrEstructura(csmvertex_t);
ArrEstructura(csmedge_t);
ArrEstructura(csmhedge_t);
ArrEstructura(csmface_t);

enum i_position_t
{
    i_POSITION_ABOVE,
    i_POSITION_ON,
    i_POSITION_BELOW
};

struct i_neighborhood_t
{
    struct csmhedge_t *hedge;
    enum i_position_t position;
};

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_neighborhood_t *, i_create_neighborhod, (struct csmhedge_t *hedge, enum i_position_t position))
{
    struct i_neighborhood_t *neighborhood;
    
    neighborhood = MALLOC(struct i_neighborhood_t);
    
    neighborhood->hedge = hedge;
    neighborhood->position = position;
    
    return neighborhood;
}

// ----------------------------------------------------------------------------------------------------

static void i_free_neighborhood(struct i_neighborhood_t **neighborhood)
{
    assert_no_null(neighborhood);
    assert_no_null(*neighborhood);
    
    FREE_PP(neighborhood, struct i_neighborhood_t);
}

// ----------------------------------------------------------------------------------------------------

static void i_classify_point_respect_to_plane(
                        double x, double y, double z,
                        double A, double B, double C, double D,
                        double tolerance,
                        double *dist_to_plane_opc, enum i_position_t *cl_resp_plane_opc)
{
    double dist_to_plane_loc;
    enum i_position_t cl_resp_plane_loc;
    
    dist_to_plane_loc = csmmath_signed_distance_point_to_plane(x, y, z, A, B, C, D);
    
    switch (csmmath_compare_doubles(dist_to_plane_loc, 0., tolerance))
    {
        case CSMMATH_VALUE1_LESS_THAN_VALUE2:
            
            cl_resp_plane_loc = i_POSITION_BELOW;
            break;
            
        case CSMMATH_EQUAL_VALUES:
            
            cl_resp_plane_loc = i_POSITION_ON;
            break;
            
        case CSMMATH_VALUE1_GREATER_THAN_VALUE2:
            
            cl_resp_plane_loc = i_POSITION_ABOVE;
            break;
            
        default_error();
    }

    ASIGNA_OPC(dist_to_plane_opc, dist_to_plane_loc);
    ASIGNA_OPC(cl_resp_plane_opc, cl_resp_plane_loc);
    
}

// ----------------------------------------------------------------------------------------------------

static double i_classification_tolerance(struct csmhedge_t *hedge)
{
    struct csmface_t *face_hedge;
    double face_tolerance, general_tolerance;
    
    face_hedge = csmopbas_face_from_hedge(hedge);
    face_tolerance = csmface_tolerace(face_hedge);
    general_tolerance = csmtolerance_equal_coords();
    
    return MAX(face_tolerance, general_tolerance);
}

// ----------------------------------------------------------------------------------------------------

static void i_classify_hedge_respect_to_plane(
                        struct csmhedge_t *hedge,
                        double A, double B, double C, double D,
                        struct csmvertex_t **vertex_opc, double *dist_to_plane_opc,
                        enum i_position_t *cl_resp_plane_opc)
{
    double tolerance;
    struct csmvertex_t *vertex_loc;
    double x_loc, y_loc, z_loc;
    
    tolerance = i_classification_tolerance(hedge);

    vertex_loc = csmhedge_vertex(hedge);
    csmvertex_get_coordenadas(vertex_loc, &x_loc, &y_loc, &z_loc);
    
    i_classify_point_respect_to_plane(
                        x_loc, y_loc, z_loc,
                        A, B, C, D,
                        tolerance,
                        dist_to_plane_opc, cl_resp_plane_opc);

    ASIGNA_OPC(vertex_opc, vertex_loc);
}

// ----------------------------------------------------------------------------------------------------

static enum i_position_t i_classify_hedge_bisector_respect_to_plane(
                        struct csmhedge_t *hedge,
                        double A, double B, double C, double D)
{
    enum i_position_t cl_resp_plane;
    double tolerance;
    struct csmvertex_t *vertex;
    double x, y, z;
    struct csmhedge_t *hedge_prv, *hedge_next;
    struct csmvertex_t *vertex_prv, *vertex_next;
    double x_prv, y_prv, z_prv;
    double x_nxt, y_nxt, z_nxt;
    double Ux_to_prv, Uy_to_prv, Uz_to_prv;
    double Ux_to_nxt, Uy_to_nxt, Uz_to_nxt;
    double Ux_bisector, Uy_bisector, Uz_bisector;
    double x_bisector, y_bisector, z_bisector;
    
    tolerance = i_classification_tolerance(hedge);

    vertex = csmhedge_vertex(hedge);
    csmvertex_get_coordenadas(vertex, &x, &y, &z);
    
    hedge_prv = csmhedge_prev(hedge);
    vertex_prv = csmhedge_vertex(hedge_prv);
    csmvertex_get_coordenadas(vertex_prv, &x_prv, &y_prv, &z_prv);
    
    hedge_next = csmhedge_next(hedge);
    vertex_next = csmhedge_vertex(hedge_next);
    csmvertex_get_coordenadas(vertex_next, &x_nxt, &y_nxt, &z_nxt);
    
    csmmath_vector_between_two_3D_points(x, y, z, x_prv, y_prv, z_prv, &Ux_to_prv, &Uy_to_prv, &Uz_to_prv);
    csmmath_vector_between_two_3D_points(x, y, z, x_nxt, y_nxt, z_nxt, &Ux_to_nxt, &Uy_to_nxt, &Uz_to_nxt);
    
    Ux_bisector = .5 * (Ux_to_prv + Ux_to_nxt);
    Uy_bisector = .5 * (Uy_to_prv + Uy_to_nxt);
    Uz_bisector = .5 * (Uz_to_prv + Uz_to_nxt);
    
    csmmath_make_unit_vector3D(&Ux_bisector, &Uy_bisector, &Uz_bisector);
    
    x_bisector = x + 10. * Ux_bisector;
    y_bisector = y + 10. * Uy_bisector;
    z_bisector = z + 10. * Uz_bisector;
    
    i_classify_point_respect_to_plane(
                        x_bisector, y_bisector, z_bisector,
                        A, B, C, D,
                        tolerance,
                        NULL, &cl_resp_plane);

    return cl_resp_plane;
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static ArrEstructura(csmvertex_t) *, i_split_edges_by_plane, (
                        struct csmsolid_t *work_solid,
                        double A, double B, double C, double D))
{
    ArrEstructura(csmvertex_t) *set_of_on_vertices;
    struct csmhashtb_iterator(csmedge_t) *edge_iterator;
    
    assert_no_null(work_solid);

    edge_iterator = csmhashtb_create_iterator(work_solid->sedges, csmedge_t);
    set_of_on_vertices = arr_CreaPunteroST(0, csmvertex_t);
    
    while (csmhashtb_has_next(edge_iterator, csmedge_t) == CIERTO)
    {
        struct csmedge_t *edge;
        struct csmhedge_t *hedge_pos, *hedge_neg;
        struct csmvertex_t *vertex_pos, *vertex_neg;
        double dist_to_plane_pos, dist_to_plane_neg;
        enum i_position_t cl_resp_plane_pos, cl_resp_plane_neg;
        csmvertex_mask_t new_vertex_algorithm_attrib_mask;
        
        csmhashtb_next_pair(edge_iterator, NULL, &edge, csmedge_t);
        
        hedge_pos = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_POS);
        i_classify_hedge_respect_to_plane(hedge_pos, A, B, C, D, &vertex_pos, &dist_to_plane_pos, &cl_resp_plane_pos);
        
        hedge_neg = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_NEG);
        i_classify_hedge_respect_to_plane(hedge_neg, A, B, C, D, &vertex_neg, &dist_to_plane_neg, &cl_resp_plane_neg);
        
        if (csmsetopcom_is_edge_on_cycle_of_edges_with_a_side_on_a_hole(edge) == CIERTO)
            new_vertex_algorithm_attrib_mask = CSMVERTEX_MASK_VERTEX_ON_HOLE_LOOP;
        else
            new_vertex_algorithm_attrib_mask = CSMVERTEX_NULL_MASK;

        if (csmdebug_debug_enabled() == CIERTO)
        {
            double x_pos, y_pos, z_pos, x_neg, y_neg, z_neg;

            csmvertex_get_coordenadas(vertex_pos, &x_pos, &y_pos, &z_pos);
            csmvertex_get_coordenadas(vertex_neg, &x_neg, &y_neg, &z_neg);
            
            csmdebug_print_debug_info(
                    "Analizing edge %lu (%g, %g, %g) -> (%g, %g, %g)\n",
                    csmedge_id(edge),
                    x_pos, y_pos, z_pos, x_neg, y_neg, z_neg);
        }
        
        if ((cl_resp_plane_pos == i_POSITION_BELOW && cl_resp_plane_neg == i_POSITION_ABOVE)
                || (cl_resp_plane_pos == i_POSITION_ABOVE && cl_resp_plane_neg == i_POSITION_BELOW))
        {
            double t;
            double x_pos, y_pos, z_pos;
            double x_neg, y_neg, z_neg;
            double x_inters, y_inters, z_inters;
            struct csmhedge_t *hedge_neg_next;
            struct csmvertex_t *new_vertex;
            
            t = dist_to_plane_pos / (dist_to_plane_pos - dist_to_plane_neg);
            
            csmvertex_get_coordenadas(vertex_pos, &x_pos, &y_pos, &z_pos);
            csmvertex_get_coordenadas(vertex_neg, &x_neg, &y_neg, &z_neg);
            
            x_inters = x_pos + t * (x_neg - x_pos);
            y_inters = y_pos + t * (y_neg - y_pos);
            z_inters = z_pos + t * (z_neg - z_pos);
            
            hedge_neg_next = csmhedge_next(hedge_neg);
            assert(vertex_pos == csmhedge_vertex(hedge_neg_next));
            
            csmeuler_lmev(hedge_pos, hedge_neg_next, x_inters, y_inters, z_inters, &new_vertex, NULL, NULL, NULL);
            arr_AppendPunteroST(set_of_on_vertices, new_vertex, csmvertex_t);
            
            csmvertex_set_mask_attrib(new_vertex, new_vertex_algorithm_attrib_mask);
            
            if (csmdebug_debug_enabled() == CIERTO)
                csmdebug_print_debug_info("Intersection at (%g, %g, %g), New Vertex Id: %lu\n", x_inters, y_inters, z_inters, csmvertex_id(new_vertex));
        }
        else
        {
            if (cl_resp_plane_pos == i_POSITION_ON)
            {
                csmvertex_set_mask_attrib(vertex_pos, new_vertex_algorithm_attrib_mask);
                csmsetopcom_append_vertex_if_not_exists(vertex_pos, set_of_on_vertices);
                
                if (csmdebug_debug_enabled() == CIERTO)
                    csmdebug_print_debug_info("Already On Vertex %lu\n", csmvertex_id(vertex_pos));
            }
            
            if (cl_resp_plane_neg == i_POSITION_ON)
            {
                csmvertex_set_mask_attrib(vertex_neg, new_vertex_algorithm_attrib_mask);
                csmsetopcom_append_vertex_if_not_exists(vertex_neg, set_of_on_vertices);
                
                if (csmdebug_debug_enabled() == CIERTO)
                    csmdebug_print_debug_info("Already On Vertex %lu\n", csmvertex_id(vertex_neg));
            }
        }
    }
    
    csmhashtb_free_iterator(&edge_iterator, csmedge_t);
    
    return set_of_on_vertices;
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static ArrEstructura(i_neighborhood_t) *, i_initial_vertex_neighborhood, (
                        struct csmvertex_t *vertex,
                        double A, double B, double C, double D))
{
    ArrEstructura(i_neighborhood_t) *vertex_neighborhood;
    register struct csmhedge_t *hedge_iterator, *vertex_hedge;
    unsigned long num_iters;
    
    vertex_neighborhood = arr_CreaPunteroST(0, i_neighborhood_t);
    
    vertex_hedge = csmvertex_hedge(vertex);
    hedge_iterator = vertex_hedge;
    
    num_iters = 0;
    
    do
    {
        struct csmhedge_t *hedge_next;
        enum i_position_t cl_resp_plane;
        struct i_neighborhood_t *hedge_neighborhood;
        
        assert(num_iters < 10000);
        num_iters++;
        
        hedge_next = csmhedge_next(hedge_iterator);
        i_classify_hedge_respect_to_plane(hedge_next, A, B, C, D, NULL, NULL, &cl_resp_plane);
        
        hedge_neighborhood = i_create_neighborhod(hedge_iterator, cl_resp_plane);
        arr_AppendPunteroST(vertex_neighborhood, hedge_neighborhood, i_neighborhood_t);
        
        if (csmopbas_is_convex_hedge(hedge_iterator) == FALSO)
        {
            cl_resp_plane = i_classify_hedge_bisector_respect_to_plane(hedge_iterator, A, B, C, D);
            hedge_neighborhood = i_create_neighborhod(hedge_iterator, cl_resp_plane);
        }
        
        hedge_iterator = csmhedge_next(csmopbas_mate(hedge_iterator));
    }
    while (hedge_iterator != vertex_hedge);
    
    return vertex_neighborhood;
}

// ----------------------------------------------------------------------------------------------------

static void i_reclassify_on_sector_vertex_neighborhood(
                        struct i_neighborhood_t *hedge_neighborhood,
                        struct i_neighborhood_t *next_hedge_neighborhood,
                        double A, double B, double C, double D,
                        double tolerance_coplanarity)
{
    struct csmface_t *face;
    CYBOOL same_orientation;
    
    assert_no_null(hedge_neighborhood);
    assert_no_null(next_hedge_neighborhood);
    
    face = csmopbas_face_from_hedge(hedge_neighborhood->hedge);
    
    if (csmface_is_coplanar_to_plane(face, A, B, C, D, tolerance_coplanarity, &same_orientation) == CIERTO)
    {
        if (same_orientation == CIERTO)
        {
            hedge_neighborhood->position = i_POSITION_BELOW;
            next_hedge_neighborhood->position = i_POSITION_BELOW;
        }
        else
        {
            hedge_neighborhood->position = i_POSITION_ABOVE;
            next_hedge_neighborhood->position = i_POSITION_ABOVE;
        }
    }
}
    
// ----------------------------------------------------------------------------------------------------

static void i_reclassify_on_sectors_vertex_neighborhood(
                        double A, double B, double C, double D,
                        ArrEstructura(i_neighborhood_t) *vertex_neighborhood)
{
    unsigned long i, num_sectors;
    double tolerance_coplanarity;
    
    num_sectors = arr_NumElemsPunteroST(vertex_neighborhood, i_neighborhood_t);
    tolerance_coplanarity = csmtolerance_coplanarity();
    
    for (i = 0; i < num_sectors; i++)
    {
        struct i_neighborhood_t *hedge_neighborhood;
        unsigned long next_idx;
        struct i_neighborhood_t *next_hedge_neighborhood;
        
        hedge_neighborhood = arr_GetPunteroST(vertex_neighborhood, i, i_neighborhood_t);
        
        next_idx = (i + 1) % num_sectors;
        next_hedge_neighborhood = arr_GetPunteroST(vertex_neighborhood, next_idx, i_neighborhood_t);
        
        i_reclassify_on_sector_vertex_neighborhood(
                        hedge_neighborhood,
                        next_hedge_neighborhood,
                        A, B, C, D,
                        tolerance_coplanarity);
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_reclassify_on_edge_vertex_neighborhood(
                        struct i_neighborhood_t *hedge_neighborhood,
                        struct i_neighborhood_t *prev_hedge_neighborhood,
                        struct i_neighborhood_t *next_hedge_neighborhood)
{
    assert_no_null(hedge_neighborhood);
    assert_no_null(prev_hedge_neighborhood);
    assert_no_null(next_hedge_neighborhood);
    
    if (hedge_neighborhood->position == i_POSITION_ON)
    {
        enum i_position_t new_position;

        if (prev_hedge_neighborhood->position == i_POSITION_ABOVE && next_hedge_neighborhood->position == i_POSITION_ABOVE)
        {
            new_position = i_POSITION_BELOW;
        }
        else if (prev_hedge_neighborhood->position == i_POSITION_ABOVE && next_hedge_neighborhood->position == i_POSITION_BELOW)
        {
            new_position = i_POSITION_BELOW;
        }
        else if (prev_hedge_neighborhood->position == i_POSITION_BELOW && next_hedge_neighborhood->position == i_POSITION_BELOW)
        {
            new_position = i_POSITION_ABOVE;
        }
        else
        {
            assert(prev_hedge_neighborhood->position == i_POSITION_BELOW && next_hedge_neighborhood->position == i_POSITION_ABOVE);
            new_position = i_POSITION_BELOW;
        }
        
        hedge_neighborhood->position = new_position;
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_reclassify_on_edges_vertex_neighborhood(
                        double A, double B, double C, double D,
                        ArrEstructura(i_neighborhood_t) *vertex_neighborhood)
{
    unsigned long i, num_sectors;
    double tolerance_coplanarity;
    
    num_sectors = arr_NumElemsPunteroST(vertex_neighborhood, i_neighborhood_t);
    tolerance_coplanarity = csmtolerance_coplanarity();
    
    for (i = 0; i < num_sectors; i++)
    {
        struct i_neighborhood_t *hedge_neighborhood;
        unsigned long prev_idx;
        struct i_neighborhood_t *prev_hedge_neighborhood;
        unsigned long next_idx;
        struct i_neighborhood_t *next_hedge_neighborhood;
        
        hedge_neighborhood = arr_GetPunteroST(vertex_neighborhood, i, i_neighborhood_t);
        
        prev_idx = (num_sectors + i - 1) % num_sectors;
        prev_hedge_neighborhood = arr_GetPunteroST(vertex_neighborhood, prev_idx, i_neighborhood_t);
        
        next_idx = (i + 1) % num_sectors;
        next_hedge_neighborhood = arr_GetPunteroST(vertex_neighborhood, next_idx, i_neighborhood_t);
        
        i_reclassify_on_edge_vertex_neighborhood(
                        hedge_neighborhood,
                        prev_hedge_neighborhood,
                        next_hedge_neighborhood);
    }
}

// ----------------------------------------------------------------------------------------------------

static enum i_position_t i_sector_position(const ArrEstructura(i_neighborhood_t) *vertex_neighborhood, unsigned long idx)
{
    const struct i_neighborhood_t *hedge_neighborhood;
    
    hedge_neighborhood = arr_GetPunteroST(vertex_neighborhood, idx, i_neighborhood_t);
    assert_no_null(hedge_neighborhood);
    
    return hedge_neighborhood->position;    
}

// ----------------------------------------------------------------------------------------------------

static CYBOOL i_is_below_above_sequence_at_index(const ArrEstructura(i_neighborhood_t) *vertex_neighborhood, unsigned long idx)
{
    unsigned long num_sectors;
    
    num_sectors = arr_NumElemsPunteroST(vertex_neighborhood, i_neighborhood_t);
    assert(num_sectors > 0);
    
    if (i_sector_position(vertex_neighborhood, idx) == i_POSITION_BELOW)
    {
        if (i_sector_position(vertex_neighborhood, (idx + 1) % num_sectors) == i_POSITION_ABOVE)
            return CIERTO;
        else
            return FALSO;
    }
    else
    {
        return FALSO;
    }
}

// ----------------------------------------------------------------------------------------------------

static CYBOOL i_could_locate_begin_sequence(const ArrEstructura(i_neighborhood_t) *vertex_neighborhood, unsigned long *start_idx)
{
    CYBOOL found;
    unsigned long start_idx_loc;
    unsigned long i, num_sectors;
    
    num_sectors = arr_NumElemsPunteroST(vertex_neighborhood, i_neighborhood_t);
    assert(num_sectors > 0);
    assert_no_null(start_idx);
    
    found = FALSO;
    start_idx_loc = ULONG_MAX;
    
    for (i = 0; i < num_sectors; i++)
    {
        if (i_is_below_above_sequence_at_index(vertex_neighborhood, i) == CIERTO)
        {
            found = CIERTO;
            start_idx_loc = i;
            break;
        }
    }
    
    *start_idx = start_idx_loc;
    
    return found;
}

// ----------------------------------------------------------------------------------------------------

static CYBOOL i_is_above_below_sequence_at_index(const ArrEstructura(i_neighborhood_t) *vertex_neighborhood, unsigned long idx)
{
    unsigned long num_sectors;
    
    num_sectors = arr_NumElemsPunteroST(vertex_neighborhood, i_neighborhood_t);
    assert(num_sectors > 0);
    
    if (i_sector_position(vertex_neighborhood, idx) == i_POSITION_ABOVE)
    {
        if (i_sector_position(vertex_neighborhood, (idx + 1) % num_sectors) == i_POSITION_BELOW)
            return CIERTO;
        else
            return FALSO;
    }
    else
    {
        return FALSO;
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_print_debug_info_vertex_neighborhood(const char *description, const struct csmvertex_t *vertex, ArrEstructura(i_neighborhood_t) *vertex_neighborhood)
{
    if (csmdebug_debug_enabled() == CIERTO)
    {
        double x, y, z;
        unsigned long i, num_sectors;
        
        csmvertex_get_coordenadas(vertex, &x, &y, &z);
        csmdebug_print_debug_info("Vertex neighborhood [%s]: %lu (%g, %g, %g): ", description, csmvertex_id(vertex), x, y, z);
        
        num_sectors = arr_NumElemsPunteroST(vertex_neighborhood, i_neighborhood_t);
        
        for (i = 0; i < num_sectors; i++)
        {
            struct i_neighborhood_t *hedge_neighborhood;
            
            hedge_neighborhood = arr_GetPunteroST(vertex_neighborhood, i, i_neighborhood_t);
            assert_no_null(hedge_neighborhood);
            
            csmdebug_print_debug_info("(hedge = %lu, classification: %d): ", csmhedge_id(hedge_neighborhood->hedge), hedge_neighborhood->position);
        }
        
        csmdebug_print_debug_info("\n");
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_insert_nulledges_to_split_solid_at_on_vertex_neihborhood(
                        struct csmvertex_t *vertex,
                        double A, double B, double C, double D,
                        ArrEstructura(csmedge_t) *set_of_null_edges)
{
    ArrEstructura(i_neighborhood_t) *vertex_neighborhood;
    unsigned long start_idx;
    csmvertex_mask_t vertex_algorithm_mask;
    
    vertex_algorithm_mask = csmvertex_get_mask_attrib(vertex);
    
    vertex_neighborhood = i_initial_vertex_neighborhood(vertex, A, B, C, D);
    i_print_debug_info_vertex_neighborhood("Initial", vertex, vertex_neighborhood);
    
    i_reclassify_on_sectors_vertex_neighborhood(A, B, C, D, vertex_neighborhood);
    i_print_debug_info_vertex_neighborhood("After Reclassify On Sectors", vertex, vertex_neighborhood);
    
    i_reclassify_on_edges_vertex_neighborhood(A, B, C, D, vertex_neighborhood);
    i_print_debug_info_vertex_neighborhood("After Reclassify On Edges", vertex, vertex_neighborhood);
    
    if (i_could_locate_begin_sequence(vertex_neighborhood, &start_idx) == CIERTO)
    {
        unsigned long num_sectors;
        unsigned long idx;
        struct i_neighborhood_t *head_neighborhood;
        unsigned long num_iters;
        CYBOOL process_next_sequence;

        num_sectors = arr_NumElemsPunteroST(vertex_neighborhood, i_neighborhood_t);
        assert(num_sectors > 0);
        
        idx = start_idx;
        head_neighborhood = arr_GetPunteroST(vertex_neighborhood, (idx + 1) % num_sectors, i_neighborhood_t);
        
        num_iters = 0;
        process_next_sequence = CIERTO;
        
        while (process_next_sequence == CIERTO)
        {
            struct i_neighborhood_t *tail_neighborhood;
            double x_split, y_split, z_split;
            struct csmvertex_t *split_vertex;
            struct csmedge_t *null_edge;
            
            assert_no_null(head_neighborhood);
            assert(num_iters < 100000);
            num_iters++;
            
            while (i_is_above_below_sequence_at_index(vertex_neighborhood, idx) == FALSO)
            {
                assert(num_iters < 100000);
                num_iters++;
                
                idx = (idx + 1) % num_sectors;
            }
            
            tail_neighborhood = arr_GetPunteroST(vertex_neighborhood, (idx + 1) % num_sectors, i_neighborhood_t);
            assert_no_null(tail_neighborhood);
            
            csmvertex_get_coordenadas(csmhedge_vertex(head_neighborhood->hedge), &x_split, &y_split, &z_split);

            if (csmdebug_debug_enabled() == CIERTO)
            {
                csmdebug_print_debug_info(
                        "Inserting null edge at (%g, %g, %g) from hedge %lu to hedge %lu.\n",
                        x_split, y_split, z_split,
                        csmhedge_id(head_neighborhood->hedge),
                        csmhedge_id(tail_neighborhood->hedge));
            }
            
            csmeuler_lmev(head_neighborhood->hedge, tail_neighborhood->hedge, x_split, y_split, z_split, &split_vertex, &null_edge, NULL, NULL);
            arr_AppendPunteroST(set_of_null_edges, null_edge, csmedge_t);
            
            csmvertex_set_mask_attrib(split_vertex, vertex_algorithm_mask);
            
            while (i_is_below_above_sequence_at_index(vertex_neighborhood, idx) == FALSO)
            {
                assert(num_iters < 100000);
                num_iters++;
                
                idx = (idx + 1) % num_sectors;
                
                if (idx == start_idx)
                {
                    process_next_sequence = FALSO;
                    break;
                }
            }
            
            head_neighborhood = arr_GetPunteroST(vertex_neighborhood, (idx + 1) % num_sectors, i_neighborhood_t);
        }
    }
    
    arr_DestruyeEstructurasST(&vertex_neighborhood, i_free_neighborhood, i_neighborhood_t);
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static ArrEstructura(csmedge_t) *, i_insert_nulledges_to_split_solid, (
                        double A, double B, double C, double D,
                        ArrEstructura(csmvertex_t) *set_of_on_vertices))
{
    ArrEstructura(csmedge_t) *set_of_null_edges;
    unsigned long i, num_vertices;
    
    num_vertices = arr_NumElemsPunteroST(set_of_on_vertices, csmvertex_t);
    
    set_of_null_edges = arr_CreaPunteroST(0, csmedge_t);
    
    for (i = 0; i < num_vertices; i++)
    {
        struct csmvertex_t *vertex;
        
        vertex = arr_GetPunteroST(set_of_on_vertices, i, csmvertex_t);
        i_insert_nulledges_to_split_solid_at_on_vertex_neihborhood(vertex, A, B, C, D, set_of_null_edges);
    }
    
    return set_of_null_edges;
}

// ----------------------------------------------------------------------------------------------------

static CYBOOL i_can_join_he(struct csmhedge_t *he, ArrEstructura(csmhedge_t) *loose_ends, struct csmhedge_t **matching_loose_end)
{
    CYBOOL can_join;
    struct csmhedge_t *matching_loose_end_loc;
    unsigned long i, no_loose_ends;
    
    assert_no_null(matching_loose_end);
    
    no_loose_ends = arr_NumElemsPunteroST(loose_ends, csmhedge_t);
    
    can_join = FALSO;
    matching_loose_end_loc = NULL;
    
    for (i = 0; i < no_loose_ends; i++)
    {
        struct csmhedge_t *loose_end;
        
        loose_end = arr_GetPunteroST(loose_ends, i, csmhedge_t);
        
        if (csmsetopcom_hedges_are_neighbors(he, loose_end) == CIERTO)
        {
            can_join = CIERTO;
            matching_loose_end_loc = loose_end;
            
            arr_BorrarEstructuraST(loose_ends, i, NULL, csmhedge_t);
            break;
        }
    }
    
    if (can_join == FALSO)
    {
        matching_loose_end_loc = NULL;
        arr_AppendPunteroST(loose_ends, he, csmhedge_t);
    }
    
    *matching_loose_end = matching_loose_end_loc;
    
    return can_join;
}

// ----------------------------------------------------------------------------------------------------

static void i_join_null_edges(ArrEstructura(csmedge_t) *set_of_null_edges, ArrEstructura(csmface_t) **set_of_null_faces)
{
    ArrEstructura(csmface_t) *set_of_null_faces_loc;
    ArrEstructura(csmhedge_t) *loose_ends;
    unsigned long i, no_null_edges;
    unsigned long no_null_edges_deleted;
    
    csmsetopcom_sort_edges_lexicographically_by_xyz(set_of_null_edges);
    no_null_edges = arr_NumElemsPunteroST(set_of_null_edges, csmedge_t);
    assert(no_null_edges > 0);
    assert_no_null(set_of_null_faces);
    
    set_of_null_faces_loc = arr_CreaPunteroST(0, csmface_t);
    loose_ends = arr_CreaPunteroST(0, csmhedge_t);
    
    no_null_edges_deleted = 0;
    
    for (i = 0; i < no_null_edges; i++)
    {
        unsigned long idx;
        struct csmedge_t *next_edge;
        struct csmhedge_t *he1_next_edge, *he2_next_edge;
        struct csmhedge_t *matching_loose_end_he1, *matching_loose_end_he2;
        
        if (csmdebug_debug_enabled() == CIERTO)
            csmsetopcom_print_set_of_null_edges(set_of_null_edges);
        
        idx = i - no_null_edges_deleted;
        next_edge = arr_GetPunteroST(set_of_null_edges, idx, csmedge_t);
        
        he1_next_edge = csmedge_hedge_lado(next_edge, CSMEDGE_LADO_HEDGE_POS);
        he2_next_edge = csmedge_hedge_lado(next_edge, CSMEDGE_LADO_HEDGE_NEG);
     
        if (i_can_join_he(he1_next_edge, loose_ends, &matching_loose_end_he1) == CIERTO)
        {
            csmsetopcom_join_hedges(matching_loose_end_he1, he1_next_edge);
            
            if (csmsetopcom_is_loose_end(csmopbas_mate(matching_loose_end_he1), loose_ends) == FALSO)
                csmsetopcom_cut_he(matching_loose_end_he1, set_of_null_edges, set_of_null_faces_loc, &no_null_edges_deleted);
        }
        else
        {
            matching_loose_end_he1 = NULL;
        }

        if (i_can_join_he(he2_next_edge, loose_ends, &matching_loose_end_he2) == CIERTO)
        {
            csmsetopcom_join_hedges(matching_loose_end_he2, he2_next_edge);
            
            if (csmsetopcom_is_loose_end(csmopbas_mate(matching_loose_end_he2), loose_ends) == FALSO)
                csmsetopcom_cut_he(matching_loose_end_he2, set_of_null_edges, set_of_null_faces_loc, &no_null_edges_deleted);
        }
        else
        {
            matching_loose_end_he2 = NULL;
        }
        
        if (matching_loose_end_he1 != NULL && matching_loose_end_he2 != NULL)
            csmsetopcom_cut_he(he1_next_edge, set_of_null_edges, set_of_null_faces_loc, &no_null_edges_deleted);
        
        if (csmdebug_debug_enabled() == CIERTO)
        {
            csmsetopcom_print_debug_info_loose_ends(loose_ends);
            csmsolid_print_debug(csmopbas_solid_from_hedge(he1_next_edge), CIERTO);
        }
    }
    
    *set_of_null_faces = set_of_null_faces_loc;
    
    assert(arr_NumElemsPunteroST(set_of_null_edges, csmedge_t) == 0);
    assert(arr_NumElemsPunteroST(loose_ends, csmhedge_t) == 0);
    arr_DestruyeEstructurasST(&loose_ends, NULL, csmhedge_t);
}

// ----------------------------------------------------------------------------------------------------

static void i_finish_split(
                        ArrEstructura(csmface_t) *set_of_null_faces,
                        struct csmsolid_t *work_solid,
                        struct csmsolid_t **solid_above, struct csmsolid_t **solid_below)
{
    unsigned long i, no_null_faces;
    struct csmsolid_t *solid_above_loc, *solid_below_loc;
    ArrEstructura(csmface_t) *set_of_null_faces_below;

    assert_no_null(solid_above);
    assert_no_null(solid_below);

    if (csmdebug_debug_enabled() == CIERTO)
        csmsolid_print_debug(work_solid, CIERTO);

    set_of_null_faces_below = csmsetopcom_convert_inner_loops_of_null_faces_to_faces_solid_below(set_of_null_faces);
    
    csmsolid_redo_geometric_generated_data(work_solid);
    csmsetopcom_reintroduce_holes_in_corresponding_faces(set_of_null_faces);
    csmsetopcom_reintroduce_holes_in_corresponding_faces(set_of_null_faces_below);

    no_null_faces = arr_NumElemsPunteroST(set_of_null_faces, csmface_t);
    assert(no_null_faces == arr_NumElemsPunteroST(set_of_null_faces_below, csmface_t));
    assert(no_null_faces > 0);
    
    if (csmdebug_debug_enabled() == CIERTO)
        csmsolid_print_debug(work_solid, CIERTO);
    
    solid_above_loc = csmsolid_crea_vacio();
    solid_below_loc = csmsolid_crea_vacio();
    
    for (i = 0; i < no_null_faces; i++)
    {
        struct csmface_t *face_to_solid_above;
        struct csmface_t *face_to_solid_below;
        
        face_to_solid_above = arr_GetPunteroST(set_of_null_faces, i, csmface_t);
        csmsetopcom_move_face_to_solid(0, face_to_solid_above, work_solid, solid_above_loc);
        
        face_to_solid_below = arr_GetPunteroST(set_of_null_faces_below, i, csmface_t);
        csmsetopcom_move_face_to_solid(0, face_to_solid_below, work_solid, solid_below_loc);
    }
    
    csmsetopcom_cleanup_solid(work_solid, solid_above_loc);
    csmsetopcom_cleanup_solid(work_solid, solid_below_loc);

    if (csmdebug_debug_enabled() == CIERTO)
    {
        csmsolid_print_debug(work_solid, CIERTO);
        csmsolid_print_debug(solid_above_loc, CIERTO);
        csmsolid_print_debug(solid_below_loc, CIERTO);
    }
    
    *solid_above = solid_above_loc;
    *solid_below = solid_below_loc;
    
    arr_DestruyeEstructurasST(&set_of_null_faces_below, NULL, csmface_t);
}

// ----------------------------------------------------------------------------------------------------

CYBOOL csmsplit_does_plane_split_solid(
                        const struct csmsolid_t *solid,
                        double A, double B, double C, double D,
                        struct csmsolid_t **solid_above, struct csmsolid_t **solid_below)
{
    CYBOOL does_plane_split_solid;
    struct csmsolid_t *solid_above_loc, *solid_below_loc;
    struct csmsolid_t *work_solid;
    ArrEstructura(csmvertex_t) *set_of_on_vertices;
    ArrEstructura(csmedge_t) *set_of_null_edges;

    assert_no_null(solid_above);
    assert_no_null(solid_below);
    
    work_solid = csmsolid_duplicate(solid);
    csmsolid_redo_geometric_generated_data(work_solid);
    csmsolid_clear_algorithm_vertex_mask(work_solid);
    
    set_of_on_vertices = i_split_edges_by_plane(work_solid, A, B, C, D);
    set_of_null_edges = i_insert_nulledges_to_split_solid(A, B, C, D, set_of_on_vertices);
    
    if (arr_NumElemsPunteroST(set_of_null_edges, csmedge_t) == 0)
    {
        does_plane_split_solid = FALSO;
        
        solid_above_loc = NULL;
        solid_below_loc = NULL;
    }
    else
    {
        ArrEstructura(csmface_t) *set_of_null_faces;
        
        does_plane_split_solid = CIERTO;
        
        i_join_null_edges(set_of_null_edges, &set_of_null_faces);
        i_finish_split(set_of_null_faces, work_solid, &solid_above_loc, &solid_below_loc);

        csmsolid_clear_algorithm_vertex_mask(solid_above_loc);
        csmsolid_clear_algorithm_vertex_mask(solid_below_loc);
        
        assert(csmsolid_is_empty(work_solid) == CIERTO);

        arr_DestruyeEstructurasST(&set_of_null_faces, NULL, csmface_t);
    }

    *solid_above = solid_above_loc;
    *solid_below = solid_below_loc;
    
    csmsolid_free(&work_solid);
    arr_DestruyeEstructurasST(&set_of_on_vertices, NULL, csmvertex_t);
    arr_DestruyeEstructurasST(&set_of_null_edges, NULL, csmedge_t);
    
    return does_plane_split_solid;
}
