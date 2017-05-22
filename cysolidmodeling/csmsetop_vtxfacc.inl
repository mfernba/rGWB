// Vertex - face classifier...

#include "csmfwddecl.hxx"
#include "csmsetop.tli"
#include "a_punter.h"
ArrEstructura(csmsetop_vtxfacc_inters_t);
ArrEstructura(csmedge_t);


// Intersections...

CONSTRUCTOR(struct csmsetop_vtxfacc_inters_t *, csmsetop_vtxfacc_create_inters, (struct csmvertex_t *vertex, struct csmface_t *face));

void csmsetop_vtxfacc_free_inters(struct csmsetop_vtxfacc_inters_t **vf_inters);


// Classifier...

void csmsetop_vtxfacc_append_null_edges(
                        const ArrEstructura(csmsetop_vtxfacc_inters_t) *vf_intersections,
                        enum csmsetop_operation_t set_operation,
                        ArrEstructura(csmedge_t) *set_of_null_edges);
