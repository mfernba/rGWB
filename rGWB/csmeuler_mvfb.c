//  Creates a new body inside a solid...

#include "csmeuler_mvfb.inl"

#include "csmface.inl"
#include "csmhedge.inl"
#include "csmloop.inl"
#include "csmsolid.inl"
#include "csmvertex.inl"

// ----------------------------------------------------------------------------------------------------

void csmeuler_mvfb(struct csmsolid_t *solid, double x, double y, double z, struct csmhedge_t **hedge_opc)
{
    unsigned long *id_nuevo_elemento;
    struct csmvertex_t *vertex;
    struct csmface_t *face;
    struct csmloop_t *loop;
    struct csmhedge_t *hedge;
    
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
}
