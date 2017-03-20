// Loop...

#include "csmloop.inl"

#include "csmnode.inl"
#include "csmhedge.inl"

#include "cyassert.h"
#include "cypeid.h"
#include "cypespy.h"

struct csmloop_t
{
    struct csmnode_t clase_base;
    
    struct csmhedge_t *ledge;
    struct csmface_t *lface;
};

// --------------------------------------------------------------------------------------------------------------

static void i_csmloop_destruye(struct csmloop_t **loop)
{
    assert_no_null(loop);
    assert_no_null(*loop);

    if ((*loop)->ledge != NULL)
        csmnode_free_node_list(&(*loop)->ledge, csmhedge_t);
    
    FREE_PP(loop, struct csmloop_t);
}

// --------------------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmloop_t *, i_crea, (
                        unsigned long id,
                        struct csmhedge_t *ledge,
                        struct csmface_t *lface))
{
    struct csmloop_t *loop;
    
    loop = MALLOC(struct csmloop_t);
    
    loop->clase_base = csmnode_crea_node(id, loop, i_csmloop_destruye, csmloop_t);
    
    loop->ledge = ledge;
    loop->lface = lface;
    
    return loop;
}

// --------------------------------------------------------------------------------------------------------------

struct csmloop_t *csmloop_crea(struct csmface_t *face, unsigned long *id_nuevo_elemento)
{
    unsigned long id;
    struct csmhedge_t *ledge;
    struct csmface_t *lface;
    
    id = cypeid_nuevo_id(id_nuevo_elemento, NULL);
    
    ledge = NULL;
    lface = face;
    
    return i_crea(id, ledge, lface);
}

// --------------------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmloop_t *, i_duplicate_loop, (struct csmface_t *lface, unsigned long *id_nuevo_elemento))
{
    unsigned long id;
    struct csmhedge_t *ledge;
    
    id = cypeid_nuevo_id(id_nuevo_elemento, NULL);
    
    ledge = NULL;
    
    return i_crea(id, ledge, lface);
}

// --------------------------------------------------------------------------------------------------------------

struct csmloop_t *csmloop_duplicate(
                        const struct csmloop_t *loop,
                        struct csmface_t *lface,
                        unsigned long *id_nuevo_elemento,
                        struct csmhashtb(csmvertex_t) *relation_svertexs_old_to_new,
                        struct csmhashtb(csmhedge_t) *relation_shedges_old_to_new)
{
    struct csmloop_t *new_loop;
    register struct csmhedge_t *iterator, *last_hedge;
    unsigned long num_iteraciones;
    
    assert_no_null(loop);
    
    new_loop = i_duplicate_loop(lface, id_nuevo_elemento);
    assert_no_null(new_loop);
    
    iterator = loop->ledge;
    last_hedge = NULL;
    num_iteraciones = 0;
    
    do
    {
        struct csmhedge_t *iterator_copy;
        
        assert(num_iteraciones < 10000);
        num_iteraciones++;
        
        iterator_copy = csmhedge_duplicate(iterator, new_loop, id_nuevo_elemento, relation_svertexs_old_to_new, relation_shedges_old_to_new);
        
        if (new_loop->ledge == NULL)
            new_loop->ledge = iterator_copy;
        else
            csmnode_insert_node2_after_node1(last_hedge, iterator_copy, csmhedge_t);
        
        last_hedge = iterator_copy;
        iterator = csmhedge_next(iterator);
    }
    while (iterator != loop->ledge);

    assert(csmhedge_next(last_hedge) == NULL);
    csmhedge_set_next(last_hedge, new_loop->ledge);

    assert(csmhedge_prev(new_loop->ledge) == NULL);
    csmhedge_set_prev(new_loop->ledge, last_hedge);
    
    return new_loop;
}

// --------------------------------------------------------------------------------------------------------------

struct csmhedge_t *csmloop_ledge(struct csmloop_t *loop)
{
    assert_no_null(loop);
    assert_no_null(loop->ledge);
    
    return loop->ledge;
}

// --------------------------------------------------------------------------------------------------------------

void csmloop_set_ledge(struct csmloop_t *loop, struct csmhedge_t *ledge)
{
    assert_no_null(loop);
    loop->ledge = ledge;
 }

// --------------------------------------------------------------------------------------------------------------

struct csmface_t *csmloop_lface(struct csmloop_t *loop)
{
    assert_no_null(loop);
    assert_no_null(loop->lface);
    
    return loop->lface;
}

// --------------------------------------------------------------------------------------------------------------

void csmloop_set_lface(struct csmloop_t *loop, struct csmface_t *face)
{
    assert_no_null(loop);
    loop->lface = face;
 }

// --------------------------------------------------------------------------------------------------------------

struct csmloop_t *csmloop_next(struct csmloop_t *loop)
{
    assert_no_null(loop);
    return csmnode_downcast(csmnode_next(CSMNODE(loop)), csmloop_t);
}

// ----------------------------------------------------------------------------------------------------

struct csmloop_t *csmloop_prev(struct csmloop_t *loop)
{
    assert_no_null(loop);
    return csmnode_downcast(csmnode_prev(CSMNODE(loop)), csmloop_t);
}
