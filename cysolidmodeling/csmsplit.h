// Split operation...

#include "csmfwddecl.hxx"

DLL_CYSOLIDMODELING void csmsplit_split_solid(
                        const struct csmsolid_t *solid,
                        double A, double B, double C, double D,
                        CYBOOL *there_is_solid_above, struct csmsolid_t **solid_above,
                        CYBOOL *there_is_solid_below, struct csmsolid_t **solid_below);

