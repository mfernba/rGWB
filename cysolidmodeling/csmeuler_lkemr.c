// low level kill edge make ring
//
// Removes the edge of he1 and he2, and divides their common loop into two components (it creates a new ring).
// If the original loop was outer, the component he1->vtx becomes the new outer loop.
//
//

#include "csmeuler_lkemr.inl"

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

void csmeuler_lkemr(
                struct csmhedge_t **he_to_ring, struct csmhedge_t **he_from_ring,
                struct csmhedge_t **he_to_ring_next, struct csmhedge_t **he_from_ring_next)
{
    struct csmhedge_t *he1_loc, *he2_loc;
    struct csmedge_t *edge_to_remove;
    struct csmloop_t *loop_to_divide;
    struct csmhedge_t *he1_old_next, *he2_old_next;
    struct csmface_t *he1_and_he2_face;
    unsigned long *id_nuevo_elemento;
    struct csmloop_t *new_loop;
    struct csmhedge_t *he_iterator;
    unsigned long num_iteraciones;
    struct csmhedge_t *he1_prev, *he2_prev;
    struct csmvertex_t *vertex1, *vertex2;
    struct csmsolid_t *he1_and_he2_solid;
    
    he1_loc = ASIGNA_PUNTERO_PP_NO_NULL(he_to_ring, struct csmhedge_t);
    he2_loc = ASIGNA_PUNTERO_PP_NO_NULL(he_from_ring, struct csmhedge_t);
    assert(he1_loc != he2_loc);
    
    edge_to_remove = csmhedge_edge(he1_loc);
    assert(edge_to_remove == csmhedge_edge(he2_loc));
    
    loop_to_divide = csmhedge_loop(he1_loc);
    assert(loop_to_divide == csmhedge_loop(he2_loc));
    
    he1_old_next = csmhedge_next(he1_loc);
    he2_old_next = csmhedge_next(he2_loc);
    
    vertex1 = csmhedge_vertex(he1_loc);
    vertex2 = csmhedge_vertex(he2_loc);
    
    csmhedge_set_next(he1_loc, he2_old_next);
    csmhedge_set_prev(he2_old_next, he1_loc);

    csmhedge_set_next(he2_loc, he1_old_next);
    csmhedge_set_prev(he1_old_next, he2_loc);

    he1_and_he2_face = csmloop_lface(loop_to_divide);
    id_nuevo_elemento = csmsolid_id_new_element(csmface_fsolid(he1_and_he2_face));
    new_loop = csmloop_crea(he1_and_he2_face, id_nuevo_elemento);
    csmface_add_loop_while_removing_from_old(he1_and_he2_face, new_loop);
 
    he_iterator = csmhedge_next(he2_loc);
    num_iteraciones = 0;
    
    do
    {
        assert(num_iteraciones < 10000);
        num_iteraciones++;
        
        csmhedge_set_loop(he_iterator, new_loop);
        he_iterator = csmhedge_next(he_iterator);
        
    } while (he_iterator != he2_loc);

    csmopbas_delhe(&he1_loc, &he1_prev, he_to_ring_next);
    csmloop_set_ledge(loop_to_divide, he1_prev);
    
    csmopbas_delhe(&he2_loc, &he2_prev, he_from_ring_next);
    csmloop_set_ledge(new_loop, he2_prev);
    
    csmvertex_set_hedge(vertex1, he2_old_next);
    csmvertex_set_hedge(vertex2, he1_old_next);

    he1_and_he2_solid = csmface_fsolid(he1_and_he2_face);
    csmsolid_remove_edge(he1_and_he2_solid, &edge_to_remove);
}









