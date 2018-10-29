// Operaciones básicas sobre elementos con dependencias entre sí...

#include "csmopbas.inl"

#include "csmedge.inl"
#include "csmedge.tli"
#include "csmface.inl"
#include "csmhedge.inl"
#include "csmloop.inl"
#include "csmmath.inl"
#include "csmnode.inl"
#include "csmtolerance.inl"
#include "csmvertex.inl"

#ifdef __STANDALONE_DISTRIBUTABLE
#include "csmassert.inl"
#include "csmmem.inl"
#else
#include "cyassert.h"
#include "cypespy.h"
#endif

// ------------------------------------------------------------------------------------------

struct csmhedge_t *csmopbas_mate(struct csmhedge_t *hedge)
{
    struct csmedge_t *edge;
    
    edge = csmhedge_edge(hedge);
    return csmedge_mate(edge, hedge);
}

// ------------------------------------------------------------------------------------------

struct csmface_t *csmopbas_face_from_hedge(struct csmhedge_t *hedge)
{
    struct csmloop_t *loop;
    
    loop = csmhedge_loop(hedge);
    return csmloop_lface(loop);
}

// ------------------------------------------------------------------------------------------

struct csmsolid_t *csmopbas_solid_from_hedge(struct csmhedge_t *hedge)
{
    struct csmface_t *lface;
    
    lface = csmopbas_face_from_hedge(hedge);
    return csmface_fsolid(lface);
}

// ------------------------------------------------------------------------------------------

struct csmsolid_t *csmopbas_solid_from_hedges(struct csmhedge_t *he1, struct csmhedge_t *he2)
{
    struct csmsolid_t *edge_solid_he1, *edge_solid_he2;
    
    edge_solid_he1 = csmopbas_solid_from_hedge(he1);
    edge_solid_he2 = csmopbas_solid_from_hedge(he2);
    assert(edge_solid_he1 == edge_solid_he2);
    
    return edge_solid_he1;
}

// ------------------------------------------------------------------------------------------

struct csmsolid_t *csmopbas_solid_aux_from_hedge(struct csmhedge_t *hedge)
{
    struct csmface_t *lface;
    
    lface = csmopbas_face_from_hedge(hedge);
    return csmface_fsolid_aux(lface);
}

// ------------------------------------------------------------------------------------------

void csmopbas_addhe(
                struct csmedge_t *edge,
                struct csmvertex_t *vertex,
                struct csmhedge_t *hedge,
                enum csmedge_hedge_side_t lado,
                unsigned long *id_new_element,
                struct csmhedge_t **new_hedge_opc)
{
    struct csmhedge_t *new_hedge_loc;
    struct csmloop_t *loop;
    
    assert_no_null(edge);
    assert_no_null(vertex);
    
    if (csmhedge_edge(hedge) == NULL)
    {
        new_hedge_loc = hedge;
    }
    else
    {
        new_hedge_loc = csmhedge_new(id_new_element);
        csmnode_insert_node2_before_node1(hedge, new_hedge_loc, csmhedge_t);
    }
    
    csmedge_set_edge_lado(edge, lado, new_hedge_loc);
    csmhedge_set_edge(new_hedge_loc, edge);
    
    loop = csmhedge_loop(hedge);
    csmhedge_set_loop(new_hedge_loc, loop);
    
    csmhedge_set_vertex(new_hedge_loc, vertex);
    
    ASSIGN_OPTIONAL_VALUE(new_hedge_opc, new_hedge_loc);
}

// ------------------------------------------------------------------------------------------

void csmopbas_delhe(struct csmhedge_t **hedge, struct csmhedge_t **hedge_prev_opc, struct csmhedge_t **hedge_next_opc)
{
    struct csmhedge_t *hedge_prev, *hedge_next;
    struct csmhedge_t *hedge_loc;
    struct csmedge_t *edge;
    struct csmloop_t *loop;
    struct csmvertex_t *vertex;
    CSMBOOL destroy_hedge;
    
    assert_no_null(hedge);
    assert_no_null(*hedge);
    
    hedge_loc = ASSIGN_POINTER_PP_NOT_NULL(hedge, struct csmhedge_t);
    edge = csmhedge_edge(hedge_loc);
    loop = csmhedge_loop(hedge_loc);
    vertex = csmhedge_vertex(hedge_loc);
    
    if (edge == NULL)
    {
        destroy_hedge = CSMTRUE;
        
        hedge_prev = NULL;
        hedge_next = NULL;
    }
    else
    {
        if (csmhedge_next(hedge_loc) == hedge_loc)
        {
            assert(csmhedge_prev(hedge_loc) == hedge_loc);
            
            destroy_hedge = CSMFALSE;
            
            hedge_prev = hedge_loc;
            hedge_next = hedge_loc;
        }
        else
        {
            destroy_hedge = CSMTRUE;
            
            hedge_prev = csmhedge_prev(hedge_loc);
            assert(hedge_prev != hedge_loc);
            
            hedge_next = csmhedge_next(hedge_loc);
            assert(hedge_next != hedge_loc);
        }
        
        csmhedge_set_edge(hedge_loc, NULL);
        csmedge_remove_hedge(edge, hedge_loc);
    }
    
    if (destroy_hedge == CSMTRUE)
    {
        if (csmvertex_hedge(vertex) == hedge_loc)
            csmvertex_set_hedge(vertex, NULL);
        
        if (loop != NULL)
            csmloop_set_ledge(loop, hedge_prev);
        
        csmnode_free_node_in_list(&hedge_loc, csmhedge_t);
    }
    
    ASSIGN_OPTIONAL_VALUE(hedge_prev_opc, hedge_prev);
    ASSIGN_OPTIONAL_VALUE(hedge_next_opc, hedge_next);
}

// ------------------------------------------------------------------------------------------

static CSMBOOL i_is_null_edge(struct csmhedge_t *he, const struct csmtolerance_t *tolerances)
{
    struct csmvertex_t *vertex, *vertex_nxt;
    
    vertex = csmhedge_vertex(he);
    vertex_nxt = csmhedge_vertex(csmhedge_next(he));
    
    return csmvertex_equal_coords(vertex, vertex_nxt, csmtolerance_equal_coords(tolerances));
}

// ------------------------------------------------------------------------------------------

struct csmhedge_t *csmopbas_get_prev_no_null_hedge(struct csmhedge_t *hedge, const struct csmtolerance_t *tolerances)
{
    struct csmhedge_t *hedge_prev;

    hedge_prev = csmhedge_prev(hedge);

    while (i_is_null_edge(hedge_prev, tolerances) == CSMTRUE)
        hedge_prev = csmhedge_prev(hedge_prev);

    return hedge_prev;
}

// ------------------------------------------------------------------------------------------

static CSMBOOL i_hedges_equal_coords(struct csmhedge_t *he1, struct csmhedge_t *he2, const struct csmtolerance_t *tolerances)
{
    struct csmvertex_t *vertex, *vertex_nxt;
    
    vertex = csmhedge_vertex(he1);
    vertex_nxt = csmhedge_vertex(he2);
    
    return csmvertex_equal_coords(vertex, vertex_nxt, csmtolerance_equal_coords(tolerances));
}

// ------------------------------------------------------------------------------------------

struct csmhedge_t *csmopbas_get_next_no_null_hedge(struct csmhedge_t *hedge, const struct csmtolerance_t *tolerances)
{
    struct csmhedge_t *hedge_next;

    hedge_next = csmhedge_next(hedge);

    while (i_hedges_equal_coords(hedge, hedge_next, tolerances) == CSMTRUE)
        hedge_next = csmhedge_next(hedge_next);

    return hedge_next;
}

// ------------------------------------------------------------------------------------------

CSMBOOL csmopbas_is_wide_hedge(
                        struct csmhedge_t *hedge,
                        const struct csmtolerance_t *tolerances,
                        double *Ux_bisec_opt, double *Uy_bisec_opt, double *Uz_bisec_opt)
{
    CSMBOOL is_wide;
    double Ux_bisec_loc, Uy_bisec_loc, Uz_bisec_loc;
    struct csmhedge_t *hedge_prev, *hedge_next;
    const struct csmvertex_t *vertex_prev, *vertex, *vertex_next;
    double Ux_to_prev, Uy_to_prev, Uz_to_prev;
    double Ux_to_next, Uy_to_next, Uz_to_next;
    struct csmface_t *face;
    double A, B, C, D;

    vertex = csmhedge_vertex(hedge);
    
    hedge_prev = csmopbas_get_prev_no_null_hedge(hedge, tolerances);
    vertex_prev = csmhedge_vertex(hedge_prev);
    csmvertex_vector_from_vertex1_to_vertex2(vertex, vertex_prev, &Ux_to_prev, &Uy_to_prev, &Uz_to_prev);
    
    hedge_next = csmopbas_get_next_no_null_hedge(hedge, tolerances);
    vertex_next = csmhedge_vertex(hedge_next);
    csmvertex_vector_from_vertex1_to_vertex2(vertex, vertex_next, &Ux_to_next, &Uy_to_next, &Uz_to_next);
    
    csmmath_make_unit_vector3D(&Ux_to_prev, &Uy_to_prev, &Uz_to_prev);
    csmmath_make_unit_vector3D(&Ux_to_next, &Uy_to_next, &Uz_to_next);

    face = csmopbas_face_from_hedge(hedge);
    csmface_face_equation(face, &A, &B, &C, &D);
    
    if (csmmath_vectors_are_parallel(Ux_to_prev, Uy_to_prev, Uz_to_prev, Ux_to_next, Uy_to_next, Uz_to_next, tolerances) == CSMTRUE)
    {
        is_wide = CSMTRUE;
        csmmath_cross_product3D(A, B, C, Ux_to_next, Uy_to_next, Uz_to_next, &Ux_bisec_loc, &Uy_bisec_loc, &Uz_bisec_loc);
    }
    else
    {
        double Ux_cross_prev_next, Uy_cross_prev_next, Uz_cross_prev_next;
        double dot_product;

        csmmath_cross_product3D(
                        Ux_to_prev, Uy_to_prev, Uz_to_prev, Ux_to_next, Uy_to_next, Uz_to_next,
                        &Ux_cross_prev_next, &Uy_cross_prev_next, &Uz_cross_prev_next);
        
        dot_product = csmmath_dot_product3D(Ux_cross_prev_next, Uy_cross_prev_next, Uz_cross_prev_next, A, B, C);
        
        if (dot_product > 0.)
        {
            is_wide = CSMTRUE;
            
            Ux_bisec_loc = -(Ux_to_prev + Ux_to_next);
            Uy_bisec_loc = -(Uy_to_prev + Uy_to_next);
            Uz_bisec_loc = -(Uz_to_prev + Uz_to_next);
        }
        else
        {
            is_wide = CSMFALSE;
            
            Ux_bisec_loc = 0.;
            Uy_bisec_loc = 0.;
            Uz_bisec_loc = 0.;
        }
    }
    
    ASSIGN_OPTIONAL_VALUE(Ux_bisec_opt, Ux_bisec_loc);
    ASSIGN_OPTIONAL_VALUE(Uy_bisec_opt, Uy_bisec_loc);
    ASSIGN_OPTIONAL_VALUE(Uz_bisec_opt, Uz_bisec_loc);
    
    return is_wide;
}





