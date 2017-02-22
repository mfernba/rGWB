// Vertex...

#include "csmvertex.inl"

#include "csmutilptr.hxx"

#include "cyassert.h"
#include "cypeid.h"
#include "cypespy.h"

struct csmvertex_t
{
    unsigned long id;
    unsigned long num_referencias;
    
    double x, y, z;
    
    struct csmvertex_t *prev, *next;
};

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmvertex_t *, i_crea, (unsigned long id, double x, double y, double z))
{
    struct csmvertex_t *vertex;
    
    vertex = MALLOC(struct csmvertex_t);
    
    vertex->id = id;
    vertex->num_referencias = 1;

    vertex->x = x;
    vertex->y = y;
    vertex->z = z;
    
    vertex->prev = NULL;
    vertex->next = NULL;

    return vertex;
}

// ----------------------------------------------------------------------------------------------------

struct csmvertex_t *csmvertex_crea(double x, double y, double z, unsigned long *id_nuevo_elemento)
{
    unsigned long id;
    
    id = cypeid_nuevo_id(id_nuevo_elemento, NULL);
    return i_crea(id, x, y, z);
}

// ----------------------------------------------------------------------------------------------------

void csmvertex_destruye(struct csmvertex_t **vertex)
{
    assert_no_null(vertex);
    assert_no_null(*vertex);
    assert((*vertex)->num_referencias > 0);
    
    (*vertex)->num_referencias--;
    
    if ((*vertex)->num_referencias == 0)
    {
        FREE_PP(vertex, struct csmvertex_t);
    }
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

static void i_set_ptr_next_or_prev(struct csmvertex_t **ptr, struct csmvertex_t *next_or_prev_vertex)
{
    assert_no_null(ptr);
    
    if (*ptr != NULL)
    {
        assert((*ptr)->num_referencias >= 2);
        (*ptr)->num_referencias--;
    }
    
    *ptr = next_or_prev_vertex;
    
    if (next_or_prev_vertex != NULL)
        next_or_prev_vertex->num_referencias++;
}

// ----------------------------------------------------------------------------------------------------

struct csmvertex_t *csmvertex_next(struct csmvertex_t *vertex)
{
    assert_no_null(vertex);
    return vertex->next;
}

// ----------------------------------------------------------------------------------------------------

void csmvertex_set_next(struct csmvertex_t *vertex, struct csmvertex_t *next_vertex)
{
    assert_no_null(vertex);
    assert_no_null(next_vertex);

    csmutilptr_set_ptr_next_or_prev(&vertex->next, next_vertex);
}

// ----------------------------------------------------------------------------------------------------

struct csmvertex_t *csmvertex_prev(struct csmvertex_t *vertex)
{
    assert_no_null(vertex);
    return vertex->prev;
}

// ----------------------------------------------------------------------------------------------------

void csmvertex_set_prev(struct csmvertex_t *vertex, struct csmvertex_t *prev_vertex)
{
    assert_no_null(vertex);
    assert_no_null(prev_vertex);

    csmutilptr_set_ptr_next_or_prev(&vertex->prev, prev_vertex);
}
