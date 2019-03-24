//
//  csmquaternion.h
//  rGWB
//
//  Created by Manuel Fernández Baños on 23/3/19.
//  Copyright © 2019 Manuel Fernández. All rights reserved.
//

#include "csmfwddecl.hxx"

DLL_RGWB CONSTRUCTOR(struct csmquaternion_t *, csmquaternion_new, (double w, double x, double y, double z));

DLL_RGWB CONSTRUCTOR(struct csmquaternion_t *, csmquaternion_new_identity, (void));

DLL_RGWB CONSTRUCTOR(struct csmquaternion_t *, csmquaternion_new_from_vector, (double Ux, double Uy, double Uz));

DLL_RGWB CONSTRUCTOR(struct csmquaternion_t *, csmquaternion_new_from_rotation_axis, (double Ux, double Uy, double Uz, double rotation_rad));

DLL_RGWB CONSTRUCTOR(struct csmquaternion_t *, csmquaternion_from_rotation_matrix, (
                        double a00, double a01, double a02,
                        double a10, double a11, double a12,
                        double a20, double a21, double a22));

DLL_RGWB void csmquaternion_free(struct csmquaternion_t **q);


// Operations with quaternions...

DLL_RGWB void csmquaternion_make_zero(struct csmquaternion_t *q);

DLL_RGWB void csmquaternion_inverse(struct csmquaternion_t *q);

DLL_RGWB void csmquaternion_add_q2_to_q1(struct csmquaternion_t *q1, const struct csmquaternion_t *q2, double q2_factor);

DLL_RGWB void csmquaternion_multiply_by_scalar(struct csmquaternion_t *q, double scalar);

DLL_RGWB void csmquaternion_negate(struct csmquaternion_t *q);

DLL_RGWB double csmquaternion_magnitude(const struct csmquaternion_t *q);

DLL_RGWB void csmquaternion_normalize(struct csmquaternion_t *q);

DLL_RGWB double csmquaternion_dot_product(const struct csmquaternion_t *q1, const struct csmquaternion_t *q2);

DLL_RGWB void csmquaternion_concatenate_q2_to_left_q1(struct csmquaternion_t *q1, const struct csmquaternion_t *q2);


// Conversion to other representations...

DLL_RGWB void csmquaternion_convert_to_axis_angle(
                        const struct csmquaternion_t *q,
                        double *Ux, double *Uy, double *Uz, double *rotation_rad);

DLL_RGWB void csmquaternion_to_rotation_matrix_3x3(
                        const struct csmquaternion_t *q,
                        double *a11, double *a12, double *a13,
                        double *a21, double *a22, double *a23,
                        double *a31, double *a32, double *a33);

// Vector rotation...

DLL_RGWB void csmquaternion_apply_rotation_to_vector(const struct csmquaternion_t *q, double *Ux, double *Uy, double *Uz);


// Interpolation...

DLL_RGWB void csmquaternion_lerp(
                        const struct csmquaternion_t *q_start, const struct csmquaternion_t *q_end,
                        double t,
                        struct csmquaternion_t *q_result);

DLL_RGWB void csmquaternion_slerp(
                        const struct csmquaternion_t *q_start, const struct csmquaternion_t *q_end,
                        double t,
                        struct csmquaternion_t *q_result);

