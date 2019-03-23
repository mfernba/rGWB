//
//  csmquaternion.c
//  rGWB
//
//  Created by Manuel Fernández Baños on 23/3/19.
//  Copyright © 2019 Manuel Fernández. All rights reserved.
//

#include "csmquaternion.h"

#include "csmassert.inl"
#include "csmmath.inl"
#include "csmmem.inl"

struct csmquaternion_t
{
    double w;
    double x, y, z;
};

// ----------------------------------------------------------------------

CONSTRUCTOR(static struct csmquaternion_t *, i_new, (double w, double x, double y, double z))
{
    struct csmquaternion_t *q;
    
    q = MALLOC(struct csmquaternion_t);
    
    q->w = w;
    
    q->x = x;
    q->y = y;
    q->z = z;

    return q;
}

// ----------------------------------------------------------------------

struct csmquaternion_t *csmquaternion_new(double w, double x, double y, double z)
{
    return i_new(w, x, y, z);
}

// ----------------------------------------------------------------------

struct csmquaternion_t *csmquaternion_new_identity(void)
{
    return i_new(1., 0., 0., 0.);
}

// ----------------------------------------------------------------------

struct csmquaternion_t *csmquaternion_new_from_vector(double Ux, double Uy, double Uz)
{
    double w, x, y, z;
    
    w = 0.;
    x = Ux;
    y = Uy;
    z = Uz;
    csmmath_make_unit_vector3D(&x, &y, &z);
    
    return i_new(w, x, y, z);
}

// ----------------------------------------------------------------------

struct csmquaternion_t *csmquaternion_new_from_rotation_axis(double Ux, double Uy, double Uz, double rotation_rad)
{
    double cos_half_rotation, sin_half_rotation;
    double w, x, y, z;
    
    cos_half_rotation = csmmath_cos(0.5 * rotation_rad);
    sin_half_rotation = csmmath_sin(0.5 * rotation_rad);
    
    w = cos_half_rotation;
    x = Ux;
    y = Uy;
    z = Uz;
    csmmath_make_unit_vector3D(&x, &y, &z);
    
    return i_new(w, x * sin_half_rotation, y * sin_half_rotation, z * sin_half_rotation);
}

// ----------------------------------------------------------------------

static double i_magnitude(double w, double x, double y, double z)
{
    return csmmath_sqrt(CSMMATH_CUAD(w) + CSMMATH_CUAD(x) + CSMMATH_CUAD(y) + CSMMATH_CUAD(z));
}

// ----------------------------------------------------------------------

static void i_normalize(double *w, double *x, double *y, double *z)
{
    double magnitude;
    
    assert_no_null(w);
    assert_no_null(x);
    assert_no_null(y);
    assert_no_null(z);
    
    magnitude = i_magnitude(*w, *x, *y, *z);
    
    if (magnitude > 0.)
    {
        *w /= magnitude;
        *x /= magnitude;
        *y /= magnitude;
        *z /= magnitude;
    }
}

// ----------------------------------------------------------------------

struct csmquaternion_t *csmquaternion_from_rotation_matrix(
                        double a00, double a01, double a02,
                        double a10, double a11, double a12,
                        double a20, double a21, double a22)
{
    double matrix_trace;
    double w, x, y, z;
    
    matrix_trace = a00 + a11 + a22;
    w = matrix_trace + 1.;
    
    x = a21 - a12;
    y = a02 - a20;
    z = a10 - a01;

    i_normalize(&w, &x, &y, &z);
    
    return i_new(w, x, y, z);
}

// ----------------------------------------------------------------------

void csmquaternion_free(struct csmquaternion_t **q)
{
    assert_no_null(q);
    assert_no_null(*q);
    
    FREE_PP(q, struct csmquaternion_t);
}

// ----------------------------------------------------------------------

void csmquaternion_add_q2_to_q1(struct csmquaternion_t *q1, const struct csmquaternion_t *q2)
{
    assert_no_null(q1);
    assert_no_null(q2);
    
    q1->w += q2->w;
    
    q1->x += q2->x;
    q1->y += q2->y;
    q1->z += q2->z;
}

// ----------------------------------------------------------------------

void csmquaternion_multiply_by_scalar(struct csmquaternion_t *q, double scalar)
{
    assert_no_null(q);
    
    q->w *= scalar;
    
    q->x *= scalar;
    q->y *= scalar;
    q->z *= scalar;
}

// ----------------------------------------------------------------------

void csmquaternion_negate(struct csmquaternion_t *q)
{
    assert_no_null(q);
    
    q->w = -q->w;
    
    q->x = -q->x;
    q->y = -q->y;
    q->z = -q->z;
}

// ----------------------------------------------------------------------

double csmquaternion_magnitude(const struct csmquaternion_t *q)
{
    assert_no_null(q);
    return i_magnitude(q->w, q->x, q->y, q->z);
}

// ----------------------------------------------------------------------

void csmquaternion_normalize(struct csmquaternion_t *q)
{
    assert_no_null(q);
    i_normalize(&q->w, &q->x, &q->y, &q->z);
}

// ----------------------------------------------------------------------

double csmquaternion_dot_product(const struct csmquaternion_t *q1, const struct csmquaternion_t *q2)
{
    assert_no_null(q1);
    assert_no_null(q2);
    
    return q1->w * q2->w + q1->x * q2->x + q1->y * q2->y + q1->z * q2->z;
}

// ----------------------------------------------------------------------

void csmquaternion_concatenate_q2_to_left_q1(struct csmquaternion_t *q1, const struct csmquaternion_t *q2)
{
    double Ux1, Uy1, Uz1, Ux2, Uy2, Uz2, Wx21, Wy21, Wz21;
    
    assert_no_null(q1);
    assert_no_null(q2);
    
    Ux1 = q1->w * q2->x;
    Uy1 = q1->w * q2->y;
    Uz1 = q1->w * q2->z;

    Ux2 = q2->w * q1->x;
    Uy2 = q2->w * q1->y;
    Uz2 = q2->w * q1->z;
    
    csmmath_cross_product3D(q2->x, q2->y, q2->z, q1->x, q1->y, q1->z, &Wx21, &Wy21, &Wz21);

    q1->w = q1->w * q2->w - csmmath_dot_product3D(q1->x, q1->y, q1->z, q2->x, q2->y, q2->z);
    q1->x = Ux1 + Ux2 + Wx21;
    q1->y = Uy1 + Uy2 + Wy21;
    q1->z = Uz1 + Uz2 + Wz21;
}

// ----------------------------------------------------------------------

void csmquaternion_convert_to_axis_angle(
						const struct csmquaternion_t *q,
                        double *Ux, double *Uy, double *Uz, double *rotation_rad)
{
    double axis_magnitude;
    
    assert_no_null(q);
    assert_no_null(Ux);
    assert_no_null(Uy);
    assert_no_null(Uz);
    assert_no_null(rotation_rad);
    
    axis_magnitude = csmmath_sqrt(1. - CSMMATH_CUAD(q->w));

    *Ux = q->x / axis_magnitude;
    *Uy = q->y / axis_magnitude;
    *Uz = q->z / axis_magnitude;

    *rotation_rad = 2. * csmmath_acos(q->w);
}

// ----------------------------------------------------------------------

void csmquaternion_to_rotation_matrix_3x3(
						const struct csmquaternion_t *q,
                        double *a11, double *a12, double *a13,
                        double *a21, double *a22, double *a23,
                        double *a31, double *a32, double *a33)
{
    double w, x, y, z;
    
    assert_no_null(q);
    assert_no_null(a11);
    assert_no_null(a12);
    assert_no_null(a13);
    assert_no_null(a21);
    assert_no_null(a22);
    assert_no_null(a23);
    assert_no_null(a31);
    assert_no_null(a32);
    assert_no_null(a33);

    w = q->w;
    x = q->x;
    y = q->y;
    z = q->z;
    i_normalize(&w, &x, &y, &z);

    *a11 = 1. - 2. * (y * y + z * z);
    *a12 = 2. * (x * y - w * z);
    *a13 = 2. * (x * z + w * y);

    *a21 = 2. * (x * y + w * z);
    *a22 = 1. - 2. * (x * x + z * z);
    *a23 = 2. * (y * z - w * x);

    *a31 = 2. * (x * z - w * y);
    *a32 = 2. * (y * z + w * x);
    *a33 = 1. - 2. * (x * x - y * y);
}

// ----------------------------------------------------------------------
