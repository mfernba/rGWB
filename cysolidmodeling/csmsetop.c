//
//  csmsetop.c
//  cysolidmodeling
//
//  Created by Manuel Fernández on 16/5/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmsetop.h"

#include "csmhashtb.inl"
#include "csmhedge.inl"
#include "csmedge.inl"
#include "csmedge.tli"
#include "csmeuler_lmev.inl"
#include "csmeuler_lkemr.inl"
#include "csmface.inl"
#include "csmloop.inl"
#include "csmmath.inl"
#include "csmmath.tli"
#include "csmopbas.inl"
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
ArrEstructura(i_edge_intersection_t);
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
    struct csmhedge_t *null_edge_internal_loop;
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
    struct csmvertex_t *hit_vertex;
    struct csmhedge_t *hit_hedge;
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

CONSTRUCTOR(static struct i_vf_inters_t *, i_create_vf_inters, (
                        struct csmvertex_t *vertex,
                        struct csmface_t *face,
                        struct csmhedge_t *null_edge_internal_loop))
{
    struct i_vf_inters_t *vf_inters;
    
    vf_inters = MALLOC(struct i_vf_inters_t);
    
    vf_inters->vertex = vertex;
    
    vf_inters->face = face;
    vf_inters->null_edge_internal_loop = null_edge_internal_loop;
    
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
                        struct csmvertex_t *hit_vertex,
                        struct csmhedge_t *hit_hedge,
                        struct csmface_t *face))
{
    struct i_edge_intersection_t *edge_intersection;
    
    edge_intersection = MALLOC(struct i_edge_intersection_t);
    
    edge_intersection->x = x;
    edge_intersection->y = y;
    edge_intersection->z = z;
    edge_intersection->t = t;
    
    edge_intersection->type = type;
    edge_intersection->hit_vertex = hit_vertex;
    edge_intersection->hit_hedge = hit_hedge;
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

static void i_append_common_vertices_solid_A_and_B_not_previously_found(
                        struct csmsolid_t *solid_A, struct csmsolid_t *solid_B,
                        ArrEstructura(i_vv_inters_t) *vv_intersections)
{
    i_append_common_vertices_solid_A_on_solid_B(solid_A, solid_B, vv_intersections);
    i_append_common_vertices_solid_A_on_solid_B(solid_B, solid_A, vv_intersections);
}

// ------------------------------------------------------------------------------------------

static void i_append_new_edge_intersection(
                        double x_inters, double y_inters, double z_inters, double t,
                        enum csmmath_contaiment_point_loop_t type_of_containment,
                        struct csmvertex_t *hit_vertex,
                        struct csmhedge_t *hit_hedge,
                        struct csmface_t *face,
                        ArrEstructura(i_edge_intersection_t) *edge_intersections)
{
    enum i_type_edge_intersection_t intersection_type;
    struct i_edge_intersection_t *edge_intersection;
            
    switch (type_of_containment)
    {
        case CSMMATH_CONTAIMENT_POINT_LOOP_INTERIOR:
            
            intersection_type = i_TYPE_EDGE_INTERSECTION_INTERIOR_FACE;
            break;
            
        case CSMMATH_CONTAIMENT_POINT_LOOP_ON_VERTEX:
            
            intersection_type = i_TYPE_EDGE_INTERSECTION_VERTEX;
            break;
            
        case CSMMATH_CONTAIMENT_POINT_LOOP_ON_HEDGE:
            
            intersection_type = i_TYPE_EDGE_INTERSECTION_INTERIOR_EDGE;
            break;
            
        default_error();
    }
    
    edge_intersection = i_create_edge_intersection(x_inters, y_inters, z_inters, t, intersection_type, hit_vertex, hit_hedge, face);
    arr_AppendPunteroST(edge_intersections, edge_intersection, i_edge_intersection_t);
}

// ------------------------------------------------------------------------------------------

static void i_append_intersection_between_vertex_A_and_face_B(
                        struct csmvertex_t *vertex, struct csmface_t *face_B,
                        double t_vertex_on_edge,
                        ArrEstructura(i_edge_intersection_t) *edge_intersections)
{
    enum csmmath_contaiment_point_loop_t type_of_containment;
    struct csmvertex_t *hit_vertex;
    struct csmhedge_t *hit_hedge;
    
    hit_vertex = NULL;
    hit_hedge = NULL;
    
    if (csmface_contains_vertex(face_B, vertex, &type_of_containment, &hit_vertex, &hit_hedge) == CIERTO)
    {
        double x_inters, y_inters, z_inters;
        
        csmvertex_get_coordenadas(vertex, &x_inters, &y_inters, &z_inters);
        
        i_append_new_edge_intersection(
                        x_inters, y_inters, z_inters, t_vertex_on_edge,
                        type_of_containment,
                        hit_vertex,
                        hit_hedge,
                        face_B,
                        edge_intersections);
    }
}

// ------------------------------------------------------------------------------------------

static void i_append_intersections_between_A_edge_and_B_face(
                        struct csmedge_t *edge_A, struct csmface_t *face_B,
                        ArrEstructura(i_edge_intersection_t) *edge_intersections)
{
    struct csmhedge_t *hedge_pos, *hedge_neg;
    struct csmvertex_t *vertex_pos, *vertex_neg;
    enum csmmath_double_relation_t classification_vertex_pos, classification_vertex_neg;
    
    hedge_pos = csmedge_hedge_lado(edge_A, CSMEDGE_LADO_HEDGE_POS);
    vertex_pos = csmhedge_vertex(hedge_pos);
    classification_vertex_pos = csmface_classify_vertex_relative_to_face(face_B, vertex_pos);

    hedge_neg = csmedge_hedge_lado(edge_A, CSMEDGE_LADO_HEDGE_NEG);
    vertex_neg = csmhedge_vertex(hedge_neg);
    classification_vertex_neg = csmface_classify_vertex_relative_to_face(face_B, vertex_neg);
    
    if (classification_vertex_pos == CSMMATH_EQUAL_VALUES || classification_vertex_neg == CSMMATH_EQUAL_VALUES)
    {
        if (classification_vertex_pos == CSMMATH_EQUAL_VALUES)
            i_append_intersection_between_vertex_A_and_face_B(vertex_pos, face_B, 0., edge_intersections);

        if (classification_vertex_neg == CSMMATH_EQUAL_VALUES)
            i_append_intersection_between_vertex_A_and_face_B(vertex_neg, face_B, 1., edge_intersections);
    }
    else
    {
        double x1, y1, z1, x2, y2, z2;
        double x_inters, y_inters, z_inters, t;
    
        csmvertex_get_coordenadas(vertex_pos, &x1, &y1, &z1);
        csmvertex_get_coordenadas(vertex_neg, &x2, &y2, &z2);
    
        if (csmface_exists_intersection_between_line_and_face_plane(
                    face_B,
                    x1, y1, z1, x2, y2, z2,
                    &x_inters, &y_inters, &z_inters, &t) == CIERTO)
        {
            enum csmmath_contaiment_point_loop_t type_of_containment;
            struct csmvertex_t *hit_vertex;
            struct csmhedge_t *hit_hedge;
            
            hit_vertex = NULL;
            hit_hedge = NULL;
            
            if (csmface_contains_point(face_B, x_inters, y_inters, z_inters, &type_of_containment, &hit_vertex, &hit_hedge) == CIERTO)
            {
                i_append_new_edge_intersection(
                        x_inters, y_inters, z_inters, t,
                        type_of_containment,
                        hit_vertex,
                        hit_hedge,
                        face_B,
                        edge_intersections);
            }
        }
    }
}

// ------------------------------------------------------------------------------------------
    
static enum comparac_t i_compara_edge_intersection(
                        const struct i_edge_intersection_t *edge_intersection1,
                        const struct i_edge_intersection_t *edge_intersection2)
{
    double tolerance;
    
    assert_no_null(edge_intersection1);
    assert_no_null(edge_intersection2);
    
    tolerance = csmtolerance_relative_position_over_edge();
    
    switch (csmmath_compare_doubles(edge_intersection1->t, edge_intersection2->t, tolerance))
    {
        case CSMMATH_VALUE1_LESS_THAN_VALUE2:
            
            return comparac_PRIMERO_MENOR;
            
        case CSMMATH_EQUAL_VALUES:
            
            return comparac_IGUALES;
            
        case CSMMATH_VALUE1_GREATER_THAN_VALUE2:
            
            return comparac_PRIMERO_MAYOR;
            
        default_error();
    }
}

// ------------------------------------------------------------------------------------------

static void i_append_new_vv_inters(
                        struct csmvertex_t *vertex_a, struct csmvertex_t *vertex_b,
                        ArrEstructura(i_vv_inters_t) *vv_intersections)
{
    if (csmvertex_has_mask_attrib(vertex_a, CSMVERTEX_MASK_SETOP_COMMON_VERTEX) == CIERTO)
    {
        assert(csmvertex_has_mask_attrib(vertex_b, CSMVERTEX_MASK_SETOP_COMMON_VERTEX) == CIERTO);
    }
    else
    {
        struct i_vv_inters_t *vv_inters;
        
        csmvertex_set_mask_attrib(vertex_a, CSMVERTEX_MASK_SETOP_COMMON_VERTEX);
        csmvertex_set_mask_attrib(vertex_b, CSMVERTEX_MASK_SETOP_COMMON_VERTEX);
    
        vv_inters = i_create_vv_inters(vertex_a, vertex_b);
        arr_AppendPunteroST(vv_intersections, vv_inters, i_vv_inters_t);
    }
}

// ------------------------------------------------------------------------------------------

static void i_append_new_vf_inters(
                        struct csmvertex_t *vertex_a, struct csmface_t *face_b, struct csmhedge_t *null_edge_internal_loop,
                        ArrEstructura(i_vf_inters_t) *vertex_face_neighborhood)
{
    struct i_vf_inters_t *vf_inters;
    
    vf_inters = i_create_vf_inters(vertex_a, face_b, null_edge_internal_loop);
    arr_AppendPunteroST(vertex_face_neighborhood, vf_inters, i_vf_inters_t);
}

// ------------------------------------------------------------------------------------------

static void i_process_edge_intersections(
                        struct csmedge_t *edge,
                        const ArrEstructura(i_edge_intersection_t) *edge_intersecctions,
                        ArrEstructura(i_vv_inters_t) *vv_intersections,
                        ArrEstructura(i_vf_inters_t) *vertex_face_neighborhood)
{
    unsigned long i, num_intersections;
    
    num_intersections = arr_NumElemsPunteroST(edge_intersecctions, i_edge_intersection_t);
    assert(num_intersections > 0);
    
    for (i = 0; i < num_intersections; i++)
    {
        const struct i_edge_intersection_t *edge_intersection;
        struct csmhedge_t *hedge_pos, *hedge_neg;
        struct csmhedge_t *hedge_pos_next;
        struct csmvertex_t *new_vertex;
        
        edge_intersection = arr_GetPunteroConstST(edge_intersecctions, i, i_edge_intersection_t);
        assert_no_null(edge_intersection);
        
        hedge_pos = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_POS);
        hedge_pos_next = csmhedge_next(hedge_pos);
        
        hedge_neg = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_NEG);
        
        csmeuler_lmev(
                    hedge_neg, hedge_pos_next, 
                    edge_intersection->x, edge_intersection->y, edge_intersection->z,
                    &new_vertex,
                    NULL,
                    NULL, NULL);
        
        switch (edge_intersection->type)
        {
            case i_TYPE_EDGE_INTERSECTION_VERTEX:
                
                i_append_new_vv_inters(new_vertex, edge_intersection->hit_vertex, vv_intersections);
                break;
                
            case i_TYPE_EDGE_INTERSECTION_INTERIOR_EDGE:
            {
                struct csmhedge_t *mate_hit_hedge, *mate_hit_hedge_next;
                struct csmvertex_t *new_vertex_on_hit_hedge;
                
                mate_hit_hedge = csmopbas_mate(edge_intersection->hit_hedge);
                mate_hit_hedge_next = csmhedge_next(mate_hit_hedge);
                
                csmeuler_lmev(
                          edge_intersection->hit_hedge, mate_hit_hedge_next,
                          edge_intersection->x, edge_intersection->y, edge_intersection->z,
                          &new_vertex_on_hit_hedge,
                          NULL,
                          NULL, NULL);
                
                i_append_new_vv_inters(new_vertex, new_vertex_on_hit_hedge, vv_intersections);
                break;
            }
                
            case i_TYPE_EDGE_INTERSECTION_INTERIOR_FACE:
            {
                struct csmloop_t *flout_face;
                struct csmhedge_t *ledge_flout_face;
                struct csmvertex_t *new_vertex_on_B;
                struct csmedge_t *edge_to_delete_on_B;
                struct csmhedge_t *hedge_pos_to_delete, *hedge_neg_to_delete;
                struct csmhedge_t *null_edge_internal_loop;
                
                flout_face = csmface_flout(edge_intersection->face);
                ledge_flout_face = csmloop_ledge(flout_face);

                csmeuler_lmev(
                          ledge_flout_face, ledge_flout_face,
                          edge_intersection->x, edge_intersection->y, edge_intersection->z,
                          &new_vertex_on_B,
                          &edge_to_delete_on_B,
                          NULL, NULL);
                
                hedge_pos_to_delete = csmedge_hedge_lado(edge_to_delete_on_B, CSMEDGE_LADO_HEDGE_POS);
                hedge_neg_to_delete = csmedge_hedge_lado(edge_to_delete_on_B, CSMEDGE_LADO_HEDGE_NEG);
                
                csmeuler_lkemr(&hedge_pos_to_delete, &hedge_neg_to_delete, NULL, &null_edge_internal_loop);
                assert(csmhedge_vertex(null_edge_internal_loop) == new_vertex_on_B);
                
                i_append_new_vv_inters(new_vertex, new_vertex_on_B, vv_intersections);
                i_append_new_vf_inters(new_vertex, edge_intersection->face, null_edge_internal_loop, vertex_face_neighborhood);
                break;
            }
                
            default_error();
        }
    }
}

// ------------------------------------------------------------------------------------------

static void i_generate_intersections_edge_with_solid_faces(
                        struct csmedge_t *edge_A,
                        struct csmsolid_t *solid_B,
                        ArrEstructura(i_vv_inters_t) *vv_intersections,
                        ArrEstructura(i_vf_inters_t) *vertex_face_neighborhood)
{
    ArrEstructura(i_edge_intersection_t) *edge_intersecctions;
    struct csmhashtb_iterator(csmface_t) *face_iterator_B;
    
    edge_intersecctions = arr_CreaPunteroST(0, i_edge_intersection_t);
    face_iterator_B = csmsolid_face_iterator(solid_B);

    while (csmhashtb_has_next(face_iterator_B, csmface_t) == CIERTO)
    {
        struct csmface_t *face_B;
    
        csmhashtb_next_pair(face_iterator_B, NULL, &face_B, csmface_t);
        i_append_intersections_between_A_edge_and_B_face(edge_A, face_B, edge_intersecctions);
    }
    
    if (arr_NumElemsPunteroST(edge_intersecctions, i_edge_intersection_t) > 0)
    {
        arr_QSortPunteroST(edge_intersecctions, i_compara_edge_intersection, i_edge_intersection_t);
        i_process_edge_intersections(edge_A, edge_intersecctions, vv_intersections, vertex_face_neighborhood);
    }
    
    csmhashtb_free_iterator(&face_iterator_B, csmface_t);
    arr_DestruyeEstructurasST(&edge_intersecctions, i_free_edge_intersection, i_edge_intersection_t);
}

// ------------------------------------------------------------------------------------------

static void i_generate_edge_intersections_solid_A_with_solid_B(
                        struct csmsolid_t *solid_A, struct csmsolid_t *solid_B,
                        ArrEstructura(i_vv_inters_t) *vv_intersections,
                        ArrEstructura(i_vf_inters_t) *vertex_face_neighborhood)
{
    struct csmhashtb_iterator(csmedge_t) *edge_iterator_A;
    
    edge_iterator_A = csmsolid_edge_iterator(solid_A);
    
    while (csmhashtb_has_next(edge_iterator_A, csmedge_t) == CIERTO)
    {
        struct csmedge_t *edge_A;
        
        csmhashtb_next_pair(edge_iterator_A, NULL, &edge_A, csmedge_t);
        i_generate_intersections_edge_with_solid_faces(edge_A, solid_B, vv_intersections, vertex_face_neighborhood);
    }
    
    csmhashtb_free_iterator(&edge_iterator_A, csmedge_t);
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

    vv_intersections_loc = arr_CreaPunteroST(0, i_vv_inters_t);

    solid_data_A_loc = i_create_empty_solid_data();
    assert_no_null(solid_data_A_loc);
    
    solid_data_B_loc = i_create_empty_solid_data();
    assert_no_null(solid_data_B_loc);

    i_generate_edge_intersections_solid_A_with_solid_B(solid_A, solid_B, vv_intersections_loc, solid_data_A_loc->vertex_face_neighborhood);
    i_generate_edge_intersections_solid_A_with_solid_B(solid_B, solid_A, vv_intersections_loc, solid_data_B_loc->vertex_face_neighborhood);
    i_append_common_vertices_solid_A_and_B_not_previously_found(solid_A, solid_B, vv_intersections_loc);
    
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
