// Vertex...

#include "csmfwddecl.hxx"

CONSTRUCTOR(struct csmvertex_t *, csmvertex_crea, (double x, double y, double z, unsigned long *id_nuevo_elemento));

CONSTRUCTOR(struct csmvertex_t *, csmvertex_duplicate, (
                        const struct csmvertex_t *vertex,
                        unsigned long *id_nuevo_elemento,
                        struct csmhashtb(csmvertex_t) *relation_svertexs_old_to_new));

void csmvertex_destruye(struct csmvertex_t **vertex);


unsigned long csmvertex_id(const struct csmvertex_t *vertex);

void csmvertex_reassign_id(struct csmvertex_t *vertex, unsigned long *id_nuevo_elemento, unsigned long *new_id_opc);


// Topología...

struct csmhedge_t *csmvertex_hedge(struct csmvertex_t *vertex);

void csmvertex_set_hedge(struct csmvertex_t *vertex, struct csmhedge_t *hedge);



// Geometría...

void csmvertex_get_coordenadas(const struct csmvertex_t *vertex, double *x, double *y, double *z);

void csmvertex_get_coords_not_dropped(
                        const struct csmvertex_t *vertex,
                        enum csmmath_dropped_coord_t dropped_coord,
                        double *x, double *y);

void csmvertex_set_coordenadas(struct csmvertex_t *vertex, double x, double y, double z);

CYBOOL csmvertex_equal_coords(const struct csmvertex_t *vertex1, const struct csmvertex_t *vertex2, double epsilon);


void csmvertex_apply_transform(struct csmvertex_t *vertex, const struct csmtransform_t *transform);
