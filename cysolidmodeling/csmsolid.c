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
                        struct csmface_t *sfaces,
                        struct csmedge_t *sedges,
                        struct csmvertex_t *svertexs))
{
    struct csmsolid_t *solido;
    
    solido = MALLOC(struct csmsolid_t);
    
    solido->id = id;
    
    solido->sfaces = sfaces;
    solido->sedges = sedges;
    solido->svertexs = svertexs;
    
    return solido;
}

// ----------------------------------------------------------------------------------------------------

struct csmsolid_t *csmsolid_crea_vacio(unsigned long *id_nuevo_elemento)
{
    unsigned long id;
    struct csmface_t *sfaces;
    struct csmedge_t *sedges;
    struct csmvertex_t *svertexs;
    
    id = cypeid_nuevo_id(id_nuevo_elemento, NULL);
    
    sfaces = NULL;
    sedges = NULL;
    svertexs = NULL;
    
    return i_crea(id, sfaces, sedges, svertexs);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_destruye(struct csmsolid_t **solido)
{
    assert_no_null(solido);
    assert_no_null(*solido);
    
    if ((*solido)->sfaces != NULL)
        csmnode_free_node_list(&(*solido)->sfaces, csmface_t);
    
    if ((*solido)->sedges != NULL)
        csmnode_free_node_list(&(*solido)->sedges, csmedge_t);

    if ((*solido)->svertexs != NULL)
        csmnode_free_node_list(&(*solido)->svertexs, csmvertex_t);
    
    FREE_PP(solido, struct csmsolid_t);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_append_new_face(struct csmsolid_t *solido, unsigned long *id_nuevo_elemento, struct csmface_t **face)
{
    struct csmface_t *face_loc;
    
    assert_no_null(solido);
    assert_no_null(face);
    
    face_loc = csmface_crea(solido, id_nuevo_elemento);
    
    if (solido->sfaces != NULL)
        csmnode_insert_node2_before_node1(solido->sfaces, face_loc, csmface_t);
    
    solido->sfaces = face_loc;
    
    *face = face_loc;
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_append_new_edge(struct csmsolid_t *solido, unsigned long *id_nuevo_elemento, struct csmedge_t **edge)
{
    struct csmedge_t *edge_loc;
    
    assert_no_null(solido);
    assert_no_null(edge);
    
    edge_loc = csmedge_crea(id_nuevo_elemento);

    if (solido->sedges != NULL)
        csmnode_insert_node2_before_node1(solido->sedges, edge_loc, csmedge_t);
    
    solido->sedges = edge_loc;
    
    *edge = edge_loc;
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_append_new_vertex(struct csmsolid_t *solido, double x, double y, double z, unsigned long *id_nuevo_elemento, struct csmvertex_t **vertex)
{
    struct csmvertex_t *vertex_loc;
    
    assert_no_null(solido);
    assert_no_null(vertex);
    
    vertex_loc = csmvertex_crea(x, y, z, id_nuevo_elemento);
    
    if (solido->svertexs != NULL)
        csmnode_insert_node2_before_node1(solido->svertexs, vertex_loc, csmvertex_t);

    solido->svertexs = vertex_loc;
    
    *vertex = vertex_loc;
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_remove_face(struct csmsolid_t *solido, struct csmface_t **face)
{
    assert_no_null(solido);
    assert_no_null(face);
    
    if (solido->sfaces == *face)
        solido->sfaces = csmface_next(*face);
    
    csmnode_free_node_in_list(face, csmface_t);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_remove_edge(struct csmsolid_t *solido, struct csmedge_t **edge)
{
    assert_no_null(solido);
    assert_no_null(edge);
    
    if (solido->sedges == *edge)
        solido->sedges = csmedge_next(*edge);
    
    csmnode_free_node_in_list(edge, csmedge_t);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_remove_vertex(struct csmsolid_t *solido, struct csmvertex_t **vertex)
{
    assert_no_null(solido);
    assert_no_null(vertex);
    
    if (solido->svertexs == *vertex)
        solido->svertexs = csmvertex_next(*vertex);
    
    csmnode_free_node_in_list(vertex, csmvertex_t);
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

static void i_print_debug_info_edges(struct csmedge_t *sedges, CYBOOL assert_si_no_es_integro, unsigned long *num_edges)
{
    struct csmedge_t *edge_iterator;
    
    assert_no_null(num_edges);
    
    edge_iterator = sedges;
    *num_edges = 0;
    
    while (edge_iterator != NULL)
    {
        i_edge_print_debug_info(edge_iterator, assert_si_no_es_integro);
        (*num_edges)++;
        
        edge_iterator = csmedge_next(edge_iterator);
    }
    
    fprintf(stdout, "\tNo. of egdes: %lu\n", *num_edges);
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

static void i_print_debug_info_vertexs(struct csmvertex_t *svertexs, CYBOOL assert_si_no_es_integro, unsigned long *num_vertexs)
{
    struct csmvertex_t *iterator;
    unsigned long num_iters;
    
    assert_no_null(num_vertexs);
    
    iterator = svertexs;
    *num_vertexs = 0;
    
    num_iters = 0;
    
    while (iterator != NULL)
    {
        struct csmvertex_t *next_vertex;
        
        assert(num_iters < 1000000);
        num_iters++;
        
        i_print_debug_info_vertex(iterator, assert_si_no_es_integro);
        (*num_vertexs)++;
        
        next_vertex = csmvertex_next(iterator);
        
        if (assert_si_no_es_integro == CIERTO && next_vertex != NULL)
            assert(csmvertex_prev(next_vertex) == iterator);
        
        iterator = next_vertex;
    }
    
    fprintf(stdout, "\tNo. of vertexs: %lu\n", *num_vertexs);
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
        
        fprintf(
                stdout,
                "\t\t(He %4lu [edge %4lu], %6.3f, %6.3f, %6.3f, %d)\n",
                csmnode_id(CSMNODE(iterator)),
                csmnode_id(CSMNODE(edge)),
                x, y, z,
                ES_CIERTO(csmhedge_loop(iterator) == loop));
        
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
                        struct csmface_t *sfaces,
                        struct csmsolid_t *solid,
                        CYBOOL assert_si_no_es_integro,
                        unsigned long *num_faces, unsigned long *num_holes)
{
    struct csmface_t *iterator;
    
    assert_no_null(num_faces);
    assert_no_null(num_holes);
    
    iterator = sfaces;
    
    *num_faces = 0;
    *num_holes = 0;
    
    while (iterator != NULL)
    {
        struct csmface_t *next_face;
        
        i_print_info_debug_face(iterator, assert_si_no_es_integro, num_holes);
        (*num_faces)++;
        
        next_face = csmface_next(iterator);
        
        if (assert_si_no_es_integro == CIERTO)
        {
            assert(csmface_fsolid(iterator) == solid);
            
            if (next_face != NULL)
                assert(csmface_prev(next_face) == iterator);
        }
        
        iterator = next_face;
    }
    
    fprintf(stdout, "\tNo. of faces: %lu\n", *num_faces);
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























