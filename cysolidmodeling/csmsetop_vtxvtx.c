// Boolean operations on solids: Vertex - Vertex classifier...

#include "csmsetop_vtxvtx.inl"

#include "csmmath.inl"
#include "csmhedge.inl"
#include "csmface.inl"
#include "csmopbas.inl"
#include "csmsetop.tli"
#include "csmsetopcom.inl"
#include "csmtolerance.inl"
#include "csmvertex.inl"

#include "a_punter.h"
#include "cyassert.h"
#include "cypespy.h"
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

    assert_no_null(neighborhood_A);
    assert_no_null(neighborhood_B);
    assert_no_null(neighborhood_intersections);
    
    neighborhood_A_loc = i_preprocess_neighborhood(vertex_a);
    neighborhood_B_loc = i_preprocess_neighborhood(vertex_b);
    
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

static void i_hedge_plane_equation(struct csmhedge_t *he, double *A, double *B, double *C)
{
    struct csmface_t *face;
    double D;
    
    face = csmopbas_face_from_hedge(he);
    csmface_face_equation(face, A, B, C, &D);
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

static void i_vtxvtx_append_null_edges(
                        const struct csmsetop_vtxvtx_inters_t *vv_intersection,
                        enum csmsetop_operation_t set_operation,
                        ArrEstructura(csmedge_t) *set_of_null_edges_A,
                        ArrEstructura(csmedge_t) *set_of_null_edges_B)
{
    ArrEstructura(i_neighborhood_t) *neighborhood_A, *neighborhood_B;
    ArrEstructura(i_inters_sectors_t) *neighborhood_intersections;
    
    assert_no_null(vv_intersection);
    
    i_generate_neighboorhoods(
                        vv_intersection->vertex_a, vv_intersection->vertex_b,
                        &neighborhood_A, &neighborhood_B,
                        &neighborhood_intersections);
    
    i_reclassify_on_sectors(
                        set_operation,
                        neighborhood_A, neighborhood_B,
                        neighborhood_intersections);
    
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
    
    for (i = 0; i < num_intersections; i++)
    {
        const struct csmsetop_vtxvtx_inters_t *vv_intersection;
        
        vv_intersection = arr_GetPunteroConstST(vv_intersections, i, csmsetop_vtxvtx_inters_t);
        i_vtxvtx_append_null_edges(vv_intersection, set_operation, set_of_null_edges_A, set_of_null_edges_B);
    }
}























