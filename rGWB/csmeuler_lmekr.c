// low level make edge kill ring...
//
// Inverse of lkemr.
// It inserts a new edge between the vertices of he1 and he2, and merges the corresponding
// loops into one loop (removes a ring).
//

#include "csmeuler_lmekr.inl"

#include "csmedge.tli"
#include "csmface.inl"
#include "csmhedge.inl"
#include "csmloop.inl"
#include "csmopbas.inl"
#include "csmsolid.inl"

#ifdef __STANDALONE_DISTRIBUTABLE
#include "csmassert.inl"
#else
#include "cyassert.h"
#endif

// --------------------------------------------------------------------------------

void csmeuler_lmekr(
                struct csmhedge_t *he1, struct csmhedge_t *he2,
                struct csmhedge_t **hedge_lado_neg_opc, struct csmhedge_t **hedge_lado_pos_opc)
{
    struct csmloop_t *he1_loop, *he2_loop;
    struct csmface_t *he1_and_he2_face;
    struct csmsolid_t *he1_and_he2_solid;
    unsigned long *id_new_element;
    register struct csmhedge_t *he_iterator;
    register unsigned long no_iterations;
    struct csmedge_t *new_edge;
    struct csmhedge_t *nhe1, *nhe2;
    
    he1_loop = csmhedge_loop(he1);
    he2_loop = csmhedge_loop(he2);
    assert(he1_loop != he2_loop);
    
    he1_and_he2_face = csmloop_lface(he1_loop);
    assert(he1_and_he2_face == csmloop_lface(he2_loop));
    assert(csmhedge_edge(he1) != csmhedge_edge(he2));
    
    he_iterator = he2;
    no_iterations = 0;
    
    do
    {
        assert(no_iterations < 10000);
        no_iterations++;
        
        csmhedge_set_loop(he_iterator, he1_loop);
        he_iterator = csmhedge_next(he_iterator);
        
    } while (he_iterator != he2);
    
    he1_and_he2_solid = csmface_fsolid(he1_and_he2_face);
    csmsolid_append_new_edge(he1_and_he2_solid, &new_edge);

    id_new_element = csmsolid_id_new_element(he1_and_he2_solid);
    csmopbas_addhe(new_edge, csmhedge_vertex(he1), he1, CSMEDGE_LADO_HEDGE_POS, id_new_element, &nhe1);
    csmopbas_addhe(new_edge, csmhedge_vertex(he2), he2, CSMEDGE_LADO_HEDGE_NEG, id_new_element, &nhe2);

    csmhedge_set_next(nhe1, he2);
    csmhedge_set_next(nhe2, he1);
    
    csmhedge_set_prev(he2, nhe1);
    csmhedge_set_prev(he1, nhe2);
    
    if (csmface_flout(he1_and_he2_face) == he2_loop)
        csmface_set_flout(he1_and_he2_face, he1_loop);
        
    csmloop_set_ledge(he2_loop, NULL);
    csmface_remove_loop(he1_and_he2_face, &he2_loop);
    
    ASSIGN_OPTIONAL_VALUE(hedge_lado_pos_opc, nhe1);
    ASSIGN_OPTIONAL_VALUE(hedge_lado_neg_opc, nhe2);
}

