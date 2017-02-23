// Vertex...

#include "csmvertex.inl"

#include "csmnode.inl"

#include "cyassert.h"
#include "cypeid.h"
#include "cypespy.h"

struct csmvertex_t
{
    struct csmnode_t clase_base;
    
    double x, y, z;
    struct csmhedge_t *hedge;
};

// ----------------------------------------------------------------------------------------------------

static void i_csmvertex_destruye(struct csmvertex_t **vertex)
{
    assert_no_null(vertex);
    assert_no_null(*vertex);

    csmnode_release_ex((*vertex)->hedge, csmhedge_t);
    
    FREE_PP(vertex, struct csmvertex_t);
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmvertex_t *, i_crea, (
						unsigned long id,
                        double x, double y, double z,
                        struct csmhedge_t *hedge))
{
    struct csmvertex_t *vertex;
    
    vertex = MALLOC(struct csmvertex_t);
    
    vertex->clase_base = csmnode_crea_node(id, vertex, i_csmvertex_destruye, csmvertex_t);

    vertex->x = x;
    vertex->y = y;
    vertex->z = z;
    
    vertex->hedge = hedge;

    return vertex;
}

// ----------------------------------------------------------------------------------------------------

struct csmvertex_t *csmvertex_crea(double x, double y, double z, unsigned long *id_nuevo_elemento)
{
    unsigned long id;
    struct csmhedge_t *hedge;
    
    id = cypeid_nuevo_id(id_nuevo_elemento, NULL);
    
    hedge = NULL;
    
    return i_crea(id, x, y, z, hedge);
}

// ----------------------------------------------------------------------------------------------------

void csmvertex_set_hedge(struct csmvertex_t *vertex, struct csmhedge_t *hedge)
{
    assert_no_null(vertex);
    
    csmnode_release_ex(vertex->hedge, csmhedge_t);
    vertex->hedge = csmnode_retain_ex(vertex->hedge, csmhedge_t);
}

// ----------------------------------------------------------------------------------------------------

void csmvertex_get_coordenadas(const struct csmvertex_t *vertex, double *x, double *y, double *z)
{
    assert_no_null(vertex);
    assert_no_null(x);
    assert_no_null(y);
    assert_no_null(z);
    
    *x = vertex->x;
    *y = vertex->y;
    *z = vertex->z;
}

// ----------------------------------------------------------------------------------------------------

void csmvertex_set_coordenadas(struct csmvertex_t *vertex, double x, double y, double z)
{
    assert_no_null(vertex);
    
    vertex->x = x;
    vertex->y = y;
    vertex->z = z;
}

// ----------------------------------------------------------------------------------------------------

struct csmvertex_t *csmvertex_next(struct csmvertex_t *vertex)
{
    assert_no_null(vertex);
    return csmnode_downcast(csmnode_next(CSMNODE(vertex)), csmvertex_t);
}

// ----------------------------------------------------------------------------------------------------

void csmvertex_set_next(struct csmvertex_t *vertex, struct csmvertex_t *next_vertex)
{
    assert_no_null(vertex);
    assert_no_null(next_vertex);
    
    csmnode_set_ptr_next(CSMNODE(vertex), CSMNODE(next_vertex));
}

// ----------------------------------------------------------------------------------------------------

struct csmvertex_t *csmvertex_prev(struct csmvertex_t *vertex)
{
    assert_no_null(vertex);
    return csmnode_downcast(csmnode_prev(CSMNODE(vertex)), csmvertex_t);
}

// ----------------------------------------------------------------------------------------------------

void csmvertex_set_prev(struct csmvertex_t *vertex, struct csmvertex_t *prev_vertex)
{
    assert_no_null(vertex);
    assert_no_null(prev_vertex);

    csmnode_set_ptr_prev(CSMNODE(vertex), CSMNODE(prev_vertex));
}
