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
    struct csmface_t *face;
};

// --------------------------------------------------------------------------------------------------------------

static void i_csmloop_destruye(struct csmloop_t **loop)
{
    assert_no_null(loop);
    assert_no_null(*loop);

    csmnode_release_ex((*loop)->ledge, csmhedge_t);
    csmnode_release_ex((*loop)->face, csmface_t);
    
    FREE_PP(loop, struct csmloop_t);
}

// --------------------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmloop_t *, i_crea, (
                        unsigned long id,
                        struct csmhedge_t *ledge,
                        struct csmface_t *face))
{
    struct csmloop_t *loop;
    
    loop->clase_base = csmnode_crea_node(id, loop, i_csmloop_destruye, csmloop_t);
    
    loop->ledge = ledge;
    loop->face = face;
    
    return loop;
}

// --------------------------------------------------------------------------------------------------------------

struct csmloop_t *csmloop_crea(unsigned long *id_nuevo_elemento)
{
    unsigned long id;
    struct csmhedge_t *ledge;
    struct csmface_t *face;
    
    id = cypeid_nuevo_id(id_nuevo_elemento, NULL);
    
    ledge = NULL;
    face = NULL;
    
    return i_crea(id, ledge, face);
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
    
    csmnode_release_ex(loop->ledge, csmhedge_t);
    loop->ledge = csmnode_retain_ex(ledge, csmhedge_t);
 }

// --------------------------------------------------------------------------------------------------------------

struct csmface_t *csmloop_face(struct csmloop_t *loop)
{
    assert_no_null(loop);
    assert_no_null(loop->face);
    
    return loop->face;
}

// --------------------------------------------------------------------------------------------------------------

void csmloop_set_face(struct csmloop_t *loop, struct csmface_t *face)
{
    assert_no_null(loop);
    
    csmnode_release_ex(loop->face, csmface_t);
    loop->face = csmnode_retain_ex(face, csmface_t);
 }

// --------------------------------------------------------------------------------------------------------------

struct csmloop_t *csmloop_next(struct csmloop_t *loop)
{
    assert_no_null(loop);
    return csmnode_downcast(csmnode_next(CSMNODE(loop)), csmloop_t);
}

// ----------------------------------------------------------------------------------------------------

void csmloop_set_next(struct csmloop_t *loop, struct csmloop_t *next_loop)
{
    assert_no_null(loop);
    assert_no_null(next_loop);
    
    csmnode_set_ptr_next(CSMNODE(loop), CSMNODE(next_loop));
}

// ----------------------------------------------------------------------------------------------------

struct csmloop_t *csmloop_prev(struct csmloop_t *loop)
{
    assert_no_null(loop);
    return csmnode_downcast(csmnode_prev(CSMNODE(loop)), csmloop_t);
}

// ----------------------------------------------------------------------------------------------------

void csmloop_set_prev(struct csmloop_t *loop, struct csmloop_t *prev_loop)
{
    assert_no_null(loop);
    assert_no_null(prev_loop);

    csmnode_set_ptr_prev(CSMNODE(loop), CSMNODE(prev_loop));
}
