// Generate a solid from a 2D-Shape...

#include "csmfwddecl.hxx"

DLL_RGWB CONSTRUCTOR(struct csmsolid_t *, csmsweep_create_solid_from_shape, (
                                const struct gccontorno_t *shape2d_top,
                                double Xo_top, double Yo_top, double Zo_top,
                                double Ux_top, double Uy_top, double Uz_top, double Vx_top, double Vy_top, double Vz_top,
                                const struct gccontorno_t *shape2d_bot,
                                double Xo_bot, double Yo_bot, double Zo_bot,
                                double Ux_bot, double Uy_bot, double Uz_bot, double Vx_bot, double Vy_bot, double Vz_bot));

DLL_RGWB CONSTRUCTOR(struct csmsolid_t *, csmsweep_create_solid_from_shape_debug, (
                                const struct gccontorno_t *shape2d_top,
                                double Xo_top, double Yo_top, double Zo_top,
                                double Ux_top, double Uy_top, double Uz_top, double Vx_top, double Vy_top, double Vz_top,
                                const struct gccontorno_t *shape2d_bot,
                                double Xo_bot, double Yo_bot, double Zo_bot,
                                double Ux_bot, double Uy_bot, double Uz_bot, double Vx_bot, double Vy_bot, double Vz_bot,
                                unsigned long start_id_of_new_element));
