// Face...

#include "csmface.inl"

#include "csmbbox.inl"
#include "csmloop.inl"
#include "csmhedge.inl"
#include "csmmath.inl"
#include "csmnode.inl"
#include "csmtolerance.inl"
#include "csmvertex.inl"

#include "cyassert.h"
#include "cypeid.h"
#include "cypespy.h"
#include "defmath.tlh"
#include "standarc.h"

struct csmface_t
{
    unsigned long id;

    struct csmsolid_t *fsolid;
    
    struct csmloop_t *flout;
    struct csmloop_t *floops;
    
    double A, B, C, D;
    double fuzzy_epsilon;
    enum csmmath_dropped_coord_t dropped_coord;
    struct csmbbox_t *bbox;
};

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmface_t *, i_crea, (
                        unsigned long id,
                        struct csmsolid_t *fsolid,
                        struct csmloop_t *flout,
                        struct csmloop_t *floops,
                        double A, double B, double C, double D, double fuzzy_epsilon, enum csmmath_dropped_coord_t dropped_coord,
                        struct csmbbox_t **bbox))
{
    struct csmface_t *face;
    
    face = MALLOC(struct csmface_t);
    
    face->id = id;
    
    face->fsolid = fsolid;
    
    face->flout = flout;
    face->floops = floops;
    
    face->A = A;
    face->B = B;
    face->C = C;
    face->D = D;
    face->fuzzy_epsilon = fuzzy_epsilon;
    face->dropped_coord = dropped_coord;
    
    face->bbox = ASIGNA_PUNTERO_PP_NO_NULL(bbox, struct csmbbox_t);
    
    return face;
}

// ------------------------------------------------------------------------------------------

struct csmface_t *csmface_crea(struct csmsolid_t *solido, unsigned long *id_nuevo_elemento)
{
    unsigned long id;
    struct csmsolid_t *fsolid;
    struct csmloop_t *flout;
    struct csmloop_t *floops;
    double A, B, C, D, fuzzy_epsilon;
    enum csmmath_dropped_coord_t dropped_coord;
    struct csmbbox_t *bbox;
    
    id = cypeid_nuevo_id(id_nuevo_elemento, NULL);

    fsolid = solido;
    flout = NULL;
    floops = NULL;
    
    A = 0.;
    B = 0.;
    C = 0.;
    D = 0.;
    fuzzy_epsilon = 1.e-6;
    dropped_coord = (enum csmmath_dropped_coord_t)USHRT_MAX;
    
    bbox = csmbbox_create_empty_box();
    
    return i_crea(id, fsolid, flout, floops, A, B, C, D, fuzzy_epsilon, dropped_coord, &bbox);
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmface_t *, i_duplicate_face, (
                        struct csmsolid_t *fsolid,
                        double A, double B, double C, double D, double fuzzy_epsilon, enum csmmath_dropped_coord_t dropped_coord,
                        unsigned long *id_nuevo_elemento))
{
    unsigned long id;
    struct csmloop_t *flout, *floops;
    struct csmbbox_t *bbox;
    
    id = cypeid_nuevo_id(id_nuevo_elemento, NULL);
    flout = NULL;
    floops = NULL;
    bbox = csmbbox_create_empty_box();
    
    return i_crea(id, fsolid, flout, floops, A, B, C, D, fuzzy_epsilon, dropped_coord, &bbox);
}

// ------------------------------------------------------------------------------------------

struct csmface_t *csmface_duplicate(
                        struct csmface_t *face,
                        struct csmsolid_t *fsolid,
                        unsigned long *id_nuevo_elemento,
                        struct csmhashtb(csmvertex_t) *relation_svertexs_old_to_new,
                        struct csmhashtb(csmhedge_t) *relation_shedges_old_to_new)
{
    struct csmface_t *new_face;
    struct csmloop_t *iterator, *last_loop;
    
    assert_no_null(face);

    new_face = i_duplicate_face(fsolid, face->A, face->B, face->C, face->D, face->fuzzy_epsilon, face->dropped_coord, id_nuevo_elemento);
    assert_no_null(new_face);
    
    iterator = face->floops;
    last_loop = NULL;
    
    while (iterator != NULL)
    {
        struct csmloop_t *iterator_copy;
        
        iterator_copy = csmloop_duplicate(
                        iterator,
                        new_face,
                        id_nuevo_elemento,
                        relation_svertexs_old_to_new,
                        relation_shedges_old_to_new);
        
        if (new_face->floops == NULL)
            new_face->floops = iterator_copy;
        else
            csmnode_insert_node2_after_node1(last_loop, iterator_copy, csmloop_t);
        
        if (iterator == face->flout)
            new_face->flout = iterator_copy;
        
        last_loop = iterator_copy;
        iterator = csmloop_next(iterator);
    }
    
    assert_no_null(new_face->flout);
    assert_no_null(new_face->floops);
    
    return new_face;
}

// ------------------------------------------------------------------------------------------

void csmface_destruye(struct csmface_t **face)
{
    assert_no_null(face);
    assert_no_null(*face);

    if ((*face)->floops != NULL)
        csmnode_free_node_list(&(*face)->floops, csmloop_t);
    
    csmbbox_free(&(*face)->bbox);
    
    FREE_PP(face, struct csmface_t);
}

// ------------------------------------------------------------------------------------------

unsigned long csmface_id(const struct csmface_t *face)
{
    assert_no_null(face);
    return face->id;
}

// ----------------------------------------------------------------------------------------------------

void csmface_reassign_id(struct csmface_t *face, unsigned long *id_nuevo_elemento, unsigned long *new_id_opc)
{
    assert_no_null(face);
    face->id = cypeid_nuevo_id(id_nuevo_elemento, new_id_opc);
}

// ----------------------------------------------------------------------------------------------------

static void i_compute_bounding_box(struct csmloop_t *floops, struct csmbbox_t *bbox)
{
    register struct csmloop_t *iterator;
    unsigned long num_iters;
    
    csmbbox_reset(bbox);
    
    iterator = floops;
    num_iters = 0;
    
    do
    {
        assert(num_iters < 100000);
        num_iters++;

        csmloop_update_bounding_box(iterator, bbox);
        iterator = csmloop_next(iterator);
        
    } while (iterator != NULL);
}

// ----------------------------------------------------------------------------------------------------

static double i_compute_fuzzy_epsilon_for_containing_test(double A, double B, double C, double D, double max_tolerable_distance, struct csmloop_t *floops)
{
    register struct csmloop_t *iterator;
    unsigned long num_iters;
    double max_distance_to_plane;
    
    iterator = floops;
    num_iters = 0;
    max_distance_to_plane = 0.;
    
    do
    {
        double distance;
        
        assert(num_iters < 100000);
        num_iters++;

        distance = csmloop_max_distance_to_plane(iterator, A, B, C, D);
        assert(distance >= 0.);
        assert(distance <= max_tolerable_distance);
        
        max_distance_to_plane = MAX(max_distance_to_plane, distance);
        
        iterator = csmloop_next(iterator);
        
    } while (iterator != NULL);
    
    return MAX(1.01 * max_distance_to_plane, 1.e-4);
}

// ----------------------------------------------------------------------------------------------------

void csmface_redo_geometric_generated_data(struct csmface_t *face)
{
    double max_tolerable_distance;
    assert_no_null(face);
    assert_no_null(face->flout);
    assert_no_null(face->floops);
    
    csmloop_face_equation(face->flout, &face->A, &face->B, &face->C, &face->D);
    
    max_tolerable_distance = 1.1 * csmloop_max_distance_to_plane(face->flout, face->A, face->B, face->C, face->D);
    face->fuzzy_epsilon = i_compute_fuzzy_epsilon_for_containing_test(face->A, face->B, face->C, face->D, max_tolerable_distance, face->floops);
    face->dropped_coord = csmmath_dropped_coord(face->A, face->B, face->C);
    
    i_compute_bounding_box(face->floops, face->bbox);
}

// ------------------------------------------------------------------------------------------

CYBOOL csmface_contains_vertex(
                        const struct csmface_t *face,
                        const struct csmvertex_t *vertex,
                        enum csmmath_contaiment_point_loop_t *type_of_containment_opc,
                        struct csmvertex_t **hit_vertex_opc,
                        struct csmhedge_t **hit_hedge_opc)
{
    double x, y, z;
    CYBOOL is_outer_loop;
    
    assert_no_null(face);
    
    csmvertex_get_coordenadas(vertex, &x, &y, &z);
    is_outer_loop = CIERTO;
    
    return csmloop_is_point_inside_loop(
                        face->flout, is_outer_loop,
                        x, y, z, face->dropped_coord,
                        face->fuzzy_epsilon,
                        type_of_containment_opc, hit_vertex_opc, hit_hedge_opc);
}

// ------------------------------------------------------------------------------------------

CYBOOL csmface_contains_point(
                        const struct csmface_t *face,
                        double x, double y, double z,
                        enum csmmath_contaiment_point_loop_t *type_of_containment_opc,
                        struct csmvertex_t **hit_vertex_opc,
                        struct csmhedge_t **hit_hedge_opc)
{
    CYBOOL is_outer_loop;
    
    assert_no_null(face);
    
    is_outer_loop = CIERTO;
    return csmloop_is_point_inside_loop(
                        face->flout, is_outer_loop,
                        x, y, z, face->dropped_coord,
                        face->fuzzy_epsilon,
                        type_of_containment_opc, hit_vertex_opc, hit_hedge_opc);
}

// ------------------------------------------------------------------------------------------

CYBOOL csmface_is_loop_contained_in_face(struct csmface_t *face, struct csmloop_t *loop)
{
    assert_no_null(face);
    
    if (face->flout == loop)
    {
        return CIERTO;
    }
    else
    {
        register struct csmhedge_t *iterator;
        unsigned long num_iteraciones;
        CYBOOL is_outer_loop;
        
        iterator = csmloop_ledge(loop);
        num_iteraciones = 0;
        is_outer_loop = CIERTO;
        
        do
        {
            struct csmvertex_t *vertex;
            double x, y, z;
            
            assert(num_iteraciones < 100000);
            num_iteraciones++;
            
            vertex = csmhedge_vertex(iterator);
            csmvertex_get_coordenadas(vertex, &x, &y, &z);
    
            if (csmloop_is_point_inside_loop(
                        face->flout, is_outer_loop,
                        x, y, z, face->dropped_coord,
                        face->fuzzy_epsilon,
                        NULL, NULL, NULL) == FALSO)
            {
                return FALSO;
            }
            
        } while (iterator != csmloop_ledge(loop));
        
        return CIERTO;
    }
}

// ------------------------------------------------------------------------------------------

CYBOOL csmface_is_convex_hedge(struct csmface_t *face, struct csmhedge_t *hedge)
{
    struct csmhedge_t *hedge_prev, *hedge_next;
    const struct csmvertex_t *vertex_prev, *vertex, *vertex_next;
    double x_prev, y_prev, z_prev, x, y, z, x_next, y_next, z_next;
    double Ux_to_prev, Uy_to_prev, Uz_to_prev;
    double Ux_to_next, Uy_to_next, Uz_to_next;
    double Ux_cross_prev_next, Uy_cross_prev_next, Uz_cross_prev_next;
    double angle;
    double dot_product_face_normal;
    
    assert_no_null(face);
    
    hedge_prev = csmhedge_prev(hedge);
    vertex_prev = csmhedge_vertex(hedge_prev);
    csmvertex_get_coordenadas(vertex_prev, &x_prev, &y_prev, &z_prev);
    
    vertex = csmhedge_vertex(hedge);
    csmvertex_get_coordenadas(vertex, &x, &y, &z);
    
    hedge_next = csmhedge_next(hedge);
    vertex_next = csmhedge_vertex(hedge_next);
    csmvertex_get_coordenadas(vertex_next, &x_next, &y_next, &z_next);
    
    csmmath_vector_between_two_3D_points(x, y, z, x_prev, y_prev, z_prev, &Ux_to_prev, &Uy_to_prev, &Uz_to_prev);
    csmmath_vector_between_two_3D_points(x, y, z, x_next, y_next, z_next, &Ux_to_next, &Uy_to_next, &Uz_to_next);

    csmmath_cross_product3D(
                        Ux_to_next, Uy_to_next, Uz_to_next, Ux_to_prev, Uy_to_prev, Uz_to_prev,
                        &Ux_cross_prev_next, &Uy_cross_prev_next, &Uz_cross_prev_next);

    dot_product_face_normal =csmmath_dot_product3D(Ux_cross_prev_next, Uy_cross_prev_next, Uz_cross_prev_next, face->A, face->B, face->C);
    angle = acos(csmmath_dot_product3D(Ux_to_prev, Uy_to_prev, Uz_to_prev, Ux_to_next, Uy_to_next, Uz_to_next));
    
    if (dot_product_face_normal < 0.)
        angle = 2. * PI - angle;
    
    if (angle - csmtolerance_angle_rad () < PI)
        return CIERTO;
    else
        return FALSO;
}

// ------------------------------------------------------------------------------------------

double csmface_tolerace(const struct csmface_t *face)
{
    assert_no_null(face);
    return face->fuzzy_epsilon;
}

// ------------------------------------------------------------------------------------------

struct csmsolid_t *csmface_fsolid(struct csmface_t *face)
{
    assert_no_null(face);
    return face->fsolid;
}

// ------------------------------------------------------------------------------------------

void csmface_set_fsolid(struct csmface_t *face, struct csmsolid_t *solid)
{
    assert_no_null(face);
    face->fsolid = solid;
}

// ------------------------------------------------------------------------------------------

struct csmloop_t *csmface_flout(struct csmface_t *face)
{
    assert_no_null(face);
    return face->flout;
}

// ------------------------------------------------------------------------------------------

void csmface_set_flout(struct csmface_t *face, struct csmloop_t *loop)
{
    assert_no_null(face);
    
    if (face->floops == NULL)
        face->floops = loop;
    
    face->flout = loop;
}

// ------------------------------------------------------------------------------------------

struct csmloop_t *csmface_floops(struct csmface_t *face)
{
    assert_no_null(face);
    return face->floops;
}

// ------------------------------------------------------------------------------------------

void csmface_set_floops(struct csmface_t *face, struct csmloop_t *loop)
{
    assert_no_null(face);
    face->floops = loop;
}

// ----------------------------------------------------------------------------------------------------

void csmface_add_loop_while_removing_from_old(struct csmface_t *face, struct csmloop_t *loop)
{
    struct csmface_t *loop_old_face;
    
    assert_no_null(face);
    assert_no_null(loop);
    
    loop_old_face = csmloop_lface(loop);
    assert_no_null(loop_old_face);
    assert_no_null(loop_old_face->floops);
    
    if (loop_old_face->floops == loop)
        loop_old_face->floops = csmloop_next(loop_old_face->floops);

    if (loop_old_face->flout == loop)
        loop_old_face->flout = loop_old_face->floops;
    
    csmloop_set_lface(loop, face);
    
    if (face->floops == NULL)
    {
        csmnode_remove_from_own_list(loop, csmloop_t);
        
        face->floops = loop;
        face->flout = loop;
    }
    else
    {
        csmnode_insert_node2_before_node1(face->floops, loop, csmloop_t);
        face->floops = loop;
    }
}

// ----------------------------------------------------------------------------------------------------

void csmface_remove_loop(struct csmface_t *face, struct csmloop_t **loop)
{
    assert_no_null(face);
    assert_no_null(loop);
    assert(face == csmloop_lface(*loop));
    
    if (face->floops == *loop)
        face->floops = csmloop_next(face->floops);

    if (face->flout == *loop)
        face->flout = face->floops;
    
    csmnode_free_node_in_list(loop, csmloop_t);
}

// ----------------------------------------------------------------------------------------------------

CYBOOL csmface_has_holes(const struct csmface_t *face)
{
    assert_no_null(face);
    
    if (face->floops != NULL && csmloop_next(face->floops) != NULL)
        return CIERTO;
    else
        return FALSO;
}









