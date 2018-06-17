//  Kills and edge and a vertex...
//
// It's the inverse of lmev. It removes the edge pointed at by both he1, he2, and "joins"
// the two vertices which must be distinct (but could be geometrically equal).
// Vertex he1->vtx is removed.
 
#include "csmeuler_lkev.inl"

#include "csmdebug.inl"
#include "csmedge.tli"
#include "csmhedge.inl"
#include "csmopbas.inl"
#include "csmsolid.inl"
#include "csmvertex.inl"

#ifdef __STANDALONE_DISTRIBUTABLE
#include "csmassert.inl"
#include "csmmem.inl"
#else
#include "cyassert.h"
#include "cypespy.h"
#endif

// ----------------------------------------------------------------------------------------------------

void csmeuler_lkev(
                struct csmhedge_t **he1, struct csmhedge_t **he2,
                struct csmhedge_t **he1_prev_opc, struct csmhedge_t **he1_next_opc,
                struct csmhedge_t **he2_prev_opc, struct csmhedge_t **he2_next_opc)
{
    struct csmhedge_t *he1_loc, *he2_loc;
    struct csmhedge_t *he_vertex_to_retain;
    struct csmsolid_t *hes_solid;
    struct csmvertex_t *vertex_to_retain, *vertex_to_delete;
    register struct csmhedge_t *he_iterator;
    register unsigned long no_iterations;
    struct csmedge_t *edge;    
    CSMBOOL delete_vertex;
    
    he1_loc = ASSIGN_POINTER_PP_NOT_NULL(he1, struct csmhedge_t);
    he2_loc = ASSIGN_POINTER_PP_NOT_NULL(he2, struct csmhedge_t);
    assert(csmhedge_equal_id(he1_loc, he2_loc) == CSMFALSE);
    assert(he1_loc != he2_loc);
    
    hes_solid = csmopbas_solid_from_hedges(he1_loc, he2_loc);
    
    edge = csmhedge_edge(he1_loc);
    assert(edge == csmhedge_edge(he2_loc));
    assert(csmopbas_mate(he1_loc) == he2_loc);
    assert(csmopbas_mate(he2_loc) == he1_loc);
    
    vertex_to_delete = csmhedge_vertex(he1_loc);
    vertex_to_retain = csmhedge_vertex(he2_loc);

    if (vertex_to_delete != vertex_to_retain)
    {
        delete_vertex = CSMTRUE;
    }
    else
    {
        //assert(csmhedge_next(csmhedge_next(he1_loc)) == he1_loc);
        //assert(csmhedge_next(csmhedge_next(he2_loc)) == he2_loc);
        
        delete_vertex = CSMFALSE;
    }
    
    he_iterator = csmhedge_next(he2_loc);
    no_iterations = 0;
    he_vertex_to_retain = NULL;
    
    while (he_iterator != he1_loc)
    {
        assert(no_iterations < 10000);
        no_iterations++;
        
        assert(csmhedge_vertex(he_iterator) == vertex_to_delete);
        csmhedge_set_vertex(he_iterator, vertex_to_retain);
        
        if (he_vertex_to_retain == NULL && he_iterator != he2_loc)
            he_vertex_to_retain = he_iterator;
        
        he_iterator = csmhedge_next(csmopbas_mate(he_iterator));
    }
    
    if (csmvertex_hedge(vertex_to_retain) == he2_loc)
    {
        assert(he_vertex_to_retain != he2_loc);
        
        if (he_vertex_to_retain != NULL)
            assert(csmhedge_vertex(he_vertex_to_retain) == vertex_to_retain);
        
        csmvertex_set_hedge(vertex_to_retain, he_vertex_to_retain);
    }
    
    csmopbas_delhe(&he1_loc, he1_prev_opc, he1_next_opc);
    csmopbas_delhe(&he2_loc, he2_prev_opc, he2_next_opc);
    csmsolid_remove_edge(hes_solid, &edge);
    
    if (delete_vertex == CSMTRUE)
        csmsolid_remove_vertex(hes_solid, &vertex_to_delete);
}
