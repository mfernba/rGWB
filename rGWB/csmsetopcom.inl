// Commons operations for Set Operators...

#include "csmfwddecl.hxx"

// Vertexs...

void csmsetopcom_append_vertex_if_not_exists(struct csmvertex_t *vertex, csmArrayStruct(csmvertex_t) *set_of_on_vertices);


// Hedges...

CSMBOOL csmsetopcom_is_edge_on_cycle_of_edges_with_a_side_on_a_hole(struct csmedge_t *edge);

CSMBOOL csmsetopcom_hedges_are_neighbors(struct csmhedge_t *he1, struct csmhedge_t *he2);


// Edges...

void csmsetopcom_sort_edges_lexicographically_by_xyz(csmArrayStruct(csmedge_t) *set_of_null_edges, const struct csmtolerance_t *tolerances);

void csmsetopcom_sort_pair_edges_lexicographically_by_xyz(
                        csmArrayStruct(csmedge_t) *set_of_null_edges_A, csmArrayStruct(csmedge_t) *set_of_null_edges_B,
                        const struct csmtolerance_t *tolerances);

void csmsetopcom_print_set_of_null_edges(const csmArrayStruct(csmedge_t) *set_of_null_edges, csmArrayStruct(csmhedge_t) *loose_ends);

CSMBOOL csmsetopcom_is_loose_end(struct csmhedge_t *hedge, csmArrayStruct(csmhedge_t) *loose_ends);

void csmsetopcom_print_debug_info_loose_ends(const csmArrayStruct(csmhedge_t) *loose_ends);

void csmsetopcom_print_debug_info_faces_null_edges(const char *solid_reference, const csmArrayStruct(csmedge_t) *null_edges);

void csmsetopcom_join_hedges(
                        struct csmhedge_t *he1, struct csmhedge_t *he2,
                        const struct csmtolerance_t *tolerances);

void csmsetopcom_cut_he(
                    struct csmhedge_t *hedge,
                    csmArrayStruct(csmedge_t) *set_of_null_edges,
                    csmArrayStruct(csmface_t) *set_of_null_faces,
                    unsigned long *no_null_edges_deleted,
                    CSMBOOL *null_face_created_opt);

void csmsetopcom_postprocess_join_edges(struct csmsolid_t *solid);

void csmsetopcom_remove_null_edges(csmArrayStruct(csmedge_t) *set_of_null_edges);

// Faces...

CONSTRUCTOR(csmArrayStruct(csmface_t) *, csmsetopcom_convert_inner_loops_of_null_faces_to_faces, (csmArrayStruct(csmface_t) *set_of_null_faces));

void csmsetopcom_reintroduce_holes_in_corresponding_faces(
                        csmArrayStruct(csmface_t) *set_of_null_faces,
                        const struct csmtolerance_t *tolerances);

void csmsetopcom_introduce_holes_in_in_component_null_faces_if_proceed(
                        struct csmsolid_t *solid,
                        const struct csmtolerance_t *tolerances,
                        csmArrayStruct(csmface_t) *set_of_null_faces);

void csmsetopcom_correct_faces_after_joining_null_edges(struct csmsolid_t *solid, const struct csmtolerance_t *tolerances);

void csmsetopcom_move_face_to_solid(
                        unsigned long recursion_level,
                        struct csmface_t *face, struct csmsolid_t *face_solid,
                        struct csmsolid_t *destination_solid);

enum csmsetop_classify_resp_solid_t csmsetopcom_classify_value_respect_to_plane(double value, double tolerance);

struct csmface_t *csmsetopcom_face_for_hedge_sector(struct csmhedge_t *hedge, struct csmhedge_t *hedge_next);

void csmsetopcom_enumerate_shells(struct csmsolid_t *solid, unsigned long *id_new_shell);

// Finish algorithm...

void csmsetopcom_cleanup_solid(struct csmsolid_t *origin_solid, struct csmsolid_t *destination_solid);

void csmsetopcom_cleanup_solid_setop(
                        struct csmsolid_t *origin_solid_A, struct csmsolid_t *origin_solid_B,
                        struct csmsolid_t *destination_solid);
