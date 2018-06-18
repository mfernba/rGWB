// Writeable solid...

#include "csmfwddecl.hxx"

CONSTRUCTOR(struct csmwriteablesolid_t *, csmwriteablesolid_new, (void));

void csmwriteablesolid_free(struct csmwriteablesolid_t **writeable_solid);

CONSTRUCTOR(struct csmwriteablesolid_t *, csmwriteablesolid_read, (struct csmsave_t *csmsave));

void csmwriteablesolid_write(const struct csmwriteablesolid_t *writeable_solid, struct csmsave_t *csmsave);


// Writing...

void csmwriteablesolid_append_vertex(
                        struct csmwriteablesolid_t *writeable_solid, 
                        unsigned long vertex_id,
                        double x, double y, double z,
                        unsigned long hedge_id);

void csmwriteablesolid_append_hedge(
                        struct csmwriteablesolid_t *writeable_solid,
                        unsigned long hedge_id,
                        unsigned long loop_id, unsigned long vertex_id);

void csmwriteablesolid_append_loop(
                        struct csmwriteablesolid_t *writeable_solid,
                        unsigned long loop_id,
                        unsigned long ledge_id, unsigned long face_id);

void csmwriteablesolid_append_face(
                        struct csmwriteablesolid_t *writeable_solid,
                        unsigned long face_id,
                        unsigned long outer_loop_id, csmArrULong **loops_ids);

void csmwriteablesolid_append_edge(
                        struct csmwriteablesolid_t *writeable_solid,
                        unsigned long edge_id,
                        unsigned long hedge_pos_id, unsigned long hedge_neg_id);
