// Half-Edge...

#include "csmhedge.inl"

#include "csmnode.inl"
#include "csmutilptr.hxx"

#include "cyassert.h"
#include "cypeid.h"
#include "cypespy.h"

struct csmhedge_t
{
    struct csmnode_t clase_base;
    
    struct csmedge_t *edge;
    struct csmvertex_t *vertex;
    struct csmloop_t *loop;
    
    struct csmhedge_t *prev, *next;
};

// --------------------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmhedge_t *, i_crea, (
                        unsigned long id,
                        struct csmedge_t *edge,
                        struct csmvertex_t *vertex,
                        struct csmloop_t *loop,
                        struct csmhedge_t *prev, struct csmhedge_t *next))
{
    struct csmhedge_t *hedge;
    
    hedge = MALLOC(struct csmhedge_t);
    
    hedge->clase_base = csmnode_crea_node(id, hedge, csmhedge_destruye, csmhedge_t);
    
    hedge->edge = edge;
    hedge->vertex = vertex;
    hedge->loop = loop;
    
    hedge->prev = prev;
    hedge->next = next;
    
    return hedge;
}

// --------------------------------------------------------------------------------------------------------------

struct csmhedge_t *csmhedge_crea(unsigned long *id_nuevo_elemento)
{
    unsigned long id;
    struct csmedge_t *edge;
    struct csmvertex_t *vertex;
    struct csmloop_t *loop;
    struct csmhedge_t *prev, *next;
    
    id = cypeid_nuevo_id(id_nuevo_elemento, NULL);

    edge = NULL;
    vertex = NULL;
    loop = NULL;
    prev = NULL;
    next = NULL;
    
    return i_crea(id, edge, vertex, loop, prev, next);
}

// --------------------------------------------------------------------------------------------------------------

void csmhedge_destruye(struct csmhedge_t **hedge)
{
    assert_no_null(hedge);
    assert_no_null(*hedge);
    
    csmnode_unretain(CSMNODE(*hedge));
    *hedge = NULL;
}







