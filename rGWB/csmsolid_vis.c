//
//  csmsolid_vis.c
//  rGWB
//
//  Created by Manuel Fernandez on 14/10/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmsolid_vis.h"
#include "csmsolid.inl"
#include "csmsolid.tli"

#include "csmedge.inl"
#include "csmedge.tli"
#include "csmface.inl"
#include "csmhashtb.inl"
#include "csmhedge.inl"
#include "csmloop.inl"
#include "csmmath.inl"

#ifdef __STANDALONE_DISTRIBUTABLE

#include "csmassert.inl"
#include "csmdebug.inl"
#include "csmstring.inl"
#include "csmvertex.inl"

#include <basicGraphics/bsgraphics2.h>

#else

#include "a_bool.h"
#include "a_pto3d.h"
#include "a_punter.h"
#include "a_ulong.h"
#include "cyassert.h"

#endif

#ifdef __STANDALONE_DISTRIBUTABLE

// ----------------------------------------------------------------------------------------------------

static void i_draw_debug_info_vertex(struct csmvertex_t *vertex, struct bsgraphics2_t *graphics)
{
    double x, y, z;
    
    assert_no_null(vertex);
    
    csmvertex_get_coords(vertex, &x, &y, &z);
    bsgraphics2_escr_punto3D(graphics, x, y, z);
    
    if (csmvertex_id(vertex) == 1881 || csmvertex_id(vertex) == 1711 || csmvertex_id(vertex) == 1833 || csmvertex_id(vertex) == 867)
    {
        bsgraphics2_append_desplazamiento_3D(graphics, x, y, z);
        bsgraphics2_append_ejes_plano_pantalla(graphics);
        {
            char *texto;
            
            texto = copiafor_codigo1("%lu", csmvertex_id(vertex));
            bsgraphics2_escr_texto_mts(graphics, texto, 0., 0., 1., 0., BSGRAPHICS2_JUSTIFICACION_INF_IZQ, BSGRAPHICS2_ESTILO_NORMAL, 0.002);
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
                        CSMBOOL force_draw_edge,
                        struct bsgraphics2_t *graphics)
{
    struct csmedge_t *edge;
    double x1, y1, z1, x2, y2, z2;
    CSMBOOL is_he_pos;
    
    csmvertex_get_coords(csmhedge_vertex(he), &x1, &y1, &z1);
    csmvertex_get_coords(csmhedge_vertex(he_next), &x2, &y2, &z2);
    
    edge = csmhedge_edge(he);
    
    if (csmedge_hedge_lado_const(edge, CSMEDGE_LADO_HEDGE_POS) == he)
        is_he_pos = CSMTRUE;
    else
        is_he_pos = CSMFALSE;
    
    if (force_draw_edge == CSMTRUE || is_he_pos == CSMTRUE || is_null_face == CSMTRUE)
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
                bsgraphics2_escr_texto_mts(graphics, "***", 0., 0., 1., 0., BSGRAPHICS2_JUSTIFICACION_INF_IZQ, BSGRAPHICS2_ESTILO_NORMAL, 0.005);
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
                
                bsgraphics2_escr_texto_mts(graphics, description, 0., 0., 1., 0., BSGRAPHICS2_JUSTIFICACION_INF_CEN, BSGRAPHICS2_ESTILO_NORMAL, 0.005);
                
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
        unsigned long face_id;
        struct csmface_t *face;
        double A, B, C, D;
        CSMBOOL is_null_face, draw_face;
        struct csmloop_t *floops;
        
        csmhashtb_next_pair(iterator, &face_id, &face, csmface_t);
        
        csmface_face_equation_info(face, &A, &B, &C, &D);
        draw_face = csmdebug_draw_face(face_id);
        
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
                struct csmedge_t *he_edge;
                struct csmhedge_t *he_next;
                
                he_edge = csmhedge_edge(he);
                he_next = csmhedge_next(he);
                
                if (csmdebug_draw_edge(csmedge_id(he_edge)) == CSMTRUE || draw_face == CSMTRUE)
                {
                    CSMBOOL force_draw_edge;
                    
                    force_draw_edge = csmdebug_get_enable_face_edge_filter();
                    i_draw_debug_info_hedge(he, he_next, A, B, C, is_ledge, draw_edge_info, is_null_face, force_draw_edge, graphics);
                }
                
                he = he_next;
                is_ledge = CSMFALSE;
                
            } while (he != csmloop_ledge(floops));
            
            floops = csmloop_next(floops);
        }
        
        if (draw_face == CSMTRUE)
            csmface_draw_normal(face, graphics);
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

static void i_draw_null_edge(struct csmedge_t *edge, struct bsgraphics2_t *graphics)
{
    if (csmedge_setop_is_null_edge(edge) == CSMTRUE)
    {
        double x1, y1, z1, x2, y2, z2;
    
        csmedge_vertex_coordinates(edge, &x1, &y1, &z1, NULL, &x2, &y2, &z2, NULL);
        
        if (csmmath_distance_3D(x1, y1, z1, x2, y2, z2) < 1.e-6)
        {
            struct csmhedge_t *he1, *he1_prev, *he1_prev_prev;
            
            he1 = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_POS);
            he1_prev = csmhedge_prev(he1);
            he1_prev_prev = csmhedge_prev(he1_prev);
            
            if (he1_prev_prev == he1)
            {
                
            }
            else
            {
                double x_prev, y_prev, z_prev;
                double Ux, Uy, Uz, length;
                double x, y, z;
                
                if (csmedge_setop_is_null_edge(csmhedge_edge(he1_prev)) == CSMTRUE)
                    he1_prev = he1_prev_prev;

                if (csmedge_setop_is_null_edge(csmhedge_edge(he1_prev)) == CSMTRUE)
                    he1_prev = csmhedge_prev(he1_prev);
                
                csmvertex_get_coords(csmhedge_vertex(he1_prev), &x_prev, &y_prev, &z_prev);

                length = csmmath_distance_3D(x1, y1, z1, x_prev, y_prev, z_prev);
                
                if (length > 0.)
                {
                    csmmath_unit_vector_between_two_3D_points(x1, y1, z1, x_prev, y_prev, z_prev, &Ux, &Uy, &Uz);
                    csmmath_move_point(x1, y1, z1, Ux, Uy, Uz, 0.25 * length, &x, &y, &z);
                
                    bsgraphics2_escr_grosor_linea(graphics, BSGRAPHICS2_GROSOR_TREMENDAMENTE_GRUESO);
                    bsgraphics2_escr_linea3D(graphics, x1, y1, z1, x, y, z);
                }
            }
        }
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_draw_null_edges(struct csmhashtb(csmedge_t) *sedges, struct bsgraphics2_t *graphics)
{
    struct csmhashtb_iterator(csmedge_t) *iterator;
    
    iterator = csmhashtb_create_iterator(sedges, csmedge_t);
    
    while (csmhashtb_has_next(iterator, csmedge_t) == CSMTRUE)
    {
        unsigned long edge_id;
        struct csmedge_t *edge;
        
        csmhashtb_next_pair(iterator, &edge_id, &edge, csmedge_t);
        
        if (csmdebug_draw_edge(edge_id) == CSMTRUE)
            i_draw_null_edge(edge, graphics);
    }
    
    csmhashtb_free_iterator(&iterator, csmedge_t);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_vis_draw_debug_info(struct csmsolid_t *solid, CSMBOOL draw_edge_info, struct bsgraphics2_t *graphics)
{
    assert_no_null(solid);
    
    i_draw_debug_info_vertexs(solid->svertexs, graphics);
    i_draw_debug_info_faces(solid->sfaces, draw_edge_info, graphics);
    i_draw_null_edges(solid->sedges, graphics);
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
        unsigned long face_id;
        struct csmface_t *face;
        
        csmhashtb_next_pair(iterator, &face_id, &face, csmface_t);
        
        if (csmdebug_draw_face(face_id) == CSMTRUE)
            csmface_draw_solid(face, draw_solid_face, draw_face_normal, face_material, normal_material, graphics);
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
        unsigned long edge_id;
        struct csmedge_t *edge;
        
        csmhashtb_next_pair(iterator, &edge_id, &edge, csmedge_t);
        
        if (csmdebug_draw_edge(edge_id) == CSMTRUE)
            i_draw_border_edge(edge, draw_only_border_edges, graphics);
    }
    
    csmhashtb_free_iterator(&iterator, csmedge_t);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_vis_draw(
                struct csmsolid_t *solid,
                CSMBOOL draw_solid_face,
                CSMBOOL draw_face_normal,
                const struct bsmaterial_t *face_material,
                const struct bsmaterial_t *normal_material,
                const struct bsmaterial_t *border_edges_material,
                struct bsgraphics2_t *graphics)
{
    assert_no_null(solid);
    
    i_draw_solid_faces(solid->sfaces, draw_solid_face, draw_face_normal, face_material, normal_material, graphics);
    
    bsgraphics2_escr_color(graphics, border_edges_material);
    i_draw_border_edges(solid->sedges, solid->draw_only_border_edges, graphics);
}

#else

// ----------------------------------------------------------------------------------------------------

static void i_genera_mesh_solido(
                        struct csmhashtb(csmface_t) *sfaces,
                        ArrPunto3D **puntos, ArrPunto3D **normales, ArrBool **es_borde,
                        ArrEnum(cplan_tipo_primitiva_t) **tipo_primitivas, ArrPuntero(ArrULong) **inds_caras)
{
    struct csmhashtb_iterator(csmface_t) *face_iterator;

    assert_no_null(puntos);
    assert_no_null(normales);
    assert_no_null(es_borde);
    assert_no_null(tipo_primitivas);
    assert_no_null(inds_caras);

    *puntos = arr_CreaPunto3D(0);
    *normales = arr_CreaPunto3D(0);
    *es_borde = arr_CreaBOOL(0);

    *tipo_primitivas = arr_CreaEnum(0, cplan_tipo_primitiva_t);
    *inds_caras = arr_CreaPunteroTD(0, ArrULong);

    face_iterator = csmhashtb_create_iterator(sfaces, csmface_t);
    
    while (csmhashtb_has_next(face_iterator, csmface_t) == CSMTRUE)
    {
        struct csmface_t *face;
        
        csmhashtb_next_pair(face_iterator, NULL, &face, csmface_t);

        csmface_append_datos_mesh(
                    face,
                    *puntos, *normales, *es_borde,
                    *tipo_primitivas, *inds_caras);
    }

    csmhashtb_free_iterator(&face_iterator, csmface_t);
}

// ----------------------------------------------------------------------------------------------------

static struct csmface_t *i_face_from_hedge(struct csmhedge_t *hedge)
{
    struct csmloop_t *loop;
    
    loop = csmhedge_loop(hedge);
    return csmloop_lface(loop);
}

// ----------------------------------------------------------------------------------------------------

static void i_append_lineas_contorno(struct csmedge_t *edge, ArrPuntero(ArrPunto3D) *lineas_contorno)
{
    struct csmhedge_t *he1, *he2;
    struct csmface_t *face_he1, *face_he2;

    he1 = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_POS);
    face_he1 = i_face_from_hedge(he1);
    
    he2 = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_NEG);
    face_he2 = i_face_from_hedge(he2);
    
    if (csmface_faces_define_border_edge(face_he1, face_he2) == CSMTRUE)
    {
        double x1, y1, z1, x2, y2, z2;
        ArrPunto3D *linea_contorno;
    
        csmedge_vertex_coordinates(edge, &x1, &y1, &z1, NULL, &x2, &y2, &z2, NULL);

        linea_contorno = arr_CreaPunto3D(2);
        arr_SetPunto3D(linea_contorno, 0, x1, y1, z1);
        arr_SetPunto3D(linea_contorno, 1, x2, y2, z2);

        arr_AppendPunteroTD(lineas_contorno, linea_contorno, ArrPunto3D);
    }
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static ArrPuntero(ArrPunto3D) *, i_lineas_contorno, (struct csmhashtb(csmedge_t) *sedges))
{
    ArrPuntero(ArrPunto3D) *lineas_contorno;
    struct csmhashtb_iterator(csmedge_t) *iterator;
    
    lineas_contorno = arr_CreaPunteroTD(0, ArrPunto3D);
    iterator = csmhashtb_create_iterator(sedges, csmedge_t);
    
    while (csmhashtb_has_next(iterator, csmedge_t) == CSMTRUE)
    {
        unsigned long edge_id;
        struct csmedge_t *edge;
        
        csmhashtb_next_pair(iterator, &edge_id, &edge, csmedge_t);
        i_append_lineas_contorno(edge, lineas_contorno);
    }
    
    csmhashtb_free_iterator(&iterator, csmedge_t);

    return lineas_contorno;
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_vis_datos_mesh(
                        struct csmsolid_t *solid, 
                        ArrPunto3D **puntos, ArrPunto3D **normales, ArrBool **es_borde,
                        ArrEnum(cplan_tipo_primitiva_t) **tipo_primitivas, ArrPuntero(ArrULong) **inds_caras,
                        ArrPuntero(ArrPunto3D) **lineas_contorno_opc)
{
    assert_no_null(solid);

    csmsolid_redo_geometric_generated_data(solid);

    i_genera_mesh_solido(
                        solid->sfaces,
                        puntos, normales, es_borde,
                        tipo_primitivas, inds_caras);

    if (lineas_contorno_opc != NULL)
        *lineas_contorno_opc = i_lineas_contorno(solid->sedges);
}

// ----------------------------------------------------------------------------------------------------

static void i_append_linea_arista(struct csmedge_t *edge, ArrPuntero(ArrPunto3D) *lineas_contorno)
{
    double x1, y1, z1, x2, y2, z2;

    csmedge_vertex_coordinates(edge, &x1, &y1, &z1, NULL, &x2, &y2, &z2, NULL);
    
    if (csmmath_distance_3D(x1, y1, z1, x2, y2, z2) > 0)
    {
        ArrPunto3D *linea_contorno;

        linea_contorno = arr_CreaPunto3D(2);
        arr_SetPunto3D(linea_contorno, 0, x1, y1, z1);
        arr_SetPunto3D(linea_contorno, 1, x2, y2, z2);

        arr_AppendPunteroTD(lineas_contorno, linea_contorno, ArrPunto3D);
    }
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static ArrPuntero(ArrPunto3D) *, i_lineas_aristas, (struct csmhashtb(csmedge_t) *sedges))
{
    ArrPuntero(ArrPunto3D) *lineas_contorno;
    struct csmhashtb_iterator(csmedge_t) *iterator;
    
    lineas_contorno = arr_CreaPunteroTD(0, ArrPunto3D);
    iterator = csmhashtb_create_iterator(sedges, csmedge_t);
    
    while (csmhashtb_has_next(iterator, csmedge_t) == CSMTRUE)
    {
        unsigned long edge_id;
        struct csmedge_t *edge;
        
        csmhashtb_next_pair(iterator, &edge_id, &edge, csmedge_t);
        i_append_linea_arista(edge, lineas_contorno);
    }
    
    csmhashtb_free_iterator(&iterator, csmedge_t);

    return lineas_contorno;
}

// ----------------------------------------------------------------------------------------------------

ArrPuntero(ArrPunto3D) *csmsolid_vis_datos_lineas(struct csmsolid_t *solid)
{
    assert_no_null(solid);

    csmsolid_redo_geometric_generated_data(solid);
    return i_lineas_aristas(solid->sedges);
}
#endif
