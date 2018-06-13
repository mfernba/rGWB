// Edge...

#include "csmedge.inl"
#include "csmedge.tli"

#include "csmhashtb.inl"
#include "csmhedge.inl"
#include "csmid.inl"
#include "csmvertex.inl"

#ifdef __STANDALONE_DISTRIBUTABLE
#include "csmassert.inl"
#include "csmmem.inl"
#else
#include "cyassert.h"
#include "cypespy.h"
#include "cypespy.inl"
#endif

// --------------------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmedge_t *, i_crea, (
                        unsigned long id,
                        struct csmhedge_t *he1,
                        struct csmhedge_t *he2,
                        CSMBOOL setop_is_null_edge))
{
    struct csmedge_t *edge;
    
    edge = MALLOC(struct csmedge_t);
    
    edge->id = id;
    
    edge->he1 = he1;
    edge->he2 = he2;
    
    edge->setop_is_null_edge = setop_is_null_edge;
    
    return edge;
}

// --------------------------------------------------------------------------------------------------------------

struct csmedge_t *csmedge_crea(unsigned long *id_nuevo_elemento)
{
    unsigned long id;
    struct csmhedge_t *he1, *he2;
    CSMBOOL setop_is_null_edge;
    
    id = csmid_new_id(id_nuevo_elemento, NULL);

    he1 = NULL;
    he2 = NULL;
    
    setop_is_null_edge = CSMFALSE;
    
    return i_crea(id, he1, he2, setop_is_null_edge);
}

// --------------------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmedge_t *, i_duplicate_edge, (unsigned long *id_nuevo_elemento))
{
    unsigned long id;
    struct csmhedge_t *he1, *he2;
    CSMBOOL setop_is_null_edge;
    
    id = csmid_new_id(id_nuevo_elemento, NULL);

    he1 = NULL;
    he2 = NULL;
    
    setop_is_null_edge = CSMFALSE;
    
    return i_crea(id, he1, he2, setop_is_null_edge);
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
    
    edge->id = csmid_new_id(id_nuevo_elemento, new_id_opc);
    
    if (edge->he1 != NULL)
        csmhedge_reassign_id(edge->he1, id_nuevo_elemento, NULL);
    
    if (edge->he2 != NULL)
        csmhedge_reassign_id(edge->he2, id_nuevo_elemento, NULL);
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

enum csmedge_lado_hedge_t csmedge_hedge_side(const struct csmedge_t *edge, const struct csmhedge_t *hedge)
{
    assert_no_null(edge);
    
    if (edge->he1 == hedge)
    {
        return CSMEDGE_LADO_HEDGE_POS;
    }
    else
    {
        assert(edge->he2 == hedge);
        return CSMEDGE_LADO_HEDGE_NEG;
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
    
    if (csmhedge_id_igual(edge->he1, hedge) == CSMTRUE)
    {
        return edge->he2;
    }
    else
    {
        assert(csmhedge_id_igual(edge->he2, hedge) == CSMTRUE);
        return edge->he1;
    }
}

// ----------------------------------------------------------------------------------------------------

void csmedge_reverse_orientation(struct csmedge_t *edge)
{
    struct csmhedge_t *he_aux;
    
    assert_no_null(edge);
    
    he_aux = edge->he1;
    edge->he1 = edge->he2;
    edge->he2 = he_aux;
}

// ----------------------------------------------------------------------------------------------------

void csmedge_clear_algorithm_mask(struct csmedge_t *edge)
{
    assert_no_null(edge);
    
    edge->setop_is_null_edge = CSMFALSE;
    
    csmhedge_clear_algorithm_mask(edge->he1);
    csmhedge_clear_algorithm_mask(edge->he2);
}

// ----------------------------------------------------------------------------------------------------

void csmedge_setop_set_is_null_edge(struct csmedge_t *edge, CSMBOOL is_null_edge)
{
    assert_no_null(edge);
    
    edge->setop_is_null_edge = is_null_edge;
    csmhedge_setop_set_loose_end(edge->he1, edge->setop_is_null_edge);
    csmhedge_setop_set_loose_end(edge->he2, edge->setop_is_null_edge);
}

// ----------------------------------------------------------------------------------------------------

CSMBOOL csmedge_setop_is_null_edge(struct csmedge_t *edge)
{
    assert_no_null(edge);
    return edge->setop_is_null_edge;
}

// ----------------------------------------------------------------------------------------------------

void csmedge_vertex_coordinates(
                        const struct csmedge_t *edge,
                        double *x1, double *y1, double *z1, unsigned long *id_vertex1_opt,
                        double *x2, double *y2, double *z2, unsigned long *id_vertex2_opt)
{
    const struct csmvertex_t *v1, *v2;
    
    assert_no_null(edge);
    
    v1 = csmhedge_vertex_const(edge->he1);
    csmvertex_get_coordenadas(v1, x1, y1, z1);
    ASSIGN_OPTIONAL_VALUE(id_vertex1_opt, csmvertex_id(v1));
    
    v2 = csmhedge_vertex_const(edge->he2);
    csmvertex_get_coordenadas(v2, x2, y2, z2);
    ASSIGN_OPTIONAL_VALUE(id_vertex2_opt, csmvertex_id(v2));
}
