// Geometric transforms defined by matrices...

#include "csmtransform.inl"

#include "csmmath.inl"

#ifdef __STANDALONE_DISTRIBUTABLE
#include "csmassert.inl"
#include "csmmem.inl"
#else
#include "cyassert.h"
#include "cypespy.h"
#endif

struct csmtransform_t
{
    double data[4][4];
};

// -------------------------------------------------

static void i_init_matrix(double diagona_value, double data[4][4])
{
    int i;
    
    for (i = 0; i < 4; i++)
    {
        int j;
        
        for (j = 0; j < 4; j++)
        {
            if (i == j)
                data[i][j] = diagona_value;
            else
                data[i][j] = 0.;
        }
    }
}

// -------------------------------------------------

CONSTRUCTOR(static struct csmtransform_t *, i_crea, (void))
{
    struct csmtransform_t *transformacion;
    
    transformacion = MALLOC(struct csmtransform_t);
    
    return transformacion;
}

// -------------------------------------------------

struct csmtransform_t *csmtransform_make_identity(void)
{
    struct csmtransform_t *transform;
    
    transform = i_crea();
    assert_no_null(transform);
    
    i_init_matrix(1., transform->data);
    
    return transform;
}

// -------------------------------------------------

struct csmtransform_t *csmtransform_make_displacement(double dx, double dy, double dz)
{
    struct csmtransform_t *transform;
    
    transform = i_crea();
    assert_no_null(transform);
    
    i_init_matrix(1., transform->data);
    
    transform->data[0][3] = dx;
    transform->data[1][3] = dy;
    transform->data[2][3] = dz;
    transform->data[3][3] = 1.;
    
    return transform;
}

// ------------------------------------------------------------------------------------------

struct csmtransform_t *csmtransform_make_arbitrary_axis_rotation(
                        double angulo_rotacion_rad,
                        double Xo, double Yo, double Zo, double Ux, double Uy, double Uz)
{
    struct csmtransform_t *transform;
    double a, b, c;
    double u, v, w;
    double u2, v2, w2;
    double seno, coseno, uno_menos_coseno;
    
    transform = i_crea();
    assert_no_null(transform);
    
    a = Xo;
    b = Yo;
    c = Zo;
    
    u = Ux;
    v = Uy;
    w = Uz;
    
    u2 = u * u;
    v2 = v * v;
    w2 = w * w;
    
    seno = csmmath_sin(angulo_rotacion_rad);
    coseno = csmmath_cos(angulo_rotacion_rad);
    uno_menos_coseno = 1. - coseno;
    
    transform->data[0][0] = u2 + (v2  + w2) * coseno;
    transform->data[0][1] = u * v * uno_menos_coseno - w * seno;
    transform->data[0][2] = u * w * uno_menos_coseno + v * seno;
    transform->data[0][3] = (a * (v2 + w2) - u * (b * v + c * w)) * uno_menos_coseno + (b * w - c * v) * seno;
    
    transform->data[1][0] = u * v * uno_menos_coseno + w * seno;
    transform->data[1][1] = v2 + (u2 + w2) * coseno;
    transform->data[1][2] = v * w * uno_menos_coseno - u * seno;
    transform->data[1][3] = (b * (u2 + w2) - v * (a * u + c * w)) * uno_menos_coseno + (c * u - a * w) * seno;
    
    transform->data[2][0] = u * w * uno_menos_coseno - v * seno;
    transform->data[2][1] = v * w * uno_menos_coseno + u * seno;
    transform->data[2][2] = w2 + (u2 + v2) * coseno;
    transform->data[2][3] = (c * (u2 + v2) - w * (a * u + b * v)) * uno_menos_coseno + (a * v - b * u) * seno;
    
    transform->data[3][0] = 0.;
    transform->data[3][1] = 0.;
    transform->data[3][2] = 0.;
    transform->data[3][3] = 1.;
    
    return transform;
}

// ----------------------------------------------------

struct csmtransform_t *csmtransform_make_general(
                        double Ux, double Uy, double Uz, double Dx,
                        double Vx, double Vy, double Vz, double Dy,
                        double Wx, double Wy, double Wz, double Dz)
{
    struct csmtransform_t *transform;
    
    transform = i_crea();
    assert_no_null(transform);
    
    transform->data[0][0] = Ux;
    transform->data[0][1] = Uy;
    transform->data[0][2] = Uz;
    transform->data[0][3] = Dx;
    
    transform->data[1][0] = Vx;
    transform->data[1][1] = Vy;
    transform->data[1][2] = Vz;
    transform->data[1][3] = Dy;
    
    transform->data[2][0] = Wx;
    transform->data[2][1] = Wy;
    transform->data[2][2] = Wz;
    transform->data[2][3] = Dz;
    
    transform->data[3][0] = 0.;
    transform->data[3][1] = 0.;
    transform->data[3][2] = 0.;
    transform->data[3][3] = 1.;
    
    return transform;
}

// ----------------------------------------------------

void csmtransform_free(struct csmtransform_t **transformacion)
{
    FREE_PP(transformacion, struct csmtransform_t);
}

// ----------------------------------------------------

static double i_product_row_per_vector(const double fila[4], const double vector[4])
{
    return fila[0] * vector[0] + fila[1] * vector[1] + fila[2] * vector[2] + fila[3] * vector[3];
}

// ----------------------------------------------------

static void i_product_matrix_vector(const struct csmtransform_t *transform, const double vector[4], double vector_resultado[4])
{
    assert_no_null(transform);
    
    vector_resultado[0] = i_product_row_per_vector(&transform->data[0][0], vector);
    vector_resultado[1] = i_product_row_per_vector(&transform->data[1][0], vector);
    vector_resultado[2] = i_product_row_per_vector(&transform->data[2][0], vector);
    vector_resultado[3] = i_product_row_per_vector(&transform->data[3][0], vector);
}

// ----------------------------------------------------

void csmtransform_transform_point(
                        const struct csmtransform_t *transform,
                        double x, double y, double z,
                        double *x_trans_opc, double *y_trans_opc, double *z_trans_opc)
{
    double vector[4];
    double vector_resultado[4];
    double w;
    
    vector[0] = x;
    vector[1] = y;
    vector[2] = z;
    vector[3] = 1.;
    
    i_product_matrix_vector(transform, vector, vector_resultado);
    
    w = vector_resultado[3];
    assert(w > 0.);
    
    ASSIGN_OPTIONAL_VALUE(x_trans_opc, vector_resultado[0] / w);
    ASSIGN_OPTIONAL_VALUE(y_trans_opc, vector_resultado[1] / w);
    ASSIGN_OPTIONAL_VALUE(z_trans_opc, vector_resultado[2] / w);
}
