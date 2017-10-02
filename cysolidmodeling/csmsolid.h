// Solid...

#include "csmfwddecl.hxx"

#ifdef __cplusplus
extern "C" {
#endif

DLL_CYSOLIDMODELING CONSTRUCTOR(struct csmsolid_t *, csmsolid_duplicate, (const struct csmsolid_t *solid));

DLL_CYSOLIDMODELING void csmsolid_free(struct csmsolid_t **solido);


// Transformations...

DLL_CYSOLIDMODELING void csmsolid_move(struct csmsolid_t *solid, double dx, double dy, double dz);

DLL_CYSOLIDMODELING void csmsolid_rotate(
                    struct csmsolid_t *solid,
                    double angulo_rotacion_rad,
                    double Xo, double Yo, double Zo, double Ux, double Uy, double Uz);

DLL_CYSOLIDMODELING void csmsolid_general_transform(
                    struct csmsolid_t *solid,
                    double Ux, double Uy, double Uz, double Dx,
                    double Vx, double Vy, double Vz, double Dy,
                    double Wx, double Wy, double Wz, double Dz);


// Properties...

DLL_CYSOLIDMODELING double csmsolid_volume(const struct csmsolid_t *solid);


// Debug...

DLL_CYSOLIDMODELING void csmsolid_draw_debug_info(struct csmsolid_t *solido, CSMBOOL draw_edge_info, struct bsgraphics2_t *graphics);

DLL_CYSOLIDMODELING void csmsolid_draw(
                            struct csmsolid_t *solido,
                            CSMBOOL draw_solid_face,
                            CSMBOOL draw_face_normal,
                            const struct bsmaterial_t *face_material,
                            const struct bsmaterial_t *normal_material,
                            const struct bsmaterial_t *border_edges_material,
                            struct bsgraphics2_t *graphics);
    
#ifdef __cplusplus
}
#endif
