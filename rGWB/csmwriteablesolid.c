//
//  csmwriteablesolid.c
//  rGWB
//
//  Created by Manuel Fernández on 18/6/18.
//  Copyright © 2018 Manuel Fernández. All rights reserved.
//

#include "csmwriteablesolid.inl"

#include "csmarrayc.h"
#include "csmArrULong.h"
#include "csmsave.inl"

#ifdef __STANDALONE_DISTRIBUTABLE
#include "csmassert.inl"
#include "csmmem.inl"
#else
#include "cyassert.h"
#include "cypespy.h"
#endif

struct i_vertex_t
{
    unsigned long vertex_id;
    double x, y, z;
    
    unsigned long hedge_id;
};

struct i_hedge_t
{
    unsigned long hedge_id;
    
    unsigned long loop_id;
    unsigned long vertex_id;
    
    unsigned long hedge_prev_id, hedge_next_id;
};

struct i_loop_t
{
    unsigned long loop_id;
    
    unsigned long ledge_id;
    unsigned long face_id;
};

struct i_face_t
{
    unsigned long face_id;
    
    unsigned long outer_loop_id;
    csmArrULong *loops_ids;
};

struct i_edge_t
{
    unsigned long edge_id;
    unsigned long hedge_pos_id;
    unsigned long hedge_neg_id;
};

struct csmwriteablesolid_t
{
    csmArrayStruct(i_vertex_t) *vertexs;
    csmArrayStruct(i_hedge_t) *hedges;
    csmArrayStruct(i_loop_t) *loops;
    csmArrayStruct(i_face_t) *faces;
    csmArrayStruct(i_edge_t) *edges;
};

static const unsigned char i_FILE_VERSION = 0;

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_vertex_t *, i_new_vertex, (
                        unsigned long vertex_id,
                        double x, double y, double z,
                        unsigned long hedge_id))
{
    struct i_vertex_t *vertex;
    
    vertex = MALLOC(struct i_vertex_t);
    
    vertex->vertex_id = vertex_id;
    
    vertex->x = x;
    vertex->y = y;
    vertex->z = z;
    
    vertex->hedge_id = hedge_id;
    
    return vertex;
}

// ----------------------------------------------------------------------------------------------------            

static void i_free_vertex(struct i_vertex_t **vertex)
{
    assert_no_null(vertex);
    assert_no_null(*vertex);
    
    FREE_PP(vertex, struct i_vertex_t);
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_vertex_t *, i_read_vertex, (struct csmsave_t *csmsave))
{
    unsigned long vertex_id;
    double x, y, z;
    unsigned long hedge_id;
    
    vertex_id = csmsave_read_ulong(csmsave);
    
    x = csmsave_read_double(csmsave);
    y = csmsave_read_double(csmsave);
    z = csmsave_read_double(csmsave);
    
    hedge_id = csmsave_read_ulong(csmsave);
    
    return i_new_vertex(vertex_id, x, y, z, hedge_id);
}

// ----------------------------------------------------------------------------------------------------

static void i_write_vertex(const struct i_vertex_t *vertex, struct csmsave_t *csmsave)
{
    assert_no_null(vertex);
    
    csmsave_write_ulong(csmsave, vertex->vertex_id);
    
    csmsave_write_double(csmsave, vertex->x);
    csmsave_write_double(csmsave, vertex->y);
    csmsave_write_double(csmsave, vertex->z);

    csmsave_write_ulong(csmsave, vertex->hedge_id);
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_hedge_t *, i_new_hedge, (
                        unsigned long hedge_id,
                        unsigned long loop_id, unsigned long vertex_id,
                        unsigned long hedge_prev_id, unsigned long hedge_next_id))
{
    struct i_hedge_t *hedge;
    
    hedge = MALLOC(struct i_hedge_t);
    
    hedge->vertex_id = vertex_id;
    
    hedge->loop_id = loop_id;
    hedge->vertex_id = vertex_id;
    
    hedge->hedge_prev_id = hedge_prev_id;
    hedge->hedge_next_id = hedge_next_id;
    
    return hedge;
}

// ----------------------------------------------------------------------------------------------------

static void i_free_hedge(struct i_hedge_t **hedge)
{
    assert_no_null(hedge);
    assert_no_null(*hedge);
    
    FREE_PP(hedge, struct i_hedge_t);
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_hedge_t *, i_read_hedge, (struct csmsave_t *csmsave))
{
    unsigned long hedge_id, loop_id, vertex_id;
    unsigned long hedge_prev_id, hedge_next_id;
    
    hedge_id = csmsave_read_ulong(csmsave);
    
    loop_id = csmsave_read_ulong(csmsave);
    vertex_id = csmsave_read_ulong(csmsave);
    
    hedge_prev_id = csmsave_read_ulong(csmsave);
    hedge_next_id = csmsave_read_ulong(csmsave);
    
    return i_new_hedge(
                    hedge_id,
                    loop_id, vertex_id,
                    hedge_prev_id, hedge_next_id);
}

// ----------------------------------------------------------------------------------------------------

static void i_write_hedge(const struct i_hedge_t *hedge, struct csmsave_t *csmsave)
{
    assert_no_null(hedge);
    
    csmsave_write_ulong(csmsave, hedge->vertex_id);
    
    csmsave_write_ulong(csmsave, hedge->loop_id);
    csmsave_write_ulong(csmsave, hedge->vertex_id);
    
    csmsave_write_ulong(csmsave, hedge->hedge_prev_id);
    csmsave_write_ulong(csmsave, hedge->hedge_next_id);
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_loop_t *, i_new_loop, (
                        unsigned long loop_id,
                        unsigned long ledge_id, unsigned long face_id))
{
    struct i_loop_t *loop;
    
    loop = MALLOC(struct i_loop_t);
    
    loop->loop_id = loop_id;
    
    loop->ledge_id = ledge_id;
    loop->face_id = face_id;
    
    return loop;
}

// ----------------------------------------------------------------------------------------------------

static void i_free_loop(struct i_loop_t **loop)
{
    assert_no_null(loop);
    assert_no_null(*loop);
    
    FREE_PP(loop, struct i_loop_t);
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_loop_t *, i_read_loop, (struct csmsave_t *csmsave))
{
    unsigned long loop_id, ledge_id, face_id;
    
    loop_id = csmsave_read_ulong(csmsave);
    
    ledge_id = csmsave_read_ulong(csmsave);
    face_id = csmsave_read_ulong(csmsave);
    
    return i_new_loop(loop_id, ledge_id, face_id);
}

// ----------------------------------------------------------------------------------------------------

static void i_write_loop(const struct i_loop_t *loop, struct csmsave_t *csmsave)
{
    assert_no_null(loop);
    
    csmsave_write_ulong(csmsave, loop->loop_id);
    
    csmsave_write_ulong(csmsave, loop->ledge_id);
    csmsave_write_ulong(csmsave, loop->face_id);
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_face_t *, i_new_face, (
                        unsigned long face_id,
                        unsigned long outer_loop_id, csmArrULong **loops_ids))
{
    struct i_face_t *face;
    
    face = MALLOC(struct i_face_t);
    
    face->face_id = face_id;
    
    face->outer_loop_id = outer_loop_id;
    face->loops_ids = ASSIGN_POINTER_PP_NOT_NULL(loops_ids, csmArrULong);
    
    return face;
}

// ----------------------------------------------------------------------------------------------------

static void i_free_face(struct i_face_t **face)
{
    assert_no_null(face);
    assert_no_null(*face);
    
    csmArrULong_free(&(*face)->loops_ids);
    
    FREE_PP(face, struct i_face_t);
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_face_t *, i_read_face, (struct csmsave_t *csmsave))
{
    unsigned long face_id;
    unsigned long outer_loop_id;
    csmArrULong *loops_ids;
    
    face_id = csmsave_read_ulong(csmsave);
    
    outer_loop_id = csmsave_read_ulong(csmsave);
    loops_ids = csmsave_read_arr_ulong(csmsave);
    
    return i_new_face(face_id, outer_loop_id, &loops_ids);
}

// ----------------------------------------------------------------------------------------------------

static void i_write_face(const struct i_face_t *face, struct csmsave_t *csmsave)
{
    assert_no_null(face);
    
    csmsave_write_ulong(csmsave, face->face_id);
    
    csmsave_write_ulong(csmsave, face->outer_loop_id);
    csmsave_write_arr_ulong(csmsave, face->loops_ids);
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_edge_t *, i_new_edge, (
                        unsigned long edge_id,
                        unsigned long hedge_pos_id, unsigned long hedge_neg_id))
{
    struct i_edge_t *edge;
    
    edge = MALLOC(struct i_edge_t);
    
    edge->edge_id = edge_id;
    
    edge->hedge_pos_id = hedge_pos_id;
    edge->hedge_neg_id = hedge_neg_id;
    
    return edge;
}

// ----------------------------------------------------------------------------------------------------

static void i_free_edge(struct i_edge_t **edge)
{
    assert_no_null(edge);
    assert_no_null(*edge);
    
    FREE_PP(edge, struct i_edge_t);
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_edge_t *, i_read_edge, (struct csmsave_t *csmsave))
{
    unsigned long edge_id;
    unsigned long hedge_pos_id, hedge_neg_id;
    
    edge_id = csmsave_read_ulong(csmsave);
    
    hedge_pos_id = csmsave_read_ulong(csmsave);
    hedge_neg_id = csmsave_read_ulong(csmsave);
    
    return i_new_edge(edge_id, hedge_pos_id, hedge_neg_id);
}

// ----------------------------------------------------------------------------------------------------

static void i_write_edge(const struct i_edge_t *edge, struct csmsave_t *csmsave)
{
    assert_no_null(edge);
    
    csmsave_write_ulong(csmsave, edge->edge_id);
    
    csmsave_write_ulong(csmsave, edge->hedge_pos_id);
    csmsave_write_ulong(csmsave, edge->hedge_neg_id);
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmwriteablesolid_t *, i_new_writable_solid, (
                        csmArrayStruct(i_vertex_t) **vertexs,
                        csmArrayStruct(i_hedge_t) **hedges,
                        csmArrayStruct(i_loop_t) **loops,
                        csmArrayStruct(i_face_t) **faces,
                        csmArrayStruct(i_edge_t) **edges))
{
    struct csmwriteablesolid_t *writeable_solid;
    
    writeable_solid = MALLOC(struct csmwriteablesolid_t);
    
    writeable_solid->vertexs = ASSIGN_POINTER_PP_NOT_NULL(vertexs, csmArrayStruct(i_vertex_t));
    writeable_solid->hedges = ASSIGN_POINTER_PP_NOT_NULL(hedges, csmArrayStruct(i_hedge_t));
    writeable_solid->loops = ASSIGN_POINTER_PP_NOT_NULL(loops, csmArrayStruct(i_loop_t));
    writeable_solid->faces = ASSIGN_POINTER_PP_NOT_NULL(faces, csmArrayStruct(i_face_t));
    writeable_solid->edges = ASSIGN_POINTER_PP_NOT_NULL(edges, csmArrayStruct(i_edge_t));
    
    return writeable_solid;
}

// ----------------------------------------------------------------------------------------------------

struct csmwriteablesolid_t *csmwriteablesolid_new(void)
{
    csmArrayStruct(i_vertex_t) *vertexs;
    csmArrayStruct(i_hedge_t) *hedges;
    csmArrayStruct(i_loop_t) *loops;
    csmArrayStruct(i_face_t) *faces;
    csmArrayStruct(i_edge_t) *edges;
    
    vertexs = csmarrayc_new_st_array(0, i_vertex_t);
    hedges = csmarrayc_new_st_array(0, i_hedge_t);
    loops = csmarrayc_new_st_array(0, i_loop_t);
    faces = csmarrayc_new_st_array(0, i_face_t);
    edges = csmarrayc_new_st_array(0, i_edge_t);

    return i_new_writable_solid(&vertexs, &hedges, &loops, &faces, &edges);
}

// ----------------------------------------------------------------------------------------------------

void csmwriteablesolid_free(struct csmwriteablesolid_t **writeable_solid)
{
    assert_no_null(writeable_solid);
    assert_no_null(*writeable_solid);
    
    csmarrayc_free_st(&(*writeable_solid)->vertexs, i_vertex_t, i_free_vertex);
    csmarrayc_free_st(&(*writeable_solid)->hedges, i_hedge_t, i_free_hedge);
    csmarrayc_free_st(&(*writeable_solid)->loops, i_loop_t, i_free_loop);
    csmarrayc_free_st(&(*writeable_solid)->faces, i_face_t, i_free_face);
    csmarrayc_free_st(&(*writeable_solid)->edges, i_edge_t, i_free_edge);
    
    FREE_PP(writeable_solid, struct csmwriteablesolid_t);
}

// ----------------------------------------------------------------------------------------------------

struct csmwriteablesolid_t *csmwriteablesolid_read(struct csmsave_t *csmsave)
{
    csmArrayStruct(i_vertex_t) *vertexs;
    csmArrayStruct(i_hedge_t) *hedges;
    csmArrayStruct(i_loop_t) *loops;
    csmArrayStruct(i_face_t) *faces;
    csmArrayStruct(i_edge_t) *edges;
    
    vertexs = csmsave_read_arr_st(csmsave, i_read_vertex, i_vertex_t);
    hedges = csmsave_read_arr_st(csmsave, i_read_hedge, i_hedge_t);
    loops = csmsave_read_arr_st(csmsave, i_read_loop, i_loop_t);
    faces = csmsave_read_arr_st(csmsave, i_read_face, i_face_t);
    edges = csmsave_read_arr_st(csmsave, i_read_edge, i_edge_t);

    return i_new_writable_solid(&vertexs, &hedges, &loops, &faces, &edges);
}

// ----------------------------------------------------------------------------------------------------

void csmwriteablesolid_write(const struct csmwriteablesolid_t *writeable_solid, struct csmsave_t *csmsave)
{
    assert_no_null(writeable_solid);
    
    csmsave_write_uchar(csmsave, i_FILE_VERSION);
    
    csmsave_write_arr_st(csmsave, writeable_solid->vertexs, i_write_vertex, i_vertex_t);
    csmsave_write_arr_st(csmsave, writeable_solid->hedges, i_write_hedge, i_hedge_t);
    csmsave_write_arr_st(csmsave, writeable_solid->loops, i_write_loop, i_loop_t);
    csmsave_write_arr_st(csmsave, writeable_solid->faces, i_write_face, i_face_t);
    csmsave_write_arr_st(csmsave, writeable_solid->edges, i_write_edge, i_edge_t);
}

// ----------------------------------------------------------------------------------------------------

void csmwriteablesolid_append_vertex(
                        struct csmwriteablesolid_t *writeable_solid,
                        unsigned long vertex_id,
                        double x, double y, double z,
                        unsigned long hedge_id)
{
    struct i_vertex_t *vertex;
    
    assert_no_null(writeable_solid);
    
    vertex = i_new_vertex(vertex_id, x, y, z, hedge_id);
    csmarrayc_append_element_st(writeable_solid->vertexs, vertex, i_vertex_t);
}

// ----------------------------------------------------------------------------------------------------

void csmwriteablesolid_append_hedge(
                        struct csmwriteablesolid_t *writeable_solid,
                        unsigned long hedge_id,
                        unsigned long loop_id, unsigned long vertex_id,
                        unsigned long hedge_prev_id, unsigned long hedge_next_id)
{
    struct i_hedge_t *hedge;
    
    assert_no_null(writeable_solid);
    
    hedge = i_new_hedge(hedge_id, loop_id, vertex_id, hedge_prev_id, hedge_next_id);
    csmarrayc_append_element_st(writeable_solid->hedges, hedge, i_hedge_t);
}

// ----------------------------------------------------------------------------------------------------

void csmwriteablesolid_append_loop(
                        struct csmwriteablesolid_t *writeable_solid,
                        unsigned long loop_id,
                        unsigned long ledge_id, unsigned long face_id)
{
    struct i_loop_t *loop;
    
    assert_no_null(writeable_solid);
    
    loop = i_new_loop(loop_id, ledge_id, face_id);
    csmarrayc_append_element_st(writeable_solid->loops, loop, i_loop_t);
}

// ----------------------------------------------------------------------------------------------------

void csmwriteablesolid_append_face(
                        struct csmwriteablesolid_t *writeable_solid,
                        unsigned long face_id,
                        unsigned long outer_loop_id, csmArrULong **loops_ids)
{
    struct i_face_t *face;
    
    assert_no_null(writeable_solid);
    
    face = i_new_face(face_id, outer_loop_id, loops_ids);
    csmarrayc_append_element_st(writeable_solid->faces, face, i_face_t);
}

// ----------------------------------------------------------------------------------------------------

void csmwriteablesolid_append_edge(
                        struct csmwriteablesolid_t *writeable_solid,
                        unsigned long edge_id,
                        unsigned long hedge_pos_id, unsigned long hedge_neg_id)
{
    struct i_edge_t *edge;
    
    assert_no_null(writeable_solid);
    
    edge = i_new_edge(edge_id, hedge_pos_id, hedge_neg_id);
    csmarrayc_append_element_st(writeable_solid->edges, edge, i_edge_t);
}

