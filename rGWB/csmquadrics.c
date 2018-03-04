//
//  csmquadrics.c
//  rGWB
//
//  Created by Manuel Fernández on 25/9/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmquadrics.h"

#include "csmassert.inl"
#include "csmeuler_lkev.inl"
#include "csmeuler_lmef.inl"
#include "csmeuler_lmev.inl"
#include "csmeuler_mvfs.inl"
#include "csmface.inl"
#include "csmhashtb.inl"
#include "csmhedge.inl"
#include "csmloop.inl"
#include "csmloopglue.inl"
#include "csmmath.inl"
#include "csmmath.tli"
#include "csmmem.inl"
#include "csmopbas.inl"
#include "csmsolid.h"
#include "csmsolid.inl"
#include "csmsurface.inl"
#include "csmtolerance.inl"
#include "csmvertex.inl"

// --------------------------------------------------------------------------------

static void i_assign_face_eq_to_solid_faces(struct csmsolid_t *solid, struct csmsurface_t **surface)
{
    struct csmsurface_t *surface_loc;
    struct csmhashtb_iterator(csmface_t) *face_iterator;
    
    surface_loc = ASIGNA_PUNTERO_PP_NO_NULL(surface, struct csmsurface_t);
    face_iterator = csmsolid_face_iterator(solid);
    
    while (csmhashtb_has_next(face_iterator, csmface_t) == CSMTRUE)
    {
        struct csmface_t *face;
        
        csmhashtb_next_pair(face_iterator, NULL, &face, csmface_t);
        csmface_set_surface_eq(face, surface_loc);
    }
    
    csmhashtb_free_iterator(&face_iterator, csmface_t);
    
    csmsurface_free(&surface_loc);
}

// --------------------------------------------------------------------------------

static void i_compute_point_on_torus(
                        double R, double r, double alfa_rad, double beta_rad,
                        double *x, double *y, double *z)
{
    assert(R > 0.);
    assert(r > 0.);
    assert_no_null(x);
    assert_no_null(y);
    assert_no_null(z);
    
    *x = (R + r * csmmath_cos(alfa_rad)) * csmmath_cos(beta_rad);
    *y = (R + r * csmmath_cos(alfa_rad)) * csmmath_sin(beta_rad);
    *z = r * csmmath_sin(alfa_rad);
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

static void i_apply_transform_to_solid(
                        double x_center, double y_center, double z_center,
                        double Ux, double Uy, double Uz, double Vx, double Vy, double Vz,
                        struct csmsolid_t *solid)
{
    double Wx, Wy, Wz;
    
    csmmath_cross_product3D(Ux, Uy, Uz, Vx, Vy, Vz, &Wx, &Wy, &Wz);
    csmsolid_general_transform(solid, Ux, Vx, Wx, 0., Uy, Vy, Wy, 0., Uz, Vz, Wz, 0.);
    csmsolid_move(solid, x_center, y_center, z_center);
}

// --------------------------------------------------------------------------------

struct csmsolid_t *csmquadrics_create_torus(
                        double R, unsigned long no_points_circle_R,
                        double r, unsigned long no_points_circle_r,
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
    struct csmtolerance_t *tolerances;
    
    assert(no_points_circle_R >= 3);
    assert(no_points_circle_r >= 3);
    
    i_compute_point_on_torus(R, r, 0., 0., &x, &y, &z);
    torus = csmeuler_mvfs(x, y, z, start_id_of_new_element, &initial_hedge);
    //csmsolid_set_draw_only_border_edges(torus, CSMFALSE);    
    
    incr_alfa_rad = 2. * CSMMATH_PI / no_points_circle_r;
    incr_beta_rad = 2. * CSMMATH_PI / no_points_circle_R;
    csmvertex_set_mask_attrib(csmhedge_vertex(initial_hedge), (csmvertex_mask_t)0);
    
    tolerances = csmtolerance_new();
    
    previous_hedge = initial_hedge;
    
    for (i = 1; i < no_points_circle_r; i++)
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
    
    for (i = 1; i < no_points_circle_R + 1; i++)
    {
        struct csmloop_t *flout;
        double beta_rad;
        struct csmhedge_t *first, *scan;
        struct csmhedge_t *scan_prev, *scan_next_next;
        
        flout = csmface_flout(face_to_extrude);
        first = csmloop_ledge(flout);
        scan = csmhedge_next(first);
        
        if (i == no_points_circle_R)
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
    }
    
    csmloopglue_merge_faces(initial_face, &face_to_extrude, tolerances);

    i_apply_transform_to_solid(
                        x_center, y_center, z_center,
                        Ux, Uy, Uz, Vx, Vy, Vz,
                        torus);
    
    {
        struct csmsurface_t *surface;
        
        surface = csmsurface_new_torus(R, r);
        i_assign_face_eq_to_solid_faces(torus, &surface);
    }
    
    csmsolid_redo_geometric_face_data(torus);
    
    csmtolerance_free(&tolerances);
    
    return torus;
}

// --------------------------------------------------------------------------------

static void i_compute_point_on_cone_base(
                        double r_x, double r_y, double beta_rad,
                        double *x, double *y, double *z)
{
    assert(r_x > 0.);
    assert(r_y > 0.);
    assert_no_null(x);
    assert_no_null(y);
    assert_no_null(z);
    
    *x = r_x * csmmath_cos(beta_rad);
    *y = r_y * csmmath_sin(beta_rad);
    *z = 0.;
}


// --------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmsolid_t *, i_create_ellipsoid_base_solid, (
                        double radius_x, double radius_y,
                        unsigned long no_points_circle_radius,
                        unsigned long start_id_of_new_element,
                        struct csmface_t **bottom_face_opt, struct csmface_t **top_face_opt))
{
    struct csmsolid_t *circle;
    double x, y, z;
    struct csmhedge_t *initial_hedge, *previous_hedge;
    unsigned long i;
    double incr_beta_rad;
    struct csmface_t *bottom_face_loc;
    
    assert(radius_x > 0.);
    assert(radius_y > 0.);
    assert(no_points_circle_radius >= 3);

    i_compute_point_on_cone_base(radius_x, radius_y, 0., &x, &y, &z);
    circle = csmeuler_mvfs(x, y, z, start_id_of_new_element, &initial_hedge);
    bottom_face_loc = csmopbas_face_from_hedge(initial_hedge);
    
    incr_beta_rad = 2. * CSMMATH_PI / no_points_circle_radius;
    
    previous_hedge = initial_hedge;
    csmvertex_set_mask_attrib(csmhedge_vertex(initial_hedge), (csmvertex_mask_t)0);
    
    for (i = 1; i < no_points_circle_radius; i++)
    {
        double beta_rad;
     
        beta_rad = 2. * CSMMATH_PI - incr_beta_rad * i;
        i_compute_point_on_cone_base(radius_x, radius_y, beta_rad, &x, &y, &z);
        
        csmeuler_lmev_strut_edge(previous_hedge, x, y, z, &previous_hedge);
        csmvertex_set_mask_attrib(csmhedge_vertex(previous_hedge), (csmvertex_mask_t)i);
    }
    
    csmeuler_lmef(initial_hedge, previous_hedge, top_face_opt, NULL, NULL);
    
    ASSIGN_OPTIONAL_VALUE(bottom_face_opt, bottom_face_loc);
    
    return circle;
}

// --------------------------------------------------------------------------------

static void i_connect_face_hegdes_with_vertex(
                        struct csmface_t *face,
                        double x, double y, double z)
{
    struct csmloop_t *face_flout;
    struct csmhedge_t *first, *scan;
    struct csmhedge_t *scan_prev, *scan_next_next;
    struct csmvertex_t *top_vertex;
    
    face_flout = csmface_flout(face);
    first = csmloop_ledge(face_flout);
    scan = csmhedge_next(first);
    
    csmeuler_lmev(scan, scan, x, y, z, &top_vertex, NULL, NULL, NULL);
    
    while (scan != first)
    {
        struct csmhedge_t *scan_next;
        struct csmhedge_t *new_he_pos, *new_he_neg;
        
        scan_next = csmhedge_next(scan);
        csmeuler_lmev_strut_edge(scan_next, x, y, z, NULL);
        
        scan_prev = csmhedge_prev(scan);
        scan_next_next = csmhedge_next(csmhedge_next(scan));
        csmeuler_lmef(scan_prev, scan_next_next, NULL, &new_he_pos, &new_he_neg);        
        csmeuler_lkev(&new_he_neg, &new_he_pos, NULL, NULL, NULL, NULL);
        
        scan = csmhedge_next(csmopbas_mate(csmhedge_next(scan)));
    }
}
                        
// --------------------------------------------------------------------------------

struct csmsolid_t *csmquadrics_create_cone(
                        double height, double radius, unsigned long no_points_circle_radius,
                        double x_base_center, double y_base_center, double z_base_center,
                        double Ux, double Uy, double Uz, double Vx, double Vy, double Vz,
                        unsigned long start_id_of_new_element)
{
    struct csmsolid_t *cone;
    struct csmface_t *top_face;
    
    assert(height > 0.);

    cone = i_create_ellipsoid_base_solid(radius, radius, no_points_circle_radius, start_id_of_new_element, NULL, &top_face);
    i_connect_face_hegdes_with_vertex(top_face, 0., 0., height);

    i_apply_transform_to_solid(
                        x_base_center, y_base_center, z_base_center,
                        Ux, Uy, Uz, Vx, Vy, Vz,
                        cone);
    
    {
        struct csmsurface_t *surface;
        
        surface = csmsurface_new_cone(height, radius);
        i_assign_face_eq_to_solid_faces(cone, &surface);
    }
    
    csmsolid_clear_algorithm_data(cone);
    
    return cone;
}

// --------------------------------------------------------------------------------

static void i_extrude_ellipsoid_hedge(
                        double rx_sphere, double ry_sphere, double rz_sphere,
                        unsigned long no_points_circle_radius_meridians,
                        double direction_sign,
                        double alfa_rad,
                        struct csmhedge_t *hedge)
{
    struct csmvertex_t *iterator_vertex;
    unsigned long circle_point_idx;
    double incr_beta_rad, beta_rad;
    struct csmhedge_t *new_hedge;
    double x, y, z;
    
    assert(rx_sphere > 0.);
    assert(ry_sphere > 0.);
    assert(rz_sphere > 0.);
    assert(no_points_circle_radius_meridians >= 3);
    
    iterator_vertex = csmhedge_vertex(hedge);
    circle_point_idx = (unsigned long)csmvertex_get_mask_attrib(iterator_vertex);
    
    incr_beta_rad = 2. * CSMMATH_PI / no_points_circle_radius_meridians;
    beta_rad = 2. * CSMMATH_PI - incr_beta_rad * circle_point_idx;
    
    x = rx_sphere * csmmath_cos(beta_rad) * csmmath_cos(alfa_rad);
    y = ry_sphere * csmmath_sin(beta_rad) * csmmath_cos(alfa_rad);
    z = direction_sign * rz_sphere * csmmath_sin(alfa_rad);
    
    csmeuler_lmev_strut_edge(hedge, x, y, z, &new_hedge);
    csmvertex_set_mask_attrib(csmhedge_vertex(new_hedge), (csmvertex_mask_t)circle_point_idx);
}

// --------------------------------------------------------------------------------

static void i_generate_half_ellipsoid(
                        struct csmface_t *initial_face,
                        double direction_sign,
                        double radius_x, double radius_y, double radius_z,
                        unsigned long no_points_circle_radius_parallels_semisphere,
                        unsigned long no_points_circle_radius_meridians)
{
    unsigned long i, no_homothetic_circles;
    double incr_alfa;
    struct csmface_t *face_to_extrude;
    
    assert(no_points_circle_radius_parallels_semisphere >= 3);
    
    no_homothetic_circles = no_points_circle_radius_parallels_semisphere - 1;
    incr_alfa = 0.5 * CSMMATH_PI / no_points_circle_radius_parallels_semisphere;
    
    face_to_extrude = initial_face;
    
    for (i = 0; i < no_homothetic_circles; i++)
    {
        struct csmloop_t *flout;
        struct csmhedge_t *first, *scan;
        struct csmhedge_t *scan_prev, *scan_next_next;
        double alfa_rad;
        
        flout = csmface_flout(face_to_extrude);
        first = csmloop_ledge(flout);
        scan = csmhedge_next(first);
        
        alfa_rad = (i + 1) * incr_alfa;
        i_extrude_ellipsoid_hedge(radius_x, radius_y, radius_z, no_points_circle_radius_meridians, direction_sign, alfa_rad, scan);
        
        while (scan != first)
        {
            struct csmhedge_t *scan_next;
            
            scan_next = csmhedge_next(scan);
            i_extrude_ellipsoid_hedge(radius_x, radius_y, radius_z, no_points_circle_radius_meridians, direction_sign, alfa_rad, scan_next);
            
            scan_prev = csmhedge_prev(scan);
            scan_next_next = csmhedge_next(csmhedge_next(scan));
            csmeuler_lmef(scan_prev, scan_next_next, NULL, NULL, NULL);
            
            scan = csmhedge_next(csmopbas_mate(csmhedge_next(scan)));
        }
        
        scan_prev = csmhedge_prev(scan);
        scan_next_next = csmhedge_next(csmhedge_next(scan));
        csmeuler_lmef(scan_prev, scan_next_next, NULL, NULL, NULL);
        
        face_to_extrude = csmopbas_face_from_hedge(scan_next_next);
    }
    
    i_connect_face_hegdes_with_vertex(face_to_extrude, 0., 0., direction_sign * radius_z);
}

// --------------------------------------------------------------------------------

struct csmsolid_t *csmquadrics_create_sphere(
                        double radius,
                        unsigned long no_points_circle_radius_parallels_semisphere,
                        unsigned long no_points_circle_radius_meridians,
                        double x_center, double y_center, double z_center,
                        double Ux, double Uy, double Uz, double Vx, double Vy, double Vz,
                        unsigned long start_id_of_new_element)
{
    struct csmsolid_t *sphere;
    struct csmface_t *bottom_face, *top_face;
    double direction_sign;
    
    sphere = i_create_ellipsoid_base_solid(radius, radius, no_points_circle_radius_meridians, start_id_of_new_element, &bottom_face, &top_face);
    //csmsolid_set_draw_only_border_edges(sphere, CSMFALSE);
    
    direction_sign = 1.;
    i_generate_half_ellipsoid(top_face, direction_sign, radius, radius, radius, no_points_circle_radius_parallels_semisphere, no_points_circle_radius_meridians);

    direction_sign = -1.;
    i_generate_half_ellipsoid(bottom_face, direction_sign, radius, radius, radius, no_points_circle_radius_parallels_semisphere, no_points_circle_radius_meridians);
    
    i_apply_transform_to_solid(
                        x_center, y_center, z_center,
                        Ux, Uy, Uz, Vx, Vy, Vz,
                        sphere);

    {
        struct csmsurface_t *surface;
        
        surface = csmsurface_new_ellipsoid(radius, radius, radius);
        i_assign_face_eq_to_solid_faces(sphere, &surface);
    }
    
    csmsolid_redo_geometric_face_data(sphere);
    
    return sphere;
}

// --------------------------------------------------------------------------------

struct csmsolid_t *csmquadrics_create_ellipsoid(
                        double radius_x, double radius_y, double radius_z,
                        unsigned long no_points_circle_radius_parallels_semisphere,
                        unsigned long no_points_circle_radius_meridians,
                        double x_center, double y_center, double z_center,
                        double Ux, double Uy, double Uz, double Vx, double Vy, double Vz,
                        unsigned long start_id_of_new_element)
{
    struct csmsolid_t *ellipsoid;
    struct csmface_t *bottom_face, *top_face;
    double direction_sign;
    
    ellipsoid = i_create_ellipsoid_base_solid(radius_x, radius_y, no_points_circle_radius_meridians, start_id_of_new_element, &bottom_face, &top_face);
    //csmsolid_set_draw_only_border_edges(ellipsoid, CSMFALSE);
    
    direction_sign = 1.;
    i_generate_half_ellipsoid(top_face, direction_sign, radius_x, radius_y, radius_z, no_points_circle_radius_parallels_semisphere, no_points_circle_radius_meridians);

    direction_sign = -1.;
    i_generate_half_ellipsoid(bottom_face, direction_sign, radius_x, radius_y, radius_z, no_points_circle_radius_parallels_semisphere, no_points_circle_radius_meridians);
    
    i_apply_transform_to_solid(
                        x_center, y_center, z_center,
                        Ux, Uy, Uz, Vx, Vy, Vz,
                        ellipsoid);

    {
        struct csmsurface_t *surface;
        
        surface = csmsurface_new_ellipsoid(radius_x, radius_y, radius_z);
        i_assign_face_eq_to_solid_faces(ellipsoid, &surface);
    }
    
    csmsolid_redo_geometric_face_data(ellipsoid);
    
    return ellipsoid;
}

// --------------------------------------------------------------------------------

static void i_extrude_hyperbolid_hedge(
                        double a, double c, double coord_u,
                        unsigned long no_points_circle_radius_meridians,
                        double direction_sign,
                        struct csmhedge_t *hedge)
{
    struct csmvertex_t *iterator_vertex;
    unsigned long circle_point_idx;
    double incr_beta_rad, beta_rad;
    double common;
    struct csmhedge_t *new_hedge;
    double x, y, z;
    
    assert(no_points_circle_radius_meridians >= 3);
    
    iterator_vertex = csmhedge_vertex(hedge);
    circle_point_idx = (unsigned long)csmvertex_get_mask_attrib(iterator_vertex);
    
    incr_beta_rad = 2. * CSMMATH_PI / no_points_circle_radius_meridians;
    beta_rad = 2. * CSMMATH_PI - incr_beta_rad * circle_point_idx;
    
    common = a * csmmath_sqrt(1 + coord_u * coord_u);
    
    x = common * csmmath_cos(beta_rad);
    y = common * csmmath_sin(beta_rad);
    z = direction_sign * c * coord_u;
    
    csmeuler_lmev_strut_edge(hedge, x, y, z, &new_hedge);
    csmvertex_set_mask_attrib(csmhedge_vertex(new_hedge), (csmvertex_mask_t)circle_point_idx);
}

// --------------------------------------------------------------------------------

static void i_generate_half_hiperboloid(
                        struct csmface_t *initial_face,
                        double direction_sign,
                        double a, double c, double height,
                        unsigned long no_points_circle_radius_parallels_semisphere,
                        unsigned long no_points_circle_radius_meridians)
{
    unsigned long i, no_homothetic_circles;
    double incr_height;
    struct csmface_t *face_to_extrude;
    
    assert(no_points_circle_radius_parallels_semisphere >= 3);
    assert(height > 0.);
    
    no_homothetic_circles = no_points_circle_radius_parallels_semisphere;
    incr_height = height / no_points_circle_radius_parallels_semisphere;
    
    face_to_extrude = initial_face;
    
    for (i = 0; i < no_homothetic_circles; i++)
    {
        struct csmloop_t *flout;
        struct csmhedge_t *first, *scan;
        struct csmhedge_t *scan_prev, *scan_next_next;
        double coord_u;
        
        flout = csmface_flout(face_to_extrude);
        first = csmloop_ledge(flout);
        scan = csmhedge_next(first);
        
        coord_u = (i + 1) * incr_height;
        i_extrude_hyperbolid_hedge(a, c, coord_u, no_points_circle_radius_meridians, direction_sign, scan);
        
        while (scan != first)
        {
            struct csmhedge_t *scan_next;
            
            scan_next = csmhedge_next(scan);
            i_extrude_hyperbolid_hedge(a, c, coord_u, no_points_circle_radius_meridians, direction_sign, scan_next);
            
            scan_prev = csmhedge_prev(scan);
            scan_next_next = csmhedge_next(csmhedge_next(scan));
            csmeuler_lmef(scan_prev, scan_next_next, NULL, NULL, NULL);
            
            scan = csmhedge_next(csmopbas_mate(csmhedge_next(scan)));
        }
        
        scan_prev = csmhedge_prev(scan);
        scan_next_next = csmhedge_next(csmhedge_next(scan));
        csmeuler_lmef(scan_prev, scan_next_next, NULL, NULL, NULL);
        
        face_to_extrude = csmopbas_face_from_hedge(scan_next_next);
    }
    
    i_connect_face_hegdes_with_vertex(face_to_extrude, 0., 0., direction_sign * height * c);
}

// --------------------------------------------------------------------------------

struct csmsolid_t *csmquadrics_create_hyperbolid_one_sheet(
                        double a, double c, double height,
                        unsigned long no_points_circle_radius_parallels_semisphere,
                        unsigned long no_points_circle_radius_meridians,
                        double x_center, double y_center, double z_center,
                        double Ux, double Uy, double Uz, double Vx, double Vy, double Vz,
                        unsigned long start_id_of_new_element)
{
    struct csmsolid_t *hyperboloid;
    struct csmface_t *bottom_face, *top_face;
    double direction_sign;
    
    hyperboloid = i_create_ellipsoid_base_solid(a, a, no_points_circle_radius_meridians, start_id_of_new_element, &bottom_face, &top_face);
    //csmsolid_set_draw_only_border_edges(hyperboloid, CSMFALSE);
    
    direction_sign = 1.;
    i_generate_half_hiperboloid(top_face, direction_sign, a, c, .5 * height, no_points_circle_radius_parallels_semisphere, no_points_circle_radius_meridians);

    direction_sign = -1.;
    i_generate_half_hiperboloid(bottom_face, direction_sign, a, c, .5 * height, no_points_circle_radius_parallels_semisphere, no_points_circle_radius_meridians);
    
    i_apply_transform_to_solid(
                        x_center, y_center, z_center,
                        Ux, Uy, Uz, Vx, Vy, Vz,
                        hyperboloid);

    {
        struct csmsurface_t *surface;
        
        surface = csmsurface_new_hyperboloid(a, c);
        i_assign_face_eq_to_solid_faces(hyperboloid, &surface);
    }
    
    csmsolid_redo_geometric_face_data(hyperboloid);
    
    return hyperboloid;
}



