// Bounding box...

#include "csmfwddecl.hxx"

struct csmbbox_t *csmbbox_create_empty_box(void);

void csmbbox_free(struct csmbbox_t **bbox);


void csmbbox_reset(struct csmbbox_t *bbox);

void csmbbox_maximize_coord(struct csmbbox_t *bbox, double x, double y, double z);

void csmbbox_maximize_bbox(struct csmbbox_t *bbox_maximizar, const struct csmbbox_t *bbox);

