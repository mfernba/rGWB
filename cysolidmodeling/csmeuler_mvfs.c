//
//  csmeuler.c
//  cysolidmodeling
//
//  Created by Manuel Fernández on 24/2/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmeuler_mvfs.inl"

#include "csmface.inl"
#include "csmhedge.inl"
#include "csmloop.inl"
#include "csmnode.inl"
#include "csmvertex.inl"
#include "csmsolid.inl"

// ----------------------------------------------------------------------------------------------------

struct csmsolid_t *csmeuler_mvfs(double x, double y, double z, unsigned long *id_nuevo_elemento, struct csmhedge_t **hedge_opc)
{
    struct csmsolid_t *solido;
    struct csmvertex_t *vertex;
    struct csmface_t *face;
    struct csmloop_t *loop;
    struct csmhedge_t *hedge;
    
    solido = csmsolid_crea_vacio(id_nuevo_elemento);
    
    csmsolid_append_new_face(solido, id_nuevo_elemento, &face);
    loop = csmloop_crea(face, id_nuevo_elemento);
    hedge = csmhedge_crea(id_nuevo_elemento);
    csmsolid_append_new_vertex(solido, x, y, z, id_nuevo_elemento, &vertex);
    
    csmface_set_flout(face, loop);

    csmloop_set_ledge(loop, hedge);
    
    csmhedge_set_vertex(hedge, vertex);
    csmhedge_set_loop(hedge, loop);
    csmhedge_set_next(hedge, hedge);
    csmhedge_set_prev(hedge, hedge);
    
    csmvertex_set_hedge(vertex, hedge);
    
    ASIGNA_OPC(hedge_opc, hedge);
    
    return solido;
}
