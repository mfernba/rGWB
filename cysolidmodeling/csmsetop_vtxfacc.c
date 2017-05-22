// Vertex - face classifier...

#include "csmsetop_vtxfacc.inl"

#include "csmsetop.tli"

#include "a_punter.h"
#include "cyassert.h"
#include "cypespy.h"

struct csmsetop_vtxfacc_inters_t
{
    struct csmvertex_t *vertex;
    struct csmface_t *face;
};

// ------------------------------------------------------------------------------------------

struct csmsetop_vtxfacc_inters_t *csmsetop_vtxfacc_create_inters(struct csmvertex_t *vertex, struct csmface_t *face)
{
    struct csmsetop_vtxfacc_inters_t *vf_inters;
    
    vf_inters = MALLOC(struct csmsetop_vtxfacc_inters_t);
    
    vf_inters->vertex = vertex;
    vf_inters->face = face;
    
    return vf_inters;
}

// ------------------------------------------------------------------------------------------

void csmsetop_vtxfacc_free_inters(struct csmsetop_vtxfacc_inters_t **vf_inters)
{
    assert_no_null(vf_inters);
    assert_no_null(*vf_inters);
    
    FREE_PP(vf_inters, struct csmsetop_vtxfacc_inters_t);
}

// ------------------------------------------------------------------------------------------

static void i_process_vf_inters(
                        const struct csmsetop_vtxfacc_inters_t *vf_inters,
                        enum csmsetop_operation_t set_operation,
                        ArrEstructura(csmedge_t) *set_of_null_edges)
{
    assert_no_null(vf_inters);
}

// ------------------------------------------------------------------------------------------

void csmsetop_vtxfacc_append_null_edges(
                        const ArrEstructura(csmsetop_vtxfacc_inters_t) *vf_intersections,
                        enum csmsetop_operation_t set_operation,
                        ArrEstructura(csmedge_t) *set_of_null_edges)
{
    unsigned long i, num_intersections;
    
    num_intersections = arr_NumElemsPunteroST(vf_intersections, csmsetop_vtxfacc_inters_t);
    
    for (i = 0; i < num_intersections; i++)
    {
        const struct csmsetop_vtxfacc_inters_t *vf_inters;
        
        vf_inters = arr_GetPunteroConstST(vf_intersections, i, csmsetop_vtxfacc_inters_t);
        i_process_vf_inters(vf_inters, set_operation, set_of_null_edges);
    }
}
