
//
//  csmsweepshape.c
//  rGWB
//
//  Created by Manuel Fernández on 22/3/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//
//  csmsweepshape_create_solid_from_shape creates a new solid given top and bottom shapes.
//  Shapes must have the same number of polygons and points per polygon.
//
//  Sweep its made in such way that the original face becomes the new top face.
//  Shapes can have holes;
//


#include "csmsweep.h"

#include "csmarrayc.inl"
#include "csmassert.inl"
#include "csmgeom.inl"
#include "csmloop.inl"
#include "csmface.inl"
#include "csmhedge.inl"
#include "csmopbas.inl"
#include "csmsolid.inl"
#include "csmeuler_mvfs.inl"
#include "csmeuler_lmev.inl"
#include "csmeuler_lmef.inl"
#include "csmeuler_lkemr.inl"
#include "csmeuler_lkfmrh.inl"
#include "csmeuler_lmfkrh.inl"
#include "csmshape2d.inl"

csmArrayStruct(csmhedge_t);

// --------------------------------------------------------------------------------

static void i_check_compatibility_between_shapes(
                        const struct csmshape2d_t *shape2d_top, const struct csmshape2d_t *shape2d_bot,
                        unsigned long *idx_outer_loop)
{
    unsigned long i, no_of_polygons;
    
    no_of_polygons = csmshape2d_polygon_count(shape2d_top);
    assert(no_of_polygons == csmshape2d_polygon_count(shape2d_bot));
    assert(no_of_polygons > 0);
    assert_no_null(idx_outer_loop);
    
    *idx_outer_loop = ULONG_MAX;
    
    for (i = 0; i < no_of_polygons; i++)
    {
        unsigned long no_of_points_in_polygon;
        CSMBOOL is_hole_loop_top;
        
        no_of_points_in_polygon = csmshape2d_point_polygon_count(shape2d_top, i);
        assert(no_of_points_in_polygon == csmshape2d_point_polygon_count(shape2d_bot, i));
        
        is_hole_loop_top = csmshape2d_polygon_is_hole(shape2d_top, i);
        assert(is_hole_loop_top == csmshape2d_polygon_is_hole(shape2d_bot, i));
        
        if (is_hole_loop_top == CSMFALSE)
        {
            assert(*idx_outer_loop == ULONG_MAX);
            *idx_outer_loop = i;
        }
    }
}

// --------------------------------------------------------------------------------

static void i_coords_point_in_loop_3D(
                        const struct csmshape2d_t *shape2d,
                        unsigned long idx_loop, unsigned long idx_point,
                        double Xo, double Yo, double Zo,
						double Ux, double Uy, double Uz, double Vx, double Vy, double Vz,
                        double *x_3d, double *y_3d, double *z_3d)
{
    double x_2d, y_2d;
    
    csmshape2d_point_polygon_coords(shape2d, idx_loop, idx_point, &x_2d, &y_2d);
    
    csmgeom_coords_2d_to_3d(
                        Xo, Yo, Zo,
						Ux, Uy, Uz, Vx, Vy, Vz,
						x_2d, y_2d,
                        x_3d, y_3d, z_3d);
}

// --------------------------------------------------------------------------------

static void i_append_loop_from_hedge(
                        struct csmhedge_t *first_hedge,
                        const struct csmshape2d_t *shape2d, unsigned long idx_loop,
                        double Xo, double Yo, double Zo,
                        double Ux, double Uy, double Uz, double Vx, double Vy, double Vz,
                        struct csmface_t **new_face,
                        csmArrayStruct(csmhedge_t) **hedges_from_vertexs)
{
    struct csmface_t *new_face_loc;
    csmArrayStruct(csmhedge_t) *hedges_from_vertexs_loc;
    unsigned long i, no_of_points_outer_loop;
    struct csmhedge_t *last_hedge, *he_from_first_vertex;
    
    no_of_points_outer_loop = csmshape2d_point_polygon_count(shape2d, idx_loop);
    assert(no_of_points_outer_loop >= 3);
    assert_no_null(new_face);
    assert_no_null(hedges_from_vertexs);

    last_hedge = first_hedge;
    hedges_from_vertexs_loc = csmarrayc_new_st_array(no_of_points_outer_loop, csmhedge_t);
    
    for (i = 1; i < no_of_points_outer_loop; i++)
    {
        double x, y, z;
        struct csmhedge_t *hedge_from_new_vertex;
        
        i_coords_point_in_loop_3D(
                        shape2d,
                        idx_loop, i,
                        Xo, Yo, Zo,
						Ux, Uy, Uz, Vx, Vy, Vz,
                        &x, &y, &z);
        
        csmeuler_lmev_strut_edge(last_hedge, x, y, z, &hedge_from_new_vertex);
        
        csmarrayc_set_st(hedges_from_vertexs_loc, i, hedge_from_new_vertex, csmhedge_t);
        last_hedge = hedge_from_new_vertex;
    }
    
    csmeuler_lmef(first_hedge, last_hedge, &new_face_loc, &he_from_first_vertex, NULL);
    csmarrayc_set_st(hedges_from_vertexs_loc, 0, he_from_first_vertex, csmhedge_t);
    
    *new_face = new_face_loc;
    *hedges_from_vertexs = hedges_from_vertexs_loc;
}

// --------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmsolid_t *, i_create_solid_from_face, (
                        const struct csmshape2d_t *shape2d,
                        unsigned long start_id_of_new_element,
                        unsigned long idx_outer_loop,
                        double Xo, double Yo, double Zo,
                        double Ux, double Uy, double Uz, double Vx, double Vy, double Vz,
                        struct csmface_t **bottom_face, struct csmface_t **top_face,
                        csmArrayStruct(csmhedge_t) **hedges_from_vertexs))
{
    struct csmsolid_t *solid;
    struct csmface_t *bottom_face_loc, *top_face_loc;
    csmArrayStruct(csmhedge_t) *hedges_from_vertexs_loc;
    double x, y, z;
    struct csmhedge_t *first_hedge;
    
    assert_no_null(bottom_face);
    assert_no_null(top_face);
    assert_no_null(hedges_from_vertexs);
    
    i_coords_point_in_loop_3D(
                        shape2d,
                        idx_outer_loop, 0,
                        Xo, Yo, Zo,
						Ux, Uy, Uz, Vx, Vy, Vz,
                        &x, &y, &z);
    
    solid = csmeuler_mvfs(x, y, z, start_id_of_new_element, &first_hedge);
    top_face_loc = csmopbas_face_from_hedge(first_hedge);
    
    i_append_loop_from_hedge(
                        first_hedge,
                        shape2d, idx_outer_loop,
                        Xo, Yo, Zo,
                        Ux, Uy, Uz, Vx, Vy, Vz,
                        &bottom_face_loc,
                        &hedges_from_vertexs_loc);
    
    *bottom_face = bottom_face_loc;
    *top_face = top_face_loc;
    *hedges_from_vertexs = hedges_from_vertexs_loc;
    
    return solid;
}

// --------------------------------------------------------------------------------

static void i_create_hedges_from_bottom_to_top_face(
                        const struct csmshape2d_t *shape2d_top,
                        unsigned long idx_outer_loop,
                        double Xo_top, double Yo_top, double Zo_top,
                        double Ux_top, double Uy_top, double Uz_top, double Vx_top, double Vy_top, double Vz_top,
                        csmArrayStruct(csmhedge_t) *hedges_from_vertexs_bottom_face)
{
    unsigned long i, no_of_points_outer_loop;
    
    no_of_points_outer_loop = csmshape2d_point_polygon_count(shape2d_top, idx_outer_loop);
    assert(no_of_points_outer_loop >= 3);
    assert(no_of_points_outer_loop == csmarrayc_count_st(hedges_from_vertexs_bottom_face, csmhedge_t));
    
    for (i = 0; i < no_of_points_outer_loop; i++)
    {
        double x, y, z;
        struct csmhedge_t *hedge_from_vertex_in_bottom_face;
        struct csmhedge_t *hedge_from_new_vertex_bottom_to_top;
        
        i_coords_point_in_loop_3D(
                        shape2d_top,
                        idx_outer_loop, i,
                        Xo_top, Yo_top, Zo_top,
                        Ux_top, Uy_top, Uz_top, Vx_top, Vy_top, Vz_top,
                        &x, &y, &z);
        
        hedge_from_vertex_in_bottom_face = csmarrayc_get_st(hedges_from_vertexs_bottom_face, i, csmhedge_t);
        csmeuler_lmev_strut_edge(hedge_from_vertex_in_bottom_face, x, y, z, &hedge_from_new_vertex_bottom_to_top);
    }
}

// --------------------------------------------------------------------------------

static void i_create_lateral_faces(
                        csmArrayStruct(csmhedge_t) *hedges_from_vertexs_bottom_face,
                        struct csmsolid_t *solid)
{
    unsigned long num_hedges;
    unsigned long num_iters;
    struct csmhedge_t *scan, *stop_hedge;
    struct csmhedge_t *hedge_prev, *hedge_next_next;
    
    num_hedges = csmarrayc_count_st(hedges_from_vertexs_bottom_face, csmhedge_t);
    assert(num_hedges >= 3);
    
    scan = csmarrayc_get_st(hedges_from_vertexs_bottom_face, 0, csmhedge_t);
    stop_hedge = csmhedge_prev(csmhedge_prev(scan));
    num_iters = 0;
    
    while (csmhedge_next(scan) != stop_hedge)
    {
        assert(num_iters < 100000);
        num_iters++;
        
        hedge_prev = csmhedge_prev(scan);
        hedge_next_next = csmhedge_next(csmhedge_next(scan));
        
        csmeuler_lmef(hedge_prev, hedge_next_next, NULL, NULL, NULL);
        //csmsolid_debug_print_debug(solid, CSMTRUE);
        
        scan = csmhedge_next(hedge_next_next);
    }
    
    hedge_prev = csmhedge_prev(scan);
    hedge_next_next = csmhedge_next(csmhedge_next(scan));
    
    csmeuler_lmef(hedge_prev, hedge_next_next, NULL, NULL, NULL);
}

// --------------------------------------------------------------------------------

static void i_append_holes_to_solid(
                        struct csmsolid_t *solid,
                        struct csmface_t *top_face, struct csmface_t *bottom_face,
                        unsigned long idx_hole_loop,
                        const struct csmshape2d_t *shape2d_top,
                        double Xo_top, double Yo_top, double Zo_top,
                        double Ux_top, double Uy_top, double Uz_top, double Vx_top, double Vy_top, double Vz_top,
                        const struct csmshape2d_t *shape2d_bot,
                        double Xo_bot, double Yo_bot, double Zo_bot,
                        double Ux_bot, double Uy_bot, double Uz_bot, double Vx_bot, double Vy_bot, double Vz_bot)
{
    unsigned long no_of_points;
    double x, y, z;
    struct csmloop_t *bottom_face_outer_loop;
    struct csmhedge_t *ledge;
    struct csmhedge_t *he_from_vertex, *he_to_vertex, *he_from_ring;
    struct csmloop_t *original_ring_loop;
    struct csmface_t *top_hole_face_loc;
    csmArrayStruct(csmhedge_t) *hedges_from_vertexs_bottom_face;
    
    no_of_points = csmshape2d_point_polygon_count(shape2d_bot, idx_hole_loop);
    assert(no_of_points == csmshape2d_point_polygon_count(shape2d_top, idx_hole_loop));
    
    i_coords_point_in_loop_3D(
                        shape2d_bot,
                        idx_hole_loop, 0,
                        Xo_bot, Yo_bot, Zo_bot,
                        Ux_bot, Uy_bot, Uz_bot, Vx_bot, Vy_bot, Vz_bot,
                        &x, &y, &z);
    
    bottom_face_outer_loop = csmface_flout(bottom_face);
    ledge = csmloop_ledge(bottom_face_outer_loop);
    
    csmeuler_lmev(ledge, ledge, x, y, z, NULL, NULL, &he_from_vertex, &he_to_vertex);
    csmeuler_lkemr(&he_to_vertex, &he_from_vertex, NULL, &he_from_ring);
    
    original_ring_loop = csmhedge_loop(he_from_ring);
    
    //csmsolid_debug_print_debug(solid, CSMTRUE);
    
    i_append_loop_from_hedge(
                        he_from_ring,
                        shape2d_bot, idx_hole_loop,
                        Xo_bot, Yo_bot, Zo_bot,
                        Ux_bot, Uy_bot, Uz_bot, Vx_bot, Vy_bot, Vz_bot,
                        &top_hole_face_loc,
                        &hedges_from_vertexs_bottom_face);

    // Sweep is made in such way that the old face contains the extruded hedges,
    // because hole is initially made in the bottom face, I exchange the generated face and the
    // old ring in order to make the extruded hedges appear in the new face, which will be part of
    // the top face.
    csmeuler_lkfmrh(bottom_face, &top_hole_face_loc);
    csmeuler_lmfkrh(original_ring_loop, &top_hole_face_loc);
    
    //csmsolid_debug_print_debug(solid, CSMTRUE);

    i_create_hedges_from_bottom_to_top_face(
                        shape2d_top,
                        idx_hole_loop,
                        Xo_top, Yo_top, Zo_top,
                        Ux_top, Uy_top, Uz_top, Vx_top, Vy_top, Vz_top,
                        hedges_from_vertexs_bottom_face);

    //csmsolid_debug_print_debug(solid, CSMTRUE);
    
    i_create_lateral_faces(hedges_from_vertexs_bottom_face, solid);
 
    //csmsolid_debug_print_debug(solid, CSMTRUE);
    
    csmeuler_lkfmrh(top_face, &top_hole_face_loc);
    
    //csmsolid_debug_print_debug(solid, CSMTRUE);

    csmarrayc_free_st(&hedges_from_vertexs_bottom_face, csmhedge_t, NULL);
}

// --------------------------------------------------------------------------------

static void i_append_holes_to_solid_if_proceed(
                        struct csmsolid_t *solid,
                        struct csmface_t *top_face, struct csmface_t *bottom_face,
                        const struct csmshape2d_t *shape2d_top,
                        double Xo_top, double Yo_top, double Zo_top,
                        double Ux_top, double Uy_top, double Uz_top, double Vx_top, double Vy_top, double Vz_top,
                        const struct csmshape2d_t *shape2d_bot,
                        double Xo_bot, double Yo_bot, double Zo_bot,
                        double Ux_bot, double Uy_bot, double Uz_bot, double Vx_bot, double Vy_bot, double Vz_bot)
{
    unsigned long i, no_of_polygons;
    
    no_of_polygons = csmshape2d_polygon_count(shape2d_top);
    assert(no_of_polygons == csmshape2d_polygon_count(shape2d_bot));
    assert(no_of_polygons > 0);
    
    for (i = 0; i < no_of_polygons; i++)
    {
        CSMBOOL is_hole_loop;
        
        is_hole_loop = csmshape2d_polygon_is_hole(shape2d_top, i);
        assert(is_hole_loop == csmshape2d_polygon_is_hole(shape2d_bot, i) == CSMTRUE);
        
        if (is_hole_loop == CSMTRUE)
        {
            i_append_holes_to_solid(
                        solid,
                        top_face, bottom_face,
                        i,
                        shape2d_top,
                        Xo_top, Yo_top, Zo_top,
                        Ux_top, Uy_top, Uz_top, Vx_top, Vy_top, Vz_top,
                        shape2d_bot,
                        Xo_bot, Yo_bot, Zo_bot,
                        Ux_bot, Uy_bot, Uz_bot, Vx_bot, Vy_bot, Vz_bot);
        }
    }
}

// --------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmsolid_t *, i_create_solid_from_shape_without_holes, (
                        const struct csmshape2d_t *shape2d_top,
                        double Xo_top, double Yo_top, double Zo_top,
                        double Ux_top, double Uy_top, double Uz_top, double Vx_top, double Vy_top, double Vz_top,
                        const struct csmshape2d_t *shape2d_bot,
                        double Xo_bot, double Yo_bot, double Zo_bot,
                        double Ux_bot, double Uy_bot, double Uz_bot, double Vx_bot, double Vy_bot, double Vz_bot,
                        unsigned long start_id_of_new_element,
                        struct csmface_t **bottom_face, struct csmface_t **top_face))
{
    struct csmsolid_t *solid;
    unsigned long idx_outer_loop;
    csmArrayStruct(csmhedge_t) *hedges_from_vertexs_bottom_face;
    
    i_check_compatibility_between_shapes(shape2d_top, shape2d_bot, &idx_outer_loop);

    solid = i_create_solid_from_face(
                        shape2d_bot,
                        start_id_of_new_element,
                        idx_outer_loop,
                        Xo_bot, Yo_bot, Zo_bot,
                        Ux_bot, Uy_bot, Uz_bot, Vx_bot, Vy_bot, Vz_bot,
                        bottom_face, top_face,
                        &hedges_from_vertexs_bottom_face);
    
    //csmsolid_debug_print_debug(solid, CSMTRUE);

    i_create_hedges_from_bottom_to_top_face(
                        shape2d_top,
                        idx_outer_loop,
                        Xo_top, Yo_top, Zo_top,
                        Ux_top, Uy_top, Uz_top, Vx_top, Vy_top, Vz_top,
                        hedges_from_vertexs_bottom_face);

    //csmsolid_debug_print_debug(solid, CSMTRUE);
    
    i_create_lateral_faces(hedges_from_vertexs_bottom_face, solid);

    //csmsolid_debug_print_debug(solid, CSMTRUE);
    
    csmarrayc_free_st(&hedges_from_vertexs_bottom_face, csmhedge_t, NULL);
    
    return solid;
}

// --------------------------------------------------------------------------------

struct csmsolid_t *csmsweep_create_solid_from_shape(
                        const struct csmshape2d_t *shape2d_top,
                        double Xo_top, double Yo_top, double Zo_top,
                        double Ux_top, double Uy_top, double Uz_top, double Vx_top, double Vy_top, double Vz_top,
                        const struct csmshape2d_t *shape2d_bot,
                        double Xo_bot, double Yo_bot, double Zo_bot,
                        double Ux_bot, double Uy_bot, double Uz_bot, double Vx_bot, double Vy_bot, double Vz_bot)
{
    unsigned long start_id_of_new_element;
    struct csmsolid_t *solid;
    struct csmface_t *bottom_face, *top_face;
    
    start_id_of_new_element = 0;
    
    solid = i_create_solid_from_shape_without_holes(
                        shape2d_top,
                        Xo_top, Yo_top, Zo_top,
                        Ux_top, Uy_top, Uz_top, Vx_top, Vy_top, Vz_top,
                        shape2d_bot,
                        Xo_bot, Yo_bot, Zo_bot,
                        Ux_bot, Uy_bot, Uz_bot, Vx_bot, Vy_bot, Vz_bot,
                        start_id_of_new_element,
                        &bottom_face, &top_face);
    
    i_append_holes_to_solid_if_proceed(
                        solid,
                        top_face, bottom_face,
                        shape2d_top,
                        Xo_top, Yo_top, Zo_top,
                        Ux_top, Uy_top, Uz_top, Vx_top, Vy_top, Vz_top,
                        shape2d_bot,
                        Xo_bot, Yo_bot, Zo_bot,
                        Ux_bot, Uy_bot, Uz_bot, Vx_bot, Vy_bot, Vz_bot);

    return solid;
}

// --------------------------------------------------------------------------------

struct csmsolid_t *csmsweep_create_solid_from_shape_debug(
                        const struct csmshape2d_t *shape2d_top,
                        double Xo_top, double Yo_top, double Zo_top,
                        double Ux_top, double Uy_top, double Uz_top, double Vx_top, double Vy_top, double Vz_top,
                        const struct csmshape2d_t *shape2d_bot,
                        double Xo_bot, double Yo_bot, double Zo_bot,
                        double Ux_bot, double Uy_bot, double Uz_bot, double Vx_bot, double Vy_bot, double Vz_bot,
                        unsigned long start_id_of_new_element)
{
    struct csmsolid_t *solid;
    struct csmface_t *bottom_face, *top_face;
    
    solid = i_create_solid_from_shape_without_holes(
                        shape2d_top,
                        Xo_top, Yo_top, Zo_top,
                        Ux_top, Uy_top, Uz_top, Vx_top, Vy_top, Vz_top,
                        shape2d_bot,
                        Xo_bot, Yo_bot, Zo_bot,
                        Ux_bot, Uy_bot, Uz_bot, Vx_bot, Vy_bot, Vz_bot,
                        start_id_of_new_element,
                        &bottom_face, &top_face);
    
    i_append_holes_to_solid_if_proceed(
                        solid,
                        top_face, bottom_face,
                        shape2d_top,
                        Xo_top, Yo_top, Zo_top,
                        Ux_top, Uy_top, Uz_top, Vx_top, Vy_top, Vz_top,
                        shape2d_bot,
                        Xo_bot, Yo_bot, Zo_bot,
                        Ux_bot, Uy_bot, Uz_bot, Vx_bot, Vy_bot, Vz_bot);

    return solid;
}









