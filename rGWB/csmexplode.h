// Explode a solid into several shells (if they exist...)

#include "csmfwddecl.hxx"

#ifdef __cplusplus
extern "C" {
#endif

DLL_RGWB CONSTRUCTOR(csmArrayStruct(csmsolid_t) *, csmexplode_explode_shells, (const struct csmsolid_t *solid));

#ifdef __cplusplus
}
#endif
