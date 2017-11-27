// Tolerances...

#include "csmfwddecl.hxx"

CONSTRUCTOR(struct csmtolerance_t *, csmtolerance_new,  (void));

void csmtolerance_free(struct csmtolerance_t **tolerance);

double csmtolerance_equal_coords(const struct csmtolerance_t *tolerance);

double csmtolerance_angle_rad(const struct csmtolerance_t *tolerance);

double csmtolerance_dot_product_parallel_vectors(const struct csmtolerance_t *tolerance);

double csmtolerance_coplanarity(const struct csmtolerance_t *tolerance);

double csmtolerance_relative_position_over_edge(const struct csmtolerance_t *tolerance);

double csmtolerance_null_vector(const struct csmtolerance_t *tolerance);

double csmtolerance_bbox_absolute_tolerance(const struct csmtolerance_t *tolerance);


double csmtolerance_default_null_vector(void);

double csmtolerance_default_coplanarity(void);
