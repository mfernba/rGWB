// Loop...

#include "csmfwddecl.hxx"
#include "csmvertex.hxx"

CONSTRUCTOR(struct csmloop_t *, csmloop_crea, (struct csmface_t *face, unsigned long *id_nuevo_elemento));

CONSTRUCTOR(struct csmloop_t *, csmloop_duplicate, (
                        const struct csmloop_t *loop,
                        struct csmface_t *lface,
                        unsigned long *id_nuevo_elemento,
                        struct csmhashtb(csmvertex_t) *relation_svertexs_old_to_new,
                        struct csmhashtb(csmhedge_t) *relation_shedges_old_to_new));


unsigned long csmloop_id(const struct csmloop_t *loop);

void csmloop_reassign_id(struct csmloop_t *loop, unsigned long *id_new_element, unsigned long *new_id_opt);


// Geometry...

void csmloop_face_equation(
                        const struct csmloop_t *loop,
                        double *A, double *B, double *C, double *D);

void csmloop_update_bounding_box(const struct csmloop_t *loop, struct csmbbox_t *bbox);

double csmloop_max_distance_to_plane(
                        const struct csmloop_t *loop,
                        double A, double B, double C, double D);

CYBOOL csmloop_is_point_inside_loop(
                        const struct csmloop_t *loop, CYBOOL is_outer_loop,
                        double x, double y, double z, enum csmmath_dropped_coord_t dropped_coord,
                        double tolerance,
                        enum csmmath_contaiment_point_loop_t *type_of_containment_opc,
                        struct csmvertex_t **hit_vertex_opc,
                        struct csmhedge_t **hit_hedge_opc);

CYBOOL csmloop_is_bounded_by_vertex_with_mask_attrib(const struct csmloop_t *loop, csmvertex_mask_t mask_attrib);

// Topology...

struct csmhedge_t *csmloop_ledge(struct csmloop_t *loop);
void csmloop_set_ledge(struct csmloop_t *loop, struct csmhedge_t *ledge);

struct csmface_t *csmloop_lface(struct csmloop_t *loop);
void csmloop_set_lface(struct csmloop_t *loop, struct csmface_t *face);

struct csmloop_t *csmloop_next(struct csmloop_t *loop);
struct csmloop_t *csmloop_prev(struct csmloop_t *loop);

void csmloop_revert_loop_orientation(struct csmloop_t *loop);
