//
//  csmfacbrep2solid.c
//  rGWB
//
//  Created by Manuel Fernandez on 21/4/18.
//  Copyright © 2018 Manuel Fernández. All rights reserved.
//

#include "csmfacbrep2solid.h"

#include "csmarrayc.h"
#include "csmedge.inl"
#include "csmedge.tli"
#include "csmface.inl"
#include "csmhashtb.inl"
#include "csmhedge.inl"
#include "csmloop.inl"
#include "csmmath.inl"
#include "csmnode.inl"
#include "csmsolid.h"
#include "csmsolid.inl"
#include "csmsimplifysolid.inl"
#include "csmtolerance.inl"
#include "csmvertex.inl"

#ifdef RGWB_STANDALONE_DISTRIBUTABLE

#include "csmapto3d.h"
#include "csmassert.inl"
#include "csmmem.inl"

struct i_null_value_t;
csmArrayStruct(i_edge_t);

#else

#include "cyassert.h"
#include "cypespy.h"

#endif

struct i_loop_point_t
{
    double x, y, z;
    
    unsigned long vertex_idx;
    unsigned long edge_idx;
    
    struct csmhedge_t *shedge;
};

struct csmfacbrep2solid_loop_t
{
    csmArrayStruct(i_loop_point_t) *points;
    struct csmloop_t *sloop;
};

struct csmfacbrep2solid_face_t
{
    struct csmfacbrep2solid_loop_t *outer_loop;
    csmArrayStruct(csmfacbrep2solid_loop_t) *inner_loops;
    
    struct csmface_t *sface;
};

struct i_vertex_t
{
    double x, y, z;
    
    unsigned long no_uses;
    struct csmvertex_t *svertex;
};

struct csmfacbrep2solid_t
{
    double equal_points_tolerance;
    CSMBOOL face_normal_point_out_of_solid;
    
    unsigned long id_new_element;
    
    csmArrayStruct(csmfacbrep2solid_face_t) *faces;
    csmArrayStruct(i_vertex_t) *vertexs;
};

struct i_hedge_id_t
{
    unsigned long idx_face, idx_loop, idx_point;
};

struct i_edge_t
{
    unsigned long idx_vertex1, idx_vertex2;
    
    struct i_hedge_id_t *he1;
    struct i_hedge_id_t *he2;
};

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_loop_point_t *, i_new_loop_point, (
                        double x, double y, double z,
                        unsigned long vertex_idx,
                        unsigned long edge_idx,
                        struct csmhedge_t *shedge))
{
    struct i_loop_point_t *loop_point;
    
    loop_point = MALLOC(struct i_loop_point_t);
    
    loop_point->x = x;
    loop_point->y = y;
    loop_point->z = z;
    
    loop_point->vertex_idx = vertex_idx;
    loop_point->edge_idx = edge_idx;
    
    loop_point->shedge = shedge;
    
    return loop_point;
}

// ------------------------------------------------------------------------------------------

static void i_free_loop_point(struct i_loop_point_t **loop_point)
{
    assert_no_null(loop_point);
    assert_no_null(*loop_point);
    
    FREE_PP(loop_point, struct i_loop_point_t);
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmfacbrep2solid_loop_t *, i_new_loop, (csmArrayStruct(i_loop_point_t) **points, struct csmloop_t *sloop))
{
    struct csmfacbrep2solid_loop_t *loop;
    
    loop = MALLOC(struct csmfacbrep2solid_loop_t);
    
    loop->points = ASSIGN_POINTER_PP(points, csmArrayStruct(i_loop_point_t));
    loop->sloop = sloop;
    
    return loop;
}

// ------------------------------------------------------------------------------------------

static void i_free_loop(struct csmfacbrep2solid_loop_t **loop)
{
    assert_no_null(loop);
    assert_no_null(*loop);
    
    csmarrayc_free_st(&(*loop)->points, i_loop_point_t, i_free_loop_point);
    
    FREE_PP(loop, struct csmfacbrep2solid_loop_t);
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmfacbrep2solid_face_t *, i_new_face, (
                        struct csmfacbrep2solid_loop_t **outer_loop,
                        csmArrayStruct(csmfacbrep2solid_loop_t) **inner_loops,
                        struct csmface_t *sface))
{
    struct csmfacbrep2solid_face_t *face;
    
    face = MALLOC(struct csmfacbrep2solid_face_t);
    
    face->outer_loop = ASSIGN_POINTER_PP(outer_loop, struct csmfacbrep2solid_loop_t);
    face->inner_loops = ASSIGN_POINTER_PP(inner_loops, csmArrayStruct(csmfacbrep2solid_loop_t));
    
    face->sface = sface;
    
    return face;
}

// ------------------------------------------------------------------------------------------

static void i_free_face(struct csmfacbrep2solid_face_t **face)
{
    assert_no_null(face);
    assert_no_null(*face);
    
    if ((*face)->outer_loop != NULL)
        i_free_loop(&(*face)->outer_loop);
                    
    csmarrayc_free_st(&(*face)->inner_loops, csmfacbrep2solid_loop_t, i_free_loop);
    
    FREE_PP(face, struct csmfacbrep2solid_face_t);
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_vertex_t *, i_new_vertex, (
                        double x, double y, double z,
                        unsigned long no_uses,
                        struct csmvertex_t *svertex))
{
    struct i_vertex_t *vertex;
    
    vertex = MALLOC(struct i_vertex_t);
    
    vertex->x = x;
    vertex->y = y;
    vertex->z = z;
    
    vertex->no_uses = no_uses;
    vertex->svertex = svertex;
    
    return vertex;
}

// ------------------------------------------------------------------------------------------

static void i_free_vertex(struct i_vertex_t **vertex)
{
    assert_no_null(vertex);
    assert_no_null(*vertex);
    
    FREE_PP(vertex, struct i_vertex_t);
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmfacbrep2solid_t *, i_new_facbrep2solid, (
                        double equal_points_tolerance,
                        CSMBOOL face_normal_point_out_of_solid,
                        unsigned long id_new_element,
                        csmArrayStruct(csmfacbrep2solid_face_t) **faces,
                        csmArrayStruct(i_vertex_t) **vertexs))
{
    struct csmfacbrep2solid_t *builder;
    
    builder = MALLOC(struct csmfacbrep2solid_t);
    
    builder->equal_points_tolerance = equal_points_tolerance;
    builder->face_normal_point_out_of_solid = face_normal_point_out_of_solid;

    builder->id_new_element = id_new_element;
    
    builder->faces = ASSIGN_POINTER_PP(faces, csmArrayStruct(csmfacbrep2solid_face_t));
    builder->vertexs = ASSIGN_POINTER_PP(vertexs, csmArrayStruct(i_vertex_t));
    
    return builder;
}

// ------------------------------------------------------------------------------------------

struct csmfacbrep2solid_t *csmfacbrep2solid_new(double equal_points_tolerance, CSMBOOL face_normal_point_out_of_solid)
{
    unsigned long id_new_element;
    csmArrayStruct(csmfacbrep2solid_face_t) *faces;
    csmArrayStruct(i_vertex_t) *vertexs;
    
    assert(equal_points_tolerance > 0. && equal_points_tolerance < 1.e-3);
    
    id_new_element = 0;
    
    faces = csmarrayc_new_st_array(0, csmfacbrep2solid_face_t);
    vertexs = csmarrayc_new_st_array(0, i_vertex_t);
    
    return i_new_facbrep2solid(equal_points_tolerance, face_normal_point_out_of_solid, id_new_element, &faces, &vertexs);
}

// ------------------------------------------------------------------------------------------

void csmfacbrep2solid_free(struct csmfacbrep2solid_t **builder)
{
    assert_no_null(builder);
    assert_no_null(*builder);
    
    csmarrayc_free_st(&(*builder)->faces, csmfacbrep2solid_face_t, i_free_face);
    csmarrayc_free_st(&(*builder)->vertexs, i_vertex_t, i_free_vertex);
    
    FREE_PP(builder, struct csmfacbrep2solid_t);
}

// ------------------------------------------------------------------------------------------

struct csmfacbrep2solid_face_t *csmfacbrep2solid_new_face(void)
{
    struct csmfacbrep2solid_loop_t *outer_loop;
    csmArrayStruct(csmfacbrep2solid_loop_t) *inner_loops;
    struct csmface_t *sface;
    
    outer_loop = NULL;
    inner_loops = csmarrayc_new_st_array(0, csmfacbrep2solid_loop_t);
    sface = NULL;
    
    return i_new_face(&outer_loop, &inner_loops, sface);
}

// ------------------------------------------------------------------------------------------

void csmfacbrep2solid_append_outer_loop_to_face(struct csmfacbrep2solid_face_t *face, struct csmfacbrep2solid_loop_t **outer_loop)
{
    assert_no_null(face);
    assert(face->outer_loop == NULL);
    
    face->outer_loop = ASSIGN_POINTER_PP_NOT_NULL(outer_loop, struct csmfacbrep2solid_loop_t);
}

// ------------------------------------------------------------------------------------------

void csmfacbrep2solid_append_inner_loop_to_face(struct csmfacbrep2solid_face_t *face, struct csmfacbrep2solid_loop_t **inner_loop)
{
    struct csmfacbrep2solid_loop_t *inner_loop_loc;
    
    assert_no_null(face);
    
    inner_loop_loc = ASSIGN_POINTER_PP_NOT_NULL(inner_loop, struct csmfacbrep2solid_loop_t);
    csmarrayc_append_element_st(face->inner_loops, inner_loop_loc, csmfacbrep2solid_loop_t);
}

// ------------------------------------------------------------------------------------------

struct csmfacbrep2solid_loop_t *csmfacbrep2solid_new_loop(void)
{
    csmArrayStruct(i_loop_point_t) *points;
    struct csmloop_t *sloop;
    
    points = csmarrayc_new_st_array(0, i_loop_point_t);
    sloop = NULL;
    
    return i_new_loop(&points, sloop);
}

// ------------------------------------------------------------------------------------------

void csmfacbrep2solid_append_point_to_loop(struct csmfacbrep2solid_loop_t *loop, double x, double y, double z)
{
    struct i_loop_point_t *point;
    struct csmhedge_t *shedge;
    
    assert_no_null(loop);
    
    shedge = NULL;
    point = i_new_loop_point(x, y, z, ULONG_MAX, ULONG_MAX, shedge);
    
    csmarrayc_append_element_st(loop->points, point, i_loop_point_t);
}

// ------------------------------------------------------------------------------------------

void csmfacbrep2solid_append_indexed_point_to_loop(struct csmfacbrep2solid_loop_t *loop, unsigned long point_idx)
{
    struct i_loop_point_t *point;
    struct csmhedge_t *shedge;
    
    shedge = NULL;
    point = i_new_loop_point(0., 0., 0., point_idx, ULONG_MAX, shedge);
    
    csmarrayc_append_element_st(loop->points, point, i_loop_point_t);
}

// ------------------------------------------------------------------------------------------

void csmfacbrep2solid_reverse(struct csmfacbrep2solid_loop_t *loop)
{
    assert_no_null(loop);
    assert(loop->sloop == NULL);
    
    csmarrayc_invert(loop->points, i_loop_point_t);
}

// ------------------------------------------------------------------------------------------

static unsigned long i_get_vertex_idx_for_point(
                        double x, double y, double z,
                        double tolerance,
                        csmArrayStruct(i_vertex_t) *vertexs)
{
    unsigned long vertex_idx;
    unsigned long i, no_points;
    
    no_points = csmarrayc_count_st(vertexs, i_vertex_t);
    vertex_idx = ULONG_MAX;
    
    for (i = 0; i < no_points; i++)
    {
        struct i_vertex_t *vertex;
        
        vertex = csmarrayc_get_st(vertexs, i, i_vertex_t);
        assert_no_null(vertex);
        
        if (csmmath_equal_coords(x, y, z, vertex->x, vertex->y, vertex->z, tolerance) == CSMTRUE)
        {
            vertex_idx = i;
            vertex->no_uses++;
            break;
        }
    }
    
    if (vertex_idx == ULONG_MAX)
    {
        struct i_vertex_t *vertex;
        unsigned long no_uses;
        struct csmvertex_t *svertex;
        
        no_uses = 1;
        svertex = NULL;
        
        vertex = i_new_vertex(x, y, z, no_uses, svertex);
        csmarrayc_append_element_st(vertexs, vertex, i_vertex_t);
        
        vertex_idx = csmarrayc_count_st(vertexs, i_vertex_t) - 1;
    }
    
    return vertex_idx;
}

// ------------------------------------------------------------------------------------------

static void i_register_loop_vertexs(
                        struct csmfacbrep2solid_loop_t *loop,
                        double tolerance,
                        csmArrayStruct(i_vertex_t) *vertexs)
{
    unsigned long i, no_points;
    
    assert_no_null(loop);
    no_points = csmarrayc_count_st(loop->points, i_loop_point_t);
    assert(no_points >= 3);
    
    for (i = 0; i < no_points; i++)
    {
        struct i_loop_point_t *loop_point;
        
        loop_point = csmarrayc_get_st(loop->points, i, i_loop_point_t);
        assert_no_null(loop_point);
        
        if (loop_point->vertex_idx == ULONG_MAX)
        {
            loop_point->vertex_idx = i_get_vertex_idx_for_point(loop_point->x, loop_point->y, loop_point->z, tolerance, vertexs);
        }
        else
        {
            struct i_vertex_t *vertex;
            
            vertex = csmarrayc_get_st(vertexs, loop_point->vertex_idx, i_vertex_t);
            assert_no_null(vertex);
            
            loop_point->x = vertex->x;
            loop_point->y = vertex->y;
            loop_point->z = vertex->z;
            
            vertex->no_uses++;
        }
    }
}

// ------------------------------------------------------------------------------------------

static void i_register_inner_loops_vertexs(
                        csmArrayStruct(csmfacbrep2solid_loop_t) *inner_loops,
                        double tolerance,
                        csmArrayStruct(i_vertex_t) *vertexs)
{
    unsigned long i, no_inner_loops;
    
    no_inner_loops = csmarrayc_count_st(inner_loops, csmfacbrep2solid_loop_t);
    
    for (i = 0; i < no_inner_loops; i++)
    {
        struct csmfacbrep2solid_loop_t *inner_loop;
        
        inner_loop = csmarrayc_get_st(inner_loops, i, csmfacbrep2solid_loop_t);
        i_register_loop_vertexs(inner_loop, tolerance, vertexs);
    }
}

// ------------------------------------------------------------------------------------------

unsigned long csmfacbrep2solid_append_point(struct csmfacbrep2solid_t *builder, double x, double y, double z)
{
    struct i_vertex_t *vertex;
    unsigned long no_uses;
    struct csmvertex_t *svertex;
    
    assert_no_null(builder);

    no_uses = 0;
    svertex = NULL;

    vertex = i_new_vertex(x, y, z, no_uses, svertex);
    csmarrayc_append_element_st(builder->vertexs, vertex, i_vertex_t);

    return csmarrayc_count_st(builder->vertexs, i_vertex_t) - 1;
}

// ------------------------------------------------------------------------------------------

void csmfacbrep2solid_append_face(struct csmfacbrep2solid_t *builder, struct csmfacbrep2solid_face_t **face)
{
    struct csmfacbrep2solid_face_t *face_loc;
    
    assert_no_null(builder);
    
    face_loc = ASSIGN_POINTER_PP_NOT_NULL(face, struct csmfacbrep2solid_face_t);
    assert_no_null(face_loc);
    
    i_register_loop_vertexs(face_loc->outer_loop, builder->equal_points_tolerance, builder->vertexs);
    i_register_inner_loops_vertexs(face_loc->inner_loops, builder->equal_points_tolerance, builder->vertexs);
    
    csmarrayc_append_element_st(builder->faces, face_loc, csmfacbrep2solid_face_t);
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_hedge_id_t *, i_new_hedge_id, (unsigned long idx_face, unsigned long idx_loop, unsigned long idx_point))
{
    struct i_hedge_id_t *hedge_id;
    
    hedge_id = MALLOC(struct i_hedge_id_t);
    
    hedge_id->idx_face = idx_face;
    hedge_id->idx_loop = idx_loop;
    hedge_id->idx_point = idx_point;
    
    return hedge_id;
}

// ------------------------------------------------------------------------------------------

static void i_free_hedge_id(struct i_hedge_id_t **hedge_id)
{
    assert_no_null(hedge_id);
    assert_no_null(*hedge_id);
    
    FREE_PP(hedge_id, struct i_hedge_id_t);
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_edge_t *, i_new_edge, (
                        unsigned long idx_vertex1, unsigned long idx_vertex2,
                        struct i_hedge_id_t **he1, struct i_hedge_id_t **he2))
{
    struct i_edge_t *edge;
    
    edge = MALLOC(struct i_edge_t);
    
    edge->idx_vertex1 = idx_vertex1;
    edge->idx_vertex2 = idx_vertex2;
    
    edge->he1 = ASSIGN_POINTER_PP(he1, struct i_hedge_id_t);
    edge->he2 = ASSIGN_POINTER_PP(he2, struct i_hedge_id_t);
    
    return edge;
}

// ------------------------------------------------------------------------------------------

static void i_free_edge(struct i_edge_t **edge)
{
    assert_no_null(edge);
    assert_no_null(*edge);
    
    if ((*edge)->he1 != NULL)
        i_free_hedge_id(&(*edge)->he1);

    if ((*edge)->he2 != NULL)
        i_free_hedge_id(&(*edge)->he2);
    
    FREE_PP(edge, struct i_edge_t);
}

// ------------------------------------------------------------------------------------------

static CSMBOOL i_is_edge_with_vertexs(
                        const struct i_edge_t *edge,
                        unsigned long idx_vertex1, unsigned long idx_vertex2)
{
    assert_no_null(edge);
    assert(idx_vertex1 != ULONG_MAX);
    assert(idx_vertex2 != ULONG_MAX);
    
    if (edge->idx_vertex1 == idx_vertex1 && edge->idx_vertex2 == idx_vertex2)
    {
        return CSMTRUE;
    }
    else if (edge->idx_vertex1 == idx_vertex2 && edge->idx_vertex2 == idx_vertex1)
    {
        return CSMTRUE;
    }
    else
    {
        return CSMFALSE;
    }
}

// ------------------------------------------------------------------------------------------

static void i_append_loop_point_to_edge_lists(
                        struct i_loop_point_t *loop_point, const struct i_loop_point_t *next_loop_point,
                        unsigned long idx_face, unsigned long idx_loop, unsigned long idx_point,
                        CSMBOOL *did_find_edge_used_more_than_once,
                        csmArrayStruct(i_edge_t) *edges)
{
    unsigned long i, no_edges;
    unsigned long edge_idx;
    
    assert_no_null(loop_point);
    assert(loop_point->vertex_idx != ULONG_MAX);
    assert(loop_point->edge_idx == ULONG_MAX);
    assert_no_null(next_loop_point);
    assert(next_loop_point->vertex_idx != ULONG_MAX);
    assert_no_null(did_find_edge_used_more_than_once);
    assert(*did_find_edge_used_more_than_once == CSMFALSE);
    
    no_edges = csmarrayc_count_st(edges, i_edge_t);
    edge_idx = ULONG_MAX;
    
    for (i = 0; i < no_edges; i++)
    {
        struct i_edge_t *edge;
        
        edge = csmarrayc_get_st(edges, i, i_edge_t);
        assert_no_null(edge);
        
        if (i_is_edge_with_vertexs(edge, loop_point->vertex_idx, next_loop_point->vertex_idx) == CSMTRUE)
        {
            assert_no_null(edge->he1);
            
            edge_idx = i;
            
            if (edge->he2 == NULL)
                edge->he2 = i_new_hedge_id(idx_face, idx_loop, idx_point);
            else
                *did_find_edge_used_more_than_once = CSMTRUE;
            
            break;
        }
    }
    
    if (edge_idx == ULONG_MAX && *did_find_edge_used_more_than_once == CSMFALSE)
    {
        struct i_hedge_id_t *he1, *he2;
        struct i_edge_t *edge;
        
        he1 = i_new_hedge_id(idx_face, idx_loop, idx_point);
        he2 = NULL;
        
        edge = i_new_edge(loop_point->vertex_idx, next_loop_point->vertex_idx, &he1, &he2);
        csmarrayc_append_element_st(edges, edge, i_edge_t);
        
        edge_idx = csmarrayc_count_st(edges, i_edge_t) - 1;
    }
    
    loop_point->edge_idx = edge_idx;
}

// ------------------------------------------------------------------------------------------

static void i_append_loop_points_to_edge_list(
                        struct csmfacbrep2solid_loop_t *loop,
                        unsigned long idx_face, unsigned long idx_loop,
                        CSMBOOL *did_find_edge_used_more_than_once,
                        csmArrayStruct(i_edge_t) *edges)
{
    unsigned long i, no_points;
    
    assert_no_null(loop);
    assert(loop->sloop == NULL);
    no_points = csmarrayc_count_st(loop->points, i_loop_point_t);
    assert(no_points >= 3);
    assert_no_null(did_find_edge_used_more_than_once);
    assert(*did_find_edge_used_more_than_once == CSMFALSE);

    for (i = 0; i < no_points; i++)
    {
        struct i_loop_point_t *loop_point, *next_loop_point;
        
        loop_point = csmarrayc_get_st(loop->points, i, i_loop_point_t);
        next_loop_point = csmarrayc_get_st(loop->points, (i + 1) % no_points, i_loop_point_t);
        
        i_append_loop_point_to_edge_lists(
                        loop_point, next_loop_point,
                        idx_face, idx_loop, i,
                        did_find_edge_used_more_than_once,
                        edges);
        
        if (*did_find_edge_used_more_than_once == CSMTRUE)
            break;
    }
}

// ------------------------------------------------------------------------------------------

static void i_append_face_loop_points_to_edge_list(
                        struct csmfacbrep2solid_face_t *face,
                        unsigned long idx_face,
                        CSMBOOL *did_find_edge_used_more_than_once,
                        csmArrayStruct(i_edge_t) *edges)
{
    assert_no_null(face);
    assert_no_null(face->outer_loop);
    assert_no_null(did_find_edge_used_more_than_once);
    assert(*did_find_edge_used_more_than_once == CSMFALSE);
    
    i_append_loop_points_to_edge_list(
                        face->outer_loop,
                        idx_face, ULONG_MAX,
                        did_find_edge_used_more_than_once,
                        edges);
    
    if (*did_find_edge_used_more_than_once == CSMFALSE)
    {
        unsigned long i, no_inner_loops;
        
        no_inner_loops = csmarrayc_count_st(face->inner_loops, csmfacbrep2solid_loop_t);
        
        for (i = 0; i < no_inner_loops; i++)
        {
            struct csmfacbrep2solid_loop_t *inner_loop;
            
            inner_loop = csmarrayc_get_st(face->inner_loops, i, csmfacbrep2solid_loop_t);
            
            i_append_loop_points_to_edge_list(
                        inner_loop,
                        idx_face, i,
                        did_find_edge_used_more_than_once,
                        edges);
            
            if (*did_find_edge_used_more_than_once == CSMTRUE)
                break;
        }
    }
}

// ------------------------------------------------------------------------------------------

static CSMBOOL i_did_generate_edge_list(
                        csmArrayStruct(csmfacbrep2solid_face_t) *faces,
                        csmArrayStruct(i_edge_t) **edges)
{
    CSMBOOL did_generate;
    csmArrayStruct(i_edge_t) *edges_loc;
    unsigned long i, no_faces;
    CSMBOOL did_find_edge_used_more_than_once;
    
    no_faces = csmarrayc_count_st(faces, csmfacbrep2solid_face_t);
    assert(no_faces > 0);
    assert_no_null(edges);

    did_generate = CSMTRUE;
    edges_loc = csmarrayc_new_st_array(0, i_edge_t);
    
    did_find_edge_used_more_than_once = CSMFALSE;
    
    for (i = 0; i < no_faces; i++)
    {
        struct csmfacbrep2solid_face_t *face;
        
        face = csmarrayc_get_st(faces, i, csmfacbrep2solid_face_t);
        i_append_face_loop_points_to_edge_list(face, i, &did_find_edge_used_more_than_once, edges_loc);
        
        if (did_find_edge_used_more_than_once == CSMTRUE)
        {
            did_generate = CSMFALSE;
            csmarrayc_free_st(&edges_loc, i_edge_t, i_free_edge);
            break;
        }
    }
    
    *edges = edges_loc;
    
    return did_generate;
}

// ------------------------------------------------------------------------------------------

static void i_generate_solid_vertexs(csmArrayStruct(i_vertex_t) *vertexs, struct csmsolid_t *solid)
{
    unsigned long i, no_vertexs;
    
    no_vertexs = csmarrayc_count_st(vertexs, i_vertex_t);
    assert(no_vertexs);
    
    for (i = 0; i < no_vertexs; i++)
    {
        struct i_vertex_t *vertex;
        
        vertex = csmarrayc_get_st(vertexs, i, i_vertex_t);
        assert_no_null(vertex);
        
        csmsolid_append_new_vertex(solid, vertex->x, vertex->y, vertex->z, &vertex->svertex);
    }
}

// ------------------------------------------------------------------------------------------

static void i_generate_loop_point_solid_hedge(
                        struct i_loop_point_t *loop_point,
                        struct csmloop_t *sloop,
                        unsigned long *id_new_element,
                        const csmArrayStruct(i_vertex_t) *vertexs)
{
    const struct i_vertex_t *vertex;
    
    assert_no_null(loop_point);
    assert(loop_point->vertex_idx != ULONG_MAX);
    assert(loop_point->shedge == NULL);

    vertex = csmarrayc_get_st(vertexs, loop_point->vertex_idx, i_vertex_t);
    assert_no_null(vertex);
    
    loop_point->shedge = csmhedge_new(id_new_element);
    csmhedge_set_vertex(loop_point->shedge, vertex->svertex);
    csmhedge_set_loop(loop_point->shedge, sloop);
    
    if (csmvertex_hedge(vertex->svertex) == NULL)
        csmvertex_set_hedge(vertex->svertex, loop_point->shedge);
    
    if (csmloop_ledge(sloop) == NULL)
        csmloop_set_ledge(sloop, loop_point->shedge);
}

// ------------------------------------------------------------------------------------------

static void i_generate_loop_solid_loop(
                        struct csmfacbrep2solid_loop_t *loop, CSMBOOL is_outer_loop,
                        struct csmface_t *sface,
                        unsigned long *id_new_element,
                        const csmArrayStruct(i_vertex_t) *vertexs)
{
    unsigned long i, no_points;
    
    assert_no_null(loop);
    assert(loop->sloop == NULL);
    no_points = csmarrayc_count_st(loop->points, i_loop_point_t);
    assert(no_points >= 3);

    loop->sloop = csmloop_new(sface, id_new_element);
    
    if (is_outer_loop == CSMTRUE)
    {
        assert(csmface_floops(sface) == NULL);
        assert(csmface_flout(sface) == NULL);

        csmface_set_flout(sface, loop->sloop);
    }
    else
    {
        struct csmloop_t *floops;
        
        floops = csmface_floops(sface);
        assert(floops != NULL);
        
        csmnode_insert_node2_before_node1(floops, loop->sloop, csmloop_t);
        csmface_set_floops(sface, loop->sloop);
    }
    
    for (i = 0; i < no_points; i++)
    {
        struct i_loop_point_t *loop_point;
        
        loop_point = csmarrayc_get_st(loop->points, i, i_loop_point_t);
        assert_no_null(loop_point);
        
        i_generate_loop_point_solid_hedge(loop_point, loop->sloop, id_new_element, vertexs);
    }
    
    for (i = 0; i < no_points; i++)
    {
        struct i_loop_point_t *loop_point, *loop_point_next;
        
        loop_point = csmarrayc_get_st(loop->points, i, i_loop_point_t);
        assert_no_null(loop_point);
        assert_no_null(loop_point->shedge);
        
        loop_point_next = csmarrayc_get_st(loop->points, (i + 1) % no_points , i_loop_point_t);
        assert_no_null(loop_point_next);
        assert_no_null(loop_point_next->shedge);
        
        csmhedge_set_next(loop_point->shedge, loop_point_next->shedge);
        csmhedge_set_prev(loop_point_next->shedge, loop_point->shedge);
    }
}

// ------------------------------------------------------------------------------------------

static void i_generate_solid_face(
                        struct csmfacbrep2solid_face_t *face,
                        struct csmsolid_t *solid,
                        unsigned long *id_new_element,
                        const csmArrayStruct(i_vertex_t) *vertexs)
{
    unsigned long i, no_inner_loops;
    
    assert_no_null(face);
    assert(face->sface == NULL);
    assert(face->outer_loop != NULL);
    
    csmsolid_append_new_face(solid, &face->sface);
    i_generate_loop_solid_loop(face->outer_loop, CSMTRUE, face->sface, id_new_element, vertexs);
    
    no_inner_loops = csmarrayc_count_st(face->inner_loops, csmfacbrep2solid_loop_t);
    
    for (i = 0; i < no_inner_loops; i++)
    {
        struct csmfacbrep2solid_loop_t *inner_loop;
        
        inner_loop = csmarrayc_get_st(face->inner_loops, i, csmfacbrep2solid_loop_t);
        i_generate_loop_solid_loop(inner_loop, CSMFALSE, face->sface, id_new_element, vertexs);
    }
}

// ------------------------------------------------------------------------------------------

static struct csmhedge_t *i_get_he(
                        const struct i_hedge_id_t *hedge_id,
                        const csmArrayStruct(csmfacbrep2solid_face_t) *faces)
{
    const struct csmfacbrep2solid_face_t *face;
    const struct csmfacbrep2solid_loop_t *loop;
    const struct i_loop_point_t *loop_point;
    
    assert_no_null(hedge_id);
    
    face = csmarrayc_get_const_st(faces, hedge_id->idx_face, csmfacbrep2solid_face_t);
    assert_no_null(face);
    
    if (hedge_id->idx_loop == ULONG_MAX)
        loop = face->outer_loop;
    else
        loop = csmarrayc_get_const_st(face->inner_loops, hedge_id->idx_loop, csmfacbrep2solid_loop_t);
    
    assert_no_null(loop);
    
    loop_point = csmarrayc_get_const_st(loop->points, hedge_id->idx_point, i_loop_point_t);
    assert_no_null(loop_point);
    assert(loop_point->shedge != NULL);
    
    return loop_point->shedge;
}

// ------------------------------------------------------------------------------------------

static void i_generate_solid_hedges(
                        const csmArrayStruct(i_edge_t) *edges,
                        const csmArrayStruct(csmfacbrep2solid_face_t) *faces,
                        struct csmsolid_t *solid)
{
    unsigned long i, no_edges;
    
    no_edges = csmarrayc_count_st(edges, i_edge_t);
    assert(no_edges > 0);
    
    for (i = 0; i < no_edges; i++)
    {
        struct i_edge_t *edge;
        struct csmhedge_t *she1, *she2;
        struct csmedge_t *sedge;
        
        edge = csmarrayc_get_st(edges, i, i_edge_t);
        assert_no_null(edge);
        
        she1 = i_get_he(edge->he1, faces);
        assert(csmhedge_edge(she1) == NULL);
        
        she2 = i_get_he(edge->he2, faces);
        assert(csmhedge_edge(she2) == NULL);
        
        csmsolid_append_new_edge(solid, &sedge);
        
        csmedge_set_edge_lado(sedge, CSMEDGE_HEDGE_SIDE_POS, she1);
        csmhedge_set_edge(she1, sedge);
        
        csmedge_set_edge_lado(sedge, CSMEDGE_HEDGE_SIDE_NEG, she2);
        csmhedge_set_edge(she2, sedge);
    }
}

// ------------------------------------------------------------------------------------------

static CSMBOOL i_is_incomplete_edge(const struct i_edge_t *edge, const struct i_null_value_t *null_value)
{
    assert_no_null(edge);
    assert(null_value == NULL);
    
    return IS_TRUE(edge->he1 == NULL || edge->he2 == NULL);
}

// ------------------------------------------------------------------------------------------

static CSMBOOL i_is_non_manifold_vertex(const struct i_vertex_t *vertex, const struct i_null_value_t *null_value)
{
    assert_no_null(vertex);
    assert(null_value == NULL);
    
    return IS_TRUE(vertex->no_uses < 2);
}

// ------------------------------------------------------------------------------------------

static void i_reverse_face_loop_because_csm_outer_loop_points_to_interior(struct csmfacbrep2solid_loop_t *loop)
{
    assert_no_null(loop);
    csmarrayc_invert(loop->points, i_loop_point_t);
}

// ------------------------------------------------------------------------------------------

static void i_reverse_face_loops_because_csm_outer_loop_points_to_interior(struct csmfacbrep2solid_face_t *face)
{
    unsigned long i, no_inner_loops;
    
    assert_no_null(face);
    assert(face->sface == NULL);
    assert(face->outer_loop != NULL);
    
    i_reverse_face_loop_because_csm_outer_loop_points_to_interior(face->outer_loop);
    
    no_inner_loops = csmarrayc_count_st(face->inner_loops, csmfacbrep2solid_loop_t);
    
    for (i = 0; i < no_inner_loops; i++)
    {
        struct csmfacbrep2solid_loop_t *inner_loop;
        
        inner_loop = csmarrayc_get_st(face->inner_loops, i, csmfacbrep2solid_loop_t);
        i_reverse_face_loop_because_csm_outer_loop_points_to_interior(inner_loop);
    }
}

// ------------------------------------------------------------------------------------------

static void i_reverse_faces_loops_because_csm_outer_loop_points_to_interior(csmArrayStruct(csmfacbrep2solid_face_t) *faces)
{
    unsigned long i, no_faces;
    
    no_faces = csmarrayc_count_st(faces, csmfacbrep2solid_face_t);
    
    for (i = 0; i < no_faces; i++)
    {
        struct csmfacbrep2solid_face_t *face;
        
        face = csmarrayc_get_st(faces, i, csmfacbrep2solid_face_t);
        i_reverse_face_loops_because_csm_outer_loop_points_to_interior(face);
    }
}

// ------------------------------------------------------------------------------------------

static CSMBOOL i_check_inner_loop_orientation(struct csmsolid_t *solid)
{
    CSMBOOL inner_loop_orientation_correct;
    struct csmhashtb_iterator(csmface_t) *face_iterator_i;
    
    inner_loop_orientation_correct = CSMTRUE;
    face_iterator_i = csmsolid_face_iterator(solid);
        
    while (csmhashtb_has_next(face_iterator_i, csmface_t) == CSMTRUE)
    {
        struct csmface_t *face;
    
        csmhashtb_next_pair(face_iterator_i, NULL, &face, csmface_t);
        
        if (csmface_has_holes(face) == CSMTRUE)
        {
            struct csmloop_t *face_flout, *face_floops, *loop_iterator;
    
            face_flout = csmface_flout(face);
            face_floops = csmface_floops(face);
            loop_iterator = face_floops;
        
            do
            {
                struct csmloop_t *next_loop;
                
                next_loop = csmloop_next(loop_iterator);
                
                if (loop_iterator != face_flout)
                {
                    double loop_area;
                    
                    loop_area = csmface_loop_area_in_face(face, loop_iterator);
                    
                    if (loop_area > 0.)
                    {
                        inner_loop_orientation_correct = CSMFALSE;
                        break;
                    }
                }
                
                loop_iterator = next_loop;
                
            } while (loop_iterator != NULL);
            
            if (inner_loop_orientation_correct == CSMFALSE)
                break;
        }
    }
        
    csmhashtb_free_iterator(&face_iterator_i, csmface_t);
    
    return inner_loop_orientation_correct;
}

// ------------------------------------------------------------------------------------------

enum csmfacbrep2solid_result_t csmfacbrep2solid_build(struct csmfacbrep2solid_t *builder, struct csmsolid_t **solid)
{
    enum csmfacbrep2solid_result_t result;
    struct csmsolid_t *solid_loc;
    unsigned long no_faces;
    
    assert_no_null(builder);
    assert_no_null(solid);
    
    no_faces = csmarrayc_count_st(builder->faces, csmfacbrep2solid_face_t);
    
    if (no_faces == 0)
    {
        result = CSMFACBREP2SOLID_RESULT_EMPTY_SOLID;
        solid_loc = NULL;
    }
    else
    {
        csmArrayStruct(i_edge_t) *edges;
        
        if (builder->face_normal_point_out_of_solid == CSMTRUE)
            i_reverse_faces_loops_because_csm_outer_loop_points_to_interior(builder->faces);
        
        if (i_did_generate_edge_list(builder->faces, &edges) == CSMFALSE)
        {
            result = CSMFACBREP2SOLID_RESULT_NON_MANIFOLD_FACETED_BREP;
            solid_loc = NULL;
        }
        else
        {
            if (csmarrayc_contains_element_st(edges, i_edge_t, NULL, struct i_null_value_t, i_is_incomplete_edge, NULL) == CSMTRUE
                    || csmarrayc_contains_element_st(builder->vertexs, i_vertex_t, NULL, struct i_null_value_t, i_is_non_manifold_vertex, NULL) == CSMTRUE)
            {
                result = CSMFACBREP2SOLID_RESULT_MALFORMED_FACETED_BREP;
                solid_loc = NULL;
            }
            else
            {
                unsigned long i;
                struct csmtolerance_t *tolerances;
                
                solid_loc = csmsolid_new_empty_solid(builder->id_new_element);
                
                i_generate_solid_vertexs(builder->vertexs, solid_loc);
                
                for (i = 0; i < no_faces; i++)
                {
                    struct csmfacbrep2solid_face_t *face;
                    
                    face = csmarrayc_get_st(builder->faces, i, csmfacbrep2solid_face_t);
                    i_generate_solid_face(face, solid_loc, &builder->id_new_element, builder->vertexs);
                }
                
                i_generate_solid_hedges(edges, builder->faces, solid_loc);
                
                tolerances = csmtolerance_new();
                csmsimplifysolid_simplify(solid_loc, tolerances);
                
                if (i_check_inner_loop_orientation(solid_loc) == CSMTRUE)
                {
                    result = CSMFACBREP2SOLID_RESULT_OK;
                }
                else
                {
                    result = CSMFACBREP2SOLID_RESULT_INCONSISTENT_INNER_LOOP_ORIENTATION;
                    csmsolid_free(&solid_loc);
                }
                
                csmtolerance_free(&tolerances);
            }
            
            csmarrayc_free_st(&edges, i_edge_t, i_free_edge);
        }
    }
    
    *solid = solid_loc;
    
    return result;
}




























