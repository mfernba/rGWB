 //
//  csmsetop_procedges.c
//  rGWB
//
//  Created by Manuel Fernández on 22/5/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmsetop_procedges.inl"

#include "csmarrayc.inl"
#include "csmbbox.inl"
#include "csmdebug.inl"
#include "csmedge.inl"
#include "csmedge.tli"
#include "csmedge_debug.inl"
#include "csmeuler_lmev.inl"
#include "csmface.inl"
#include "csmhashtb.inl"
#include "csmhedge.inl"
#include "csmid.inl"
#include "csmmath.inl"
#include "csmmath.tli"
#include "csmopbas.inl"
#include "csmsetop.tli"
#include "csmsetop_vtxfacc.inl"
#include "csmsetop_vtxvtx.inl"
#include "csmsolid.inl"
#include "csmsolid_debug.inl"
#include "csmtolerance.inl"
#include "csmvertex.inl"
#include "csmvertex.tli"

#ifdef __STANDALONE_DISTRIBUTABLE
#include "csmassert.inl"
#include "csmmem.inl"
#include "csmstring.inl"
#else
#include "cyassert.h"
#include "cypespy.h"
#include "copiafor.h"
#endif

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
    unsigned long intersection_id;
    CSMBOOL generate_intersection;
    
    enum i_intersection_position_t intersection_position_at_edge;
    struct csmvertex_t *edge_vertex;
    double x_edge_interior, y_edge_interior, z_edge_interior;
    double t_absolute_intersection_on_edge;
    
    struct csmface_t *face;
    enum i_type_edge_intersection_t edge_intersection_at_face;
    struct csmvertex_t *hit_vertex_at_face;
    struct csmhedge_t *hit_hedge_at_face;
    double x_edge_interior_hedge_at_face, y_edge_interior_hedge_at_face, z_edge_interior_hedge_at_face;
    
    double tolerance_equal_coords;
};

struct i_optimized_edge_data_t
{
    struct csmvertex_t *vertex_pos, *vertex_neg;
    unsigned long edge_id;
    double x1, y1, z1, x2, y2, z2;
    double length;
};

/*
struct i_inters_vertex_t
{
    struct csmvertex_t *vertex;
    struct csmhashtb(csmvertex_t) *overlapped_vertexs;
};
*/

csmArrayStruct(i_edge_intersection_t);

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_edge_intersection_t *, i_create_edge_intersection, (
                        unsigned long intersection_id,
                        CSMBOOL generate_intersection,
                        enum i_intersection_position_t intersection_position_at_edge,
                        struct csmvertex_t *edge_vertex,
                        double x_edge_interior, double y_edge_interior, double z_edge_interior,
                        double t_absolute_intersection_on_edge,
                        struct csmface_t *face,
                        enum i_type_edge_intersection_t edge_intersection_at_face,
                        struct csmvertex_t *hit_vertex_at_face,
                        struct csmhedge_t *hit_hedge_at_face,
                        double x_edge_interior_hedge_at_face, double y_edge_interior_hedge_at_face, double z_edge_interior_hedge_at_face,
                        double tolerance_equal_coords))
{
    struct i_edge_intersection_t *edge_intersection;
    
    edge_intersection = MALLOC(struct i_edge_intersection_t);
    
    edge_intersection->intersection_id = intersection_id;
    edge_intersection->generate_intersection = generate_intersection;
    
    edge_intersection->intersection_position_at_edge = intersection_position_at_edge;
    edge_intersection->edge_vertex = edge_vertex;
    edge_intersection->x_edge_interior = x_edge_interior;
    edge_intersection->y_edge_interior = y_edge_interior;
    edge_intersection->z_edge_interior = z_edge_interior;
    edge_intersection->t_absolute_intersection_on_edge = t_absolute_intersection_on_edge;
    
    edge_intersection->face = face;
    edge_intersection->edge_intersection_at_face = edge_intersection_at_face;
    edge_intersection->hit_vertex_at_face = hit_vertex_at_face;
    edge_intersection->hit_hedge_at_face = hit_hedge_at_face;
    edge_intersection->x_edge_interior_hedge_at_face = x_edge_interior_hedge_at_face;
    edge_intersection->y_edge_interior_hedge_at_face = y_edge_interior_hedge_at_face;
    edge_intersection->z_edge_interior_hedge_at_face = z_edge_interior_hedge_at_face;
    
    edge_intersection->tolerance_equal_coords = tolerance_equal_coords;
    
    return edge_intersection;
}

// ------------------------------------------------------------------------------------------

static void i_free_edge_intersection(struct i_edge_intersection_t **edge_intersection)
{
    assert_no_null(edge_intersection);
    assert_no_null(*edge_intersection);
    
    FREE_PP(edge_intersection, struct i_edge_intersection_t);
}

/*
// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_inters_vertex_t *, i_new_inters_vertex, (struct csmvertex_t *vertex, struct csmhashtb(csmvertex_t) **overlapped_vertexs))
{
    struct i_inters_vertex_t *inters_vertex;
    
    inters_vertex = MALLOC(struct i_inters_vertex_t);
    
    inters_vertex->vertex = vertex;
    inters_vertex->overlapped_vertexs = ASIGNA_PUNTERO_PP_NO_NULL(overlapped_vertexs, struct csmhashtb(csmvertex_t));
    
    return inters_vertex;
}

// ------------------------------------------------------------------------------------------

static void i_free_inters_vertex(struct i_inters_vertex_t **inters_vertex)
{
    assert_no_null(inters_vertex);
    assert_no_null(*inters_vertex);
    
    csmhashtb_free(&(*inters_vertex)->overlapped_vertexs, csmvertex_t, NULL);
    
    FREE_PP(inters_vertex, struct i_inters_vertex_t);
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_inters_vertex_t *, i_new_inters_vertex_empty, (struct csmvertex_t *vertex))
{
    struct csmhashtb(csmvertex_t) *overlapped_vertexs;
    
    overlapped_vertexs = csmhashtb_create_empty(csmvertex_t);
    return i_new_inters_vertex(vertex, &overlapped_vertexs);
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmhashtb(i_inters_vertex_t) *, i_new_inters_vertex_table, (void))
{
    return csmhashtb_create_empty(i_inters_vertex_t);
}
*/

// ------------------------------------------------------------------------------------------

static void i_append_new_vv_inters(
                        unsigned long intersection_id,
                        struct csmvertex_t *vertex_a, struct csmvertex_t *vertex_b,
                        CSMBOOL is_A_vs_B,
                        csmArrayStruct(csmsetop_vtxvtx_inters_t) *vv_intersections,
                        double tolerance_equal_coords,
                        CSMBOOL *did_add_intersection)
{
    assert_no_null(did_add_intersection);
    assert(csmvertex_equal_coords(vertex_a, vertex_b, 2. * tolerance_equal_coords) == CSMTRUE);
    
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
    
        if (is_A_vs_B == CSMTRUE)
            vv_inters = csmsetop_vtxvtx_create_inters(intersection_id, vertex_a, vertex_b);
        else
            vv_inters = csmsetop_vtxvtx_create_inters(intersection_id, vertex_b, vertex_a);
        
        csmarrayc_append_element_st(vv_intersections, vv_inters, csmsetop_vtxvtx_inters_t);
    }
}

// ------------------------------------------------------------------------------------------

static void i_append_common_vertices_solid_A_on_solid_B(
                        struct csmsolid_t *solid_A, struct csmsolid_t *solid_B,
                        const struct csmtolerance_t *tolerances,
                        unsigned long *id_new_intersection,
                        csmArrayStruct(csmsetop_vtxvtx_inters_t) *vv_intersections,
                        CSMBOOL *did_find_non_manifold_operand_solid)
{
    double tolerance;
    struct csmhashtb_iterator(csmvertex_t) *vertex_iterator_A;
    
    assert_no_null(did_find_non_manifold_operand_solid);
    *did_find_non_manifold_operand_solid = CSMFALSE;
    
    tolerance = csmtolerance_equal_coords(tolerances);
    vertex_iterator_A = csmsolid_vertex_iterator(solid_A);
    
    while (csmhashtb_has_next(vertex_iterator_A, csmvertex_t) == CSMTRUE)
    {
        struct csmvertex_t *vertex_A, *vertex_B;
        
        csmhashtb_next_pair(vertex_iterator_A, NULL, &vertex_A, csmvertex_t);
        
        if (csmvertex_has_mask_attrib(vertex_A, CSMVERTEX_MASK_SETOP_COMMON_VERTEX) == CSMFALSE
                && csmsolid_contains_vertex_in_same_coordinates_as_given(solid_B, vertex_A, tolerance, &vertex_B) == CSMTRUE)
        {
            if (csmvertex_has_mask_attrib(vertex_B, CSMVERTEX_MASK_SETOP_COMMON_VERTEX) == CSMFALSE)
            {
                unsigned long intersection_id;
                CSMBOOL is_A_vs_B;
                CSMBOOL did_add_intersection;
                
                intersection_id = csmid_new_id(id_new_intersection, NULL);
                is_A_vs_B = CSMTRUE;
                i_append_new_vv_inters(intersection_id, vertex_A, vertex_B, is_A_vs_B, vv_intersections, tolerance, &did_add_intersection);
                assert(did_add_intersection == CSMTRUE);
                
                if (csmdebug_debug_enabled() == CSMTRUE)
                {
                    char *description;
                    double x, y, z;
                    
                    csmvertex_get_coordenadas(vertex_A, &x, &y, &z);
                    description = copiafor_codigo5("EQ VV (%g, %g, %g) %lu %lu", x, y, z, csmvertex_id(vertex_A), csmvertex_id(vertex_B));
                    csmdebug_append_debug_point(x, y, z, &description);
                
                    csmdebug_print_debug_info("-->Coincident vertices: (%lu, %lu)\n", csmvertex_id(vertex_A), csmvertex_id(vertex_B));
                }
            }
            else
            {
                *did_find_non_manifold_operand_solid = CSMTRUE;
                break;
            }
        }
    }
    
    csmhashtb_free_iterator(&vertex_iterator_A, csmvertex_t);
}

// ------------------------------------------------------------------------------------------

static void i_append_common_vertices_solid_A_and_B_not_previously_found(
                        struct csmsolid_t *solid_A, struct csmsolid_t *solid_B,
                        const struct csmtolerance_t *tolerances,
                        unsigned long *id_new_intersection,
                        csmArrayStruct(csmsetop_vtxvtx_inters_t) *vv_intersections,
                        CSMBOOL *did_find_non_manifold_operand_solid_A,
                        CSMBOOL *did_find_non_manifold_operand_solid_B)
{
    i_append_common_vertices_solid_A_on_solid_B(solid_A, solid_B, tolerances, id_new_intersection, vv_intersections, did_find_non_manifold_operand_solid_A);
    i_append_common_vertices_solid_A_on_solid_B(solid_B, solid_A, tolerances, id_new_intersection, vv_intersections, did_find_non_manifold_operand_solid_B);
}

// ------------------------------------------------------------------------------------------

static CSMBOOL i_equal_edge_intersection(const struct i_edge_intersection_t *edge_intersection1, const struct i_edge_intersection_t *edge_intersection2)
{
    assert_no_null(edge_intersection1);
    assert_no_null(edge_intersection2);
    assert(edge_intersection1->tolerance_equal_coords == edge_intersection2->tolerance_equal_coords);
    
    return csmmath_equal_coords(
                        edge_intersection1->x_edge_interior, edge_intersection1->y_edge_interior, edge_intersection1->z_edge_interior,
                        edge_intersection2->x_edge_interior, edge_intersection2->y_edge_interior, edge_intersection2->z_edge_interior,
                        edge_intersection1->tolerance_equal_coords);
}

// ------------------------------------------------------------------------------------------

static void i_append_new_edge_intersection(
                        enum i_intersection_position_t intersection_position_at_edge,
                        struct csmvertex_t *edge_vertex,
                        double x_edge_interior, double y_edge_interior, double z_edge_interior,
                        double t_absolute_intersection_on_edge,
                        struct csmface_t *face,
                        enum csmmath_contaiment_point_loop_t type_of_containment_at_face,
                        struct csmvertex_t *hit_vertex_at_face,
                        struct csmhedge_t *hit_hedge_at_face,
                        double x_edge_interior_hedge_at_face, double y_edge_interior_hedge_at_face, double z_edge_interior_hedge_at_face,
                        const struct csmtolerance_t *tolerances,
                        unsigned long *id_new_intersection,
                        csmArrayStruct(i_edge_intersection_t) *edge_intersections)
{
    CSMBOOL generate_intersection;
    enum i_type_edge_intersection_t edge_intersection_at_face;
    struct i_edge_intersection_t *edge_intersection;
    unsigned long intersection_id;
    double tolerance_equal_coords;
    
    generate_intersection = CSMTRUE;
    
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
    
    intersection_id = csmid_new_id(id_new_intersection, NULL);
    tolerance_equal_coords = csmtolerance_equal_coords(tolerances);
    
    edge_intersection = i_create_edge_intersection(
                        intersection_id,
                        generate_intersection,
                        intersection_position_at_edge,
                        edge_vertex,
                        x_edge_interior, y_edge_interior, z_edge_interior,
                        t_absolute_intersection_on_edge,
                        face,
                        edge_intersection_at_face,
                        hit_vertex_at_face,
                        hit_hedge_at_face,
                        x_edge_interior_hedge_at_face, y_edge_interior_hedge_at_face, z_edge_interior_hedge_at_face,
                        tolerance_equal_coords);
    
    if (csmarrayc_contains_element_st(
                        edge_intersections, i_edge_intersection_t,
                        edge_intersection, struct i_edge_intersection_t,
                        i_equal_edge_intersection,
                        NULL) == CSMTRUE)
    {
        i_free_edge_intersection(&edge_intersection);
    }
    else
    {
        csmarrayc_append_element_st(edge_intersections, edge_intersection, i_edge_intersection_t);
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
                        double t_absolute_vertex_on_edge,
                        const struct csmtolerance_t *tolerances,
                        unsigned long *id_new_intersection,
                        csmArrayStruct(i_edge_intersection_t) *edge_intersections)
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
                        tolerances,
                        &type_of_containment_at_face,
                        &hit_vertex_at_face,
                        &hit_hedge_at_face, &t_relative_to_hit_hedge_at_face) == CSMTRUE)
    {
        enum i_intersection_position_t intersection_position_at_edge;
        double x_edge_interior, y_edge_interior, z_edge_interior;
        double x_edge_interior_hedge_at_face, y_edge_interior_hedge_at_face, z_edge_interior_hedge_at_face;
        
        intersection_position_at_edge = i_INTERSECTION_POSITION_AT_EDGE_VERTEX;
        csmvertex_get_coordenadas(vertex, &x_edge_interior, &y_edge_interior,  &z_edge_interior);
        
        i_intersection_coords_at_hedge(
                        type_of_containment_at_face,
                        hit_hedge_at_face,
                        t_relative_to_hit_hedge_at_face,
                        &x_edge_interior_hedge_at_face, &y_edge_interior_hedge_at_face, &z_edge_interior_hedge_at_face);
        
        i_append_new_edge_intersection(
                        intersection_position_at_edge,
                        vertex,
                        x_edge_interior, y_edge_interior, z_edge_interior,
                        t_absolute_vertex_on_edge,
                        face_B,
                        type_of_containment_at_face,
                        hit_vertex_at_face,
                        hit_hedge_at_face, x_edge_interior_hedge_at_face, y_edge_interior_hedge_at_face, z_edge_interior_hedge_at_face,
                        tolerances,
                        id_new_intersection,
                        edge_intersections);
    }
}

// ------------------------------------------------------------------------------------------

static void i_append_intersections_between_A_edge_and_B_face(
                        const struct i_optimized_edge_data_t *optimized_edge_data,
                        struct csmface_t *face_B,
                        const struct csmtolerance_t *tolerances,
                        unsigned long *id_new_intersection,
                        csmArrayStruct(i_edge_intersection_t) *edge_intersections)
{
    assert_no_null(optimized_edge_data);
    
    if (csmface_should_analyze_intersections_with_segment(
	                    face_B,
                        optimized_edge_data->x1, optimized_edge_data->y1, optimized_edge_data->z1,
                        optimized_edge_data->x2, optimized_edge_data->y2, optimized_edge_data->z2) == CSMTRUE)
    {
        enum csmcompare_t classification_vertex_pos, classification_vertex_neg;
        
        classification_vertex_pos = csmface_classify_vertex_relative_to_face(face_B, optimized_edge_data->vertex_pos);
        classification_vertex_neg = csmface_classify_vertex_relative_to_face(face_B, optimized_edge_data->vertex_neg);
        
        if (classification_vertex_pos == CSMCOMPARE_EQUAL || classification_vertex_neg == CSMCOMPARE_EQUAL)
        {
            if (classification_vertex_pos == CSMCOMPARE_EQUAL)
            {
                i_append_intersection_between_vertex_A_and_face_B(
                        optimized_edge_data->vertex_pos,
                        face_B,
                        0.,
                        tolerances,
                        id_new_intersection,
                        edge_intersections);
            }

            if (classification_vertex_neg == CSMCOMPARE_EQUAL)
            {
                i_append_intersection_between_vertex_A_and_face_B(
                        optimized_edge_data->vertex_neg,
                        face_B,
                        optimized_edge_data->length,
                        tolerances,
                        id_new_intersection,
                        edge_intersections);
            }
        }
        else
        {
            double x_inters, y_inters, z_inters, t;
        
            if (csmface_exists_intersection_between_line_and_face_plane(
                        face_B,
                        optimized_edge_data->x1, optimized_edge_data->y1, optimized_edge_data->z1,
                        optimized_edge_data->x2, optimized_edge_data->y2, optimized_edge_data->z2,
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
                                tolerances,
                                &type_of_containment_at_face,
                                &hit_vertex_at_face,
                                &hit_hedge_at_face, &t_relative_to_hit_hedge_at_face) == CSMTRUE)
                {
                    enum i_intersection_position_t intersection_position_at_edge;
                    struct csmvertex_t *edge_vertex;
                    double x_edge_interior_hedge_at_face, y_edge_interior_hedge_at_face, z_edge_interior_hedge_at_face;
                    double tolerance_equal_coords;

                    tolerance_equal_coords = csmtolerance_equal_coords(tolerances);
                
                    if (csmmath_equal_coords(
                                optimized_edge_data->x1, optimized_edge_data->y1, optimized_edge_data->z1,
                                x_inters, y_inters, z_inters,
                                tolerance_equal_coords) == CSMTRUE)
                    {
                        intersection_position_at_edge = i_INTERSECTION_POSITION_AT_EDGE_VERTEX;
                        
                        edge_vertex = optimized_edge_data->vertex_pos;
                        csmvertex_get_coordenadas(optimized_edge_data->vertex_pos, &x_inters, &y_inters, &z_inters);
                        t = 0.;
                    }
                    else if (csmmath_equal_coords(
                                optimized_edge_data->x2, optimized_edge_data->y2, optimized_edge_data->z2,
                                x_inters, y_inters, z_inters,
                                tolerance_equal_coords) == CSMTRUE)
                    {
                        intersection_position_at_edge = i_INTERSECTION_POSITION_AT_EDGE_VERTEX;
                        
                        edge_vertex = optimized_edge_data->vertex_neg;
                        csmvertex_get_coordenadas(optimized_edge_data->vertex_neg, &x_inters, &y_inters, &z_inters);
                        t = 1.;
                    }
                    else
                    {
                        intersection_position_at_edge = i_INTERSECTION_POSITION_AT_EDGE_INTERIOR;
                        edge_vertex = NULL;
                    }
                        
                    i_intersection_coords_at_hedge(
                                type_of_containment_at_face,
                                hit_hedge_at_face,
                                t_relative_to_hit_hedge_at_face,
                                &x_edge_interior_hedge_at_face, &y_edge_interior_hedge_at_face, &z_edge_interior_hedge_at_face);
                        
                    i_append_new_edge_intersection(
                                intersection_position_at_edge,
                                edge_vertex,
                                x_inters, y_inters, z_inters,
                                t * optimized_edge_data->length,
                                face_B,
                                type_of_containment_at_face,
                                hit_vertex_at_face,
                                hit_hedge_at_face,
                                x_edge_interior_hedge_at_face, y_edge_interior_hedge_at_face, z_edge_interior_hedge_at_face,
                                tolerances,
                                id_new_intersection,
                                edge_intersections);
                }
            }
        }
    }
}

// ------------------------------------------------------------------------------------------
    
static enum csmcompare_t i_compara_edge_intersection(
                        const struct i_edge_intersection_t *edge_intersection1,
                        const struct i_edge_intersection_t *edge_intersection2)
{
    double tolerance;
    
    assert_no_null(edge_intersection1);
    assert_no_null(edge_intersection2);
    
    tolerance = 1.e-9;
    return csmmath_compare_doubles(edge_intersection1->t_absolute_intersection_on_edge, edge_intersection2->t_absolute_intersection_on_edge, tolerance);
}

// ------------------------------------------------------------------------------------------

static void i_append_new_vf_inters(
                        unsigned long intersection_id,
                        struct csmvertex_t *vertex_a, struct csmface_t *face_b,
                        csmArrayStruct(csmsetop_vtxfacc_inters_t) *vertex_face_neighborhood,
                        CSMBOOL *added)
{
    struct csmsetop_vtxfacc_inters_t *vf_inters;
    
    assert_no_null(added);
    
    vf_inters = csmsetop_vtxfacc_create_inters(intersection_id, vertex_a, face_b);
    
    if (csmarrayc_contains_element_st(
                        vertex_face_neighborhood, csmsetop_vtxfacc_inters_t,
                        vf_inters, struct csmsetop_vtxfacc_inters_t,
                        csmsetop_vtxfacc_equals,
                        NULL) == CSMTRUE)
    {
        *added = CSMFALSE;
        csmsetop_vtxfacc_free_inters(&vf_inters);
    }
    else
    {
        *added = CSMTRUE;
        csmarrayc_append_element_st(vertex_face_neighborhood, vf_inters, csmsetop_vtxfacc_inters_t);
    }
}

// ------------------------------------------------------------------------------------------

static void i_discard_invalid_intersections(csmArrayStruct(i_edge_intersection_t) *edge_intersecctions)
{
    unsigned long i, num_intersections;
    
    num_intersections = csmarrayc_count_st(edge_intersecctions, i_edge_intersection_t);
    assert(num_intersections > 0);
    
    for (i = 0; i < num_intersections; i++)
    {
        struct i_edge_intersection_t *edge_intersection_i;
        
        edge_intersection_i = csmarrayc_get_st(edge_intersecctions, i, i_edge_intersection_t);
        assert_no_null(edge_intersection_i);
        
        if (edge_intersection_i->generate_intersection == CSMTRUE
                && edge_intersection_i->edge_intersection_at_face == i_TYPE_EDGE_INTERSECTION_INTERIOR_FACE)
        {
            unsigned long j;
            
            for (j = 0; j < num_intersections; j++)
            {
                if (i != j)
                {
                    const struct i_edge_intersection_t *edge_intersection_j;
                    
                    edge_intersection_j = csmarrayc_get_const_st(edge_intersecctions, j, i_edge_intersection_t);
                    assert_no_null(edge_intersection_j);
                    
                    if (edge_intersection_j->generate_intersection == CSMTRUE
                            && edge_intersection_j->edge_intersection_at_face == i_TYPE_EDGE_INTERSECTION_INTERIOR_EDGE)
                    {
                        struct csmedge_t *edge_j;
                        struct csmhedge_t *he1_j, *he2_j;
                        
                        edge_j = csmhedge_edge(edge_intersection_j->hit_hedge_at_face);
                        he1_j = csmedge_hedge_lado(edge_j, CSMEDGE_LADO_HEDGE_POS);
                        he2_j = csmedge_hedge_lado(edge_j, CSMEDGE_LADO_HEDGE_NEG);
                        
                        if (csmopbas_face_from_hedge(he1_j) == edge_intersection_i->face
                                || csmopbas_face_from_hedge(he2_j) == edge_intersection_i->face)
                        {
                            edge_intersection_i->generate_intersection = CSMFALSE;
                            break;
                        }
                    }
                }
            }
        }
    }
}

// ------------------------------------------------------------------------------------------

static void i_update_splitted_hedge_on_intersections(
                        const csmArrayStruct(i_edge_intersection_t) *edge_intersecctions,
                        unsigned long start_idx,
                        struct csmhedge_t *hit_hedge_at_face_to_replace, struct csmedge_t *hit_edge_at_face_replacement)
{
    unsigned long i, num_intersections;
    struct csmedge_t *hit_edge_at_face_to_replace;
    struct csmhedge_t *mate_hit_hedge_at_face_to_replace;
    
    num_intersections = csmarrayc_count_st(edge_intersecctions, i_edge_intersection_t);
    assert(num_intersections > 0);

    hit_edge_at_face_to_replace = csmhedge_edge(hit_hedge_at_face_to_replace);
    mate_hit_hedge_at_face_to_replace = csmopbas_mate(hit_hedge_at_face_to_replace);

    for (i = start_idx; i < num_intersections; i++)
    {
        struct i_edge_intersection_t *edge_intersection;
        
        edge_intersection = csmarrayc_get_st(edge_intersecctions, i, i_edge_intersection_t);
        assert_no_null(edge_intersection);

        switch (edge_intersection->edge_intersection_at_face)
        {
            case i_TYPE_EDGE_INTERSECTION_INTERIOR_EDGE:

                if (edge_intersection->hit_hedge_at_face == hit_hedge_at_face_to_replace
                        || edge_intersection->hit_hedge_at_face == mate_hit_hedge_at_face_to_replace)
                {
                    double x1_esplit, y1_esplit, z1_esplit, x2_esplit, y2_esplit, z2_esplit;

                    csmedge_vertex_coordinates(
                                hit_edge_at_face_to_replace,
                                &x1_esplit, &y1_esplit, &z1_esplit, NULL, &x2_esplit, &y2_esplit, &z2_esplit, NULL);

                    if (csmmath_is_point_in_segment3D(
                                edge_intersection->x_edge_interior, edge_intersection->y_edge_interior, edge_intersection->z_edge_interior,
                                x1_esplit, y1_esplit, z1_esplit, x2_esplit, y2_esplit, z2_esplit,
                                edge_intersection->tolerance_equal_coords,
                                NULL) == CSMFALSE)
                    {
                        struct csmhedge_t *he_replacement;

                        csmedge_vertex_coordinates(
                                hit_edge_at_face_replacement,
                                &x1_esplit, &y1_esplit, &z1_esplit, NULL, &x2_esplit, &y2_esplit, &z2_esplit, NULL);

                        assert(csmmath_is_point_in_segment3D(
                                edge_intersection->x_edge_interior, edge_intersection->y_edge_interior, edge_intersection->z_edge_interior,
                                x1_esplit, y1_esplit, z1_esplit, x2_esplit, y2_esplit, z2_esplit,
                                edge_intersection->tolerance_equal_coords,
                                NULL) == CSMTRUE);

                        he_replacement = csmedge_hedge_lado(hit_edge_at_face_replacement, CSMEDGE_LADO_HEDGE_POS);

                        if (csmhedge_next(edge_intersection->hit_hedge_at_face) == he_replacement
                                    || csmhedge_prev(edge_intersection->hit_hedge_at_face) == he_replacement)
                        {
                            edge_intersection->hit_hedge_at_face = he_replacement;
                        }
                        else
                        {
                            he_replacement = csmedge_hedge_lado(hit_edge_at_face_replacement, CSMEDGE_LADO_HEDGE_NEG);

                            assert(csmhedge_next(edge_intersection->hit_hedge_at_face) == he_replacement
                                    || csmhedge_prev(edge_intersection->hit_hedge_at_face) == he_replacement);
                        }

                        assert(hit_hedge_at_face_to_replace != he_replacement);
                        edge_intersection->hit_hedge_at_face = he_replacement;
                    }
                }
                break;

            case i_TYPE_EDGE_INTERSECTION_VERTEX:
            case i_TYPE_EDGE_INTERSECTION_INTERIOR_FACE:
                break;

            default_error();
        }
    }
}

// ------------------------------------------------------------------------------------------

static void i_process_edge_intersections(
                        struct csmedge_t *original_edge,
                        csmArrayStruct(i_edge_intersection_t) *edge_intersecctions,
                        CSMBOOL is_A_vs_B,
                        csmArrayStruct(csmsetop_vtxvtx_inters_t) *vv_intersections,
                        csmArrayStruct(csmsetop_vtxfacc_inters_t) *vertex_face_neighborhood,
                        CSMBOOL *did_find_non_manifold_operand_update)
{
    unsigned long i, num_intersections;
    struct csmedge_t *edge_to_split;
    
    num_intersections = csmarrayc_count_st(edge_intersecctions, i_edge_intersection_t);
    assert(num_intersections > 0);
    assert_no_null(did_find_non_manifold_operand_update);
    assert(*did_find_non_manifold_operand_update == CSMFALSE);
    
    if (csmdebug_debug_enabled() == CSMTRUE)
    {
        double x1, y1, z1, x2, y2, z2;
        unsigned long id_vertex1, id_vertex2;
        
        csmedge_vertex_coordinates(original_edge, &x1, &y1, &z1, &id_vertex1, &x2, &y2, &z2, &id_vertex2);
        csmdebug_print_debug_info("\n");
        csmdebug_print_debug_info("Edge: %lu. [%lu](%lf, %lf, %lf) -> [%lu](%lf, %lf, %lf)\n", csmedge_id(original_edge), id_vertex1, x1, y1, z1, id_vertex2, x2, y2, z2);
        csmdebug_print_debug_info("No. intersections: %lu\n", num_intersections);
    }

    edge_to_split = original_edge;
    
    for (i = 0; i < num_intersections && *did_find_non_manifold_operand_update == CSMFALSE; i++)
    {
        const struct i_edge_intersection_t *edge_intersection;
        
        edge_intersection = csmarrayc_get_const_st(edge_intersecctions, i, i_edge_intersection_t);
        assert_no_null(edge_intersection);
        
        if (edge_intersection->generate_intersection == CSMTRUE)
        {
            struct csmvertex_t *edge_vertex_intersection;
            
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
            
                        csmedge_vertex_coordinates(edge_to_split, &x1_esplit, &y1_esplit, &z1_esplit, NULL, &x2_esplit, &y2_esplit, &z2_esplit, NULL);
                        
                        assert(csmmath_equal_coords(
                                    x1_esplit, y1_esplit, z1_esplit,
                                    edge_intersection->x_edge_interior, edge_intersection->y_edge_interior, edge_intersection->z_edge_interior,
                                    edge_intersection->tolerance_equal_coords) == CSMFALSE);

                        assert(csmmath_equal_coords(
                                    x2_esplit, y2_esplit, z2_esplit,
                                    edge_intersection->x_edge_interior, edge_intersection->y_edge_interior, edge_intersection->z_edge_interior,
                                    edge_intersection->tolerance_equal_coords) == CSMFALSE);
                        
                        assert(csmmath_is_point_in_segment3D(
                                    edge_intersection->x_edge_interior, edge_intersection->y_edge_interior, edge_intersection->z_edge_interior,
                                    x1_esplit, y1_esplit, z1_esplit, x2_esplit, y2_esplit, z2_esplit,
                                    edge_intersection->tolerance_equal_coords,
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
                    
                    i_append_new_vv_inters(
                                    edge_intersection->intersection_id,
                                    edge_vertex_intersection, edge_intersection->hit_vertex_at_face,
                                    is_A_vs_B,
                                    vv_intersections,
                                    edge_intersection->tolerance_equal_coords,
                                    &did_add_intersection);
                    
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
                        csmedge_vertex_coordinates(edge_to_split_other_solid, &x1_esplit, &y1_esplit, &z1_esplit, NULL, &x2_esplit, &y2_esplit, &z2_esplit, NULL);
                        
                        assert(csmmath_is_point_in_segment3D(
                                    edge_intersection->x_edge_interior_hedge_at_face, edge_intersection->y_edge_interior_hedge_at_face, edge_intersection->z_edge_interior_hedge_at_face,
                                    x1_esplit, y1_esplit, z1_esplit, x2_esplit, y2_esplit, z2_esplit,
                                    edge_intersection->tolerance_equal_coords,
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

                    i_update_splitted_hedge_on_intersections(
                                edge_intersecctions,
                                i + 1,
                                edge_intersection->hit_hedge_at_face, new_edge_other_solid);            
                    
                    if (csmdebug_debug_enabled() == CSMTRUE)
                    {
                        char *description;
                        
                        csmdebug_print_debug_info("-->Splitted hedge, new vertex: %lu\n", csmvertex_id(new_vertex_on_hit_hedge));
                        csmedge_debug_print_debug_info(new_edge_other_solid, CSMTRUE);
                        csmdebug_print_debug_info("Added VV intersection (%lu, %lu)\n", csmvertex_id(edge_vertex_intersection), csmvertex_id(new_vertex_on_hit_hedge));
                        
                        description = copiafor_codigo4(
                                "IE-F %lu (%.3g, %.3g, %.3g)",
                                csmvertex_id(new_vertex_on_hit_hedge),
                                edge_intersection->x_edge_interior_hedge_at_face, edge_intersection->y_edge_interior_hedge_at_face, edge_intersection->z_edge_interior_hedge_at_face);
                        
                        csmdebug_append_debug_point(
                                edge_intersection->x_edge_interior_hedge_at_face, edge_intersection->y_edge_interior_hedge_at_face, edge_intersection->z_edge_interior_hedge_at_face,
                                &description);
                    }
                    
                    i_append_new_vv_inters(edge_intersection->intersection_id, edge_vertex_intersection, new_vertex_on_hit_hedge, is_A_vs_B, vv_intersections, edge_intersection->tolerance_equal_coords, &did_add_intersection);
                    
                    if (did_add_intersection == CSMFALSE)
                        *did_find_non_manifold_operand_update = CSMTRUE;
                    break;
                }
                    
                case i_TYPE_EDGE_INTERSECTION_INTERIOR_FACE:
                {
                    CSMBOOL added;
                    
                    i_append_new_vf_inters(edge_intersection->intersection_id, edge_vertex_intersection, edge_intersection->face, vertex_face_neighborhood, &added);
                    
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
}

// ------------------------------------------------------------------------------------------

static void i_generate_intersections_edge_with_solid_faces(
                        struct csmedge_t *edge_A,
                        struct csmsolid_t *solid_B,
                        const struct csmtolerance_t *tolerances,
                        CSMBOOL is_A_vs_B,
                        unsigned long *id_new_intersection,
                        csmArrayStruct(csmsetop_vtxvtx_inters_t) *vv_intersections,
                        csmArrayStruct(csmsetop_vtxfacc_inters_t) *vertex_face_neighborhood,
                        CSMBOOL *did_find_non_manifold_operand_update)
{
    struct i_optimized_edge_data_t optimized_edge_data;
    struct csmhedge_t *hedge_pos, *hedge_neg;
    const struct csmbbox_t *solid_b_bbox;
    
    assert_no_null(did_find_non_manifold_operand_update);
    assert(*did_find_non_manifold_operand_update == CSMFALSE);
    
    optimized_edge_data.edge_id = csmedge_id(edge_A);
    
    hedge_pos = csmedge_hedge_lado(edge_A, CSMEDGE_LADO_HEDGE_POS);
    optimized_edge_data.vertex_pos = csmhedge_vertex(hedge_pos);
    csmvertex_get_coordenadas(optimized_edge_data.vertex_pos, &optimized_edge_data.x1, &optimized_edge_data.y1, &optimized_edge_data.z1);
    
    hedge_neg = csmedge_hedge_lado(edge_A, CSMEDGE_LADO_HEDGE_NEG);
    optimized_edge_data.vertex_neg = csmhedge_vertex(hedge_neg);
    csmvertex_get_coordenadas(optimized_edge_data.vertex_neg, &optimized_edge_data.x2, &optimized_edge_data.y2, &optimized_edge_data.z2);
    
    solid_b_bbox = csmsolid_get_bbox(solid_B);
    
    if (csmbbox_intersects_with_segment(
                        solid_b_bbox,
                        optimized_edge_data.x1, optimized_edge_data.y1, optimized_edge_data.z1,
                        optimized_edge_data.x2, optimized_edge_data.y2, optimized_edge_data.z2) == CSMTRUE)
    {
        csmArrayStruct(i_edge_intersection_t) *edge_intersecctions;
        struct csmhashtb_iterator(csmface_t) *face_iterator_B;
        
        optimized_edge_data.length = csmmath_distance_3D(
                        optimized_edge_data.x1, optimized_edge_data.y1, optimized_edge_data.z1,
                        optimized_edge_data.x2, optimized_edge_data.y2, optimized_edge_data.z2);
        
        edge_intersecctions = csmarrayc_new_st_array(0, i_edge_intersection_t);
        face_iterator_B = csmsolid_face_iterator(solid_B);

        while (csmhashtb_has_next(face_iterator_B, csmface_t) == CSMTRUE)
        {
            struct csmface_t *face_B;
        
            csmhashtb_next_pair(face_iterator_B, NULL, &face_B, csmface_t);
            
            i_append_intersections_between_A_edge_and_B_face(
                        &optimized_edge_data,
                        face_B,
                        tolerances,
                        id_new_intersection,
                        edge_intersecctions);
        }
        
        if (csmarrayc_count_st(edge_intersecctions, i_edge_intersection_t) > 0)
        {
            csmarrayc_qsort_st(edge_intersecctions, i_edge_intersection_t, i_compara_edge_intersection);
            
            i_discard_invalid_intersections(edge_intersecctions);
            i_process_edge_intersections(edge_A, edge_intersecctions, is_A_vs_B, vv_intersections, vertex_face_neighborhood, did_find_non_manifold_operand_update);
        }
        
        csmhashtb_free_iterator(&face_iterator_B, csmface_t);
        csmarrayc_free_st(&edge_intersecctions, i_edge_intersection_t, i_free_edge_intersection);
    }
}

// ------------------------------------------------------------------------------------------

static void i_generate_edge_intersections_solid_A_with_solid_B(
                        struct csmsolid_t *solid_A, struct csmsolid_t *solid_B,
                        const struct csmtolerance_t *tolerances,
                        CSMBOOL is_A_vs_B,
                        unsigned long *id_new_intersection,
                        csmArrayStruct(csmsetop_vtxvtx_inters_t) *vv_intersections,
                        csmArrayStruct(csmsetop_vtxfacc_inters_t) *vertex_face_neighborhood,
                        CSMBOOL *did_find_non_manifold_operand_update)
{
    struct csmhashtb_iterator(csmedge_t) *edge_iterator_A;
    
    assert_no_null(did_find_non_manifold_operand_update);
    
    edge_iterator_A = csmsolid_edge_iterator(solid_A);
    
    while (csmhashtb_has_next(edge_iterator_A, csmedge_t) == CSMTRUE && *did_find_non_manifold_operand_update == CSMFALSE)
    {
        struct csmedge_t *edge_A;
        
        csmhashtb_next_pair(edge_iterator_A, NULL, &edge_A, csmedge_t);
        
        i_generate_intersections_edge_with_solid_faces(
                        edge_A,
                        solid_B,
                        tolerances,
                        is_A_vs_B,
                        id_new_intersection,
                        vv_intersections, vertex_face_neighborhood,
                        did_find_non_manifold_operand_update);
    }
    
    csmhashtb_free_iterator(&edge_iterator_A, csmedge_t);
}

// ------------------------------------------------------------------------------------------

void csmsetop_procedges_generate_intersections_on_both_solids(
                        struct csmsolid_t *solid_A, struct csmsolid_t *solid_B,
                        const struct csmtolerance_t *tolerances,
                        csmArrayStruct(csmsetop_vtxvtx_inters_t) **vv_intersections,
                        csmArrayStruct(csmsetop_vtxfacc_inters_t) **vf_intersections_A,
                        csmArrayStruct(csmsetop_vtxfacc_inters_t) **vf_intersections_B,
                        CSMBOOL *did_find_non_manifold_operand)
{
    csmArrayStruct(csmsetop_vtxvtx_inters_t) *vv_intersections_loc;
    csmArrayStruct(csmsetop_vtxfacc_inters_t) *vf_intersections_A_loc, *vf_intersections_B_loc;
    CSMBOOL did_find_non_manifold_operand_solid_A, did_find_non_manifold_operand_solid_B;
    unsigned long id_new_intersection;
    CSMBOOL is_A_vs_B;
    
    assert_no_null(vv_intersections);
    assert_no_null(vf_intersections_A);
    assert_no_null(vf_intersections_B);
    assert_no_null(did_find_non_manifold_operand);

    vv_intersections_loc = csmarrayc_new_st_array(0, csmsetop_vtxvtx_inters_t);
    vf_intersections_A_loc = csmarrayc_new_st_array(0, csmsetop_vtxfacc_inters_t);
    vf_intersections_B_loc = csmarrayc_new_st_array(0, csmsetop_vtxfacc_inters_t);
    id_new_intersection = 0;

    csmdebug_begin_context("csmsetop_procedges_generate_intersections_on_both_solids");
    {
        csmdebug_begin_context("Intersections A vs B");
        {
            csmdebug_clear_debug_points();
            
            is_A_vs_B = CSMTRUE;
            did_find_non_manifold_operand_solid_A = CSMFALSE;
            
            i_generate_edge_intersections_solid_A_with_solid_B(
                        solid_A,
                        solid_B,
                        tolerances,
                        is_A_vs_B,
                        &id_new_intersection,
                        vv_intersections_loc, vf_intersections_A_loc,
                        &did_find_non_manifold_operand_solid_A);
            
            csmsolid_debug_print_debug(solid_A, CSMTRUE);
            csmsolid_debug_print_debug(solid_B, CSMTRUE);
        }
        csmdebug_end_context();
    
        //csmdebug_show_viewer();
        
        if (did_find_non_manifold_operand_solid_A == CSMFALSE)
        {
            csmdebug_begin_context("Intersections B vs A");
            {
                csmdebug_clear_debug_points();
                
                is_A_vs_B = CSMFALSE;
                did_find_non_manifold_operand_solid_B = CSMFALSE;
                
                i_generate_edge_intersections_solid_A_with_solid_B(
                        solid_B,
                        solid_A,
                        tolerances,
                        is_A_vs_B,
                        &id_new_intersection,
                        vv_intersections_loc, vf_intersections_B_loc,
                        &did_find_non_manifold_operand_solid_B);
            
                csmsolid_debug_print_debug(solid_A, CSMTRUE);
                csmsolid_debug_print_debug(solid_B, CSMTRUE);
            }
            csmdebug_end_context();
        }
        else
        {
            did_find_non_manifold_operand_solid_B = CSMFALSE;
        }

        //csmdebug_show_viewer();
        
        if (did_find_non_manifold_operand_solid_A == CSMFALSE && did_find_non_manifold_operand_solid_B == CSMFALSE)
        {
            i_append_common_vertices_solid_A_and_B_not_previously_found(
                        solid_A, solid_B,
                        tolerances,
                        &id_new_intersection,
                        vv_intersections_loc,
                        &did_find_non_manifold_operand_solid_A,
                        &did_find_non_manifold_operand_solid_B);
        }
    }
    csmdebug_end_context();
    
    *vv_intersections = vv_intersections_loc;
    *vf_intersections_A = vf_intersections_A_loc;
    *vf_intersections_B = vf_intersections_B_loc;
    *did_find_non_manifold_operand = IS_TRUE(did_find_non_manifold_operand_solid_A == CSMTRUE || did_find_non_manifold_operand_solid_B == CSMTRUE);
}
