//
//  csmtest_quaternion.c
//  rGWB
//
//  Created by Manuel Fernández Baños on 26/3/19.
//  Copyright © 2019 Manuel Fernández. All rights reserved.
//

#include "csmtest_quaternion.inl"

#include "csmassert.inl"
#include "csmmath.inl"
#include "csmmath.tli"
#include "csmquaternion.h"
#include "csmtransform.inl"

// ------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmtransform_t *, i_make_transform, (
                        double dx, double dy, double dz,
                        double scale_factor,
                        double angle_x_rad, double angle_y_rad, double angle_z_rad))
{
    struct csmtransform_t *transform;
    struct csmtransform_t *translation;
    struct csmtransform_t *scale;
    struct csmtransform_t *rx, *ry, *rz, *ryz, *rxyz;
    struct csmtransform_t *rotation_scale;

    translation = csmtransform_make_displacement(dx, dy, dz);
    scale = csmtransform_make_general(scale_factor, 0., 0., 0., 0., scale_factor, 0., 0., 0., 0., scale_factor, 0.);
    
    rx = csmtransform_make_arbitrary_axis_rotation(angle_x_rad, 0., 0., 0., 1., 0., 0.);
    ry = csmtransform_make_arbitrary_axis_rotation(angle_y_rad, 0., 0., 0., 0., 1., 0.);
    rz = csmtransform_make_arbitrary_axis_rotation(angle_z_rad, 0., 0., 0., 0., 0., 1.);

    ryz = csmtransform_multiply(ry, rz);
    rxyz = csmtransform_multiply(rx, ryz);
    
    rotation_scale = csmtransform_multiply(rxyz, scale);
    transform = csmtransform_multiply(translation, rotation_scale);
    
    csmtransform_free(&translation);
    csmtransform_free(&scale);
    csmtransform_free(&rx);
    csmtransform_free(&ry);
    csmtransform_free(&rz);
    csmtransform_free(&ryz);
    csmtransform_free(&rxyz);
    csmtransform_free(&rotation_scale);

    return transform;
}

// ------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmquaternion_t *, i_make_quaternion, (
                        double dx, double dy, double dz,
                        double scale_factor,
                        double angle_x_rad, double angle_y_rad, double angle_z_rad))
{
    struct csmquaternion_t *q;
    struct csmtransform_t *transform;
    double coefs[4][4];
    double coefs2[3][3];

    transform = i_make_transform(dx, dy, dz, scale_factor, angle_x_rad, angle_y_rad, angle_z_rad);
    csmtransform_get_coefs(transform, coefs);
    
    q = csmquaternion_from_rotation_matrix(
                        coefs[0][0], coefs[0][1], coefs[0][2],
                        coefs[1][0], coefs[1][1], coefs[1][2],
                        coefs[2][0], coefs[2][1], coefs[2][2]);
    
    csmquaternion_to_rotation_matrix_3x3(
                        q,
                        &coefs2[0][0], &coefs2[0][1], &coefs2[0][2],
                        &coefs2[1][0], &coefs2[1][1], &coefs2[1][2],
                        &coefs2[2][0], &coefs2[2][1], &coefs2[2][2]);
    
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            assert(csmmath_fabs(coefs[i][j] - coefs2[i][j]) < 1.e-6);
    
    csmtransform_free(&transform);
    
    return q;
}

// ------------------------------------------------------------------------------------

void csmtest_quaternion_test(void)
{
    struct csmquaternion_t *q1, *q2, *q3;
    struct csmquaternion_t *q4, *q5;
    
    q1 = i_make_quaternion(0., 0., 0., 1., 0., 0., 0.);
    q2 = i_make_quaternion(0., 0., 0., 1., 0.5 * CSMMATH_PI, 0., 0.5 * CSMMATH_PI);
    
    q3 = csmquaternion_new_identity();
    
    for (int i = 0; i < 10; i++)
    {
        double t;
        double coefs2[3][3];
        struct csmtransform_t *transform;
        double x0, y0, z0, x1, y1, z1, x2, y2, z2, x3, y3, z3;
        double l1, l2, l3;

        t = 0.1 * i;
        csmquaternion_slerp(q1, q2, t, q3);
    
        csmquaternion_to_rotation_matrix_3x3(
                        q3,
                        &coefs2[0][0], &coefs2[0][1], &coefs2[0][2],
                        &coefs2[1][0], &coefs2[1][1], &coefs2[1][2],
                        &coefs2[2][0], &coefs2[2][1], &coefs2[2][2]);
    
        transform = csmtransform_make_general(
                        coefs2[0][0], coefs2[0][1], coefs2[0][2], 0.,
                        coefs2[1][0], coefs2[1][1], coefs2[1][2], 0.,
                        coefs2[2][0], coefs2[2][1], coefs2[2][2], 0.);
    
        csmtransform_transform_point(transform, 0., 0., 0., &x0, &y0, &z0);
        csmtransform_transform_point(transform, 1., 0., 0., &x1, &y1, &z1);
        csmtransform_transform_point(transform, 0., 1., 0., &x2, &y2, &z2);
        csmtransform_transform_point(transform, 0., 0., 1., &x3, &y3, &z3);
        
        l1 = csmmath_distance_3D(x0, y0, z0, x1, y1, z1);
        assert(csmmath_fabs(l1 - 1.) < 1.e-6);
        
        l2 = csmmath_distance_3D(x0, y0, z0, x2, y2, z2);
        assert(csmmath_fabs(l2 - 1.) < 1.e-6);
        
        l3 = csmmath_distance_3D(x0, y0, z0, x3, y3, z3);
        assert(csmmath_fabs(l3 - 1.) < 1.e-6);

        csmtransform_free(&transform);
    }

    q4 = i_make_quaternion(0., 0., 0., 1., 0.5 * CSMMATH_PI, 0., 0.);
    q5 = i_make_quaternion(0., 0., 0., 1., 0., 0., 0.5 * CSMMATH_PI);
    csmquaternion_concatenate_q2_to_left_q1(q5, q4);
    
    csmquaternion_free(&q1);
    csmquaternion_free(&q2);
    csmquaternion_free(&q3);
    csmquaternion_free(&q4);
    csmquaternion_free(&q5);
}
