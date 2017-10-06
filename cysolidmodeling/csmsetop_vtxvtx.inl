// Boolean operations on solids: Vertex - Vertex classifier...
#include "csmfwddecl.hxx"

// Intersections...

CONSTRUCTOR(struct csmsetop_vtxvtx_inters_t *, csmsetop_vtxvtx_create_inters, (struct csmvertex_t *vertex_a, struct csmvertex_t *vertex_b));

void csmsetop_vtxvtx_free_inters(struct csmsetop_vtxvtx_inters_t **vv_inters);


// Classifier...

void csmsetop_vtxvtx_append_null_edges(
                        const csmArrayStruct(csmsetop_vtxvtx_inters_t) *vv_intersections,
                        enum csmsetop_operation_t set_operation,
                        csmArrayStruct(csmedge_t) *set_of_null_edges_A,
                        csmArrayStruct(csmedge_t) *set_of_null_edges_B);
