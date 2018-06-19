//
//  csmmaterial.c
//  rGWB
//
//  Created by Manuel Fernandez on 14/10/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmmaterial.h"
#include "csmmaterial.tli"
#include "csmsave.inl"

#ifdef __STANDALONE_DISTRIBUTABLE
#include "csmassert.inl"
#include "csmmem.inl"
#else
#include "cyassert.h"
#include "cypespy.h"
#endif

static const unsigned char i_FILE_VERSION = 0;

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

// ----------------------------------------------------------------------------------------------------

struct csmmaterial_t *csmmaterial_read(struct csmsave_t *csmsave)
{
    unsigned char file_version;
    float r, g, b, a;
    
    file_version = csmsave_read_uchar(csmsave);
    assert(file_version == 0);
    
    r = csmsave_read_float(csmsave);
    g = csmsave_read_float(csmsave);
    b = csmsave_read_float(csmsave);
    a = csmsave_read_float(csmsave);
    
    return csmmaterial_new_flat_material(r, g, b, a);
}

// ----------------------------------------------------------------------------------------------------

void csmmaterial_write(const struct csmmaterial_t *material, struct csmsave_t *csmsave)
{
    assert_no_null(material);
    
    csmsave_write_uchar(csmsave, i_FILE_VERSION);
    
    csmsave_write_float(csmsave, material->r);
    csmsave_write_float(csmsave, material->g);
    csmsave_write_float(csmsave, material->b);
    csmsave_write_float(csmsave, material->a);
}

