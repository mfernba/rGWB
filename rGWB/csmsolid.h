// Solid...

#include "csmfwddecl.hxx"

#ifdef __cplusplus
extern "C" {
#endif

DLL_RGWB CONSTRUCTOR(struct csmsolid_t *, csmsolid_duplicate, (const struct csmsolid_t *solid));

DLL_RGWB void csmsolid_free(struct csmsolid_t **solid);


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


// Visualization...

DLL_RGWB void csmsolid_set_visualization_material(struct csmsolid_t *solid, struct csmmaterial_t **visz_material);
    

#ifdef __cplusplus
}
#endif
