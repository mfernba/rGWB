//  Kills and edge and a vertex...
//
// It's the inverse of lmev. It removes the edge pointed at by both he1, he2, and "joins"
// the two vertices which must be distinct (but could be geometrically equal).
// Vertex he1->vtx is removed.
 
#include "csmeuler_lkev.inl"

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

// ----------------------------------------------------------------------------------------------------

void csmeuler_lkev(struct csmhedge_t **he1, struct csmhedge_t **he2)
{
    struct csmhedge_t *he1_loc, *he2_loc;
    struct csmsolid_t *hes_solid;
    struct csmvertex_t *vertex_to_retain, *vertex_to_delete;
    register struct csmhedge_t *he_iterator;
    register unsigned long num_iteraciones;
    struct csmedge_t *edge;    
    
    he1_loc = ASIGNA_PUNTERO_PP_NO_NULL(he1, struct csmhedge_t);
    he2_loc = ASIGNA_PUNTERO_PP_NO_NULL(he2, struct csmhedge_t);
    assert(csmhedge_id_igual(he1_loc, he2_loc) == FALSO);
    assert(he1_loc != he2_loc);
    
    hes_solid = csmopbas_solid_from_hedges(he1_loc, he2_loc);
    
    edge = csmhedge_edge(he1_loc);
    assert(edge == csmhedge_edge(he2_loc));
    assert(csmopbas_mate(he1_loc) == he2_loc);
    assert(csmopbas_mate(he2_loc) == he1_loc);
    
    vertex_to_delete = csmhedge_vertex(he1_loc);
    vertex_to_retain = csmhedge_vertex(he2_loc);
    assert(vertex_to_delete != vertex_to_retain);
    
    he_iterator = csmhedge_next(he2_loc);
    num_iteraciones = 0;
    
    while (he_iterator != he1_loc)
    {
        assert(num_iteraciones < 10000);
        num_iteraciones++;
        
        assert(csmhedge_vertex(he_iterator) == vertex_to_delete);
        csmhedge_set_vertex(he_iterator, vertex_to_retain);
        
        he_iterator = csmhedge_next(csmopbas_mate(he_iterator));
    }
    
    csmopbas_delhe(&he1_loc, NULL, he1);
    csmopbas_delhe(&he2_loc, he2, NULL);
    csmsolid_remove_edge(hes_solid, &edge);
    csmsolid_remove_vertex(hes_solid, &vertex_to_delete);
}
