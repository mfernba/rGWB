//
//  csmface_vis.c
//  cysolidmodeling
//
//  Created by Manuel Fernandez on 14/10/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmface_vis.inl"
#include "csmface.tli"

#include "csmloop.inl"
#include "csmloop_vis.inl"
#include "csmhedge.inl"
#include "csmface.inl"
#include "csmmath.inl"
#include "csmnode.inl"
#include "csmtolerance.inl"
#include "csmvertex.inl"
#include "csmassert.inl"
#include "csmid.inl"
#include "csmmem.inl"
#include "csmmath.inl"
#include "csmmath.tli"
#include "csmmaterial.tli"

#include <basicSystem/bsmaterial.h>
#include <basicGraphics/bsgraphics2.h>
#include <geomcomp/gccontorno.h>

// ----------------------------------------------------------------------------------------------------

void csmface_vis_draw_solid(
                    struct csmface_t *face,
                    CSMBOOL draw_solid_face,
                    CSMBOOL draw_face_normal,
                    const struct bsmaterial_t *face_material,
                    const struct bsmaterial_t *normal_material,
                    struct bsgraphics2_t *graphics)
{
    assert_no_null(face);
    
    if (draw_solid_face == CSMTRUE)
    {
        double Xo, Yo, Zo, Ux, Uy, Uz, Vx, Vy, Vz;
        struct csmloop_t *loop_iterator;
        struct gccontorno_t *shape;
    
        csmmath_plane_axis_from_implicit_plane_equation(
                        face->A, face->B, face->C, face->D,
                        &Xo, &Yo, &Zo,
                        &Ux, &Uy, &Uz, &Vx, &Vy, &Vz);
    
        loop_iterator = csmface_floops(face);
        shape = gccontorno_crea_vacio();
        
        while (loop_iterator != NULL)
        {
            csmloop_vis_append_loop_to_shape(loop_iterator, Xo, Yo, Zo, Ux, Uy, Uz, Vx, Vy, Vz, shape);
            loop_iterator = csmloop_next(loop_iterator);
        }
        
        if (gccontorno_num_poligonos(shape) > 0)
        {
            if (face->visz_material_opt != NULL)
            {
                struct bsmaterial_t *bsmaterial;
                
                bsmaterial = bsmaterial_crea_rgba(face->visz_material_opt->r, face->visz_material_opt->g, face->visz_material_opt->b, face->visz_material_opt->a);
                bsgraphics2_escr_color(graphics, bsmaterial);
                
                bsmaterial_destruye(&bsmaterial);
            }
            else
            {
                bsgraphics2_escr_color(graphics, face_material);
            }
            
            gccontorno_dibuja_3d_ex(shape, Xo, Yo, Zo, Ux, Uy, Uz, Vx, Vy, Vz, CSMFALSE, graphics);
        }
        
        gccontorno_destruye(&shape);
    }

    if (draw_face_normal == CSMTRUE)
    {
        double x_geometric_center, y_geometric_center, z_geometric_center;
        double disp;
        
        bsgraphics2_escr_color(graphics, normal_material);
        csmloop_geometric_center_3d(face->flout, &x_geometric_center, &y_geometric_center, &z_geometric_center);
    
        disp = 0.1;
        bsgraphics2_escr_linea3D(
                        graphics,
                        x_geometric_center, y_geometric_center, z_geometric_center,
                        x_geometric_center + disp * face->A, y_geometric_center + disp * face->B, z_geometric_center + disp * face->C);
    }
}

// ----------------------------------------------------------------------------------------------------

void csmface_vis_draw_normal(struct csmface_t *face, struct bsgraphics2_t *graphics)
{
    double x_geometric_center, y_geometric_center, z_geometric_center;
    double disp;

    assert_no_null(face);
    
    csmloop_geometric_center_3d(face->flout, &x_geometric_center, &y_geometric_center, &z_geometric_center);
    
    disp = 0.10;
    bsgraphics2_escr_linea3D(
                        graphics,
                        x_geometric_center, y_geometric_center, z_geometric_center,
                        x_geometric_center + disp * face->A, y_geometric_center + disp * face->B, z_geometric_center + disp * face->C);
}
