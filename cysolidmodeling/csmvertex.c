// Vertex...

#include "csmvertex.inl"
#include "csmvertex.tli"

#include "csmdebug.inl"
#include "csmhashtb.inl"
#include "csmmath.inl"
#include "csmtransform.inl"

#include "csmassert.inl"
#include "csmid.inl"
#include "csmmem.inl"

struct csmvertex_t
{
    unsigned long id;
    
    double x, y, z;
    struct csmhedge_t *hedge;
    
    csmvertex_mask_t algorithm_attrib_mask;
};

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmvertex_t *, i_crea, (
						unsigned long id,
                        double x, double y, double z,
                        struct csmhedge_t *hedge,
                        csmvertex_mask_t algorithm_attrib_mask))
{
    struct csmvertex_t *vertex;
    
    vertex = MALLOC(struct csmvertex_t);
    
    vertex->id = id;

    vertex->x = x;
    vertex->y = y;
    vertex->z = z;
    
    vertex->hedge = hedge;
    
    vertex->algorithm_attrib_mask = algorithm_attrib_mask;

    return vertex;
}

// ----------------------------------------------------------------------------------------------------

struct csmvertex_t *csmvertex_crea(double x, double y, double z, unsigned long *id_nuevo_elemento)
{
    unsigned long id;
    struct csmhedge_t *hedge;
    csmvertex_mask_t algorithm_attrib_mask;
    
    id = csmid_new_id(id_nuevo_elemento, NULL);
    
    hedge = NULL;
    
    algorithm_attrib_mask = CSMVERTEX_NULL_MASK;
    
    return i_crea(id, x, y, z, hedge, algorithm_attrib_mask);
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmvertex_t *, i_duplicate_vertex, (double x, double y, double z, unsigned long *id_nuevo_elemento))
{
    unsigned long id;
    struct csmhedge_t *hedge;
    csmvertex_mask_t algorithm_attrib_mask;
    
    id = csmid_new_id(id_nuevo_elemento, NULL);
    
    hedge = NULL;
    
    algorithm_attrib_mask = CSMVERTEX_NULL_MASK;
    
    return i_crea(id, x, y, z, hedge, algorithm_attrib_mask);
}

// ----------------------------------------------------------------------------------------------------

struct csmvertex_t *csmvertex_duplicate(
                        const struct csmvertex_t *vertex,
                        unsigned long *id_nuevo_elemento,
                        struct csmhashtb(csmvertex_t) *relation_svertexs_old_to_new)
{
    struct csmvertex_t *new_vertex;
    
    assert_no_null(vertex);
    
    new_vertex = i_duplicate_vertex(vertex->x, vertex->y, vertex->z, id_nuevo_elemento);
    assert_no_null(new_vertex);
    assert(new_vertex->hedge == NULL);
    
    csmhashtb_add_item(relation_svertexs_old_to_new, vertex->id, new_vertex, csmvertex_t);
    
    return new_vertex;
}

// ----------------------------------------------------------------------------------------------------

void csmvertex_destruye(struct csmvertex_t **vertex)
{
    assert_no_null(vertex);
    assert_no_null(*vertex);

    FREE_PP(vertex, struct csmvertex_t);
}

// ----------------------------------------------------------------------------------------------------

unsigned long csmvertex_id(const struct csmvertex_t *vertex)
{
    assert_no_null(vertex);
    return vertex->id;
}

// ----------------------------------------------------------------------------------------------------

void csmvertex_reassign_id(struct csmvertex_t *vertex, unsigned long *id_nuevo_elemento, unsigned long *new_id_opc)
{
    assert_no_null(vertex);
    vertex->id = csmid_new_id(id_nuevo_elemento, new_id_opc);
}

// ----------------------------------------------------------------------------------------------------

struct csmhedge_t *csmvertex_hedge(struct csmvertex_t *vertex)
{
    assert_no_null(vertex);
    return vertex->hedge;
}

// ----------------------------------------------------------------------------------------------------

void csmvertex_set_hedge(struct csmvertex_t *vertex, struct csmhedge_t *hedge)
{
    assert_no_null(vertex);
    vertex->hedge = hedge;
}

// ----------------------------------------------------------------------------------------------------

CSMBOOL csmvertex_has_mask_attrib(const struct csmvertex_t *vertex, csmvertex_mask_t mask_attrib)
{
    assert_no_null(vertex);
    return IS_TRUE((vertex->algorithm_attrib_mask & mask_attrib) == mask_attrib);
}

// ----------------------------------------------------------------------------------------------------

csmvertex_mask_t csmvertex_get_mask_attrib(const struct csmvertex_t *vertex)
{
    assert_no_null(vertex);
    return vertex->algorithm_attrib_mask;
}

// ----------------------------------------------------------------------------------------------------

void csmvertex_set_mask_attrib(struct csmvertex_t *vertex, csmvertex_mask_t mask_attrib)
{
    assert_no_null(vertex);
    vertex->algorithm_attrib_mask |= mask_attrib;
}

// ----------------------------------------------------------------------------------------------------

void csmvertex_clear_mask_attrib(struct csmvertex_t *vertex, csmvertex_mask_t mask_attrib)
{
    assert_no_null(vertex);
    vertex->algorithm_attrib_mask &= ~mask_attrib;
}

// ----------------------------------------------------------------------------------------------------

void csmvertex_clear_mask(struct csmvertex_t *vertex)
{
    assert_no_null(vertex);
    vertex->algorithm_attrib_mask = CSMVERTEX_NULL_MASK;
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

void csmvertex_vector_from_vertex1_to_vertex2(
                        const struct csmvertex_t *vertex1, const struct csmvertex_t *vertex2,
                        double *Ux, double *Uy, double *Uz)
{
    assert_no_null(vertex1);
    assert_no_null(vertex2);
    
    csmmath_vector_between_two_3D_points(vertex1->x, vertex1->y, vertex1->z, vertex2->x, vertex2->y, vertex2->z, Ux, Uy, Uz);
}

// ----------------------------------------------------------------------------------------------------

void csmvertex_get_coords_not_dropped(
                        const struct csmvertex_t *vertex,
                        enum csmmath_dropped_coord_t dropped_coord,
                        double *x, double *y)
{
    assert_no_null(vertex);
    csmmath_select_not_dropped_coords(vertex->x, vertex->y, vertex->z, dropped_coord, x, y);
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

CSMBOOL csmvertex_equal_coords(const struct csmvertex_t *vertex1, const struct csmvertex_t *vertex2, double epsilon)
{
    assert_no_null(vertex1);
    assert_no_null(vertex2);
    
    return csmmath_equal_coords(vertex1->x, vertex1->y, vertex1->z, vertex2->x, vertex2->y, vertex2->z, epsilon);
}

// ----------------------------------------------------------------------------------------------------

void csmvertex_apply_transform(struct csmvertex_t *vertex, const struct csmtransform_t *transform)
{
    assert_no_null(vertex);
    csmtransform_transform_point(transform, vertex->x, vertex->y, vertex->z, &vertex->x, &vertex->y, &vertex->z);
}

// ----------------------------------------------------------------------------------------------------

void csmvertex_print_debug_info(struct csmvertex_t *vertex)
{
    assert_no_null(vertex);
    
    csmdebug_print_debug_info("\tVertex %6lu\t%6.3lf\t%6.3lf\t%6.3lf [%lu]", vertex->id, vertex->x, vertex->y, vertex->z, vertex->algorithm_attrib_mask);
    
    if (vertex->algorithm_attrib_mask & CSMVERTEX_MASK_VERTEX_ON_HOLE_LOOP)
        csmdebug_print_debug_info(" [H] ");
    
    if (vertex->algorithm_attrib_mask & CSMVERTEX_MASK_SETOP_COMMON_VERTEX)
        csmdebug_print_debug_info(" [CV] ");
    
    if (vertex->algorithm_attrib_mask &     CSMVERTEX_MASK_SETOP_VTX_FAC_CLASS)
        csmdebug_print_debug_info(" [VTXFACC] ");
}





