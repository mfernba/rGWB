//
//  csmbasicshape2d.c
//  cysolidmodeling
//
//  Created by Manuel Fernández on 19/11/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmbasicshape2d.h"

#include "csmassert.inl"
#include "csmArrPoint2D.h"
#include "csmshape2d.h"

// ------------------------------------------------------------------------------------------

static void i_append_rectangular_polygon(double dim_x, double dim_y, CSMBOOL invert_polygon, struct csmshape2d_t *shape2d)
{
    csmArrPoint2D *points;
    
    assert(dim_x > 0.);
    assert(dim_y > 0.);
    
    points = csmArrPoint2D_new(4);
    csmArrPoint2D_set(points, 0, -.5 * dim_x, -.5 * dim_y);
    csmArrPoint2D_set(points, 1,  .5 * dim_x, -.5 * dim_y);
    csmArrPoint2D_set(points, 2,  .5 * dim_x,  .5 * dim_y);
    csmArrPoint2D_set(points, 3, -.5 * dim_x,  .5 * dim_y);
    
    if (invert_polygon == CSMTRUE)
        csmArrPoint2D_invert(points);
    
    csmshape2d_append_new_polygon_with_points(shape2d, &points);
}

// ------------------------------------------------------------------------------------------

struct csmshape2d_t *csmbasicshape2d_rectangular_shape(double dim_x, double dim_y)
{
    struct csmshape2d_t *shape2d;
    
    assert(dim_x > 0.);
    assert(dim_y > 0.);
    
    shape2d = csmshape2d_new();
    i_append_rectangular_polygon(dim_x, dim_y, CSMFALSE, shape2d);

    return shape2d;
}

// ------------------------------------------------------------------------------------------

struct csmshape2d_t *csmbasicshape2d_rectangular_hollow_shape(double dim_x, double dim_y, double dim_x_h, double dim_y_h)
{
    struct csmshape2d_t *shape2d;
    
    assert(dim_x > 0.);
    assert(dim_y > 0.);
    assert(dim_x_h < dim_x);
    assert(dim_y_h < dim_y);
    
    shape2d = csmshape2d_new();
    i_append_rectangular_polygon(dim_x, dim_y, CSMFALSE, shape2d);
    i_append_rectangular_polygon(dim_x_h, dim_y_h, CSMTRUE, shape2d);

    return shape2d;
}

// ------------------------------------------------------------------------------------------

struct csmshape2d_t *csmbasicshape2d_circular_shape(double radius, unsigned long no_points_circle)
{
    struct csmshape2d_t *shape2d;
    csmArrPoint2D *points;

    assert(radius > 0.);
    
    points = csmArrPoint2D_new(0);
    csmArrPoint2D_append_circle_points(points, 0., 0., radius, no_points_circle, CSMFALSE);
    
    shape2d = csmshape2d_new();
    csmshape2d_append_new_polygon_with_points(shape2d, &points);
    
    return shape2d;
}

// ------------------------------------------------------------------------------------------

struct csmshape2d_t *csmbasicshape2d_circular_hollow_shape(double outer_radius, double inner_radius, unsigned long no_points_circle)
{
    struct csmshape2d_t *shape2d;
    csmArrPoint2D *points;
    
    assert(outer_radius > 0.);
    assert(inner_radius < outer_radius);

    shape2d = csmshape2d_new();
    
    points = csmArrPoint2D_new(0);
    csmArrPoint2D_append_circle_points(points, 0., 0., outer_radius, no_points_circle, CSMFALSE);
    csmshape2d_append_new_polygon_with_points(shape2d, &points);

    points = csmArrPoint2D_new(0);
    csmArrPoint2D_append_circle_points(points, 0., 0., inner_radius, no_points_circle, CSMTRUE);
    csmshape2d_append_new_polygon_with_points(shape2d, &points);
    
    return shape2d;
}

// ------------------------------------------------------------------------------------------

struct csmshape2d_t *csmbasicshape2d_C_shape(double dim_x, double dim_y)
{
    struct csmshape2d_t *shape2d;
    csmArrPoint2D *points;
    double dim_x_c, dim_y_c;
    
    assert(dim_x > 0.);
    assert(dim_y > 0.);
    
    dim_x_c = dim_x / 3.;
    dim_y_c = dim_y / 3.;
    
    shape2d = csmshape2d_new();
    
    points = csmArrPoint2D_new(0);
    csmArrPoint2D_append(points, -.5 * dim_x, -.5 * dim_y);
    csmArrPoint2D_append(points,  .5 * dim_x, -.5 * dim_y);
    
    csmArrPoint2D_append(points,  .5 * dim_x, -.5 * dim_y + dim_y_c);
    csmArrPoint2D_append(points,  .5 * dim_x - dim_x_c, -.5 * dim_y + dim_y_c);
    csmArrPoint2D_append(points,  .5 * dim_x - dim_x_c, .5 * dim_y - dim_y_c);
    csmArrPoint2D_append(points,  .5 * dim_x, .5 * dim_y - dim_y_c);
    
    csmArrPoint2D_append(points,  .5 * dim_x,  .5 * dim_y);
    csmArrPoint2D_append(points, -.5 * dim_x,  .5 * dim_y);

    csmshape2d_append_new_polygon_with_points(shape2d, &points);
    
    return shape2d;
}

    // ------------------------------------------------------------------------------------------

struct csmshape2d_t *csmbasicshape2d_L_shape(double dim_x, double dim_y)
{
    struct csmshape2d_t *shape2d;
    csmArrPoint2D *points;
    double dim_x_c, dim_y_c;
    
    assert(dim_x > 0.);
    assert(dim_y > 0.);
    
    dim_x_c = dim_x / 3.;
    dim_y_c = dim_y / 3.;
    
    shape2d = csmshape2d_new();
    
    points = csmArrPoint2D_new(0);
    csmArrPoint2D_append(points, -.5 * dim_x, -.5 * dim_y);
    csmArrPoint2D_append(points,  .5 * dim_x, -.5 * dim_y);
    
    csmArrPoint2D_append(points,  .5 * dim_x, -.5 * dim_y + dim_y_c);
    csmArrPoint2D_append(points,  .5 * dim_x - dim_x_c, -.5 * dim_y + dim_y_c);
    csmArrPoint2D_append(points,  .5 * dim_x - dim_x_c, .5 * dim_y - dim_y_c);
    csmArrPoint2D_append(points,  .5 * dim_x - dim_x_c, .5 * dim_y);
    csmArrPoint2D_append(points, -.5 * dim_x,  .5 * dim_y);
    
    csmshape2d_append_new_polygon_with_points(shape2d, &points);
    
    return shape2d;
}

