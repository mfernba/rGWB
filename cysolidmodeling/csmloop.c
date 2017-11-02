// Loop...

#include "csmloop.inl"
#include "csmloop.tli"

#include "csmbbox.inl"
#include "csmgeom.inl"
#include "csmmath.inl"
#include "csmmath.tli"
#include "csmnode.inl"
#include "csmedge.inl"
#include "csmedge.tli"
#include "csmhedge.inl"
#include "csmtolerance.inl"
#include "csmvertex.inl"
#include "csmassert.inl"
#include "csmid.inl"
#include "csmmem.inl"
#include "csmstring.inl"
#include "csmmath.inl"
#include "csmmath.tli"

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
                        CSMBOOL setop_convert_loop_in_face,
                        CSMBOOL setop_loop_was_a_hole))
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
    CSMBOOL setop_convert_loop_in_face;
    CSMBOOL setop_loop_was_a_hole;
    
    id = csmid_new_id(id_nuevo_elemento, NULL);
    
    ledge = NULL;
    lface = face;
    
    setop_convert_loop_in_face = CSMFALSE;
    setop_loop_was_a_hole = CSMFALSE;
    
    return i_crea(id, ledge, lface, setop_convert_loop_in_face, setop_loop_was_a_hole);
}

// --------------------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmloop_t *, i_duplicate_loop, (struct csmface_t *lface, unsigned long *id_nuevo_elemento))
{
    unsigned long id;
    struct csmhedge_t *ledge;
    CSMBOOL setop_convert_loop_in_face;
    CSMBOOL setop_loop_was_a_hole;
    
    id = csmid_new_id(id_nuevo_elemento, NULL);
    ledge = NULL;
    setop_convert_loop_in_face = CSMFALSE;
    setop_loop_was_a_hole = CSMFALSE;
    
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
    loop->clase_base.id = csmid_new_id(id_new_element, new_id_opt);
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
    
    if (csmmath_is_null_vector(A_loc, B_loc, C_loc, csmtolerance_null_vector()) == CSMTRUE)
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
        
        distance = csmmath_fabs(csmmath_signed_distance_point_to_plane(x, y, z, A, B, C, D));
        max_distance_to_plane = CSMMATH_MAX(max_distance_to_plane, distance);

        iterator = csmhedge_next(iterator);
        
    } while (iterator != loop->ledge);
    
    return CSMMATH_MAX(max_distance_to_plane, csmtolerance_coplanarity());
}

// ----------------------------------------------------------------------------------------------------

double csmloop_compute_area(
                        struct csmloop_t *loop,
                        double Xo, double Yo, double Zo,
                        double Ux, double Uy, double Uz, double Vx, double Vy, double Vz)
{
    double area;
    struct csmhedge_t *iterator;
    unsigned long num_iteraciones;
    
    assert_no_null(loop);
    
    iterator = loop->ledge;
    num_iteraciones = 0;
    
    area = 0.0;
    
    do
    {
        struct csmvertex_t *vertex_i, *vertex_i_1;
        double x_3d, y_3d, z_3d;
        double x_i, y_i, x_i_1, y_i_1;
        double area_i;
        
        assert(num_iteraciones < 10000);
        num_iteraciones++;
        
        vertex_i = csmhedge_vertex(iterator);
        csmvertex_get_coordenadas(vertex_i, &x_3d, &y_3d, &z_3d);
        csmgeom_project_coords_3d_to_2d(Xo, Yo, Zo, Ux, Uy, Uz, Vx, Vy, Vz, x_3d, y_3d, z_3d, &x_i, &y_i);
        
        iterator = csmhedge_next(iterator);
        vertex_i_1 = csmhedge_vertex(iterator);
        csmvertex_get_coordenadas(vertex_i_1, &x_3d, &y_3d, &z_3d);
        csmgeom_project_coords_3d_to_2d(Xo, Yo, Zo, Ux, Uy, Uz, Vx, Vy, Vz, x_3d, y_3d, z_3d, &x_i_1, &y_i_1);
        
        area_i = x_i * y_i_1 - y_i * x_i_1;
        area += area_i;
        
    } while (iterator != loop->ledge);
    
    return 0.5 * area;
}

// --------------------------------------------------------------------------------------------------------------

static CSMBOOL i_is_point_on_loop_boundary(
                        struct csmhedge_t *ledge,
                        double x, double y, double z, double tolerance,
                        struct csmvertex_t **hit_vertex_opc,
                        struct csmhedge_t **hit_hedge_opc, double *t_relative_to_hit_hedge_opc)
{
    CSMBOOL is_point_on_loop_boundary;
    struct csmvertex_t *hit_vertex_loc;
    struct csmhedge_t *hit_hedge_loc;
    double t_relative_to_hit_hedge_loc;
    register struct csmhedge_t *iterator;
    unsigned long num_iteraciones;
    
    iterator = ledge;
    num_iteraciones = 0;
    
    is_point_on_loop_boundary = CSMFALSE;
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
                        &t) == CSMTRUE)
        {
            is_point_on_loop_boundary = CSMTRUE;
            
            if (csmmath_compare_doubles(t, 0.0, tolerance) == CSMCOMPARE_EQUAL)
            {
                hit_vertex_loc = vertex;
                hit_hedge_loc = NULL;
                t_relative_to_hit_hedge_loc = 0.0;
            }
            else if (csmmath_compare_doubles(t, 1.0, tolerance) == CSMCOMPARE_EQUAL)
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
    
    ASSIGN_OPTIONAL_VALUE(hit_vertex_opc, hit_vertex_loc);
    ASSIGN_OPTIONAL_VALUE(hit_hedge_opc, hit_hedge_loc);
    ASSIGN_OPTIONAL_VALUE(t_relative_to_hit_hedge_opc, t_relative_to_hit_hedge_loc);
    
    return is_point_on_loop_boundary;
}

// --------------------------------------------------------------------------------------------------------------

static CSMBOOL i_are_hedges_collinear(struct csmhedge_t *he0, struct csmhedge_t *he1, struct csmhedge_t *he2)
{
    struct csmvertex_t *vertex0, *vertex1, *vertex2;
    double tolerance_equal_coords;
    
    vertex0 = csmhedge_vertex(he0);
    vertex1 = csmhedge_vertex(he1);
    vertex2 = csmhedge_vertex(he2);
    
    tolerance_equal_coords = csmtolerance_equal_coords();
    
    if (csmvertex_equal_coords(vertex0, vertex1, tolerance_equal_coords) == CSMTRUE
            || csmvertex_equal_coords(vertex1, vertex2, tolerance_equal_coords) == CSMTRUE)
    {
        return CSMFALSE;
    }
    else
    {
        double Ux1, Uy1, Uz1, Ux2, Uy2, Uz2;
        
        csmvertex_vector_from_vertex1_to_vertex2(vertex0, vertex1, &Ux1, &Uy1, &Uz1);
        csmvertex_vector_from_vertex1_to_vertex2(vertex1, vertex2, &Ux2, &Uy2, &Uz2);
        
        if (csmmath_vectors_are_parallel(Ux1, Uy1, Uz1, Ux2, Uy2, Uz2) == CSMFALSE)
        {
            return CSMFALSE;
        }
        else
        {
            double dot_product;
            
            dot_product = csmmath_dot_product3D(Ux1, Uy1, Uz1, Ux2, Uy2, Uz2);
            
            if (dot_product < -1.e-6)
                return CSMTRUE;
            else
                return CSMFALSE;
        }
    }
}

// --------------------------------------------------------------------------------------------------------------
static unsigned long i_niter = 0;

CSMBOOL csmloop_is_point_inside_loop(
                        const struct csmloop_t *loop,
                        double x, double y, double z, enum csmmath_dropped_coord_t dropped_coord,
                        double tolerance,
                        enum csmmath_contaiment_point_loop_t *type_of_containment_opc,
                        struct csmvertex_t **hit_vertex_opc,
                        struct csmhedge_t **hit_hedge_opc, double *t_relative_to_hit_hedge_opc)
{
    CSMBOOL is_point_inside_loop;
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
                        &hit_hedge_loc, &t_relative_to_hit_hedge_loc) == CSMTRUE)
    {
        is_point_inside_loop = CSMTRUE;
        
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
        
        is_point_inside_loop = CSMFALSE;
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
            
            if (i_are_hedges_collinear(prev_ray_hedge, ray_hedge, next_ray_hedge) == CSMFALSE)
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
                CSMBOOL hedges_collinear;
                struct csmvertex_t *vertex_i, *vertex_j;
                double x_vertex_i, y_vertex_i, x_vertex_j, y_vertex_j;
                
                assert(num_iteraciones < 100000);
                num_iteraciones++;
                
                next_ray_hedge = csmhedge_next(ray_hedge);
                hedges_collinear = CSMFALSE;
                
                do
                {
                    struct csmhedge_t *next_next_ray_hedge;
                    
                    next_next_ray_hedge = csmhedge_next(next_ray_hedge);
                    
                    if (i_are_hedges_collinear(ray_hedge, next_ray_hedge, next_next_ray_hedge) == CSMTRUE)
                    {
                        hedges_collinear = CSMTRUE;
                        next_ray_hedge = next_next_ray_hedge;
                    }
                    else
                    {
                        hedges_collinear = CSMFALSE;
                    }
                    
                } while (hedges_collinear == CSMTRUE);
                
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
                is_point_inside_loop = CSMFALSE;
            else
                is_point_inside_loop = CSMTRUE;
        }
    }
    
    
    ASSIGN_OPTIONAL_VALUE(type_of_containment_opc, type_of_containment_loc);
    ASSIGN_OPTIONAL_VALUE(hit_vertex_opc, hit_vertex_loc);
    ASSIGN_OPTIONAL_VALUE(hit_hedge_opc, hit_hedge_loc);
    ASSIGN_OPTIONAL_VALUE(t_relative_to_hit_hedge_opc, t_relative_to_hit_hedge_loc);
    
    return is_point_inside_loop;
}

// --------------------------------------------------------------------------------------------------------------

CSMBOOL csmloop_is_bounded_by_vertex_with_mask_attrib(const struct csmloop_t *loop, csmvertex_mask_t mask_attrib)
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
        
        if (csmvertex_has_mask_attrib(vertex, mask_attrib) == CSMFALSE)
            return CSMFALSE;
        
        iterator = csmhedge_next(iterator);
        
    } while (iterator != loop->ledge);
    
    return CSMTRUE;
}

// --------------------------------------------------------------------------------------------------------------

CSMBOOL csmloop_has_only_a_null_edge(const struct csmloop_t *loop)
{
    struct csmhedge_t *hedge_next;
    
    assert_no_null(loop);
    
    hedge_next = csmhedge_next(loop->ledge);
    
    if (hedge_next == loop->ledge)
        return CSMTRUE;
    else
        return CSMFALSE;
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

CSMBOOL csmloop_setop_convert_loop_in_face(const struct csmloop_t *loop)
{
    assert_no_null(loop);
    return loop->setop_convert_loop_in_face;
}

// ----------------------------------------------------------------------------------------------------

void csmloop_set_setop_convert_loop_in_face(struct csmloop_t *loop, CSMBOOL setop_convert_loop_in_face)
{
    assert_no_null(loop);
    loop->setop_convert_loop_in_face = setop_convert_loop_in_face;
}

// ----------------------------------------------------------------------------------------------------

CSMBOOL csmloop_setop_loop_was_a_hole(const struct csmloop_t *loop)
{
    assert_no_null(loop);
    return loop->setop_loop_was_a_hole;
}

// ----------------------------------------------------------------------------------------------------

void csmloop_set_setop_loop_was_a_hole(struct csmloop_t *loop, CSMBOOL setop_loop_was_a_hole)
{
    assert_no_null(loop);
    loop->setop_loop_was_a_hole = setop_loop_was_a_hole;
}

// ----------------------------------------------------------------------------------------------------

void csmloop_clear_algorithm_mask(struct csmloop_t *loop)
{
    assert_no_null(loop);
    
    loop->setop_convert_loop_in_face = CSMFALSE;
    loop->setop_loop_was_a_hole = CSMFALSE;
}




