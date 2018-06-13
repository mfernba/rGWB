//
//  csmeuler.c
//  rGWB
//
//  Created by Manuel Fernández on 24/2/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmeuler_mvfs.inl"

#include "csmface.inl"
#include "csmhedge.inl"
#include "csmloop.inl"
#include "csmsolid.inl"
#include "csmvertex.inl"

// ----------------------------------------------------------------------------------------------------

struct csmsolid_t *csmeuler_mvfs(double x, double y, double z, unsigned long start_id_of_new_element, struct csmhedge_t **hedge_opc)
{
    struct csmsolid_t *solid;
    unsigned long *id_nuevo_elemento;
    struct csmvertex_t *vertex;
    struct csmface_t *face;
    struct csmloop_t *loop;
    struct csmhedge_t *hedge;
    
    solid = csmsolid_crea_vacio(start_id_of_new_element);
    
    csmsolid_append_new_face(solid, &face);
    
    id_nuevo_elemento = csmsolid_id_new_element(solid);
    loop = csmloop_crea(face, id_nuevo_elemento);
    hedge = csmhedge_crea(id_nuevo_elemento);
    csmsolid_append_new_vertex(solid, x, y, z, &vertex);
    
    csmface_set_flout(face, loop);

    csmloop_set_ledge(loop, hedge);
    
    csmhedge_set_vertex(hedge, vertex);
    csmhedge_set_loop(hedge, loop);
    csmhedge_set_next(hedge, hedge);
    csmhedge_set_prev(hedge, hedge);
    
    csmvertex_set_hedge(vertex, hedge);
    
    ASSIGN_OPTIONAL_VALUE(hedge_opc, hedge);
    
    return solid;
}
