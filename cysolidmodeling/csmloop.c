// Loop...

#include "csmloop.inl"

#include "csmbbox.inl"
#include "csmmath.inl"
#include "csmmath.tli"
#include "csmnode.inl"
#include "csmhedge.inl"
#include "csmvertex.inl"

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
                        struct csmface_t *lface))
{
    struct csmloop_t *loop;
    
    loop = MALLOC(struct csmloop_t);
    
    loop->clase_base = csmnode_crea_node(id, loop, i_csmloop_destruye, csmloop_t);
    
    loop->ledge = ledge;
    loop->lface = lface;
    
    return loop;
}

// --------------------------------------------------------------------------------------------------------------

struct csmloop_t *csmloop_crea(struct csmface_t *face, unsigned long *id_nuevo_elemento)
{
    unsigned long id;
    struct csmhedge_t *ledge;
    struct csmface_t *lface;
    
    id = cypeid_nuevo_id(id_nuevo_elemento, NULL);
    
    ledge = NULL;
    lface = face;
    
    return i_crea(id, ledge, lface);
}

// --------------------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmloop_t *, i_duplicate_loop, (struct csmface_t *lface, unsigned long *id_nuevo_elemento))
{
    unsigned long id;
    struct csmhedge_t *ledge;
    
    id = cypeid_nuevo_id(id_nuevo_elemento, NULL);
    
    ledge = NULL;
    
    return i_crea(id, ledge, lface);
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
    
    assert(num_vertexs >= 3);
    
    csmmath_make_unit_vector3D(&A_loc, &B_loc, &C_loc);
    D_loc = -csmmath_dot_product3D(A_loc, B_loc, C_loc, xc, yc, zc) / num_vertexs;
    
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
    
    return max_distance_to_plane;
}

// --------------------------------------------------------------------------------------------------------------

static CYBOOL i_is_point_on_loop_boundary(
                        struct csmhedge_t *ledge,
                        double x, double y, double z, double tolerance,
                        struct csmvertex_t **hit_vertex_opc,
                        struct csmhedge_t **hit_hedge_opc)
{
    CYBOOL is_point_on_loop_boundary;
    struct csmvertex_t *hit_vertex_loc;
    struct csmhedge_t *hit_hedge_loc;
    register struct csmhedge_t *iterator;
    unsigned long num_iteraciones;
    
    iterator = ledge;
    num_iteraciones = 0;
    
    is_point_on_loop_boundary = FALSO;
    hit_vertex_loc = NULL;
    hit_hedge_loc = NULL;
    
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
            }
            else if (csmmath_compare_doubles(t, 1.0, tolerance) == CSMMATH_EQUAL_VALUES)
            {
                hit_vertex_loc = next_vertex;
                hit_hedge_loc = NULL;
            }
            else
            {
                hit_vertex_loc = NULL;
                hit_hedge_loc = iterator;
            }
            break;
        }
        
        iterator = next_hedge;
        
    } while (iterator != ledge);
    
    ASIGNA_OPC(hit_vertex_opc, hit_vertex_loc);
    ASIGNA_OPC(hit_hedge_opc, hit_hedge_loc);
    
    return is_point_on_loop_boundary;
}

// --------------------------------------------------------------------------------------------------------------

CYBOOL csmloop_is_point_inside_loop(
                        const struct csmloop_t *loop, CYBOOL is_outer_loop,
                        double x, double y, double z, enum csmmath_dropped_coord_t dropped_coord,
                        double tolerance,
                        enum csmmath_contaiment_point_loop_t *type_of_containment_opc,
                        struct csmvertex_t **hit_vertex_opc,
                        struct csmhedge_t **hit_hedge_opc)
{
    CYBOOL is_point_inside_loop;
    enum csmmath_contaiment_point_loop_t type_of_containment_loc;
    struct csmvertex_t *hit_vertex_loc;
    struct csmhedge_t *hit_hedge_loc;

    assert_no_null(loop);
    
    if (i_is_point_on_loop_boundary(loop->ledge, x, y, z, tolerance, &hit_vertex_loc, &hit_hedge_loc) == CIERTO)
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
        double x_not_dropped, y_not_dropped;
        register struct csmhedge_t *ray_hedge;
        unsigned long num_iteraciones;
        int count;
        
        // According to "Geometric tools for computer graphics", Page 701 (different from Mäntyllä)
    
        csmmath_select_not_dropped_coords(x, y, z, dropped_coord, &x_not_dropped, &y_not_dropped);
        
        ray_hedge = loop->ledge;
        num_iteraciones = 0;
        
        is_point_inside_loop = FALSO;
        type_of_containment_loc = CSMMATH_CONTAIMENT_POINT_LOOP_INTERIOR;
        hit_vertex_loc = NULL;
        hit_hedge_loc = NULL;
        
        count = 0;
        
        do
        {
            struct csmhedge_t *next_ray_hedge;
            struct csmvertex_t *vertex1, *vertex2;
            double x_vertex1, y_vertex1, x_vertex2, y_vertex2;
            
            assert(num_iteraciones < 100000);
            num_iteraciones++;
            
            next_ray_hedge = csmhedge_next(ray_hedge);
            
            vertex1 = csmhedge_vertex(ray_hedge);
            csmvertex_get_coords_not_dropped(vertex1, dropped_coord, &x_vertex1, &y_vertex1);
            
            vertex2 = csmhedge_vertex(next_ray_hedge);
            csmvertex_get_coords_not_dropped(vertex2, dropped_coord, &x_vertex2, &y_vertex2);
            
            if ((y_vertex1 - tolerance < y_not_dropped && y_not_dropped < y_vertex2)
                    || (y_vertex2 - tolerance < y_not_dropped && y_not_dropped < y_vertex1))
            {
                double x = x_vertex1 + ((y_not_dropped - y_vertex1) * (x_vertex2 - x_vertex1) / (y_vertex2 - y_vertex1));
                
                if (is_outer_loop == CIERTO)
                {
                    if (x < x_not_dropped)
                        count++;
                }
                else
                {
                    if (x > x_not_dropped)
                        count++;
                }
            }
            
            ray_hedge = next_ray_hedge;
            
        } while (ray_hedge != loop->ledge);
        
        if (count % 2 == 0)
            is_point_inside_loop = FALSO;
        else
            is_point_inside_loop = CIERTO;
    }
    
    
    ASIGNA_OPC(type_of_containment_opc, type_of_containment_loc);
    ASIGNA_OPC(hit_vertex_opc, hit_vertex_loc);
    ASIGNA_OPC(hit_hedge_opc, hit_hedge_loc);
    
    return is_point_inside_loop;
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
