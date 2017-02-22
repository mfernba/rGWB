// Vertex...

#include "csmfwddecl.hxx"

CONSTRUCTOR(struct csmvertex_t *, csmvertex_crea, (double x, double y, double z, unsigned long *id_nuevo_elemento));

void csmvertex_destruye(struct csmvertex_t **vertex);


// Geometría...

void csmvertex_get_coordenadas(const struct csmvertex_t *vertex, double *x, double *y, double *z);

void csmvertex_set_coordenadas(struct csmvertex_t *vertex, double x, double y, double z);


// Lista...

struct csmvertex_t *csmvertex_next(struct csmvertex_t *vertex);
void csmvertex_set_next(struct csmvertex_t *vertex, struct csmvertex_t *next_vertex);

struct csmvertex_t *csmvertex_prev(struct csmvertex_t *vertex);
void csmvertex_set_prev(struct csmvertex_t *vertex, struct csmvertex_t *prev_vertex);
