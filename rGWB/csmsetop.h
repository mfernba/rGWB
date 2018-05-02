// Boolean operations on solids...

#include "csmfwddecl.hxx"
#include "csmsetop.hxx"

#ifdef __cplusplus
extern "C" {
#endif

DLL_RGWB enum csmsetop_opresult_t csmsetop_difference_A_minus_B(const struct csmsolid_t *solid_A, const struct csmsolid_t *solid_B, struct csmsolid_t **solid_res);

DLL_RGWB enum csmsetop_opresult_t csmsetop_union_A_and_B(const struct csmsolid_t *solid_A, const struct csmsolid_t *solid_B, struct csmsolid_t **solid_res);

DLL_RGWB enum csmsetop_opresult_t csmsetop_intersection_A_and_B(const struct csmsolid_t *solid_A, const struct csmsolid_t *solid_B, struct csmsolid_t **solid_res);

#ifdef __cplusplus
}
#endif
