//
//  csmloop_vis.c
//  rGWB
//
//  Created by Manuel Fernandez on 14/10/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmloop_vis.inl"
#include "csmloop.tli"

#include "csmassert.inl"
#include "csmArrPoint2D.h"
#include "csmArrPoint3D.h"
#include "csmgeom.inl"
#include "csmhedge.inl"
#include "csmmath.inl"
#include "csmvertex.inl"
#include "csmshape2d.h"

// ----------------------------------------------------------------------------------------------------

void csmloop_vis_append_loop_to_shape(
                        struct csmloop_t *loop,
                        double Xo, double Yo, double Zo,
                        double Ux, double Uy, double Uz, double Vx, double Vy, double Vz,
                        struct csmshape2d_t *shape)
{
    struct csmhedge_t *iterator;
    unsigned long num_iteraciones;
    double Wx, Wy, Wz;
    csmArrPoint2D *points;
    csmArrPoint3D *points_normals;
    
    assert_no_null(loop);
    
    iterator = loop->ledge;
    num_iteraciones = 0;
    
    csmmath_cross_product3D(Ux, Uy, Uz, Vx, Vy, Vz, &Wx, &Wy, &Wz);
    
    points = csmArrPoint2D_new(0);
    points_normals = csmArrPoint3D_new(0);
    
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
        
        csmArrPoint2D_append(points, x_2d, y_2d);
        csmArrPoint3D_append(points_normals, -Nx, -Ny, -Nz);
        
        iterator = csmhedge_next(iterator);
        
    } while (iterator != loop->ledge);
    
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

