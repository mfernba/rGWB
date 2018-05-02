//
//  csmface_vis.h
//  rGWB
//
//  Created by Manuel Fernandez on 14/10/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmfwddecl.hxx"

#ifdef __STANDALONE_DISTRIBUTABLE

void csmface_vis_draw_solid(
                    struct csmface_t *face,
                    CSMBOOL draw_solid_face,
                    CSMBOOL draw_face_normal,
                    const struct bsmaterial_t *face_material,
                    const struct bsmaterial_t *normal_material,
                    struct bsgraphics2_t *graphics);

void csmface_vis_draw_normal(struct csmface_t *face, struct bsgraphics2_t *graphics);

void csmface_vis_draw_edges(
                        struct csmface_t *face,
                        const struct bsmaterial_t *outer_loop,
                        const struct bsmaterial_t *hole_loop,
                        const struct bsmaterial_t *inner_non_hole_loop,
                        struct bsgraphics2_t *graphics);


#else

void csmface_vis_append_datos_mesh(
                    struct csmface_t *face,
                    ArrPunto3D *puntos, ArrPunto3D *normales, ArrBool *es_borde,
                    ArrEnum(cplan_tipo_primitiva_t) *tipo_primitivas, ArrPuntero(ArrULong) *inds_caras);

#endif
