//
//  csmshape3d.c
//  cysolidmodeling
//
//  Created by Manuel Fernández on 25/9/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmshape3d.h"

#include "csmeuler_lmef.inl"
#include "csmeuler_lmev.inl"
#include "csmeuler_mvfs.inl"
#include "csmmath.inl"
#include "csmdebug.inl"
#include "csmface.inl"
#include "csmhedge.inl"
#include "csmloop.inl"
#include "csmloopglue.inl"
#include "csmopbas.inl"
#include "csmsolid.h"
#include "csmsolid.inl"
#include "csmvertex.inl"

#include "cyassert.h"
#include "defmath.tlh"
#include "standarc.h"

// --------------------------------------------------------------------------------

static void i_compute_point_on_torus(
                        double R, double r, double alfa_rad, double beta_rad,
                        double *x, double *y, double *z)
{
    assert_no_null(x);
    assert_no_null(y);
    assert_no_null(z);
    
    *x = (R + r * cos(alfa_rad)) * cos(beta_rad);
    *y = (R + r * cos(alfa_rad)) * sin(beta_rad);
    *z = r * sin(alfa_rad);
}

// --------------------------------------------------------------------------------

static void i_extrude_torus_hedge(
                        double R, double r,
                        double incr_alfa_rad, double beta_rad,
                        struct csmhedge_t *hedge)
{
    struct csmvertex_t *iterator_vertex;
    unsigned long circle_point_idx;
    double alfa_rad;
    struct csmhedge_t *new_hedge;
    double x, y, z;
    
    iterator_vertex = csmhedge_vertex(hedge);
    circle_point_idx = (unsigned long)csmvertex_get_mask_attrib(iterator_vertex);
    alfa_rad = incr_alfa_rad * circle_point_idx;
    
    i_compute_point_on_torus(R, r, alfa_rad, beta_rad, &x, &y, &z);
    csmeuler_lmev_strut_edge(hedge, x, y, z, &new_hedge);
    csmvertex_set_mask_attrib(csmhedge_vertex(new_hedge), (csmvertex_mask_t)circle_point_idx);
}

// --------------------------------------------------------------------------------

struct csmsolid_t *csmshape3d_create_torus(
                        double R, double r,
                        unsigned long no_points_circle,
                        double x_center, double y_center, double z_center,
                        double Ux, double Uy, double Uz, double Vx, double Vy, double Vz,
                        unsigned long start_id_of_new_element)
{
    struct csmsolid_t *torus;
    double x, y, z;
    struct csmhedge_t *initial_hedge, *previous_hedge;
    unsigned long i;
    double incr_alfa_rad, incr_beta_rad;
    struct csmface_t *initial_face, *face_to_extrude;
    double Wx, Wy, Wz;
    
    assert(no_points_circle >= 3);
    
    i_compute_point_on_torus(R, r, 0., 0., &x, &y, &z);
    torus = csmeuler_mvfs(x, y, z, start_id_of_new_element, &initial_hedge);
    csmdebug_set_viewer_parameters(torus, NULL);
    
    incr_alfa_rad = 2. * PI / no_points_circle;
    incr_beta_rad = 2. * PI / no_points_circle;
    csmvertex_set_mask_attrib(csmhedge_vertex(initial_hedge), (csmvertex_mask_t)0);
    
    previous_hedge = initial_hedge;
    
    for (i = 1; i < no_points_circle; i++)
    {
        double alfa_rad, beta_rad;
        struct csmhedge_t *new_hedge;
     
        alfa_rad = incr_alfa_rad * i;
        beta_rad = 0.;
        
        i_compute_point_on_torus(R, r, alfa_rad, beta_rad, &x, &y, &z);
        csmeuler_lmev_strut_edge(previous_hedge, x, y, z, &new_hedge);
        previous_hedge = new_hedge;
        
        csmvertex_set_mask_attrib(csmhedge_vertex(new_hedge), (csmvertex_mask_t)i);
    }
    
    csmeuler_lmef(initial_hedge, previous_hedge, &face_to_extrude, NULL, NULL);
    initial_face = csmopbas_face_from_hedge(previous_hedge);
    
    for (i = 1; i < no_points_circle + 1; i++)
    {
        struct csmloop_t *flout;
        double beta_rad;
        struct csmhedge_t *first, *scan;
        struct csmhedge_t *scan_prev, *scan_next_next;
        
        flout = csmface_flout(face_to_extrude);
        first = csmloop_ledge(flout);
        scan = csmhedge_next(first);
        
        if (i == no_points_circle)
            beta_rad = 0.;
        else
            beta_rad = incr_beta_rad * i;
        
        i_extrude_torus_hedge(R, r, incr_alfa_rad, beta_rad, scan);
        
        while (scan != first)
        {
            struct csmhedge_t *scan_next;
            
            scan_next = csmhedge_next(scan);
            i_extrude_torus_hedge(R, r, incr_alfa_rad, beta_rad, scan_next);
            
            scan_prev = csmhedge_prev(scan);
            scan_next_next = csmhedge_next(csmhedge_next(scan));
            csmeuler_lmef(scan_prev, scan_next_next, NULL, NULL, NULL);
            
            scan = csmhedge_next(csmopbas_mate(csmhedge_next(scan)));
        }
        
        scan_prev = csmhedge_prev(scan);
        scan_next_next = csmhedge_next(csmhedge_next(scan));
        csmeuler_lmef(scan_prev, scan_next_next, NULL, NULL, NULL);
        
        face_to_extrude = csmopbas_face_from_hedge(scan_next_next);
        
        //csmsolid_print_complete_debug(torus, FALSO);
        //csmdebug_show_viewer();
    }
    
    //csmsolid_print_complete_debug(torus, FALSO);
    csmloopglue_merge_faces(initial_face, &face_to_extrude);
    //csmsolid_print_complete_debug(torus, FALSO);

    csmmath_cross_product3D(Ux, Uy, Uz, Vx, Vy, Vz, &Wx, &Wy, &Wz);
    csmsolid_general_transform(torus, Ux, Vx, Wx, 0., Uy, Vy, Wy, 0., Uz, Vz, Wz, 0.);
    csmsolid_move(torus, x_center, y_center, z_center);
    
    csmsolid_clear_algorithm_data(torus);
    
    return torus;
}










