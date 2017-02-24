//
//  csmsolid.c
//  cysolidmodeling
//
//  Created by Manuel Fernández on 23/2/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmsolid.inl"
#include "csmsolid.tli"

#include "csmface.inl"
#include "csmhedge.inl"
#include "csmloop.inl"
#include "csmnode.inl"
#include "csmvertex.inl"

#include "cyassert.h"
#include "cypeid.h"
#include "cypespy.h"

// ----------------------------------------------------------------------------------------------------

static void i_csmsolid_destruye(struct csmsolid_t **solido)
{
    assert_no_null(solido);
    assert_no_null(*solido);
    
    csmnode_release_ex(&(*solido)->sfaces, csmface_t);
    csmnode_release_ex(&(*solido)->sedges, csmedge_t);
    csmnode_release_ex(&(*solido)->svertexs, csmvertex_t);
    
    FREE_PP(solido, struct csmsolid_t);
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmsolid_t *, i_crea, (
                        unsigned long id,
                        struct csmface_t *sfaces,
                        struct csmedge_t *sedges,
                        struct csmvertex_t *svertexs))
{
    struct csmsolid_t *solido;
    
    solido = MALLOC(struct csmsolid_t);
    
    solido->clase_base = csmnode_crea_node(id, solido, i_csmsolid_destruye, csmsolid_t);
    
    solido->sfaces = sfaces;
    solido->sedges = sedges;
    solido->svertexs = svertexs;
    
    return solido;
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmsolid_t *, i_crea_solido_vacio, (unsigned long *id_nuevo_elemento))
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

struct csmsolid_t *csmsolid_mvfs(double x, double y, double z, unsigned long *id_nuevo_elemento)
{
    struct csmsolid_t *solido;
    struct csmface_t *face;
    struct csmloop_t *loop;
    struct csmvertex_t *vertex;
    struct csmhedge_t *hedge;
    
    solido = i_crea_solido_vacio(id_nuevo_elemento);
    assert_no_null(solido);
    assert(solido->svertexs == NULL);
    assert(solido->sfaces == NULL);
    assert(solido->sedges == NULL);
    
    face = csmface_crea(solido, id_nuevo_elemento);
    loop = csmloop_crea(face, id_nuevo_elemento);
    vertex = csmvertex_crea(x, y, z, id_nuevo_elemento);
    hedge = csmhedge_crea(id_nuevo_elemento);
    
    csmface_set_flout(face, loop);
    csmface_set_floops(face, loop);
    
    csmloop_set_ledge(loop, hedge);
    
    csmhedge_set_vertex(hedge, vertex);
    csmhedge_set_loop(hedge, loop);
    csmhedge_set_next(hedge, hedge);
    csmhedge_set_prev(hedge, hedge);
    csmvertex_set_hedge(vertex, hedge);
    
    solido->svertexs = vertex;
    solido->sfaces = face;

    csmnode_release_ex(&hedge, csmhedge_t);
    csmnode_release_ex(&loop, csmloop_t);
    
    return solido;
}


