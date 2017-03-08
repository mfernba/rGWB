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




