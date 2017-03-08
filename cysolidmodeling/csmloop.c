// Loop...

#include "csmloop.inl"

#include "csmnode.inl"

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

CYBOOL csmloop_ids_iguales(const struct csmloop_t *loop1, const struct csmloop_t *loop2)
{
    assert_no_null(loop1);
    assert_no_null(loop2);
    
    return ES_CIERTO(loop1->clase_base.id == loop2->clase_base.id);
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
