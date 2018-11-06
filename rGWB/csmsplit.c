// Split operation...

#include "csmsplit.h"
#include "csmsplit.hxx"

#include "csmarrayc.h"
#include "csmdebug.inl"
#include "csmedge.inl"
#include "csmedge.tli"
#include "csmeuler_lmev.inl"
#include "csmface.inl"
#include "csmhashtb.inl"
#include "csmhedge.inl"
#include "csmmaterial.inl"
#include "csmmath.inl"
#include "csmmath.tli"
#include "csmopbas.inl"
#include "csmsetopcom.inl"
#include "csmsimplifysolid.inl"
#include "csmsolid.h"
#include "csmsolid.inl"
#include "csmstring.inl"
#include "csmtolerance.inl"
#include "csmvertex.inl"
#include "csmvertex.tli"

#ifdef RGWB_STANDALONE_DISTRIBUTABLE
#include "csmassert.inl"
#include "csmmem.inl"
#else
#include "cyassert.h"
#include "cypespy.h"
#include "copiafor.h"
#endif

enum i_position_t
{
    i_POSITION_ABOVE,
    i_POSITION_ON,
    i_POSITION_BELOW
};

struct i_neighborhood_t
{
    struct csmhedge_t *hedge;
    
    enum i_position_t prev_position;
    enum i_position_t recl_position;
};

static const unsigned long i_NUM_MAX_PERTURBATIONS = 10;

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_neighborhood_t *, i_create_neighborhod, (
                        struct csmhedge_t *hedge,
                        enum i_position_t prev_position, enum i_position_t recl_position))
{
    struct i_neighborhood_t *neighborhood;
    
    neighborhood = MALLOC(struct i_neighborhood_t);
    
    neighborhood->hedge = hedge;
    
    neighborhood->prev_position = prev_position;
    neighborhood->recl_position = recl_position;
    
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
        case CSMCOMPARE_FIRST_LESS:
            
            cl_resp_plane_loc = i_POSITION_BELOW;
            break;
            
        case CSMCOMPARE_EQUAL:
            
            cl_resp_plane_loc = i_POSITION_ON;
            break;
            
        case CSMCOMPARE_FIRST_GREATER:
            
            cl_resp_plane_loc = i_POSITION_ABOVE;
            break;
            
        default_error();
    }

    ASSIGN_OPTIONAL_VALUE(dist_to_plane_opc, dist_to_plane_loc);
    ASSIGN_OPTIONAL_VALUE(cl_resp_plane_opc, cl_resp_plane_loc);
    
}

// ----------------------------------------------------------------------------------------------------

static void i_classify_hedge_respect_to_plane(
                        struct csmhedge_t *hedge,
                        double A, double B, double C, double D,
                        const struct csmtolerance_t *tolerances,
                        struct csmvertex_t **vertex_opc, double *dist_to_plane_opc,
                        enum i_position_t *cl_resp_plane_opc)
{
    double tolerance;
    struct csmvertex_t *vertex_loc;
    double x_loc, y_loc, z_loc;
    
    tolerance = csmtolerance_point_on_plane(tolerances);

    vertex_loc = csmhedge_vertex(hedge);
    csmvertex_get_coords(vertex_loc, &x_loc, &y_loc, &z_loc);
    
    i_classify_point_respect_to_plane(
                        x_loc, y_loc, z_loc,
                        A, B, C, D,
                        tolerance,
                        dist_to_plane_opc, cl_resp_plane_opc);

    ASSIGN_OPTIONAL_VALUE(vertex_opc, vertex_loc);
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static csmArrayStruct(csmvertex_t) *, i_split_edges_by_plane, (
                        struct csmsolid_t *work_solid,
                        double A, double B, double C, double D,
                        const struct csmtolerance_t *tolerances))
{
    csmArrayStruct(csmvertex_t) *set_of_on_vertices;
    struct csmhashtb_iterator(csmedge_t) *edge_iterator;
    
    assert_no_null(work_solid);

    csmdebug_begin_context("Split edges by plane");

    edge_iterator = csmsolid_edge_iterator(work_solid);
    set_of_on_vertices = csmarrayc_new_st_array(0, csmvertex_t);
    
    while (csmhashtb_has_next(edge_iterator, csmedge_t) == CSMTRUE)
    {
        struct csmedge_t *edge;
        struct csmhedge_t *hedge_pos, *hedge_neg;
        struct csmvertex_t *vertex_pos, *vertex_neg;
        double dist_to_plane_pos, dist_to_plane_neg;
        enum i_position_t cl_resp_plane_pos, cl_resp_plane_neg;
        csmvertex_mask_t new_vertex_algorithm_attrib_mask;
        
        csmhashtb_next_pair(edge_iterator, NULL, &edge, csmedge_t);
        
        hedge_pos = csmedge_hedge_lado(edge, CSMEDGE_HEDGE_SIDE_POS);
        i_classify_hedge_respect_to_plane(hedge_pos, A, B, C, D, tolerances, &vertex_pos, &dist_to_plane_pos, &cl_resp_plane_pos);
        
        hedge_neg = csmedge_hedge_lado(edge, CSMEDGE_HEDGE_SIDE_NEG);
        i_classify_hedge_respect_to_plane(hedge_neg, A, B, C, D, tolerances, &vertex_neg, &dist_to_plane_neg, &cl_resp_plane_neg);
        
        if (csmsetopcom_is_edge_on_cycle_of_edges_with_a_side_on_a_hole(edge) == CSMTRUE)
            new_vertex_algorithm_attrib_mask = CSMVERTEX_MASK_VERTEX_ON_HOLE_LOOP;
        else
            new_vertex_algorithm_attrib_mask = CSMVERTEX_NULL_MASK;

        if (csmdebug_debug_enabled() == CSMTRUE)
        {
            double x_pos, y_pos, z_pos, x_neg, y_neg, z_neg;

            csmvertex_get_coords(vertex_pos, &x_pos, &y_pos, &z_pos);
            csmvertex_get_coords(vertex_neg, &x_neg, &y_neg, &z_neg);
            
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
            
            csmvertex_get_coords(vertex_pos, &x_pos, &y_pos, &z_pos);
            csmvertex_get_coords(vertex_neg, &x_neg, &y_neg, &z_neg);
            
            x_inters = x_pos + t * (x_neg - x_pos);
            y_inters = y_pos + t * (y_neg - y_pos);
            z_inters = z_pos + t * (z_neg - z_pos);
            
            hedge_neg_next = csmhedge_next(hedge_neg);
            assert(vertex_pos == csmhedge_vertex(hedge_neg_next));
            
            csmeuler_lmev(hedge_pos, hedge_neg_next, x_inters, y_inters, z_inters, &new_vertex, NULL, NULL, NULL);
            csmarrayc_append_element_st(set_of_on_vertices, new_vertex, csmvertex_t);
            
            csmvertex_set_mask_attrib(new_vertex, new_vertex_algorithm_attrib_mask);
            
            if (csmdebug_debug_enabled() == CSMTRUE)
                csmdebug_print_debug_info("Intersection at (%g, %g, %g), New Vertex Id: %lu\n", x_inters, y_inters, z_inters, csmvertex_id(new_vertex));
        }
        else
        {
            if (cl_resp_plane_pos == i_POSITION_ON)
            {
                csmvertex_set_mask_attrib(vertex_pos, new_vertex_algorithm_attrib_mask);
                csmsetopcom_append_vertex_if_not_exists(vertex_pos, set_of_on_vertices);
                
                if (csmdebug_debug_enabled() == CSMTRUE)
                    csmdebug_print_debug_info("Already On Vertex %lu\n", csmvertex_id(vertex_pos));
            }
            
            if (cl_resp_plane_neg == i_POSITION_ON)
            {
                csmvertex_set_mask_attrib(vertex_neg, new_vertex_algorithm_attrib_mask);
                csmsetopcom_append_vertex_if_not_exists(vertex_neg, set_of_on_vertices);
                
                if (csmdebug_debug_enabled() == CSMTRUE)
                    csmdebug_print_debug_info("Already On Vertex %lu\n", csmvertex_id(vertex_neg));
            }
        }
    }
    
    csmhashtb_free_iterator(&edge_iterator, csmedge_t);
    
    csmdebug_end_context();
    
    return set_of_on_vertices;
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static csmArrayStruct(i_neighborhood_t) *, i_initial_vertex_neighborhood, (
                        struct csmvertex_t *vertex,
                        double A, double B, double C, double D,
                        const struct csmtolerance_t *tolerances))
{
    csmArrayStruct(i_neighborhood_t) *vertex_neighborhood;
    double x_vertex, y_vertex, z_vertex;
    struct csmhedge_t *hedge_iterator, *vertex_hedge;
    unsigned long num_iters;
    
    vertex_neighborhood = csmarrayc_new_st_array(0, i_neighborhood_t);
    
    csmvertex_get_coords(vertex, &x_vertex, &y_vertex, &z_vertex);
    vertex_hedge = csmvertex_hedge(vertex);
    hedge_iterator = vertex_hedge;
    
    num_iters = 0;
    
    do
    {
        struct csmhedge_t *hedge_next;
        enum i_position_t cl_resp_plane;
        struct i_neighborhood_t *hedge_neighborhood;
        double Ux_bisec, Uy_bisec, Uz_bisec;
        
        assert(num_iters < 10000);
        num_iters++;
        
        hedge_next = csmhedge_next(hedge_iterator);
        i_classify_hedge_respect_to_plane(hedge_next, A, B, C, D, tolerances, NULL, NULL, &cl_resp_plane);
        
        hedge_neighborhood = i_create_neighborhod(hedge_iterator, cl_resp_plane, cl_resp_plane);
        csmarrayc_append_element_st(vertex_neighborhood, hedge_neighborhood, i_neighborhood_t);
        
        if (csmopbas_is_wide_hedge(hedge_iterator, tolerances, &Ux_bisec, &Uy_bisec, &Uz_bisec) == CSMTRUE)
        {
            struct i_neighborhood_t *hedge_neighborhood_wide;
            double tolerance;
            
            hedge_neighborhood_wide = i_create_neighborhod(hedge_iterator, hedge_neighborhood->prev_position, hedge_neighborhood->recl_position);
            csmarrayc_append_element_st(vertex_neighborhood, hedge_neighborhood_wide, i_neighborhood_t);
            
            tolerance = csmtolerance_point_on_plane(tolerances);
            i_classify_point_respect_to_plane(x_vertex + Ux_bisec, y_vertex + Uy_bisec, z_vertex + Uz_bisec, A, B, C, D, tolerance, NULL, &cl_resp_plane);
            hedge_neighborhood->prev_position = cl_resp_plane;
            hedge_neighborhood->recl_position = cl_resp_plane;
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
                        double A, double B, double C,
                        const struct csmtolerance_t *tolerances)
{
    assert_no_null(hedge_neighborhood);
    assert_no_null(next_hedge_neighborhood);

    if (hedge_neighborhood->prev_position == i_POSITION_ON && next_hedge_neighborhood->prev_position == i_POSITION_ON)
    {
        struct csmface_t *common_face;
        
        common_face = csmsetopcom_face_for_hedge_sector(hedge_neighborhood->hedge, next_hedge_neighborhood->hedge);
    
        if (csmface_is_oriented_in_direction(common_face, A, B, C) == CSMTRUE)
        {
            hedge_neighborhood->recl_position = i_POSITION_BELOW;
            next_hedge_neighborhood->recl_position = i_POSITION_BELOW;
        }
        else
        {
            hedge_neighborhood->recl_position = i_POSITION_ABOVE;
            next_hedge_neighborhood->recl_position = i_POSITION_ABOVE;
        }
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_update_vertex_neighborhood_prev_position_after_reclassification(csmArrayStruct(i_neighborhood_t) *vertex_neighborhood)
{
    unsigned long i, num_sectors;
    
    num_sectors = csmarrayc_count_st(vertex_neighborhood, i_neighborhood_t);
    
    for (i = 0; i < num_sectors; i++)
    {
        struct i_neighborhood_t *hedge_neighborhood;
        
        hedge_neighborhood = csmarrayc_get_st(vertex_neighborhood, i, i_neighborhood_t);
        assert_no_null(hedge_neighborhood);
        
        hedge_neighborhood->prev_position = hedge_neighborhood->recl_position;
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_reclassify_on_sectors_vertex_neighborhood(
                        double A, double B, double C,
                        const struct csmtolerance_t *tolerances,
                        csmArrayStruct(i_neighborhood_t) *vertex_neighborhood)
{
    unsigned long i, num_sectors;
    
    num_sectors = csmarrayc_count_st(vertex_neighborhood, i_neighborhood_t);
    
    for (i = 0; i < num_sectors; i++)
    {
        struct i_neighborhood_t *hedge_neighborhood;
        unsigned long next_idx;
        struct i_neighborhood_t *next_hedge_neighborhood;
        
        hedge_neighborhood = csmarrayc_get_st(vertex_neighborhood, i, i_neighborhood_t);
        
        next_idx = (i + 1) % num_sectors;
        next_hedge_neighborhood = csmarrayc_get_st(vertex_neighborhood, next_idx, i_neighborhood_t);
        
        i_reclassify_on_sector_vertex_neighborhood(
                        hedge_neighborhood,
                        next_hedge_neighborhood,
                        A, B, C,
                        tolerances);
    }
    
    i_update_vertex_neighborhood_prev_position_after_reclassification(vertex_neighborhood);
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
    
    if (hedge_neighborhood->prev_position == i_POSITION_ON)
    {
        enum i_position_t new_position;

        if (prev_hedge_neighborhood->prev_position == i_POSITION_ABOVE && next_hedge_neighborhood->prev_position == i_POSITION_ABOVE)
        {
            new_position = i_POSITION_BELOW;
        }
        else if (prev_hedge_neighborhood->prev_position == i_POSITION_ABOVE && next_hedge_neighborhood->prev_position == i_POSITION_BELOW)
        {
            new_position = i_POSITION_BELOW;
        }
        else if (prev_hedge_neighborhood->prev_position == i_POSITION_BELOW && next_hedge_neighborhood->prev_position == i_POSITION_BELOW)
        {
            new_position = i_POSITION_ABOVE;
        }
        else
        {
            assert(prev_hedge_neighborhood->prev_position == i_POSITION_BELOW && next_hedge_neighborhood->prev_position == i_POSITION_ABOVE);
            new_position = i_POSITION_BELOW;
        }
        
        hedge_neighborhood->recl_position = new_position;
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_reclassify_on_edges_vertex_neighborhood(csmArrayStruct(i_neighborhood_t) *vertex_neighborhood)
{
    unsigned long i, num_sectors;
    
    num_sectors = csmarrayc_count_st(vertex_neighborhood, i_neighborhood_t);
    
    for (i = 0; i < num_sectors; i++)
    {
        struct i_neighborhood_t *hedge_neighborhood;
        unsigned long prev_idx;
        struct i_neighborhood_t *prev_hedge_neighborhood;
        unsigned long next_idx;
        struct i_neighborhood_t *next_hedge_neighborhood;
        
        hedge_neighborhood = csmarrayc_get_st(vertex_neighborhood, i, i_neighborhood_t);
        
        prev_idx = (num_sectors + i - 1) % num_sectors;
        prev_hedge_neighborhood = csmarrayc_get_st(vertex_neighborhood, prev_idx, i_neighborhood_t);
        
        next_idx = (i + 1) % num_sectors;
        next_hedge_neighborhood = csmarrayc_get_st(vertex_neighborhood, next_idx, i_neighborhood_t);
        
        i_reclassify_on_edge_vertex_neighborhood(
                        hedge_neighborhood,
                        prev_hedge_neighborhood,
                        next_hedge_neighborhood);
    }
    
    i_update_vertex_neighborhood_prev_position_after_reclassification(vertex_neighborhood);
}

// ----------------------------------------------------------------------------------------------------

static enum i_position_t i_sector_position(const csmArrayStruct(i_neighborhood_t) *vertex_neighborhood, unsigned long idx)
{
    const struct i_neighborhood_t *hedge_neighborhood;
    
    hedge_neighborhood = csmarrayc_get_st(vertex_neighborhood, idx, i_neighborhood_t);
    assert_no_null(hedge_neighborhood);
    
    return hedge_neighborhood->recl_position;
}

// ----------------------------------------------------------------------------------------------------

static CSMBOOL i_is_below_above_sequence_at_index(const csmArrayStruct(i_neighborhood_t) *vertex_neighborhood, unsigned long idx)
{
    unsigned long num_sectors;
    
    num_sectors = csmarrayc_count_st(vertex_neighborhood, i_neighborhood_t);
    assert(num_sectors > 0);
    
    if (i_sector_position(vertex_neighborhood, idx) == i_POSITION_BELOW)
    {
        if (i_sector_position(vertex_neighborhood, (idx + 1) % num_sectors) == i_POSITION_ABOVE)
            return CSMTRUE;
        else
            return CSMFALSE;
    }
    else
    {
        return CSMFALSE;
    }
}

// ----------------------------------------------------------------------------------------------------

static CSMBOOL i_could_locate_begin_sequence(const csmArrayStruct(i_neighborhood_t) *vertex_neighborhood, unsigned long *start_idx)
{
    CSMBOOL found;
    unsigned long start_idx_loc;
    unsigned long i, num_sectors;
    
    num_sectors = csmarrayc_count_st(vertex_neighborhood, i_neighborhood_t);
    assert(num_sectors > 0);
    assert_no_null(start_idx);
    
    found = CSMFALSE;
    start_idx_loc = ULONG_MAX;
    
    for (i = 0; i < num_sectors; i++)
    {
        if (i_is_below_above_sequence_at_index(vertex_neighborhood, i) == CSMTRUE)
        {
            found = CSMTRUE;
            start_idx_loc = i;
            break;
        }
    }
    
    *start_idx = start_idx_loc;
    
    return found;
}

// ----------------------------------------------------------------------------------------------------

static CSMBOOL i_is_above_below_sequence_at_index(const csmArrayStruct(i_neighborhood_t) *vertex_neighborhood, unsigned long idx)
{
    unsigned long num_sectors;
    
    num_sectors = csmarrayc_count_st(vertex_neighborhood, i_neighborhood_t);
    assert(num_sectors > 0);
    
    if (i_sector_position(vertex_neighborhood, idx) == i_POSITION_ABOVE)
    {
        if (i_sector_position(vertex_neighborhood, (idx + 1) % num_sectors) == i_POSITION_BELOW)
            return CSMTRUE;
        else
            return CSMFALSE;
    }
    else
    {
        return CSMFALSE;
    }
}

// ----------------------------------------------------------------------------------------------------

static const char *i_cl_plane_to_string(enum i_position_t position)
{
    switch (position)
    {
        case i_POSITION_ON:     return "ON";
        case i_POSITION_ABOVE:  return "ABOVE";
        case i_POSITION_BELOW:  return "BELOW";
        default_error();
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_print_debug_info_vertex_neighborhood(
                        const char *description,
                        const struct csmvertex_t *vertex,
                        csmArrayStruct(i_neighborhood_t) *vertex_neighborhood)
{
    if (csmdebug_debug_enabled() == CSMTRUE)
    {
        double x, y, z;
        unsigned long i, num_sectors;
        
        csmvertex_get_coords(vertex, &x, &y, &z);
        csmdebug_print_debug_info("Vertex neighborhood [%s]: %lu (%g, %g, %g):\n", description, csmvertex_id(vertex), x, y, z);
        
        num_sectors = csmarrayc_count_st(vertex_neighborhood, i_neighborhood_t);
        
        for (i = 0; i < num_sectors; i++)
        {
            struct i_neighborhood_t *hedge_neighborhood;
            
            hedge_neighborhood = csmarrayc_get_st(vertex_neighborhood, i, i_neighborhood_t);
            assert_no_null(hedge_neighborhood);
            
            csmdebug_print_debug_info(
                        "\t(hedge = %5lu\trecl_cl: %5s)\n ",
                        csmhedge_id(hedge_neighborhood->hedge),
                        i_cl_plane_to_string(hedge_neighborhood->recl_position));
        }
        
        csmdebug_print_debug_info("\n");
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_insert_nulledges_to_split_solid_at_on_vertex_neihborhood(
                        struct csmvertex_t *vertex,
                        double A, double B, double C, double D,
                        const struct csmtolerance_t *tolerances,
                        csmArrayStruct(csmedge_t) *set_of_null_edges)
{
    csmArrayStruct(i_neighborhood_t) *vertex_neighborhood;
    unsigned long start_idx;
    csmvertex_mask_t vertex_algorithm_mask;
    
    csmdebug_print_debug_info("Vertex: %lu\n", csmvertex_id(vertex));
    
    vertex_algorithm_mask = csmvertex_get_mask_attrib(vertex);
    
    vertex_neighborhood = i_initial_vertex_neighborhood(vertex, A, B, C, D, tolerances);
    i_print_debug_info_vertex_neighborhood("Initial", vertex, vertex_neighborhood);
    
    i_reclassify_on_sectors_vertex_neighborhood(A, B, C, tolerances, vertex_neighborhood);
    i_print_debug_info_vertex_neighborhood("After Reclassify On Sectors", vertex, vertex_neighborhood);
    
    i_reclassify_on_edges_vertex_neighborhood(vertex_neighborhood);
    i_print_debug_info_vertex_neighborhood("After Reclassify On Edges", vertex, vertex_neighborhood);
    
    if (i_could_locate_begin_sequence(vertex_neighborhood, &start_idx) == CSMTRUE)
    {
        unsigned long num_sectors;
        unsigned long idx;
        struct i_neighborhood_t *head_neighborhood;
        unsigned long num_iters;
        CSMBOOL process_next_sequence;

        num_sectors = csmarrayc_count_st(vertex_neighborhood, i_neighborhood_t);
        assert(num_sectors > 0);
        
        idx = start_idx;
        head_neighborhood = csmarrayc_get_st(vertex_neighborhood, (idx + 1) % num_sectors, i_neighborhood_t);
        
        num_iters = 0;
        process_next_sequence = CSMTRUE;
        
        while (process_next_sequence == CSMTRUE)
        {
            struct i_neighborhood_t *tail_neighborhood;
            struct csmhedge_t *head_next;
            enum i_position_t cl_head_next_resp_plane;
            double x_split, y_split, z_split;
            struct csmvertex_t *split_vertex;
            struct csmedge_t *null_edge;
            
            assert_no_null(head_neighborhood);
            assert(num_iters < 100000);
            num_iters++;
            
            while (i_is_above_below_sequence_at_index(vertex_neighborhood, idx) == CSMFALSE)
            {
                assert(num_iters < 100000);
                num_iters++;
                
                idx = (idx + 1) % num_sectors;
            }
            
            tail_neighborhood = csmarrayc_get_st(vertex_neighborhood, (idx + 1) % num_sectors, i_neighborhood_t);
            assert_no_null(tail_neighborhood);
            
            csmvertex_get_coords(csmhedge_vertex(head_neighborhood->hedge), &x_split, &y_split, &z_split);

            csmdebug_print_debug_info(
                        "Inserting null edge at (%g, %g, %g) from hedge %lu to hedge %lu.\n",
                        x_split, y_split, z_split,
                        csmhedge_id(head_neighborhood->hedge),
                        csmhedge_id(tail_neighborhood->hedge));
            
            head_next = csmhedge_next(head_neighborhood->hedge);
            i_classify_hedge_respect_to_plane(head_next, A, B, C, D, tolerances, NULL, NULL, &cl_head_next_resp_plane);
            
            // Null edge oriented from vertex below SP to above. tail is the below hedge.
            if (cl_head_next_resp_plane == i_POSITION_ABOVE || cl_head_next_resp_plane == i_POSITION_ON)
            {
                csmeuler_lmev(tail_neighborhood->hedge, head_neighborhood->hedge, x_split, y_split, z_split, &split_vertex, &null_edge, NULL, NULL);
            }
            else
            {
                assert(cl_head_next_resp_plane == i_POSITION_BELOW);
                csmeuler_lmev(head_neighborhood->hedge, tail_neighborhood->hedge, x_split, y_split, z_split, &split_vertex, &null_edge, NULL, NULL);
            }
            
            csmarrayc_append_element_st(set_of_null_edges, null_edge, csmedge_t);
            csmedge_setop_set_is_null_edge(null_edge, CSMTRUE);
            
            if (csmdebug_debug_enabled() == CSMTRUE)
                csmedge_print_debug_info(null_edge, CSMTRUE);
            
            csmvertex_set_mask_attrib(split_vertex, vertex_algorithm_mask);
            
            while (i_is_below_above_sequence_at_index(vertex_neighborhood, idx) == CSMFALSE)
            {
                assert(num_iters < 100000);
                num_iters++;
                
                idx = (idx + 1) % num_sectors;
                
                if (idx == start_idx)
                {
                    process_next_sequence = CSMFALSE;
                    break;
                }
            }
            
            head_neighborhood = csmarrayc_get_st(vertex_neighborhood, (idx + 1) % num_sectors, i_neighborhood_t);
        }
    }
    
    csmarrayc_free_st(&vertex_neighborhood, i_neighborhood_t, i_free_neighborhood);
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static csmArrayStruct(csmedge_t) *, i_insert_nulledges_to_split_solid, (
                        double A, double B, double C, double D,
                        const struct csmtolerance_t *tolerances,
                        csmArrayStruct(csmvertex_t) *set_of_on_vertices))
{
    csmArrayStruct(csmedge_t) *set_of_null_edges;
    unsigned long i, num_vertices;
    
    csmdebug_begin_context("Insert null edges");

    num_vertices = csmarrayc_count_st(set_of_on_vertices, csmvertex_t);
    
    set_of_null_edges = csmarrayc_new_st_array(0, csmedge_t);
    
    for (i = 0; i < num_vertices; i++)
    {
        struct csmvertex_t *vertex;
        
        vertex = csmarrayc_get_st(set_of_on_vertices, i, csmvertex_t);
        i_insert_nulledges_to_split_solid_at_on_vertex_neihborhood(vertex, A, B, C, D, tolerances, set_of_null_edges);
    }
    
    csmdebug_end_context();
    
    return set_of_null_edges;
}

// ----------------------------------------------------------------------------------------------------

static CSMBOOL i_can_join_he(struct csmhedge_t *he, csmArrayStruct(csmhedge_t) *loose_ends, struct csmhedge_t **matching_loose_end)
{
    CSMBOOL can_join;
    struct csmhedge_t *matching_loose_end_loc;
    unsigned long i, no_loose_ends;
    
    assert_no_null(matching_loose_end);
    
    no_loose_ends = csmarrayc_count_st(loose_ends, csmhedge_t);
    
    can_join = CSMFALSE;
    matching_loose_end_loc = NULL;
    
    for (i = 0; i < no_loose_ends; i++)
    {
        struct csmhedge_t *loose_end;
        
        loose_end = csmarrayc_get_st(loose_ends, i, csmhedge_t);
        
        if (csmsetopcom_hedges_are_neighbors(he, loose_end) == CSMTRUE)
        {
            can_join = CSMTRUE;
            matching_loose_end_loc = loose_end;
            
            csmarrayc_delete_element_st(loose_ends, i, csmhedge_t, NULL);
            break;
        }
    }
    
    if (can_join == CSMFALSE)
    {
        matching_loose_end_loc = NULL;
        csmarrayc_append_element_st(loose_ends, he, csmhedge_t);
    }
    
    *matching_loose_end = matching_loose_end_loc;
    
    return can_join;
}

// ----------------------------------------------------------------------------------------------------

static void i_cut_he(
                    struct csmhedge_t *hedge,
                    csmArrayStruct(csmedge_t) *set_of_null_edges,
                    csmArrayStruct(csmface_t) *set_of_null_faces,
                    unsigned long *no_null_edges_deleted)
{
    CSMBOOL null_face_created_loc;
    
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
}

// ----------------------------------------------------------------------------------------------------

static void i_join_null_edges(
                    csmArrayStruct(csmedge_t) *set_of_null_edges,
                    const struct csmtolerance_t *tolerances,
                    csmArrayStruct(csmface_t) **set_of_null_faces,
                    CSMBOOL *did_join_all_null_edges)
{
    csmArrayStruct(csmface_t) *set_of_null_faces_loc;
    csmArrayStruct(csmhedge_t) *loose_ends;
    unsigned long i, no_null_edges;
    unsigned long no_null_edges_deleted;
    
    assert_no_null(did_join_all_null_edges);
    
    csmdebug_begin_context("****JOIN NULL EDGES\n");
    
    csmsetopcom_sort_edges_lexicographically_by_xyz(set_of_null_edges, tolerances);
    no_null_edges = csmarrayc_count_st(set_of_null_edges, csmedge_t);
    assert(no_null_edges > 0);
    assert_no_null(set_of_null_faces);
    
    set_of_null_faces_loc = csmarrayc_new_st_array(0, csmface_t);
    loose_ends = csmarrayc_new_st_array(0, csmhedge_t);
    
    no_null_edges_deleted = 0;
    
    for (i = 0; i < no_null_edges; i++)
    {
        unsigned long idx;
        struct csmedge_t *next_edge;
        struct csmhedge_t *he1_next_edge, *he2_next_edge;
        struct csmhedge_t *matching_loose_end_he1, *matching_loose_end_he2;
        
        if (csmdebug_debug_enabled() == CSMTRUE)
            csmsetopcom_print_set_of_null_edges(set_of_null_edges, loose_ends);
        
        idx = i - no_null_edges_deleted;
        next_edge = csmarrayc_get_st(set_of_null_edges, idx, csmedge_t);
        
        he1_next_edge = csmedge_hedge_lado(next_edge, CSMEDGE_HEDGE_SIDE_POS);
        he2_next_edge = csmedge_hedge_lado(next_edge, CSMEDGE_HEDGE_SIDE_NEG);
     
        if (i_can_join_he(he1_next_edge, loose_ends, &matching_loose_end_he1) == CSMTRUE)
        {
            csmsetopcom_join_hedges(matching_loose_end_he1, he1_next_edge, tolerances);
            
            if (csmsetopcom_is_loose_end(csmopbas_mate(matching_loose_end_he1), loose_ends) == CSMFALSE)
                i_cut_he(matching_loose_end_he1, set_of_null_edges, set_of_null_faces_loc, &no_null_edges_deleted);
        }
        else
        {
            matching_loose_end_he1 = NULL;
        }

        if (i_can_join_he(he2_next_edge, loose_ends, &matching_loose_end_he2) == CSMTRUE)
        {
            csmsetopcom_join_hedges(matching_loose_end_he2, he2_next_edge, tolerances);
            
            if (csmsetopcom_is_loose_end(csmopbas_mate(matching_loose_end_he2), loose_ends) == CSMFALSE)
                i_cut_he(matching_loose_end_he2, set_of_null_edges, set_of_null_faces_loc, &no_null_edges_deleted);
        }
        else
        {
            matching_loose_end_he2 = NULL;
        }
        
        if (matching_loose_end_he1 != NULL && matching_loose_end_he2 != NULL)
            i_cut_he(he1_next_edge, set_of_null_edges, set_of_null_faces_loc, &no_null_edges_deleted);
        
        if (csmdebug_debug_enabled() == CSMTRUE)
        {
            csmsetopcom_print_debug_info_loose_ends(loose_ends);
            //csmsolid_print_debug(csmopbas_solid_from_hedge(he1_next_edge), CSMTRUE);
        }
    }
    
    csmdebug_end_context();
    
    *set_of_null_faces = set_of_null_faces_loc;
    *did_join_all_null_edges = IS_TRUE(csmarrayc_count_st(set_of_null_edges, csmedge_t) == 0);
    
    if (csmdebug_get_treat_improper_solid_operations_as_errors() == CSMTRUE)
        assert(csmarrayc_count_st(set_of_null_edges, csmedge_t) == 0);
    
    assert(csmarrayc_count_st(loose_ends, csmhedge_t) == 0);
    csmarrayc_free_st(&loose_ends, csmhedge_t, NULL);
}

// ------------------------------------------------------------------------------------------

static void i_assign_result_material(const struct csmsolid_t *solid, struct csmsolid_t *result)
{
    const struct csmmaterial_t *material;
    
    material = csmsolid_get_material(solid);
    
    if (material != NULL)
    {
        struct csmmaterial_t *material_copy;
        
        material_copy = csmmaterial_copy(material);
        csmsolid_set_visualization_material(result, &material_copy);
    }
}

// ----------------------------------------------------------------------------------------------------

static CSMBOOL i_did_finish_split(
                        csmArrayStruct(csmface_t) *set_of_null_faces,
                        struct csmsolid_t *work_solid,
                        const struct csmtolerance_t *tolerances,
                        struct csmsolid_t **solid_above, struct csmsolid_t **solid_below)
{
    CSMBOOL did_finish;
    unsigned long i, no_null_faces;
    struct csmsolid_t *solid_above_loc, *solid_below_loc;
    csmArrayStruct(csmface_t) *set_of_null_faces_above;
    csmArrayStruct(csmface_t) *set_of_null_faces_below;
    CSMBOOL there_is_improper_intersection_error;

    assert_no_null(solid_above);
    assert_no_null(solid_below);
    
    csmdebug_begin_context("********FINISH SPLIT");

    if (csmdebug_debug_enabled() == CSMTRUE)
        csmsolid_print_debug(work_solid, CSMTRUE);

    set_of_null_faces_above = csmsetopcom_convert_inner_loops_of_null_faces_to_faces(set_of_null_faces);
    set_of_null_faces_below = set_of_null_faces;
    
    csmsolid_redo_geometric_face_data(work_solid);
    csmsetopcom_reintroduce_holes_in_corresponding_faces(set_of_null_faces_above, tolerances);
    csmsetopcom_reintroduce_holes_in_corresponding_faces(set_of_null_faces_below, tolerances);

    no_null_faces = csmarrayc_count_st(set_of_null_faces, csmface_t);
    assert(no_null_faces == csmarrayc_count_st(set_of_null_faces_below, csmface_t));
    assert(no_null_faces > 0);
    
    if (csmdebug_debug_enabled() == CSMTRUE)
        csmsolid_print_debug(work_solid, CSMTRUE);

    solid_above_loc = csmsolid_new_empty_solid(0);
    i_assign_result_material(work_solid, solid_above_loc);
    
    solid_below_loc = csmsolid_new_empty_solid(0);
    i_assign_result_material(work_solid, solid_below_loc);
    
    there_is_improper_intersection_error = CSMFALSE;
    
    for (i = 0; i < no_null_faces; i++)
    {
        struct csmface_t *face_to_solid_above;
        
        face_to_solid_above = csmarrayc_get_st(set_of_null_faces_above, i, csmface_t);
        csmsetopcom_move_face_to_solid(0, face_to_solid_above, work_solid, solid_above_loc, &there_is_improper_intersection_error);

        if (there_is_improper_intersection_error == CSMFALSE)
        {
            struct csmface_t *face_to_solid_below;
            
            face_to_solid_below = csmarrayc_get_st(set_of_null_faces_below, i, csmface_t);
            csmsetopcom_move_face_to_solid(0, face_to_solid_below, work_solid, solid_below_loc, &there_is_improper_intersection_error);
        }

        if (there_is_improper_intersection_error == CSMTRUE)
            break;
    }

    csmsetopcom_cleanup_solid(work_solid, solid_above_loc);
    csmsetopcom_cleanup_solid(work_solid, solid_below_loc);

    if (there_is_improper_intersection_error == CSMTRUE)
    {
        did_finish = CSMFALSE;

        csmsolid_free(&solid_above_loc);
        csmsolid_free(&solid_below_loc);
    }
    else
    {   
        did_finish = CSMTRUE;
    
        csmsolid_redo_geometric_face_data(solid_above_loc);
        csmsetopcom_correct_faces_after_joining_null_edges(solid_above_loc, tolerances);
    
        csmsolid_redo_geometric_face_data(solid_below_loc);
        csmsetopcom_correct_faces_after_joining_null_edges(solid_below_loc, tolerances);

        if (csmdebug_debug_enabled() == CSMTRUE)
        {
            csmsolid_print_debug(work_solid, CSMTRUE);
            csmsolid_print_debug(solid_above_loc, CSMTRUE);
            csmsolid_print_debug(solid_below_loc, CSMTRUE);
        }
    }
 
    csmdebug_end_context();
    
    *solid_above = solid_above_loc;
    *solid_below = solid_below_loc;
    
    csmarrayc_free_st(&set_of_null_faces_above, csmface_t, NULL);

    return did_finish;
}

// ----------------------------------------------------------------------------------------------------

enum csmsplit_opresult_t csmsplit_split_solid(
                        const struct csmsolid_t *solid,
                        double A, double B, double C, double D,
                        struct csmsolid_t **solid_above, struct csmsolid_t **solid_below)
{
    enum csmsplit_opresult_t operation_result;
    struct csmsolid_t *solid_above_loc, *solid_below_loc;
    struct csmtolerance_t *tolerances;
    CSMBOOL apply_perturbation;
    double D_work;
    unsigned long no_perturbations;

    csmdebug_begin_context("Split");
    
    tolerances = csmtolerance_new();

    apply_perturbation = CSMFALSE;
    D_work = D;
    no_perturbations = 0;

    do
    {
        struct csmsolid_t *work_solid;
        csmArrayStruct(csmvertex_t) *set_of_on_vertices;
        csmArrayStruct(csmedge_t) *set_of_null_edges;

        apply_perturbation = CSMFALSE;

        work_solid = csmsolid_duplicate(solid);
        csmsolid_redo_geometric_face_data(work_solid);
        csmsolid_clear_algorithm_data(work_solid);

        csmdebug_set_viewer_results(NULL, NULL);
        csmdebug_set_viewer_parameters(work_solid, NULL);
        csmdebug_set_plane(A, B, C, D);
        //csmdebug_show_viewer();

        set_of_on_vertices = i_split_edges_by_plane(work_solid, A, B, C, D_work, tolerances);
    
        if (csmdebug_debug_enabled() == CSMTRUE)
        {
            csmdebug_print_debug_info("---->WORK SOLID BEFORE INSERTING NULL EDGES\n");
            csmsolid_print_debug(work_solid, CSMTRUE);
            csmdebug_print_debug_info("<----WORK SOLID BEFORE INSERTING NULL EDGES\n");
        }

        set_of_null_edges = i_insert_nulledges_to_split_solid(A, B, C, D_work, tolerances, set_of_on_vertices);

        if (csmdebug_debug_enabled() == CSMTRUE)
        {
            csmdebug_print_debug_info("---->WORK SOLID AFTER INSERTING NULL EDGES\n");
            csmsolid_print_debug(work_solid, CSMTRUE);
            csmdebug_print_debug_info("<----WORK SOLID AFTER INSERTING NULL EDGES\n");
        }

        //csmdebug_show_viewer();
    
        if (csmarrayc_count_st(set_of_null_edges, csmedge_t) == 0)
        {
            operation_result = CSMSPLIT_OPRESULT_NO;
        
            solid_above_loc = NULL;
            solid_below_loc = NULL;
        }
        else
        {
            csmArrayStruct(csmface_t) *set_of_null_faces;
            CSMBOOL did_join_all_null_edges;
        
            i_join_null_edges(set_of_null_edges, tolerances, &set_of_null_faces, &did_join_all_null_edges);
        
            if (did_join_all_null_edges == CSMFALSE || csmarrayc_count_st(set_of_null_faces, csmface_t) == 0)
            {
                operation_result = CSMSPLIT_OPRESULT_IMPROPER_CUT;
            
                solid_above_loc = NULL;
                solid_below_loc = NULL;
            }
            else
            {   
                if (i_did_finish_split(set_of_null_faces, work_solid, tolerances, &solid_above_loc, &solid_below_loc) == CSMFALSE)
                {
                    operation_result = CSMSPLIT_OPRESULT_IMPROPER_CUT;

                    if (csmdebug_get_treat_improper_solid_operations_as_errors() == CSMTRUE)
                        apply_perturbation = CSMFALSE;
                    else
                        apply_perturbation = CSMTRUE;
                }
                else
                {
                    double volume_above, volume_below;

                    csmsolid_clear_algorithm_data(solid_above_loc);
                    csmsolid_clear_algorithm_data(solid_below_loc);
            
                    assert(csmsolid_is_empty(work_solid) == CSMTRUE);
            
                    csmsolid_redo_geometric_face_data(solid_above_loc);
                    volume_above = csmsolid_volume(solid_above_loc);
            
                    csmsolid_redo_geometric_face_data(solid_below_loc);
                    volume_below = csmsolid_volume(solid_below_loc);
            
                    if (volume_above > 1.e-6 || volume_below > 1.e-6)
                    {
                        operation_result = CSMSPLIT_OPRESULT_OK;
            
                        if (volume_above > 1.e-6)
                            csmsimplifysolid_simplify(solid_above_loc, tolerances);
                
                        if (volume_below > 1.e-6)
                            csmsimplifysolid_simplify(solid_below_loc, tolerances);
                    }
                    else
                    {
                        operation_result = CSMSPLIT_OPRESULT_NO;
                
                        csmsolid_free(&solid_above_loc);
                        csmsolid_free(&solid_below_loc);
                    }
                }
            }

            if (apply_perturbation == CSMTRUE)
            {
                D_work += csmtolerance_perturbation_increment(tolerances);
                no_perturbations++;
            
                if (csmdebug_debug_enabled() == CSMTRUE)
                {
                    char *text;
                
                    text = copiafor_codigo2("Perturbation %lu, D: %lf\n", no_perturbations, D_work);
                    csmdebug_print_debug_info(text);
                
                    csmstring_free(&text);
                }
            }

            csmarrayc_free_st(&set_of_null_faces, csmface_t, NULL);
        }

        csmsolid_free(&work_solid);
        csmarrayc_free_st(&set_of_on_vertices, csmvertex_t, NULL);
        csmarrayc_free_st(&set_of_null_edges, csmedge_t, NULL);

    } while (operation_result != CSMSPLIT_OPRESULT_OK && apply_perturbation == CSMTRUE && no_perturbations < i_NUM_MAX_PERTURBATIONS);

    if (solid_above != NULL)
        *solid_above = solid_above_loc;
    else if (solid_above_loc != NULL)
        csmsolid_free(&solid_above_loc);
        
    if (solid_below != NULL)
        *solid_below = solid_below_loc;
    else if (solid_below_loc != NULL)
        csmsolid_free(&solid_below_loc);
    
    csmtolerance_free(&tolerances);
    
    csmdebug_end_context();
    
    return operation_result;
}
