// Face...

#include "csmface.inl"

#include "csmbbox.inl"
#include "csmdebug.inl"
#include "csmloop.inl"
#include "csmhedge.inl"
#include "csmmath.inl"
#include "csmnode.inl"
#include "csmtolerance.inl"
#include "csmvertex.inl"

#include "cont2d.h"
#include "cyassert.h"
#include "cypeid.h"
#include "cypespy.h"
#include "defmath.tlh"
#include "standarc.h"

struct csmface_t
{
    unsigned long id;

    struct csmsolid_t *fsolid;
    struct csmsolid_t *fsolid_aux;
    
    struct csmloop_t *flout;
    struct csmloop_t *floops;
    
    double A, B, C, D;
    double fuzzy_epsilon;
    enum csmmath_dropped_coord_t dropped_coord;
    struct csmbbox_t *bbox;
};

static const double i_COS_15_DEGREES = 0.9659358;

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmface_t *, i_crea, (
                        unsigned long id,
                        struct csmsolid_t *fsolid, struct csmsolid_t *fsolid_aux,
                        struct csmloop_t *flout,
                        struct csmloop_t *floops,
                        double A, double B, double C, double D, double fuzzy_epsilon, enum csmmath_dropped_coord_t dropped_coord,
                        struct csmbbox_t **bbox))
{
    struct csmface_t *face;
    
    face = MALLOC(struct csmface_t);
    
    face->id = id;
    
    face->fsolid = fsolid;
    face->fsolid_aux = fsolid_aux;
    
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
    struct csmsolid_t *fsolid, *fsolid_aux;
    struct csmloop_t *flout;
    struct csmloop_t *floops;
    double A, B, C, D, fuzzy_epsilon;
    enum csmmath_dropped_coord_t dropped_coord;
    struct csmbbox_t *bbox;
    
    id = cypeid_nuevo_id(id_nuevo_elemento, NULL);

    fsolid = solido;
    fsolid_aux = NULL;
    flout = NULL;
    floops = NULL;
    
    A = 0.;
    B = 0.;
    C = 0.;
    D = 0.;
    fuzzy_epsilon = 1.e-6;
    dropped_coord = (enum csmmath_dropped_coord_t)USHRT_MAX;
    
    bbox = csmbbox_create_empty_box();
    
    return i_crea(id, fsolid, fsolid_aux, flout, floops, A, B, C, D, fuzzy_epsilon, dropped_coord, &bbox);
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
    
    return i_crea(id, fsolid, NULL, flout, floops, A, B, C, D, fuzzy_epsilon, dropped_coord, &bbox);
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
    register struct csmloop_t *iterator;
    unsigned long num_iters;
    
    assert_no_null(face);
    
    face->id = cypeid_nuevo_id(id_nuevo_elemento, new_id_opc);
    
    iterator = face->floops;
    num_iters = 0;
    
    do
    {
        assert(num_iters < 100000);
        num_iters++;

        csmloop_reassign_id(iterator, id_nuevo_elemento, new_id_opc);
        iterator = csmloop_next(iterator);
        
    } while (iterator != NULL);
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
    
    return MAX(1.01 * max_distance_to_plane, 1.e-3);
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

static CYBOOL i_is_point_on_face_plane(
                        double x, double y, double z,
                        double A, double B, double C, double D,
                        double fuzzy_epsilon)
{
    double distance;
    
    distance = csmmath_signed_distance_point_to_plane(x, y, z, A, B, C, D);
    
    if (fabs(distance) < fuzzy_epsilon)
        return CIERTO;
    else
        return FALSO;
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
    
    assert_no_null(face);
    
    csmvertex_get_coordenadas(vertex, &x, &y, &z);
    
    if (i_is_point_on_face_plane(
                        x, y, z,
                        face->A, face->B, face->C, face->D,
                        face->fuzzy_epsilon) == FALSO)
    {
        return FALSO;
    }
    else
    {
        return csmloop_is_point_inside_loop(
                        face->flout,
                        x, y, z, face->dropped_coord,
                        face->fuzzy_epsilon,
                        type_of_containment_opc, hit_vertex_opc, hit_hedge_opc);
    }
}

// ------------------------------------------------------------------------------------------

CYBOOL csmface_contains_point(
                        const struct csmface_t *face,
                        double x, double y, double z,
                        enum csmmath_contaiment_point_loop_t *type_of_containment_opc,
                        struct csmvertex_t **hit_vertex_opc,
                        struct csmhedge_t **hit_hedge_opc)
{
    CYBOOL containts_point;
    enum csmmath_contaiment_point_loop_t type_of_containment_loc;
    struct csmvertex_t *hit_vertex_loc;
    struct csmhedge_t *hit_hedge_loc;
    
    assert_no_null(face);
    
    if (i_is_point_on_face_plane(
                        x, y, z,
                        face->A, face->B, face->C, face->D,
                        face->fuzzy_epsilon) == FALSO)
    {
        containts_point = FALSO;
        
        type_of_containment_loc = (enum csmmath_contaiment_point_loop_t)USHRT_MAX;
        hit_vertex_loc = NULL;
        hit_hedge_loc = NULL;
    }
    else
    {
        if (csmloop_is_point_inside_loop(
                        face->flout,
                        x, y, z, face->dropped_coord,
                        face->fuzzy_epsilon,
                        &type_of_containment_loc, &hit_vertex_loc, &hit_hedge_loc) == FALSO)
        {
            containts_point = FALSO;
        }
        else
        {
            if (type_of_containment_loc != CSMMATH_CONTAIMENT_POINT_LOOP_INTERIOR)
            {
                containts_point = CIERTO;
            }
            else
            {
                struct csmloop_t *loop_iterator;
            
                loop_iterator = face->floops;
                containts_point = CIERTO;
            
                while (loop_iterator != NULL)
                {
                    if (loop_iterator != face->flout && csmloop_has_only_a_null_edge(loop_iterator) == FALSO)
                    {
                        enum csmmath_contaiment_point_loop_t type_of_containment_hole;
                        struct csmvertex_t *hit_vertex_hole;
                        struct csmhedge_t *hit_hedge_hole;
                        
                        if (csmloop_is_point_inside_loop(
                                loop_iterator,
                                x, y, z, face->dropped_coord,
                                face->fuzzy_epsilon,
                                &type_of_containment_hole, &hit_vertex_hole, &hit_hedge_hole) == CIERTO)
                        {
                            if (type_of_containment_hole == CSMMATH_CONTAIMENT_POINT_LOOP_INTERIOR)
                            {
                                containts_point = FALSO;
                            }
                            else
                            {
                                containts_point = CIERTO;
                                
                                type_of_containment_loc = type_of_containment_hole;
                                hit_vertex_loc = hit_vertex_hole;
                                hit_hedge_loc = hit_hedge_hole;
                            }
                            break;
                        }
                    }
                
                    loop_iterator = csmloop_next(loop_iterator);
                }
            }
        }
    }

    ASIGNA_OPC(type_of_containment_opc, type_of_containment_loc);
    ASIGNA_OPC(hit_vertex_opc, hit_vertex_loc);
    ASIGNA_OPC(hit_hedge_opc, hit_hedge_loc);
    
    return containts_point;
}

// ------------------------------------------------------------------------------------------

CYBOOL csmface_is_point_interior_to_face(const struct csmface_t *face, double x, double y, double z)
{
    CYBOOL is_interior_to_face;
    
    assert_no_null(face);
    
    if (i_is_point_on_face_plane(
                        x, y, z,
                        face->A, face->B, face->C, face->D,
                        face->fuzzy_epsilon) == FALSO)
    {
        return FALSO;
    }
    else
    {
        enum csmmath_contaiment_point_loop_t type_of_containment;
    
        if (csmloop_is_point_inside_loop(
                        face->flout,
                        x, y, z, face->dropped_coord,
                        face->fuzzy_epsilon,
                        &type_of_containment, NULL, NULL) == FALSO)
        {
            is_interior_to_face = FALSO;
        }
        else if (type_of_containment != CSMMATH_CONTAIMENT_POINT_LOOP_INTERIOR)
        {
            is_interior_to_face = CIERTO;
        }
        else
        {
            struct csmloop_t *loop_iterator;
            
            loop_iterator = face->floops;
            is_interior_to_face = CIERTO;
            
            while (loop_iterator != NULL)
            {
                if (loop_iterator != face->flout && csmloop_has_only_a_null_edge(loop_iterator) == FALSO)
                {
                    if (csmloop_is_point_inside_loop(
                            loop_iterator,
                            x, y, z, face->dropped_coord,
                            face->fuzzy_epsilon,
                            &type_of_containment, NULL, NULL) == CIERTO)
                    {
                        if (type_of_containment == CSMMATH_CONTAIMENT_POINT_LOOP_INTERIOR)
                            is_interior_to_face = FALSO;
                        else
                            is_interior_to_face = CIERTO;
                        
                        break;
                    }
                }
            
                loop_iterator = csmloop_next(loop_iterator);
            }
        }
    }
    
    return is_interior_to_face;
}

// ------------------------------------------------------------------------------------------

enum csmmath_double_relation_t csmface_classify_vertex_relative_to_face(const struct csmface_t *face, const struct csmvertex_t *vertex)
{
    double x, y, z;
    double dist;
    
    assert_no_null(face);
    
    csmvertex_get_coordenadas(vertex, &x, &y, &z);
    dist = csmmath_signed_distance_point_to_plane(x, y, z, face->A, face->B, face->C, face->D);
    
    return csmmath_compare_doubles(dist, 0., face->fuzzy_epsilon);
}

// ------------------------------------------------------------------------------------------

CYBOOL csmface_exists_intersection_between_line_and_face_plane(
                        const struct csmface_t *face,
                        double x1, double y1, double z1, double x2, double y2, double z2,
                        double *x_inters_opc, double *y_inters_opc, double *z_inters_opc, double *t_inters_opc)
{
    CYBOOL exists_intersection;
    double x_inters_loc, y_inters_loc, z_inters_loc, t_inters_loc;
    double dist1, dist2;
    enum csmmath_double_relation_t relation1, relation2;
    
    assert_no_null(face);
    
    dist1 = csmmath_signed_distance_point_to_plane(x1, y1, z1, face->A, face->B, face->C, face->D);
    relation1 = csmmath_compare_doubles(dist1, 0., face->fuzzy_epsilon);
    
    dist2 = csmmath_signed_distance_point_to_plane(x2, y2, z2, face->A, face->B, face->C, face->D);
    relation2 = csmmath_compare_doubles(dist2, 0., face->fuzzy_epsilon);
    
    if ((relation1 == CSMMATH_VALUE1_LESS_THAN_VALUE2 && relation2 == CSMMATH_VALUE1_GREATER_THAN_VALUE2)
            || (relation1 == CSMMATH_VALUE1_GREATER_THAN_VALUE2 && relation2 == CSMMATH_VALUE1_LESS_THAN_VALUE2))
    {
        exists_intersection = CIERTO;
        
        t_inters_loc = dist1 / (dist1 - dist2);
            
        x_inters_loc = x1 + t_inters_loc * (x2 - x1);
        y_inters_loc = y1 + t_inters_loc * (y2 - y1);
        z_inters_loc = z1 + t_inters_loc * (z2 - z1);
    }
    else if (relation1 == CSMMATH_EQUAL_VALUES && relation2 != CSMMATH_EQUAL_VALUES)
    {
        exists_intersection = CIERTO;
        
        t_inters_loc = 0.;
            
        x_inters_loc = x1;
        y_inters_loc = y1;
        z_inters_loc = z1;
    }
    else if (relation1 != CSMMATH_EQUAL_VALUES && relation2 == CSMMATH_EQUAL_VALUES)
    {
        exists_intersection = CIERTO;
        
        t_inters_loc = 1.;
            
        x_inters_loc = x2;
        y_inters_loc = y2;
        z_inters_loc = z2;
    }
    else
    {
        exists_intersection = FALSO;
        
        x_inters_loc = 0.;
        y_inters_loc = 0.;
        z_inters_loc = 0.;
        t_inters_loc = 0.;
    }
    
    ASIGNA_OPC(x_inters_opc, x_inters_loc);
    ASIGNA_OPC(y_inters_opc, y_inters_loc);
    ASIGNA_OPC(z_inters_opc, z_inters_loc);
    ASIGNA_OPC(t_inters_opc, t_inters_loc);
    
    return exists_intersection;
}

// ------------------------------------------------------------------------------------------

CYBOOL csmface_is_loop_contained_in_face(struct csmface_t *face, struct csmloop_t *loop)
{
    register struct csmhedge_t *iterator;
    unsigned long num_iteraciones;
    
    assert_no_null(face);
    assert(face->flout != loop);
    
    iterator = csmloop_ledge(loop);
    num_iteraciones = 0;
    
    do
    {
        struct csmvertex_t *vertex;
        double x, y, z;
        
        assert(num_iteraciones < 100000);
        num_iteraciones++;
        
        vertex = csmhedge_vertex(iterator);
        csmvertex_get_coordenadas(vertex, &x, &y, &z);

        if (csmloop_is_point_inside_loop(
                    face->flout,
                    x, y, z, face->dropped_coord,
                    face->fuzzy_epsilon,
                    NULL, NULL, NULL) == FALSO)
        {
            return FALSO;
        }
        
        iterator = csmhedge_next(iterator);
        
    } while (iterator != csmloop_ledge(loop));
    
    return CIERTO;
}

// ------------------------------------------------------------------------------------------

double csmface_tolerace(const struct csmface_t *face)
{
    assert_no_null(face);
    return face->fuzzy_epsilon;
}

// ------------------------------------------------------------------------------------------

CYBOOL csmface_is_coplanar_to_plane(
                        const struct csmface_t *face,
                        double A, double B, double C, double D,
                        double tolerance,
                        CYBOOL *same_orientation_opt)
{
    CYBOOL is_coplanar;
    CYBOOL same_orientation_loc;
    double Wx, Wy, Wz;
    double dot;
    
    assert_no_null(face);
    
    csmmath_cross_product3D(face->A, face->B, face->C, A, B, C, &Wx, &Wy, &Wz);
    dot = csmmath_dot_product3D(Wx, Wy, Wz, Wx, Wy, Wz);
    
    if (csmmath_compare_doubles(dot, 0., tolerance * tolerance) == CSMMATH_EQUAL_VALUES)
    {
        is_coplanar = CIERTO;
        
        dot = csmmath_dot_product3D(face->A, face->B, face->C, A, B, C);

        if (csmmath_compare_doubles(dot, 0., tolerance) == CSMMATH_VALUE1_GREATER_THAN_VALUE2)
            same_orientation_loc = CIERTO;
        else
            same_orientation_loc = FALSO;
    }
    else
    {
        is_coplanar = FALSO;
        same_orientation_loc = FALSO;
    }
    
    ASIGNA_OPC(same_orientation_opt, same_orientation_loc);
    
    return is_coplanar;
}

// ------------------------------------------------------------------------------------------

CYBOOL csmface_are_coplanar_faces(struct csmface_t *face1, const struct csmface_t *face2)
{
    double Xo1, Yo1, Zo1, Ux1, Uy1, Uz1, Vx1, Vy1, Vz1;
    double Xo2, Yo2, Zo2, Ux2, Uy2, Uz2, Vx2, Vy2, Vz2;
    double d1, d2;
    
    assert_no_null(face1);
    assert_no_null(face2);
    
    csmmath_plane_axis_from_implicit_plane_equation(
						face1->A, face1->B, face1->C, face1->D,
                        &Xo1, &Yo1, &Zo1,
                        &Ux1, &Uy1, &Uz1, &Vx1, &Vy1, &Vz1);
    
    csmmath_plane_axis_from_implicit_plane_equation(
						face2->A, face2->B, face2->C, face2->D,
                        &Xo2, &Yo2, &Zo2,
                        &Ux2, &Uy2, &Uz2, &Vx2, &Vy2, &Vz2);
    
    d1 = csmmath_signed_distance_point_to_plane(Xo1, Yo1, Zo1, face2->A, face2->B, face2->C, face2->D);
    d2 = csmmath_signed_distance_point_to_plane(Xo2, Yo2, Zo2, face1->A, face1->B, face1->C, face1->D);
    
    if (fabs(d1) > face2->fuzzy_epsilon || fabs(d2) > face1->fuzzy_epsilon)
    {
        return FALSO;
    }
    else
    {
        double dot;
        
        dot = csmmath_dot_product3D(face1->A, face1->B, face1->C, face2->A, face2->B, face2->C);
        
        if (fabs(1. - fabs(dot)) > 1.e-5)
            return FALSO;
        else
            return CIERTO;
    }
}

// ------------------------------------------------------------------------------------------

CYBOOL csmface_faces_define_border_edge(struct csmface_t *face1, const struct csmface_t *face2)
{
    double dot;
    
    assert_no_null(face1);
    assert_no_null(face2);
    
    dot = csmmath_dot_product3D(face1->A, face1->B, face1->C, face2->A, face2->B, face2->C);
    
    if (fabs(dot) > i_COS_15_DEGREES)
        return FALSO;
    else
        return CIERTO;
}

// ------------------------------------------------------------------------------------------

CYBOOL csmface_is_oriented_in_direction(const struct csmface_t *face, double Wx, double Wy, double Wz)
{
    double dot_product;
    
    assert_no_null(face);
    
    dot_product = csmmath_dot_product3D(face->A, face->B, face->C, Wx, Wy, Wz);
    return ES_CIERTO(dot_product > 0.);
}

// ------------------------------------------------------------------------------------------

void csmface_face_equation(
                        const struct csmface_t *face,
                        double *A, double *B, double *C, double *D)
{
    assert_no_null(face);
    assert_no_null(A);
    assert_no_null(B);
    assert_no_null(C);
    assert_no_null(D);
    assert(fabs(face->A) > 0. || fabs(face->B) > 0. || fabs(face->C) > 0.);
    
    *A = face->A;
    *B = face->B;
    *C = face->C;
    *D = face->D;
}

// ------------------------------------------------------------------------------------------

void csmface_face_equation_info(
                        const struct csmface_t *face,
                        double *A, double *B, double *C, double *D)
{
    assert_no_null(face);
    assert_no_null(A);
    assert_no_null(B);
    assert_no_null(C);
    assert_no_null(D);
    
    *A = face->A;
    *B = face->B;
    *C = face->C;
    *D = face->D;
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

struct csmsolid_t *csmface_fsolid_aux(struct csmface_t *face)
{
    assert_no_null(face);
    return face->fsolid_aux;
}

// ------------------------------------------------------------------------------------------

void csmface_set_fsolid_aux(struct csmface_t *face, struct csmsolid_t *solid)
{
    assert_no_null(face);
    face->fsolid_aux = solid;
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

// ----------------------------------------------------------------------------------------------------

void csmface_revert(struct csmface_t *face)
{
    struct csmloop_t *loop_iterator;
    
    assert_no_null(face);
        
    loop_iterator = face->floops;
    
    while (loop_iterator != NULL)
    {
        csmloop_revert_loop_orientation(loop_iterator);
        loop_iterator = csmloop_next(loop_iterator);
    }
}

// ----------------------------------------------------------------------------------------------------

void csmface_clear_algorithm_mask(struct csmface_t *face)
{
    struct csmloop_t *loop_iterator;
    
    assert_no_null(face);
        
    loop_iterator = face->floops;
    
    while (loop_iterator != NULL)
    {
        csmloop_clear_algorithm_mask(loop_iterator);
        loop_iterator = csmloop_next(loop_iterator);
    }
}

// ----------------------------------------------------------------------------------------------------

void csmface_print_info_debug(struct csmface_t *face, CYBOOL assert_si_no_es_integro, unsigned long *num_holes_opc)
{
    unsigned long num_holes_loc;
    struct csmloop_t *loop_iterator;
    double A, B, C, D;
    
    csmface_face_equation_info(face, &A, &B, &C, &D);
    csmdebug_print_debug_info("\tFace %lu (%g, %g, %g, %g)\n", face->id, A, B, C, D);
    
    loop_iterator = csmface_floops(face);
    num_holes_loc = 0;
    
    while (loop_iterator != NULL)
    {
        struct csmloop_t *next_loop;
        CYBOOL is_outer_loop;
        
        is_outer_loop = ES_CIERTO(csmface_flout(face) == loop_iterator);
        csmloop_print_info_debug(loop_iterator, is_outer_loop, assert_si_no_es_integro);
        
        if (is_outer_loop == FALSO)
            num_holes_loc++;
        
        next_loop = csmloop_next(loop_iterator);
        
        if (assert_si_no_es_integro == CIERTO)
        {
            assert(csmloop_lface(loop_iterator) == face);
            
            if (next_loop != NULL)
                assert(csmloop_prev(next_loop) == loop_iterator);
        }
        
        loop_iterator = next_loop;
    }
    
    ASIGNA_OPC(num_holes_opc, num_holes_loc);
}

// ----------------------------------------------------------------------------------------------------

#include <basicGraphics/bsgraphics2.h>

void csmface_draw_solid(
                    struct csmface_t *face,
                    CYBOOL draw_solid_face,
                    CYBOOL draw_face_normal,
                    const struct bsmaterial_t *face_material,
                    const struct bsmaterial_t *normal_material,
                    struct bsgraphics2_t *graphics)
{
    assert_no_null(face);
    
    if (draw_solid_face == CIERTO)
    {
        double Xo, Yo, Zo, Ux, Uy, Uz, Vx, Vy, Vz;
        struct csmloop_t *loop_iterator;
        struct gccontorno_t *shape;
    
        csmmath_plane_axis_from_implicit_plane_equation(
						face->A, face->B, face->C, face->D,
                        &Xo, &Yo, &Zo,
                        &Ux, &Uy, &Uz, &Vx, &Vy, &Vz);
    
        loop_iterator = csmface_floops(face);
        shape = gccontorno_crea_vacio();
        
        while (loop_iterator != NULL)
        {
            csmloop_append_loop_to_shape(loop_iterator, Xo, Yo, Zo, Ux, Uy, Uz, Vx, Vy, Vz, shape);
            loop_iterator = csmloop_next(loop_iterator);
        }
        
        if (gccontorno_num_poligonos(shape) > 0)
        {
            bsgraphics2_escr_color(graphics, face_material);
            gccontorno_dibuja_3d_ex(shape, Xo, Yo, Zo, Ux, Uy, Uz, Vx, Vy, Vz, FALSO, graphics);
        }
        
        gccontorno_destruye(&shape);
    }

    if (draw_face_normal == CIERTO)
    {
        double x_geometric_center, y_geometric_center, z_geometric_center;
        double disp;
        
        bsgraphics2_escr_color(graphics, normal_material);
        csmloop_geometric_center_3d(face->flout, &x_geometric_center, &y_geometric_center, &z_geometric_center);
    
        disp = 0.01;
        bsgraphics2_escr_linea3D(
                        graphics,
                        x_geometric_center, y_geometric_center, z_geometric_center,
                        x_geometric_center + disp * face->A, y_geometric_center + disp * face->B, z_geometric_center + disp * face->C);
    }
}

// ----------------------------------------------------------------------------------------------------

void csmface_draw_normal(struct csmface_t *face, struct bsgraphics2_t *graphics)
{
    double x_geometric_center, y_geometric_center, z_geometric_center;
    double disp;

    assert_no_null(face);
    
    csmloop_geometric_center_3d(face->flout, &x_geometric_center, &y_geometric_center, &z_geometric_center);
    
    disp = 0.01;
    bsgraphics2_escr_linea3D(
                        graphics,
                        x_geometric_center, y_geometric_center, z_geometric_center,
                        x_geometric_center + disp * face->A, y_geometric_center + disp * face->B, z_geometric_center + disp * face->C);
}



