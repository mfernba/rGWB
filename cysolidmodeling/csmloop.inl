// Loop...

#include "csmfwddecl.hxx"

CONSTRUCTOR(struct csmloop_t *, csmloop_crea, (struct csmface_t *face, unsigned long *id_nuevo_elemento));

CONSTRUCTOR(struct csmloop_t *, csmloop_duplicate, (
                        const struct csmloop_t *loop,
                        struct csmface_t *lface,
                        unsigned long *id_nuevo_elemento,
                        struct csmhashtb(csmvertex_t) *relation_svertexs_old_to_new,
                        struct csmhashtb(csmhedge_t) *relation_shedges_old_to_new));


// Topología...

struct csmhedge_t *csmloop_ledge(struct csmloop_t *loop);
void csmloop_set_ledge(struct csmloop_t *loop, struct csmhedge_t *ledge);

struct csmface_t *csmloop_lface(struct csmloop_t *loop);
void csmloop_set_lface(struct csmloop_t *loop, struct csmface_t *face);


// Lista...

struct csmloop_t *csmloop_next(struct csmloop_t *loop);

struct csmloop_t *csmloop_prev(struct csmloop_t *loop);

