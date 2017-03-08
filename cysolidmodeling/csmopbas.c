// Operaciones básicas sobre elementos con dependencias entre sí...

#include "csmopbas.inl"

#include "csmedge.inl"
#include "csmedge.tli"
#include "csmface.inl"
#include "csmhedge.inl"
#include "csmloop.inl"
#include "csmnode.inl"
#include "csmvertex.inl"

#include "cyassert.h"
#include "cypespy.h"

// ------------------------------------------------------------------------------------------

struct csmhedge_t *csmopbas_mate(struct csmhedge_t *hedge)
{
    struct csmedge_t *edge;
    
    edge = csmhedge_edge(hedge);
    return csmedge_mate(edge, hedge);
}

// ------------------------------------------------------------------------------------------

struct csmface_t *csmopbas_face_from_hedge(struct csmhedge_t *hedge)
{
    struct csmloop_t *loop;
    
    loop = csmhedge_loop(hedge);
    return csmloop_lface(loop);
}

// ------------------------------------------------------------------------------------------

struct csmsolid_t *csmopbas_solid_from_hedge(struct csmhedge_t *hedge)
{
    struct csmface_t *lface;
    
    lface = csmopbas_face_from_hedge(hedge);
    return csmface_fsolid(lface);
}

// ------------------------------------------------------------------------------------------

void csmopbas_addhe(
                struct csmedge_t *edge,
                struct csmvertex_t *vertex,
                struct csmhedge_t *hedge,
                enum csmedge_lado_hedge_t lado,
                unsigned long *id_nuevo_elemento,
                struct csmhedge_t **new_hedge_opc)
{
    struct csmhedge_t *new_hedge_loc;
    struct csmloop_t *loop;
    
    assert_no_null(edge);
    assert_no_null(vertex);
    
    if (csmhedge_edge(hedge) == NULL)
    {
        new_hedge_loc = hedge;
    }
    else
    {
        new_hedge_loc = csmhedge_crea(id_nuevo_elemento);
        csmnode_insert_node2_before_node1(new_hedge_loc, hedge, csmhedge_t);
    }
    
    csmedge_set_edge_lado(edge, lado, new_hedge_loc);
    csmhedge_set_edge(new_hedge_loc, edge);
    
    loop = csmhedge_loop(hedge);
    csmhedge_set_loop(new_hedge_loc, loop);
    
    csmhedge_set_vertex(new_hedge_loc, vertex);
    
    ASIGNA_OPC(new_hedge_opc, new_hedge_loc);
}

// ------------------------------------------------------------------------------------------

void csmopbas_delhe(struct csmhedge_t **hedge, struct csmhedge_t **hedge_prev_opc)
{
    struct csmhedge_t *hedge_prev;
    struct csmhedge_t *hedge_loc;
    struct csmedge_t *edge;
    struct csmloop_t *loop;
    struct csmvertex_t *vertex;
    CYBOOL destroy_hedge;
    struct csmhedge_t *hedge_for_vertex;
    
    hedge_loc = ASIGNA_PUNTERO_PP_NO_NULL(hedge, struct csmhedge_t);
    edge = csmhedge_edge(hedge_loc);
    loop = csmhedge_loop(hedge_loc);
    vertex = csmhedge_vertex(hedge_loc);
    
    if (edge == NULL)
    {
        destroy_hedge = CIERTO;
        
        hedge_prev = NULL;
        hedge_for_vertex = NULL;
    }
    else
    {
        if (csmhedge_next(hedge_loc) == hedge_loc)
        {
            destroy_hedge = FALSO;
            
            hedge_prev = hedge_loc;
            hedge_for_vertex = NULL;
        }
        else
        {
            destroy_hedge = CIERTO;
            
            hedge_prev = csmhedge_prev(hedge_loc);
            hedge_for_vertex = csmhedge_next(csmopbas_mate(hedge_loc));
        }
        
        csmhedge_set_edge(hedge_loc, NULL);
        csmedge_remove_hedge(edge, hedge_loc);
    }
    
    if (destroy_hedge == CIERTO)
    {
        csmloop_set_ledge(loop, hedge_prev);
        csmvertex_set_hedge(vertex, hedge_for_vertex);
        
        csmnode_free_node_in_list(&hedge_loc, csmhedge_t);
    }
    
    ASIGNA_OPC(hedge_prev_opc, hedge_prev);
}
