//
//  csmsetop.c
//  cysolidmodeling
//
//  Created by Manuel Fernández on 16/5/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmsetop.h"

#include "csmhashtb.inl"
#include "csmsolid.h"
#include "csmsolid.inl"
#include "csmtolerance.inl"
#include "csmvertex.inl"
#include "csmvertex.tli"

#include "a_punter.h"
#include "cyassert.h"
#include "cypespy.h"

ArrEstructura(i_vv_inters_t);
ArrEstructura(i_vf_inters_t);
ArrEstructura(csmedge_t);
ArrEstructura(csmface_t);

enum i_set_operation_t
{
    i_SET_OPERATION_UNION,
    i_SET_OPERATION_INTERSECTION,
    i_SET_OPERATION_DIFFERENCE
};

struct i_vv_inters_t
{
    struct csmvertex_t *vertex_a, *vertex_b;
};

struct i_vf_inters_t
{
    struct csmvertex_t *vertex;
    struct csmface_t *face;
};

struct i_solid_data_t
{
    ArrEstructura(i_vf_inters_t) *vertex_face_neighborhood;
    
    ArrEstructura(csmedge_t) *set_of_null_edges;
    ArrEstructura(csmface_t) *set_of_null_faces;
};

enum i_type_edge_intersection_t
{
    i_TYPE_EDGE_INTERSECTION_VERTEX,
    i_TYPE_EDGE_INTERSECTION_INTERIOR_EDGE,
    i_TYPE_EDGE_INTERSECTION_INTERIOR_FACE
};

struct i_edge_intersection_t
{
    double x, y, z;
    double t;

    enum i_type_edge_intersection_t type;
    struct csmvertex_t *vertex;
    struct csmedge_t *edge;
    struct csmface_t *face;
};

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_vv_inters_t *, i_create_vv_inters, (struct csmvertex_t *vertex_a, struct csmvertex_t *vertex_b))
{
    struct i_vv_inters_t *vv_inters;
    
    vv_inters = MALLOC(struct i_vv_inters_t);
    
    vv_inters->vertex_a = vertex_a;
    vv_inters->vertex_b = vertex_b;
    
    return vv_inters;
}

// ------------------------------------------------------------------------------------------

static void i_free_vv_inters(struct i_vv_inters_t **vv_inters)
{
    assert_no_null(vv_inters);
    assert_no_null(*vv_inters);
    
    FREE_PP(vv_inters, struct i_vv_inters_t);
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_vf_inters_t *, i_create_vf_inters, (struct csmvertex_t *vertex, struct csmface_t *face))
{
    struct i_vf_inters_t *vf_inters;
    
    vf_inters = MALLOC(struct i_vf_inters_t);
    
    vf_inters->vertex = vertex;
    vf_inters->face = face;
    
    return vf_inters;
}

// ------------------------------------------------------------------------------------------

static void i_free_vf_inters(struct i_vf_inters_t **vf_inters)
{
    assert_no_null(vf_inters);
    assert_no_null(*vf_inters);
    
    FREE_PP(vf_inters, struct i_vf_inters_t);
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_solid_data_t *, i_create_solid_data, (
                        ArrEstructura(i_vf_inters_t) **vertex_face_neighborhood,
                        ArrEstructura(csmedge_t) **set_of_null_edges,
                        ArrEstructura(csmface_t) **set_of_null_faces))
{
    struct i_solid_data_t *solid_data;
    
    solid_data = MALLOC(struct i_solid_data_t);
    
    solid_data->vertex_face_neighborhood = ASIGNA_PUNTERO_PP_NO_NULL(vertex_face_neighborhood, ArrEstructura(i_vf_inters_t));

    solid_data->set_of_null_edges = ASIGNA_PUNTERO_PP_NO_NULL(set_of_null_edges, ArrEstructura(csmedge_t));
    solid_data->set_of_null_faces = ASIGNA_PUNTERO_PP_NO_NULL(set_of_null_faces, ArrEstructura(csmface_t));
    
    return solid_data;
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_solid_data_t *, i_create_empty_solid_data, (void))
{
    ArrEstructura(i_vf_inters_t) *vertex_face_neighborhood;
    ArrEstructura(csmedge_t) *set_of_null_edges;
    ArrEstructura(csmface_t) *set_of_null_faces;
    
    vertex_face_neighborhood = arr_CreaPunteroST(0, i_vf_inters_t);
    set_of_null_edges = arr_CreaPunteroST(0, csmedge_t);
    set_of_null_faces = arr_CreaPunteroST(0, csmface_t);

    return i_create_solid_data(&vertex_face_neighborhood, &set_of_null_edges, &set_of_null_faces);
}

// ------------------------------------------------------------------------------------------

static void i_free_solid_data(struct i_solid_data_t **solid_data)
{
    assert_no_null(solid_data);
    assert_no_null(*solid_data);
    
    arr_DestruyeEstructurasST(&(*solid_data)->vertex_face_neighborhood, i_free_vf_inters, i_vf_inters_t);

    arr_DestruyeEstructurasST(&(*solid_data)->set_of_null_edges, NULL, csmedge_t);
    arr_DestruyeEstructurasST(&(*solid_data)->set_of_null_faces, NULL, csmface_t);
    
    FREE_PP(solid_data, struct i_solid_data_t);
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_edge_intersection_t *, i_create_edge_intersection, (
                        double x, double y, double z,
                        double t,
                        enum i_type_edge_intersection_t type,
                        struct csmvertex_t *vertex,
                        struct csmedge_t *edge,
                        struct csmface_t *face))
{
    struct i_edge_intersection_t *edge_intersection;
    
    edge_intersection = MALLOC(struct i_edge_intersection_t);
    
    edge_intersection->x = x;
    edge_intersection->y = y;
    edge_intersection->z = z;
    edge_intersection->t = t;
    
    edge_intersection->type = type;
    edge_intersection->vertex = vertex;
    edge_intersection->edge = edge;
    edge_intersection->face = face;
    
    return edge_intersection;
}

// ------------------------------------------------------------------------------------------

static void i_free_edge_intersection(struct i_edge_intersection_t **edge_intersection)
{
    assert_no_null(edge_intersection);
    assert_no_null(*edge_intersection);
    
    FREE_PP(edge_intersection, struct i_edge_intersection_t);
}

// ------------------------------------------------------------------------------------------

static void i_append_common_vertices_solid_A_on_solid_B(
                        struct csmsolid_t *solid_A, struct csmsolid_t *solid_B,
                        ArrEstructura(i_vv_inters_t) *vv_intersections)
{
    double tolerance;
    struct csmhashtb_iterator(csmvertex_t) *vertex_iterator_A;
    
    tolerance = csmtolerance_equal_coords();
    vertex_iterator_A = csmsolid_vertex_iterator(solid_A);
    
    while (csmhashtb_has_next(vertex_iterator_A, csmvertex_t) == CIERTO)
    {
        struct csmvertex_t *vertex_A, *vertex_B;
        
        csmhashtb_next_pair(vertex_iterator_A, NULL, &vertex_A, csmvertex_t);
        
        if (csmvertex_has_mask_attrib(vertex_A, CSMVERTEX_MASK_SETOP_COMMON_VERTEX) == FALSO
                && csmsolid_contains_vertex_in_same_coordinates_as_given(solid_B, vertex_A, tolerance, &vertex_B) == CIERTO)
        {
            if (csmvertex_has_mask_attrib(vertex_B, CSMVERTEX_MASK_SETOP_COMMON_VERTEX) == FALSO)
            {
                struct i_vv_inters_t *vv_inters;
            
                vv_inters = i_create_vv_inters(vertex_A, vertex_B);
                arr_AppendPunteroST(vv_intersections, vv_inters, i_vv_inters_t);
            
                csmvertex_set_mask_attrib(vertex_A, CSMVERTEX_MASK_SETOP_COMMON_VERTEX);
                csmvertex_set_mask_attrib(vertex_B, CSMVERTEX_MASK_SETOP_COMMON_VERTEX);
            }
        }
    }
    
    csmhashtb_free_iterator(&vertex_iterator_A, csmvertex_t);
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static ArrEstructura(i_vv_inters_t) *, i_locate_common_vertices, (struct csmsolid_t *solid_A, struct csmsolid_t *solid_B))
{
    ArrEstructura(i_vv_inters_t) *vv_intersections;
    
    assert_no_null(solid_A);
    assert_no_null(solid_B);
    
    vv_intersections = arr_CreaPunteroST(0, i_vv_inters_t);
    i_append_common_vertices_solid_A_on_solid_B(solid_A, solid_B, vv_intersections);
    i_append_common_vertices_solid_A_on_solid_B(solid_B, solid_A, vv_intersections);
    
    return vv_intersections;
}
            
// ------------------------------------------------------------------------------------------

static void i_generate_intersections_on_both_solids(
                        struct csmsolid_t *solid_A, struct csmsolid_t *solid_B,
                        ArrEstructura(i_vv_inters_t) **vv_intersections,
                        struct i_solid_data_t **solid_data_A,
                        struct i_solid_data_t **solid_data_B)
{
    ArrEstructura(i_vv_inters_t) *vv_intersections_loc;
    struct i_solid_data_t *solid_data_A_loc, *solid_data_B_loc;
    
    assert_no_null(vv_intersections);
    assert_no_null(solid_data_A);
    assert_no_null(solid_data_B);
    
    solid_data_A_loc = i_create_empty_solid_data();
    solid_data_B_loc = i_create_empty_solid_data();
    
    vv_intersections_loc = i_locate_common_vertices(solid_A, solid_B);
    
    *vv_intersections = vv_intersections_loc;
    *solid_data_A = solid_data_A_loc;
    *solid_data_B = solid_data_B_loc;
}
                        
// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmsolid_t *, i_set_operation_modifying_solids, (
                        enum i_set_operation_t set_operation,
                        struct csmsolid_t *solid_A, struct csmsolid_t *solid_B))
{
    struct csmsolid_t *result;
    ArrEstructura(i_vv_inters_t) *vv_intersections;
    struct i_solid_data_t *solid_data_A, *solid_data_B;

    csmsolid_redo_geometric_generated_data(solid_A);
    csmsolid_clear_algorithm_vertex_mask(solid_A);
    
    csmsolid_redo_geometric_generated_data(solid_B);
    csmsolid_clear_algorithm_vertex_mask(solid_B);
    
    i_generate_intersections_on_both_solids(solid_A, solid_B, &vv_intersections, &solid_data_A, &solid_data_B);
    
    result = NULL;
    
    
    
    arr_DestruyeEstructurasST(&vv_intersections, i_free_vv_inters, i_vv_inters_t);
    i_free_solid_data(&solid_data_A);
    i_free_solid_data(&solid_data_B);
    
    return result;
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmsolid_t *, i_set_operation, (
                        enum i_set_operation_t set_operation,
                        const struct csmsolid_t *solid_A, const struct csmsolid_t *solid_B))
{
    struct csmsolid_t *result;
    struct csmsolid_t *solid_A_copy, *solid_B_copy;
    
    solid_A_copy = csmsolid_duplicate(solid_A);
    solid_B_copy = csmsolid_duplicate(solid_B);
    
    result = i_set_operation_modifying_solids(set_operation, solid_A_copy, solid_B_copy);
    
    csmsolid_free(&solid_A_copy);
    csmsolid_free(&solid_B_copy);
    
    return result;
}

// ------------------------------------------------------------------------------------------

struct csmsolid_t *csmsetop_difference_A_minus_B(const struct csmsolid_t *solid_A, const struct csmsolid_t *solid_B)
{
    enum i_set_operation_t set_operation;
    
    set_operation = i_SET_OPERATION_DIFFERENCE;
    return i_set_operation(set_operation, solid_A, solid_B);
}

// ------------------------------------------------------------------------------------------

struct csmsolid_t *csmsetop_union_A_and_B(const struct csmsolid_t *solid_A, const struct csmsolid_t *solid_B)
{
    enum i_set_operation_t set_operation;
    
    set_operation = i_SET_OPERATION_UNION;
    return i_set_operation(set_operation, solid_A, solid_B);
}

// ------------------------------------------------------------------------------------------

struct csmsolid_t *csmsetop_intersection_A_and_B(const struct csmsolid_t *solid_A, const struct csmsolid_t *solid_B)
{
    enum i_set_operation_t set_operation;
    
    set_operation = i_SET_OPERATION_DIFFERENCE;
    return i_set_operation(set_operation, solid_A, solid_B);
}
