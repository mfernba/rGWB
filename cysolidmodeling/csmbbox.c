//
//  csmbbox.c
//  cysolidmodeling
//
//  Created by Manuel Fernández on 22/2/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmbbox.inl"

#include "csmassert.inl"
#include "csmmem.inl"
#include "csmmath.tli"

struct csmbbox_t
{
    double x_min, y_min, z_min;
    double x_max, y_max, z_max;
};

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmbbox_t *, i_crea, (
                        double x_min, double y_min, double z_min,
                        double x_max, double y_max, double z_max))
{
    struct csmbbox_t *bbox;
    
    bbox = MALLOC(struct csmbbox_t);
    
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
    return i_crea(0., 0., 0., 0., 0., 0.);
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

    bbox->x_min = CSMMATH_MIN(bbox->x_min, x);
    bbox->y_min = CSMMATH_MIN(bbox->y_min, y);
    bbox->z_min = CSMMATH_MIN(bbox->z_min, z);
    
    bbox->x_max = CSMMATH_MAX(bbox->x_max, x);
    bbox->y_max = CSMMATH_MAX(bbox->y_max, y);
    bbox->z_max = CSMMATH_MAX(bbox->z_max, z);
}

// ------------------------------------------------------------------------------------------

void csmbbox_maximize_bbox(struct csmbbox_t *bbox_maximizar, const struct csmbbox_t *bbox)
{
    assert_no_null(bbox_maximizar);
    assert_no_null(bbox);

    bbox_maximizar->x_min = CSMMATH_MIN(bbox_maximizar->x_min, bbox->x_min);
    bbox_maximizar->y_min = CSMMATH_MIN(bbox_maximizar->y_min, bbox->y_min);
    bbox_maximizar->z_min = CSMMATH_MIN(bbox_maximizar->z_min, bbox->z_min);
    
    bbox_maximizar->x_max = CSMMATH_MAX(bbox_maximizar->x_max, bbox->x_max);
    bbox_maximizar->y_max = CSMMATH_MAX(bbox_maximizar->y_max, bbox->y_max);
    bbox_maximizar->z_max = CSMMATH_MAX(bbox_maximizar->z_max, bbox->z_max);
}



