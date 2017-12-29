// Split operation...

#include "csmfwddecl.hxx"
#include "csmsplit.hxx"

DLL_RGWB enum csmsplit_opresult_t csmsplit_split_solid(
                        const struct csmsolid_t *solid,
                        double A, double B, double C, double D,
                        struct csmsolid_t **solid_above, struct csmsolid_t **solid_below);



