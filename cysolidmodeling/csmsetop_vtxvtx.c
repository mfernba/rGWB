// Boolean operations on solids: Vertex - Vertex classifier...

#include "csmsetop_vtxvtx.inl"

#include "csmsetop.tli"

#include "a_punter.h"
#include "cyassert.h"
#include "cypespy.h"

struct csmsetop_vtxvtx_inters_t
{
    struct csmvertex_t *vertex_a, *vertex_b;
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

void csmsetop_vtxvtx_append_null_edges(
                        const ArrEstructura(csmsetop_vtxvtx_inters_t) *vv_intersections,
                        enum csmsetop_operation_t set_operation,
                        ArrEstructura(csmedge_t) *set_of_null_edges_A,
                        ArrEstructura(csmedge_t) *set_of_null_edges_B)
{
}
