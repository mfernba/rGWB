// Solid...

#include "csmfwddecl.hxx"

#ifdef __cplusplus
extern "C" {
#endif

DLL_RGWB CONSTRUCTOR(struct csmsolid_t *, csmsolid_duplicate, (const struct csmsolid_t *solid));

DLL_RGWB void csmsolid_free(struct csmsolid_t **solid);

DLL_RGWB CONSTRUCTOR(struct csmsolid_t *, csmsolid_read, (struct csmsave_t *csmsave));

DLL_RGWB void csmsolid_write(const struct csmsolid_t *solid, struct csmsave_t *csmsave);


// Transformations...

DLL_RGWB void csmsolid_move(struct csmsolid_t *solid, double dx, double dy, double dz);

DLL_RGWB void csmsolid_scale(struct csmsolid_t *solid, double Sx, double Sy, double Sz);

DLL_RGWB void csmsolid_rotate(
                        struct csmsolid_t *solid,
                        double angulo_rotacion_rad,
                        double Xo, double Yo, double Zo, double Ux, double Uy, double Uz);

DLL_RGWB void csmsolid_general_transform(
                        struct csmsolid_t *solid,
                        double Ux, double Uy, double Uz, double Dx,
                        double Vx, double Vy, double Vz, double Dy,
                        double Wx, double Wy, double Wz, double Dz);


// Geometric tests...

DLL_RGWB CSMBOOL csmsolid_does_solid_contain_point(
                        const struct csmsolid_t *solid,
                        double x, double y, double z,
                        const struct csmtolerance_t *tolerances);

// Properties...

DLL_RGWB double csmsolid_volume(const struct csmsolid_t *solid);

DLL_RGWB void csmsolid_get_bbox_extension(
                        const struct csmsolid_t *solid,
                        double *x_min, double *y_min, double *z_min,
                        double *x_max, double *y_max, double *z_max);

// Visualization...

DLL_RGWB void csmsolid_set_visualization_material(struct csmsolid_t *solid, struct csmmaterial_t **visz_material);
    

#ifdef __cplusplus
}
#endif
