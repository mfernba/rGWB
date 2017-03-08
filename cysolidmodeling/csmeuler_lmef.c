// Low level make edge-face...
//
// Adds a new edge between he1 and he2 and "splits" their common face into two faces
// such that he1 will occur in the new face and he2 remains in the old one.
//
// The new edge is oriented from he1->vtx to he2->vtx.
//
// he1 and he2 belong to the same loop, and may be equal, in which case a "circular" face
// with just one edge is created. A pointer to the new face is returned.
//
// lmef doesn't classify the loops of the divided face into faces resulting from the division,
// all other loops of the old face remain in it.
//

#include "csmeuler_lmef.inl"

#include "csmedge.tli"
#include "csmface.inl"
#include "csmhedge.inl"
#include "csmloop.inl"
#include "csmnode.inl"
#include "csmopbas.inl"
#include "csmsolid.inl"
#include "csmvertex.inl"

#include "cyassert.h"
#include "cypespy.h"

// --------------------------------------------------------------------------------

struct csmface_t *csmeuler_lmef(
                struct csmhedge_t *he1, struct csmhedge_t *he2,
                unsigned long *id_nuevo_elemento)
{
    struct csmsolid_t *he1_solid;
    struct csmloop_t *loop_he1, *loop_he2;
    struct csmface_t *new_face;
    struct csmedge_t *new_edge;
    struct csmloop_t *new_loop;
    struct csmhedge_t *he_iterator;
    struct csmhedge_t *new_he1, *new_he2;
    struct csmhedge_t *prev_new_he1, *prev_new_he2;
    
    loop_he1 = csmhedge_loop(he1);
    loop_he2 = csmhedge_loop(he2);
    assert(csmloop_ids_iguales(loop_he1, loop_he2) == CIERTO);
    assert(loop_he1 == loop_he2);

    he1_solid = csmopbas_solid_from_hedge(he1);
    csmsolid_append_new_face(he1_solid, id_nuevo_elemento, &new_face);
    csmsolid_append_new_edge(he1_solid, id_nuevo_elemento, &new_edge);
    
    new_loop = csmloop_crea(new_face, id_nuevo_elemento);
    csmface_set_flout(new_face, new_loop);
    
    he_iterator = he1;
    while (he_iterator != he2)
    {
        csmhedge_set_loop(he_iterator, new_loop);
        he_iterator = csmhedge_next(he_iterator);
    }
    
    csmopbas_addhe(new_edge, csmhedge_vertex(he2), he1, CSMEDGE_LADO_HEDGE_NEG, id_nuevo_elemento, &new_he1);
    csmopbas_addhe(new_edge, csmhedge_vertex(he1), he2, CSMEDGE_LADO_HEDGE_POS, id_nuevo_elemento, &new_he2);

    prev_new_he1 = csmhedge_prev(new_he1);
    csmhedge_set_next(prev_new_he1, new_he2);
                      
    prev_new_he2 = csmhedge_prev(new_he2);
    csmhedge_set_next(prev_new_he2, new_he1);
    
    csmhedge_set_prev(new_he1, prev_new_he2);
    csmhedge_set_prev(new_he2, prev_new_he1);
    
    csmloop_set_ledge(new_loop, new_he1);
    csmloop_set_ledge(csmhedge_loop(he2), new_he2);

    return new_face;
}










