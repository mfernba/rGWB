// Loop...

#include "csmloop.inl"

#include "csmbbox.inl"
#include "csmdebug.inl"
#include "csmgeom.inl"
#include "csmmath.inl"
#include "csmmath.tli"
#include "csmnode.inl"
#include "csmedge.inl"
#include "csmedge.tli"
#include "csmhedge.inl"
#include "csmtolerance.inl"
#include "csmvertex.inl"

#include "a_pto2d.h"
#include "cont2d.h"
#include "cyassert.h"
#include "cypeid.h"
#include "cypespy.h"
#include "defmath.tlh"
#include "standarc.h"

struct csmloop_t
{
    struct csmnode_t clase_base;
    
    struct csmhedge_t *ledge;
    struct csmface_t *lface;
    
    CYBOOL setop_convert_loop_in_face;
    CYBOOL setop_loop_was_a_hole;
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
                        struct csmface_t *lface,
                        CYBOOL setop_convert_loop_in_face,
                        CYBOOL setop_loop_was_a_hole))
{
    struct csmloop_t *loop;
    
    loop = MALLOC(struct csmloop_t);
    
    loop->clase_base = csmnode_crea_node(id, loop, i_csmloop_destruye, csmloop_t);
    
    loop->ledge = ledge;
    loop->lface = lface;
    
    loop->setop_convert_loop_in_face = setop_convert_loop_in_face;
    loop->setop_loop_was_a_hole = setop_loop_was_a_hole;
    
    return loop;
}

// --------------------------------------------------------------------------------------------------------------

struct csmloop_t *csmloop_crea(struct csmface_t *face, unsigned long *id_nuevo_elemento)
{
    unsigned long id;
    struct csmhedge_t *ledge;
    struct csmface_t *lface;
    CYBOOL setop_convert_loop_in_face;
    CYBOOL setop_loop_was_a_hole;
    
    id = cypeid_nuevo_id(id_nuevo_elemento, NULL);
    
    ledge = NULL;
    lface = face;
    
    setop_convert_loop_in_face = FALSO;
    setop_loop_was_a_hole = FALSO;
    
    return i_crea(id, ledge, lface, setop_convert_loop_in_face, setop_loop_was_a_hole);
}

// --------------------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmloop_t *, i_duplicate_loop, (struct csmface_t *lface, unsigned long *id_nuevo_elemento))
{
    unsigned long id;
    struct csmhedge_t *ledge;
    CYBOOL setop_convert_loop_in_face;
    CYBOOL setop_loop_was_a_hole;
    
    id = cypeid_nuevo_id(id_nuevo_elemento, NULL);
    ledge = NULL;
    setop_convert_loop_in_face = FALSO;
    setop_loop_was_a_hole = FALSO;
    
    return i_crea(id, ledge, lface, setop_convert_loop_in_face, setop_loop_was_a_hole);
}

// --------------------------------------------------------------------------------------------------------------

struct csmloop_t *csmloop_duplicate(
                        const struct csmloop_t *loop,
                        struct csmface_t *lface,
                        unsigned long *id_nuevo_elemento,
                        struct csmhashtb(csmvertex_t) *relation_svertexs_old_to_new,
                        struct csmhashtb(csmhedge_t) *relation_shedges_old_to_new)
{
    struct csmloop_t *new_loop;
    register struct csmhedge_t *iterator, *last_hedge;
    unsigned long num_iteraciones;
    
    assert_no_null(loop);
    
    new_loop = i_duplicate_loop(lface, id_nuevo_elemento);
    assert_no_null(new_loop);
    
    iterator = loop->ledge;
    last_hedge = NULL;
    num_iteraciones = 0;
    
    do
    {
        struct csmhedge_t *iterator_copy;
        
        assert(num_iteraciones < 10000);
        num_iteraciones++;
        
        iterator_copy = csmhedge_duplicate(iterator, new_loop, id_nuevo_elemento, relation_svertexs_old_to_new, relation_shedges_old_to_new);
        
        if (new_loop->ledge == NULL)
            new_loop->ledge = iterator_copy;
        else
            csmnode_insert_node2_after_node1(last_hedge, iterator_copy, csmhedge_t);
        
        last_hedge = iterator_copy;
        iterator = csmhedge_next(iterator);
    }
    while (iterator != loop->ledge);

    assert(csmhedge_next(last_hedge) == NULL);
    csmhedge_set_next(last_hedge, new_loop->ledge);

    assert(csmhedge_prev(new_loop->ledge) == NULL);
    csmhedge_set_prev(new_loop->ledge, last_hedge);
    
    return new_loop;
}

// --------------------------------------------------------------------------------------------------------------

unsigned long csmloop_id(const struct csmloop_t *loop)
{
    assert_no_null(loop);
    return csmnode_id(CSMNODE(loop));
}

// --------------------------------------------------------------------------------------------------------------

void csmloop_reassign_id(struct csmloop_t *loop, unsigned long *id_new_element, unsigned long *new_id_opt)
{
    assert_no_null(loop);
    loop->clase_base.id = cypeid_nuevo_id(id_new_element, new_id_opt);
}

// --------------------------------------------------------------------------------------------------------------

void csmloop_face_equation(
                        const struct csmloop_t *loop,
                        double *A, double *B, double *C, double *D)
{
    double A_loc, B_loc, C_loc, D_loc;
    double xc, yc, zc;
    register struct csmhedge_t *iterator;
    unsigned long num_iteraciones;
    unsigned long num_vertexs;
    
    assert_no_null(loop);
    assert_no_null(A);
    assert_no_null(B);
    assert_no_null(C);
    assert_no_null(D);
    
    A_loc = 0.;
    B_loc = 0.;
    C_loc = 0.;
    D_loc = 0.;
    
    xc = 0.;
    yc = 0.;
    zc = 0.;
    
    num_vertexs = 0;
    
    iterator = loop->ledge;
    num_iteraciones = 0;
    
    do
    {
        struct csmhedge_t *next_hedge;
        struct csmvertex_t *vertex, *next_vertex;
        double x1, y1, z1, x2, y2, z2;
        
        assert(num_iteraciones < 10000);
        num_iteraciones++;
        
        next_hedge = csmhedge_next(iterator);
        
        vertex = csmhedge_vertex(iterator);
        csmvertex_get_coordenadas(vertex, &x1, &y1, &z1);
        
        next_vertex = csmhedge_vertex(next_hedge);
        csmvertex_get_coordenadas(next_vertex, &x2, &y2, &z2);
        
        A_loc += (y1 - y2) * (z1 + z2);
        B_loc += (z1 - z2) * (x1 + x2);
        C_loc += (x1 - x2) * (y1 + y2);
        
        xc += x1;
        yc += y1;
        zc += z1;
        
        num_vertexs++;
        
        iterator = next_hedge;
        
    } while (iterator != loop->ledge);
    
    //assert(num_vertexs >= 3);
    
    if (csmmath_is_null_vector(A_loc, B_loc, C_loc, csmtolerance_null_vector()) == CIERTO)
    {
        A_loc = 0.;
        B_loc = 0.;
        C_loc = 1.;
        D_loc = 0.;
    }
    else
    {
        csmmath_make_unit_vector3D(&A_loc, &B_loc, &C_loc);
        D_loc = -csmmath_dot_product3D(A_loc, B_loc, C_loc, xc, yc, zc) / num_vertexs;
    }
    
    *A = A_loc;
    *B = B_loc;
    *C = C_loc;
    *D = D_loc;
}

// --------------------------------------------------------------------------------------------------------------

void csmloop_update_bounding_box(const struct csmloop_t *loop, struct csmbbox_t *bbox)
{
    register struct csmhedge_t *iterator;
    unsigned long num_iteraciones;
    
    assert_no_null(loop);
    
    iterator = loop->ledge;
    num_iteraciones = 0;
    
    do
    {
        struct csmvertex_t *vertex;
        double x, y, z;
        
        assert(num_iteraciones < 10000);
        num_iteraciones++;
        
        vertex = csmhedge_vertex(iterator);
        csmvertex_get_coordenadas(vertex, &x, &y, &z);
        
        csmbbox_maximize_coord(bbox, x, y, z);
        
        iterator = csmhedge_next(iterator);
        
    } while (iterator != loop->ledge);
}

// --------------------------------------------------------------------------------------------------------------

double csmloop_max_distance_to_plane(
                        const struct csmloop_t *loop,
                        double A, double B, double C, double D)
{
    double max_distance_to_plane;
    register struct csmhedge_t *iterator;
    unsigned long num_iteraciones;
    
    assert_no_null(loop);
    
    iterator = loop->ledge;
    num_iteraciones = 0;
    
    max_distance_to_plane = 0.;
    
    do
    {
        struct csmvertex_t *vertex;
        double x, y, z;
        double distance;
        
        assert(num_iteraciones < 10000);
        num_iteraciones++;
        
        vertex = csmhedge_vertex(iterator);
        csmvertex_get_coordenadas(vertex, &x, &y, &z);
        
        distance = fabs(csmmath_signed_distance_point_to_plane(x, y, z, A, B, C, D));
        max_distance_to_plane = MAX(max_distance_to_plane, distance);

        iterator = csmhedge_next(iterator);
        
    } while (iterator != loop->ledge);
    
    return MAX(max_distance_to_plane, csmtolerance_coplanarity());
}

// --------------------------------------------------------------------------------------------------------------

static CYBOOL i_is_point_on_loop_boundary(
                        struct csmhedge_t *ledge,
                        double x, double y, double z, double tolerance,
                        struct csmvertex_t **hit_vertex_opc,
                        struct csmhedge_t **hit_hedge_opc, double *t_relative_to_hit_hedge_opc)
{
    CYBOOL is_point_on_loop_boundary;
    struct csmvertex_t *hit_vertex_loc;
    struct csmhedge_t *hit_hedge_loc;
    double t_relative_to_hit_hedge_loc;
    register struct csmhedge_t *iterator;
    unsigned long num_iteraciones;
    
    iterator = ledge;
    num_iteraciones = 0;
    
    is_point_on_loop_boundary = FALSO;
    hit_vertex_loc = NULL;
    hit_hedge_loc = NULL;
    t_relative_to_hit_hedge_loc = 0.;
    
    do
    {
        struct csmhedge_t *next_hedge;
        struct csmvertex_t *vertex;
        double x_vertex, y_vertex, z_vertex;
        struct csmvertex_t *next_vertex;
        double x_next_vertex, y_next_vertex, z_next_vertex;
        double t;
        
        assert(num_iteraciones < 10000);
        num_iteraciones++;
        
        next_hedge = csmhedge_next(iterator);
        
        vertex = csmhedge_vertex(iterator);
        csmvertex_get_coordenadas(vertex, &x_vertex, &y_vertex, &z_vertex);

        next_vertex = csmhedge_vertex(next_hedge);
        csmvertex_get_coordenadas(next_vertex, &x_next_vertex, &y_next_vertex, &z_next_vertex);
        
        if (csmmath_is_point_in_segment3D(
                        x, y, z,
                        x_vertex, y_vertex, z_vertex, x_next_vertex, y_next_vertex, z_next_vertex,
                        tolerance,
                        &t) == CIERTO)
        {
            is_point_on_loop_boundary = CIERTO;
            
            if (csmmath_compare_doubles(t, 0.0, tolerance) == CSMMATH_EQUAL_VALUES)
            {
                hit_vertex_loc = vertex;
                hit_hedge_loc = NULL;
                t_relative_to_hit_hedge_loc = 0.0;
            }
            else if (csmmath_compare_doubles(t, 1.0, tolerance) == CSMMATH_EQUAL_VALUES)
            {
                hit_vertex_loc = next_vertex;
                hit_hedge_loc = NULL;
                t_relative_to_hit_hedge_loc = 1.0;
            }
            else
            {
                hit_vertex_loc = NULL;
                hit_hedge_loc = iterator;
                t_relative_to_hit_hedge_loc = t;
            }
            break;
        }
        
        iterator = next_hedge;
        
    } while (iterator != ledge);
    
    ASIGNA_OPC(hit_vertex_opc, hit_vertex_loc);
    ASIGNA_OPC(hit_hedge_opc, hit_hedge_loc);
    ASIGNA_OPC(t_relative_to_hit_hedge_opc, t_relative_to_hit_hedge_loc);
    
    return is_point_on_loop_boundary;
}

// --------------------------------------------------------------------------------------------------------------

static CYBOOL i_are_hedges_collinear(struct csmhedge_t *he0, struct csmhedge_t *he1, struct csmhedge_t *he2)
{
    struct csmvertex_t *vertex0, *vertex1, *vertex2;
    double Ux1, Uy1, Uz1, Ux2, Uy2, Uz2;
    double Wx, Wy, Wz;
    
    vertex0 = csmhedge_vertex(he0);
    vertex1 = csmhedge_vertex(he1);
    vertex2 = csmhedge_vertex(he2);
    
    csmvertex_vector_from_vertex1_to_vertex2(vertex0, vertex1, &Ux1, &Uy1, &Uz1);
    csmvertex_vector_from_vertex1_to_vertex2(vertex1, vertex2, &Ux2, &Uy2, &Uz2);

    csmmath_cross_product3D(Ux1, Uy1, Uz1, Ux2, Uy2, Uz2, &Wx, &Wy, &Wz);
            
    if (csmmath_is_null_vector(Wx, Wy, Wz, csmtolerance_null_vector()) == FALSO)
    {
        return FALSO;
    }
    else
    {
        double dot_product;
        
        dot_product = csmmath_dot_product3D(Ux1, Uy1, Uz1, Ux2, Uy2, Uz2);
        
        if (dot_product < -1.e-6)
            return CIERTO;
        else
            return FALSO;
    }
}

// --------------------------------------------------------------------------------------------------------------
static unsigned long i_niter = 0;

CYBOOL csmloop_is_point_inside_loop(
                        const struct csmloop_t *loop,
                        double x, double y, double z, enum csmmath_dropped_coord_t dropped_coord,
                        double tolerance,
                        enum csmmath_contaiment_point_loop_t *type_of_containment_opc,
                        struct csmvertex_t **hit_vertex_opc,
                        struct csmhedge_t **hit_hedge_opc, double *t_relative_to_hit_hedge_opc)
{
    CYBOOL is_point_inside_loop;
    enum csmmath_contaiment_point_loop_t type_of_containment_loc;
    struct csmvertex_t *hit_vertex_loc;
    struct csmhedge_t *hit_hedge_loc;
    double t_relative_to_hit_hedge_loc;

    assert_no_null(loop);
    i_niter++;
    
    if (i_is_point_on_loop_boundary(
                        loop->ledge,
                        x, y, z,
                        tolerance,
                        &hit_vertex_loc,
                        &hit_hedge_loc, &t_relative_to_hit_hedge_loc) == CIERTO)
    {
        is_point_inside_loop = CIERTO;
        
        if (hit_vertex_loc != NULL)
        {
            assert(hit_hedge_loc == NULL);
            type_of_containment_loc = CSMMATH_CONTAIMENT_POINT_LOOP_ON_VERTEX;
        }
        else
        {
            assert(hit_hedge_loc != NULL);
            type_of_containment_loc = CSMMATH_CONTAIMENT_POINT_LOOP_ON_HEDGE;
        }
    }
    else
    {
        register struct csmhedge_t *ray_hedge;
        struct csmhedge_t *start_hedge;
        unsigned long num_iteraciones;
        
        is_point_inside_loop = FALSO;
        type_of_containment_loc = CSMMATH_CONTAIMENT_POINT_LOOP_INTERIOR;
        hit_vertex_loc = NULL;
        hit_hedge_loc = NULL;
        t_relative_to_hit_hedge_loc = 0.;
        
        ray_hedge = loop->ledge;
        start_hedge = NULL;
        num_iteraciones = 0;

        do
        {
            struct csmhedge_t *prev_ray_hedge, *next_ray_hedge;
            
            assert(num_iteraciones < 100000);
            num_iteraciones++;
            
            prev_ray_hedge = csmhedge_prev(ray_hedge);
            next_ray_hedge = csmhedge_next(ray_hedge);
            
            if (i_are_hedges_collinear(prev_ray_hedge, ray_hedge, next_ray_hedge) == FALSO)
            {
                start_hedge = ray_hedge;
                break;
            }
            
            ray_hedge = next_ray_hedge;
            
        } while (ray_hedge != loop->ledge);
        
        if (start_hedge != NULL)
        {
            double x_not_dropped, y_not_dropped;
            int count;
            
            // According to "Geometric tools for computer graphics", Page 701 (different from Mäntyllä)
            csmmath_select_not_dropped_coords(x, y, z, dropped_coord, &x_not_dropped, &y_not_dropped);
        
            ray_hedge = start_hedge;
            num_iteraciones = 0;
        
            count = 0;
            
            do
            {
                struct csmhedge_t *next_ray_hedge;
                CYBOOL hedges_collinear;
                struct csmvertex_t *vertex_i, *vertex_j;
                double x_vertex_i, y_vertex_i, x_vertex_j, y_vertex_j;
                
                assert(num_iteraciones < 100000);
                num_iteraciones++;
                
                next_ray_hedge = csmhedge_next(ray_hedge);
                hedges_collinear = FALSO;
                
                do
                {
                    struct csmhedge_t *next_next_ray_hedge;
                    
                    next_next_ray_hedge = csmhedge_next(next_ray_hedge);
                    
                    if (i_are_hedges_collinear(ray_hedge, next_ray_hedge, next_next_ray_hedge) == CIERTO)
                    {
                        hedges_collinear = CIERTO;
                        next_ray_hedge = next_next_ray_hedge;
                    }
                    else
                    {
                        hedges_collinear = FALSO;
                    }
                    
                } while (hedges_collinear == CIERTO);
                
                vertex_i = csmhedge_vertex(next_ray_hedge);
                csmvertex_get_coords_not_dropped(vertex_i, dropped_coord, &x_vertex_i, &y_vertex_i);
                
                vertex_j = csmhedge_vertex(ray_hedge);
                csmvertex_get_coords_not_dropped(vertex_j, dropped_coord, &x_vertex_j, &y_vertex_j);
                
                if (y_vertex_i > y_not_dropped != y_vertex_j > y_not_dropped)
                {
                    double term;
                    
                    term = (x_vertex_j - x_vertex_i) * (y_not_dropped - y_vertex_i) / (y_vertex_j - y_vertex_i) + x_vertex_i;
                    
                    if (x_not_dropped < term)
                        count++;
                }
                
                ray_hedge = next_ray_hedge;
                
            } while (ray_hedge != start_hedge);
            
            if (count % 2 == 0)
                is_point_inside_loop = FALSO;
            else
                is_point_inside_loop = CIERTO;
        }
    }
    
    
    ASIGNA_OPC(type_of_containment_opc, type_of_containment_loc);
    ASIGNA_OPC(hit_vertex_opc, hit_vertex_loc);
    ASIGNA_OPC(hit_hedge_opc, hit_hedge_loc);
    ASIGNA_OPC(t_relative_to_hit_hedge_opc, t_relative_to_hit_hedge_loc);
    
    return is_point_inside_loop;
}

// --------------------------------------------------------------------------------------------------------------

CYBOOL csmloop_is_bounded_by_vertex_with_mask_attrib(const struct csmloop_t *loop, csmvertex_mask_t mask_attrib)
{
    register struct csmhedge_t *iterator;
    unsigned long num_iteraciones;
    
    assert_no_null(loop);
    
    iterator = loop->ledge;
    num_iteraciones = 0;
    
    do
    {
        struct csmvertex_t *vertex;
        
        assert(num_iteraciones < 10000);
        num_iteraciones++;
        
        vertex = csmhedge_vertex(iterator);
        
        if (csmvertex_has_mask_attrib(vertex, mask_attrib) == FALSO)
            return FALSO;
        
        iterator = csmhedge_next(iterator);
        
    } while (iterator != loop->ledge);
    
    return CIERTO;
}

// --------------------------------------------------------------------------------------------------------------

CYBOOL csmloop_has_only_a_null_edge(const struct csmloop_t *loop)
{
    struct csmhedge_t *hedge_next;
    
    assert_no_null(loop);
    
    hedge_next = csmhedge_next(loop->ledge);
    
    if (hedge_next == loop->ledge)
        return CIERTO;
    else
        return FALSO;
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

// ----------------------------------------------------------------------------------------------------

void csmloop_revert_loop_orientation(struct csmloop_t *loop)
{
    register struct csmhedge_t *he_iterator;
    struct csmvertex_t *prev_vertex;
    unsigned long no_iters;
    
    assert_no_null(loop);
    
    he_iterator = loop->ledge;
    no_iters = 0;
            
    do
    {
        struct csmhedge_t *he_iter_prv, *he_iter_nxt;
        
        assert(no_iters < 10000);
        no_iters++;
        
        he_iter_prv = csmhedge_prev(he_iterator);
        he_iter_nxt = csmhedge_next(he_iterator);
        
        csmhedge_set_next(he_iterator, he_iter_prv);
        csmhedge_set_prev(he_iterator, he_iter_nxt);
        
        he_iterator = he_iter_nxt;
    }
    while (he_iterator != loop->ledge);
    
    prev_vertex = csmhedge_vertex(csmhedge_prev(he_iterator));
    no_iters = 0;
    
    do
    {
        struct csmvertex_t *vertex_aux;
        
        assert(no_iters < 10000);
        no_iters++;
        
        vertex_aux = csmhedge_vertex(he_iterator);
        
        csmhedge_set_vertex(he_iterator, prev_vertex);
        csmvertex_set_hedge(prev_vertex, he_iterator);
        
        prev_vertex = vertex_aux;
        
        he_iterator = csmhedge_next(he_iterator);
    }
    while (he_iterator != loop->ledge);
}

// ----------------------------------------------------------------------------------------------------

CYBOOL csmloop_setop_convert_loop_in_face(const struct csmloop_t *loop)
{
    assert_no_null(loop);
    return loop->setop_convert_loop_in_face;
}

// ----------------------------------------------------------------------------------------------------

void csmloop_set_setop_convert_loop_in_face(struct csmloop_t *loop, CYBOOL setop_convert_loop_in_face)
{
    assert_no_null(loop);
    loop->setop_convert_loop_in_face = setop_convert_loop_in_face;
}

// ----------------------------------------------------------------------------------------------------

CYBOOL csmloop_setop_loop_was_a_hole(const struct csmloop_t *loop)
{
    assert_no_null(loop);
    return loop->setop_loop_was_a_hole;
}

// ----------------------------------------------------------------------------------------------------

void csmloop_set_setop_loop_was_a_hole(struct csmloop_t *loop, CYBOOL setop_loop_was_a_hole)
{
    assert_no_null(loop);
    loop->setop_loop_was_a_hole = setop_loop_was_a_hole;
}

// ----------------------------------------------------------------------------------------------------

void csmloop_clear_algorithm_mask(struct csmloop_t *loop)
{
    assert_no_null(loop);
    
    loop->setop_convert_loop_in_face = FALSO;
    loop->setop_loop_was_a_hole = FALSO;
}

// ----------------------------------------------------------------------------------------------------

void csmloop_print_info_debug(struct csmloop_t *loop, CYBOOL is_outer_loop, CYBOOL assert_si_no_es_integro)
{
    struct csmhedge_t *ledge;
    struct csmhedge_t *iterator;
    unsigned long num_iters;
    
    ledge = csmloop_ledge(loop);
    iterator = ledge;
    csmdebug_print_debug_info("\tLoop %4lu: Outer = %d\n", csmnode_id(CSMNODE(loop)), is_outer_loop);
    
    num_iters = 0;
    
    do
    {
        struct csmvertex_t *vertex;
        double x, y, z;
        struct csmedge_t *edge;
        struct csmhedge_t *next_edge;
        
        assert(num_iters < 10000);
        num_iters++;
        
        vertex = csmhedge_vertex(iterator);
        csmvertex_get_coordenadas(vertex, &x, &y, &z);
        
        edge = csmhedge_edge(iterator);
        
        if (edge == NULL)
        {
            csmdebug_print_debug_info(
                "\t\t(He %4lu [edge (null)], %4lu, %6.3f, %6.3f, %6.3f, %d)\n",
                csmnode_id(CSMNODE(iterator)),
                csmnode_id(CSMNODE(vertex)),
                x, y, z,
                ES_CIERTO(csmhedge_loop(iterator) == loop));
        }
        else
        {
            const char *he_position;
            struct csmhedge_t *he1, *he2;
            struct csmhedge_t *he_mate;
            
            he1 = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_POS);
            he2 = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_NEG);
            he_mate = (iterator == he1) ? he2: he1;
            he_position = (iterator == he1) ? "HE1": "HE2";
            
            if (he_mate != NULL)
            {
                csmdebug_print_debug_info(
                    "\t\t(%3s %4lu [edge %6lu. Mate: %4lu], %4lu, %6.3f, %6.3f, %6.3f, %d)\n",
                    he_position,
                    csmnode_id(CSMNODE(iterator)),
                    csmnode_id(CSMNODE(edge)),
                    csmnode_id(CSMNODE(he_mate)),
                    csmnode_id(CSMNODE(vertex)),
                    x, y, z,
                    ES_CIERTO(csmhedge_loop(iterator) == loop));
            }
            else
            {
                csmdebug_print_debug_info(
                    "\t\t(%3s %4lu [edge %6lu. Mate: ----], %4lu, %6.3f, %6.3f, %6.3f, %d)\n",
                    he_position,
                    csmnode_id(CSMNODE(iterator)),
                    csmnode_id(CSMNODE(edge)),
                    csmnode_id(CSMNODE(vertex)),
                    x, y, z,
                    ES_CIERTO(csmhedge_loop(iterator) == loop));
            }
        }
        
        if (assert_si_no_es_integro == CIERTO)
            assert(csmhedge_loop(iterator) == loop);
        
        next_edge = csmhedge_next(iterator);
        
        if (assert_si_no_es_integro == CIERTO)
            assert(csmhedge_prev(next_edge) == iterator);
                    
        iterator = next_edge;
    }
    while (iterator != ledge);
}

// ----------------------------------------------------------------------------------------------------

void csmloop_append_loop_to_shape(
                        struct csmloop_t *loop,
                        double Xo, double Yo, double Zo,
                        double Ux, double Uy, double Uz, double Vx, double Vy, double Vz,
                        struct gccontorno_t *shape)
{
    struct csmhedge_t *iterator;
    unsigned long num_iteraciones;
    ArrPunto2D *points;
    
    assert_no_null(loop);
    
    iterator = loop->ledge;
    num_iteraciones = 0;
    
    points = arr_CreaPunto2D(0);
    
    do
    {
        struct csmvertex_t *vertex;
        double x_3d, y_3d, z_3d;
        double x_2d, y_2d;
        
        assert(num_iteraciones < 10000);
        num_iteraciones++;
        
        vertex = csmhedge_vertex(iterator);
        csmvertex_get_coordenadas(vertex, &x_3d, &y_3d, &z_3d);
        
        csmgeom_project_coords_3d_to_2d(
                        Xo, Yo, Zo,
                        Ux, Uy, Uz, Vx, Vy, Vz,
                        x_3d, y_3d, z_3d,
                        &x_2d, &y_2d);
        
        arr_AppendPunto2D(points, x_2d, y_2d);
        
        iterator = csmhedge_next(iterator);
        
    } while (iterator != loop->ledge);
    
    if (arr_NumElemsPunto2D(points) >= 3)
    {
        arr_InvertirPunto2D(points);
        gccontorno_append_array_puntos(shape, &points);
    }
    else
    {
        arr_DestruyePunto2D(&points);
    }
}

// ----------------------------------------------------------------------------------------------------

void csmloop_geometric_center_3d(struct csmloop_t *loop, double *x, double *y, double *z)
{
    struct csmhedge_t *iterator;
    unsigned long no_iters;
    unsigned long no_vertex;
    
    assert_no_null(loop);
    assert_no_null(x);
    assert_no_null(y);
    assert_no_null(z);
    
    *x = 0.;
    *y = 0.;
    *z = 0.;
    no_vertex = 0;
    
    iterator = loop->ledge;
    no_iters = 0;
    
    do
    {
        struct csmvertex_t *vertex;
        double x_3d, y_3d, z_3d;
        
        assert(no_iters < 10000);
        no_iters++;
        
        vertex = csmhedge_vertex(iterator);
        csmvertex_get_coordenadas(vertex, &x_3d, &y_3d, &z_3d);
        
        *x += x_3d;
        *y += y_3d;
        *z += z_3d;
        
        no_vertex++;
        
        iterator = csmhedge_next(iterator);
        
    } while (iterator != loop->ledge);
    
    assert(no_vertex > 0);
    
    *x /= no_vertex;
    *y /= no_vertex;
    *z /= no_vertex;
}





