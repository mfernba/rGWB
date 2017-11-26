// Generate a solid from a 2D-Shape...

#include "csmfwddecl.hxx"

DLL_RGWB CONSTRUCTOR(struct csmsolid_t *, csmsweep_create_solid_from_shape, (
                                const struct csmshape2d_t *shape2d_top,
                                double Xo_top, double Yo_top, double Zo_top,
                                double Ux_top, double Uy_top, double Uz_top, double Vx_top, double Vy_top, double Vz_top,
                                const struct csmshape2d_t *shape2d_bot,
                                double Xo_bot, double Yo_bot, double Zo_bot,
                                double Ux_bot, double Uy_bot, double Uz_bot, double Vx_bot, double Vy_bot, double Vz_bot));

DLL_RGWB CONSTRUCTOR(struct csmsolid_t *, csmsweep_create_solid_from_shape_debug, (
                                const struct csmshape2d_t *shape2d_top,
                                double Xo_top, double Yo_top, double Zo_top,
                                double Ux_top, double Uy_top, double Uz_top, double Vx_top, double Vy_top, double Vz_top,
                                const struct csmshape2d_t *shape2d_bot,
                                double Xo_bot, double Yo_bot, double Zo_bot,
                                double Ux_bot, double Uy_bot, double Uz_bot, double Vx_bot, double Vy_bot, double Vz_bot,
                                unsigned long start_id_of_new_element));


// Sweep along a path...

DLL_RGWB CONSTRUCTOR(struct csmsweep_path_t *, csmsweep_new_empty_path, (void));

DLL_RGWB CONSTRUCTOR(struct csmsweep_path_t *, csmsweep_new_elliptical_plane_path, (
                                double x, double y, double radius_x, double radius_y,
                                unsigned long no_points_circle,
                                double Xo, double Yo, double Zo,
                                double Ux, double Uy, double Uz, double Vx, double Vy, double Vz,
                                const struct csmshape2d_t *shape));

DLL_RGWB CONSTRUCTOR(struct csmsweep_path_t *, csmsweep_new_helix_plane_path, (
                                double x, double y, double radius, unsigned long no_points_circle,
                                double one_helix_heigth, unsigned long no_helix,
                                double Xo, double Yo, double Zo,
                                double Ux, double Uy, double Uz, double Vx, double Vy, double Vz,
                                const struct csmshape2d_t *shape));

DLL_RGWB void csmsweep_free_path(struct csmsweep_path_t **sweep_path);

DLL_RGWB void csmsweep_append_point_to_path(
                                struct csmsweep_path_t *sweep_path,
                                double Xo, double Yo, double Zo,
                                double Ux, double Uy, double Uz, double Vx, double Vy, double Vz,
                                struct csmshape2d_t **shape);

DLL_RGWB CONSTRUCTOR(struct csmsolid_t *, csmsweep_create_from_path, (const struct csmsweep_path_t *sweep_path));
