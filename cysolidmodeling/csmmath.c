//
//  csmmath.c
//  cysolidmodeling
//
//  Created by Manuel Fernández on 22/3/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmmath.inl"

#include "cyassert.h"
#include <math.h>

// ------------------------------------------------------------------------------------------

CYBOOL csmmath_equal_coords(
                        double x1, double y1, double z1,
                        double x2, double y2, double z2,
                        double epsilon)
{
    if (fabs(x1 - x2) > epsilon)
        return FALSO;
    
    if (fabs(y1 - y2) > epsilon)
        return FALSO;
    
    if (fabs(z1 - z2) > epsilon)
        return FALSO;
    
    return CIERTO;
}

// ------------------------------------------------------------------------------------------

double csmmath_length_vector3D(double x, double y, double z)
{
    return sqrt(x * x + y * y + z * z);
}

// ------------------------------------------------------------------------------------------

void csmmath_make_unit_vector3D(double *Ux, double *Uy, double *Uz)
{
    double length;
    
    assert_no_null(Ux);
    assert_no_null(Uy);
    assert_no_null(Uz);
    
    length = csmmath_length_vector3D(*Ux, *Uy, *Uz);
    assert(length > 0.);
    
    *Ux /= length;
    *Uy /= length;
    *Uz /= length;
}

// ------------------------------------------------------------------------------------------

double csmmath_dot_product(double Ux, double Uy, double Uz, double Vx, double Vy, double Vz)
{
    return Ux * Vx + Uy * Vy + Uz * Vz;
}


// ------------------------------------------------------------------------------------------

double csmmath_signed_distance_point_to_plane(double x, double y, double z, double A, double B, double C, double D)
{
	return A * x + B * y + C * z + D;
}
