// Half-Edge...

#include "csmhedge.inl"

#include "csmnode.inl"

#include "cyassert.h"
#include "cypeid.h"
#include "cypespy.h"

struct csmhedge_t
{
    struct csmnode_t clase_base;
    
    struct csmedge_t *edge;
    struct csmvertex_t *vertex;
    struct csmloop_t *loop;
};

// --------------------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmhedge_t *, i_crea, (
                        unsigned long id,
                        struct csmedge_t *edge,
                        struct csmvertex_t *vertex,
                        struct csmloop_t *loop))
{
    struct csmhedge_t *hedge;
    
    hedge = MALLOC(struct csmhedge_t);
    
    hedge->clase_base = csmnode_crea_node(id, hedge, csmhedge_destruye, csmhedge_t);
    
    hedge->edge = edge;
    hedge->vertex = vertex;
    hedge->loop = loop;
    
    return hedge;
}

// --------------------------------------------------------------------------------------------------------------

struct csmhedge_t *csmhedge_crea(unsigned long *id_nuevo_elemento)
{
    unsigned long id;
    struct csmedge_t *edge;
    struct csmvertex_t *vertex;
    struct csmloop_t *loop;
    
    id = cypeid_nuevo_id(id_nuevo_elemento, NULL);

    edge = NULL;
    vertex = NULL;
    loop = NULL;
    
    return i_crea(id, edge, vertex, loop);
}

// --------------------------------------------------------------------------------------------------------------

void csmhedge_destruye(struct csmhedge_t **hedge)
{
    assert_no_null(hedge);
    assert_no_null(*hedge);

    csmnode_unretain_ex((*hedge)->edge, csmedge_t);
    csmnode_unretain_ex((*hedge)->vertex, csmvertex_t);
    csmnode_unretain_ex((*hedge)->loop, csmloop_t);
    
    csmnode_unretain_ex(*hedge, csmhedge_t);
    *hedge = NULL;
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
    
    csmnode_unretain_ex(hedge->edge, csmedge_t);
    hedge->edge = csmnode_retain_ex(edge, csmedge_t);
}

// --------------------------------------------------------------------------------------------------------------

struct csmvertex_t *csmhedge_vertex(struct csmhedge_t *hedge)
{
    assert_no_null(hedge);
    return hedge->vertex;
}

// --------------------------------------------------------------------------------------------------------------

void csmhedge_set_vertex(struct csmhedge_t *hedge, struct csmvertex_t *vertex)
{
    assert_no_null(hedge);
    
    csmnode_unretain_ex(hedge->vertex, csmvertex_t);
    hedge->vertex = csmnode_retain_ex(vertex, csmvertex_t);
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
    
    csmnode_unretain_ex(hedge->loop, csmloop_t);
    hedge->loop = csmnode_retain_ex(loop, csmloop_t);
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
    assert_no_null(next_hedge);
    
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
    assert_no_null(prev_hedge);

    csmnode_set_ptr_prev(CSMNODE(hedge), CSMNODE(prev_hedge));
}







