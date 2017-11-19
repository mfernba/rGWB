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

double csmloop_compute_area(
                        const struct csmloop_t *loop,
                        double Xo, double Yo, double Zo,
                        double Ux, double Uy, double Uz, double Vx, double Vy, double Vz);

CSMBOOL csmloop_is_point_inside_loop(
                        const struct csmloop_t *loop,
                        double x, double y, double z, enum csmmath_dropped_coord_t dropped_coord,
                        const struct csmtolerance_t *tolerances,
                        enum csmmath_contaiment_point_loop_t *type_of_containment_opc,
                        struct csmvertex_t **hit_vertex_opc,
                        struct csmhedge_t **hit_hedge_opc, double *t_relative_to_hit_hedge_opc);

CSMBOOL csmloop_is_bounded_by_vertex_with_mask_attrib(const struct csmloop_t *loop, csmvertex_mask_t mask_attrib);

CSMBOOL csmloop_has_only_a_null_edge(const struct csmloop_t *loop);

void csmloop_geometric_center_3d(struct csmloop_t *loop, double *x, double *y, double *z);


// Topology...

struct csmhedge_t *csmloop_ledge(struct csmloop_t *loop);
void csmloop_set_ledge(struct csmloop_t *loop, struct csmhedge_t *ledge);

struct csmface_t *csmloop_lface(struct csmloop_t *loop);
void csmloop_set_lface(struct csmloop_t *loop, struct csmface_t *face);

struct csmloop_t *csmloop_next(struct csmloop_t *loop);
struct csmloop_t *csmloop_prev(struct csmloop_t *loop);

void csmloop_revert_loop_orientation(struct csmloop_t *loop);

CSMBOOL csmloop_setop_convert_loop_in_face(const struct csmloop_t *loop);
void csmloop_set_setop_convert_loop_in_face(struct csmloop_t *loop, CSMBOOL setop_convert_loop_in_face);

CSMBOOL csmloop_setop_loop_was_a_hole(const struct csmloop_t *loop);
void csmloop_set_setop_loop_was_a_hole(struct csmloop_t *loop, CSMBOOL setop_loop_was_a_hole);

void csmloop_clear_algorithm_mask(struct csmloop_t *loop);



