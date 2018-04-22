// Face...

#include "csmface.inl"
#include "csmface.tli"

#include "csmbbox.inl"
#include "csmloop.inl"
#include "csmhedge.inl"
#include "csmmath.inl"
#include "csmmath.tli"
#include "csmmaterial.inl"
#include "csmnode.inl"
#include "csmtolerance.inl"
#include "csmvertex.inl"
#include "csmid.inl"
#include "csmsurface.inl"

#ifdef __STANDALONE_DISTRIBUTABLE
#include "csmassert.inl"
#include "csmmem.inl"
#else
#include "cyassert.h"
#include "cypespy.h"
#endif

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmface_t *, i_crea, (
                        unsigned long id,
                        struct csmsolid_t *fsolid, struct csmsolid_t *fsolid_aux,
                        struct csmloop_t *flout,
                        struct csmloop_t *floops,
                        struct csmsurface_t **surface_eq,
                        struct csmmaterial_t **visz_material_opt,
                        double A, double B, double C, double D, double fuzzy_epsilon, enum csmmath_dropped_coord_t dropped_coord,
                        double x_center, double y_center, double z_center,
                        struct csmbbox_t **bbox,
                        CSMBOOL setop_is_null_face))
{
    struct csmface_t *face;
    
    face = MALLOC(struct csmface_t);
    
    face->id = id;
    
    face->fsolid = fsolid;
    face->fsolid_aux = fsolid_aux;
    
    face->flout = flout;
    face->floops = floops;
    
    face->surface_eq = ASIGNA_PUNTERO_PP_NO_NULL(surface_eq, struct csmsurface_t);
    face->visz_material_opt = ASIGNA_PUNTERO_PP(visz_material_opt, struct csmmaterial_t);
    
    face->A = A;
    face->B = B;
    face->C = C;
    face->D = D;
    face->fuzzy_epsilon = fuzzy_epsilon;
    face->dropped_coord = dropped_coord;
    face->x_center = x_center;
    face->y_center = y_center;
    face->z_center = z_center;
    
    face->bbox = ASIGNA_PUNTERO_PP_NO_NULL(bbox, struct csmbbox_t);
 
    face->setop_is_null_face = setop_is_null_face;
    
    return face;
}

// ------------------------------------------------------------------------------------------

struct csmface_t *csmface_new(struct csmsolid_t *solido, unsigned long *id_nuevo_elemento)
{
    unsigned long id;
    struct csmsolid_t *fsolid, *fsolid_aux;
    struct csmloop_t *flout;
    struct csmloop_t *floops;
    struct csmsurface_t *surface_eq;
    struct csmmaterial_t *visz_material_opt;
    double A, B, C, D, fuzzy_epsilon;
    enum csmmath_dropped_coord_t dropped_coord;
    double x_center, y_center, z_center;
    struct csmbbox_t *bbox;
    CSMBOOL setop_is_null_face;
    
    id = csmid_new_id(id_nuevo_elemento, NULL);

    fsolid = solido;
    fsolid_aux = NULL;
    flout = NULL;
    floops = NULL;
    
    surface_eq = csmsurface_new_undefined();
    visz_material_opt = NULL;
    
    A = 0.;
    B = 0.;
    C = 0.;
    D = 0.;
    fuzzy_epsilon = csmtolerance_default_point_on_plane();
    dropped_coord = (enum csmmath_dropped_coord_t)USHRT_MAX;
    x_center = 0.;
    y_center = 0.;
    z_center = 0.;
    
    bbox = csmbbox_create_empty_box();

    setop_is_null_face = CSMFALSE;
    
    return i_crea(
                id,
                fsolid,
                fsolid_aux,
                flout,
                floops,
                &surface_eq,
                &visz_material_opt,
                A, B, C, D,
                fuzzy_epsilon,
                dropped_coord,
                x_center, y_center, z_center,
                &bbox,
                setop_is_null_face);
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmface_t *, i_duplicate_face, (
                        struct csmsolid_t *fsolid,
                        struct csmsurface_t **surface_eq,
	                    struct csmmaterial_t **visz_material_opt,
                        double A, double B, double C, double D, double fuzzy_epsilon, enum csmmath_dropped_coord_t dropped_coord,
                        double x_center, double y_center, double z_center,
                        unsigned long *id_nuevo_elemento))
{
    unsigned long id;
    struct csmloop_t *flout, *floops;
    struct csmbbox_t *bbox;
    CSMBOOL setop_is_null_face;
    
    id = csmid_new_id(id_nuevo_elemento, NULL);
    flout = NULL;
    floops = NULL;
    bbox = csmbbox_create_empty_box();
    setop_is_null_face = CSMFALSE;
    
    return i_crea(
                id,
                fsolid,
                NULL,
                flout,
                floops,
                surface_eq,
                visz_material_opt,
                A, B, C, D,
                fuzzy_epsilon,
                dropped_coord,
                x_center, y_center, z_center,
                &bbox,
                setop_is_null_face);
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
    struct csmsurface_t *surface_eq;
    struct csmmaterial_t *visz_material_opt;
    struct csmloop_t *iterator, *last_loop;
    
    assert_no_null(face);
    
    surface_eq = csmsurface_copy(face->surface_eq);
    
    if (face->visz_material_opt != NULL)
        visz_material_opt = csmmaterial_copy(face->visz_material_opt);
    else
        visz_material_opt = NULL;

    new_face = i_duplicate_face(
                        fsolid,
                        &surface_eq,
                        &visz_material_opt,
                        face->A, face->B, face->C, face->D, face->fuzzy_epsilon, face->dropped_coord,
                        face->x_center, face->y_center, face->z_center,
                        id_nuevo_elemento);
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

void csmface_free(struct csmface_t **face)
{
    assert_no_null(face);
    assert_no_null(*face);

    if ((*face)->floops != NULL)
        csmnode_free_node_list(&(*face)->floops, csmloop_t);

    csmsurface_free(&(*face)->surface_eq);
    
    if ((*face)->visz_material_opt != NULL)
        csmmaterial_free(&(*face)->visz_material_opt);
    
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
    
    face->id = csmid_new_id(id_nuevo_elemento, new_id_opc);
    
    iterator = face->floops;
    num_iters = 0;
    
    do
    {
        assert(num_iters < 100000);
        num_iters++;

        csmloop_reassign_id(iterator, id_nuevo_elemento, NULL);
        iterator = csmloop_next(iterator);
        
    } while (iterator != NULL);
}

// ----------------------------------------------------------------------------------------------------

static void i_set_visualization_material(const struct csmmaterial_t *visz_material_opt, struct csmmaterial_t **face_visz_material_opt)
{
    assert_no_null(face_visz_material_opt);
    
    if (*face_visz_material_opt != NULL)
        csmmaterial_free(face_visz_material_opt);
    
    if (visz_material_opt != NULL)
        *face_visz_material_opt = csmmaterial_copy(visz_material_opt);
}

// ----------------------------------------------------------------------------------------------------

void csmface_set_visualization_material(struct csmface_t *face, const struct csmmaterial_t *visz_material_opt)
{
    assert_no_null(face);
    i_set_visualization_material(visz_material_opt, &face->visz_material_opt);
}

// ------------------------------------------------------------------------------------------

static void i_set_surface_eq(struct csmsurface_t **surface_eq_solid, const struct csmsurface_t *surface_eq)
{
    assert_no_null(surface_eq_solid);
    
    csmsurface_free(surface_eq_solid);
    *surface_eq_solid = csmsurface_copy(surface_eq);
}

// ------------------------------------------------------------------------------------------

void csmface_set_surface_eq(struct csmface_t *face, const struct csmsurface_t *surface_eq)
{
    assert_no_null(face);
    i_set_surface_eq(&face->surface_eq, surface_eq);
}

// ----------------------------------------------------------------------------------------------------

void csmface_copy_attributtes_from_face1(const struct csmface_t *face1, struct csmface_t *face2)
{
    assert_no_null(face1);
    assert_no_null(face2);
    
    i_set_surface_eq(&face2->surface_eq, face1->surface_eq);
    i_set_visualization_material(face1->visz_material_opt, &face2->visz_material_opt);
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
    
    csmbbox_compute_bsphere_and_margins(bbox);
}

// ----------------------------------------------------------------------------------------------------

static double i_compute_fuzzy_epsilon_for_containing_test(double A, double B, double C, double D, double max_tolerable_distance, struct csmloop_t *floops)
{
    register struct csmloop_t *iterator;
    unsigned long num_iters;
    double max_distance_to_plane;
    double tolerance_point_on_plane;
    
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
        
        max_distance_to_plane = CSMMATH_MAX(max_distance_to_plane, distance);
        
        iterator = csmloop_next(iterator);
        
    } while (iterator != NULL);
    
    tolerance_point_on_plane = csmtolerance_default_point_on_plane();
    return CSMMATH_MAX(1.01 * max_distance_to_plane, tolerance_point_on_plane);
}

// ----------------------------------------------------------------------------------------------------

void csmface_redo_geometric_generated_data(struct csmface_t *face)
{
    double max_tolerable_distance;
    
    assert_no_null(face);
    assert_no_null(face->flout);
    assert_no_null(face->floops);
    
    csmloop_face_equation(
                        face->flout,
                        &face->A, &face->B, &face->C, &face->D,
                        &face->x_center, &face->y_center, &face->z_center);
    
    max_tolerable_distance = 1.1 * csmloop_max_distance_to_plane(face->flout, face->A, face->B, face->C, face->D);
    face->fuzzy_epsilon = i_compute_fuzzy_epsilon_for_containing_test(face->A, face->B, face->C, face->D, max_tolerable_distance, face->floops);
    face->dropped_coord = csmmath_dropped_coord(face->A, face->B, face->C);
    
    i_compute_bounding_box(face->floops, face->bbox);
}

// ------------------------------------------------------------------------------------------

void csmface_face_baricenter(const struct csmface_t *face, double *x, double *y, double *z)
{
    assert_no_null(face);
    assert_no_null(x);
    assert_no_null(y);
    assert_no_null(z);
    
    *x = face->x_center;
    *y = face->y_center;
    *z = face->z_center;
}

// ------------------------------------------------------------------------------------------

CSMBOOL csmface_should_analyze_intersections_between_faces(const struct csmface_t *face1, const struct csmface_t *face2)
{
    assert_no_null(face1);
    assert_no_null(face2);
    
    return csmbbox_intersects_with_other_bbox(face1->bbox, face2->bbox);
}

// ------------------------------------------------------------------------------------------

CSMBOOL csmface_should_analyze_intersections_with_segment(
                        const struct csmface_t *face,
                        double x1, double y1, double z1, double x2, double y2, double z2)
{
    assert_no_null(face);
    return csmbbox_intersects_with_segment(face->bbox, x1, y1, z1, x2, y2, z2);
}

// ------------------------------------------------------------------------------------------

static CSMBOOL i_is_point_on_face_plane(
                        double x, double y, double z,
                        double A, double B, double C, double D,
                        double fuzzy_epsilon,
                        const struct csmbbox_t *bbox)
{
    if (csmbbox_contains_point(bbox, x, y, z) == CSMFALSE)
    {
        return CSMFALSE;
    }
    else
    {
        double distance;
        
        distance = csmmath_signed_distance_point_to_plane(x, y, z, A, B, C, D);
        
        if (csmmath_compare_doubles(distance, 0., fuzzy_epsilon) == CSMCOMPARE_EQUAL)
            return CSMTRUE;
        else
            return CSMFALSE;
    }
}

// ------------------------------------------------------------------------------------------

CSMBOOL csmface_contains_vertex(
                        const struct csmface_t *face,
                        const struct csmvertex_t *vertex,
                        const struct csmtolerance_t *tolerances,
                        enum csmmath_contaiment_point_loop_t *type_of_containment_opc,
                        struct csmvertex_t **hit_vertex_opc,
                        struct csmhedge_t **hit_hedge_opc, double *t_relative_to_hit_hedge_opc)
{
    double x, y, z;
    
    assert_no_null(face);
    
    csmvertex_get_coordenadas(vertex, &x, &y, &z);
    
    return csmface_contains_point(
                        face,
                        x, y, z,
                        tolerances,
                        type_of_containment_opc,
                        hit_vertex_opc,
                        hit_hedge_opc, t_relative_to_hit_hedge_opc);
}

// ------------------------------------------------------------------------------------------

CSMBOOL csmface_contains_point(
                        const struct csmface_t *face,
                        double x, double y, double z,
                        const struct csmtolerance_t *tolerances,
                        enum csmmath_contaiment_point_loop_t *type_of_containment_opc,
                        struct csmvertex_t **hit_vertex_opc,
                        struct csmhedge_t **hit_hedge_opc, double *t_relative_to_hit_hedge_opc)
{
    CSMBOOL containts_point;
    enum csmmath_contaiment_point_loop_t type_of_containment_loc;
    struct csmvertex_t *hit_vertex_loc;
    struct csmhedge_t *hit_hedge_loc;
    double t_relative_to_hit_hedge_loc;
    
    assert_no_null(face);
    
    if (i_is_point_on_face_plane(
                        x, y, z,
                        face->A, face->B, face->C, face->D,
                        face->fuzzy_epsilon,
                        face->bbox) == CSMFALSE)
    {
        containts_point = CSMFALSE;
        
        type_of_containment_loc = (enum csmmath_contaiment_point_loop_t)USHRT_MAX;
        hit_vertex_loc = NULL;
        hit_hedge_loc = NULL;
        t_relative_to_hit_hedge_loc = 0.;
    }
    else
    {
        if (csmloop_is_point_inside_loop(
                        face->flout,
                        x, y, z, face->dropped_coord,
                        tolerances,
                        &type_of_containment_loc, &hit_vertex_loc, &hit_hedge_loc, &t_relative_to_hit_hedge_loc) == CSMFALSE)
        {
            containts_point = CSMFALSE;
        }
        else
        {
            if (type_of_containment_loc != CSMMATH_CONTAIMENT_POINT_LOOP_INTERIOR)
            {
                containts_point = CSMTRUE;
            }
            else
            {
                struct csmloop_t *loop_iterator;
                
                loop_iterator = face->floops;
                containts_point = CSMTRUE;
                
                while (loop_iterator != NULL)
                {
                    if (loop_iterator != face->flout && csmloop_has_only_a_null_edge(loop_iterator) == CSMFALSE)
                    {
                        enum csmmath_contaiment_point_loop_t type_of_containment_hole;
                        struct csmvertex_t *hit_vertex_hole;
                        struct csmhedge_t *hit_hedge_hole;
                        double t_relative_to_hit_hedge_hole;
                        
                        if (csmloop_is_point_inside_loop(
                                loop_iterator,
                                x, y, z, face->dropped_coord,
                                tolerances,
                                &type_of_containment_hole, &hit_vertex_hole, &hit_hedge_hole, &t_relative_to_hit_hedge_hole) == CSMTRUE)
                        {
                            if (type_of_containment_hole == CSMMATH_CONTAIMENT_POINT_LOOP_INTERIOR)
                            {
                                containts_point = CSMFALSE;
                            }
                            else
                            {
                                containts_point = CSMTRUE;
                                
                                type_of_containment_loc = type_of_containment_hole;
                                hit_vertex_loc = hit_vertex_hole;
                                hit_hedge_loc = hit_hedge_hole;
                                t_relative_to_hit_hedge_loc = t_relative_to_hit_hedge_hole;
                            }
                            break;
                        }
                    }
                
                    loop_iterator = csmloop_next(loop_iterator);
                }
            }
        }
    }

    ASSIGN_OPTIONAL_VALUE(type_of_containment_opc, type_of_containment_loc);
    ASSIGN_OPTIONAL_VALUE(hit_vertex_opc, hit_vertex_loc);
    ASSIGN_OPTIONAL_VALUE(hit_hedge_opc, hit_hedge_loc);
    ASSIGN_OPTIONAL_VALUE(t_relative_to_hit_hedge_opc, t_relative_to_hit_hedge_loc);
    
    return containts_point;
}

// ------------------------------------------------------------------------------------------

CSMBOOL csmface_is_point_interior_to_face(
                        const struct csmface_t *face,
                        double x, double y, double z,
                        const struct csmtolerance_t *tolerances)
{
    CSMBOOL is_interior_to_face;
    
    assert_no_null(face);
    
    if (i_is_point_on_face_plane(
                        x, y, z,
                        face->A, face->B, face->C, face->D,
                        face->fuzzy_epsilon,
                        face->bbox) == CSMFALSE)
    {
        return CSMFALSE;
    }
    else
    {
        enum csmmath_contaiment_point_loop_t type_of_containment;
    
        if (csmloop_is_point_inside_loop(
                        face->flout,
                        x, y, z, face->dropped_coord,
                        tolerances,
                        &type_of_containment, NULL, NULL, NULL) == CSMFALSE)
        {
            is_interior_to_face = CSMFALSE;
        }
        else if (type_of_containment != CSMMATH_CONTAIMENT_POINT_LOOP_INTERIOR)
        {
            is_interior_to_face = CSMTRUE;
        }
        else
        {
            struct csmloop_t *loop_iterator;
            
            loop_iterator = face->floops;
            is_interior_to_face = CSMTRUE;
            
            while (loop_iterator != NULL)
            {
                if (loop_iterator != face->flout && csmloop_has_only_a_null_edge(loop_iterator) == CSMFALSE)
                {
                    if (csmloop_is_point_inside_loop(
                            loop_iterator,
                            x, y, z, face->dropped_coord,
                            tolerances,
                            &type_of_containment, NULL, NULL, NULL) == CSMTRUE)
                    {
                        if (type_of_containment == CSMMATH_CONTAIMENT_POINT_LOOP_INTERIOR)
                            is_interior_to_face = CSMFALSE;
                        else
                            is_interior_to_face = CSMTRUE;
                        
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

CSMBOOL csmface_is_vertex_used_by_hedge_on_face(const struct csmface_t *face, const struct csmvertex_t *vertex)
{
    struct csmloop_t *loop_iterator;
    
    assert_no_null(face);
    
    loop_iterator = face->floops;
    
    while (loop_iterator != NULL)
    {
        if (csmloop_is_vertex_used_by_hedge_on_loop(loop_iterator, vertex) == CSMTRUE)
            return CSMTRUE;
    
        loop_iterator = csmloop_next(loop_iterator);
    }
    
    return CSMFALSE;
}

// ------------------------------------------------------------------------------------------

enum csmcompare_t csmface_classify_vertex_relative_to_face(const struct csmface_t *face, const struct csmvertex_t *vertex)
{
    double x, y, z;
    double dist;
    
    assert_no_null(face);
    
    csmvertex_get_coordenadas(vertex, &x, &y, &z);
    dist = csmmath_signed_distance_point_to_plane(x, y, z, face->A, face->B, face->C, face->D);
    
    return csmmath_compare_doubles(dist, 0., face->fuzzy_epsilon);
}

// ------------------------------------------------------------------------------------------

CSMBOOL csmface_exists_intersection_between_line_and_face_plane(
                        const struct csmface_t *face,
                        double x1, double y1, double z1, double x2, double y2, double z2,
                        double *x_inters_opc, double *y_inters_opc, double *z_inters_opc, double *t_inters_opc)
{
    CSMBOOL exists_intersection;
    double x_inters_loc, y_inters_loc, z_inters_loc, t_inters_loc;
    double dist1, dist2;
    enum csmcompare_t relation1, relation2;
    
    assert_no_null(face);
    
    dist1 = csmmath_signed_distance_point_to_plane(x1, y1, z1, face->A, face->B, face->C, face->D);
    relation1 = csmmath_compare_doubles(dist1, 0., face->fuzzy_epsilon);
    
    dist2 = csmmath_signed_distance_point_to_plane(x2, y2, z2, face->A, face->B, face->C, face->D);
    relation2 = csmmath_compare_doubles(dist2, 0., face->fuzzy_epsilon);
    
    if ((relation1 == CSMCOMPARE_FIRST_LESS && relation2 == CSMCOMPARE_FIRST_GREATER)
            || (relation1 == CSMCOMPARE_FIRST_GREATER && relation2 == CSMCOMPARE_FIRST_LESS))
    {
        exists_intersection = CSMTRUE;
        
        t_inters_loc = dist1 / (dist1 - dist2);
            
        x_inters_loc = x1 + t_inters_loc * (x2 - x1);
        y_inters_loc = y1 + t_inters_loc * (y2 - y1);
        z_inters_loc = z1 + t_inters_loc * (z2 - z1);
    }
    else if (relation1 == CSMCOMPARE_EQUAL && relation2 != CSMCOMPARE_EQUAL)
    {
        exists_intersection = CSMTRUE;
        
        t_inters_loc = 0.;
            
        x_inters_loc = x1;
        y_inters_loc = y1;
        z_inters_loc = z1;
    }
    else if (relation1 != CSMCOMPARE_EQUAL && relation2 == CSMCOMPARE_EQUAL)
    {
        exists_intersection = CSMTRUE;
        
        t_inters_loc = 1.;
            
        x_inters_loc = x2;
        y_inters_loc = y2;
        z_inters_loc = z2;
    }
    else
    {
        exists_intersection = CSMFALSE;
        
        x_inters_loc = 0.;
        y_inters_loc = 0.;
        z_inters_loc = 0.;
        t_inters_loc = 0.;
    }
    
    ASSIGN_OPTIONAL_VALUE(x_inters_opc, x_inters_loc);
    ASSIGN_OPTIONAL_VALUE(y_inters_opc, y_inters_loc);
    ASSIGN_OPTIONAL_VALUE(z_inters_opc, z_inters_loc);
    ASSIGN_OPTIONAL_VALUE(t_inters_opc, t_inters_loc);
    
    return exists_intersection;
}

// ------------------------------------------------------------------------------------------

CSMBOOL csmface_is_loop_contained_in_face(
                        struct csmface_t *face,
                        struct csmloop_t *loop,
                        const struct csmtolerance_t *tolerances)
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

        if (csmface_is_point_interior_to_face(face, x, y, z, tolerances) == CSMFALSE)
        {
            return CSMFALSE;
        }
        
        iterator = csmhedge_next(iterator);
        
    } while (iterator != csmloop_ledge(loop));
    
    return CSMTRUE;
}

// ------------------------------------------------------------------------------------------

CSMBOOL csmface_is_point_interior_to_face_outer_loop(
                        const struct csmface_t *face,
                        double x, double y, double z,
                        const struct csmtolerance_t *tolerances)
{
    assert_no_null(face);
    
    if (i_is_point_on_face_plane(
                        x, y, z,
                        face->A, face->B, face->C, face->D,
                        face->fuzzy_epsilon,
                        face->bbox) == CSMFALSE)
    {
        return CSMFALSE;
    }
    else
    {
        enum csmmath_contaiment_point_loop_t type_of_containment;
    
        if (csmloop_is_point_inside_loop(
                        face->flout,
                        x, y, z, face->dropped_coord,
                        tolerances,
                        &type_of_containment, NULL, NULL, NULL) == CSMFALSE)
        {
            return CSMFALSE;
        }
        else
        {
            return CSMTRUE;
        }
    }
}

// ------------------------------------------------------------------------------------------

CSMBOOL csmface_is_loop_contained_in_face_outer_loop(
                        struct csmface_t *face,
                        struct csmloop_t *loop,
                        const struct csmtolerance_t *tolerances)
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

        if (csmface_is_point_interior_to_face_outer_loop(face, x, y, z, tolerances) == CSMFALSE)
        {
            return CSMFALSE;
        }
        
        iterator = csmhedge_next(iterator);
        
    } while (iterator != csmloop_ledge(loop));
    
    return CSMTRUE;
}

// ------------------------------------------------------------------------------------------

double csmface_tolerace(const struct csmface_t *face)
{
    assert_no_null(face);
    return face->fuzzy_epsilon;
}

// ------------------------------------------------------------------------------------------

CSMBOOL csmface_is_coplanar_to_plane(
                        const struct csmface_t *face,
                        double A, double B, double C, double D,
                        const struct csmtolerance_t *tolerances,
                        CSMBOOL *same_orientation_opt)
{
    CSMBOOL is_coplanar;
    CSMBOOL same_orientation_loc;
    
    assert_no_null(face);
    
    if (csmmath_vectors_are_parallel(
                        face->A, face->B, face->C, A, B, C,
                        tolerances) == CSMTRUE)
    {
        double dot;
        double D_loc;

        dot = csmmath_dot_product3D(face->A, face->B, face->C, A, B, C);
        same_orientation_loc = (dot > 0.0) ? CSMTRUE: CSMFALSE;
        
        if (same_orientation_loc == CSMTRUE)
            D_loc = D;
        else
            D_loc = -D;
            
        if (csmmath_compare_doubles(face->D, D_loc, csmtolerance_point_on_plane(tolerances)) == CSMCOMPARE_EQUAL)
            is_coplanar = CSMTRUE;
        else
            is_coplanar = CSMFALSE;
    }
    else
    {
        is_coplanar = CSMFALSE;
        same_orientation_loc = CSMFALSE;
    }
    
    ASSIGN_OPTIONAL_VALUE(same_orientation_opt, same_orientation_loc);
    
    return is_coplanar;
}

// ------------------------------------------------------------------------------------------

CSMBOOL csmface_are_coplanar_faces(struct csmface_t *face1, const struct csmface_t *face2, const struct csmtolerance_t *tolerances, CSMBOOL *same_sense_opc)
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
    
    if (csmmath_fabs(d1) > face2->fuzzy_epsilon || csmmath_fabs(d2) > face1->fuzzy_epsilon)
    {
        return CSMFALSE;
    }
    else
    {
        return csmmath_unit_vectors_are_parallel_ex(
                        face1->A, face1->B, face1->C, face2->A, face2->B, face2->C,
                        tolerances,
                        same_sense_opc);
    }
}

// ------------------------------------------------------------------------------------------

CSMBOOL csmface_are_coplanar_faces_at_common_base_vertex(struct csmface_t *face1, const struct csmface_t *face2, const struct csmtolerance_t *tolerances)
{
    assert_no_null(face1);
    assert_no_null(face2);
    
    return csmmath_vectors_are_parallel(
                        face1->A, face1->B, face1->C, face2->A, face2->B, face2->C,
                        tolerances);
}

// ------------------------------------------------------------------------------------------

CSMBOOL csmface_faces_define_border_edge(const struct csmface_t *face1, const struct csmface_t *face2)
{
    assert_no_null(face1);
    assert_no_null(face2);
    
    return csmsurface_surfaces_define_border_edge(
                        face1->surface_eq, face1->A, face1->B, face1->C,
                        face2->surface_eq, face2->A, face2->B, face2->C);
}

// ------------------------------------------------------------------------------------------

CSMBOOL csmface_is_oriented_in_direction(const struct csmface_t *face, double Wx, double Wy, double Wz)
{
    double dot_product;
    
    assert_no_null(face);
    
    dot_product = csmmath_dot_product3D(face->A, face->B, face->C, Wx, Wy, Wz);
    return IS_TRUE(dot_product > 0.);
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
    assert(csmmath_fabs(face->A) > 0. || csmmath_fabs(face->B) > 0. || csmmath_fabs(face->C) > 0.);
    
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

const struct csmbbox_t *csmface_bbox(const struct csmface_t *face)
{
    assert_no_null(face);
    return face->bbox;
}

// ------------------------------------------------------------------------------------------

double csmface_loop_area_in_face(const struct csmface_t *face, const struct csmloop_t *loop)
{
    double Xo, Yo, Zo, Ux, Uy, Uz, Vx, Vy, Vz;
    
    assert_no_null(face);

    csmmath_plane_axis_from_implicit_plane_equation(
						face->A, face->B, face->C, face->D,
                        &Xo, &Yo, &Zo,
                        &Ux, &Uy, &Uz, &Vx, &Vy, &Vz);
    
    return csmloop_compute_area(loop, Xo, Yo, Zo, Ux, Uy, Uz, Vx, Vy, Vz);
}

// ----------------------------------------------------------------------------------------------------

void csmface_reorient_loops_in_face(struct csmface_t *face, const struct csmtolerance_t *tolerances)
{
    struct csmloop_t *iterator;

    assert_no_null(face);
    csmface_redo_geometric_generated_data(face);
    
    iterator = face->floops;
    
    while (iterator != NULL)
    {
        double A_loop, B_loop, C_loop, D_loop, xc_loop, yc_loop, zc_loop;
        CSMBOOL parallel, same_sense;

        csmloop_face_equation(iterator, &A_loop, &B_loop, &C_loop, &D_loop, &xc_loop, &yc_loop, &zc_loop);

        parallel = csmmath_unit_vectors_are_parallel_ex(face->A, face->B, face->C, A_loop, B_loop, C_loop, tolerances, &same_sense);
        assert(parallel == CSMTRUE);
        
        if (iterator == csmface_flout(face))
        {
            assert(same_sense == CSMTRUE);
        }
        else
        {
            if (same_sense == CSMTRUE)
                csmloop_revert_loop_orientation(iterator);
        }
        
        iterator = csmloop_next(iterator);
    }
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

CSMBOOL csmface_has_holes(const struct csmface_t *face)
{
    assert_no_null(face);
    
    if (face->floops != NULL && csmloop_next(face->floops) != NULL)
        return CSMTRUE;
    else
        return CSMFALSE;
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
    
    face->setop_is_null_face = CSMFALSE;
    
    loop_iterator = face->floops;
    
    while (loop_iterator != NULL)
    {
        csmloop_clear_algorithm_mask(loop_iterator);
        loop_iterator = csmloop_next(loop_iterator);
    }
}

// ----------------------------------------------------------------------------------------------------

CSMBOOL csmface_is_setop_null_face(struct csmface_t *face)
{
    assert_no_null(face);
    return face->setop_is_null_face;
}

// ----------------------------------------------------------------------------------------------------

void csmface_mark_setop_null_face(struct csmface_t *face)
{
    assert_no_null(face);
    face->setop_is_null_face = CSMTRUE;
}
