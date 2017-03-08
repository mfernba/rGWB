// Loop...

#include "csmfwddecl.hxx"

CONSTRUCTOR(struct csmloop_t *, csmloop_crea, (struct csmface_t *face, unsigned long *id_nuevo_elemento));


CYBOOL csmloop_ids_iguales(const struct csmloop_t *loop1, const struct csmloop_t *loop2);

// Topología...

struct csmhedge_t *csmloop_ledge(struct csmloop_t *loop);
void csmloop_set_ledge(struct csmloop_t *loop, struct csmhedge_t *ledge);

struct csmface_t *csmloop_lface(struct csmloop_t *loop);
void csmloop_set_lface(struct csmloop_t *loop, struct csmface_t *face);


// Lista...

struct csmloop_t *csmloop_next(struct csmloop_t *loop);

struct csmloop_t *csmloop_prev(struct csmloop_t *loop);

