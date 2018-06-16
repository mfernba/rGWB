//
//  csmface_vis.c
//  rGWB
//
//  Created by Manuel Fernandez on 14/10/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmface_vis.inl"
#include "csmface.tli"

#include "csmloop.inl"
#include "csmhedge.inl"
#include "csmface.inl"
#include "csmvertex.inl"
#include "csmmath.inl"
#include "csmmath.tli"
#include "csmgeom.inl"

#ifdef __STANDALONE_DISTRIBUTABLE

#include "csmArrPoint2D.h"
#include "csmArrPoint3D.h"
#include "csmassert.inl"
#include "csmedge.inl"
#include "csmid.inl"
#include "csmmaterial.tli"
#include "csmmem.inl"
#include "csmnode.inl"
#include "csmshape2d.h"
#include "csmshape2d.inl"
#include "csmsurface.inl"
#include "csmtolerance.inl"

#include <basicSystem/bsmaterial.h>
#include <basicGraphics/bsgraphics2.h>
#else

#include "a_bool.h"
#include "a_pto2d.h"
#include "a_pto3d.h"
#include "a_punter.h"
#include "a_ulong.h"
#include "cyassert.h"
#include "cypemesh.h"
#include "ebageom.h"

#endif

#ifdef __STANDALONE_DISTRIBUTABLE

// ------------------------------------------------------------------------------------------

static struct csmhedge_t *i_hedge_mate(struct csmhedge_t *hedge)
{
    struct csmedge_t *edge;
    
    edge = csmhedge_edge(hedge);
    return csmedge_mate(edge, hedge);
}

// ----------------------------------------------------------------------------------------------------

static void i_compute_vertex_normal_for_hedge(
                        struct csmhedge_t *loop_hedge, double Wx_hedge, double Wy_hedge, double Wz_hedge,
                        const struct csmsurface_t *surface_eq,
                        double *Nx, double *Ny, double *Nz)
{
    struct csmhedge_t *iterator;
    unsigned long no_faces, no_iters;
    
    assert_no_null(Nx);
    assert_no_null(Ny);
    assert_no_null(Nz);
    
    iterator = loop_hedge;
    
    *Nx = 0.;
    *Ny = 0.;
    *Nz = 0.;
    no_faces = 0;
    no_iters = 0;
    
    do
    {
        assert(no_iters < 100);
        no_iters++;
        
        if (iterator == loop_hedge)
        {
            *Nx += Wx_hedge;
            *Ny += Wy_hedge;
            *Nz += Wz_hedge;
            no_faces++;
        }
        else
        {
            struct csmface_t *face_from_hedge;
            
            face_from_hedge = csmloop_lface(csmhedge_loop(iterator));
            assert_no_null(face_from_hedge);
            
            if (csmsurface_surfaces_define_border_edge(
                        surface_eq, Wx_hedge, Wy_hedge, Wz_hedge,
                        face_from_hedge->surface_eq, face_from_hedge->A, face_from_hedge->B, face_from_hedge->C) == CSMFALSE)
            {
                *Nx += face_from_hedge->A;
                *Ny += face_from_hedge->B;
                *Nz += face_from_hedge->C;
                
                no_faces++;
            }
        }
        
        iterator = csmhedge_next(i_hedge_mate(iterator));
    }
    while (iterator != loop_hedge);
    
    assert(no_faces > 0);
    
    *Nx /= no_faces;
    *Ny /= no_faces;
    *Nz /= no_faces;
}

// ----------------------------------------------------------------------------------------------------

static void i_vis_append_loop_to_shape(
                        struct csmloop_t *loop,
                        const struct csmsurface_t *surface_eq,
                        double Xo, double Yo, double Zo,
                        double Ux, double Uy, double Uz, double Vx, double Vy, double Vz,
                        struct csmshape2d_t *shape)
{
    struct csmhedge_t *ledge, *iterator;
    unsigned long no_iterations;
    double Wx, Wy, Wz;
    csmArrPoint2D *points;
    csmArrPoint3D *points_normals;
    
    assert_no_null(loop);
    
    ledge = csmloop_ledge(loop);
    iterator = ledge;
    no_iterations = 0;
    
    csmmath_cross_product3D(Ux, Uy, Uz, Vx, Vy, Vz, &Wx, &Wy, &Wz);
    
    points = csmArrPoint2D_new(0);
    points_normals = csmArrPoint3D_new(0);
    
    do
    {
        struct csmvertex_t *vertex;
        double x_3d, y_3d, z_3d;
        double Nx, Ny, Nz;
        double x_2d, y_2d;
        
        assert(no_iterations < 10000);
        no_iterations++;
        
        vertex = csmhedge_vertex(iterator);
        csmvertex_get_coords(vertex, &x_3d, &y_3d, &z_3d);
        
        i_compute_vertex_normal_for_hedge(
                        iterator, Wx, Wy, Wz,
                        surface_eq,
                        &Nx, &Ny, &Nz);
        
        csmgeom_project_coords_3d_to_2d(
                        Xo, Yo, Zo,
                        Ux, Uy, Uz, Vx, Vy, Vz,
                        x_3d, y_3d, z_3d,
                        &x_2d, &y_2d);
        
        csmArrPoint2D_append(points, x_2d, y_2d);
        csmArrPoint3D_append(points_normals, -Nx, -Ny, -Nz);
        
        iterator = csmhedge_next(iterator);
        
    } while (iterator != ledge);
    
    if (csmArrPoint2D_count(points) >= 3 && csmmath_fabs(csmArrPoint2D_area(points)) > 0.)
    {
        csmArrPoint2D_invert(points);
        csmArrPoint3D_invert(points_normals);
        csmshape2d_append_new_polygon_with_points_and_normals(shape, &points, &points_normals);
    }
    else
    {
        csmArrPoint2D_free(&points);
        csmArrPoint3D_free(&points_normals);
    }
}

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
        struct csmshape2d_t *shape;
    
        csmmath_plane_axis_from_implicit_plane_equation(
                        face->A, face->B, face->C, face->D,
                        &Xo, &Yo, &Zo,
                        &Ux, &Uy, &Uz, &Vx, &Vy, &Vz);
    
        loop_iterator = csmface_floops(face);
        shape = csmshape2d_new();
        
        while (loop_iterator != NULL)
        {
            i_vis_append_loop_to_shape(loop_iterator, face->surface_eq, Xo, Yo, Zo, Ux, Uy, Uz, Vx, Vy, Vz, shape);
            loop_iterator = csmloop_next(loop_iterator);
        }
        
        if (csmshape2d_polygon_count(shape) > 0)
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
            
            csmshape2d_draw_3D(shape, Xo, Yo, Zo, Ux, Uy, Uz, Vx, Vy, Vz, CSMFALSE, graphics);
        }
        
        csmshape2d_free(&shape);
    }

    if (draw_face_normal == CSMTRUE)
    {
        double x_geometric_center, y_geometric_center, z_geometric_center;
        double disp;
        
        bsgraphics2_escr_color(graphics, normal_material);
        csmloop_geometric_center_3d(face->flout, &x_geometric_center, &y_geometric_center, &z_geometric_center);
    
        disp = 0.005;
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
    
    disp = 0.005;
    bsgraphics2_escr_linea3D(
                        graphics,
                        x_geometric_center, y_geometric_center, z_geometric_center,
                        x_geometric_center + disp * face->A, y_geometric_center + disp * face->B, z_geometric_center + disp * face->C);
}

// ----------------------------------------------------------------------------------------------------

void csmface_vis_draw_edges(
                        struct csmface_t *face,
                        const struct bsmaterial_t *outer_loop,
                        const struct bsmaterial_t *hole_loop,
                        const struct bsmaterial_t *inner_non_hole_loop,
                        struct bsgraphics2_t *graphics)
{
    double Xo, Yo, Zo, Ux, Uy, Uz, Vx, Vy, Vz;
    struct csmloop_t *loop_iterator;
    
    assert_no_null(face);
    
    csmmath_plane_axis_from_implicit_plane_equation(
                        face->A, face->B, face->C, face->D,
                        &Xo, &Yo, &Zo,
                        &Ux, &Uy, &Uz, &Vx, &Vy, &Vz);
    
    loop_iterator = csmface_floops(face);

    while (loop_iterator != NULL)
    {
        double loop_area;
        struct csmhedge_t *he, *lhedge;
        
        loop_area = csmloop_compute_area(loop_iterator, Xo, Yo, Zo, Ux, Uy, Uz, Vx, Vy, Vz);
        
        if (loop_iterator == face->flout)
        {
            assert(loop_area >= 0.);
            bsgraphics2_escr_color(graphics, outer_loop);
        }
        else
        {
            if (loop_area < 0.)
                bsgraphics2_escr_color(graphics, hole_loop);
            else
                bsgraphics2_escr_color(graphics, inner_non_hole_loop);
        }
        
        lhedge = csmloop_ledge(loop_iterator);
        he = lhedge;
        
        do
        {
            struct csmedge_t *edge;
            
            edge = csmhedge_edge(he);
            
            if (edge != NULL)
            {
                double x1, y1, z1, x2, y2, z2;
                
                csmedge_vertex_coordinates(edge, &x1, &y1, &z1, NULL, &x2, &y2, &z2, NULL);
                bsgraphics2_escr_linea3D(graphics, x1, y1, z1, x2, y2, z2);
            }
            
            he = csmhedge_next(he);
        }
        while (he != lhedge);
        
        loop_iterator = csmloop_next(loop_iterator);
    }
}

#else

// ----------------------------------------------------------------------------------------------------

static void i_vis_append_loop_to_shape(
                        struct csmloop_t *loop,
                        double Xo, double Yo, double Zo,
                        double Ux, double Uy, double Uz, double Vx, double Vy, double Vz,
                        ArrPuntero(ArrPunto3D) *poligonos_3d_cara)
{
    struct csmhedge_t *ledge, *iterator;
    unsigned long no_iterations;
    ArrPunto3D *points;
    ArrPunto2D *points_2d;
    
    assert_no_null(loop);
    
    ledge = csmloop_ledge(loop);
    iterator = ledge;
    no_iterations = 0;
    
    points = arr_CreaPunto3D(0);
    points_2d = arr_CreaPunto2D(0);
    
    do
    {
        struct csmvertex_t *vertex;
        double x_3d, y_3d, z_3d;
        double x_2d, y_2d;
        
        assert(no_iterations < 10000);
        no_iterations++;
        
        vertex = csmhedge_vertex(iterator);
        csmvertex_get_coords(vertex, &x_3d, &y_3d, &z_3d);

        arr_AppendPunto3D(points, x_3d, y_3d, z_3d);
        
        csmgeom_project_coords_3d_to_2d(
                        Xo, Yo, Zo,
                        Ux, Uy, Uz, Vx, Vy, Vz,
                        x_3d, y_3d, z_3d,
                        &x_2d, &y_2d);

        arr_AppendPunto2D(points_2d, x_2d, y_2d);
        
        iterator = csmhedge_next(iterator);
        
    } while (iterator != ledge);
    
    if (arr_NumElemsPunto3D(points) >= 3 && csmmath_fabs(arr_CalcularAreaPunto2D(points_2d)) > 0.)
    {
        ebageom_invierte_puntos3D(points);
        arr_AppendPunteroTD(poligonos_3d_cara, points, ArrPunto3D);
    }
    else
    {
        arr_DestruyePunto3D(&points);
    }

    arr_DestruyePunto2D(&points_2d);
}

// ----------------------------------------------------------------------------------------------------

static void i_desplaza_indices_vertices(ArrULong *indices_vertices, unsigned long num_vertices_solido)
{
    unsigned long i, num_vertices;

    num_vertices = arr_NumElemsULong(indices_vertices);

    for (i = 0; i < num_vertices; i++)
    {
        unsigned long indice;

        indice = arr_GetULong(indices_vertices, i);
        arr_SetULong(indices_vertices, i, indice + num_vertices_solido);
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_desplaza_array_indices_vertices(ArrPuntero(ArrULong) *inds_caras, unsigned long num_vertices_solido)
{
    unsigned long i, num_caras;

    num_caras = arr_NumElemsPunteroTD(inds_caras, ArrULong);

    for (i = 0; i < num_caras; i++)
    {
        ArrULong *indices_vertices_cara;

        indices_vertices_cara = arr_GetPunteroTD(inds_caras, i, ArrULong);
        i_desplaza_indices_vertices(indices_vertices_cara, num_vertices_solido);
    }
}

// ----------------------------------------------------------------------------------------------------

void csmface_vis_append_datos_mesh(
                    struct csmface_t *face,
                    ArrPunto3D *puntos, ArrPunto3D *normales, ArrBool *es_borde,
                    ArrEnum(cplan_tipo_primitiva_t) *tipo_primitivas, ArrPuntero(ArrULong) *inds_caras)
{
    double Xo, Yo, Zo, Ux, Uy, Uz, Vx, Vy, Vz;
    struct csmloop_t *flout, *loop_iterator;
    ArrPuntero(ArrPunto3D) *poligonos_3d_cara;
    
    assert_no_null(face);

    csmmath_plane_axis_from_implicit_plane_equation(
                        face->A, face->B, face->C, face->D,
                        &Xo, &Yo, &Zo,
                        &Ux, &Uy, &Uz, &Vx, &Vy, &Vz);

    poligonos_3d_cara = arr_CreaPunteroTD(0, ArrPunto3D);

    flout = csmface_flout(face);
    i_vis_append_loop_to_shape(flout, Xo, Yo, Zo, Ux, Uy, Uz, Vx, Vy, Vz, poligonos_3d_cara);

    loop_iterator = csmface_floops(face);
        
    while (loop_iterator != NULL)
    {
        if (loop_iterator != flout)
            i_vis_append_loop_to_shape(loop_iterator, Xo, Yo, Zo, Ux, Uy, Uz, Vx, Vy, Vz, poligonos_3d_cara);

        loop_iterator = csmloop_next(loop_iterator);
    }
        
    if (arr_NumElemsPunteroTD(poligonos_3d_cara, ArrPunto3D) > 0)
    {
        unsigned long num_vertices_solido;
        ArrPunto3D *puntos_cara, *normales_cara;
        ArrBool *es_borde_cara;
        ArrEnum(cplan_tipo_primitiva_t) *tipo_primitivas_cara;
        ArrPuntero(ArrULong) *inds_caras_cara;

        num_vertices_solido = arr_NumElemsPunto3D(puntos);
        assert(num_vertices_solido == arr_NumElemsPunto3D(normales));
        assert(num_vertices_solido == arr_NumElemsBOOL(es_borde));

        cypemesh_genera_contorno(
                        poligonos_3d_cara,
                        &puntos_cara, &normales_cara, &es_borde_cara,
                        &tipo_primitivas_cara,
                        &inds_caras_cara);

        i_desplaza_array_indices_vertices(inds_caras_cara, num_vertices_solido);

        arr_ConcatenaPunto3D(puntos, puntos_cara);
        arr_ConcatenaPunto3D(normales, normales_cara);
        arr_ConcatenaBOOL(es_borde, es_borde_cara);
        arr_ConcatenaEnum(tipo_primitivas, tipo_primitivas_cara, cplan_tipo_primitiva_t);
        arr_ConcatenaEstructurasTD(inds_caras, inds_caras_cara, ArrULong, NULL);

        arr_DestruyePunto3D(&puntos_cara);
        arr_DestruyePunto3D(&normales_cara);
        arr_DestruyeBOOL(&es_borde_cara);
        arr_DestruyeEnum(&tipo_primitivas_cara, cplan_tipo_primitiva_t);
        arr_DestruyeEstructurasTD(&inds_caras_cara, ArrULong, NULL);
    }

    arr_DestruyeEstructurasTD(&poligonos_3d_cara, ArrPunto3D, arr_DestruyePunto3D);
}

#endif
