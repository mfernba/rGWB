// Geometric transforms defined by matrices...

#include "csmfwddecl.hxx"

// Constructores...

CONSTRUCTOR(struct csmtransform_t *, csmtransform_make_identity, (void));

CONSTRUCTOR(struct csmtransform_t *, csmtransform_make_displacement, (double dx, double dy, double dz));

CONSTRUCTOR(struct csmtransform_t *, csmtransform_make_arbitrary_axis_rotation, (
                        double angulo_rotacion_rad,
                        double Xo, double Yo, double Zo, double Ux, double Uy, double Uz));

CONSTRUCTOR(struct csmtransform_t *, csmtransform_make_general, (
                        double Ux, double Uy, double Uz, double Dx,
                        double Vx, double Vy, double Vz, double Dy,
                        double Wx, double Wy, double Wz, double Dz));

void csmtransform_free(struct csmtransform_t **transformacion);


// Operaciones...

void csmtransform_transform_point(
                            const struct csmtransform_t *transformacion,
                            double x, double y, double z,
                            double *x_trans_opc, double *y_trans_opc, double *z_trans_opc);

CONSTRUCTOR(struct csmtransform_t *, csmtransform_multiply, (const struct csmtransform_t *matrix1, const struct csmtransform_t *matrix2));

void csmtransform_get_coefs(const struct csmtransform_t *transform, double coefs[4][4]);
