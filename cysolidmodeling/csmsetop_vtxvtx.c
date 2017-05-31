// Boolean operations on solids: Vertex - Vertex classifier...

#include "csmsetop_vtxvtx.inl"

#include "csmmath.inl"
#include "csmhedge.inl"
#include "csmface.inl"
#include "csmopbas.inl"
#include "csmsetop.tli"
#include "csmtolerance.inl"
#include "csmvertex.inl"

#include "a_punter.h"
#include "cyassert.h"
#include "cypespy.h"

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
                        double Ux12, double Uy12, double Uz12))
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
        struct csmhedge_t *he_iterator_prv, *he_iterator_nxt;
        double Ux1, Uy1, Uz1, Ux2, Uy2, Uz2;
        double Ux12, Uy12, Uz12;
        CYBOOL is_null_vector, is_oriented_in_direction;
        struct i_neighborhood_t *neigborhood;
        
        assert(num_iters < 10000);
        num_iters++;
        
        assert(csmhedge_vertex(he_iterator) == vertex);
        
        he_iterator_face = csmopbas_face_from_hedge(he_iterator);
        
        he_iterator_prv = csmhedge_prev(he_iterator);
        i_vector_to_he(he_iterator_prv, vertex, &Ux1, &Uy1, &Uz1);
        
        he_iterator_nxt = csmhedge_next(he_iterator);
        i_vector_to_he(he_iterator_nxt, vertex, &Ux2, &Uy2, &Uz2);

        csmmath_cross_product3D(Ux1, Uy1, Uz1, Ux2, Uy2, Uz2, &Ux12, &Uy12, &Uz12);
        
        is_null_vector = csmmath_is_null_vector(Ux12, Uy12, Uz12, null_vector_tolerance);
        is_oriented_in_direction = csmface_is_oriented_in_direction(he_iterator_face, Ux12, Uy12, Uz12);
        
        neigborhood = i_create_neighborhood(he_iterator, Ux1, Uy1, Uz1, Ux2, Uy2, Uz2, Ux12, Uy12, Uz12);
        arr_AppendPunteroST(neighborhoods, neigborhood, i_neighborhood_t);
        
        if (is_null_vector == CIERTO || is_oriented_in_direction == CIERTO)
        {
            double Ux_bisec, Uy_bisec, Uz_bisec;
            double Ux12_bisec, Uy12_bisec, Uz12_bisec;
            struct i_neighborhood_t *neigborhood_bisec;
            
            if (is_null_vector == CIERTO)
            {
                double A, B, C, D;
                
                csmface_face_equation(he_iterator_face, &A, &B, &C, &D);
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
            neigborhood_bisec = i_create_neighborhood(he_iterator, Ux_bisec, Uy_bisec, Uz_bisec, Ux2, Uy2, Uz2, Ux12, Uy12, Uz12);
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
        }
    }
    
    
    *neighborhood_A = neighborhood_A_loc;
    *neighborhood_B = neighborhood_B_loc;
    *neighborhood_intersections = neighborhood_intersections_loc;
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























