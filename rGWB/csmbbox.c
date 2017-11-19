//
//  csmbbox.c
//  rGWB
//
//  Created by Manuel Fernández on 22/2/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmbbox.inl"

#include "csmassert.inl"
#include "csmmem.inl"
#include "csmmath.tli"
#include "csmtolerance.inl"

struct csmbbox_t
{
    CSMBOOL initialized;
    double x_min, y_min, z_min;
    double x_max, y_max, z_max;
};

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmbbox_t *, i_crea, (
                        CSMBOOL initialized,
                        double x_min, double y_min, double z_min,
                        double x_max, double y_max, double z_max))
{
    struct csmbbox_t *bbox;
    
    bbox = MALLOC(struct csmbbox_t);
    
    bbox->initialized = initialized;
    
    bbox->x_min = x_min;
    bbox->y_min = y_min;
    bbox->z_min = z_min;

    bbox->x_max = x_max;
    bbox->y_max = y_max;
    bbox->z_max = z_max;
    
    return bbox;
}

// ------------------------------------------------------------------------------------------

struct csmbbox_t *csmbbox_create_empty_box(void)
{
    CSMBOOL initialized;
    
    initialized = CSMFALSE;
    return i_crea(initialized, 0., 0., 0., 0., 0., 0.);
}

// ------------------------------------------------------------------------------------------

struct csmbbox_t *csmbbox_copy(const struct csmbbox_t *bbox)
{
    assert_no_null(bbox);
    return i_crea(bbox->initialized, bbox->x_min, bbox->y_min, bbox->z_min, bbox->x_max, bbox->y_max, bbox->z_max);
}

// ------------------------------------------------------------------------------------------

void csmbbox_free(struct csmbbox_t **bbox)
{
    assert_no_null(bbox);
    assert_no_null(*bbox);
    
    FREE_PP(bbox, struct csmbbox_t);
}

// ------------------------------------------------------------------------------------------

void csmbbox_reset(struct csmbbox_t *bbox)
{
    assert_no_null(bbox);

    bbox->initialized = CSMFALSE;
    
    bbox->x_min = 0.;
    bbox->y_min = 0.;
    bbox->z_min = 0.;
    
    bbox->x_max = 0.;
    bbox->y_max = 0.;
    bbox->z_max = 0.;
}

// ------------------------------------------------------------------------------------------

void csmbbox_maximize_coord(struct csmbbox_t *bbox, double x, double y, double z)
{
    assert_no_null(bbox);

    if (bbox->initialized == CSMFALSE)
    {
        bbox->initialized = CSMTRUE;
        
        bbox->x_min = x;
        bbox->y_min = y;
        bbox->z_min = z;

        bbox->x_max = x;
        bbox->y_max = y;
        bbox->z_max = z;
    }
    else
    {
        bbox->x_min = CSMMATH_MIN(bbox->x_min, x);
        bbox->y_min = CSMMATH_MIN(bbox->y_min, y);
        bbox->z_min = CSMMATH_MIN(bbox->z_min, z);

        bbox->x_max = CSMMATH_MAX(bbox->x_max, x);
        bbox->y_max = CSMMATH_MAX(bbox->y_max, y);
        bbox->z_max = CSMMATH_MAX(bbox->z_max, z);
    }
}

// ------------------------------------------------------------------------------------------

void csmbbox_maximize_bbox(struct csmbbox_t *bbox_maximizar, const struct csmbbox_t *bbox)
{
    assert_no_null(bbox_maximizar);
    assert_no_null(bbox);

    if (bbox->initialized == CSMTRUE)
    {
        if (bbox_maximizar->initialized == CSMFALSE)
        {
            bbox_maximizar->initialized = CSMTRUE;
            
            bbox_maximizar->x_min = bbox->x_min;
            bbox_maximizar->y_min = bbox->y_min;
            bbox_maximizar->z_min = bbox->z_min;

            bbox_maximizar->x_max = bbox->x_max;
            bbox_maximizar->y_max = bbox->y_max;
            bbox_maximizar->z_max = bbox->z_max;
        }
        else
        {
            bbox_maximizar->x_min = CSMMATH_MIN(bbox_maximizar->x_min, bbox->x_min);
            bbox_maximizar->y_min = CSMMATH_MIN(bbox_maximizar->y_min, bbox->y_min);
            bbox_maximizar->z_min = CSMMATH_MIN(bbox_maximizar->z_min, bbox->z_min);

            bbox_maximizar->x_max = CSMMATH_MAX(bbox_maximizar->x_max, bbox->x_max);
            bbox_maximizar->y_max = CSMMATH_MAX(bbox_maximizar->y_max, bbox->y_max);
            bbox_maximizar->z_max = CSMMATH_MAX(bbox_maximizar->z_max, bbox->z_max);
        }
    }
}

// ------------------------------------------------------------------------------------------

static double i_compute_margin(double min, double max)
{
    assert(min <= max);
    return CSMMATH_MAX(0.025 * (max - min), 1.e-3);
}

// ------------------------------------------------------------------------------------------

static void i_increase_coordinates(double *min, double *max)
{
    double margin;
    
    assert_no_null(min);
    assert_no_null(max);
    
    margin = i_compute_margin(*min, *max);
    
    *min -= margin;
    *max += margin;
}

// ------------------------------------------------------------------------------------------

void csmbbox_add_margin(struct csmbbox_t *bbox)
{
    assert_no_null(bbox);
    
    i_increase_coordinates(&bbox->x_min, &bbox->x_max);
    i_increase_coordinates(&bbox->y_min, &bbox->y_max);
    i_increase_coordinates(&bbox->z_min, &bbox->z_max);
}

// ------------------------------------------------------------------------------------------

void csmbbox_get_extension(
                        const struct csmbbox_t *bbox,
                        double *x_min, double *y_min, double *z_min,
                        double *x_max, double *y_max, double *z_max)
{
    assert_no_null(bbox);
    assert_no_null(x_min);
    assert_no_null(y_min);
    assert_no_null(z_min);
    assert_no_null(x_max);
    assert_no_null(y_max);
    assert_no_null(z_max);
    
    *x_min = bbox->x_min;
    *y_min = bbox->y_min;
    *z_min = bbox->z_min;
    
    *x_max = bbox->x_max;
    *y_max = bbox->y_max;
    *z_max = bbox->z_max;
}

// ------------------------------------------------------------------------------------------

double csmbbox_minimun_side_length(const struct csmbbox_t *bbox)
{
    double minimun_side_length;
    double diff_x, diff_y, diff_z;
    
    assert_no_null(bbox);
    
    diff_x = bbox->x_max - bbox->x_min;
    diff_y = bbox->y_max - bbox->y_min;
    diff_z = bbox->z_max - bbox->z_min;
    
    minimun_side_length = CSMMATH_MIN(diff_x, diff_y);
    minimun_side_length = CSMMATH_MIN(minimun_side_length, diff_z);
    
    return minimun_side_length;
}

// ------------------------------------------------------------------------------------------

CSMBOOL csmbbox_contains_point(const struct csmbbox_t *bbox, double x, double y, double z)
{
    assert_no_null(bbox);
    
    if (x < bbox->x_min || x > bbox->x_max)
        return CSMFALSE;
    
    if (y < bbox->y_min || y > bbox->y_max)
        return CSMFALSE;
    
    if (z < bbox->z_min || z > bbox->z_max)
        return CSMFALSE;
    
    return CSMTRUE;
}

// ------------------------------------------------------------------------------------------

static CSMBOOL i_exists_intersection_between_bbboxes(
                        double x_min1, double y_min1, double z_min1, double x_max1, double y_max1, double z_max1,
                        double x_min2, double y_min2, double z_min2, double x_max2, double y_max2, double z_max2)
{
    assert(x_min1 < x_max1);
    assert(y_min1 < y_max1);
    assert(z_min1 < z_max1);
    assert(x_min2 < x_max2);
    assert(y_min2 < y_max2);
    assert(z_min2 < z_max2);
    
    if (x_max1 < x_min2 || x_min1 > x_max2)
        return CSMFALSE;
    
    if (y_max1 < y_min2 || y_min1 > y_max2)
        return CSMFALSE;
    
    if (z_max1 < z_min2 || z_min1 > z_max2)
        return CSMFALSE;
    
    return CSMTRUE;
}

// ------------------------------------------------------------------------------------------

CSMBOOL csmbbox_intersects_with_other_bbox(const struct csmbbox_t *bbox1, const struct csmbbox_t *bbox2)
{
    assert_no_null(bbox1);
    assert_no_null(bbox2);
    
    if (i_exists_intersection_between_bbboxes(
                        bbox1->x_min, bbox1->y_min, bbox1->z_min, bbox1->x_max, bbox1->y_max, bbox1->z_max,
                        bbox2->x_min, bbox2->y_min, bbox2->z_min, bbox2->x_max, bbox2->y_max, bbox2->z_max) == CSMTRUE)
    {
        return CSMTRUE;
    }
    else
    {
        return i_exists_intersection_between_bbboxes(
                        bbox2->x_min, bbox2->y_min, bbox2->z_min, bbox2->x_max, bbox2->y_max, bbox2->z_max,
                        bbox1->x_min, bbox1->y_min, bbox1->z_min, bbox1->x_max, bbox1->y_max, bbox1->z_max);
    }
}

// ------------------------------------------------------------------------------------------

CSMBOOL csmbbox_intersects_with_segment(
	                    const struct csmbbox_t *bbox,
                        double x1, double y1, double z1, double x2, double y2, double z2)
{
    double x_min1, x_max1;
    
    assert_no_null(bbox);
    
    x_min1 = CSMMATH_MIN(x1, x2);
    x_max1 = CSMMATH_MAX(x1, x2);

    if (x_max1 < bbox->x_min || x_min1 > bbox->x_max)
    {
        return CSMFALSE;
    }
    else
    {
        double y_min1, y_max1;
        
        y_min1 = CSMMATH_MIN(y1, y2);
        y_max1 = CSMMATH_MAX(y1, y2);
        
        if (y_max1 < bbox->y_min || y_min1 > bbox->y_max)
        {
            return CSMFALSE;
        }
        else
        {
            double z_min1, z_max1;
            
            z_min1 = CSMMATH_MIN(z1, z2);
            z_max1 = CSMMATH_MAX(z1, z2);
            
            if (z_max1 < bbox->z_min || z_min1 > bbox->z_max)
                return CSMFALSE;
            else
                return CSMTRUE;
        }
    }
}








