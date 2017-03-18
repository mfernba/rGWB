//
//  csmsolid.c
//  cysolidmodeling
//
//  Created by Manuel Fernández on 23/2/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmsolid.inl"
#include "csmsolid.tli"

#include "csmedge.inl"
#include "csmedge.tli"
#include "csmface.inl"
#include "csmhashtb.inl"
#include "csmhedge.inl"
#include "csmloop.inl"
#include "csmnode.inl"
#include "csmvertex.inl"

#include "cyassert.h"
#include "cypeid.h"
#include "cypespy.h"

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmsolid_t *, i_crea, (
                        unsigned long id,
                        struct csmhashtb(csmface_t) **sfaces,
                        struct csmhashtb(csmedge_t) **sedges,
                        struct csmhashtb(csmvertex_t) **svertexs))
{
    struct csmsolid_t *solido;
    
    solido = MALLOC(struct csmsolid_t);
    
    solido->id = id;
    
    solido->sfaces = ASIGNA_PUNTERO_PP_NO_NULL(sfaces, struct csmhashtb(csmface_t));
    solido->sedges = ASIGNA_PUNTERO_PP_NO_NULL(sedges, struct csmhashtb(csmedge_t));
    solido->svertexs = ASIGNA_PUNTERO_PP_NO_NULL(svertexs, struct csmhashtb(csmvertex_t));
    
    return solido;
}

// ----------------------------------------------------------------------------------------------------

struct csmsolid_t *csmsolid_crea_vacio(unsigned long *id_nuevo_elemento)
{
    unsigned long id;
    struct csmhashtb(csmface_t) *sfaces;
    struct csmhashtb(csmedge_t) *sedges;
    struct csmhashtb(csmvertex_t) *svertexs;
    
    id = cypeid_nuevo_id(id_nuevo_elemento, NULL);
    
    sfaces = csmhashtb_create_empty(csmface_t);
    sedges = csmhashtb_create_empty(csmedge_t);
    svertexs = csmhashtb_create_empty(csmvertex_t);
    
    return i_crea(id, &sfaces, &sedges, &svertexs);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_destruye(struct csmsolid_t **solido)
{
    assert_no_null(solido);
    assert_no_null(*solido);
    
    csmhashtb_free(&(*solido)->sfaces, csmface_t, csmface_destruye);
    csmhashtb_free(&(*solido)->sedges, csmedge_t, csmedge_destruye);
    csmhashtb_free(&(*solido)->svertexs, csmvertex_t, csmvertex_destruye);
    
    FREE_PP(solido, struct csmsolid_t);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_append_new_face(struct csmsolid_t *solido, unsigned long *id_nuevo_elemento, struct csmface_t **face)
{
    struct csmface_t *face_loc;
    
    assert_no_null(solido);
    assert_no_null(face);
    
    face_loc = csmface_crea(solido, id_nuevo_elemento);
    csmhashtb_add_item(solido->sfaces, csmface_id(face_loc), face_loc, csmface_t);
    
    *face = face_loc;
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_append_new_edge(struct csmsolid_t *solido, unsigned long *id_nuevo_elemento, struct csmedge_t **edge)
{
    struct csmedge_t *edge_loc;
    
    assert_no_null(solido);
    assert_no_null(edge);
    
    edge_loc = csmedge_crea(id_nuevo_elemento);
    csmhashtb_add_item(solido->sedges, csmedge_id(edge_loc), edge_loc, csmedge_t);
    
    *edge = edge_loc;
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_append_new_vertex(struct csmsolid_t *solido, double x, double y, double z, unsigned long *id_nuevo_elemento, struct csmvertex_t **vertex)
{
    struct csmvertex_t *vertex_loc;
    
    assert_no_null(solido);
    assert_no_null(vertex);
    
    vertex_loc = csmvertex_crea(x, y, z, id_nuevo_elemento);
    csmhashtb_add_item(solido->svertexs, csmvertex_id(vertex_loc), vertex_loc, csmvertex_t);
    
    *vertex = vertex_loc;
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_remove_face(struct csmsolid_t *solido, struct csmface_t **face)
{
    assert_no_null(solido);
    assert_no_null(face);
    
    csmhashtb_remove_item(solido->sfaces, csmface_id(*face), csmface_t);
    csmface_destruye(face);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_remove_edge(struct csmsolid_t *solido, struct csmedge_t **edge)
{
    assert_no_null(solido);
    assert_no_null(edge);

    csmhashtb_remove_item(solido->sedges, csmedge_id(*edge), csmedge_t);
    csmedge_destruye(edge);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_remove_vertex(struct csmsolid_t *solido, struct csmvertex_t **vertex)
{
    assert_no_null(solido);
    assert_no_null(vertex);

    csmhashtb_remove_item(solido->svertexs, csmvertex_id(*vertex), csmvertex_t);
    csmvertex_destruye(vertex);
}

// ----------------------------------------------------------------------------------------------------

static void i_edge_print_debug_info(struct csmedge_t *edge, CYBOOL assert_si_no_es_integro)
{
    struct csmnode_t *edge_node;
    struct csmhedge_t *he1, *he2;
    
    he1 = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_POS);
    he2 = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_NEG);
    
    edge_node = CSMNODE(edge);
    fprintf(stdout, "\tEdge %lu", csmnode_id(edge_node));
    
    if (he1 != NULL)
    {
        fprintf(stdout, "\tHe1 %lu [%d]", csmnode_id(CSMNODE(he1)), ES_CIERTO(csmhedge_edge(he1) == edge));
        
        if (assert_si_no_es_integro == CIERTO)
            assert(csmhedge_edge(he1) == edge);
    }
    else
    {
        fprintf(stdout, "\tHe1 (null)");
    }
    
    if (he2 != NULL)
    {
        fprintf(stdout, "\tHe2 %lu [%d]", csmnode_id(CSMNODE(he2)), ES_CIERTO(csmhedge_edge(he2) == edge));
        
        if (assert_si_no_es_integro == CIERTO)
            assert(csmhedge_edge(he2) == edge);
    }
    else
    {
        fprintf(stdout, "\tHe2 (null)");
    }
    
    fprintf(stdout, "\n");
}

// ----------------------------------------------------------------------------------------------------

static void i_print_debug_info_edges(struct csmhashtb(csmedge_t) *sedges, CYBOOL assert_si_no_es_integro, unsigned long *num_edges)
{
    struct csmhashtb_iterator(csmedge_t) *iterator;
    
    assert_no_null(num_edges);
    
    iterator = csmhashtb_create_iterator(sedges, csmedge_t);
    *num_edges = 0;
    
    while (csmhashtb_has_next(iterator, csmedge_t) == CIERTO)
    {
        struct csmedge_t *edge;
        
        csmhashtb_next_pair(iterator, NULL, &edge, csmedge_t);
        
        i_edge_print_debug_info(edge, assert_si_no_es_integro);
        (*num_edges)++;
    }
    
    fprintf(stdout, "\tNo. of egdes: %lu\n", *num_edges);
    
    assert(*num_edges == csmhashtb_count(sedges, csmedge_t));
    csmhashtb_free_iterator(&iterator, csmedge_t);
}

// ----------------------------------------------------------------------------------------------------

static void i_print_debug_info_vertex(struct csmvertex_t *vertex, CYBOOL assert_si_no_es_integro)
{
    struct csmnode_t *vertex_node;
    double x, y, z;
    struct csmhedge_t *hedge;
    
    assert_no_null(vertex);
    
    vertex_node = CSMNODE(vertex);
    csmvertex_get_coordenadas(vertex, &x, &y, &z);
    
    fprintf(stdout, "\tVertex %4lu\t%6.3lf\t%6.3lf\t%6.3lf ", csmnode_id(vertex_node), x, y, z);
    
    hedge = csmvertex_hedge(vertex);
    
    if (hedge != NULL)
    {
        fprintf(stdout, "He %6lu\n", csmnode_id(CSMNODE(hedge)));
        
        if (assert_si_no_es_integro == CIERTO)
            assert(csmhedge_vertex(hedge) == vertex);
    }
    else
    {
        fprintf(stdout, "He (null)\n");
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_print_debug_info_vertexs(struct csmhashtb(csmvertex_t) *svertexs, CYBOOL assert_si_no_es_integro, unsigned long *num_vertexs)
{
    struct csmhashtb_iterator(csmvertex_t) *iterator;
    unsigned long num_iters;
    
    assert_no_null(num_vertexs);
    
    iterator = csmhashtb_create_iterator(svertexs, csmvertex_t);
    *num_vertexs = 0;
    
    num_iters = 0;
    
    while (csmhashtb_has_next(iterator, csmvertex_t) == CIERTO)
    {
        struct csmvertex_t *vertex;
        
        assert(num_iters < 1000000);
        num_iters++;
        
        csmhashtb_next_pair(iterator, NULL, &vertex, csmvertex_t);
        i_print_debug_info_vertex(vertex, assert_si_no_es_integro);
        (*num_vertexs)++;
    }
    
    fprintf(stdout, "\tNo. of vertexs: %lu\n", *num_vertexs);
    
    assert(*num_vertexs == csmhashtb_count(svertexs, csmvertex_t));
    csmhashtb_free_iterator(&iterator, csmvertex_t);
}

// ----------------------------------------------------------------------------------------------------

static void i_print_info_debug_loop(struct csmloop_t *loop, CYBOOL is_outer_loop, CYBOOL assert_si_no_es_integro)
{
    struct csmhedge_t *ledge;
    struct csmhedge_t *iterator;
    unsigned long num_iters;
    
    ledge = csmloop_ledge(loop);
    iterator = ledge;
    fprintf(stdout, "\tLoop %4lu: Outer = %d\n", csmnode_id(CSMNODE(loop)), is_outer_loop);
    
    num_iters = 0;
    
    do
    {
        struct csmvertex_t *vertex;
        double x, y, z;
        struct csmedge_t *edge;
        struct csmhedge_t *next_edge;
        
        assert(num_iters < 10000);
        num_iters++;
        
        vertex = csmhedge_vertex(iterator);
        csmvertex_get_coordenadas(vertex, &x, &y, &z);
        
        edge = csmhedge_edge(iterator);
        
        if (edge == NULL)
        {
            fprintf(
                stdout,
                "\t\t(He %4lu [edge (null)], %6.3f, %6.3f, %6.3f, %d)\n",
                csmnode_id(CSMNODE(iterator)),
                x, y, z,
                ES_CIERTO(csmhedge_loop(iterator) == loop));
        }
        else
        {
            fprintf(
                stdout,
                "\t\t(He %4lu [edge %6lu], %6.3f, %6.3f, %6.3f, %d)\n",
                csmnode_id(CSMNODE(iterator)),
                csmnode_id(CSMNODE(edge)),
                x, y, z,
                ES_CIERTO(csmhedge_loop(iterator) == loop));
        }
        
        if (assert_si_no_es_integro == CIERTO)
            assert(csmhedge_loop(iterator) == loop);
        
        next_edge = csmhedge_next(iterator);
        
        if (assert_si_no_es_integro == CIERTO)
            assert(csmhedge_prev(next_edge) == iterator);
                    
        iterator = next_edge;
    }
    while (iterator != ledge);
}

// ----------------------------------------------------------------------------------------------------

static void i_print_info_debug_face(struct csmface_t *face, CYBOOL assert_si_no_es_integro, unsigned long *num_holes)
{
    struct csmloop_t *loop_iterator;
    
    assert_no_null(num_holes);
    
    fprintf(stdout, "\tFace %lu\n", csmnode_id(CSMNODE(face)));
    
    loop_iterator = csmface_floops(face);
    
    while (loop_iterator != NULL)
    {
        struct csmloop_t *next_loop;
        CYBOOL is_outer_loop;
        
        is_outer_loop = ES_CIERTO(csmface_flout(face) == loop_iterator);
        i_print_info_debug_loop(loop_iterator, is_outer_loop, assert_si_no_es_integro);
        
        if (is_outer_loop == FALSO)
            (*num_holes)++;
        
        next_loop = csmloop_next(loop_iterator);
        
        if (assert_si_no_es_integro == CIERTO)
        {
            assert(csmloop_lface(loop_iterator) == face);
            
            if (next_loop != NULL)
                assert(csmloop_prev(next_loop) == loop_iterator);
        }
        
        loop_iterator = next_loop;
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_print_info_debug_faces(
                        struct csmhashtb(csmface_t) *sfaces,
                        struct csmsolid_t *solid,
                        CYBOOL assert_si_no_es_integro,
                        unsigned long *num_faces, unsigned long *num_holes)
{
    struct csmhashtb_iterator(csmface_t) *iterator;
    
    assert_no_null(num_faces);
    assert_no_null(num_holes);
    
    iterator = csmhashtb_create_iterator(sfaces, csmface_t);
    
    *num_faces = 0;
    *num_holes = 0;
    
    while (csmhashtb_has_next(iterator, csmface_t) == CIERTO)
    {
        struct csmface_t *face;
        
        csmhashtb_next_pair(iterator, NULL, &face, csmface_t);
        
        i_print_info_debug_face(face, assert_si_no_es_integro, num_holes);
        (*num_faces)++;
        
        if (assert_si_no_es_integro == CIERTO)
            assert(csmface_fsolid(face) == solid);
    }
    
    fprintf(stdout, "\tNo. of faces: %lu\n", *num_faces);
    
    csmhashtb_free_iterator(&iterator, csmface_t);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_print_debug(struct csmsolid_t *solido, CYBOOL assert_si_no_es_integro)
{
    unsigned long num_faces, num_vertexs, num_edges, num_holes;
    
    assert_no_null(solido);

    fprintf(stdout, "*******************************\n");
    fprintf(stdout, "*******************************\n");
    fprintf(stdout, "Face table\n");
    i_print_info_debug_faces(solido->sfaces, solido, assert_si_no_es_integro, &num_faces, &num_holes);
    fprintf(stdout, "\n");
    
    fprintf(stdout, "Edge table\n");
    i_print_debug_info_edges(solido->sedges, assert_si_no_es_integro, &num_edges);
    fprintf(stdout, "\n");
    
    fprintf(stdout, "Vertex table\n");
    i_print_debug_info_vertexs(solido->svertexs, assert_si_no_es_integro, &num_vertexs);
    fprintf(stdout, "\n");
}























