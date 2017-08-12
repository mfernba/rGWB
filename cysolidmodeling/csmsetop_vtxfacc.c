// Vertex - face classifier...

#include "csmsetop_vtxfacc.inl"

#include "csmdebug.inl"
#include "csmedge.inl"
#include "csmedge.tli"
#include "csmface.inl"
#include "csmhashtb.inl"
#include "csmhedge.inl"
#include "csmeuler_lmev.inl"
#include "csmeuler_lkemr.inl"
#include "csmloop.inl"
#include "csmmath.inl"
#include "csmopbas.inl"
#include "csmsetopcom.inl"
#include "csmsetop.tli"
#include "csmsolid.tli"
#include "csmtolerance.inl"
#include "csmvertex.inl"
#include "csmvertex.tli"

#include "a_punter.h"
#include "copiafor.h"
#include "cyassert.h"
#include "cypespy.h"
#include "defmath.tlh"

ArrEstructura(csmvertex_t);
ArrEstructura(csmedge_t);
ArrEstructura(csmhedge_t);
ArrEstructura(csmface_t);

struct csmsetop_vtxfacc_inters_t
{
    struct csmvertex_t *vertex;
    struct csmface_t *face;
};

struct i_neighborhood_t
{
    struct csmhedge_t *hedge;
    enum csmsetop_classify_resp_solid_t position;
};

// ------------------------------------------------------------------------------------------

struct csmsetop_vtxfacc_inters_t *csmsetop_vtxfacc_create_inters(struct csmvertex_t *vertex, struct csmface_t *face)
{
    struct csmsetop_vtxfacc_inters_t *vf_inters;
    
    vf_inters = MALLOC(struct csmsetop_vtxfacc_inters_t);
    
    vf_inters->vertex = vertex;
    vf_inters->face = face;
    
    return vf_inters;
}

// ------------------------------------------------------------------------------------------

void csmsetop_vtxfacc_free_inters(struct csmsetop_vtxfacc_inters_t **vf_inters)
{
    assert_no_null(vf_inters);
    assert_no_null(*vf_inters);
    
    FREE_PP(vf_inters, struct csmsetop_vtxfacc_inters_t);
}

// ----------------------------------------------------------------------------------------------------

CYBOOL csmsetop_vtxfacc_equals(const struct csmsetop_vtxfacc_inters_t *vf_inters1, const struct csmsetop_vtxfacc_inters_t *vf_inters2)
{
    assert_no_null(vf_inters1);
    assert_no_null(vf_inters2);
    
    if (vf_inters1->vertex == vf_inters2->vertex)
    {
        if (vf_inters1->face == vf_inters2->face)
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

CONSTRUCTOR(static struct i_neighborhood_t *, i_create_neighborhod, (struct csmhedge_t *hedge, enum csmsetop_classify_resp_solid_t position))
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
                        double *dist_to_plane_opc, enum csmsetop_classify_resp_solid_t *cl_resp_plane_opc)
{
    double dist_to_plane_loc;
    enum csmsetop_classify_resp_solid_t cl_resp_plane_loc;
    
    dist_to_plane_loc = csmmath_signed_distance_point_to_plane(x, y, z, A, B, C, D);
    cl_resp_plane_loc = csmsetopcom_classify_value_respect_to_plane(dist_to_plane_loc, tolerance);

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
                        enum csmsetop_classify_resp_solid_t *cl_resp_plane_opc)
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

static enum csmsetop_classify_resp_solid_t i_classify_hedge_bisector_respect_to_plane(
                        struct csmhedge_t *hedge,
                        double A, double B, double C, double D)
{
    enum csmsetop_classify_resp_solid_t cl_resp_plane;
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
        enum csmsetop_classify_resp_solid_t cl_resp_plane;
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
                        enum csmsetop_operation_t set_operation, enum csmsetop_a_vs_b_t a_vs_b,
                        double tolerance_coplanarity)
{
    struct csmhedge_t *mate_hedge_neighborhood;
    struct csmface_t *face;
    CYBOOL same_orientation;
    
    assert_no_null(hedge_neighborhood);
    assert_no_null(next_hedge_neighborhood);
    
    mate_hedge_neighborhood = csmopbas_mate(hedge_neighborhood->hedge);
    face = csmopbas_face_from_hedge(mate_hedge_neighborhood);
    
    if (csmface_is_coplanar_to_plane(face, A, B, C, D, tolerance_coplanarity, &same_orientation) == CIERTO)
    {
        if (same_orientation == CIERTO)
        {
            switch (a_vs_b)
            {
                case CSMSETOP_A_VS_B:
                    
                    switch (set_operation)
                    {
                        case CSMSETOP_OPERATION_UNION:

                            hedge_neighborhood->position = CSMSETOP_CLASSIFY_RESP_SOLID_OUT;
                            next_hedge_neighborhood->position = CSMSETOP_CLASSIFY_RESP_SOLID_OUT;
                            break;
                            
                        case CSMSETOP_OPERATION_INTERSECTION:
                        case CSMSETOP_OPERATION_DIFFERENCE:
                            
                            hedge_neighborhood->position = CSMSETOP_CLASSIFY_RESP_SOLID_IN;
                            next_hedge_neighborhood->position = CSMSETOP_CLASSIFY_RESP_SOLID_IN;
                            break;
                            
                        default_error();
                    }
                    break;
                    
                case CSMSETOP_B_VS_A:
                    
                    switch (set_operation)
                    {
                        case CSMSETOP_OPERATION_UNION:

                            hedge_neighborhood->position = CSMSETOP_CLASSIFY_RESP_SOLID_IN;
                            next_hedge_neighborhood->position = CSMSETOP_CLASSIFY_RESP_SOLID_IN;
                            break;
                            
                        case CSMSETOP_OPERATION_INTERSECTION:
                        case CSMSETOP_OPERATION_DIFFERENCE:
                            
                            hedge_neighborhood->position = CSMSETOP_CLASSIFY_RESP_SOLID_OUT;
                            next_hedge_neighborhood->position = CSMSETOP_CLASSIFY_RESP_SOLID_OUT;
                            break;
                            
                        default_error();
                    }
                    break;
                    
                default_error();
            }
        }
        else
        {
            switch (a_vs_b)
            {
                case CSMSETOP_A_VS_B:
                case CSMSETOP_B_VS_A:
                    
                    switch (set_operation)
                    {
                        case CSMSETOP_OPERATION_UNION:

                            hedge_neighborhood->position = CSMSETOP_CLASSIFY_RESP_SOLID_IN;
                            next_hedge_neighborhood->position = CSMSETOP_CLASSIFY_RESP_SOLID_IN;
                            break;
                            
                        case CSMSETOP_OPERATION_INTERSECTION:
                        case CSMSETOP_OPERATION_DIFFERENCE:
                            
                            hedge_neighborhood->position = CSMSETOP_CLASSIFY_RESP_SOLID_OUT;
                            next_hedge_neighborhood->position = CSMSETOP_CLASSIFY_RESP_SOLID_OUT;
                            break;
                            
                        default_error();
                    }
                    break;
                    
                default_error();
            }
        }
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_reclassify_on_sectors_vertex_neighborhood(
                        double A, double B, double C, double D,
                        enum csmsetop_operation_t set_operation, enum csmsetop_a_vs_b_t a_vs_b,
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
                        set_operation, a_vs_b,
                        tolerance_coplanarity);
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_reclassify_on_edge_vertex_neighborhood(
                        struct i_neighborhood_t *hedge_neighborhood,
                        enum csmsetop_operation_t set_operation,
                        struct i_neighborhood_t *prev_hedge_neighborhood,
                        struct i_neighborhood_t *next_hedge_neighborhood)
{
    assert_no_null(hedge_neighborhood);
    assert_no_null(prev_hedge_neighborhood);
    assert_no_null(next_hedge_neighborhood);
    
    if (hedge_neighborhood->position == CSMSETOP_CLASSIFY_RESP_SOLID_ON)
    {
        enum csmsetop_classify_resp_solid_t new_position;
        
        if (prev_hedge_neighborhood->position == CSMSETOP_CLASSIFY_RESP_SOLID_IN && next_hedge_neighborhood->position == CSMSETOP_CLASSIFY_RESP_SOLID_IN)
        {
            new_position = CSMSETOP_CLASSIFY_RESP_SOLID_IN;
        }
        else if (prev_hedge_neighborhood->position == CSMSETOP_CLASSIFY_RESP_SOLID_IN && next_hedge_neighborhood->position == CSMSETOP_CLASSIFY_RESP_SOLID_OUT)
        {
            new_position = (set_operation == CSMSETOP_OPERATION_UNION) ? CSMSETOP_CLASSIFY_RESP_SOLID_OUT: CSMSETOP_CLASSIFY_RESP_SOLID_IN;
        }
        else if (prev_hedge_neighborhood->position == CSMSETOP_CLASSIFY_RESP_SOLID_OUT && next_hedge_neighborhood->position == CSMSETOP_CLASSIFY_RESP_SOLID_IN)
        {
            new_position = (set_operation == CSMSETOP_OPERATION_UNION) ? CSMSETOP_CLASSIFY_RESP_SOLID_OUT: CSMSETOP_CLASSIFY_RESP_SOLID_IN;
        }
        else
        {
            assert(prev_hedge_neighborhood->position == CSMSETOP_CLASSIFY_RESP_SOLID_OUT && next_hedge_neighborhood->position == CSMSETOP_CLASSIFY_RESP_SOLID_OUT);
            new_position = CSMSETOP_CLASSIFY_RESP_SOLID_OUT;
        }
        
        hedge_neighborhood->position = new_position;
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_reclassify_on_edges_vertex_neighborhood(
                        enum csmsetop_operation_t set_operation,
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
                        set_operation,
                        prev_hedge_neighborhood,
                        next_hedge_neighborhood);
    }
}

// ----------------------------------------------------------------------------------------------------

static enum csmsetop_classify_resp_solid_t i_sector_position(const ArrEstructura(i_neighborhood_t) *vertex_neighborhood, unsigned long idx)
{
    const struct i_neighborhood_t *hedge_neighborhood;
    
    hedge_neighborhood = arr_GetPunteroST(vertex_neighborhood, idx, i_neighborhood_t);
    assert_no_null(hedge_neighborhood);
    
    return hedge_neighborhood->position;    
}

// ----------------------------------------------------------------------------------------------------

static CYBOOL i_is_in_out_sequence_at_index(const ArrEstructura(i_neighborhood_t) *vertex_neighborhood, unsigned long idx)
{
    unsigned long num_sectors;
    
    num_sectors = arr_NumElemsPunteroST(vertex_neighborhood, i_neighborhood_t);
    assert(num_sectors > 0);
    
    if (i_sector_position(vertex_neighborhood, idx) == CSMSETOP_CLASSIFY_RESP_SOLID_IN)
    {
        if (i_sector_position(vertex_neighborhood, (idx + 1) % num_sectors) == CSMSETOP_CLASSIFY_RESP_SOLID_OUT)
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
        if (i_is_in_out_sequence_at_index(vertex_neighborhood, i) == CIERTO)
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

static CYBOOL i_is_out_in_sequence_at_index(const ArrEstructura(i_neighborhood_t) *vertex_neighborhood, unsigned long idx)
{
    unsigned long num_sectors;
    
    num_sectors = arr_NumElemsPunteroST(vertex_neighborhood, i_neighborhood_t);
    assert(num_sectors > 0);
    
    if (i_sector_position(vertex_neighborhood, idx) == CSMSETOP_CLASSIFY_RESP_SOLID_OUT)
    {
        if (i_sector_position(vertex_neighborhood, (idx + 1) % num_sectors) == CSMSETOP_CLASSIFY_RESP_SOLID_IN)
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
            
            csmdebug_print_debug_info("(he = %lu, cl: %d): ", csmhedge_id(hedge_neighborhood->hedge), hedge_neighborhood->position);
        }
        
        csmdebug_print_debug_info("\n");
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_mark_null_edge_on_face(
                        struct csmface_t *face,
                        double x_split, double y_split, double z_split, csmvertex_mask_t vertex_algorithm_mask,
                        ArrEstructura(csmedge_t) *set_of_null_edges_other_solid)
{
    struct csmloop_t *flout;
    struct csmhedge_t *flout_fledge, *old_flout_fledge_prev, *old_flout_fledge_next;
    struct csmvertex_t *new_vertex, *splitted_vertex;
    struct csmhedge_t *hedge_from_new_vertex, *hedge_to_new_vertex;
    struct csmhedge_t *he_on_new_ring;
    struct csmedge_t *null_edge;
    
    flout = csmface_flout(face);
    flout_fledge = csmloop_ledge(flout);
    old_flout_fledge_prev = csmhedge_prev(flout_fledge);
    old_flout_fledge_next = csmhedge_next(flout_fledge);
    assert(old_flout_fledge_prev != old_flout_fledge_next);
    assert(flout_fledge != old_flout_fledge_prev);
    assert(flout_fledge != old_flout_fledge_next);
    
    csmeuler_lmev(flout_fledge, flout_fledge, x_split, y_split, z_split, &new_vertex, NULL, &hedge_from_new_vertex, &hedge_to_new_vertex);
    csmvertex_set_mask_attrib(new_vertex, vertex_algorithm_mask);
    csmvertex_set_mask_attrib(new_vertex, CSMVERTEX_MASK_SETOP_VTX_FAC_CLASS);

    csmeuler_lmev(hedge_from_new_vertex, hedge_from_new_vertex, x_split, y_split, z_split, &splitted_vertex, &null_edge, NULL, NULL);
    csmvertex_set_mask_attrib(splitted_vertex, vertex_algorithm_mask);
    csmvertex_set_mask_attrib(splitted_vertex, CSMVERTEX_MASK_SETOP_VTX_FAC_CLASS);
    
    arr_AppendPunteroST(set_of_null_edges_other_solid, null_edge, csmedge_t);
    
    if (csmdebug_debug_enabled() == CIERTO)
        csmdebug_print_debug_info("Companion edge %lu on other solid\n", csmedge_id(null_edge));
    
    csmeuler_lkemr(&hedge_to_new_vertex, &hedge_from_new_vertex, NULL, &he_on_new_ring);
    
    assert(csmhedge_loop(he_on_new_ring) != flout);
    assert(old_flout_fledge_prev == csmhedge_prev(flout_fledge));
    assert(old_flout_fledge_next == csmhedge_next(flout_fledge));
}

// ----------------------------------------------------------------------------------------------------

static void i_process_vf_inters(
                        const struct csmsetop_vtxfacc_inters_t *vf_inters,
                        enum csmsetop_operation_t set_operation, enum csmsetop_a_vs_b_t a_vs_b,
                        ArrEstructura(csmedge_t) *set_of_null_edges, ArrEstructura(csmedge_t) *set_of_null_edges_other_solid)
{
    double A, B, C, D;
    ArrEstructura(i_neighborhood_t) *vertex_neighborhood;
    unsigned long start_idx;
    csmvertex_mask_t vertex_algorithm_mask;
    
    assert_no_null(vf_inters);
    
    csmface_face_equation(vf_inters->face, &A, &B, &C, &D);
    csmdebug_set_plane(A, B, C, D);
    
    vertex_algorithm_mask = csmvertex_get_mask_attrib(vf_inters->vertex);
    
    vertex_neighborhood = i_initial_vertex_neighborhood(vf_inters->vertex, A, B, C, D);
    i_print_debug_info_vertex_neighborhood("Initial", vf_inters->vertex, vertex_neighborhood);
    
    i_reclassify_on_sectors_vertex_neighborhood(A, B, C, D, set_operation, a_vs_b, vertex_neighborhood);
    i_print_debug_info_vertex_neighborhood("After Reclassify On Sectors", vf_inters->vertex, vertex_neighborhood);
    
    i_reclassify_on_edges_vertex_neighborhood(set_operation, A, B, C, D, vertex_neighborhood);
    i_print_debug_info_vertex_neighborhood("After Reclassify On Edges", vf_inters->vertex, vertex_neighborhood);
    
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
        
        {
            struct i_neighborhood_t *aux_neighborhood;
            struct csmhedge_t *mate;
            
            aux_neighborhood = arr_GetPunteroST(vertex_neighborhood, idx, i_neighborhood_t);
            assert_no_null(aux_neighborhood);
            
            mate = csmhedge_next(csmopbas_mate(aux_neighborhood->hedge));
            assert(mate == head_neighborhood->hedge);
        }
        
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
            
            while (i_is_out_in_sequence_at_index(vertex_neighborhood, idx) == FALSO)
            {
                assert(num_iters < 100000);
                num_iters++;
                
                idx = (idx + 1) % num_sectors;
            }
            
            tail_neighborhood = arr_GetPunteroST(vertex_neighborhood, (idx + 1) % num_sectors, i_neighborhood_t);
            assert_no_null(tail_neighborhood);
            
            {
                struct i_neighborhood_t *aux_neighborhood;
                struct csmhedge_t *mate;
                
                aux_neighborhood = arr_GetPunteroST(vertex_neighborhood, idx, i_neighborhood_t);
                assert_no_null(aux_neighborhood);
                
                mate = csmhedge_next(csmopbas_mate(aux_neighborhood->hedge));
                assert(mate == tail_neighborhood->hedge);
            }
            
            csmvertex_get_coordenadas(csmhedge_vertex(head_neighborhood->hedge), &x_split, &y_split, &z_split);
            
            csmeuler_lmev(head_neighborhood->hedge, tail_neighborhood->hedge, x_split, y_split, z_split, &split_vertex, &null_edge, NULL, NULL);
            csmvertex_set_mask_attrib(split_vertex, vertex_algorithm_mask);
            arr_AppendPunteroST(set_of_null_edges, null_edge, csmedge_t);
            
            if (csmdebug_debug_enabled() == CIERTO)
            {
                char *description;
                
                csmdebug_print_debug_info(
                        "Inserting null edge %lu at (%g, %g, %g) from hedge %lu to hedge %lu.\n",
                        csmedge_id(null_edge),
                        x_split, y_split, z_split,
                        csmhedge_id(head_neighborhood->hedge),
                        csmhedge_id(tail_neighborhood->hedge));
                
                description = copiafor_codigo3("NE (%g, %g, %g)", x_split, y_split, z_split);
                csmdebug_append_debug_point(x_split, y_split, z_split, &description);
            }
            
            i_mark_null_edge_on_face(
                        vf_inters->face,
                        x_split, y_split, z_split, vertex_algorithm_mask,
                        set_of_null_edges_other_solid);
            
            while (i_is_in_out_sequence_at_index(vertex_neighborhood, idx) == FALSO)
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
    
    //csmdebug_show_viewer();
}

// ------------------------------------------------------------------------------------------

void csmsetop_vtxfacc_append_null_edges(
                        const ArrEstructura(csmsetop_vtxfacc_inters_t) *vf_intersections,
                        enum csmsetop_operation_t set_operation, enum csmsetop_a_vs_b_t a_vs_b,
                        ArrEstructura(csmedge_t) *set_of_null_edges,
                        ArrEstructura(csmedge_t) *set_of_null_edges_other_solid)
{
    unsigned long i, num_intersections;
    
    num_intersections = arr_NumElemsPunteroST(vf_intersections, csmsetop_vtxfacc_inters_t);
    
    for (i = 0; i < num_intersections; i++)
    {
        csmdebug_begin_context("VF INTERS");
        {
            const struct csmsetop_vtxfacc_inters_t *vf_inters;
            
            vf_inters = arr_GetPunteroConstST(vf_intersections, i, csmsetop_vtxfacc_inters_t);
            i_process_vf_inters(vf_inters, set_operation, a_vs_b, set_of_null_edges, set_of_null_edges_other_solid);
        }
        csmdebug_end_context();
    }
}
