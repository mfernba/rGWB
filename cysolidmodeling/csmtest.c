// Test básicos...

#include "csmtest.h"

#include "csmedge.inl"
#include "csmedge.tli"
#include "csmsolid.inl"
#include "csmeuler_mvfs.inl"
#include "csmeuler_kvfs.inl"
#include "csmeuler_lmef.inl"
#include "csmeuler_lmev.inl"
#include "csmeuler_lkef.inl"
#include "csmeuler_lkev.inl"

#include "csmnode.inl"
#include "csmopbas.inl"

#include "cyassert.h"

// ------------------------------------------------------------------------------------------

static void i_test_crea_destruye_solido_vacio(void)
{
    struct csmsolid_t *solido;
    unsigned long id_nuevo_elemento;
    
    id_nuevo_elemento = 0;
    
    solido = csmeuler_mvfs(0., 0., 0., &id_nuevo_elemento, NULL);
    csmeuler_kvfs(solido);
    
    csmsolid_destruye(&solido);
}

// ------------------------------------------------------------------------------------------

static void i_test_basico_solido_una_arista(void)
{
    struct csmsolid_t *solido;
    unsigned long id_nuevo_elemento;
    struct csmhedge_t *hedge;
    struct csmhedge_t *he1, *he2;
    
    id_nuevo_elemento = 0;
    
    solido = csmeuler_mvfs(0., 0., 0., &id_nuevo_elemento, &hedge);
    
    csmeuler_lmev(hedge, hedge, 1., 0., 0., &id_nuevo_elemento, NULL, NULL, &he1, &he2);
    
    csmeuler_lkev(&he1, &he2);
    assert(he1 == he2);

    csmeuler_kvfs(solido);
    
    csmsolid_destruye(&solido);
}

// ------------------------------------------------------------------------------------------

static void i_test_crea_lamina(void)
{
    struct csmsolid_t *solido;
    unsigned long id_nuevo_elemento;
    struct csmhedge_t *initial_hedge, *hedge_from_vertex1, *hedge_from_vertex2, *hedge_from_vertex3;
    struct csmhedge_t *he_pos, *he_neg;
    struct csmhedge_t *he1, *he2;
    
    id_nuevo_elemento = 0;
    
    solido = csmeuler_mvfs(0., 0., 0., &id_nuevo_elemento, &initial_hedge);
    
    csmeuler_lmev_strut_edge(initial_hedge, 1., 0., 0., &id_nuevo_elemento, &hedge_from_vertex1);
    csmeuler_lmev_strut_edge(hedge_from_vertex1, 1., 1., 0., &id_nuevo_elemento, &hedge_from_vertex2);
    csmeuler_lmev_strut_edge(hedge_from_vertex2, 0., 1., 0., &id_nuevo_elemento, &hedge_from_vertex3);
    csmeuler_lmef(initial_hedge, hedge_from_vertex3, &id_nuevo_elemento, NULL, &he_pos, &he_neg);
    
    csmeuler_lkef(&he_pos, &he_neg);
    assert(he_pos != NULL);
    assert(he_neg != NULL);
    assert(he_pos == hedge_from_vertex3);
    
    he1 = he_pos;
    he2 = csmopbas_mate(he1);
    csmeuler_lkev(&he1, &he2);
    assert(he1 == hedge_from_vertex2);

    he2 = csmopbas_mate(he1);
    csmeuler_lkev(&he1, &he2);
    assert(he1 == hedge_from_vertex1);

    he2 = csmopbas_mate(he1);
    csmeuler_lkev(&he1, &he2);
    assert(he1 == he2);
    assert(he1 == initial_hedge);
    
    csmeuler_kvfs(solido);
    
    csmsolid_destruye(&solido);
}

// ------------------------------------------------------------------------------------------

void csmtest_test(void)
{
    i_test_crea_destruye_solido_vacio();
    i_test_basico_solido_una_arista();
    i_test_crea_lamina();
}





