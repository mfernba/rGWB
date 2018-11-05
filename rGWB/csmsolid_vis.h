//
//  csmsolid_vis.h
//  rGWB
//
//  Created by Manuel Fernandez on 14/10/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmfwddecl.hxx"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __STANDALONE_DISTRIBUTABLE

DLL_RGWB void csmsolid_vis_draw_debug_info(struct csmsolid_t *solid, CSMBOOL draw_edge_info, struct bsgraphics2_t *graphics);

DLL_RGWB void csmsolid_vis_draw(
                            struct csmsolid_t *solid,
                            CSMBOOL draw_solid_face,
                            CSMBOOL draw_face_normal,
                            const struct bsmaterial_t *face_material,
                            const struct bsmaterial_t *normal_material,
                            const struct bsmaterial_t *border_edges_material,
                            struct bsgraphics2_t *graphics);

#else

DLL_RGWB void csmsolid_vis_datos_mesh(
                        struct csmsolid_t *solid, 
                        ArrPunto3D **puntos, ArrPunto3D **normales, ArrBool **es_borde,
                        ArrEnum(cplan_tipo_primitiva_t) **tipo_primitivas, ArrPuntero(ArrULong) **inds_caras,
                        ArrPuntero(ArrPunto3D) **lineas_contorno_opc);

DLL_RGWB CONSTRUCTOR(ArrPuntero(ArrPunto3D) *, csmsolid_vis_datos_lineas, (struct csmsolid_t *solid));
    
DLL_RGWB CONSTRUCTOR(ArrArrPuntero(ArrPunto3D) *, csmsolid_vis_caras_solido, (
                        struct csmsolid_t *solid,
                        CSMBOOL only_faces_towards_direction, double Wx, double Wy, double Wz, double tolerance_rad,
                        ArrEstructura(ejes2d_t) **ejes_caras_solido_opc));

#endif

#ifdef __cplusplus
}
#endif
