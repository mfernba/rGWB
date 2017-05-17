// Set operations...

#include "csmfwddecl.hxx"

DLL_CYSOLIDMODELING CONSTRUCTOR(struct csmsolid_t *, csmsetop_difference_A_minus_B, (const struct csmsolid_t *solid_A, const struct csmsolid_t *solid_B));

DLL_CYSOLIDMODELING CONSTRUCTOR(struct csmsolid_t *, csmsetop_union_A_and_B, (const struct csmsolid_t *solid_A, const struct csmsolid_t *solid_B));

DLL_CYSOLIDMODELING CONSTRUCTOR(struct csmsolid_t *, csmsetop_intersection_A_and_B, (const struct csmsolid_t *solid_A, const struct csmsolid_t *solid_B));

