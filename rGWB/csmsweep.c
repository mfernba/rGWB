
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
#include "csmmem.inl"
#include "csmmath.inl"
#include "csmgeom.inl"
#include "csmloop.inl"
#include "csmloopglue.inl"
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
#include "csmshape2d.h"
#include "csmshape2d.inl"
#include "csmsolid.h"
#include "csmsolid.inl"
#include "csmArrPoint2D.h"
#include "csmArrPoint3D.h"
#include "csmtolerance.inl"
#include "csmsubdvfaces.inl"

csmArrayStruct(csmhedge_t);

struct i_sweep_point_t
{
    double Xo, Yo, Zo;
    double Ux, Uy, Uz, Vx, Vy, Vz;
    
    struct csmshape2d_t *shape;
};

struct csmsweep_path_t
{
    csmArrayStruct(i_sweep_point_t) *sweep_points;
    CSMBOOL needs_subdivide_faces;
};

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

// --------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_sweep_point_t *, i_new_sweep_point, (
                        double Xo, double Yo, double Zo,
                        double Ux, double Uy, double Uz, double Vx, double Vy, double Vz,
                        struct csmshape2d_t **shape))
{
    struct i_sweep_point_t *sweep_point;
    
    sweep_point = MALLOC(struct i_sweep_point_t);
    
    sweep_point->Xo = Xo;
    sweep_point->Yo = Yo;
    sweep_point->Zo = Zo;
    
    sweep_point->Ux = Ux;
    sweep_point->Uy = Uy;
    sweep_point->Uz = Uz;
    
    sweep_point->Vx = Vx;
    sweep_point->Vy = Vy;
    sweep_point->Vz = Vz;
    
    sweep_point->shape = ASIGNA_PUNTERO_PP_NO_NULL(shape, struct csmshape2d_t);
    
    return sweep_point;
}

// --------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_sweep_point_t *, i_copy_sweep_point, (const struct i_sweep_point_t *sweep_point))
{
    struct csmshape2d_t *shape;
    
    assert_no_null(sweep_point);
    
    shape = csmshape2d_copy(sweep_point->shape);
    
    return i_new_sweep_point(
                        sweep_point->Xo, sweep_point->Yo, sweep_point->Zo,
                        sweep_point->Ux, sweep_point->Uy, sweep_point->Uz, sweep_point->Vx, sweep_point->Vy, sweep_point->Vz,
                        &shape);
}

// --------------------------------------------------------------------------------

static void i_free_sweep_point(struct i_sweep_point_t **sweep_point)
{
    assert_no_null(sweep_point);
    assert_no_null(*sweep_point);
    
    csmshape2d_free(&(*sweep_point)->shape);
    
    FREE_PP(sweep_point, struct i_sweep_point_t);
}

// --------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmsweep_path_t *, i_new_sweeppath, (csmArrayStruct(i_sweep_point_t) **sweep_points, CSMBOOL needs_subdivide_faces))
{
    struct csmsweep_path_t *sweep_path;
    
    sweep_path = MALLOC(struct csmsweep_path_t);
    
    sweep_path->sweep_points = ASIGNA_PUNTERO_PP_NO_NULL(sweep_points, csmArrayStruct(i_sweep_point_t));
    sweep_path->needs_subdivide_faces = needs_subdivide_faces;
    
    return sweep_path;
}

// --------------------------------------------------------------------------------

struct csmsweep_path_t *csmsweep_new_empty_path(void)
{
    csmArrayStruct(i_sweep_point_t) *sweep_points;
    CSMBOOL needs_subdivide_faces;
    
    sweep_points = csmarrayc_new_st_array(0, i_sweep_point_t);
    needs_subdivide_faces = CSMTRUE;
    
    return i_new_sweeppath(&sweep_points, needs_subdivide_faces);
}

// --------------------------------------------------------------------------------

struct csmsweep_path_t *csmsweep_new_elliptical_plane_path(
                            double x, double y, double radius_x, double radius_y,
                            unsigned long no_points_circle,
                            double Xo, double Yo, double Zo,
                            double Ux, double Uy, double Uz, double Vx, double Vy, double Vz,
                            const struct csmshape2d_t *shape)
{
    struct csmsweep_path_t *sweep_path;
    double Wx, Wy, Wz;
    csmArrPoint2D *points;
    unsigned long i, no_points;
    csmArrayStruct(i_sweep_point_t) *sweep_points;
    const struct i_sweep_point_t *initial_point, *initial_point_copy;
    CSMBOOL needs_subdivide_faces;
    
    csmmath_cross_product3D(Ux, Uy, Uz, Vx, Vy, Vz, &Wx, &Wy, &Wz);
    
    points = csmArrPoint2D_new(0);
    csmArrPoint2D_append_elipse_points(points, x, y, radius_x, radius_y, no_points_circle, CSMFALSE);
    
    no_points = csmArrPoint2D_count(points);
    assert(no_points >= 3);
    
    sweep_points = csmarrayc_new_st_array(no_points, i_sweep_point_t);
    
    for (i = 0; i < no_points; i++)
    {
        double x_i_2d, y_i_2d;
        unsigned long idx_prev, idx_next;
        double x_2d_prev, y_2d_prev, x_2d_next, y_2d_next;
        double x_prev_3d, y_prev_3d, z_prev_3d, x_3d, y_3d, z_3d, x_next_3d, y_next_3d, z_next_3d;
        double Ux_to_prev, Uy_to_prev, Uz_to_prev;
        double Ux_to_next, Uy_to_next, Uz_to_next;
        double Ux_point, Uy_point, Uz_point;
        struct csmshape2d_t *shape2d_copy;
        struct i_sweep_point_t *sweep_point;

        idx_prev = (i == 0)? no_points - 1 : i - 1;
        csmArrPoint2D_get(points, idx_prev, &x_2d_prev, &y_2d_prev);
        
        csmArrPoint2D_get(points, i, &x_i_2d, &y_i_2d);
        
        idx_next = (i == no_points - 1)? 0 : i + 1;
        csmArrPoint2D_get(points, idx_next, &x_2d_next, &y_2d_next);
        
        csmgeom_coords_2d_to_3d(
                            Xo, Yo, Zo,
                            Ux, Uy, Uz, Vx, Vy, Vz,
                            x_i_2d, y_i_2d,
                            &x_3d, &y_3d, &z_3d);
        
        csmgeom_coords_2d_to_3d(
                            Xo, Yo, Zo,
                            Ux, Uy, Uz, Vx, Vy, Vz,
                            x_2d_prev, y_2d_prev,
                            &x_prev_3d, &y_prev_3d, &z_prev_3d);

        csmgeom_coords_2d_to_3d(
                            Xo, Yo, Zo,
                            Ux, Uy, Uz, Vx, Vy, Vz,
                            x_2d_next, y_2d_next,
                            &x_next_3d, &y_next_3d, &z_next_3d);
        
        csmmath_vector_between_two_3D_points(x_3d, y_3d, z_3d, x_prev_3d, y_prev_3d, z_prev_3d, &Ux_to_prev, &Uy_to_prev, &Uz_to_prev);
        csmmath_vector_between_two_3D_points(x_3d, y_3d, z_3d, x_next_3d, y_next_3d, z_next_3d, &Ux_to_next, &Uy_to_next, &Uz_to_next);
        
        Ux_point = .5 * (Ux_to_prev + Ux_to_next);
        Uy_point = .5 * (Uy_to_prev + Uy_to_next);
        Uz_point = .5 * (Uz_to_prev + Uz_to_next);
        csmmath_make_unit_vector3D(&Ux_point, &Uy_point, &Uz_point);
        
        shape2d_copy = csmshape2d_copy(shape);
        
        sweep_point = i_new_sweep_point(x_3d, y_3d, z_3d, Ux_point, Uy_point, Uz_point, Wx, Wy, Wz, &shape2d_copy);
        csmarrayc_set_st(sweep_points, i, sweep_point, i_sweep_point_t);
    }
    
    initial_point = csmarrayc_get_const_st(sweep_points, 0, i_sweep_point_t);
    initial_point_copy = i_copy_sweep_point(initial_point);
    
    csmarrayc_append_element_st(sweep_points, initial_point_copy, i_sweep_point_t);
    
    needs_subdivide_faces = CSMFALSE;
    sweep_path = i_new_sweeppath(&sweep_points, needs_subdivide_faces);
    
    csmArrPoint2D_free(&points);
    
    return sweep_path;
}

// --------------------------------------------------------------------------------

struct csmsweep_path_t *csmsweep_new_helix_plane_path(
                                double x, double y, double radius, unsigned long no_points_circle,
                                double one_helix_heigth, unsigned long no_helix,
                                double Xo, double Yo, double Zo,
                                double Ux, double Uy, double Uz, double Vx, double Vy, double Vz,
                                const struct csmshape2d_t *shape,
                                CSMBOOL counterclockwise_sense)
{
    double Wx, Wy, Wz;
    unsigned long i;
    unsigned long no_points_helix;
    double alpha, incr_alpha, current_heigth, incr_heigth;
    csmArrayStruct(i_sweep_point_t) *sweep_points;
    double Ux_prev, Uy_prev, Uz_prev;
    CSMBOOL needs_subdivide_faces;
    
    assert(radius > 0.);
    assert(no_points_circle > 0);
    assert(one_helix_heigth > 0.);
    assert(no_helix > 0);
    
    csmmath_cross_product3D(Ux, Uy, Uz, Vx, Vy, Vz, &Wx, &Wy, &Wz);
    
    sweep_points = csmarrayc_new_st_array(0, i_sweep_point_t);
    
    alpha = 0.;
    no_points_helix = no_points_circle + 1;
    incr_alpha = 2. * CSMMATH_PI / no_points_circle;
    
    if (counterclockwise_sense == CSMFALSE)
        incr_alpha = -incr_alpha;
    
    current_heigth = 0;;
    incr_heigth = one_helix_heigth / no_points_circle;

    Ux_prev = 0.;
    Uy_prev = 0.;
    Uz_prev = 0.;
    
    for (i = 0; i < no_helix; i++)
    {
        unsigned long j;
        
        for (j = 0; j < no_points_helix; j++)
        {
            double x_i_2d, y_i_2d, z_i_2d;
            double alpha_i_next, x_2d_next, y_2d_next, z_2d_next;
            double x_i_3d, y_i_3d, z_i_3d, x_i_3d_next, y_i_3d_next, z_i_3d_next;
            double Ux_to_next, Uy_to_next, Uz_to_next;
            double Ux_point, Uy_point, Uz_point, Vx_point, Vy_point, Vz_point, Wx_point, Wy_point, Wz_point;
            struct csmshape2d_t *shape2d_copy;
            struct i_sweep_point_t *sweep_point;
            
            x_i_2d = radius * csmmath_cos(alpha);
            y_i_2d = radius * csmmath_sin(alpha);
            z_i_2d = current_heigth;
            
            alpha_i_next = alpha + incr_alpha;
            x_2d_next = radius * csmmath_cos(alpha_i_next);
            y_2d_next = radius * csmmath_sin(alpha_i_next);
            z_2d_next = current_heigth + incr_heigth;
            
            csmgeom_coords_3d_local_to_global(
                                Xo, Yo, Zo,
                                Ux, Uy, Uz, Vx, Vy, Vz,
                                x_i_2d, y_i_2d, z_i_2d,
                                &x_i_3d, &y_i_3d, &z_i_3d);

            csmgeom_coords_3d_local_to_global(
                                Xo, Yo, Zo,
                                Ux, Uy, Uz, Vx, Vy, Vz,
                                x_2d_next, y_2d_next, z_2d_next,
                                &x_i_3d_next, &y_i_3d_next, &z_i_3d_next);

            csmmath_vector_between_two_3D_points(x_i_3d, y_i_3d, z_i_3d, x_i_3d_next, y_i_3d_next, z_i_3d_next, &Ux_to_next, &Uy_to_next, &Uz_to_next);
            csmmath_make_unit_vector3D(&Ux_to_next, &Uy_to_next, &Uz_to_next);

            if (i == 0 && j == 0)
            {
                Ux_point = Ux_to_next;
                Uy_point = Uy_to_next;
                Uz_point = Uz_to_next;
                
                csmmath_cross_product3D(Wx, Wy, Wz, Ux_point, Uy_point, Uz_point, &Vx_point, &Vy_point, &Vz_point);
            }
            else
            {
                Vx_point = 0.5 * (Ux_to_next - Ux_prev);
                Vy_point = 0.5 * (Uy_to_next - Uy_prev);
                Vz_point = 0.5 * (Uz_to_next - Uz_prev);
            }
            
            csmmath_make_unit_vector3D(&Vx_point, &Vy_point, &Vz_point);

            csmmath_cross_product3D(Ux_to_next, Uy_to_next, Uz_to_next, Vx_point, Vy_point, Vz_point, &Wx_point, &Wy_point, &Wz_point);
            csmmath_make_unit_vector3D(&Wx_point, &Wy_point, &Wz_point);

            shape2d_copy = csmshape2d_copy(shape);
            
            sweep_point = i_new_sweep_point(x_i_3d, y_i_3d, z_i_3d, Vx_point, Vy_point, Vz_point, Wx_point, Wy_point, Wz_point, &shape2d_copy);
            csmarrayc_append_element_st(sweep_points, sweep_point, i_sweep_point_t);
            
            alpha += incr_alpha;
            current_heigth += incr_heigth;
            
            Ux_prev = Ux_to_next;
            Uy_prev = Uy_to_next;
            Uz_prev = Uz_to_next;
        }
    }
    
    needs_subdivide_faces = CSMTRUE;
    return i_new_sweeppath(&sweep_points, needs_subdivide_faces);
}

// --------------------------------------------------------------------------------

void csmsweep_free_path(struct csmsweep_path_t **sweep_path)
{
    assert_no_null(sweep_path);
    assert_no_null(*sweep_path);
    
    csmarrayc_free_st(&(*sweep_path)->sweep_points, i_sweep_point_t, i_free_sweep_point);
    
    FREE_PP(sweep_path, struct csmsweep_path_t);
}

// --------------------------------------------------------------------------------

void csmsweep_append_point_to_path(
                        struct csmsweep_path_t *sweep_path,
                        double Xo, double Yo, double Zo,
                        double Ux, double Uy, double Uz, double Vx, double Vy, double Vz,
                        struct csmshape2d_t **shape)
{
    struct i_sweep_point_t *sweep_point;
    
    assert_no_null(sweep_path);
    
    sweep_point = i_new_sweep_point(Xo, Yo, Zo, Ux, Uy, Uz, Vx, Vy, Vz, shape);
    csmarrayc_append_element_st(sweep_path->sweep_points, sweep_point, i_sweep_point_t);
    
    sweep_path->needs_subdivide_faces = CSMTRUE;
}

// --------------------------------------------------------------------------------

static CSMBOOL i_points_are_equal(
                        const struct i_sweep_point_t *point1, const struct i_sweep_point_t *point2,
                        const struct csmtolerance_t *tolerances)
{
    assert_no_null(point1);
    assert_no_null(point2);
    
    if (csmmath_equal_coords(
                        point1->Xo, point1->Yo, point1->Zo,
                        point2->Xo, point2->Yo, point2->Zo,
                        csmtolerance_equal_coords(tolerances)) == CSMFALSE)
    {
        return CSMFALSE;
    }
    else if (csmmath_vectors_are_parallel(
                        point1->Ux, point1->Uy, point1->Uz,
                        point2->Ux, point2->Uy, point2->Uz,
                        tolerances) == CSMFALSE)
    {
        return CSMFALSE;
    }
    else if (csmmath_vectors_are_parallel(
                        point1->Vx, point1->Vy, point1->Vz,
                        point2->Vx, point2->Vy, point2->Vz,
                        tolerances) == CSMFALSE)
    {
        return CSMFALSE;
    }
    else
    {
        return CSMTRUE;
    }
}

// --------------------------------------------------------------------------------

struct csmsolid_t *csmsweep_create_from_path(const struct csmsweep_path_t *sweep_path)
{
    unsigned long start_id_of_new_element;
    
    start_id_of_new_element = 0;
    return csmsweep_create_from_path_debug(sweep_path, start_id_of_new_element);
}

// --------------------------------------------------------------------------------

struct csmsolid_t *csmsweep_create_from_path_debug(const struct csmsweep_path_t *sweep_path, unsigned long start_id_of_new_element)
{
    struct csmsolid_t *solid;
    const struct i_sweep_point_t *initial_point, *end_point;
    struct csmtolerance_t *tolerances;
    unsigned long i, no_points;
    struct csmface_t *bottom_face_first_solid, *top_face_solid_prev;
    
    assert_no_null(sweep_path);
    no_points = csmarrayc_count_st(sweep_path->sweep_points, i_sweep_point_t);
    assert(no_points >= 2);
    
    tolerances = csmtolerance_new();

    solid = NULL;
    
    bottom_face_first_solid = NULL;
    top_face_solid_prev = NULL;
    
    for (i = 0; i < no_points - 1; i++)
    {
        const struct i_sweep_point_t *point1, *point2;
        struct csmsolid_t *solid_i;
        struct csmface_t *bottom_face_i, *top_face_i;
        
        point1 = csmarrayc_get_const_st(sweep_path->sweep_points, i, i_sweep_point_t);
        assert_no_null(point1);
        
        point2 = csmarrayc_get_const_st(sweep_path->sweep_points, i + 1, i_sweep_point_t);
        assert_no_null(point2);
        
        solid_i = i_create_solid_from_shape_without_holes(
                        point2->shape,
                        point2->Xo, point2->Yo, point2->Zo,
                        point2->Ux, point2->Uy, point2->Uz, point2->Vx, point2->Vy, point2->Vz,
                        point1->shape,
                        point1->Xo, point1->Yo, point1->Zo,
                        point1->Ux, point1->Uy, point1->Uz, point1->Vx, point1->Vy, point1->Vz,
                        start_id_of_new_element,
                        &bottom_face_i, &top_face_i);
        
        if (i == 0)
        {
            solid = solid_i;
            bottom_face_first_solid = bottom_face_i;
        }
        else
        {
            csmsolid_merge_solids(solid, solid_i);
            
            csmloopglue_merge_faces(top_face_solid_prev, &bottom_face_i, tolerances);
            csmsolid_free(&solid_i);
        }
        
        top_face_solid_prev = top_face_i;
        //break;
    }
    
    initial_point = csmarrayc_get_const_st(sweep_path->sweep_points, 0, i_sweep_point_t);
    end_point = csmarrayc_get_const_st(sweep_path->sweep_points, no_points - 1, i_sweep_point_t);
    
    if (i_points_are_equal(initial_point, end_point, tolerances) == CSMTRUE)
        csmloopglue_merge_faces(top_face_solid_prev, &bottom_face_first_solid, tolerances);
    
    if (sweep_path->needs_subdivide_faces == CSMTRUE)
        csmsubdvfaces_subdivide_faces(solid);
    
    csmsolid_redo_geometric_face_data(solid);

    return solid;
}




















