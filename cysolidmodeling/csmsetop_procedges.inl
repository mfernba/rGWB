// Boolean Operations on solids: Edge processing...

#include "csmfwddecl.hxx"

void csmsetop_procedges_generate_intersections_on_both_solids(
                        struct csmsolid_t *solid_A, struct csmsolid_t *solid_B,
                        csmArrayStruct(csmsetop_vtxvtx_inters_t) **vv_intersections,
                        csmArrayStruct(csmsetop_vtxfacc_inters_t) **vf_intersections_A,
                        csmArrayStruct(csmsetop_vtxfacc_inters_t) **vf_intersections_B);
