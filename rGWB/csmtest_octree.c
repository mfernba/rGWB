//
//  csmtest_octree.c
//  rGWB
//
//  Created by Manuel Fernández on 29/11/18.
//  Copyright © 2018 Manuel Fernández. All rights reserved.
//

#include "csmtest_octree.inl"

#include "csmoctree.h"
#include "csmbbox.inl"
#include "csmarrayc.h"
#include "csmassert.inl"
#include "csmmem.inl"
#include "csmmath.inl"

struct i_point_t
{
    double x, y, z;
};

struct csmoctree(i_point_t);
csmArrayStruct(i_point_t);

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_point_t *, i_new_point, (double x, double y, double z))
{
    struct i_point_t *point;
    
    point = MALLOC(struct i_point_t);
    
    point->x = x;
    point->y = y;
    point->z = z;
    
    return point;
}

// ----------------------------------------------------------------------------------------------------

static void i_free_point(struct i_point_t **point)
{
    FREE_PP(point, struct i_point_t);
}

// ----------------------------------------------------------------------------------------------------

static CSMBOOL i_is_point_in_bbox(const struct i_point_t *point, const struct csmbbox_t *bbox)
{
    assert_no_null(point);
    return csmbbox_contains_point(bbox, point->x, point->y, point->z);
}

// ----------------------------------------------------------------------------------------------------

static void i_append_point(double x, double y, double z, struct csmoctree(i_point_t) *points_octree, csmArrayStruct(i_point_t) *points)
{
    struct i_point_t *point;
    
    point = i_new_point(x, y, z);
    
    csmarrayc_append_element_st(points, point, i_point_t);
    csmoctree_append_item(points_octree, i_point_t, point);
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmbbox_t *, i_new_bbox_with_dimensions, (double x_min, double y_min, double z_min, double x_max, double y_max, double z_max))
{
    struct csmbbox_t *bbox;
    
    bbox = csmbbox_create_empty_box();
    csmbbox_maximize_coord(bbox, x_min, y_min, z_min);
    csmbbox_maximize_coord(bbox, x_max, y_max, z_max);
    
    csmbbox_compute_bsphere_and_margins(bbox);
    
    return bbox;
}

// ----------------------------------------------------------------------------------------------------

static void i_test_number_of_elements_in_bbox(
                        double x_min, double y_min, double z_min, double x_max, double y_max, double z_max,
                        struct csmoctree(i_point_t) *points_octree,
                        unsigned long expected_number_of_elements)
{
    struct csmbbox_t *bbox;
    const csmArrayStruct(i_point_t) *points_in_bbox;
    
    bbox = i_new_bbox_with_dimensions(x_min, y_min, z_min, x_max, y_max, z_max);
    
    points_in_bbox = csmoctree_get_bbox_neighbors(points_octree, i_point_t, bbox);
    assert(csmarrayc_count_st(points_in_bbox, i_point_t) == expected_number_of_elements);
    
    csmarrayc_free_const_st(&points_in_bbox, i_point_t);
    csmbbox_free(&bbox);
}

// ----------------------------------------------------------------------------------------------------

static void i_test1(void)
{
    struct csmbbox_t *bbox;
    struct csmoctree(i_point_t) *points_octree;
    csmArrayStruct(i_point_t) *points;
    
    bbox = i_new_bbox_with_dimensions(0., 0., 0., 1., 1., 1.);
    points_octree = csmoctree_new(1, 0.005, &bbox, i_is_point_in_bbox, i_point_t);
    points = csmarrayc_new_st_array(0, i_point_t);
    
    i_append_point(0.25, 0.25, 0.25, points_octree, points);
    i_append_point(0.75, 0.25, 0.25, points_octree, points);
    i_append_point(0.25, 0.75, 0.25, points_octree, points);
    i_append_point(0.75, 0.75, 0.25, points_octree, points);

    i_append_point(0.25, 0.25, 0.75, points_octree, points);
    i_append_point(0.75, 0.25, 0.75, points_octree, points);
    i_append_point(0.25, 0.75, 0.75, points_octree, points);
    i_append_point(0.75, 0.75, 0.75, points_octree, points);
    
    i_test_number_of_elements_in_bbox(0.0, 0.0, 0.0, 1., 1., 1., points_octree, 8);
    i_test_number_of_elements_in_bbox(0.0, 0.0, 0.0, 0.5, 0.5, 0.5, points_octree, 1);
    i_test_number_of_elements_in_bbox(0.0, 0.0, 0.0, 0.5, 0.5, 1.0, points_octree, 2);
    i_test_number_of_elements_in_bbox(0.0, 0.0, 0.0, 0.1, 0.1, 0.1, points_octree, 0);
    i_test_number_of_elements_in_bbox(0.24, 0.24, 0.24, 0.26, 0.26, 0.26, points_octree, 1);
    i_test_number_of_elements_in_bbox(0.25, 0.25, 0.25, 0.26, 0.26, 0.26, points_octree, 1);
    i_test_number_of_elements_in_bbox(0.26, 0.26, 0.26, 0.27, 0.27, 0.27, points_octree, 0);
    
    csmoctree_free(&points_octree, i_point_t);
    csmarrayc_free_st(&points, i_point_t, i_free_point);
}

// ----------------------------------------------------------------------------------------------------

void csmtest_octree_test(void)
{
    i_test1();
}
