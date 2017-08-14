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

enum csmmath_double_relation_t csmface_classify_vertex_relative_to_face(const struct csmface_t *face, const struct csmvertex_t *vertex);

CYBOOL csmface_exists_intersection_between_line_and_face_plane(
                        const struct csmface_t *face,
                        double x1, double y1, double z1, double x2, double y2, double z2,
                        double *x_inters_opc, double *y_inters_opc, double *z_inters_opc, double *t_inters_opc);

CYBOOL csmface_is_loop_contained_in_face(struct csmface_t *face, struct csmloop_t *loop);

CYBOOL csmface_is_convex_hedge(struct csmface_t *face, struct csmhedge_t *hedge);

double csmface_tolerace(const struct csmface_t *face);

CYBOOL csmface_is_coplanar_to_plane(
                        const struct csmface_t *face,
                        double A, double B, double C, double D,
                        double tolerance,
                        CYBOOL *same_orientation);

CYBOOL csmface_is_oriented_in_direction(const struct csmface_t *face, double Wx, double Wy, double Wz);

void csmface_face_equation(
                        const struct csmface_t *face,
                        double *A, double *B, double *C, double *D);


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

void csmface_revert(struct csmface_t *face);


// Debug...

void csmface_print_info_debug(struct csmface_t *face, CYBOOL assert_si_no_es_integro, unsigned long *num_holes_opc);

