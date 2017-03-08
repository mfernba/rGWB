// Edge...

#include "csmedge.inl"
#include "csmedge.tli"

#include "csmnode.inl"
#include "csmhedge.inl"

#include "cyassert.h"
#include "cypeid.h"
#include "cypespy.h"

struct csmedge_t
{
    struct csmnode_t clase_base;
    
    struct csmhedge_t *he1;
    struct csmhedge_t *he2;
};

// --------------------------------------------------------------------------------------------------------------

static void i_csmedge_destruye(struct csmedge_t **edge)
{
    assert_no_null(edge);
    assert_no_null(*edge);

    FREE_PP(edge, struct csmedge_t);
}

// --------------------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmedge_t *, i_crea, (
                        unsigned long id,
                        struct csmhedge_t *he1,
                        struct csmhedge_t *he2))
{
    struct csmedge_t *edge;
    
    edge = MALLOC(struct csmedge_t);
    
    edge->clase_base = csmnode_crea_node(id, edge, i_csmedge_destruye, csmedge_t);
    
    edge->he1 = he1;
    edge->he2 = he2;
    
    return edge;
}

// --------------------------------------------------------------------------------------------------------------

struct csmedge_t *csmedge_crea(unsigned long *id_nuevo_elemento)
{
    unsigned long id;
    struct csmhedge_t *he1, *he2;
    
    id = cypeid_nuevo_id(id_nuevo_elemento, NULL);

    he1 = NULL;
    he2 = NULL;
    
    return i_crea(id, he1, he2);
}

// --------------------------------------------------------------------------------------------------------------

struct csmhedge_t *csmedge_hedge_lado(struct csmedge_t *edge, enum csmedge_lado_hedge_t lado)
{
    assert_no_null(edge);
    
    switch (lado)
    {
        case CSMEDGE_LADO_HEDGE_POS: return edge->he1;
        case CSMEDGE_LADO_HEDGE_NEG: return edge->he2;
        default_error();
    }
}

// --------------------------------------------------------------------------------------------------------------

void csmedge_set_edge_lado(struct csmedge_t *edge, enum csmedge_lado_hedge_t lado, struct csmhedge_t *hedge)
{
    assert_no_null(edge);

    switch (lado)
    {
        case CSMEDGE_LADO_HEDGE_POS:
            
            edge->he1 = hedge;
            break;
            
        case CSMEDGE_LADO_HEDGE_NEG:
            
            edge->he2 = hedge;
            break;
            
        default_error();
    }
}

// ----------------------------------------------------------------------------------------------------

void csmedge_remove_hedge(struct csmedge_t *edge, struct csmhedge_t *hedge)
{
    assert_no_null(edge);
    
    if (edge->he1 == hedge)
    {
        edge->he1 = NULL;
    }
    else
    {
        assert(edge->he2 == hedge);
        edge->he2 = NULL;
    }
}

// ----------------------------------------------------------------------------------------------------

struct csmhedge_t *csmedge_mate(struct csmedge_t *edge, const struct csmhedge_t *hedge)
{
    assert_no_null(edge);
    
    if (csmhedge_id_igual(edge->he1, hedge) == CIERTO)
    {
        return edge->he2;
    }
    else
    {
        assert(csmhedge_id_igual(edge->he2, hedge) == CIERTO);
        return edge->he1;
    }
}

// ----------------------------------------------------------------------------------------------------

struct csmedge_t *csmedge_next(struct csmedge_t *edge)
{
    assert_no_null(edge);
    return csmnode_downcast(csmnode_next(CSMNODE(edge)), csmedge_t);
}

// ----------------------------------------------------------------------------------------------------

struct csmedge_t *csmedge_prev(struct csmedge_t *edge)
{
    assert_no_null(edge);
    return csmnode_downcast(csmnode_prev(CSMNODE(edge)), csmedge_t);
}
