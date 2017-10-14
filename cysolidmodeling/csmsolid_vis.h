//
//  csmsolid_vis.h
//  cysolidmodeling
//
//  Created by Manuel Fernandez on 14/10/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmfwddecl.hxx"

DLL_CYSOLIDMODELING void csmsolid_vis_draw_debug_info(struct csmsolid_t *solido, CSMBOOL draw_edge_info, struct bsgraphics2_t *graphics);

DLL_CYSOLIDMODELING void csmsolid_vis_draw(
                            struct csmsolid_t *solido,
                            CSMBOOL draw_solid_face,
                            CSMBOOL draw_face_normal,
                            const struct bsmaterial_t *face_material,
                            const struct bsmaterial_t *normal_material,
                            const struct bsmaterial_t *border_edges_material,
                            struct bsgraphics2_t *graphics);

