// Edge...

#include "csmedge.inl"
#include "csmedge.tli"

#include "csmhashtb.inl"
#include "csmhedge.inl"

#include "cyassert.h"
#include "cypeid.h"
#include "cypespy.h"

struct csmedge_t
{
    unsigned long id;
    
    struct csmhedge_t *he1;
    struct csmhedge_t *he2;
};

// --------------------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmedge_t *, i_crea, (
                        unsigned long id,
                        struct csmhedge_t *he1,
                        struct csmhedge_t *he2))
{
    struct csmedge_t *edge;
    
    edge = MALLOC(struct csmedge_t);
    
    edge->id = id;
    
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

CONSTRUCTOR(struct csmedge_t *, i_duplicate_edge, (unsigned long *id_nuevo_elemento))
{
    unsigned long id;
    struct csmhedge_t *he1, *he2;
    
    id = cypeid_nuevo_id(id_nuevo_elemento, NULL);

    he1 = NULL;
    he2 = NULL;
    
    return i_crea(id, he1, he2);
}

// --------------------------------------------------------------------------------------------------------------

struct csmedge_t *csmedge_duplicate(
                        const struct csmedge_t *edge,
                        unsigned long *id_nuevo_elemento,
                        struct csmhashtb(csmhedge_t) *relation_shedges_old_to_new)
{
    struct csmedge_t *new_edge;
    
    new_edge = i_duplicate_edge(id_nuevo_elemento);
    assert_no_null(new_edge);
    assert(new_edge->he1 == NULL);
    assert(new_edge->he2 == NULL);
    
    new_edge->he1 = csmhashtb_ptr_for_id(relation_shedges_old_to_new, csmhedge_id(edge->he1), csmhedge_t);
    csmhedge_set_edge(new_edge->he1, new_edge);
    
    new_edge->he2 = csmhashtb_ptr_for_id(relation_shedges_old_to_new, csmhedge_id(edge->he2), csmhedge_t);
    csmhedge_set_edge(new_edge->he2, new_edge);
    
    return new_edge;
}

// --------------------------------------------------------------------------------------------------------------

void csmedge_destruye(struct csmedge_t **edge)
{
    assert_no_null(edge);
    assert_no_null(*edge);

    FREE_PP(edge, struct csmedge_t);
}

// --------------------------------------------------------------------------------------------------------------

unsigned long csmedge_id(const struct csmedge_t *edge)
{
    assert_no_null(edge);
    return edge->id;
}

// ----------------------------------------------------------------------------------------------------

void csmedge_reassign_id(struct csmedge_t *edge, unsigned long *id_nuevo_elemento, unsigned long *new_id_opc)
{
    assert_no_null(edge);
    edge->id = cypeid_nuevo_id(id_nuevo_elemento, new_id_opc);
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

const struct csmhedge_t *csmedge_hedge_lado_const(const struct csmedge_t *edge, enum csmedge_lado_hedge_t lado)
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
