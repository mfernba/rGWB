// Test básicos...

#include "csmtest.h"

#include "csmedge.inl"
#include "csmedge.tli"
#include "csmsolid.inl"
#include "csmeuler_mvfs.inl"
#include "csmeuler_lmev.inl"
#include "csmeuler_lkev.inl"
#include "csmnode.inl"

// ------------------------------------------------------------------------------------------

static void i_test_crea_destruye_solido_vacio(void)
{
    struct csmsolid_t *solido;
    unsigned long id_nuevo_elemento;
    
    id_nuevo_elemento = 0;
    
    solido = csmeuler_mvfs(0., 0., 0., &id_nuevo_elemento, NULL);
    
    csmnode_release_ex(&solido, csmsolid_t);
}

// ------------------------------------------------------------------------------------------

static void i_test_basico_solido_una_arista(void)
{
    struct csmsolid_t *solido;
    unsigned long id_nuevo_elemento;
    struct csmhedge_t *hedge;
    struct csmedge_t *edge;
    struct csmhedge_t *he1, *he2;
    
    id_nuevo_elemento = 0;
    
    solido = csmeuler_mvfs(0., 0., 0., &id_nuevo_elemento, &hedge);
    
    csmeuler_lmev(hedge, hedge, 1., 0., 0., &id_nuevo_elemento, &edge, NULL);
    
    he1 = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_POS);
    he2 = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_NEG);
    csmeuler_lkev(&he1, &he2);
    edge = NULL;
    
    csmnode_release_ex(&solido, csmsolid_t);
}

// ------------------------------------------------------------------------------------------

void csmtest_test(void)
{
    i_test_crea_destruye_solido_vacio();
    i_test_basico_solido_una_arista();
}
