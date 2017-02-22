//
//  csmbbox.c
//  cysolidmodeling
//
//  Created by Manuel Fernández on 22/2/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmbbox.inl"

#include "cyassert.h"
#include "cypespy.h"
#include "defmath.tlh"

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

struct csmbbox_t *csmbbox_crea_vacia(void)
{
    return i_crea(0., 0., 0., 0., 0., 0.);
}

// ------------------------------------------------------------------------------------------

void csmbbox_destruye(struct csmbbox_t **bbox)
{
    assert_no_null(bbox);
    assert_no_null(*bbox);
    
    FREE_PP(bbox, struct csmbbox_t);
}

// ------------------------------------------------------------------------------------------

void csmbbox_maximiza_coordenada(struct csmbbox_t *bbox, double x, double y, double z)
{
    assert_no_null(bbox);

    bbox->x_min = MIN(bbox->x_min, x);
    bbox->y_min = MIN(bbox->y_min, y);
    bbox->z_min = MIN(bbox->z_min, z);
    
    bbox->x_max = MAX(bbox->x_max, x);
    bbox->y_max = MAX(bbox->y_max, y);
    bbox->z_max = MAX(bbox->z_max, z);
}

// ------------------------------------------------------------------------------------------

void csmbbox_maximiza_bbox(struct csmbbox_t *bbox_maximizar, const struct csmbbox_t *bbox)
{
    assert_no_null(bbox_maximizar);
    assert_no_null(bbox);

    bbox_maximizar->x_min = MIN(bbox_maximizar->x_min, bbox->x_min);
    bbox_maximizar->y_min = MIN(bbox_maximizar->y_min, bbox->y_min);
    bbox_maximizar->z_min = MIN(bbox_maximizar->z_min, bbox->z_min);
    
    bbox_maximizar->x_max = MAX(bbox_maximizar->x_max, bbox->x_max);
    bbox_maximizar->y_max = MAX(bbox_maximizar->y_max, bbox->y_max);
    bbox_maximizar->z_max = MAX(bbox_maximizar->z_max, bbox->z_max);
}



