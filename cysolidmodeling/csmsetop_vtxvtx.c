// Boolean operations on solids: Vertex - Vertex classifier...

#include "csmsetop_vtxvtx.inl"

#include "csmmath.inl"
#include "csmhedge.inl"
#include "csmdebug.inl"
#include "csmedge.inl"
#include "csmedge.tli"
#include "csmeuler_lmev.inl"
#include "csmface.inl"
#include "csmopbas.inl"
#include "csmsetop.tli"
#include "csmsetopcom.inl"
#include "csmtolerance.inl"
#include "csmvertex.inl"

#include "a_punter.h"
#include "copiafor.h"
#include "cyassert.h"
#include "cypespy.h"
#include "cypestr.h"
#include "defmath.tlh"
#include "standarc.h"

ArrEstructura(i_neighborhood_t);
ArrEstructura(i_inters_sectors_t);

struct csmsetop_vtxvtx_inters_t
{
    struct csmvertex_t *vertex_a, *vertex_b;
};

struct i_neighborhood_t
{
    struct csmhedge_t *he;
    
    double Ux1, Uy1, Uz1;
    double Ux2, Uy2, Uz2;
    double Ux12, Uy12, Uz12;
    
    double A_face, B_face, C_face;
};

struct i_inters_sectors_t
{
    unsigned long idx_nba, idx_nbb;
    enum csmsetop_classify_resp_solid_t s1a, s2a;
    enum csmsetop_classify_resp_solid_t s1b, s2b;
    CYBOOL intersect;
};

// ------------------------------------------------------------------------------------------

struct csmsetop_vtxvtx_inters_t *csmsetop_vtxvtx_create_inters(struct csmvertex_t *vertex_a, struct csmvertex_t *vertex_b)
{
    struct csmsetop_vtxvtx_inters_t *vv_inters;
    
    vv_inters = MALLOC(struct csmsetop_vtxvtx_inters_t);
    
    vv_inters->vertex_a = vertex_a;
    vv_inters->vertex_b = vertex_b;
    
    return vv_inters;
}

// ------------------------------------------------------------------------------------------

void csmsetop_vtxvtx_free_inters(struct csmsetop_vtxvtx_inters_t **vv_inters)
{
    assert_no_null(vv_inters);
    assert_no_null(*vv_inters);
    
    FREE_PP(vv_inters, struct csmsetop_vtxvtx_inters_t);
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_neighborhood_t *, i_create_neighborhood, (
                        struct csmhedge_t *he,
                        double Ux1, double Uy1, double Uz1,
                        double Ux2, double Uy2, double Uz2,
                        double Ux12, double Uy12, double Uz12,
                        double A_face, double B_face, double C_face))
{
    struct i_neighborhood_t *neighborhood;
    
    neighborhood = MALLOC(struct i_neighborhood_t);
    
    neighborhood->he = he;
    
    neighborhood->Ux1 = Ux1;
    neighborhood->Uy1 = Uy1;
    neighborhood->Uz1 = Uz1;

    neighborhood->Ux2 = Ux2;
    neighborhood->Uy2 = Uy2;
    neighborhood->Uz2 = Uz2;

    neighborhood->Ux12 = Ux12;
    neighborhood->Uy12 = Uy12;
    neighborhood->Uz12 = Uz12;
 
    neighborhood->A_face = A_face;
    neighborhood->B_face = B_face;
    neighborhood->C_face = C_face;
    
    return neighborhood;
}

// ------------------------------------------------------------------------------------------

static void i_free_neighborhood(struct i_neighborhood_t **neighborhood)
{
    FREE_PP(neighborhood, struct i_neighborhood_t);
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_inters_sectors_t *, i_create_inters_sectors, (
                        unsigned long idx_nba, unsigned long idx_nbb,
                        enum csmsetop_classify_resp_solid_t s1a, enum csmsetop_classify_resp_solid_t s2a,
                        enum csmsetop_classify_resp_solid_t s1b, enum csmsetop_classify_resp_solid_t s2b,
                        CYBOOL intersect))
{
    struct i_inters_sectors_t *inters;
    
    inters = MALLOC(struct i_inters_sectors_t);
    
    inters->idx_nba = idx_nba;
    inters->idx_nbb = idx_nbb;
    
    inters->s1a = s1a;
    inters->s2a = s2a;

    inters->s1b = s1b;
    inters->s2b = s2b;
    
    inters->intersect = intersect;
    
    return inters;
}

// ------------------------------------------------------------------------------------------

static void i_free_inters_sectors(struct i_inters_sectors_t **inters)
{
    FREE_PP(inters, struct i_inters_sectors_t);
}

// ------------------------------------------------------------------------------------------

static void i_vector_to_he(struct csmhedge_t *he, const struct csmvertex_t *vertex, double *Ux, double *Uy, double *Uz)
{
    struct csmvertex_t *he_vertex;
    double x1, y1, z1, x2, y2, z2;
    
    assert_no_null(Ux);
    assert_no_null(Uy);
    assert_no_null(Uz);

    csmvertex_get_coordenadas(vertex, &x1, &y1, &z1);
    
    he_vertex = csmhedge_vertex(he);
    csmvertex_get_coordenadas(he_vertex, &x2, &y2, &z2);
    
    *Ux = x2 - x1;
    *Uy = y2 - y1;
    *Uz = z2 - z1;
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static ArrEstructura(i_neighborhood_t) *, i_preprocess_neighborhood, (struct csmvertex_t *vertex))
{
    ArrEstructura(i_neighborhood_t) *neighborhoods;
    register struct csmhedge_t *vhedge, *he_iterator;
    unsigned long num_iters;
    double null_vector_tolerance;
    
    neighborhoods = arr_CreaPunteroST(0, i_neighborhood_t);
    
    vhedge = csmvertex_hedge(vertex);
    he_iterator = vhedge;
    num_iters = 0;
    
    null_vector_tolerance = csmtolerance_null_vector();
    
    do
    {
        struct csmface_t *he_iterator_face;
        double A, B, C, D;
        struct csmhedge_t *he_iterator_prv, *he_iterator_nxt;
        double Ux1, Uy1, Uz1, Ux2, Uy2, Uz2;
        double Ux12, Uy12, Uz12;
        CYBOOL is_null_vector, is_oriented_in_direction;
        struct i_neighborhood_t *neigborhood;
        
        assert(num_iters < 10000);
        num_iters++;
        
        assert(csmhedge_vertex(he_iterator) == vertex);
        
        he_iterator_face = csmopbas_face_from_hedge(he_iterator);
        csmface_face_equation(he_iterator_face, &A, &B, &C, &D);
        
        he_iterator_prv = csmhedge_prev(he_iterator);
        i_vector_to_he(he_iterator_prv, vertex, &Ux1, &Uy1, &Uz1);
        
        he_iterator_nxt = csmhedge_next(he_iterator);
        i_vector_to_he(he_iterator_nxt, vertex, &Ux2, &Uy2, &Uz2);

        csmmath_cross_product3D(Ux1, Uy1, Uz1, Ux2, Uy2, Uz2, &Ux12, &Uy12, &Uz12);
        
        is_null_vector = csmmath_is_null_vector(Ux12, Uy12, Uz12, null_vector_tolerance);
        is_oriented_in_direction = csmface_is_oriented_in_direction(he_iterator_face, Ux12, Uy12, Uz12);
        
        neigborhood = i_create_neighborhood(he_iterator, Ux1, Uy1, Uz1, Ux2, Uy2, Uz2, Ux12, Uy12, Uz12, A, B, C);
        arr_AppendPunteroST(neighborhoods, neigborhood, i_neighborhood_t);
        
        if (is_null_vector == CIERTO || is_oriented_in_direction == CIERTO)
        {
            double Ux_bisec, Uy_bisec, Uz_bisec;
            double Ux12_bisec, Uy12_bisec, Uz12_bisec;
            struct i_neighborhood_t *neigborhood_bisec;
            
            if (is_null_vector == CIERTO)
            {
                csmmath_cross_product3D(A, B, C, Ux2, Uy2, Uz2, &Ux_bisec, &Uy_bisec, &Uz_bisec);
            }
            else
            {
                // It's a wide sector, bisec points out of loop, so inverse it to point to the interior of the face
                Ux_bisec = -(Ux1 + Ux2);
                Uy_bisec = -(Uy1 + Uy2);
                Uz_bisec = -(Uz1 + Uz2);
            }
            
            csmmath_cross_product3D(Ux_bisec, Uy_bisec, Uz_bisec, Ux2, Uy2, Uz2, &Ux12_bisec, &Uy12_bisec, &Uz12_bisec);
            neigborhood_bisec = i_create_neighborhood(he_iterator, Ux_bisec, Uy_bisec, Uz_bisec, Ux2, Uy2, Uz2, Ux12, Uy12, Uz12, A, B, C);
            arr_AppendPunteroST(neighborhoods, neigborhood_bisec, i_neighborhood_t);
            
            neigborhood->Ux2 = Ux_bisec;
            neigborhood->Uy2 = Uy_bisec;
            neigborhood->Uz2 = Uz_bisec;
            
            csmmath_cross_product3D(
                        neigborhood->Ux1, neigborhood->Uy1, neigborhood->Uz1,
                        neigborhood->Ux2, neigborhood->Uy2, neigborhood->Uz2,
                        &neigborhood->Ux12, &neigborhood->Uy12, &neigborhood->Uz12);
        }
        
        he_iterator = csmhedge_next(csmopbas_mate(he_iterator));
        
    } while (he_iterator != vhedge);
    
    return neighborhoods;
}

// ------------------------------------------------------------------------------------------

static enum csmsetop_classify_resp_solid_t i_classify_vector_resp_sector(
                        const struct i_neighborhood_t *neighborhood,
                        double Ux, double Uy, double Uz)
{
    struct csmface_t *neighborhood_face;
    double A, B, C, D;
    double tolerance;
    double dot_product;
    
    assert_no_null(neighborhood);
    
    neighborhood_face = csmopbas_face_from_hedge(neighborhood->he);
    
    csmface_face_equation(neighborhood_face, &A, &B, &C, &D);
    tolerance = csmface_tolerace(neighborhood_face);
    
    dot_product = csmmath_dot_product3D(A, B, C, Ux, Uy, Uz);
    return csmsetopcom_classify_value_respect_to_plane(dot_product, tolerance);
}

// ------------------------------------------------------------------------------------------

static CYBOOL i_is_intersection_within_sector(const struct i_neighborhood_t *neighborhood, double Wx_inters, double Wy_inters, double Wz_inters)
{
    double Wx1, Wy1, Wz1;
    double tolerance_null_vector;
    
    assert_no_null(neighborhood);
    
    csmmath_cross_product3D(Wx_inters, Wy_inters, Wz_inters, neighborhood->Ux1, neighborhood->Uy1, neighborhood->Uz1, &Wx1, &Wy1, &Wz1);
    tolerance_null_vector = csmtolerance_null_vector();
    
    if (csmmath_is_null_vector(Wx1, Wy1, Wz1, tolerance_null_vector) == CIERTO)
    {
        double dot_product;
        
        dot_product = csmmath_dot_product3D(Wx_inters, Wy_inters, Wz_inters, neighborhood->Ux1, neighborhood->Uy1, neighborhood->Uz1);
        return (dot_product > 0.0) ? CIERTO: FALSO;
    }
    else
    {
        double Wx2, Wy2, Wz2;
        
        csmmath_cross_product3D(neighborhood->Ux2, neighborhood->Uy2, neighborhood->Uz2, Wx_inters, Wy_inters, Wz_inters, &Wx2, &Wy2, &Wz2);

        if (csmmath_is_null_vector(Wx2, Wy2, Wz2, tolerance_null_vector) == CIERTO)
        {
            double dot_product;
            
            dot_product = csmmath_dot_product3D(Wx_inters, Wy_inters, Wz_inters, neighborhood->Ux2, neighborhood->Uy2, neighborhood->Uz2);
            return (dot_product > 0.0) ? CIERTO: FALSO;
        }
        else
        {
            double dot_product1, dot_product2;
            enum csmmath_double_relation_t t1, t2;
            
            dot_product1 = csmmath_dot_product3D(Wx1, Wy1, Wz1, neighborhood->Ux12, neighborhood->Uy12, neighborhood->Uz12);
            t1 = csmmath_compare_doubles(dot_product1, 0.0, tolerance_null_vector);
            
            dot_product2 = csmmath_dot_product3D(Wx2, Wy2, Wz2, neighborhood->Ux12, neighborhood->Uy12, neighborhood->Uz12);
            t2 = csmmath_compare_doubles(dot_product2, 0.0, tolerance_null_vector);
            
            if (t1 == CSMMATH_VALUE1_LESS_THAN_VALUE2 && t2 == CSMMATH_VALUE1_LESS_THAN_VALUE2)
                return CIERTO;
            else
                return FALSO;
        }
    }
}

// ------------------------------------------------------------------------------------------

static double i_angle_of_vector_relative_to_plane(
                        double Ux_plane, double Uy_plane, double Uz_plane,
                        double Vx_plane, double Vy_plane, double Vz_plane,
                        double Ux_vector, double Uy_vector, double Uz_vector)
{
    double Ux_vector_2d, Uy_vector_2d;
    
    Ux_vector_2d = csmmath_dot_product3D(Ux_plane, Uy_plane, Uz_plane, Ux_vector, Uy_vector, Uz_vector);
    Uy_vector_2d = csmmath_dot_product3D(Vx_plane, Vy_plane, Vz_plane, Ux_vector, Uy_vector, Uz_vector);
    
    return atan2(Uy_vector_2d, Ux_vector_2d);
}

// ------------------------------------------------------------------------------------------

static CYBOOL i_sectors_overlap(const struct i_neighborhood_t *neighborhood_a, const struct i_neighborhood_t *neighborhood_b)
{
    assert_no_null(neighborhood_a);
    assert_no_null(neighborhood_b);
    
    if (neighborhood_a->he == neighborhood_b->he)
    {
        return FALSO;
    }
    else
    {
        double Ux_plane, Uy_plane, Uz_plane, Vx_plane, Vy_plane, Vz_plane;
        double a1, a2, b1, b2;
        double a_max, b_min;
        
        Ux_plane = neighborhood_a->Ux1;
        Uy_plane = neighborhood_a->Uy1;
        Uz_plane = neighborhood_a->Uz1;
        
        csmmath_cross_product3D(neighborhood_a->A_face, neighborhood_a->B_face, neighborhood_a->C_face, Ux_plane, Uy_plane, Uz_plane, &Vx_plane, &Vy_plane, &Vz_plane);
        csmmath_make_unit_vector3D(&Vx_plane, &Vy_plane, &Vz_plane);
        
        a1 = i_angle_of_vector_relative_to_plane(Ux_plane, Uy_plane, Uz_plane, Vx_plane, Vy_plane, Vz_plane, neighborhood_a->Ux1, neighborhood_a->Uy1, neighborhood_a->Uz1);
        a2 = i_angle_of_vector_relative_to_plane(Ux_plane, Uy_plane, Uz_plane, Vx_plane, Vy_plane, Vz_plane, neighborhood_a->Ux2, neighborhood_a->Uy2, neighborhood_a->Uz2);
        b1 = i_angle_of_vector_relative_to_plane(Ux_plane, Uy_plane, Uz_plane, Vx_plane, Vy_plane, Vz_plane, neighborhood_b->Ux1, neighborhood_b->Uy1, neighborhood_b->Uz1);
        b2 = i_angle_of_vector_relative_to_plane(Ux_plane, Uy_plane, Uz_plane, Vx_plane, Vy_plane, Vz_plane, neighborhood_b->Ux2, neighborhood_b->Uy2, neighborhood_b->Uz2);
        
        a_max = MAX(a1, a2);
        b_min = MIN(b1, b2);
        
        if (a_max + csmtolerance_angle_rad() > b_min)
            return CIERTO;
        else
            return FALSO;
    }
}

// ------------------------------------------------------------------------------------------

static CYBOOL i_exists_intersection_between_sectors(const struct i_neighborhood_t *neighborhood_a, const struct i_neighborhood_t *neighborhood_b)
{
    double Wx_inters, Wy_inters, Wz_inters;
    double tolerance_null_vector;
    
    assert_no_null(neighborhood_a);
    assert_no_null(neighborhood_b);
    
    csmmath_cross_product3D(
                        neighborhood_a->A_face, neighborhood_a->B_face, neighborhood_a->C_face,
                        neighborhood_b->A_face, neighborhood_b->B_face, neighborhood_b->C_face,
                        &Wx_inters, &Wy_inters, &Wz_inters);
    
    tolerance_null_vector = csmtolerance_null_vector();

    if (csmmath_is_null_vector(Wx_inters, Wy_inters, Wz_inters, tolerance_null_vector) == CIERTO)
    {
        return i_sectors_overlap(neighborhood_a, neighborhood_b);
    }
    else
    {
        CYBOOL is_within_a, is_within_b;
        
        is_within_a = i_is_intersection_within_sector(neighborhood_a, Wx_inters, Wy_inters, Wz_inters);
        is_within_b = i_is_intersection_within_sector(neighborhood_b, Wx_inters, Wy_inters, Wz_inters);
        
        if (is_within_a == CIERTO && is_within_b == CIERTO)
        {
            return CIERTO;
        }
        else
        {
            is_within_a = i_is_intersection_within_sector(neighborhood_a, -Wx_inters, -Wy_inters, -Wz_inters);
            is_within_b = i_is_intersection_within_sector(neighborhood_b, -Wx_inters, -Wy_inters, -Wz_inters);
            
            if (is_within_a == CIERTO && is_within_b == CIERTO)
                return CIERTO;
            else
                return FALSO;
        }
    }
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_inters_sectors_t *, i_create_intersection_between_sectors, (
                        const struct i_neighborhood_t *neighborhood_a, unsigned long idx_nba,
                        const struct i_neighborhood_t *neighborhood_b, unsigned long idx_nbb))
{
    enum csmsetop_classify_resp_solid_t s1a, s2a, s1b, s2b;
    CYBOOL intersect;
    
    assert_no_null(neighborhood_a);
    assert_no_null(neighborhood_b);
    
    s1a = i_classify_vector_resp_sector(neighborhood_b, neighborhood_a->Ux1, neighborhood_a->Uy1, neighborhood_a->Uz1);
    s2a = i_classify_vector_resp_sector(neighborhood_b, neighborhood_a->Ux2, neighborhood_a->Uy2, neighborhood_a->Uz2);
    
    s1b = i_classify_vector_resp_sector(neighborhood_a, neighborhood_b->Ux1, neighborhood_b->Uy1, neighborhood_b->Uz1);
    s2b = i_classify_vector_resp_sector(neighborhood_a, neighborhood_b->Ux2, neighborhood_b->Uy2, neighborhood_b->Uz2);
    
    intersect = CIERTO;
    
    return i_create_inters_sectors(idx_nba, idx_nbb, s1a, s2a, s1b, s2b, intersect);
}

// ----------------------------------------------------------------------------------------------------

static void i_print_debug_info_vertex_neighborhood(const char *description, const struct csmvertex_t *vertex, ArrEstructura(i_neighborhood_t) *vertex_neighborhood)
{
    if (csmdebug_debug_enabled() == CIERTO)
    {
        double x, y, z;
        unsigned long i, num_sectors;
        
        csmvertex_get_coordenadas(vertex, &x, &y, &z);
        csmdebug_print_debug_info("Vertex neighborhood [%s]: %lu (%g, %g, %g): \n", description, csmvertex_id(vertex), x, y, z);
        
        num_sectors = arr_NumElemsPunteroST(vertex_neighborhood, i_neighborhood_t);
        
        for (i = 0; i < num_sectors; i++)
        {
            struct i_neighborhood_t *hedge_neighborhood;
            
            hedge_neighborhood = arr_GetPunteroST(vertex_neighborhood, i, i_neighborhood_t);
            assert_no_null(hedge_neighborhood);
            
            csmdebug_print_debug_info("He: %lu\n", csmhedge_id(hedge_neighborhood->he));
            csmdebug_print_debug_info("U1:  %g, %g, %g\n", hedge_neighborhood->Ux1, hedge_neighborhood->Uy1, hedge_neighborhood->Uz1);
            csmdebug_print_debug_info("U2:  %g, %g, %g\n", hedge_neighborhood->Ux2, hedge_neighborhood->Uy2, hedge_neighborhood->Uz2);
            csmdebug_print_debug_info("U12: %g, %g, %g\n", hedge_neighborhood->Ux12, hedge_neighborhood->Uy12, hedge_neighborhood->Uz12);
            csmdebug_print_debug_info("Face Normal: %g, %g, %g\n", hedge_neighborhood->A_face, hedge_neighborhood->B_face, hedge_neighborhood->C_face);
        }
        
        csmdebug_print_debug_info("\n");
    }
}

// ------------------------------------------------------------------------------------------

static void i_generate_neighboorhoods(
                        struct csmvertex_t *vertex_a, struct csmvertex_t *vertex_b,
                        ArrEstructura(i_neighborhood_t) **neighborhood_A,
                        ArrEstructura(i_neighborhood_t) **neighborhood_B,
                        ArrEstructura(i_inters_sectors_t) **neighborhood_intersections)
{
    ArrEstructura(i_neighborhood_t) *neighborhood_A_loc, *neighborhood_B_loc;
    ArrEstructura(i_inters_sectors_t) *neighborhood_intersections_loc;
    unsigned long i, num_neighborhoods_a, num_neighborhoods_b;

    assert(csmvertex_equal_coords(vertex_a, vertex_b, csmtolerance_equal_coords()) == CIERTO);
    assert_no_null(neighborhood_A);
    assert_no_null(neighborhood_B);
    assert_no_null(neighborhood_intersections);
    
    neighborhood_A_loc = i_preprocess_neighborhood(vertex_a);
    neighborhood_B_loc = i_preprocess_neighborhood(vertex_b);
    
    i_print_debug_info_vertex_neighborhood("nba", vertex_a, neighborhood_A_loc);
    i_print_debug_info_vertex_neighborhood("nbb", vertex_b, neighborhood_B_loc);
    
    neighborhood_intersections_loc = arr_CreaPunteroST(0, i_inters_sectors_t);
    
    num_neighborhoods_a = arr_NumElemsPunteroST(neighborhood_A_loc, i_neighborhood_t);
    num_neighborhoods_b = arr_NumElemsPunteroST(neighborhood_B_loc, i_neighborhood_t);
    
    for (i = 0; i < num_neighborhoods_a; i++)
    {
        const struct i_neighborhood_t *neighborhood_a;
        unsigned long j;
        
        neighborhood_a = arr_GetPunteroConstST(neighborhood_A_loc, i, i_neighborhood_t);
        
        for (j = 0; j < num_neighborhoods_b; j++)
        {
            const struct i_neighborhood_t *neighborhood_b;
            
            neighborhood_b = arr_GetPunteroConstST(neighborhood_B_loc, j, i_neighborhood_t);
            
            if (i_exists_intersection_between_sectors(neighborhood_a, neighborhood_b) == CIERTO)
            {
                struct i_inters_sectors_t *inters_sectors;
                
                inters_sectors = i_create_intersection_between_sectors(neighborhood_a, i, neighborhood_b, j);
                arr_AppendPunteroST(neighborhood_intersections_loc, inters_sectors, i_inters_sectors_t);
            }
        }
    }
    
    *neighborhood_A = neighborhood_A_loc;
    *neighborhood_B = neighborhood_B_loc;
    *neighborhood_intersections = neighborhood_intersections_loc;
}

// ------------------------------------------------------------------------------------------

static void i_reclassify_on_sectors(
                        enum csmsetop_operation_t set_operation,
                        ArrEstructura(i_neighborhood_t) *neighborhood_A, ArrEstructura(i_neighborhood_t) *neighborhood_B,
                        ArrEstructura(i_inters_sectors_t) *neighborhood_intersections)
{
    unsigned long num_sectors_a, num_sectors_b;
    unsigned long i, num_sectors;
    
    num_sectors_a = arr_NumElemsPunteroST(neighborhood_A, i_neighborhood_t);
    num_sectors_b = arr_NumElemsPunteroST(neighborhood_B, i_neighborhood_t);
    
    num_sectors = arr_NumElemsPunteroST(neighborhood_intersections, i_inters_sectors_t);
    
    for (i = 0; i < num_sectors; i++)
    {
        struct i_inters_sectors_t *sector_i;
        
        sector_i = arr_GetPunteroST(neighborhood_intersections, i, i_inters_sectors_t);
        assert_no_null(sector_i);
        
        if (sector_i->s1a == CSMSETOP_CLASSIFY_RESP_SOLID_ON
                && sector_i->s2a == CSMSETOP_CLASSIFY_RESP_SOLID_ON
                && sector_i->s1b == CSMSETOP_CLASSIFY_RESP_SOLID_ON
                && sector_i->s2b == CSMSETOP_CLASSIFY_RESP_SOLID_ON)
        {
            struct i_neighborhood_t *nba, *nbb;
            unsigned long idx_prev_sector_a, idx_next_sector_a;
            unsigned long idx_prev_sector_b, idx_next_sector_b;
            double dot_product;
            enum csmsetop_classify_resp_solid_t newsa, newsb;
            unsigned long j;
            
            nba = arr_GetPunteroST(neighborhood_A, sector_i->idx_nba, i_neighborhood_t);
            assert_no_null(nba);

            nbb = arr_GetPunteroST(neighborhood_B, sector_i->idx_nbb, i_neighborhood_t);
            assert_no_null(nbb);
            
            idx_prev_sector_a = csmmath_prev_idx(sector_i->idx_nba, num_sectors_a);
            idx_next_sector_a = csmmath_next_idx(sector_i->idx_nba, num_sectors_a);
        
            idx_prev_sector_b = csmmath_prev_idx(sector_i->idx_nbb, num_sectors_b);
            idx_next_sector_b = csmmath_next_idx(sector_i->idx_nbb, num_sectors_b);
            
            dot_product = csmmath_dot_product3D(nba->A_face, nba->B_face, nba->C_face, nbb->A_face, nbb->B_face, nbb->C_face);
            
            if (dot_product > 0.)
            {
                newsa = (set_operation == CSMSETOP_OPERATION_UNION) ? CSMSETOP_CLASSIFY_RESP_SOLID_OUT: CSMSETOP_CLASSIFY_RESP_SOLID_IN;
                newsb = (set_operation == CSMSETOP_OPERATION_UNION) ? CSMSETOP_CLASSIFY_RESP_SOLID_IN: CSMSETOP_CLASSIFY_RESP_SOLID_OUT;
            }
            else
            {
                assert(dot_product < 0.);
                
                newsa = (set_operation == CSMSETOP_OPERATION_UNION) ? CSMSETOP_CLASSIFY_RESP_SOLID_IN: CSMSETOP_CLASSIFY_RESP_SOLID_OUT;
                newsb = (set_operation == CSMSETOP_OPERATION_UNION) ? CSMSETOP_CLASSIFY_RESP_SOLID_IN: CSMSETOP_CLASSIFY_RESP_SOLID_OUT;
            }
            
            for (j = 0; j < num_sectors; j++)
            {
                struct i_inters_sectors_t *sector_j;
                
                sector_j = arr_GetPunteroST(neighborhood_intersections, j, i_inters_sectors_t);
                assert_no_null(sector_j);
                
                if (sector_j->idx_nba == idx_prev_sector_a && sector_j->idx_nbb == sector_i->idx_nbb)
                {
                    if (sector_j->s1a != CSMSETOP_CLASSIFY_RESP_SOLID_ON)
                        sector_j->s2a = newsa;
                }
                
                if (sector_j->idx_nba == idx_next_sector_a && sector_j->idx_nbb == sector_i->idx_nbb)
                {
                    if (sector_j->s2a != CSMSETOP_CLASSIFY_RESP_SOLID_ON)
                        sector_j->s1a = newsa;
                }

                if (sector_j->idx_nba == sector_i->idx_nba && sector_j->idx_nbb == idx_prev_sector_b)
                {
                    if (sector_j->s1b != CSMSETOP_CLASSIFY_RESP_SOLID_ON)
                        sector_j->s2b = newsb;
                }

                if (sector_j->idx_nba == sector_i->idx_nba && sector_j->idx_nbb == idx_next_sector_b)
                {
                    if (sector_j->s2b != CSMSETOP_CLASSIFY_RESP_SOLID_ON)
                        sector_j->s1b = newsb;
                }
                
                if (sector_j->s1a == sector_j->s2a
                        && (sector_j->s1a == CSMSETOP_CLASSIFY_RESP_SOLID_IN || sector_j->s1a == CSMSETOP_CLASSIFY_RESP_SOLID_OUT))
                {
                    sector_j->intersect = FALSO;
                }
                
                if (sector_j->s1b == sector_j->s2b
                        && (sector_j->s1b == CSMSETOP_CLASSIFY_RESP_SOLID_IN || sector_j->s1b == CSMSETOP_CLASSIFY_RESP_SOLID_OUT))
                {
                    sector_j->intersect = FALSO;
                }
                
                sector_i->s1a = newsa;
                sector_i->s1a = newsa;
                sector_i->s1b = newsb;
                sector_i->s1b = newsb;
                sector_i->intersect = FALSO;
            }
        }
    }
}

// ------------------------------------------------------------------------------------------

static void i_reclasssify_double_on_edges(
                        enum csmsetop_operation_t set_operation,
                        ArrEstructura(i_neighborhood_t) *neighborhood_A, ArrEstructura(i_neighborhood_t) *neighborhood_B,
                        ArrEstructura(i_inters_sectors_t) *neighborhood_intersections)
{
    unsigned long num_sectors_a, num_sectors_b;
    unsigned long i, num_sectors;
    
    num_sectors_a = arr_NumElemsPunteroST(neighborhood_A, i_neighborhood_t);
    num_sectors_b = arr_NumElemsPunteroST(neighborhood_B, i_neighborhood_t);
    
    num_sectors = arr_NumElemsPunteroST(neighborhood_intersections, i_inters_sectors_t);
    
    for (i = 0; i < num_sectors; i++)
    {
        struct i_inters_sectors_t *sector_i;
        
        sector_i = arr_GetPunteroST(neighborhood_intersections, i, i_inters_sectors_t);
        assert_no_null(sector_i);
        
        if(sector_i->intersect == CIERTO
                && sector_i->s1a == CSMSETOP_CLASSIFY_RESP_SOLID_ON
                && sector_i->s1b == CSMSETOP_CLASSIFY_RESP_SOLID_ON)
        {
            struct i_neighborhood_t *nba, *nbb;
            unsigned long idx_prev_sector_a, idx_prev_sector_b;
            enum csmsetop_classify_resp_solid_t newsa, newsb;
            unsigned long j;
            
            nba = arr_GetPunteroST(neighborhood_A, sector_i->idx_nba, i_neighborhood_t);
            assert_no_null(nba);

            nbb = arr_GetPunteroST(neighborhood_B, sector_i->idx_nbb, i_neighborhood_t);
            assert_no_null(nbb);
            
            idx_prev_sector_a = csmmath_prev_idx(sector_i->idx_nba, num_sectors_a);
            idx_prev_sector_b = csmmath_prev_idx(sector_i->idx_nbb, num_sectors_b);
            
            newsa = (set_operation == CSMSETOP_OPERATION_UNION) ? CSMSETOP_CLASSIFY_RESP_SOLID_OUT: CSMSETOP_CLASSIFY_RESP_SOLID_IN;
            newsb = (set_operation == CSMSETOP_OPERATION_UNION) ? CSMSETOP_CLASSIFY_RESP_SOLID_IN: CSMSETOP_CLASSIFY_RESP_SOLID_OUT;

            for(j = 0; j < num_sectors; j++)
            {
                struct i_inters_sectors_t *sector_j;
                
                sector_j = arr_GetPunteroST(neighborhood_intersections, j, i_inters_sectors_t);
                assert_no_null(sector_j);
                
                if (sector_j->intersect == CIERTO)
                {
                    if (sector_j->idx_nba == sector_i->idx_nba && sector_j->idx_nbb == sector_i->idx_nbb)
                    {
                        sector_j->s1a = newsa;
                        sector_j->s1b = newsb;
                    }

                    if (sector_j->idx_nba == idx_prev_sector_a && sector_j->idx_nbb == sector_i->idx_nbb)
                    {
                        sector_j->s2a = newsa;
                        sector_j->s1b = newsb;
                    }

                    if (sector_j->idx_nba == sector_i->idx_nba && sector_j->idx_nbb == idx_prev_sector_b)
                    {
                        sector_j->s1a = newsa;
                        sector_j->s2b = newsb;
                    }
                    
                    if (sector_j->idx_nba == idx_prev_sector_a && sector_j->idx_nbb == idx_prev_sector_b)
                    {
                        sector_j->s2a = newsa;
                        sector_j->s2b = newsb;
                    }
                    
                    if (sector_j->s1a == sector_j->s2a
                            && (sector_j->s1a == CSMSETOP_CLASSIFY_RESP_SOLID_IN || sector_j->s1a == CSMSETOP_CLASSIFY_RESP_SOLID_OUT))
                    {
                        sector_j->intersect = FALSO;
                    }

                    if (sector_j->s1b == sector_j->s2b
                            && (sector_j->s1b == CSMSETOP_CLASSIFY_RESP_SOLID_IN || sector_j->s1b == CSMSETOP_CLASSIFY_RESP_SOLID_OUT))
                    {
                        sector_j->intersect = FALSO;
                    }
                }
            }
        }
    }
}

// ------------------------------------------------------------------------------------------

static void i_reclasssify_single_on_edges(
                        enum csmsetop_operation_t set_operation,
                        ArrEstructura(i_neighborhood_t) *neighborhood_A, ArrEstructura(i_neighborhood_t) *neighborhood_B,
                        ArrEstructura(i_inters_sectors_t) *neighborhood_intersections)
{
    unsigned long num_sectors_a, num_sectors_b;
    unsigned long i, num_sectors;
    
    num_sectors_a = arr_NumElemsPunteroST(neighborhood_A, i_neighborhood_t);
    num_sectors_b = arr_NumElemsPunteroST(neighborhood_B, i_neighborhood_t);
    
    num_sectors = arr_NumElemsPunteroST(neighborhood_intersections, i_inters_sectors_t);

    for (i = 0; i < num_sectors; i++)
    {
        struct i_inters_sectors_t *sector_i;
        struct i_neighborhood_t *nba, *nbb;
        unsigned long idx_prev_sector_a, idx_prev_sector_b;
        
        sector_i = arr_GetPunteroST(neighborhood_intersections, i, i_inters_sectors_t);
        assert_no_null(sector_i);

        nba = arr_GetPunteroST(neighborhood_A, sector_i->idx_nba, i_neighborhood_t);
        assert_no_null(nba);

        nbb = arr_GetPunteroST(neighborhood_B, sector_i->idx_nbb, i_neighborhood_t);
        assert_no_null(nbb);
        
        idx_prev_sector_a = csmmath_prev_idx(sector_i->idx_nba, num_sectors_a);
        idx_prev_sector_b = csmmath_prev_idx(sector_i->idx_nbb, num_sectors_b);
        
        if(sector_i->intersect == CIERTO && sector_i->s1a == CSMSETOP_CLASSIFY_RESP_SOLID_ON)
        {
            enum csmsetop_classify_resp_solid_t newsa;
            unsigned long j;
            
            newsa = (set_operation == CSMSETOP_OPERATION_UNION) ? CSMSETOP_CLASSIFY_RESP_SOLID_OUT: CSMSETOP_CLASSIFY_RESP_SOLID_IN;
    
            for(j = 0; j < num_sectors; j++)
            {
                struct i_inters_sectors_t *sector_j;
                
                sector_j = arr_GetPunteroST(neighborhood_intersections, j, i_inters_sectors_t);
                assert_no_null(sector_j);
                
                if (sector_j->intersect == CIERTO)
                {
                    if (sector_j->idx_nba == sector_i->idx_nba && sector_j->idx_nbb == sector_i->idx_nbb)
                        sector_j->s1a = newsa;
                    
                    if (sector_j->idx_nba == idx_prev_sector_a && sector_j->idx_nbb == sector_i->idx_nbb)
                        sector_j->s2a = newsa;
                    
                    if (sector_j->s1a == sector_j->s2a
                            && (sector_j->s1a == CSMSETOP_CLASSIFY_RESP_SOLID_IN || sector_j->s1a == CSMSETOP_CLASSIFY_RESP_SOLID_OUT))
                    {
                        sector_j->intersect = FALSO;
                    }
                }
            }
        }
        else if(sector_i->intersect == CIERTO && sector_i->s1b == CSMSETOP_CLASSIFY_RESP_SOLID_ON)
        {
            enum csmsetop_classify_resp_solid_t newsb;
            unsigned long j;
            
            newsb = (set_operation == CSMSETOP_OPERATION_UNION) ? CSMSETOP_CLASSIFY_RESP_SOLID_OUT: CSMSETOP_CLASSIFY_RESP_SOLID_IN;
    
            for(j = 0; j < num_sectors; j++)
            {
                struct i_inters_sectors_t *sector_j;
                
                sector_j = arr_GetPunteroST(neighborhood_intersections, j, i_inters_sectors_t);
                assert_no_null(sector_j);
                
                if (sector_j->intersect == CIERTO)
                {
                    if (sector_j->idx_nba == sector_i->idx_nba && sector_j->idx_nbb == sector_i->idx_nbb)
                        sector_j->s1b = newsb;
                    
                    if (sector_j->idx_nba == sector_j->idx_nba && sector_j->idx_nbb == idx_prev_sector_b)
                        sector_j->s2b = newsb;
                    
                    if (sector_j->s1b == sector_j->s2b
                            && (sector_j->s1b == CSMSETOP_CLASSIFY_RESP_SOLID_IN || sector_j->s1b == CSMSETOP_CLASSIFY_RESP_SOLID_OUT))
                    {
                        sector_j->intersect = FALSO;
                    }
                }
            }
        }
    }
}

// ------------------------------------------------------------------------------------------

static void i_reclasssify_on_edges(
                        enum csmsetop_operation_t set_operation,
                        ArrEstructura(i_neighborhood_t) *neighborhood_A, ArrEstructura(i_neighborhood_t) *neighborhood_B,
                        ArrEstructura(i_inters_sectors_t) *neighborhood_intersections)
{
    i_reclasssify_double_on_edges(
                        set_operation,
                        neighborhood_A, neighborhood_B,
                        neighborhood_intersections);
    
    i_reclasssify_single_on_edges(
                        set_operation,
                        neighborhood_A, neighborhood_B,
                        neighborhood_intersections);
}

// ------------------------------------------------------------------------------------------

static struct i_inters_sectors_t *i_get_next_sector(ArrEstructura(i_inters_sectors_t) *neighborhood_intersections, unsigned long *last_idx)
{
    struct i_inters_sectors_t *intersection;
    unsigned long num_intersections;
    
    num_intersections = arr_NumElemsPunteroST(neighborhood_intersections, i_inters_sectors_t);
    assert_no_null(last_idx);
    
    intersection = NULL;
    
    while (*last_idx < num_intersections)
    {
        intersection = arr_GetPunteroST(neighborhood_intersections, *last_idx, i_inters_sectors_t);
        assert_no_null(intersection);
        
        if (intersection->intersect == FALSO)
        {
            (*last_idx)++;
            intersection = NULL;
        }
        else
        {
            break;
        }
    }
    
    return intersection;
}

// ------------------------------------------------------------------------------------------

static CYBOOL i_is_null_edge(struct csmhedge_t *he)
{
    struct csmvertex_t *vertex, *vertex_nxt;
    
    vertex = csmhedge_vertex(he);
    vertex_nxt = csmhedge_vertex(csmhedge_next(he));
    
    return csmvertex_equal_coords(vertex, vertex_nxt, csmtolerance_equal_coords());
}

// ------------------------------------------------------------------------------------------

static CYBOOL i_is_strutnulledge(struct csmhedge_t *he)
{
    struct csmhedge_t *he_mate;
    
    he_mate = csmopbas_mate(he);
    
    if (he == csmhedge_next(he_mate) || he == csmhedge_prev(he_mate))
        return CIERTO;
    else
        return FALSO;
}

// ------------------------------------------------------------------------------------------

static void i_face_normal_of_he_face(struct csmhedge_t *he, double *A, double *B, double *C)
{
    struct csmface_t *face;
    double D;
    
    face = csmopbas_face_from_hedge(he);
    csmface_face_equation(face, A, B, C, &D);
}

// ------------------------------------------------------------------------------------------

static CYBOOL i_is_convex_edge(struct csmhedge_t *he)
{
    double A_he, B1_he, C_he;
    struct csmhedge_t *mate_he;
    double A_mate_he, B1_mate_he, C_mate_he;
    double Wx, Wy, Wz;

    i_face_normal_of_he_face(he, &A_he, &B1_he, &C_he);
    
    mate_he = csmopbas_mate(he);
    i_face_normal_of_he_face(mate_he, &A_mate_he, &B1_mate_he, &C_mate_he);
    
    csmmath_cross_product3D(A_he, B1_he, C_he, A_mate_he, B1_mate_he, C_mate_he, &Wx, &Wy, &Wz);
    
    if (csmmath_is_null_vector(Wx, Wy, Wz, csmtolerance_null_vector()) == CIERTO)
    {
        return CIERTO;
    }
    else
    {
        struct csmhedge_t *he2;
        struct csmvertex_t *vertex_he, *vertex_he2;
        double Ux, Uy, Uz;
        double dot_product;
    
        he2 = csmhedge_next(he);
    
        if (i_is_null_edge(he2) == CIERTO)
            he2 = csmhedge_next(he2);
        
        vertex_he = csmhedge_vertex(he);
        vertex_he2 = csmhedge_vertex(he2);
        csmvertex_vector_from_vertex1_to_vertex2(vertex_he, vertex_he2, &Ux, &Uy, &Uz);
        
        dot_product = csmmath_dot_product3D(Ux, Uy, Uz, Wx, Wy, Wz);
        
        if (dot_product < 0.)
            return CIERTO;
        else
            return FALSO;
    }
}

// ------------------------------------------------------------------------------------------

static CYBOOL i_is_wide_sector(struct csmhedge_t *he)
{
    struct csmvertex_t *vertex, *vertex_prv, *vertex_nxt;
    double Ux1, Uy1, Uz1, Ux2, Uy2, Uz2;
    double Ux12, Uy12, Uz12;
    
    vertex = csmhedge_vertex(he);
    vertex_prv = csmhedge_vertex(csmhedge_prev(he));
    vertex_nxt = csmhedge_vertex(csmhedge_next(he));
    
    csmvertex_vector_from_vertex1_to_vertex2(vertex, vertex_prv, &Ux1, &Uy1, &Uz1);
    csmvertex_vector_from_vertex1_to_vertex2(vertex, vertex_nxt, &Ux2, &Uy2, &Uz2);
    
    csmmath_cross_product3D(Ux1, Uy1, Uz1, Ux2, Uy2, Uz2, &Ux12, &Uy12, &Uz12);
    
    if (csmmath_is_null_vector(Ux12, Uy12, Uz12, csmtolerance_null_vector()))
    {
        return CIERTO;
    }
    else
    {
        double A, B, C;
        double dot_product;
        
        i_face_normal_of_he_face(he, &A, &B, &C);
        dot_product = csmmath_dot_product3D(A, B, C, Ux12, Uy12, Uz12);
        
        if (dot_product > 0.)
            return FALSO;
        else
            return CIERTO;
    }
}

// ------------------------------------------------------------------------------------------

static CYBOOL i_get_orient(struct csmhedge_t *ref, struct csmhedge_t *he1, struct csmhedge_t *he2)
{
    CYBOOL orient;
    struct csmhedge_t *mhe1, *mhe2;
    
    mhe1 = csmhedge_next(csmopbas_mate(he1));
    mhe2 = csmhedge_next(csmopbas_mate(he2));
    
    if (mhe1 != he2 && mhe2 == he1)
        orient = i_is_convex_edge(he2);
    else
        orient = i_is_convex_edge(he1);
    
    if (i_is_wide_sector(mhe1) == CIERTO && i_is_wide_sector(ref) == CIERTO)
        orient = INVIERTE_CYBOOL(orient);
    
    return INVIERTE_CYBOOL(orient);
}

// ------------------------------------------------------------------------------------------

static void i_separateEdgeSequence(
                        struct csmhedge_t *from, struct csmhedge_t *to,
                        ArrEstructura(csmedge_t) *set_of_null_edges)
{
    struct csmhedge_t *from_prv, *to_prv;
    struct csmvertex_t *split_vertex;
    double x, y, z;
    struct csmedge_t *null_edge;
    
    from_prv = csmhedge_prev(from);
    
    /* recover from null edges already inserted */
    if(i_is_null_edge(from_prv) == CIERTO && i_is_strutnulledge(from_prv) == CIERTO)
    {
        struct csmedge_t *from_prv_edge;
        struct csmhedge_t *from_prv_edge_he2;
        
        from_prv_edge = csmhedge_edge(from_prv);
        from_prv_edge_he2 = csmedge_hedge_lado(from_prv_edge, CSMEDGE_LADO_HEDGE_NEG);
        
        /* look at orientation */
        if (from_prv == from_prv_edge_he2)
            from = csmhedge_prev(from_prv);
    }

    to_prv = csmhedge_prev(to);
    
    if(i_is_null_edge(to_prv) == CIERTO && i_is_strutnulledge(to_prv) == CIERTO)
    {
        struct csmedge_t *to_prv_edge;
        struct csmhedge_t *to_prv_edge_he1;
        
        to_prv_edge = csmhedge_edge(to_prv);
        to_prv_edge_he1 = csmedge_hedge_lado(to_prv_edge, CSMEDGE_LADO_HEDGE_POS);
        
        /* look at orientation */
        if (to_prv == to_prv_edge_he1)
            to = csmhedge_prev(to_prv);
    }
    
    if (csmhedge_vertex(from) != csmhedge_vertex(to))
    {
        struct csmhedge_t *from_prv, *to_prv;
        
        from_prv = csmhedge_prev(from);
        to_prv = csmhedge_prev(to);
        
        if (from_prv == csmopbas_mate(to_prv))
        {
            from = from_prv;
        }
        else if(csmhedge_vertex(from_prv) == csmhedge_vertex(to))
        {
            from = from_prv;
        }
        else if(csmhedge_vertex(to_prv) == csmhedge_vertex(from))
        {
            to = to_prv;
        }
    }
    
    split_vertex = csmhedge_vertex(from);
    csmvertex_get_coordenadas(split_vertex, &x, &y, &z);

    if (csmdebug_debug_enabled() == CIERTO)
    {
        char *description;
        
        description = copiafor_codigo4("NE %lu (%g, %g, %g)", csmvertex_id(split_vertex), x, y, z);
        csmdebug_append_debug_point(x, y, z, &description);
    }
    
    csmeuler_lmev(from, to, x, y, z, NULL, &null_edge, NULL, NULL);
    assert_no_null(null_edge);
    
    arr_AppendPunteroST(set_of_null_edges, null_edge, csmedge_t);
}

// ------------------------------------------------------------------------------------------

static void i_separateInteriorHedge(
                        struct csmhedge_t *he,
                        CYBOOL orient,
                        ArrEstructura(csmedge_t) *set_of_null_edges)
{
    struct csmhedge_t *he_prv;
    struct csmvertex_t *split_vertex;
    double x, y, z;
    struct csmedge_t *null_edge1, *null_edge2;
    
    he_prv = csmhedge_prev(he);

    /* recover from null edges inserted */
    if(i_is_null_edge(he_prv) == CIERTO)
    {
        struct csmedge_t *he_prv_edge;
        struct csmhedge_t *he1_edge_he_prv, *he2_edge_he_prv;
        
        he_prv_edge = csmhedge_edge(he_prv);
        he1_edge_he_prv = csmedge_hedge_lado(he_prv_edge, CSMEDGE_LADO_HEDGE_POS);
        he2_edge_he_prv = csmedge_hedge_lado(he_prv_edge, CSMEDGE_LADO_HEDGE_NEG);
        
        if (he_prv == he1_edge_he_prv && orient == CIERTO)
            he = he_prv;
        else if (he_prv == he2_edge_he_prv && orient == FALSO)
            he = he_prv;
    }
    
    split_vertex = csmhedge_vertex(he);
    csmvertex_get_coordenadas(split_vertex, &x, &y, &z);
    
    csmeuler_lmev(he, he, x, y, z, NULL, &null_edge1, NULL, NULL);

    if (csmdebug_debug_enabled() == CIERTO)
    {
        char *description;
        
        description = copiafor_codigo4("NE %lu (%g, %g, %g)", csmvertex_id(split_vertex), x, y, z);
        csmdebug_append_debug_point(x, y, z, &description);
    }
    
    /* a piece of Black Art: reverse orientation of the null edge */
    if (orient == CIERTO)
    {
        struct csmedge_t *he_prv_edge;
        struct csmhedge_t *he1_edge_he_prv, *he2_edge_he_prv;
        
        he_prv = csmhedge_prev(he);
        he_prv_edge = csmhedge_edge(he_prv);
        he1_edge_he_prv = csmedge_hedge_lado(he_prv_edge, CSMEDGE_LADO_HEDGE_POS);
        he2_edge_he_prv = csmedge_hedge_lado(he_prv_edge, CSMEDGE_LADO_HEDGE_NEG);
        
        csmedge_set_edge_lado(he_prv_edge, CSMEDGE_LADO_HEDGE_NEG, he1_edge_he_prv);
        csmedge_set_edge_lado(he_prv_edge, CSMEDGE_LADO_HEDGE_POS, he2_edge_he_prv);
    }
    
    null_edge2 = csmhedge_edge(csmhedge_prev(he));
    assert(null_edge1 == null_edge2);
    
    arr_AppendPunteroST(set_of_null_edges, null_edge2, csmedge_t);
}

// ------------------------------------------------------------------------------------------

static void i_insert_null_edges(
                        ArrEstructura(i_neighborhood_t) *neighborhood_A, ArrEstructura(i_neighborhood_t) *neighborhood_B,
                        ArrEstructura(i_inters_sectors_t) *neighborhood_intersections,
                        ArrEstructura(csmedge_t) *set_of_null_edges_A,
                        ArrEstructura(csmedge_t) *set_of_null_edges_B)
{
    struct csmhedge_t *ha1, *ha2, *hb1, *hb2;
    unsigned long num_iters;
    unsigned long last_idx;
    
    num_iters = 0;
    last_idx = 0;
    
    while (1)
    {
        struct i_inters_sectors_t *sector;
        
        assert(num_iters < 10000);
        num_iters++;
        
        sector = i_get_next_sector(neighborhood_intersections, &last_idx);
        
        if (sector == NULL)
        {
            break;
        }
        else
        {
            struct i_neighborhood_t *nba, *nbb;
            
            ha1 = NULL;
            ha2 = NULL;
            
            hb1 = NULL;
            hb2 = NULL;
            
            nba = arr_GetPunteroST(neighborhood_A, sector->idx_nba, i_neighborhood_t);
            assert_no_null(nba);

            nbb = arr_GetPunteroST(neighborhood_B, sector->idx_nbb, i_neighborhood_t);
            assert_no_null(nbb);
            
            if (sector->s1a == CSMSETOP_CLASSIFY_RESP_SOLID_OUT)
                ha1 = nba->he;
            else
                ha2 = nba->he;
            
            if (sector->s1b == CSMSETOP_CLASSIFY_RESP_SOLID_IN)
                hb1 = nbb->he;
            else
                hb2 = nbb->he;
            
            last_idx++;
            sector = i_get_next_sector(neighborhood_intersections, &last_idx);
            
            if (sector == NULL)
            {
                break;
            }
            else
            {
                nba = arr_GetPunteroST(neighborhood_A, sector->idx_nba, i_neighborhood_t);
                assert_no_null(nba);

                nbb = arr_GetPunteroST(neighborhood_B, sector->idx_nbb, i_neighborhood_t);
                assert_no_null(nbb);
                
                if (sector->s1a == CSMSETOP_CLASSIFY_RESP_SOLID_OUT)
                {
                    assert(ha1 == NULL);
                    assert(ha2 != NULL);
                    
                    ha1 = nba->he;
                }
                else
                {
                    assert(ha1 != NULL);
                    assert(ha2 == NULL);
                    
                    ha2 = nba->he;
                }
                
                if (sector->s1b == CSMSETOP_CLASSIFY_RESP_SOLID_IN)
                {
                    assert(hb1 == NULL);
                    assert(hb2 != NULL);
                    
                    hb1 = nbb->he;
                }
                else
                {
                    assert(hb1 != NULL);
                    assert(hb2 == NULL);
                    
                    hb2 = nbb->he;
                }
                
                last_idx++;
                
                assert_no_null(ha1);
                assert_no_null(ha2);
                assert_no_null(hb1);
                assert_no_null(hb2);
                
                if (ha1 == ha2)
                {
                    CYBOOL orient;
                    
                    orient = i_get_orient(ha1, hb1, hb2);
                    i_separateInteriorHedge(ha1, orient, set_of_null_edges_A);
                    
                    i_separateEdgeSequence(hb1, hb2, set_of_null_edges_B);
                }
                else if (hb1 == hb2)
                {
                    CYBOOL orient;
                    
                    orient = i_get_orient(hb1, ha2, ha1);
                    i_separateInteriorHedge(hb1, orient, set_of_null_edges_B);
                    
                    i_separateEdgeSequence(ha2, ha1, set_of_null_edges_A);
                }
                else
                {
                    i_separateEdgeSequence(ha2, ha1, set_of_null_edges_A);
                    i_separateEdgeSequence(hb1, hb2, set_of_null_edges_B);
                }
            }
        }
    }
}

// ------------------------------------------------------------------------------------------

static const char *i_debug_text_for_classification(enum csmsetop_classify_resp_solid_t cl)
{
    switch (cl)
    {
        case CSMSETOP_CLASSIFY_RESP_SOLID_IN:  return " IN";
        case CSMSETOP_CLASSIFY_RESP_SOLID_OUT: return "OUT";
        case CSMSETOP_CLASSIFY_RESP_SOLID_ON:  return " ON";
        default_error();
    }
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static char *, i_debug_text_for_sector_classification, (enum csmsetop_classify_resp_solid_t s1, enum csmsetop_classify_resp_solid_t s2))
{
    const char *text_s1, *text_s2;
    
    text_s1 = i_debug_text_for_classification(s1);
    text_s2 = i_debug_text_for_classification(s2);

    return copiafor_codigo2("(%s - %s)", text_s1, text_s2);
}

// ------------------------------------------------------------------------------------------

static void i_print_neighborhood_intersections(
                        const ArrEstructura(i_inters_sectors_t) *neighborhood_intersections,
                        const ArrEstructura(i_neighborhood_t) *neighborhood_A, const ArrEstructura(i_neighborhood_t) *neighborhood_B)
{
    if (csmdebug_debug_enabled() == CIERTO)
    {
        unsigned long i, no_inters;
        
        csmdebug_print_debug_info("\n");
        
        no_inters = arr_NumElemsPunteroST(neighborhood_intersections, i_inters_sectors_t);
        
        for (i = 0; i < no_inters; i++)
        {
            const struct i_inters_sectors_t *inters_sectors;
            const struct i_neighborhood_t *nba, *nbb;
            char *text_cla, *text_clb;
            
            inters_sectors = arr_GetPunteroST(neighborhood_intersections, i, i_inters_sectors_t);
            assert_no_null(inters_sectors);
            
            nba = arr_GetPunteroST(neighborhood_A, inters_sectors->idx_nba, i_neighborhood_t);
            assert_no_null(nba);

            nbb = arr_GetPunteroST(neighborhood_B, inters_sectors->idx_nbb, i_neighborhood_t);
            assert_no_null(nbb);
            
            text_cla = i_debug_text_for_sector_classification(inters_sectors->s1a, inters_sectors->s2a);
            text_clb = i_debug_text_for_sector_classification(inters_sectors->s1b, inters_sectors->s2b);
            
            csmdebug_print_debug_info(
                        "(hea %lu) (heb %lu) %s %s Intersect: %lu\n",
                        csmhedge_id(nba->he),
                        csmhedge_id(nbb->he),
                        text_cla, text_clb,
                        inters_sectors->intersect);
        
            cypestr_destruye(&text_cla);
            cypestr_destruye(&text_clb);
        }
    }
}

// ------------------------------------------------------------------------------------------

static void i_vtxvtx_append_null_edges(
                        const struct csmsetop_vtxvtx_inters_t *vv_intersection,
                        enum csmsetop_operation_t set_operation,
                        ArrEstructura(csmedge_t) *set_of_null_edges_A,
                        ArrEstructura(csmedge_t) *set_of_null_edges_B)
{
    ArrEstructura(i_neighborhood_t) *neighborhood_A, *neighborhood_B;
    ArrEstructura(i_inters_sectors_t) *neighborhood_intersections;
    
    assert_no_null(vv_intersection);
    
    {
        char *description;
        
        description = copiafor_codigo2("VV Intersection. Va %lu - Vb %lu", csmvertex_id(vv_intersection->vertex_a), csmvertex_id(vv_intersection->vertex_b));
        csmdebug_begin_context(description);
        cypestr_destruye(&description);
    }
    
    i_generate_neighboorhoods(
                        vv_intersection->vertex_a, vv_intersection->vertex_b,
                        &neighborhood_A, &neighborhood_B,
                        &neighborhood_intersections);
    
    i_print_neighborhood_intersections(neighborhood_intersections, neighborhood_A, neighborhood_B);
    
    i_reclassify_on_sectors(
                        set_operation,
                        neighborhood_A, neighborhood_B,
                        neighborhood_intersections);
    
    i_print_neighborhood_intersections(neighborhood_intersections, neighborhood_A, neighborhood_B);
    
    i_reclasssify_on_edges(
                        set_operation,
                        neighborhood_A, neighborhood_B,
                        neighborhood_intersections);
    
    i_print_neighborhood_intersections(neighborhood_intersections, neighborhood_A, neighborhood_B);

    i_insert_null_edges(
                        neighborhood_A, neighborhood_B,
                        neighborhood_intersections,
                        set_of_null_edges_A,
                        set_of_null_edges_B);
    
    csmdebug_end_context();
    
    arr_DestruyeEstructurasST(&neighborhood_A, i_free_neighborhood, i_neighborhood_t);
    arr_DestruyeEstructurasST(&neighborhood_B, i_free_neighborhood, i_neighborhood_t);
    arr_DestruyeEstructurasST(&neighborhood_intersections, i_free_inters_sectors, i_inters_sectors_t);
}

// ------------------------------------------------------------------------------------------

void csmsetop_vtxvtx_append_null_edges(
                        const ArrEstructura(csmsetop_vtxvtx_inters_t) *vv_intersections,
                        enum csmsetop_operation_t set_operation,
                        ArrEstructura(csmedge_t) *set_of_null_edges_A,
                        ArrEstructura(csmedge_t) *set_of_null_edges_B)
{
    unsigned long i, num_intersections;
    
    num_intersections = arr_NumElemsPunteroST(vv_intersections, csmsetop_vtxvtx_inters_t);
    
    csmdebug_clear_debug_points();
    
    for (i = 0; i < num_intersections; i++)
    {
        const struct csmsetop_vtxvtx_inters_t *vv_intersection;
        
        vv_intersection = arr_GetPunteroConstST(vv_intersections, i, csmsetop_vtxvtx_inters_t);
        i_vtxvtx_append_null_edges(vv_intersection, set_operation, set_of_null_edges_A, set_of_null_edges_B);
    }
    
    //csmdebug_show_viewer();
}























