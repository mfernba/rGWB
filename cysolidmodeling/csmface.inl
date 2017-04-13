// Face...

#include "csmfwddecl.hxx"

CONSTRUCTOR(struct csmface_t *, csmface_crea, (struct csmsolid_t *solido, unsigned long *id_nuevo_elemento));

CONSTRUCTOR(struct csmface_t *, csmface_duplicate, (
                        struct csmface_t *face,
                        struct csmsolid_t *fsolid,
                        unsigned long *id_nuevo_elemento,
                        struct csmhashtb(csmvertex_t) *relation_svertexs_old_to_new,
                        struct csmhashtb(csmhedge_t) *relation_shedges_old_to_new));

void csmface_destruye(struct csmface_t **face);

unsigned long csmface_id(const struct csmface_t *face);

void csmface_reassign_id(struct csmface_t *face, unsigned long *id_nuevo_elemento, unsigned long *new_id_opc);


// Geometry...

void csmface_redo_geometric_generated_data(struct csmface_t *face);

CYBOOL csmface_contains_vertex(
                        const struct csmface_t *face,
                        const struct csmvertex_t *vertex,
                        enum csmmath_contaiment_point_loop_t *type_of_containment_opc,
                        struct csmvertex_t **hit_vertex_opc,
                        struct csmhedge_t **hit_hedge_opc);

CYBOOL csmface_contains_point(
                        const struct csmface_t *face,
                        double x, double y, double z,
                        enum csmmath_contaiment_point_loop_t *type_of_containment_opc,
                        struct csmvertex_t **hit_vertex_opc,
                        struct csmhedge_t **hit_hedge_opc);

CYBOOL csmface_is_loop_contained_in_face(struct csmface_t *face, struct csmloop_t *loop);


// Topology...

struct csmsolid_t *csmface_fsolid(struct csmface_t *face);
void csmface_set_fsolid(struct csmface_t *face, struct csmsolid_t *solid);

struct csmloop_t *csmface_flout(struct csmface_t *face);
void csmface_set_flout(struct csmface_t *face, struct csmloop_t *loop);

struct csmloop_t *csmface_floops(struct csmface_t *face);
void csmface_set_floops(struct csmface_t *face, struct csmloop_t *loop);

void csmface_add_loop_while_removing_from_old(struct csmface_t *face, struct csmloop_t *loop);

void csmface_remove_loop(struct csmface_t *face, struct csmloop_t **loop);

CYBOOL csmface_has_holes(const struct csmface_t *face);
