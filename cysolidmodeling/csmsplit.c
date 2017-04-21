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

struct i_neighborhood_t
{
    struct csmhedge_t *hedge;
    enum csmmath_double_relation_t position;
};

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_neighborhood_t *, i_create_neighborhod, (struct csmhedge_t *hedge, enum csmmath_double_relation_t position))
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

static void i_classify_hedge_respect_to_plane(
                        struct csmhedge_t *hedge,
                        double A, double B, double C, double D,
                        struct csmvertex_t **vertex, double *dist_to_plane,
                        enum csmmath_double_relation_t *cl_resp_plane)
{
    struct csmface_t *face_hedge;
    double face_tolerance, general_tolerance, tolerance;
    struct csmvertex_t *vertex_loc;
    double x_loc, y_loc, z_loc;
    
    assert_no_null(dist_to_plane);
    assert_no_null(cl_resp_plane);
    
    face_hedge = csmopbas_face_from_hedge(hedge);
    face_tolerance = csmface_tolerace(face_hedge);
    general_tolerance = csmtolerance_equal_coords();
    tolerance = MAX(face_tolerance, general_tolerance);
    
    vertex_loc = csmhedge_vertex(hedge);
    csmvertex_get_coordenadas(vertex_loc, &x_loc, &y_loc, &z_loc);

    *vertex = vertex_loc;
    *dist_to_plane = csmmath_signed_distance_point_to_plane(x_loc, y_loc, z_loc, A, B, C, D);
    *cl_resp_plane = csmmath_compare_doubles(*dist_to_plane, 0., tolerance);
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

static void i_append_vertex_if_not_exists(struct csmvertex_t *vertex, ArrEstructura(csmvertex_t) *set_of_on_vertices)
{
    if (arr_ExisteEstructuraST(set_of_on_vertices, csmvertex_t, vertex, struct csmvertex_t, i_equals_vertices, NULL) == FALSO)
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
        enum csmmath_double_relation_t cl_resp_plane_pos, cl_resp_plane_neg;
        
        csmhashtb_next_pair(edge_iterator, NULL, &edge, csmedge_t);
        
        hedge_pos = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_POS);
        i_classify_hedge_respect_to_plane(hedge_pos, A, B, C, D, &vertex_pos, &dist_to_plane_pos, &cl_resp_plane_pos);
        
        hedge_neg = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_NEG);
        i_classify_hedge_respect_to_plane(hedge_neg, A, B, C, D, &vertex_neg, &dist_to_plane_neg, &cl_resp_plane_neg);
        
        if ((cl_resp_plane_pos == CSMMATH_VALUE1_LESS_THAN_VALUE2 && cl_resp_plane_neg == CSMMATH_VALUE1_GREATER_THAN_VALUE2)
                || (cl_resp_plane_pos == CSMMATH_VALUE1_GREATER_THAN_VALUE2 && cl_resp_plane_neg == CSMMATH_VALUE1_LESS_THAN_VALUE2))
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
            if (cl_resp_plane_pos == CSMMATH_EQUAL_VALUES)
                i_append_vertex_if_not_exists(vertex_pos, set_of_on_vertices);
            
            if (cl_resp_plane_neg == CSMMATH_EQUAL_VALUES)
                i_append_vertex_if_not_exists(vertex_neg, set_of_on_vertices);
        }
    }
    
    csmhashtb_free_iterator(&edge_iterator, csmedge_t);
    
    return set_of_on_vertices;
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
    ArrEstructura(i_neighborhood_t) *vertexs_neighboorhoods;
    
    work_solid = csmsolid_duplicate(solid);
    
    set_of_on_vertices = i_split_edges_by_plane(work_solid, A, B, C, D);


    *there_is_solid_above = there_is_solid_above_loc;
    *solid_above = solid_above_loc;
    
    *there_is_solid_below = there_is_solid_below_loc;
    *solid_below = solid_below;
    
    arr_DestruyeEstructurasST(&set_of_on_vertices, NULL, csmvertex_t);
    arr_DestruyeEstructurasST(&vertexs_neighboorhoods, i_free_neighborhood, i_neighborhood_t);
}
