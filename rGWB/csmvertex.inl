// Vertex...

#include "csmfwddecl.hxx"
#include "csmvertex.hxx"

CONSTRUCTOR(struct csmvertex_t *, csmvertex_new, (double x, double y, double z, unsigned long *id_new_element));

CONSTRUCTOR(struct csmvertex_t *, csmvertex_duplicate, (
                        const struct csmvertex_t *vertex,
                        unsigned long *id_new_element,
                        struct csmhashtb(csmvertex_t) *relation_svertexs_old_to_new));

CONSTRUCTOR(struct csmvertex_t *, csmvertex_new_from_writeable_vertex, (const struct csmwriteablesolid_vertex_t *w_vertex));

void csmvertex_free(struct csmvertex_t **vertex);


unsigned long csmvertex_id(const struct csmvertex_t *vertex);

void csmvertex_reassign_id(struct csmvertex_t *vertex, unsigned long *id_new_element, unsigned long *new_id_opc);


// Topology...

struct csmhedge_t *csmvertex_hedge(struct csmvertex_t *vertex);

void csmvertex_set_hedge(struct csmvertex_t *vertex, struct csmhedge_t *hedge);


// Algorithm masks...

CSMBOOL csmvertex_has_mask_attrib(const struct csmvertex_t *vertex, csmvertex_mask_t mask_attrib);

csmvertex_mask_t csmvertex_get_mask_attrib(const struct csmvertex_t *vertex);

void csmvertex_set_mask_attrib(struct csmvertex_t *vertex, csmvertex_mask_t mask_attrib);

void csmvertex_clear_mask_attrib(struct csmvertex_t *vertex, csmvertex_mask_t mask_attrib);

void csmvertex_clear_mask(struct csmvertex_t *vertex);


// Geometry...

void csmvertex_get_coords(const struct csmvertex_t *vertex, double *x, double *y, double *z);

void csmvertex_vector_from_vertex1_to_vertex2(
                        const struct csmvertex_t *vertex1, const struct csmvertex_t *vertex2,
                        double *Ux, double *Uy, double *Uz);

void csmvertex_implicit_plane_equation_given_3_vertexs(
                        const struct csmvertex_t *vertex1, const struct csmvertex_t *vertex2, const struct csmvertex_t *vertex3,
                        double *A, double *B, double *C, double *D);

double csmvertex_signed_distance_to_plane(const struct csmvertex_t *vertex, double A, double B, double C, double D);

void csmvertex_get_coords_not_dropped(
                        const struct csmvertex_t *vertex,
                        enum csmmath_dropped_coord_t dropped_coord,
                        double *x, double *y);

CSMBOOL csmvertex_equal_coords(const struct csmvertex_t *vertex1, const struct csmvertex_t *vertex2, double epsilon);


void csmvertex_apply_transform(struct csmvertex_t *vertex, const struct csmtransform_t *transform);


// Debug...

void csmvertex_print_debug_info(struct csmvertex_t *vertex);


