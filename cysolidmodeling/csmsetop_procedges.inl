// Boolean Operations on solids: Edge processing...

#include "csmfwddecl.hxx"
#include "a_punter.h"
ArrEstructura(csmsetop_vtxvtx_inters_t);
ArrEstructura(csmsetop_vtxfacc_inters_t);

void csmsetop_procedges_generate_intersections_on_both_solids(
                        struct csmsolid_t *solid_A, struct csmsolid_t *solid_B,
                        ArrEstructura(csmsetop_vtxvtx_inters_t) **vv_intersections,
                        ArrEstructura(csmsetop_vtxfacc_inters_t) **vf_intersections_A,
                        ArrEstructura(csmsetop_vtxfacc_inters_t) **vf_intersections_B);
