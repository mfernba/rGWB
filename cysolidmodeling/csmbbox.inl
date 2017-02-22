// Bounding box...

#include "csmfwddecl.hxx"

struct csmbbox_t *csmbbox_crea_vacia(void);

void csmbbox_destruye(struct csmbbox_t **bbox);


void csmbbox_maximiza_coordenada(struct csmbbox_t *bbox, double x, double y, double z);

void csmbbox_maximiza_bbox(struct csmbbox_t *bbox_maximizar, const struct csmbbox_t *bbox);

