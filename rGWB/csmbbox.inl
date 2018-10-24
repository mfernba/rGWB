// Bounding box...

#include "csmfwddecl.hxx"

CONSTRUCTOR(struct csmbbox_t *, csmbbox_create_empty_box, (void));

CONSTRUCTOR(struct csmbbox_t *, csmbbox_copy, (const struct csmbbox_t *bbox));

void csmbbox_free(struct csmbbox_t **bbox);


void csmbbox_reset(struct csmbbox_t *bbox);

void csmbbox_maximize_coord(struct csmbbox_t *bbox, double x, double y, double z);

void csmbbox_maximize_bbox(struct csmbbox_t *bbox_maximizar, const struct csmbbox_t *bbox);

void csmbbox_compute_bsphere_and_margins(struct csmbbox_t *bbox);

void csmbbox_get_extension_real(
	                    const struct csmbbox_t *bbox,
                        double *x_min, double *y_min, double *z_min,
                        double *x_max, double *y_max, double *z_max);

void csmbbox_get_extension_ext(
	                    const struct csmbbox_t *bbox,
                        double *x_min, double *y_min, double *z_min,
                        double *x_max, double *y_max, double *z_max);

CSMBOOL csmbbox_contains_point(const struct csmbbox_t *bbox, double x, double y, double z);

CSMBOOL csmbbox_intersects_with_other_bbox(const struct csmbbox_t *bbox1, const struct csmbbox_t *bbox2);

CSMBOOL csmbbox_intersects_with_segment(
                        const struct csmbbox_t *bbox,
                        double x1, double y1, double z1, double x2, double y2, double z2);
