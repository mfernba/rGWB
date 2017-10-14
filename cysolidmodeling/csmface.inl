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

void csmface_assign_visualization_material(struct csmface_t *face, const struct csmmaterial_t *visz_material_opt);

void csmface_copy_visualization_material_from_face1(const struct csmface_t *face1, struct csmface_t *face2);


// Geometry...

void csmface_redo_geometric_generated_data(struct csmface_t *face);

CSMBOOL csmface_should_analyze_intersections_between_faces(const struct csmface_t *face1, const struct csmface_t *face2);

CSMBOOL csmface_should_analyze_intersections_with_segment(
	                    const struct csmface_t *face,
                        double x1, double y1, double z1, double x2, double y2, double z2);

CSMBOOL csmface_contains_vertex(
                        const struct csmface_t *face,
                        const struct csmvertex_t *vertex,
                        enum csmmath_contaiment_point_loop_t *type_of_containment_opc,
                        struct csmvertex_t **hit_vertex_opc,
                        struct csmhedge_t **hit_hedge_opc, double *t_relative_to_hit_hedge_opc);

CSMBOOL csmface_contains_point(
                        const struct csmface_t *face,
                        double x, double y, double z,
                        enum csmmath_contaiment_point_loop_t *type_of_containment_opc,
                        struct csmvertex_t **hit_vertex_opc,
                        struct csmhedge_t **hit_hedge_opc, double *t_relative_to_hit_hedge_opc);

CSMBOOL csmface_is_point_interior_to_face(const struct csmface_t *face, double x, double y, double z);

enum csmcompare_t csmface_classify_vertex_relative_to_face(const struct csmface_t *face, const struct csmvertex_t *vertex);

CSMBOOL csmface_exists_intersection_between_line_and_face_plane(
                        const struct csmface_t *face,
                        double x1, double y1, double z1, double x2, double y2, double z2,
                        double *x_inters_opc, double *y_inters_opc, double *z_inters_opc, double *t_inters_opc);

CSMBOOL csmface_is_loop_contained_in_face(struct csmface_t *face, struct csmloop_t *loop);

double csmface_tolerace(const struct csmface_t *face);

CSMBOOL csmface_is_coplanar_to_plane(
                        const struct csmface_t *face,
                        double A, double B, double C, double D,
                        double tolerance,
                        CSMBOOL *same_orientation);

CSMBOOL csmface_are_coplanar_faces(struct csmface_t *face1, const struct csmface_t *face2);

CSMBOOL csmface_faces_define_border_edge(struct csmface_t *face1, const struct csmface_t *face2);

CSMBOOL csmface_is_oriented_in_direction(const struct csmface_t *face, double Wx, double Wy, double Wz);

void csmface_face_equation(
                        const struct csmface_t *face,
                        double *A, double *B, double *C, double *D);

void csmface_face_equation_info(
                        const struct csmface_t *face,
                        double *A, double *B, double *C, double *D);

// Topology...

struct csmsolid_t *csmface_fsolid(struct csmface_t *face);
void csmface_set_fsolid(struct csmface_t *face, struct csmsolid_t *solid);

struct csmsolid_t *csmface_fsolid_aux(struct csmface_t *face);
void csmface_set_fsolid_aux(struct csmface_t *face, struct csmsolid_t *solid);

struct csmloop_t *csmface_flout(struct csmface_t *face);
void csmface_set_flout(struct csmface_t *face, struct csmloop_t *loop);

struct csmloop_t *csmface_floops(struct csmface_t *face);
void csmface_set_floops(struct csmface_t *face, struct csmloop_t *loop);

void csmface_add_loop_while_removing_from_old(struct csmface_t *face, struct csmloop_t *loop);

void csmface_remove_loop(struct csmface_t *face, struct csmloop_t **loop);

CSMBOOL csmface_has_holes(const struct csmface_t *face);

void csmface_revert(struct csmface_t *face);


// Algorithm marks...

void csmface_clear_algorithm_mask(struct csmface_t *face);

CSMBOOL csmface_is_setop_null_face(struct csmface_t *face);
void csmface_mark_setop_null_face(struct csmface_t *face);
