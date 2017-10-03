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
#include "csmdebug.inl"
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
#include "csmassert.inl"
#include "csmmem.inl"
#include "csmstring.inl"

#include "a_punter.h"

enum i_intersection_position_t
{
    i_INTERSECTION_POSITION_AT_EDGE_VERTEX,
    i_INTERSECTION_POSITION_AT_EDGE_INTERIOR
};

enum i_type_edge_intersection_t
{
    i_TYPE_EDGE_INTERSECTION_VERTEX,
    i_TYPE_EDGE_INTERSECTION_INTERIOR_EDGE,
    i_TYPE_EDGE_INTERSECTION_INTERIOR_FACE
};

struct i_edge_intersection_t
{
    enum i_intersection_position_t intersection_position_at_edge;
    struct csmvertex_t *edge_vertex;
    double x_edge_interior, y_edge_interior, z_edge_interior;
    double t_intersection_on_edge;
    
    struct csmface_t *face;
    enum i_type_edge_intersection_t edge_intersection_at_face;
    struct csmvertex_t *hit_vertex_at_face;
    struct csmhedge_t *hit_hedge_at_face;
    double x_edge_interior_hedge_at_face, y_edge_interior_hedge_at_face, z_edge_interior_hedge_at_face;
};

ArrEstructura(i_edge_intersection_t);

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_edge_intersection_t *, i_create_edge_intersection, (
                        enum i_intersection_position_t intersection_position_at_edge,
                        struct csmvertex_t *edge_vertex,
                        double x_edge_interior, double y_edge_interior, double z_edge_interior,
                        double t_intersection_on_edge,
                        struct csmface_t *face,
                        enum i_type_edge_intersection_t edge_intersection_at_face,
                        struct csmvertex_t *hit_vertex_at_face,
                        struct csmhedge_t *hit_hedge_at_face,
                        double x_edge_interior_hedge_at_face, double y_edge_interior_hedge_at_face, double z_edge_interior_hedge_at_face))
{
    struct i_edge_intersection_t *edge_intersection;
    
    edge_intersection = MALLOC(struct i_edge_intersection_t);
    
    edge_intersection->intersection_position_at_edge = intersection_position_at_edge;
    edge_intersection->edge_vertex = edge_vertex;
    edge_intersection->x_edge_interior = x_edge_interior;
    edge_intersection->y_edge_interior = y_edge_interior;
    edge_intersection->z_edge_interior = z_edge_interior;
    edge_intersection->t_intersection_on_edge = t_intersection_on_edge;
    
    edge_intersection->face = face;
    edge_intersection->edge_intersection_at_face = edge_intersection_at_face;
    edge_intersection->hit_vertex_at_face = hit_vertex_at_face;
    edge_intersection->hit_hedge_at_face = hit_hedge_at_face;
    edge_intersection->x_edge_interior_hedge_at_face = x_edge_interior_hedge_at_face;
    edge_intersection->y_edge_interior_hedge_at_face = y_edge_interior_hedge_at_face;
    edge_intersection->z_edge_interior_hedge_at_face = z_edge_interior_hedge_at_face;
    
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

static void i_append_new_vv_inters(
                        struct csmvertex_t *vertex_a, struct csmvertex_t *vertex_b,
                        ArrEstructura(csmsetop_vtxvtx_inters_t) *vv_intersections,
                        CSMBOOL *did_add_intersection)
{
    assert_no_null(did_add_intersection);
    
    if (csmvertex_has_mask_attrib(vertex_a, CSMVERTEX_MASK_SETOP_COMMON_VERTEX) == CSMTRUE)
    {
        //assert(csmvertex_has_mask_attrib(vertex_b, CSMVERTEX_MASK_SETOP_COMMON_VERTEX) == CSMTRUE);
        *did_add_intersection = CSMFALSE;
    }
    else
    {
        struct csmsetop_vtxvtx_inters_t *vv_inters;
        
        *did_add_intersection = CSMTRUE;
        
        csmvertex_set_mask_attrib(vertex_a, CSMVERTEX_MASK_SETOP_COMMON_VERTEX);
        csmvertex_set_mask_attrib(vertex_b, CSMVERTEX_MASK_SETOP_COMMON_VERTEX);
    
        vv_inters = csmsetop_vtxvtx_create_inters(vertex_a, vertex_b);
        arr_AppendPunteroST(vv_intersections, vv_inters, csmsetop_vtxvtx_inters_t);
    }
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
    
    while (csmhashtb_has_next(vertex_iterator_A, csmvertex_t) == CSMTRUE)
    {
        struct csmvertex_t *vertex_A, *vertex_B;
        
        csmhashtb_next_pair(vertex_iterator_A, NULL, &vertex_A, csmvertex_t);
        
        if (csmvertex_has_mask_attrib(vertex_A, CSMVERTEX_MASK_SETOP_COMMON_VERTEX) == CSMFALSE
                && csmsolid_contains_vertex_in_same_coordinates_as_given(solid_B, vertex_A, tolerance, &vertex_B) == CSMTRUE)
        {
            CSMBOOL did_add_intersection;
            
            i_append_new_vv_inters(vertex_A, vertex_B, vv_intersections, &did_add_intersection);
            
            if (did_add_intersection == CSMTRUE)
            {
                char *description;
                double x, y, z;
                
                csmvertex_get_coordenadas(vertex_A, &x, &y, &z);
                description = copiafor_codigo5("EQ VV (%g, %g, %g) %lu %lu", x, y, z, csmvertex_id(vertex_A), csmvertex_id(vertex_B));
                csmdebug_append_debug_point(x, y, z, &description);
            
                csmdebug_print_debug_info("-->Coincident vertices: (%lu, %lu)\n", csmvertex_id(vertex_A), csmvertex_id(vertex_B));
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

static bool i_equal_edge_intersection(const struct i_edge_intersection_t *edge_intersection1, const struct i_edge_intersection_t *edge_intersection2)
{
    assert_no_null(edge_intersection1);
    assert_no_null(edge_intersection2);
    
    if (edge_intersection1->intersection_position_at_edge != edge_intersection2->intersection_position_at_edge)
        return false;
    
    switch (edge_intersection1->intersection_position_at_edge)
    {
        case i_INTERSECTION_POSITION_AT_EDGE_VERTEX:
            
            if (edge_intersection1->edge_vertex == edge_intersection2->edge_vertex)
                return true;
            else
                return false;
            
        case i_INTERSECTION_POSITION_AT_EDGE_INTERIOR:

            return csmmath_equal_coords(
                        edge_intersection1->x_edge_interior, edge_intersection1->y_edge_interior, edge_intersection1->z_edge_interior,
                        edge_intersection2->x_edge_interior, edge_intersection2->y_edge_interior, edge_intersection2->z_edge_interior,
                        csmtolerance_equal_coords());
            
        default_error();
    }
}

// ------------------------------------------------------------------------------------------

static void i_append_new_edge_intersection(
                        enum i_intersection_position_t intersection_position_at_edge,
                        struct csmvertex_t *edge_vertex,
                        double x_edge_interior, double y_edge_interior, double z_edge_interior,
                        double t_intersection_on_edge,
                        struct csmface_t *face,
                        enum csmmath_contaiment_point_loop_t type_of_containment_at_face,
                        struct csmvertex_t *hit_vertex_at_face,
                        struct csmhedge_t *hit_hedge_at_face,
                        double x_edge_interior_hedge_at_face, double y_edge_interior_hedge_at_face, double z_edge_interior_hedge_at_face,
                        ArrEstructura(i_edge_intersection_t) *edge_intersections)
{
    enum i_type_edge_intersection_t edge_intersection_at_face;
    struct i_edge_intersection_t *edge_intersection;
            
    switch (type_of_containment_at_face)
    {
        case CSMMATH_CONTAIMENT_POINT_LOOP_INTERIOR:
        {
            assert(hit_vertex_at_face == NULL);
            assert(hit_hedge_at_face == NULL);
            
            edge_intersection_at_face = i_TYPE_EDGE_INTERSECTION_INTERIOR_FACE;
            break;
        }
            
        case CSMMATH_CONTAIMENT_POINT_LOOP_ON_VERTEX:
        {
            assert(hit_vertex_at_face != NULL);
            edge_intersection_at_face = i_TYPE_EDGE_INTERSECTION_VERTEX;
            break;
        }
            
        case CSMMATH_CONTAIMENT_POINT_LOOP_ON_HEDGE:
        {
            assert(hit_hedge_at_face != NULL);
            edge_intersection_at_face = i_TYPE_EDGE_INTERSECTION_INTERIOR_EDGE;
            break;
        }
            
        default_error();
    }
    
    edge_intersection = i_create_edge_intersection(
                        intersection_position_at_edge,
                        edge_vertex,
                        x_edge_interior, y_edge_interior, z_edge_interior,
                        t_intersection_on_edge,
                        face,
                        edge_intersection_at_face,
                        hit_vertex_at_face,
                        hit_hedge_at_face,
                        x_edge_interior_hedge_at_face, y_edge_interior_hedge_at_face, z_edge_interior_hedge_at_face);
    
    if (arr_ExisteEstructuraST(
                        edge_intersections, i_edge_intersection_t,
                        edge_intersection, struct i_edge_intersection_t,
                        i_equal_edge_intersection,
                        NULL) == CSMTRUE)
    {
        i_free_edge_intersection(&edge_intersection);
    }
    else
    {
        arr_AppendPunteroST(edge_intersections, edge_intersection, i_edge_intersection_t);
    }
}

// ------------------------------------------------------------------------------------------

static void i_intersection_coords_at_hedge(
                        enum csmmath_contaiment_point_loop_t type_of_containment_at_face,
                        struct csmhedge_t *hedge,
                        double t_relative_to_hit_hedge_at_face,
                        double *x, double *y, double *z)
{
    assert_no_null(x);
    assert_no_null(y);
    assert_no_null(z);
    
    switch (type_of_containment_at_face)
    {
        case CSMMATH_CONTAIMENT_POINT_LOOP_ON_HEDGE:
        {
            struct csmhedge_t *hedge_next;
            double x1, y1, z1, x2, y2, z2;
            
            csmvertex_get_coordenadas(csmhedge_vertex(hedge), &x1, &y1, &z1);
            
            hedge_next = csmhedge_next(hedge);
            csmvertex_get_coordenadas(csmhedge_vertex(hedge_next), &x2, &y2, &z2);
            
            *x = x1 + (x2 - x1) * t_relative_to_hit_hedge_at_face;
            *y = y1 + (y2 - y1) * t_relative_to_hit_hedge_at_face;
            *z = z1 + (z2 - z1) * t_relative_to_hit_hedge_at_face;
            break;
        }
            
        case CSMMATH_CONTAIMENT_POINT_LOOP_ON_VERTEX:
        case CSMMATH_CONTAIMENT_POINT_LOOP_INTERIOR:
            
            *x = 0.;
            *y = 0.;
            *z = 0.;
            break;
            
        default_error();
    }
}

// ------------------------------------------------------------------------------------------

static void i_append_intersection_between_vertex_A_and_face_B(
                        struct csmvertex_t *vertex, struct csmface_t *face_B,
                        double t_vertex_on_edge,
                        ArrEstructura(i_edge_intersection_t) *edge_intersections)
{
    enum csmmath_contaiment_point_loop_t type_of_containment_at_face;
    struct csmvertex_t *hit_vertex_at_face;
    struct csmhedge_t *hit_hedge_at_face;
    double t_relative_to_hit_hedge_at_face;
    
    hit_vertex_at_face = NULL;
    hit_hedge_at_face = NULL;
    
    if (csmface_contains_vertex(
                        face_B,
                        vertex,
                        &type_of_containment_at_face,
                        &hit_vertex_at_face,
                        &hit_hedge_at_face, &t_relative_to_hit_hedge_at_face) == CSMTRUE)
    {
        enum i_intersection_position_t intersection_position_at_edge;
        double x_edge_interior, y_edge_interior, z_edge_interior;
        double x_edge_interior_hedge_at_face, y_edge_interior_hedge_at_face, z_edge_interior_hedge_at_face;
        
        intersection_position_at_edge = i_INTERSECTION_POSITION_AT_EDGE_VERTEX;
        
        x_edge_interior = 0.;
        y_edge_interior = 0.;
        z_edge_interior = 0.;
        
        i_intersection_coords_at_hedge(
                        type_of_containment_at_face,
                        hit_hedge_at_face,
                        t_relative_to_hit_hedge_at_face,
                        &x_edge_interior_hedge_at_face, &y_edge_interior_hedge_at_face, &z_edge_interior_hedge_at_face);
        
        i_append_new_edge_intersection(
                        intersection_position_at_edge,
                        vertex,
                        x_edge_interior, y_edge_interior, z_edge_interior,
                        t_vertex_on_edge,
                        face_B,
                        type_of_containment_at_face,
                        hit_vertex_at_face,
                        hit_hedge_at_face, x_edge_interior_hedge_at_face, y_edge_interior_hedge_at_face, z_edge_interior_hedge_at_face,
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
    double x1, y1, z1, x2, y2, z2;
    
    hedge_pos = csmedge_hedge_lado(edge_A, CSMEDGE_LADO_HEDGE_POS);
    vertex_pos = csmhedge_vertex(hedge_pos);
    csmvertex_get_coordenadas(vertex_pos, &x1, &y1, &z1);
    
    hedge_neg = csmedge_hedge_lado(edge_A, CSMEDGE_LADO_HEDGE_NEG);
    vertex_neg = csmhedge_vertex(hedge_neg);
    csmvertex_get_coordenadas(vertex_neg, &x2, &y2, &z2);
    
    if (csmface_should_analyze_intersections_with_segment(face_B, x1, y1, z1, x2, y2, z2) == CSMTRUE)
    {
        enum csmmath_double_relation_t classification_vertex_pos, classification_vertex_neg;
        
        classification_vertex_pos = csmface_classify_vertex_relative_to_face(face_B, vertex_pos);
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
            double x_inters, y_inters, z_inters, t;
        
            if (csmface_exists_intersection_between_line_and_face_plane(
                            face_B,
                            x1, y1, z1, x2, y2, z2,
                            &x_inters, &y_inters, &z_inters, &t) == CSMTRUE)
            {
                enum csmmath_contaiment_point_loop_t type_of_containment_at_face;
                struct csmvertex_t *hit_vertex_at_face;
                struct csmhedge_t *hit_hedge_at_face;
                double t_relative_to_hit_hedge_at_face;
                
                hit_vertex_at_face = NULL;
                hit_hedge_at_face = NULL;
                
                if (csmface_contains_point(
                            face_B,
                            x_inters, y_inters, z_inters,
                            &type_of_containment_at_face,
                            &hit_vertex_at_face,
                            &hit_hedge_at_face, &t_relative_to_hit_hedge_at_face) == CSMTRUE)
                {
                    enum i_intersection_position_t intersection_position_at_edge;
                    struct csmvertex_t *edge_vertex;
                    double x_edge_interior_hedge_at_face, y_edge_interior_hedge_at_face, z_edge_interior_hedge_at_face;
                    
                    intersection_position_at_edge = i_INTERSECTION_POSITION_AT_EDGE_INTERIOR;
                    edge_vertex = NULL;
                    
                    i_intersection_coords_at_hedge(
                            type_of_containment_at_face,
                            hit_hedge_at_face,
                            t_relative_to_hit_hedge_at_face,
                            &x_edge_interior_hedge_at_face, &y_edge_interior_hedge_at_face, &z_edge_interior_hedge_at_face);
                    
                    i_append_new_edge_intersection(
                            intersection_position_at_edge,
                            edge_vertex,
                            x_inters, y_inters, z_inters,
                            t,
                            face_B,
                            type_of_containment_at_face,
                            hit_vertex_at_face,
                            hit_hedge_at_face,
                            x_edge_interior_hedge_at_face, y_edge_interior_hedge_at_face, z_edge_interior_hedge_at_face,
                            edge_intersections);
                }
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
    
    switch (csmmath_compare_doubles(edge_intersection1->t_intersection_on_edge, edge_intersection2->t_intersection_on_edge, tolerance))
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

static bool i_csmsetop_vtxfacc_equals(const struct csmsetop_vtxfacc_inters_t *vf_inters1, const struct csmsetop_vtxfacc_inters_t *vf_inters2)
{
    return csmsetop_vtxfacc_equals(vf_inters1, vf_inters2);
}

// ------------------------------------------------------------------------------------------

static void i_append_new_vf_inters(
                        struct csmvertex_t *vertex_a, struct csmface_t *face_b,
                        ArrEstructura(csmsetop_vtxfacc_inters_t) *vertex_face_neighborhood,
                        CSMBOOL *added)
{
    struct csmsetop_vtxfacc_inters_t *vf_inters;
    
    assert_no_null(added);
    
    vf_inters = csmsetop_vtxfacc_create_inters(vertex_a, face_b);
    
    if (arr_ExisteEstructuraST(
                        vertex_face_neighborhood, csmsetop_vtxfacc_inters_t,
                        vf_inters, struct csmsetop_vtxfacc_inters_t,
                        i_csmsetop_vtxfacc_equals,
                        NULL) == CSMTRUE)
    {
        *added = CSMFALSE;
        csmsetop_vtxfacc_free_inters(&vf_inters);
    }
    else
    {
        *added = CSMTRUE;
        arr_AppendPunteroST(vertex_face_neighborhood, vf_inters, csmsetop_vtxfacc_inters_t);
    }
}

// ------------------------------------------------------------------------------------------

static void i_process_edge_intersections(
                        struct csmedge_t *original_edge,
                        const ArrEstructura(i_edge_intersection_t) *edge_intersecctions,
                        ArrEstructura(csmsetop_vtxvtx_inters_t) *vv_intersections,
                        ArrEstructura(csmsetop_vtxfacc_inters_t) *vertex_face_neighborhood)
{
    unsigned long i, num_intersections;
    struct csmedge_t *edge_to_split;
    
    num_intersections = arr_NumElemsPunteroST(edge_intersecctions, i_edge_intersection_t);
    assert(num_intersections > 0);
    
    if (csmdebug_debug_enabled() == CSMTRUE)
    {
        double x1, y1, z1, x2, y2, z2;
        
        csmedge_vertex_coordinates(original_edge, &x1, &y1, &z1, &x2, &y2, &z2);
        csmdebug_print_debug_info("\n");
        csmdebug_print_debug_info("Edge: %lu. (%lf, %lf, %lf) -> (%lf, %lf, %lf)\n", csmedge_id(original_edge), x1, y1, z1, x2, y2, z2);
        csmdebug_print_debug_info("No. intersections: %lu\n", num_intersections);
    }

    edge_to_split = original_edge;
    
    for (i = 0; i < num_intersections; i++)
    {
        const struct i_edge_intersection_t *edge_intersection;
        struct csmvertex_t *edge_vertex_intersection;
        
        edge_intersection = arr_GetPunteroConstST(edge_intersecctions, i, i_edge_intersection_t);
        assert_no_null(edge_intersection);

        if (csmdebug_debug_enabled() == CSMTRUE)
        {
            double A, B, C, D;
            
            csmface_face_equation(edge_intersection->face, &A, &B, &C, &D);
            csmdebug_print_debug_info("Intersection with face: %lu, Eq: (%lf, %lf, %lf, %lf)\n", csmface_id(edge_intersection->face), A, B, C, D);
        }
        
        switch (edge_intersection->intersection_position_at_edge)
        {
            case i_INTERSECTION_POSITION_AT_EDGE_VERTEX:
            {
                if (csmdebug_debug_enabled() == CSMTRUE)
                {
                    double x, y, z;
                
                    csmvertex_get_coordenadas(edge_intersection->edge_vertex, &x, &y, &z);
                    csmdebug_print_debug_info("Intersection at vertex: %lu (%lf, %lf, %lf)\n", csmvertex_id(edge_intersection->edge_vertex), x, y, z);
                	csmdebug_print_debug_info("-->NOTHING DONE\n");
                }
                
                edge_vertex_intersection = edge_intersection->edge_vertex;
                break;
            }
                
            case i_INTERSECTION_POSITION_AT_EDGE_INTERIOR:
            {
                struct csmhedge_t *hedge_pos, *hedge_neg;
                struct csmhedge_t *hedge_pos_next;
                struct csmvertex_t *new_vertex;
                struct csmedge_t *new_edge;
                
                {
                    double x1_esplit, y1_esplit, z1_esplit, x2_esplit, y2_esplit, z2_esplit;
        
                    csmedge_vertex_coordinates(edge_to_split, &x1_esplit, &y1_esplit, &z1_esplit, &x2_esplit, &y2_esplit, &z2_esplit);
                    
                    assert(csmmath_is_point_in_segment3D(
                                edge_intersection->x_edge_interior, edge_intersection->y_edge_interior, edge_intersection->z_edge_interior,
                                x1_esplit, y1_esplit, z1_esplit, x2_esplit, y2_esplit, z2_esplit,
                                csmtolerance_equal_coords(),
                                NULL) == CSMTRUE);
                }
            
                hedge_pos = csmedge_hedge_lado(edge_to_split, CSMEDGE_LADO_HEDGE_POS);
                hedge_pos_next = csmhedge_next(hedge_pos);
        
                hedge_neg = csmedge_hedge_lado(edge_to_split, CSMEDGE_LADO_HEDGE_NEG);
        
                csmeuler_lmev(
                        hedge_neg, hedge_pos_next,
                        edge_intersection->x_edge_interior, edge_intersection->y_edge_interior, edge_intersection->z_edge_interior,
                        &new_vertex,
                        &new_edge,
                        NULL, NULL);
        
                if (csmdebug_debug_enabled() == CSMTRUE)
                {
                    char *description;

                    csmdebug_print_debug_info("Intersection at inner point: (%lf, %lf, %lf)\n", edge_intersection->x_edge_interior, edge_intersection->y_edge_interior, edge_intersection->z_edge_interior);
                    csmdebug_print_debug_info("-->Created new vertex: %lu\n", csmvertex_id(new_vertex));
                    
                    description = copiafor_codigo4("IE %lu (%.3g, %.3g, %.3g)", csmvertex_id(new_vertex), edge_intersection->x_edge_interior, edge_intersection->y_edge_interior, edge_intersection->z_edge_interior);
                    csmdebug_append_debug_point(edge_intersection->x_edge_interior, edge_intersection->y_edge_interior, edge_intersection->z_edge_interior, &description);
                }
                
                edge_vertex_intersection = new_vertex;
                edge_to_split = new_edge;
                break;
            }
            
            default_error();
        }
        
        switch (edge_intersection->edge_intersection_at_face)
        {
            case i_TYPE_EDGE_INTERSECTION_VERTEX:
            {
                CSMBOOL did_add_intersection;
                
                i_append_new_vv_inters(edge_vertex_intersection, edge_intersection->hit_vertex_at_face, vv_intersections, &did_add_intersection);
                
                if (did_add_intersection == CSMTRUE && csmdebug_debug_enabled() == CSMTRUE)
                {
                    csmdebug_print_debug_info("Intersection at face vertex: %lu\n", csmvertex_id(edge_intersection->hit_vertex_at_face));
                    csmdebug_print_debug_info("Added VV intersection (%lu, %lu)\n", csmvertex_id(edge_vertex_intersection), csmvertex_id(edge_intersection->hit_vertex_at_face));
                }
                break;
            }
                
            case i_TYPE_EDGE_INTERSECTION_INTERIOR_EDGE:
            {
                struct csmhedge_t *hit_hedge_at_face_next, *mate_hit_hedge;
                struct csmvertex_t *new_vertex_on_hit_hedge;
                struct csmedge_t *new_edge_other_solid;
                CSMBOOL did_add_intersection;
                
                if (csmdebug_debug_enabled() == CSMTRUE)
                    csmdebug_print_debug_info("Intersection at face hedge: %lu\n", csmhedge_id(edge_intersection->hit_hedge_at_face));
                
                {
                    struct csmedge_t *edge_to_split_other_solid;
                    double x1_esplit, y1_esplit, z1_esplit, x2_esplit, y2_esplit, z2_esplit;
        
                    edge_to_split_other_solid = csmhedge_edge(edge_intersection->hit_hedge_at_face);
                    csmedge_vertex_coordinates(edge_to_split_other_solid, &x1_esplit, &y1_esplit, &z1_esplit, &x2_esplit, &y2_esplit, &z2_esplit);
                    
                    assert(csmmath_is_point_in_segment3D(
                                edge_intersection->x_edge_interior_hedge_at_face, edge_intersection->y_edge_interior_hedge_at_face, edge_intersection->z_edge_interior_hedge_at_face,
                                x1_esplit, y1_esplit, z1_esplit, x2_esplit, y2_esplit, z2_esplit,
                                csmtolerance_equal_coords(),
                                NULL) == CSMTRUE);
                }
                
                hit_hedge_at_face_next = csmhedge_next(edge_intersection->hit_hedge_at_face);
                mate_hit_hedge = csmopbas_mate(edge_intersection->hit_hedge_at_face);
                
                csmeuler_lmev(
                          mate_hit_hedge, hit_hedge_at_face_next,
                          edge_intersection->x_edge_interior_hedge_at_face, edge_intersection->y_edge_interior_hedge_at_face, edge_intersection->z_edge_interior_hedge_at_face,
                          &new_vertex_on_hit_hedge,
                          &new_edge_other_solid,
                          NULL, NULL);
                
                if (csmdebug_debug_enabled() == CSMTRUE)
                {
                    char *description;
                    
                    csmdebug_print_debug_info("-->Splitted hedge, new vertex: %lu\n", csmvertex_id(new_vertex_on_hit_hedge));
                    csmedge_print_debug_info(new_edge_other_solid, CSMTRUE);
                    csmdebug_print_debug_info("Added VV intersection (%lu, %lu)\n", csmvertex_id(edge_vertex_intersection), csmvertex_id(new_vertex_on_hit_hedge));
                    
                    description = copiafor_codigo4(
                            "IE-F %lu (%.3g, %.3g, %.3g)",
                            csmvertex_id(new_vertex_on_hit_hedge),
                            edge_intersection->x_edge_interior_hedge_at_face, edge_intersection->y_edge_interior_hedge_at_face, edge_intersection->z_edge_interior_hedge_at_face);
                    
                    csmdebug_append_debug_point(
                            edge_intersection->x_edge_interior_hedge_at_face, edge_intersection->y_edge_interior_hedge_at_face, edge_intersection->z_edge_interior_hedge_at_face,
                            &description);
                }
                
                i_append_new_vv_inters(edge_vertex_intersection, new_vertex_on_hit_hedge, vv_intersections, &did_add_intersection);
                assert(did_add_intersection == CSMTRUE);
                break;
            }
                
            case i_TYPE_EDGE_INTERSECTION_INTERIOR_FACE:
            {
                CSMBOOL added;
                
                i_append_new_vf_inters(edge_vertex_intersection, edge_intersection->face, vertex_face_neighborhood, &added);
                
                if (added == CSMTRUE && csmdebug_debug_enabled() == CSMTRUE)
                {
                    char *description;
                    double x_edge_intersection, y_edge_intersection, z_edge_intersection;

                    csmvertex_get_coordenadas(edge_vertex_intersection, &x_edge_intersection, &y_edge_intersection, &z_edge_intersection);
                    
                    csmdebug_print_debug_info(" Intersection at interior of face\n");
                    
                    description = copiafor_codigo4(
                                    "VF %lu (%.3g, %.3g, %.3g)",
                                    csmvertex_id(edge_vertex_intersection),
                                    x_edge_intersection, y_edge_intersection, z_edge_intersection);
                    
                    csmdebug_append_debug_point(x_edge_intersection, y_edge_intersection, z_edge_intersection, &description);
                }
                break;
            }
                
            default_error();
        }
        
        csmdebug_print_debug_info("\n");
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

    while (csmhashtb_has_next(face_iterator_B, csmface_t) == CSMTRUE)
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
    
    while (csmhashtb_has_next(edge_iterator_A, csmedge_t) == CSMTRUE)
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

    csmdebug_begin_context("csmsetop_procedges_generate_intersections_on_both_solids");
    {
        csmdebug_begin_context("Intersections A vs B");
        {
            csmdebug_clear_debug_points();
            
            i_generate_edge_intersections_solid_A_with_solid_B(solid_A, solid_B, vv_intersections_loc, vf_intersections_A_loc);
            
            csmsolid_print_debug(solid_A, CSMTRUE);
            csmsolid_print_debug(solid_B, CSMTRUE);
        }
        csmdebug_end_context();
    
        //csmdebug_show_viewer();
        
        csmdebug_begin_context("Intersections B vs A");
        {
            csmdebug_clear_debug_points();
            i_generate_edge_intersections_solid_A_with_solid_B(solid_B, solid_A, vv_intersections_loc, vf_intersections_B_loc);
            
            csmsolid_print_debug(solid_A, CSMTRUE);
            csmsolid_print_debug(solid_B, CSMTRUE);
        }
        csmdebug_end_context();

        //csmdebug_show_viewer();
        
        i_append_common_vertices_solid_A_and_B_not_previously_found(solid_A, solid_B, vv_intersections_loc);
    }
    csmdebug_end_context();
    
    *vv_intersections = vv_intersections_loc;
    *vf_intersections_A = vf_intersections_A_loc;
    *vf_intersections_B = vf_intersections_B_loc;
}
