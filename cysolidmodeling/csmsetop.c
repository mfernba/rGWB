// Boolean operations on solids...

#include "csmsetop.h"
#include "csmsetop.tli"

#include "csmsetop_procedges.inl"
#include "csmsetop_vtxfacc.inl"
#include "csmsetop_vtxvtx.inl"

#include "csmsolid.h"
#include "csmsolid.inl"

#include "a_punter.h"
#include "cyassert.h"
#include "cypespy.h"

ArrEstructura(csmsetop_vtxvtx_inters_t);
ArrEstructura(csmsetop_vtxfacc_inters_t);

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmsolid_t *, i_set_operation_modifying_solids, (
                        enum csmsetop_operation_t set_operation,
                        struct csmsolid_t *solid_A, struct csmsolid_t *solid_B))
{
    struct csmsolid_t *result;
    ArrEstructura(csmsetop_vtxvtx_inters_t) *vv_intersections;
    ArrEstructura(csmsetop_vtxfacc_inters_t) *vf_intersections_A, *vf_intersections_B;

    csmsolid_redo_geometric_generated_data(solid_A);
    csmsolid_clear_algorithm_vertex_mask(solid_A);
    
    csmsolid_redo_geometric_generated_data(solid_B);
    csmsolid_clear_algorithm_vertex_mask(solid_B);

    csmsetop_procedges_generate_intersections_on_both_solids(
                        solid_A, solid_B,
                        &vv_intersections,
                        &vf_intersections_A, &vf_intersections_B);
    
    result = NULL;
    
    
    
    arr_DestruyeEstructurasST(&vv_intersections, csmsetop_vtxvtx_free_inters, csmsetop_vtxvtx_inters_t);
    arr_DestruyeEstructurasST(&vf_intersections_A, csmsetop_vtxfacc_free_inters, csmsetop_vtxfacc_inters_t);
    arr_DestruyeEstructurasST(&vf_intersections_B, csmsetop_vtxfacc_free_inters, csmsetop_vtxfacc_inters_t);
    
    return result;
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmsolid_t *, i_set_operation, (
                        enum csmsetop_operation_t set_operation,
                        const struct csmsolid_t *solid_A, const struct csmsolid_t *solid_B))
{
    struct csmsolid_t *result;
    struct csmsolid_t *solid_A_copy, *solid_B_copy;
    
    solid_A_copy = csmsolid_duplicate(solid_A);
    solid_B_copy = csmsolid_duplicate(solid_B);
    
    result = i_set_operation_modifying_solids(set_operation, solid_A_copy, solid_B_copy);
    
    csmsolid_free(&solid_A_copy);
    csmsolid_free(&solid_B_copy);
    
    return result;
}

// ------------------------------------------------------------------------------------------

struct csmsolid_t *csmsetop_difference_A_minus_B(const struct csmsolid_t *solid_A, const struct csmsolid_t *solid_B)
{
    enum csmsetop_operation_t set_operation;
    
    set_operation = CSMSETOP_OPERATION_DIFFERENCE;
    return i_set_operation(set_operation, solid_A, solid_B);
}

// ------------------------------------------------------------------------------------------

struct csmsolid_t *csmsetop_union_A_and_B(const struct csmsolid_t *solid_A, const struct csmsolid_t *solid_B)
{
    enum csmsetop_operation_t set_operation;
    
    set_operation = CSMSETOP_OPERATION_UNION;
    return i_set_operation(set_operation, solid_A, solid_B);
}

// ------------------------------------------------------------------------------------------

struct csmsolid_t *csmsetop_intersection_A_and_B(const struct csmsolid_t *solid_A, const struct csmsolid_t *solid_B)
{
    enum csmsetop_operation_t set_operation;
    
    set_operation = CSMSETOP_OPERATION_INTERSECTION;
    return i_set_operation(set_operation, solid_A, solid_B);
}
