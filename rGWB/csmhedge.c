// Half-Edge...

#include "csmhedge.inl"

#include "csmhashtb.inl"
#include "csmnode.inl"
#include "csmvertex.inl"

#include "csmid.inl"

#ifdef __STANDALONE_DISTRIBUTABLE
#include "csmassert.inl"
#include "csmmem.inl"
#else
#include "cyassert.h"
#include "cypespy.h"
#include "cypespy.inl"
#endif

struct csmhedge_t
{
    struct csmnode_t clase_base;
    
    struct csmedge_t *edge;
    struct csmvertex_t *vertex;
    struct csmloop_t *loop;

    CSMBOOL setop_is_loose_end;
};

// --------------------------------------------------------------------------------------------------------------

static void i_csmhedge_destruye(struct csmhedge_t **hedge)
{
    assert_no_null(hedge);
    assert_no_null(*hedge);
    
    FREE_PP_NO_CLEAN_MEMORY(hedge, struct csmhedge_t);
}

// --------------------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmhedge_t *, i_crea, (
                        unsigned long id,
                        struct csmedge_t *edge,
                        struct csmvertex_t *vertex,
                        struct csmloop_t *loop,
                        CSMBOOL setop_is_loose_end))
{
    struct csmhedge_t *hedge;
    
    hedge = MALLOC(struct csmhedge_t);
    
    hedge->clase_base = csmnode_crea_node(id, hedge, i_csmhedge_destruye, csmhedge_t);
    
    hedge->edge = edge;
    hedge->vertex = vertex;
    hedge->loop = loop;
    
    hedge->setop_is_loose_end = setop_is_loose_end;
    
    return hedge;
}

// --------------------------------------------------------------------------------------------------------------

struct csmhedge_t *csmhedge_crea(unsigned long *id_nuevo_elemento)
{
    unsigned long id;
    struct csmedge_t *edge;
    struct csmvertex_t *vertex;
    struct csmloop_t *loop;
    CSMBOOL setop_is_loose_end;
    
    id = csmid_new_id(id_nuevo_elemento, NULL);

    edge = NULL;
    vertex = NULL;
    loop = NULL;
    
    setop_is_loose_end = CSMFALSE;
    
    return i_crea(id, edge, vertex, loop, setop_is_loose_end);
}

// --------------------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmhedge_t *, i_duplicate_hedge, (struct csmloop_t *loop, unsigned long *id_nuevo_elemento))
{
    unsigned long id;
    struct csmedge_t *edge;
    struct csmvertex_t *vertex;
    CSMBOOL setop_is_loose_end;
    
    id = csmid_new_id(id_nuevo_elemento, NULL);

    edge = NULL;
    vertex = NULL;
    setop_is_loose_end = CSMFALSE;
    
    return i_crea(id, edge, vertex, loop, setop_is_loose_end);
}

// --------------------------------------------------------------------------------------------------------------

struct csmhedge_t *csmhedge_duplicate(
                        const struct csmhedge_t *hedge,
                        struct csmloop_t *loop,
                        unsigned long *id_nuevo_elemento,
                        struct csmhashtb(csmvertex_t) *relation_svertexs_old_to_new,
                        struct csmhashtb(csmhedge_t) *relation_shedges_old_to_new)
{
    struct csmhedge_t *new_hedge;
    unsigned long id_old_vertex;
    
    assert_no_null(hedge);
    
    new_hedge = i_duplicate_hedge(loop, id_nuevo_elemento);
    assert_no_null(new_hedge);
    assert(new_hedge->edge == NULL);
    assert(new_hedge->vertex == NULL);
    
    id_old_vertex = csmvertex_id(hedge->vertex);
    new_hedge->vertex = csmhashtb_ptr_for_id(relation_svertexs_old_to_new, id_old_vertex, csmvertex_t);
    new_hedge->setop_is_loose_end = hedge->setop_is_loose_end;
    
    if (csmvertex_hedge(hedge->vertex) == hedge)
        csmvertex_set_hedge(new_hedge->vertex, new_hedge);
    
    csmhashtb_add_item(relation_shedges_old_to_new, hedge->clase_base.id, new_hedge, csmhedge_t);
    
    return new_hedge;
}

// --------------------------------------------------------------------------------------------------------------

unsigned long csmhedge_id(const struct csmhedge_t *hedge)
{
    assert_no_null(hedge);
    return hedge->clase_base.id;
}

// --------------------------------------------------------------------------------------------------------------

void csmhedge_reassign_id(struct csmhedge_t *hedge, unsigned long *id_nuevo_elemento, unsigned long *new_id_opc)
{
    assert_no_null(hedge);
    hedge->clase_base.id = csmid_new_id(id_nuevo_elemento, new_id_opc);
}

// --------------------------------------------------------------------------------------------------------------

CSMBOOL csmhedge_id_igual(const struct csmhedge_t *hedge1, const struct csmhedge_t *hedge2)
{
    assert_no_null(hedge1);
    assert_no_null(hedge2);
    
    return IS_TRUE(hedge1->clase_base.id == hedge2->clase_base.id);
}

// --------------------------------------------------------------------------------------------------------------

struct csmedge_t *csmhedge_edge(struct csmhedge_t *hedge)
{
    assert_no_null(hedge);
    return hedge->edge;
}

// --------------------------------------------------------------------------------------------------------------

void csmhedge_set_edge(struct csmhedge_t *hedge, struct csmedge_t *edge)
{
    assert_no_null(hedge);
    hedge->edge = edge;
}

// --------------------------------------------------------------------------------------------------------------

struct csmvertex_t *csmhedge_vertex(struct csmhedge_t *hedge)
{
    assert_no_null(hedge);
    return hedge->vertex;
}

// --------------------------------------------------------------------------------------------------------------

const struct csmvertex_t *csmhedge_vertex_const(const struct csmhedge_t *hedge)
{
    assert_no_null(hedge);
    return hedge->vertex;
}

// --------------------------------------------------------------------------------------------------------------

void csmhedge_set_vertex(struct csmhedge_t *hedge, struct csmvertex_t *vertex)
{
    assert_no_null(hedge);
    hedge->vertex = vertex;
}

// --------------------------------------------------------------------------------------------------------------

struct csmloop_t *csmhedge_loop(struct csmhedge_t *hedge)
{
    assert_no_null(hedge);
    return hedge->loop;
}

// --------------------------------------------------------------------------------------------------------------

void csmhedge_set_loop(struct csmhedge_t *hedge, struct csmloop_t *loop)
{
    assert_no_null(hedge);
    hedge->loop = loop;
}

// ----------------------------------------------------------------------------------------------------

void csmhedge_clear_algorithm_mask(struct csmhedge_t *hedge)
{
    assert_no_null(hedge);
    
    hedge->setop_is_loose_end = CSMFALSE;
}

// ----------------------------------------------------------------------------------------------------

void csmhedge_setop_set_loose_end(struct csmhedge_t *hedge, CSMBOOL is_loose_end)
{
    assert_no_null(hedge);
    hedge->setop_is_loose_end = is_loose_end;
}

// ----------------------------------------------------------------------------------------------------

CSMBOOL csmhedge_setop_is_loose_end(const struct csmhedge_t *hedge)
{
    assert_no_null(hedge);
    return hedge->setop_is_loose_end;
}

// ----------------------------------------------------------------------------------------------------

struct csmhedge_t *csmhedge_next(struct csmhedge_t *hedge)
{
    assert_no_null(hedge);
    return csmnode_downcast(csmnode_next(CSMNODE(hedge)), csmhedge_t);
}

// ----------------------------------------------------------------------------------------------------

void csmhedge_set_next(struct csmhedge_t *hedge, struct csmhedge_t *next_hedge)
{
    assert_no_null(hedge);
    csmnode_set_ptr_next(CSMNODE(hedge), CSMNODE(next_hedge));
}

// ----------------------------------------------------------------------------------------------------

struct csmhedge_t *csmhedge_prev(struct csmhedge_t *hedge)
{
    assert_no_null(hedge);
    return csmnode_downcast(csmnode_prev(CSMNODE(hedge)), csmhedge_t);
}

// ----------------------------------------------------------------------------------------------------

void csmhedge_set_prev(struct csmhedge_t *hedge, struct csmhedge_t *prev_hedge)
{
    assert_no_null(hedge);
    csmnode_set_ptr_prev(CSMNODE(hedge), CSMNODE(prev_hedge));
}







