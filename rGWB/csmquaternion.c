//
//  csmquaternion.c
//  rGWB
//
//  Created by Manuel Fernández Baños on 23/3/19.
//  Copyright © 2019 Manuel Fernández. All rights reserved.
//
// Based on "Essential mathematics for games and interactive graphics"

#include "csmquaternion.h"

#include "csmassert.inl"
#include "csmmath.inl"
#include "csmmem.inl"

struct csmquaternion_t
{
    double w;
    double x, y, z;
};

static const double i_COS_TOLERANCE = 1.e-6;

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

static CSMBOOL i_is_unit_quaternion(double w, double x, double y, double z)
{
    double sq_magnitude;
    
    sq_magnitude = CSMMATH_CUAD(w) + CSMMATH_CUAD(x) + CSMMATH_CUAD(y) + CSMMATH_CUAD(z);
    return IS_TRUE(CSMMATH_ABS(sq_magnitude - 1.) < 1.e-6);
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
    double rotation[3][3];
    
    rotation[0][0] = a00;
    rotation[0][1] = a01;
    rotation[0][2] = a02;

    rotation[1][0] = a10;
    rotation[1][1] = a11;
    rotation[1][2] = a12;

    rotation[2][0] = a20;
    rotation[2][1] = a21;
    rotation[2][2] = a22;
    
    #define rotation(x, y) rotation[x][y]
    
    matrix_trace = a00 + a11 + a22;

    if (matrix_trace > 0.)
    {
        double s, recips;
        
        s = csmmath_sqrt(matrix_trace + 1.);
        assert(s > 0.);
        
        recips = 0.5 / s;
        
        w = s * 0.5f;
        x = (rotation[2][1] - rotation[1][2]) * recips;
        y = (rotation[0][2] - rotation[2][0]) * recips;
        z = (rotation[1][0] - rotation[0][1]) * recips;
    }
    else
    {
        unsigned int i = 0;
        double components[3];
        
        if ( rotation(1,1) > rotation(0,0) )
            i = 1;
        
        if ( rotation(2,2) > rotation(i,i) )
            i = 2;
        
        unsigned int j = (i+1)%3;
        unsigned int k = (j+1)%3;
        
        double s, recips;
        
        s = csmmath_sqrt(rotation(i,i) - rotation(j,j) - rotation(k,k) + 1.0);
        recips = 0.5 / s;

        components[i] = 0.5f*s;
        components[j] = (rotation(j,i) + rotation(i,j)) * recips;
        components[k] = (rotation(k,i) + rotation(i,k)) * recips;
                         
        w = (rotation(k,j) - rotation(j,k)) * recips;
        x = components[0];
        y = components[1];
        z = components[2];
    }

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

void csmquaternion_make_zero(struct csmquaternion_t *q)
{
    assert_no_null(q);
    
    q->w = 0.;
    q->x = 0.;
    q->y = 0.;
    q->z = 0.;
}

// ----------------------------------------------------------------------

void csmquaternion_inverse(struct csmquaternion_t *q)
{
    double magnitude, sq_magnitude;
    
    assert_no_null(q);
    
    magnitude = i_magnitude(q->w, q->x, q->y, q->z);
    sq_magnitude = CSMMATH_CUAD(magnitude);
    
    if (csmmath_fabs(sq_magnitude) > 1.e-6)
    {
        q->w =  q->w / sq_magnitude;
        q->x = -q->x / sq_magnitude;
        q->y = -q->y / sq_magnitude;
        q->z = -q->z / sq_magnitude;
    }
}

// ----------------------------------------------------------------------

void csmquaternion_add_q2_to_q1(struct csmquaternion_t *q1, const struct csmquaternion_t *q2, double q2_factor)
{
    assert_no_null(q1);
    assert_no_null(q2);
    
    q1->w += q2->w * q2_factor;
    
    q1->x += q2->x * q2_factor;
    q1->y += q2->y * q2_factor;
    q1->z += q2->z * q2_factor;
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

    /*
    *a11 = 1. - 2. * (y * y + z * z);
    *a12 = 2. * (x * y - w * z);
    *a13 = 2. * (x * z + w * y);

    *a21 = 2. * (x * y + w * z);
    *a22 = 1. - 2. * (x * x + z * z);
    *a23 = 2. * (y * z - w * x);

    *a31 = 2. * (x * z - w * y);
    *a32 = 2. * (y * z + w * x);
    *a33 = 1. - 2. * (x * x - y * y);
     */
    
    double Nq = CSMMATH_CUAD(w) + CSMMATH_CUAD(x) + CSMMATH_CUAD(y) + CSMMATH_CUAD(z);
    double s = (Nq > 0.0) ? (2.0 / Nq) : 0.0;
    double xs = x * s, ys = y * s, zs = z * s;
    double wx = w * xs, wy = w * ys, wz = w * zs;
    double xx = x * xs, xy = x * ys, xz = x * zs;
    double yy = y * ys, yz = y * zs, zz = z * zs;
    
    *a11 = 1.0 - (yy + zz);
    *a21 = xy + wz;
    *a31 = xz - wy;
    
    *a12 = xy - wz;
    *a22 = 1.0 - (xx + zz);
    *a32 = yz + wx;
    
    *a13 = xz + wy;
    *a23 = yz - wx;
    *a33 = 1.0 - (xx + yy);
}

// ----------------------------------------------------------------------

void csmquaternion_apply_rotation_to_vector(const struct csmquaternion_t *q, double *Ux, double *Uy, double *Uz)
{
    double vMult, crossMult, pMult;
    
    assert_no_null(q);
    assert(i_is_unit_quaternion(q->w, q->x, q->y, q->z) == CSMTRUE);
    assert_no_null(Ux);
    assert_no_null(Uy);
    assert_no_null(Uz);
    
    vMult = 2. * csmmath_dot_product3D(q->x, q->y, q->z, *Ux, *Uy, *Uz);
    crossMult = 2. * q->w;
    pMult = crossMult * q->w - 1.;
    
    *Ux = pMult * (*Ux) + vMult * q->x + crossMult * (q->y * (*Uz) - q->z * (*Uy));
    *Uy = pMult * (*Uy) + vMult * q->y + crossMult * (q->z * (*Ux) - q->x * (*Uz));
    *Uz = pMult * (*Uz) + vMult * q->z + crossMult * (q->x * (*Uy) - q->y * (*Ux));
}

//-------------------------------------------------------------------------------

void csmquaternion_lerp(
                    const struct csmquaternion_t *q_start, const struct csmquaternion_t *q_end,
                    double t,
                    struct csmquaternion_t *q_result)
{
    double cos_theta;
    double startInterp;

    assert(t + 1.e-6 > 0. && t - 1.e-6 < 1.);
    
    cos_theta = csmquaternion_dot_product(q_start, q_end);

    if (cos_theta + i_COS_TOLERANCE > 0.)
        startInterp = 1. - t;
    else
        startInterp = t - 1.;
    
    csmquaternion_make_zero(q_result);
    csmquaternion_add_q2_to_q1(q_result, q_start, startInterp);
    csmquaternion_add_q2_to_q1(q_result, q_end, t);
}

//-------------------------------------------------------------------------------

void csmquaternion_slerp(
                    const struct csmquaternion_t *q_start, const struct csmquaternion_t *q_end,
                    double t,
                    struct csmquaternion_t *q_result)
{
    double cos_theta;
    double startInterp, endInterp;

    assert(t + 1.e-6 > 0. && t - 1.e-6 < 1.);

    cos_theta = csmquaternion_dot_product(q_start, q_end);
    
    // if "angle" between quaternions is less than 90 degrees
    if (cos_theta + i_COS_TOLERANCE > 0.)
    {
        // if angle is greater than zero
        if ((1.0 - cos_theta) > i_COS_TOLERANCE)
        {
            // use standard slerp
            double theta;
            double reciprocal_sin_theta;
            
            theta = csmmath_acos(cos_theta);
            reciprocal_sin_theta = 1. / csmmath_sin(theta);
            
            startInterp = csmmath_sin((1.0 - t) * theta) * reciprocal_sin_theta;
            endInterp = csmmath_sin(t * theta) * reciprocal_sin_theta;
        }
        // angle is close to zero
        else
        {
            // use linear interpolation
            startInterp = 1.0 - t;
            endInterp = t;
        }
    }
    // otherwise, take the shorter route
    else
    {
        // if angle is less than 180 degrees
        if ((1.0 + cos_theta) > i_COS_TOLERANCE)
        {
            // use slerp w/negation of start quaternion
            double theta;
            double reciprocal_sin_theta;

            theta = csmmath_acos(-cos_theta);
            reciprocal_sin_theta = 1. / csmmath_sin(theta);
            
            startInterp = csmmath_sin((t - 1.) * theta) * reciprocal_sin_theta;
            endInterp = csmmath_sin(t * theta) * reciprocal_sin_theta;
        }
        // angle is close to 180 degrees
        else
        {
            // use lerp w/negation of start quaternion
            startInterp = t - 1.0;
            endInterp = t;
        }
    }

    csmquaternion_make_zero(q_result);
    csmquaternion_add_q2_to_q1(q_result, q_start, startInterp);
    csmquaternion_add_q2_to_q1(q_result, q_end, endInterp);
}
