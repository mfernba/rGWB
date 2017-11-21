//
//  csmsurface.c
//  rGWB
//
//  Created by Manuel Fernández on 21/11/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmsurface.inl"

#include "csmassert.inl"
#include "csmmem.inl"
#include "csmmath.inl"
#include "csmtolerance.inl"

enum i_type_t
{
    i_TYPE_UNDEFINED,
    i_TYPE_ELLIPSOID,
    i_TYPE_CONE,
    i_TYPE_CYLINDER,
    i_TYPE_TORUS,
    i_TYPE_HYPERBOLOID,
};

struct i_ellipsoid_t
{
    double rx, ry, rz;
};

struct i_cone_t
{
    double heigth, radius;
};

struct i_hyperboloid_t
{
    double a, c;
};

struct i_torus_t
{
    double R, r;
};

struct csmsurface_t
{
    enum i_type_t type;
    
    union
    {
        struct i_ellipsoid_t ellipsoid;
        struct i_cone_t cone_or_cylinder;
        struct i_torus_t torus;
        struct i_hyperboloid_t hyperboloid;
    };
};

static const double i_COS_15_DEGREES = 0.9659358;
static const double i_COS_45_DEGREES = 0.70710678;

// --------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmsurface_t *, i_new, (enum i_type_t type))
{
    struct csmsurface_t *surface;
    
    surface = MALLOC(struct csmsurface_t);
    
    surface->type = type;
    
    return surface;
}

// --------------------------------------------------------------------------------

struct csmsurface_t *csmsurface_new_undefined(void)
{
    return i_new(i_TYPE_UNDEFINED);
}

// --------------------------------------------------------------------------------

struct csmsurface_t *csmsurface_new_ellipsoid(double radius_x, double radius_y, double radius_z)
{
    struct csmsurface_t *surface;
    
    surface = i_new(i_TYPE_ELLIPSOID);
    
    surface->ellipsoid.rx = radius_x;
    surface->ellipsoid.ry = radius_y;
    surface->ellipsoid.rz = radius_z;
    
    return surface;
}

// --------------------------------------------------------------------------------

struct csmsurface_t *csmsurface_new_cylinder(double heigth, double radius)
{
    struct csmsurface_t *surface;
    
    surface = i_new(i_TYPE_CONE);
    
    surface->cone_or_cylinder.heigth = heigth;
    surface->cone_or_cylinder.radius = radius;
    
    return surface;
}

// --------------------------------------------------------------------------------

struct csmsurface_t *csmsurface_new_cone(double heigth, double radius)
{
    struct csmsurface_t *surface;
    
    surface = i_new(i_TYPE_CONE);
    
    surface->cone_or_cylinder.heigth = heigth;
    surface->cone_or_cylinder.radius = radius;
    
    return surface;
}

// --------------------------------------------------------------------------------

struct csmsurface_t *csmsurface_new_torus(double R, double r)
{
    struct csmsurface_t *surface;
    
    surface = i_new(i_TYPE_TORUS);
    
    surface->torus.R = R;
    surface->torus.r = r;
    
    return surface;
}

// --------------------------------------------------------------------------------

struct csmsurface_t *csmsurface_new_hyperboloid(double a, double c)
{
    struct csmsurface_t *surface;
    
    surface = i_new(i_TYPE_HYPERBOLOID);
    
    surface->hyperboloid.a = a;
    surface->hyperboloid.c = c;
    
    return surface;
}

// --------------------------------------------------------------------------------

struct csmsurface_t *csmsurface_copy(const struct csmsurface_t *surface)
{
    struct csmsurface_t *surface_copy;
    
    assert_no_null(surface);
    
    surface_copy = i_new(surface->type);
    
    switch (surface_copy->type)
    {
        case i_TYPE_UNDEFINED:
            break;
            
        case i_TYPE_ELLIPSOID:
            
            surface_copy->ellipsoid = surface->ellipsoid;
            break;
            
        case i_TYPE_CONE:
        case i_TYPE_CYLINDER:
            
            surface_copy->cone_or_cylinder = surface->cone_or_cylinder;
            break;
            
        case i_TYPE_TORUS:
            
            surface_copy->torus = surface->torus;
            break;
            
        case i_TYPE_HYPERBOLOID:
            
            surface_copy->hyperboloid = surface->hyperboloid;
            break;
            
        default_error();
    }
    
    return surface_copy;
}

// --------------------------------------------------------------------------------

void csmsurface_free(struct csmsurface_t **surface)
{
    assert_no_null(surface);
    assert_no_null(*surface);
    
    FREE_PP(surface, struct csmsurface_t);
}

// --------------------------------------------------------------------------------

static CSMBOOL i_surfaces_define_border_edge_with_tolerance(
                        double A1, double B1, double C1, double A2, double B2, double C2,
                        double angle_tolerance)
{
    double dot;
    
    dot = csmmath_dot_product3D(A1, B1, C1, A2, B2, C2);

    if (csmmath_fabs(dot) > angle_tolerance)
        return CSMFALSE;
    else
        return CSMTRUE;
}

// --------------------------------------------------------------------------------

CSMBOOL csmsurface_surfaces_define_border_edge(
                        const struct csmsurface_t *surface1, double A1, double B1, double C1,
                        const struct csmsurface_t *surface2, double A2, double B2, double C2)
{
    assert_no_null(surface1);
    assert_no_null(surface2);
    
    if (surface1->type != surface2->type)
    {
        return CSMTRUE;
    }
    else
    {
        double tolerance;
        
        tolerance = 1.e-4;
        
        switch (surface1->type)
        {
            case i_TYPE_UNDEFINED:
                
                return i_surfaces_define_border_edge_with_tolerance(A1, B1, C1, A2, B2, C2, i_COS_15_DEGREES);
                
            case i_TYPE_ELLIPSOID:
            {
                if (csmmath_fabs(surface1->ellipsoid.rx - surface2->ellipsoid.rx) > tolerance
                        || csmmath_fabs(surface1->ellipsoid.ry - surface2->ellipsoid.ry) > tolerance
                        || csmmath_fabs(surface1->ellipsoid.rz - surface2->ellipsoid.rz) > tolerance)
                {
                    return CSMTRUE;
                }
                else
                {
                    return i_surfaces_define_border_edge_with_tolerance(A1, B1, C1, A2, B2, C2, i_COS_45_DEGREES);
                }
                break;
            }
                
            case i_TYPE_CYLINDER:
            case i_TYPE_CONE:
            {
                if (csmmath_fabs(surface1->cone_or_cylinder.heigth - surface2->cone_or_cylinder.heigth) > tolerance
                        || csmmath_fabs(surface1->cone_or_cylinder.radius - surface2->cone_or_cylinder.radius) > tolerance)
                {
                    return CSMTRUE;
                }
                else
                {
                    return i_surfaces_define_border_edge_with_tolerance(A1, B1, C1, A2, B2, C2, i_COS_45_DEGREES);
                }
                break;
            }
                
            case i_TYPE_TORUS:
            {
                if (csmmath_fabs(surface1->torus.R - surface2->torus.R) > tolerance
                        || csmmath_fabs(surface1->torus.r - surface2->torus.r) > tolerance)
                {
                    return CSMTRUE;
                }
                else
                {
                    return i_surfaces_define_border_edge_with_tolerance(A1, B1, C1, A2, B2, C2, i_COS_45_DEGREES);
                }
                break;
            }
                
            case i_TYPE_HYPERBOLOID:
            {
                if (csmmath_fabs(surface1->hyperboloid.a - surface2->hyperboloid.a) > tolerance
                        || csmmath_fabs(surface1->hyperboloid.c - surface2->hyperboloid.c) > tolerance)
                {
                    return CSMTRUE;
                }
                else
                {
                    return i_surfaces_define_border_edge_with_tolerance(A1, B1, C1, A2, B2, C2, i_COS_45_DEGREES);
                }
                break;
            }
                
            default_error();
        }
    }
}

