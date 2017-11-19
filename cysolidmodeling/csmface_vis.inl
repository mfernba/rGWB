//
//  csmface_vis.h
//  rGWB
//
//  Created by Manuel Fernandez on 14/10/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmfwddecl.hxx"

void csmface_vis_draw_solid(
                    struct csmface_t *face,
                    CSMBOOL draw_solid_face,
                    CSMBOOL draw_face_normal,
                    const struct bsmaterial_t *face_material,
                    const struct bsmaterial_t *normal_material,
                    struct bsgraphics2_t *graphics);

void csmface_vis_draw_normal(struct csmface_t *face, struct bsgraphics2_t *graphics);
