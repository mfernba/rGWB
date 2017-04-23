// Split operation...

#include "csmsplit.h"

#include "csmedge.inl"
#include "csmedge.tli"
#include "csmface.inl"
#include "csmhashtb.inl"
#include "csmhedge.inl"
#include "csmeuler_lmev.inl"
#include "csmmath.inl"
#include "csmmath.tli"
#include "csmopbas.inl"
#include "csmsolid.h"
#include "csmsolid.tli"
#include "csmtolerance.inl"
#include "csmvertex.inl"

#include "a_punter.h"
#include "cyassert.h"
#include "cypespy.h"
#include "defmath.tlh"

ArrEstructura(csmvertex_t);
ArrEstructura(csmedge_t);

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

static void i_append_vertex_if_not_exists(struct csmvertex_t *vertex, ArrEstructura(csmvertex_t) *set_of_on_vertices)
{
    if (arr_ExisteEstructuraST(set_of_on_vertices, csmvertex_t, vertex, struct csmvertex_t, i_equals_vertices2, NULL) == FALSO)
        arr_AppendPunteroST(set_of_on_vertices, vertex, csmvertex_t);
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
        
        csmhashtb_next_pair(edge_iterator, NULL, &edge, csmedge_t);
        
        hedge_pos = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_POS);
        i_classify_hedge_respect_to_plane(hedge_pos, A, B, C, D, &vertex_pos, &dist_to_plane_pos, &cl_resp_plane_pos);
        
        hedge_neg = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_NEG);
        i_classify_hedge_respect_to_plane(hedge_neg, A, B, C, D, &vertex_neg, &dist_to_plane_neg, &cl_resp_plane_neg);
        
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
        }
        else
        {
            if (cl_resp_plane_pos == i_POSITION_ON)
                i_append_vertex_if_not_exists(vertex_pos, set_of_on_vertices);
            
            if (cl_resp_plane_neg == i_POSITION_ON)
                i_append_vertex_if_not_exists(vertex_neg, set_of_on_vertices);
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

static void i_insert_nulledges_to_split_solid_at_on_vertex_neihborhood(
                        struct csmvertex_t *vertex,
                        double A, double B, double C, double D,
                        ArrEstructura(csmedge_t) *set_of_null_edges)
{
    ArrEstructura(i_neighborhood_t) *vertex_neighborhood;
    unsigned long start_idx;
    
    vertex_neighborhood = i_initial_vertex_neighborhood(vertex, A, B, C, D);
    i_reclassify_on_sectors_vertex_neighborhood(A, B, C, D, vertex_neighborhood);
    i_reclassify_on_edges_vertex_neighborhood(A, B, C, D, vertex_neighborhood);
    
    if (i_could_locate_begin_sequence(vertex_neighborhood, &start_idx) == CIERTO)
    {
        unsigned long num_sectors;
        unsigned long idx;
        struct i_neighborhood_t *head_neighborhood;
        double x_split, y_split, z_split;
        unsigned long num_iters;
        CYBOOL process_next_sequence;

        num_sectors = arr_NumElemsPunteroST(vertex_neighborhood, i_neighborhood_t);
        assert(num_sectors > 0);
        
        idx = start_idx;
        
        head_neighborhood = arr_GetPunteroST(vertex_neighborhood, start_idx, i_neighborhood_t);
        assert_no_null(head_neighborhood);
        
        csmvertex_get_coordenadas(csmhedge_vertex(head_neighborhood->hedge), &x_split, &y_split, &z_split);
        
        num_iters = 0;
        process_next_sequence = CIERTO;
        
        while (process_next_sequence == FALSO)
        {
            struct i_neighborhood_t *tail_neighborhood;
            struct csmedge_t *null_edge;
            
            assert(num_iters < 100000);
            num_iters++;
            
            while (i_is_above_below_sequence_at_index(vertex_neighborhood, idx) == FALSO)
            {
                assert(num_iters < 100000);
                num_iters++;
                
                idx = (idx + 1) % num_sectors;
            }
            
            tail_neighborhood = arr_GetPunteroST(vertex_neighborhood, idx, i_neighborhood_t);
            assert_no_null(tail_neighborhood);
            
            csmeuler_lmev(head_neighborhood->hedge, tail_neighborhood->hedge, x_split, y_split, z_split, NULL, &null_edge, NULL, NULL);
            arr_AppendPunteroST(set_of_null_edges, null_edge, csmedge_t);
            
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

void csmsplit_split_solid(
                        const struct csmsolid_t *solid,
                        double A, double B, double C, double D,
                        CYBOOL *there_is_solid_above, struct csmsolid_t **solid_above,
                        CYBOOL *there_is_solid_below, struct csmsolid_t **solid_below)
{
    CYBOOL there_is_solid_above_loc, there_is_solid_below_loc;
    struct csmsolid_t *solid_above_loc, *solid_below_loc;
    struct csmsolid_t *work_solid;
    ArrEstructura(csmvertex_t) *set_of_on_vertices;
    ArrEstructura(csmedge_t) *set_of_null_edges;
    
    work_solid = csmsolid_duplicate(solid);
    
    set_of_on_vertices = i_split_edges_by_plane(work_solid, A, B, C, D);
    set_of_null_edges = i_insert_nulledges_to_split_solid(A, B, C, D, set_of_on_vertices);


    *there_is_solid_above = there_is_solid_above_loc;
    *solid_above = solid_above_loc;
    
    *there_is_solid_below = there_is_solid_below_loc;
    *solid_below = solid_below;
    
    arr_DestruyeEstructurasST(&set_of_on_vertices, NULL, csmvertex_t);
    arr_DestruyeEstructurasST(&set_of_null_edges, NULL, csmedge_t);
}
