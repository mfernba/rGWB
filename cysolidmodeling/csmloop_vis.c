//
//  csmloop_vis.c
//  cysolidmodeling
//
//  Created by Manuel Fernandez on 14/10/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmloop_vis.inl"
#include "csmloop.tli"

#include "csmassert.inl"
#include "csmgeom.inl"
#include "csmhedge.inl"
#include "csmmath.inl"
#include "csmvertex.inl"

#include <cypearrays/a_pto2d.h>
#include <cypearrays/a_pto3d.h>
#include <geomcomp/gccontorno.h>

// ----------------------------------------------------------------------------------------------------

void csmloop_vis_append_loop_to_shape(
                        struct csmloop_t *loop,
                        double Xo, double Yo, double Zo,
                        double Ux, double Uy, double Uz, double Vx, double Vy, double Vz,
                        struct gccontorno_t *shape)
{
    struct csmhedge_t *iterator;
    unsigned long num_iteraciones;
    double Wx, Wy, Wz;
    ArrPunto2D *points;
    ArrPunto3D *points_normals;
    
    assert_no_null(loop);
    
    iterator = loop->ledge;
    num_iteraciones = 0;
    
    csmmath_cross_product3D(Ux, Uy, Uz, Vx, Vy, Vz, &Wx, &Wy, &Wz);
    
    points = arr_CreaPunto2D(0);
    points_normals = arr_CreaPunto3D(0);
    
    do
    {
        struct csmvertex_t *vertex;
        double x_3d, y_3d, z_3d;
        double Nx, Ny, Nz;
        double x_2d, y_2d;
        
        assert(num_iteraciones < 10000);
        num_iteraciones++;
        
        vertex = csmhedge_vertex(iterator);
        csmvertex_get_coordenadas(vertex, &x_3d, &y_3d, &z_3d);
        csmvertex_get_normal(vertex, &Nx, &Ny, &Nz);
        
        csmgeom_project_coords_3d_to_2d(
                        Xo, Yo, Zo,
                        Ux, Uy, Uz, Vx, Vy, Vz,
                        x_3d, y_3d, z_3d,
                        &x_2d, &y_2d);
        
        arr_AppendPunto2D(points, x_2d, y_2d);
        arr_AppendPunto3D(points_normals, -Nx, -Ny, -Nz);
        
        iterator = csmhedge_next(iterator);
        
    } while (iterator != loop->ledge);
    
    if (arr_NumElemsPunto2D(points) >= 3 && csmmath_fabs(arr_CalcularAreaPunto2D(points)) > 0.)
    {
        arr_InvertirPunto2D(points);
        arr_InvertirPunto3D(points_normals);
        gccontorno_append_array_puntos_ex(shape, &points, &points_normals);
    }
    else
    {
        arr_DestruyePunto2D(&points);
        arr_DestruyePunto3D(&points_normals);
    }
}

