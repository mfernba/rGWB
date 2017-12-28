//
//  csmsolid_vis.c
//  rGWB
//
//  Created by Manuel Fernandez on 14/10/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmsolid_vis.h"
#include "csmsolid.tli"

#include "csmdebug.inl"
#include "csmedge.inl"
#include "csmedge.tli"
#include "csmface.inl"
#include "csmface_vis.inl"
#include "csmhashtb.inl"
#include "csmhedge.inl"
#include "csmloop.inl"
#include "csmmath.inl"
#include "csmvertex.inl"
#include "csmassert.inl"
#include "csmstring.inl"

#include <basicGraphics/bsgraphics2.h>

// ----------------------------------------------------------------------------------------------------

static void i_draw_debug_info_vertex(struct csmvertex_t *vertex, struct bsgraphics2_t *graphics)
{
    double x, y, z;
    
    assert_no_null(vertex);
    
    csmvertex_get_coordenadas(vertex, &x, &y, &z);
    bsgraphics2_escr_punto3D(graphics, x, y, z);
    
    if (csmvertex_id(vertex) == 113)
    {
        bsgraphics2_append_desplazamiento_3D(graphics, x, y, z);
        bsgraphics2_append_ejes_plano_pantalla(graphics);
        {
            char *texto;
            
            texto = copiafor_codigo1("%lu", csmvertex_id(vertex));
            bsgraphics2_escr_texto_mts(graphics, texto, 0., 0., 1., 0., BSGRAPHICS2_JUSTIFICACION_INF_IZQ, BSGRAPHICS2_ESTILO_NORMAL, 0.0002);
            csmstring_free(&texto);
        }
        bsgraphics2_desapila_transformacion(graphics);
        bsgraphics2_desapila_transformacion(graphics);
    }
}

// ----------------------------------------------------------------------------------------------------

static struct csmface_t *i_face_from_hedge(struct csmhedge_t *hedge)
{
    struct csmloop_t *loop;
    
    loop = csmhedge_loop(hedge);
    return csmloop_lface(loop);
}

// ----------------------------------------------------------------------------------------------------

static void i_draw_debug_info_hedge(
                        struct csmhedge_t *he, struct csmhedge_t *he_next,
                        double A, double B, double C,
                        CSMBOOL is_ledge,
                        CSMBOOL draw_edge_info,
                        CSMBOOL is_null_face,
                        struct bsgraphics2_t *graphics)
{
    struct csmedge_t *edge;
    double x1, y1, z1, x2, y2, z2;
    CSMBOOL is_he_pos;
    
    csmvertex_get_coordenadas(csmhedge_vertex(he), &x1, &y1, &z1);
    csmvertex_get_coordenadas(csmhedge_vertex(he_next), &x2, &y2, &z2);
    
    edge = csmhedge_edge(he);
    
    if (csmedge_hedge_lado_const(edge, CSMEDGE_LADO_HEDGE_POS) == he)
        is_he_pos = CSMTRUE;
    else
        is_he_pos = CSMFALSE;
    
    if (is_he_pos == CSMTRUE || is_null_face == CSMTRUE)
    {
        bsgraphics2_escr_linea3D(graphics, x1, y1, z1, x2, y2, z2);
        is_he_pos = CSMTRUE;
    }
    
    if (draw_edge_info == CSMTRUE)
    {
        double length;
        double Ux, Uy, Uz;
        char *description;
        
        length = csmmath_distance_3D(x1, y1, z1, x2, y2, z2);
        csmmath_vector_between_two_3D_points(x1, y1, z1, x2, y2, z2, &Ux, &Uy, &Uz);
        
        if (length > 0.)
            csmmath_make_unit_vector3D(&Ux, &Uy, &Uz);
        
        {
            const char *txt_edge_pos;
            double Vx, Vy, Vz;
            
            csmmath_cross_product3D(A, B, C, Ux, Uy, Uz, &Vx, &Vy, &Vz);

            if (length > 0.)
            {
                bsgraphics2_append_ejes_2D(graphics, x1, y1, z1, Ux, Uy, Uz, Vx, Vy, Vz);
                bsgraphics2_escr_texto_mts(graphics, "***", 0., 0., 1., 0., BSGRAPHICS2_JUSTIFICACION_INF_IZQ, BSGRAPHICS2_ESTILO_NORMAL, 0.02);
                bsgraphics2_desapila_transformacion(graphics);
            }
           
            
            if (is_he_pos == CSMTRUE)
                txt_edge_pos = "+";
            else
                txt_edge_pos = "-";
            
            if (is_ledge == CSMTRUE)
                description = copiafor_codigo3("%s%lu->%lu", txt_edge_pos, csmhedge_id(he), csmhedge_id(he_next));
            else
                description = copiafor_codigo2("%s%lu", txt_edge_pos, csmhedge_id(he));

            bsgraphics2_append_desplazamiento_3D(graphics, .5 * (x1 + x2), .5 * (y1 + y2), .5 * (z1 + z2));
            {
                if (length > 0.)
                    bsgraphics2_append_ejes_2D(graphics, 0., 0., 0., Ux, Uy, Uz, Vx, Vy, Vz);
                else
                    bsgraphics2_append_ejes_plano_pantalla(graphics);
                
                bsgraphics2_escr_texto_mts(graphics, description, 0., 0., 1., 0., BSGRAPHICS2_JUSTIFICACION_INF_CEN, BSGRAPHICS2_ESTILO_NORMAL, 0.025);
                
                bsgraphics2_desapila_transformacion(graphics);
            }
            bsgraphics2_desapila_transformacion(graphics);
            
            csmstring_free(&description);
        }
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_draw_debug_info_faces(
                        struct csmhashtb(csmface_t) *sfaces,
                        CSMBOOL draw_edge_info,
                        struct bsgraphics2_t *graphics)
{
    struct csmhashtb_iterator(csmface_t) *iterator;
    
    iterator = csmhashtb_create_iterator(sfaces, csmface_t);
    
    while (csmhashtb_has_next(iterator, csmface_t) == CSMTRUE)
    {
        struct csmface_t *face;
        double A, B, C, D;
        CSMBOOL is_null_face;
        struct csmloop_t *floops;
        
        csmhashtb_next_pair(iterator, NULL, &face, csmface_t);
        csmface_face_equation_info(face, &A, &B, &C, &D);
        
        if (csmface_is_setop_null_face(face) == CSMTRUE)
        {
            is_null_face = CSMTRUE;
            bsgraphics2_escr_grosor_linea(graphics, BSGRAPHICS2_GROSOR_TREMENDAMENTE_GRUESO);
        }
        else
        {
            is_null_face = CSMFALSE;
            bsgraphics2_escr_grosor_linea(graphics, BSGRAPHICS2_GROSOR_NORMAL);
        }
        
        floops = csmface_floops(face);
        
        while (floops != NULL)
        {
            struct csmhedge_t *he;
            CSMBOOL is_ledge;
            
            he = csmloop_ledge(floops);
            is_ledge = CSMTRUE;
            
            do
            {
                struct csmhedge_t *he_next;
                
                he_next = csmhedge_next(he);
                i_draw_debug_info_hedge(he, he_next, A, B, C, is_ledge, draw_edge_info, is_null_face, graphics);
                
                he = he_next;
                is_ledge = CSMFALSE;
                
            } while (he != csmloop_ledge(floops));
            
            floops = csmloop_next(floops);
        }
        
        csmface_vis_draw_normal(face, graphics);
    }
    
    csmhashtb_free_iterator(&iterator, csmface_t);
}


// ----------------------------------------------------------------------------------------------------

static void i_draw_debug_info_vertexs(struct csmhashtb(csmvertex_t) *svertexs, struct bsgraphics2_t *graphics)
{
    struct csmhashtb_iterator(csmvertex_t) *iterator;
    unsigned long num_iters;
    
    iterator = csmhashtb_create_iterator(svertexs, csmvertex_t);
    num_iters = 0;
    
    while (csmhashtb_has_next(iterator, csmvertex_t) == CSMTRUE)
    {
        struct csmvertex_t *vertex;
        
        assert(num_iters < 1000000);
        num_iters++;
        
        csmhashtb_next_pair(iterator, NULL, &vertex, csmvertex_t);
        i_draw_debug_info_vertex(vertex, graphics);
    }
    
    csmhashtb_free_iterator(&iterator, csmvertex_t);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_vis_draw_debug_info(struct csmsolid_t *solido, CSMBOOL draw_edge_info, struct bsgraphics2_t *graphics)
{
    assert_no_null(solido);
    
    i_draw_debug_info_vertexs(solido->svertexs, graphics);
    i_draw_debug_info_faces(solido->sfaces, draw_edge_info, graphics);
}

// ----------------------------------------------------------------------------------------------------

static void i_draw_solid_faces(
                        struct csmhashtb(csmface_t) *sfaces,
                        CSMBOOL draw_solid_face,
                        CSMBOOL draw_face_normal,
                        const struct bsmaterial_t *face_material,
                        const struct bsmaterial_t *normal_material,
                        struct bsgraphics2_t *graphics)
{
    struct csmhashtb_iterator(csmface_t) *iterator;
    
    iterator = csmhashtb_create_iterator(sfaces, csmface_t);
    
    while (csmhashtb_has_next(iterator, csmface_t) == CSMTRUE)
    {
        struct csmface_t *face;
        
        csmhashtb_next_pair(iterator, NULL, &face, csmface_t);
        csmface_vis_draw_solid(face, draw_solid_face, draw_face_normal, face_material, normal_material, graphics);
    }
    
    csmhashtb_free_iterator(&iterator, csmface_t);
}

// ----------------------------------------------------------------------------------------------------

static void i_draw_border_edge(struct csmedge_t *edge, CSMBOOL draw_only_border_edges, struct bsgraphics2_t *graphics)
{
    struct csmhedge_t *he1, *he2;
    struct csmface_t *face_he1, *face_he2;

    he1 = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_POS);
    face_he1 = i_face_from_hedge(he1);
    
    he2 = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_NEG);
    face_he2 = i_face_from_hedge(he2);
    
    if (draw_only_border_edges == CSMFALSE || csmface_faces_define_border_edge(face_he1, face_he2) == CSMTRUE)
    {
        double x1, y1, z1, x2, y2, z2;
    
        csmedge_vertex_coordinates(edge, &x1, &y1, &z1, NULL, &x2, &y2, &z2, NULL);
        bsgraphics2_escr_linea3D(graphics, x1, y1, z1, x2, y2, z2);
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_draw_border_edges(struct csmhashtb(csmedge_t) *sedges, CSMBOOL draw_only_border_edges, struct bsgraphics2_t *graphics)
{
    struct csmhashtb_iterator(csmedge_t) *iterator;
    
    iterator = csmhashtb_create_iterator(sedges, csmedge_t);
    
    while (csmhashtb_has_next(iterator, csmedge_t) == CSMTRUE)
    {
        struct csmedge_t *edge;
        
        csmhashtb_next_pair(iterator, NULL, &edge, csmedge_t);
        i_draw_border_edge(edge, draw_only_border_edges, graphics);
    }
    
    csmhashtb_free_iterator(&iterator, csmedge_t);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_vis_draw(
                struct csmsolid_t *solido,
                CSMBOOL draw_solid_face,
                CSMBOOL draw_face_normal,
                const struct bsmaterial_t *face_material,
                const struct bsmaterial_t *normal_material,
                const struct bsmaterial_t *border_edges_material,
                struct bsgraphics2_t *graphics)
{
    assert_no_null(solido);
    
    i_draw_solid_faces(solido->sfaces, draw_solid_face, draw_face_normal, face_material, normal_material, graphics);
    
    bsgraphics2_escr_color(graphics, border_edges_material);
    i_draw_border_edges(solido->sedges, solido->draw_only_border_edges, graphics);
}
