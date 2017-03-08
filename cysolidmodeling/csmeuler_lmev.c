// cmseuler_lmev.
// Operator lmev "splits" the vertex pointed at by he1 and he2, and adds a new edge between the resulting
// two vertices. (x, y, z) are the coordinates of the new vertex.
// he1 == h2 is allowed, in this case, new_vertex and new edge are added into the face of he1.
// The new edge is oriented from the new vertex to the old one.

#include "csmeuler_lmev.inl"

#include "csmedge.tli"
#include "csmface.inl"
#include "csmhedge.inl"
#include "csmloop.inl"
#include "csmnode.inl"
#include "csmopbas.inl"
#include "csmsolid.inl"
#include "csmvertex.inl"

#include "cyassert.h"

// ----------------------------------------------------------------------------------------------------

void csmeuler_lmev(
                struct csmhedge_t *he1, struct csmhedge_t *he2,
                double x, double y, double z,
                unsigned long *id_nuevo_elemento,
                struct csmvertex_t **vertex_opc,
                struct csmedge_t **edge_opc,
                struct csmhedge_t **hedge_from_new_vertex_opc,
                struct csmhedge_t **hedge_to_new_vertex_opc)
{
    struct csmhedge_t *hedge_from_new_vertex_loc, *hedge_to_new_vertex_loc;
    struct csmsolid_t *solido_he1;
    struct csmedge_t *new_edge;
    struct csmvertex_t *new_vertex;
    struct csmhedge_t *he;
    unsigned long num_iteraciones;
    struct csmvertex_t *old_vertex;
    
    solido_he1 = csmopbas_solid_from_hedge(he1);
    csmsolid_append_new_edge(solido_he1, id_nuevo_elemento, &new_edge);
    csmsolid_append_new_vertex(solido_he1, x, y, z, id_nuevo_elemento, &new_vertex);
    
    he = he1;
    num_iteraciones = 0;
    
    while (csmhedge_id_igual(he, he2) == FALSO)
    {
        assert(num_iteraciones < 10000);
        num_iteraciones++;
        
        csmhedge_set_vertex(he, new_vertex);
        he = csmhedge_next(csmopbas_mate(he));
    }

    old_vertex = csmhedge_vertex(he2);
    
    csmopbas_addhe(new_edge, old_vertex, he1, CSMEDGE_LADO_HEDGE_NEG, id_nuevo_elemento, &hedge_to_new_vertex_loc);
    csmopbas_addhe(new_edge, new_vertex, he2, CSMEDGE_LADO_HEDGE_POS, id_nuevo_elemento, &hedge_from_new_vertex_loc);

    csmvertex_set_hedge(new_vertex, hedge_from_new_vertex_loc);
    
    assert(old_vertex == csmhedge_vertex(he2));
    csmvertex_set_hedge(old_vertex, he2);
    
    ASIGNA_OPC(vertex_opc, new_vertex);
    ASIGNA_OPC(edge_opc, new_edge);
    ASIGNA_OPC(hedge_from_new_vertex_opc, hedge_from_new_vertex_loc);
    ASIGNA_OPC(hedge_to_new_vertex_opc, hedge_to_new_vertex_loc);
}

// ----------------------------------------------------------------------------------------------------

void csmeuler_lmev_strut_edge(
                struct csmhedge_t *he,
                double x, double y, double z,
                unsigned long *id_nuevo_elemento,
                struct csmhedge_t **hedge_from_new_vertex_opc)
{
    csmeuler_lmev(he, he, x,  y,  z, id_nuevo_elemento, NULL, NULL, hedge_from_new_vertex_opc, NULL);
}

