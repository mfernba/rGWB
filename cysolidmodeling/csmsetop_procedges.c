//
//  csmsetop_procedges.c
//  cysolidmodeling
//
//  Created by Manuel Fernández on 22/5/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmsetop_procedges.inl"

#include "csmsetop_vtxfacc.inl"
#include "csmsetop_vtxvtx.inl"
#include "csmsetop.tli"
#include "csmedge.inl"
#include "csmedge.tli"
#include "csmeuler_lmev.inl"
#include "csmface.inl"
#include "csmhashtb.inl"
#include "csmhedge.inl"
#include "csmmath.inl"
#include "csmmath.tli"
#include "csmopbas.inl"
#include "csmsolid.inl"
#include "csmtolerance.inl"
#include "csmvertex.inl"
#include "csmvertex.tli"

#include "a_punter.h"
#include "cyassert.h"
#include "cypespy.h"

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

ArrEstructura(i_edge_intersection_t);

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
                        ArrEstructura(csmsetop_vtxvtx_inters_t) *vv_intersections)
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
                struct csmsetop_vtxvtx_inters_t *vv_inters;
            
                vv_inters = csmsetop_vtxvtx_create_inters(vertex_A, vertex_B);
                arr_AppendPunteroST(vv_intersections, vv_inters, csmsetop_vtxvtx_inters_t);
            
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
                        ArrEstructura(csmsetop_vtxvtx_inters_t) *vv_intersections)
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
        {
            assert(hit_vertex == NULL);
            assert(hit_hedge == NULL);
            
            intersection_type = i_TYPE_EDGE_INTERSECTION_INTERIOR_FACE;
            break;
        }
            
        case CSMMATH_CONTAIMENT_POINT_LOOP_ON_VERTEX:
        {
            assert(hit_vertex != NULL);
            intersection_type = i_TYPE_EDGE_INTERSECTION_VERTEX;
            break;
        }
            
        case CSMMATH_CONTAIMENT_POINT_LOOP_ON_HEDGE:
        {
            assert(hit_hedge != NULL);
            intersection_type = i_TYPE_EDGE_INTERSECTION_INTERIOR_EDGE;
            break;
        }
            
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
                        ArrEstructura(csmsetop_vtxvtx_inters_t) *vv_intersections)
{
    if (csmvertex_has_mask_attrib(vertex_a, CSMVERTEX_MASK_SETOP_COMMON_VERTEX) == CIERTO)
    {
        assert(csmvertex_has_mask_attrib(vertex_b, CSMVERTEX_MASK_SETOP_COMMON_VERTEX) == CIERTO);
    }
    else
    {
        struct csmsetop_vtxvtx_inters_t *vv_inters;
        
        csmvertex_set_mask_attrib(vertex_a, CSMVERTEX_MASK_SETOP_COMMON_VERTEX);
        csmvertex_set_mask_attrib(vertex_b, CSMVERTEX_MASK_SETOP_COMMON_VERTEX);
    
        vv_inters = csmsetop_vtxvtx_create_inters(vertex_a, vertex_b);
        arr_AppendPunteroST(vv_intersections, vv_inters, csmsetop_vtxvtx_inters_t);
    }
}

// ------------------------------------------------------------------------------------------

static void i_append_new_vf_inters(
                        struct csmvertex_t *vertex_a, struct csmface_t *face_b,
                        ArrEstructura(csmsetop_vtxfacc_inters_t) *vertex_face_neighborhood)
{
    struct csmsetop_vtxfacc_inters_t *vf_inters;
    
    vf_inters = csmsetop_vtxfacc_create_inters(vertex_a, face_b);
    arr_AppendPunteroST(vertex_face_neighborhood, vf_inters, csmsetop_vtxfacc_inters_t);
}

// ------------------------------------------------------------------------------------------

static void i_process_edge_intersections(
                        struct csmedge_t *edge,
                        const ArrEstructura(i_edge_intersection_t) *edge_intersecctions,
                        ArrEstructura(csmsetop_vtxvtx_inters_t) *vv_intersections,
                        ArrEstructura(csmsetop_vtxfacc_inters_t) *vertex_face_neighborhood)
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
            
                i_append_new_vf_inters(new_vertex, edge_intersection->face, vertex_face_neighborhood);
                break;
                
            default_error();
        }
    }
}

// ------------------------------------------------------------------------------------------

static void i_generate_intersections_edge_with_solid_faces(
                        struct csmedge_t *edge_A,
                        struct csmsolid_t *solid_B,
                        ArrEstructura(csmsetop_vtxvtx_inters_t) *vv_intersections,
                        ArrEstructura(csmsetop_vtxfacc_inters_t) *vertex_face_neighborhood)
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
                        ArrEstructura(csmsetop_vtxvtx_inters_t) *vv_intersections,
                        ArrEstructura(csmsetop_vtxfacc_inters_t) *vertex_face_neighborhood)
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

void csmsetop_procedges_generate_intersections_on_both_solids(
                        struct csmsolid_t *solid_A, struct csmsolid_t *solid_B,
                        ArrEstructura(csmsetop_vtxvtx_inters_t) **vv_intersections,
                        ArrEstructura(csmsetop_vtxfacc_inters_t) **vf_intersections_A,
                        ArrEstructura(csmsetop_vtxfacc_inters_t) **vf_intersections_B)
{
    ArrEstructura(csmsetop_vtxvtx_inters_t) *vv_intersections_loc;
    ArrEstructura(csmsetop_vtxfacc_inters_t) *vf_intersections_A_loc, *vf_intersections_B_loc;
    
    assert_no_null(vv_intersections);
    assert_no_null(vf_intersections_A);
    assert_no_null(vf_intersections_B);

    vv_intersections_loc = arr_CreaPunteroST(0, csmsetop_vtxvtx_inters_t);
    vf_intersections_A_loc = arr_CreaPunteroST(0, csmsetop_vtxfacc_inters_t);
    vf_intersections_B_loc = arr_CreaPunteroST(0, csmsetop_vtxfacc_inters_t);

    i_generate_edge_intersections_solid_A_with_solid_B(solid_A, solid_B, vv_intersections_loc, vf_intersections_A_loc);
    i_generate_edge_intersections_solid_A_with_solid_B(solid_B, solid_A, vv_intersections_loc, vf_intersections_B_loc);
    i_append_common_vertices_solid_A_and_B_not_previously_found(solid_A, solid_B, vv_intersections_loc);
    
    *vv_intersections = vv_intersections_loc;
    *vf_intersections_A = vf_intersections_A_loc;
    *vf_intersections_B = vf_intersections_B_loc;
}
