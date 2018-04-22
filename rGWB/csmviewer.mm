//
//  csmviewer.c
//  SolidModelingTester
//
//  Created by Manuel Fernandez on 23/7/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmviewer.inl"

#ifdef __cplusplus
extern "C" {
#endif
    
#include "csmface_vis.inl"
#include "csmsolid.h"
#include "csmsolid_vis.h"
#include "csmsolid.inl"
#include "csmdebug.inl"

#ifdef __cplusplus
}
#endif

#include <basicSystem/bsmaterial.h>
#include <basicGraphics/bsgraphics2.h>
#include <basicGraphics/bsgbibluz.h>
#include <basicSystem/bsassert.h>
#include <basicSystem/bsmem.h>
#include <WinFrames2/wf2DlgUtil.h>
#include <WinFrames2/wf2pnutil.h>
#include <WinFrames3/wf3Visor.h>
#include <WinFrames3/wf3Vista.h>

struct csmviewer_t
{
    struct csmface_t *face1, *face2;
    
    struct csmsolid_t *solid1, *solid2;
    struct csmsolid_t *solid_res1, *solid_res2;
    
    struct wf3Visor_t *viewer_widget;
};

// ------------------------------------------------------------------------------------------

static struct csmviewer_t *i_new(
                    struct csmface_t **face1, struct csmface_t **face2,
                    struct csmsolid_t **solid1, struct csmsolid_t **solid2,
                    struct csmsolid_t **solid_res1, struct csmsolid_t **solid_res2)
{
    struct csmviewer_t *viewer;
    
    viewer = bsmem_malloc(struct csmviewer_t);
    
    viewer->face1 = bsmem_asigna_puntero_pp(face1, struct csmface_t);
    viewer->face2 = bsmem_asigna_puntero_pp(face2, struct csmface_t);
    
    viewer->solid1 = bsmem_asigna_puntero_pp(solid1, struct csmsolid_t);
    viewer->solid2 = bsmem_asigna_puntero_pp(solid2, struct csmsolid_t);
    
    viewer->solid_res1 = bsmem_asigna_puntero_pp(solid_res1, struct csmsolid_t);
    viewer->solid_res2 = bsmem_asigna_puntero_pp(solid_res2, struct csmsolid_t);
    
    viewer->viewer_widget = NULL;
    
    return viewer;
}

// ------------------------------------------------------------------------------------------

struct csmviewer_t *csmviewer_new(void)
{
    struct csmface_t *face1, *face2;
    struct csmsolid_t *solid1, *solid2;
    struct csmsolid_t *solid_res1, *solid_res2;
    
    face1 = NULL;
    face2 = NULL;
    solid1 = NULL;
    solid2 = NULL;
    solid_res1 = NULL;
    solid_res2 = NULL;
    
    return i_new(&face1, &face2, &solid1, &solid2, &solid_res1, &solid_res2);
}

// ------------------------------------------------------------------------------------------

void csmviewer_free(struct csmviewer_t **viewer)
{
    bsassert_not_null(viewer);
    bsassert_not_null(*viewer);
    
    bsmem_destruye(viewer, struct csmviewer_t);
}

// ------------------------------------------------------------------------------------------

static void i_draw_solid_with_material(
                        struct csmsolid_t *solid,
                        CSMBOOL draw_debug_info, CSMBOOL draw_edge_info,
                        struct bsmaterial_t **material,
                        struct bsgraphics2_t *graphics)
{
    bsassert_not_null(material);
    
    if (solid != NULL)
    {
        if (draw_debug_info == CSMTRUE)
        {
            bsgraphics2_escr_color(graphics, *material);
            csmsolid_vis_draw_debug_info(solid, draw_edge_info, graphics);
        }
        else
        {
            CSMBOOL draw_solid_face;
            CSMBOOL draw_face_normal;
            struct bsmaterial_t *border_edges_color;
            
            draw_solid_face =  CSMTRUE; //CSMFALSE;
            draw_face_normal = CSMFALSE; //CSMFALSE;
            border_edges_color = bsmaterial_crea_rgb(0., 0., 0.);
            
            csmsolid_vis_draw(solid, draw_solid_face, draw_face_normal, *material, border_edges_color, border_edges_color, graphics);
            
            bsmaterial_destruye(&border_edges_color);
        }
        
        bsmaterial_destruye(material);
    }
}

// ------------------------------------------------------------------------------------------

static void i_draw_scene(struct csmviewer_t *viewer, struct bsgraphics2_t *graphics)
{
    struct bsgluz_abs_t *conf_iluminacion;
    
    bsassert_not_null(viewer);
    
    bsgraphics2_escr_se_dibuja_con_test_profundidad(graphics, true);
    
    conf_iluminacion = bsgbibluz_crea_luz_blanca_sin_reflejos_con_color_en_zonas_no_iluminadas();
    bsgraphics2_append_luz_puntual(graphics, -10., -10., 10., true, &conf_iluminacion);

    if (viewer->face1 != NULL || viewer->face2 != NULL)
    {
        struct bsmaterial_t *material1, *material2, *material3;
        
        if (viewer->face1 != NULL)
        {
            material1 = bsmaterial_crea_rgba(0., 1., 0., 1.);
            material2 = bsmaterial_crea_rgba(1., 0., 0., 1.);
            material3 = bsmaterial_crea_rgba(0., 0., 1., 1.);
            
            csmface_vis_draw_edges(viewer->face1, material1, material2, material3, graphics);
            
            bsmaterial_destruye(&material1);
            bsmaterial_destruye(&material2);
            bsmaterial_destruye(&material3);
        }
        
        if (viewer->face2 != NULL)
        {
            material1 = bsmaterial_crea_rgba(0.5, 1., 0., 1.);
            material2 = bsmaterial_crea_rgba(1., 0.5, 0., 1.);
            material3 = bsmaterial_crea_rgba(0.5, 0.5, 1., 1.);
            
            csmface_vis_draw_edges(viewer->face2, material1, material2, material3, graphics);
            
            bsmaterial_destruye(&material1);
            bsmaterial_destruye(&material2);
            bsmaterial_destruye(&material3);
        }
    }
    else if (viewer->solid_res1 != NULL || viewer->solid_res2 != NULL)
    {
        struct bsmaterial_t *material;
            
        material = bsmaterial_crea_rgba(0., 1., 0., 1.);
        i_draw_solid_with_material(viewer->solid_res1, CSMTRUE, CSMFALSE, &material, graphics);

        material = bsmaterial_crea_rgba(0., 0., 1., 1.);
        i_draw_solid_with_material(viewer->solid_res2, CSMTRUE, CSMFALSE, &material, graphics);
    }
    else
    {
        struct bsmaterial_t *material;
            
        material = bsmaterial_crea_rgb(0., 1., 0.);
        i_draw_solid_with_material(viewer->solid1, CSMTRUE, CSMFALSE, &material, graphics);

        material = bsmaterial_crea_rgb(0., 0., 1.);
        i_draw_solid_with_material(viewer->solid2, CSMTRUE, CSMFALSE, &material, graphics);
    }
    
    csmdebug_draw_debug_info(graphics);
}

// ------------------------------------------------------------------------------------------

static void i_show_viewer(struct csmviewer_t *viewer)
{
    struct wf3Visor_t *viewer_widget;
    struct wf3Vista_delegado_dibujo_t *delegado;
    struct wfPanel_t *panel_area, *panel_toolbar, *panel;
    
    bsassert_not_null(viewer);
    
    //viewer_widget = wf3Visor_crea_csgrenderer(500, 500, WF3VISTA_GESTION_DIBUJO_SIN_BDD);
    viewer_widget = wf3Visor_crea_csgrenderer(500, 500, WF3VISTA_GESTION_DIBUJO_BDD_OBRA_E_INMEDIATO_CAPTURAS);
    wf3Visor_set_vista_predeterminada(viewer_widget, WF3VISTA_VISTA_ISOMETRICA);
    
    delegado = wf3Vista_crea_delegado_dibujo(viewer, i_draw_scene, csmviewer_t);
    wf3Visor_set_delegado_dibujo(viewer_widget, &delegado);
    
    wf3Visor_regenera(viewer_widget, WF3VISTA_TIPO_REGENERACION_COMPLETA);
    wf3Visor_ventana_completa(viewer_widget);

    panel_area = wf3Visor_panel_area_dibujo(viewer_widget);
    panel_toolbar = wf3Visor_panel_toolbar(viewer_widget);
    panel = wf2pnutil_encapsula_en_bordes_norte_centro(panel_toolbar, panel_area);
    wf2DlgUtil_lanza_dialogo_aceptado("Solid Operation", panel, true);
    
    wf3Visor_destruye(&viewer_widget);
}

// ------------------------------------------------------------------------------------------

void csmviewer_set_parameters(struct csmviewer_t *viewer, struct csmsolid_t *solid1, struct csmsolid_t *solid2)
{
    bsassert_not_null(viewer);
    
    viewer->solid1 = solid1;
    viewer->solid2 = solid2;
    
    if (viewer->solid1 != NULL)
        csmsolid_redo_geometric_generated_data(viewer->solid1);

    if (viewer->solid2 != NULL)
        csmsolid_redo_geometric_generated_data(viewer->solid2);
}

// ------------------------------------------------------------------------------------------

void csmviewer_set_results(struct csmviewer_t *viewer, struct csmsolid_t *solid1, struct csmsolid_t *solid2)
{
    bsassert_not_null(viewer);
    
    viewer->solid_res1 = solid1;
    viewer->solid_res2 = solid2;
    
    if (viewer->solid_res1 != NULL)
        csmsolid_redo_geometric_generated_data(viewer->solid_res1);

    if (viewer->solid_res2 != NULL)
        csmsolid_redo_geometric_generated_data(viewer->solid_res2);
}

// ------------------------------------------------------------------------------------------

void csmviewer_show(struct csmviewer_t *viewer)
{
    bsassert_not_null(viewer);
    
    if (csmdebug_debug_visual_enabled() == CSMTRUE)
        i_show_viewer(viewer);
}

// ------------------------------------------------------------------------------------------

void csmviewer_show_face(struct csmviewer_t *viewer, struct csmface_t *face1, struct csmface_t *face2)
{
    bsassert_not_null(viewer);
    
    viewer->face1 = face1;
    viewer->face2 = face2;
    
    i_show_viewer(viewer);
    
    viewer->face1 = NULL;
    viewer->face2 = NULL;
}
