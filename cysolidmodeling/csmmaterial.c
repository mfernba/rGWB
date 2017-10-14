//
//  csmmaterial.c
//  cysolidmodeling
//
//  Created by Manuel Fernandez on 14/10/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmmaterial.h"
#include "csmmaterial.tli"

#include "csmassert.inl"
#include "csmmem.inl"

// ----------------------------------------------------------------------------------------------------

struct csmmaterial_t *csmmaterial_new_flat_material(float r, float g, float b, float a)
{
    struct csmmaterial_t *material;
    
    material = MALLOC(struct csmmaterial_t);
    
    material->r = r;
    material->g = g;
    material->b = b;
    material->a = a;
    
    return material;
}

// ----------------------------------------------------------------------------------------------------

struct csmmaterial_t *csmmaterial_copy(const struct csmmaterial_t *material)
{
    assert_no_null(material);
    return csmmaterial_new_flat_material(material->r, material->g, material->b, material->a);
}

// ----------------------------------------------------------------------------------------------------

void csmmaterial_free(struct csmmaterial_t **material)
{
    assert_no_null(material);
    assert_no_null(*material);
    
    FREE_PP(material, struct csmmaterial_t);
}