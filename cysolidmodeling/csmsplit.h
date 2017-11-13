// Split operation...

#include "csmfwddecl.hxx"

DLL_RGWB CSMBOOL csmsplit_does_plane_split_solid(
                        const struct csmsolid_t *solid,
                        double A, double B, double C, double D,
                        struct csmsolid_t **solid_above, struct csmsolid_t **solid_below);


