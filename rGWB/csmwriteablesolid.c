//
//  csmwriteablesolid.c
//  rGWB
//
//  Created by Manuel Fernández on 18/6/18.
//  Copyright © 2018 Manuel Fernández. All rights reserved.
//

#include "csmwriteablesolid.inl"
#include "csmwriteablesolid.tli"

#include "csmarrayc.h"
#include "csmArrULong.h"
#include "csmsave.h"
#include "csmmaterial.inl"
#include "csmsurface.inl"

#ifdef __STANDALONE_DISTRIBUTABLE
#include "csmassert.inl"
#include "csmmem.inl"
#else
#include "cyassert.h"
#include "cypespy.h"
#endif


static const unsigned char i_FILE_VERSION = 0;

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmwriteablesolid_vertex_t *, i_new_vertex, (
                        unsigned long vertex_id,
                        double x, double y, double z,
                        unsigned long hedge_id))
{
    struct csmwriteablesolid_vertex_t *vertex;
    
    vertex = MALLOC(struct csmwriteablesolid_vertex_t);
    
    vertex->vertex_id = vertex_id;
    
    vertex->x = x;
    vertex->y = y;
    vertex->z = z;
    
    vertex->hedge_id = hedge_id;
    
    return vertex;
}

// ----------------------------------------------------------------------------------------------------            

static void i_free_vertex(struct csmwriteablesolid_vertex_t **vertex)
{
    assert_no_null(vertex);
    assert_no_null(*vertex);
    
    FREE_PP(vertex, struct csmwriteablesolid_vertex_t);
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmwriteablesolid_vertex_t *, i_read_vertex, (struct csmsave_t *csmsave))
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

static void i_write_vertex(const struct csmwriteablesolid_vertex_t *vertex, struct csmsave_t *csmsave)
{
    assert_no_null(vertex);
    
    csmsave_write_ulong(csmsave, vertex->vertex_id);
    
    csmsave_write_double(csmsave, vertex->x);
    csmsave_write_double(csmsave, vertex->y);
    csmsave_write_double(csmsave, vertex->z);

    csmsave_write_ulong(csmsave, vertex->hedge_id);
}

// ----------------------------------------------------------------------------------------------------

struct csmwriteablesolid_hedge_t *csmwriteablesolid_new_hedge(
                        unsigned long hedge_id,
                        unsigned long loop_id,
                        unsigned long vertex_id, CSMBOOL is_reference_hedge_of_vertex)
{
    struct csmwriteablesolid_hedge_t *hedge;
    
    hedge = MALLOC(struct csmwriteablesolid_hedge_t);
    
    hedge->hedge_id = hedge_id;
    
    hedge->loop_id = loop_id;
    hedge->vertex_id = vertex_id;
    hedge->is_reference_hedge_of_vertex = is_reference_hedge_of_vertex;
    
    return hedge;
}

// ----------------------------------------------------------------------------------------------------

static void i_free_hedge(struct csmwriteablesolid_hedge_t **hedge)
{
    assert_no_null(hedge);
    assert_no_null(*hedge);
    
    FREE_PP(hedge, struct csmwriteablesolid_hedge_t);
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmwriteablesolid_hedge_t *, i_read_hedge, (struct csmsave_t *csmsave))
{
    unsigned long hedge_id, loop_id, vertex_id;
    CSMBOOL is_reference_hedge_of_vertex;
    
    hedge_id = csmsave_read_ulong(csmsave);
    
    loop_id = csmsave_read_ulong(csmsave);
    
    vertex_id = csmsave_read_ulong(csmsave);
    is_reference_hedge_of_vertex = csmsave_read_bool(csmsave);
    
    return csmwriteablesolid_new_hedge(
                        hedge_id,
                        loop_id,
                        vertex_id, is_reference_hedge_of_vertex);
}

// ----------------------------------------------------------------------------------------------------

static void i_write_hedge(const struct csmwriteablesolid_hedge_t *hedge, struct csmsave_t *csmsave)
{
    assert_no_null(hedge);
    
    csmsave_write_ulong(csmsave, hedge->hedge_id);
    
    csmsave_write_ulong(csmsave, hedge->loop_id);
    
    csmsave_write_ulong(csmsave, hedge->vertex_id);
    csmsave_write_bool(csmsave, hedge->is_reference_hedge_of_vertex);
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(struct csmwriteablesolid_loop_t *, csmwriteablesolid_loop_new, (
                        unsigned long loop_id,
                        csmArrayStruct(csmwriteablesolid_hedge_t) **hedges, unsigned long face_id))
{
    struct csmwriteablesolid_loop_t *loop;
    
    loop = MALLOC(struct csmwriteablesolid_loop_t);
    
    loop->loop_id = loop_id;
    
    loop->hedges = ASSIGN_POINTER_PP_NOT_NULL(hedges, csmArrayStruct(csmwriteablesolid_hedge_t));
    loop->face_id = face_id;
    
    return loop;
}

// ----------------------------------------------------------------------------------------------------

static void i_free_loop(struct csmwriteablesolid_loop_t **loop)
{
    assert_no_null(loop);
    assert_no_null(*loop);
    
    csmarrayc_free_st(&(*loop)->hedges, csmwriteablesolid_hedge_t, i_free_hedge);
    
    FREE_PP(loop, struct csmwriteablesolid_loop_t);
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmwriteablesolid_loop_t *, i_read_loop, (struct csmsave_t *csmsave))
{
    unsigned long loop_id;
    csmArrayStruct(csmwriteablesolid_hedge_t) *hedges;
    unsigned long face_id;
    
    loop_id = csmsave_read_ulong(csmsave);
    
    hedges = csmsave_read_arr_st(csmsave, i_read_hedge, csmwriteablesolid_hedge_t);
    face_id = csmsave_read_ulong(csmsave);
    
    return csmwriteablesolid_loop_new(loop_id, &hedges, face_id);
}

// ----------------------------------------------------------------------------------------------------

static void i_write_loop(const struct csmwriteablesolid_loop_t *loop, struct csmsave_t *csmsave)
{
    assert_no_null(loop);
    
    csmsave_write_ulong(csmsave, loop->loop_id);
    
    csmsave_write_arr_st(csmsave, loop->hedges, i_write_hedge, csmwriteablesolid_hedge_t);
    csmsave_write_ulong(csmsave, loop->face_id);
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmwriteablesolid_face_t *, i_new_face, (
                        unsigned long face_id,
                        unsigned long outer_loop_id, csmArrayStruct(csmwriteablesolid_loop_t) **floops,
                        struct csmsurface_t **surface_eq,
                        struct csmmaterial_t **visz_material_opt))
{
    struct csmwriteablesolid_face_t *face;
    
    face = MALLOC(struct csmwriteablesolid_face_t);
    
    face->face_id = face_id;
    
    face->outer_loop_id = outer_loop_id;
    face->floops = ASSIGN_POINTER_PP_NOT_NULL(floops, csmArrayStruct(csmwriteablesolid_loop_t));
    
    face->surface_eq = ASSIGN_POINTER_PP_NOT_NULL(surface_eq, struct csmsurface_t);
    face->visz_material_opt = ASSIGN_POINTER_PP_NOT_NULL(visz_material_opt, struct csmmaterial_t);
    
    return face;
}

// ----------------------------------------------------------------------------------------------------

static void i_free_face(struct csmwriteablesolid_face_t **face)
{
    assert_no_null(face);
    assert_no_null(*face);
    
    csmarrayc_free_st(&(*face)->floops, csmwriteablesolid_loop_t, i_free_loop);
    
    csmsurface_free(&(*face)->surface_eq);
    
    if ((*face)->visz_material_opt != NULL)
        csmmaterial_free(&(*face)->visz_material_opt);
    
    FREE_PP(face, struct csmwriteablesolid_face_t);
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmwriteablesolid_face_t *, i_read_face, (struct csmsave_t *csmsave))
{
    unsigned long face_id;
    unsigned long outer_loop_id;
    csmArrayStruct(csmwriteablesolid_loop_t) *floops;
    struct csmsurface_t *surface_eq;
    struct csmmaterial_t *visz_material_opt;
    
    face_id = csmsave_read_ulong(csmsave);
    
    outer_loop_id = csmsave_read_ulong(csmsave);
    floops = csmsave_read_arr_st(csmsave, i_read_loop, csmwriteablesolid_loop_t);

    surface_eq = csmsurface_read(csmsave);
    visz_material_opt = csmsave_read_optional_st(csmsave, csmmaterial_read, csmmaterial_t);
    
    return i_new_face(face_id, outer_loop_id, &floops, &surface_eq, &visz_material_opt);
}

// ----------------------------------------------------------------------------------------------------

static void i_write_face(const struct csmwriteablesolid_face_t *face, struct csmsave_t *csmsave)
{
    assert_no_null(face);
    
    csmsave_write_ulong(csmsave, face->face_id);
    
    csmsave_write_ulong(csmsave, face->outer_loop_id);
    csmsave_write_arr_st(csmsave, face->floops, i_write_loop, csmwriteablesolid_loop_t);
    
    csmsurface_write(face->surface_eq, csmsave);
    csmsave_write_optional_st(csmsave, face->visz_material_opt, csmmaterial_write, csmmaterial_t);
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmwriteablesolid_edge_t *, i_new_edge, (
                        unsigned long edge_id,
                        unsigned long hedge_pos_id, unsigned long hedge_neg_id))
{
    struct csmwriteablesolid_edge_t *edge;
    
    edge = MALLOC(struct csmwriteablesolid_edge_t);
    
    edge->edge_id = edge_id;
    
    edge->hedge_pos_id = hedge_pos_id;
    edge->hedge_neg_id = hedge_neg_id;
    
    return edge;
}

// ----------------------------------------------------------------------------------------------------

static void i_free_edge(struct csmwriteablesolid_edge_t **edge)
{
    assert_no_null(edge);
    assert_no_null(*edge);
    
    FREE_PP(edge, struct csmwriteablesolid_edge_t);
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmwriteablesolid_edge_t *, i_read_edge, (struct csmsave_t *csmsave))
{
    unsigned long edge_id;
    unsigned long hedge_pos_id, hedge_neg_id;
    
    edge_id = csmsave_read_ulong(csmsave);
    
    hedge_pos_id = csmsave_read_ulong(csmsave);
    hedge_neg_id = csmsave_read_ulong(csmsave);
    
    return i_new_edge(edge_id, hedge_pos_id, hedge_neg_id);
}

// ----------------------------------------------------------------------------------------------------

static void i_write_edge(const struct csmwriteablesolid_edge_t *edge, struct csmsave_t *csmsave)
{
    assert_no_null(edge);
    
    csmsave_write_ulong(csmsave, edge->edge_id);
    
    csmsave_write_ulong(csmsave, edge->hedge_pos_id);
    csmsave_write_ulong(csmsave, edge->hedge_neg_id);
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmwriteablesolid_t *, i_new_writable_solid, (
                        csmArrayStruct(csmwriteablesolid_vertex_t) **vertexs,
                        csmArrayStruct(csmwriteablesolid_face_t) **faces,
                        csmArrayStruct(csmwriteablesolid_edge_t) **edges))
{
    struct csmwriteablesolid_t *writeable_solid;
    
    writeable_solid = MALLOC(struct csmwriteablesolid_t);
    
    writeable_solid->vertexs = ASSIGN_POINTER_PP_NOT_NULL(vertexs, csmArrayStruct(csmwriteablesolid_vertex_t));
    writeable_solid->faces = ASSIGN_POINTER_PP_NOT_NULL(faces, csmArrayStruct(csmwriteablesolid_face_t));
    writeable_solid->edges = ASSIGN_POINTER_PP_NOT_NULL(edges, csmArrayStruct(csmwriteablesolid_edge_t));
    
    return writeable_solid;
}

// ----------------------------------------------------------------------------------------------------

struct csmwriteablesolid_t *csmwriteablesolid_new(void)
{
    csmArrayStruct(csmwriteablesolid_vertex_t) *vertexs;
    csmArrayStruct(csmwriteablesolid_face_t) *faces;
    csmArrayStruct(csmwriteablesolid_edge_t) *edges;
    
    vertexs = csmarrayc_new_st_array(0, csmwriteablesolid_vertex_t);
    faces = csmarrayc_new_st_array(0, csmwriteablesolid_face_t);
    edges = csmarrayc_new_st_array(0, csmwriteablesolid_edge_t);

    return i_new_writable_solid(&vertexs, &faces, &edges);
}

// ----------------------------------------------------------------------------------------------------

void csmwriteablesolid_free(struct csmwriteablesolid_t **writeable_solid)
{
    assert_no_null(writeable_solid);
    assert_no_null(*writeable_solid);
    
    csmarrayc_free_st(&(*writeable_solid)->vertexs, csmwriteablesolid_vertex_t, i_free_vertex);
    csmarrayc_free_st(&(*writeable_solid)->faces, csmwriteablesolid_face_t, i_free_face);
    csmarrayc_free_st(&(*writeable_solid)->edges, csmwriteablesolid_edge_t, i_free_edge);
    
    FREE_PP(writeable_solid, struct csmwriteablesolid_t);
}

// ----------------------------------------------------------------------------------------------------

struct csmwriteablesolid_t *csmwriteablesolid_read(struct csmsave_t *csmsave)
{
    unsigned char file_version;
    
    csmArrayStruct(csmwriteablesolid_vertex_t) *vertexs;
    csmArrayStruct(csmwriteablesolid_face_t) *faces;
    csmArrayStruct(csmwriteablesolid_edge_t) *edges;
    
    file_version = csmsave_read_uchar(csmsave);
    assert(file_version == 0);
    
    vertexs = csmsave_read_arr_st(csmsave, i_read_vertex, csmwriteablesolid_vertex_t);
    faces = csmsave_read_arr_st(csmsave, i_read_face, csmwriteablesolid_face_t);
    edges = csmsave_read_arr_st(csmsave, i_read_edge, csmwriteablesolid_edge_t);

    return i_new_writable_solid(&vertexs, &faces, &edges);
}

// ----------------------------------------------------------------------------------------------------

void csmwriteablesolid_write(const struct csmwriteablesolid_t *writeable_solid, struct csmsave_t *csmsave)
{
    assert_no_null(writeable_solid);
    
    csmsave_write_uchar(csmsave, i_FILE_VERSION);
    
    csmsave_write_arr_st(csmsave, writeable_solid->vertexs, i_write_vertex, csmwriteablesolid_vertex_t);
    csmsave_write_arr_st(csmsave, writeable_solid->faces, i_write_face, csmwriteablesolid_face_t);
    csmsave_write_arr_st(csmsave, writeable_solid->edges, i_write_edge, csmwriteablesolid_edge_t);
}

// ----------------------------------------------------------------------------------------------------

void csmwriteablesolid_append_vertex(
                        struct csmwriteablesolid_t *writeable_solid,
                        unsigned long vertex_id,
                        double x, double y, double z,
                        unsigned long hedge_id)
{
    struct csmwriteablesolid_vertex_t *vertex;
    
    assert_no_null(writeable_solid);
    
    vertex = i_new_vertex(vertex_id, x, y, z, hedge_id);
    csmarrayc_append_element_st(writeable_solid->vertexs, vertex, csmwriteablesolid_vertex_t);
}

// ----------------------------------------------------------------------------------------------------

void csmwriteablesolid_append_face(
                        struct csmwriteablesolid_t *writeable_solid,
                        unsigned long face_id,
                        unsigned long outer_loop_id, csmArrayStruct(csmwriteablesolid_loop_t) **floops,
                        struct csmsurface_t **surface_eq,
                        struct csmmaterial_t **visz_material_opt)
{
    struct csmwriteablesolid_face_t *face;
    
    assert_no_null(writeable_solid);
    
    face = i_new_face(face_id, outer_loop_id, floops, surface_eq, visz_material_opt);
    csmarrayc_append_element_st(writeable_solid->faces, face, csmwriteablesolid_face_t);
}

// ----------------------------------------------------------------------------------------------------

void csmwriteablesolid_append_edge(
                        struct csmwriteablesolid_t *writeable_solid,
                        unsigned long edge_id,
                        unsigned long hedge_pos_id, unsigned long hedge_neg_id)
{
    struct csmwriteablesolid_edge_t *edge;
    
    assert_no_null(writeable_solid);
    
    edge = i_new_edge(edge_id, hedge_pos_id, hedge_neg_id);
    csmarrayc_append_element_st(writeable_solid->edges, edge, csmwriteablesolid_edge_t);
}

