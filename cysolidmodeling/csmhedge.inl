// Half-Edge...

#include "csmfwddecl.hxx"

CONSTRUCTOR(struct csmhedge_t *, csmhedge_crea, (unsigned long *id_nuevo_elemento));

CONSTRUCTOR(struct csmhedge_t *, csmhedge_duplicate, (
                        const struct csmhedge_t *hedge,
                        struct csmloop_t *loop,
                        unsigned long *id_nuevo_elemento,
                        struct csmhashtb(csmvertex_t) *relation_svertexs_old_to_new,
                        struct csmhashtb(csmhedge_t) *relation_shedges_old_to_new));


unsigned long csmhedge_id(const struct csmhedge_t *hedge);

CYBOOL csmhedge_id_igual(const struct csmhedge_t *hedge1, const struct csmhedge_t *hedge2);


// Topology...

struct csmedge_t *csmhedge_edge(struct csmhedge_t *hedge);
void csmhedge_set_edge(struct csmhedge_t *hedge, struct csmedge_t *edge);

struct csmvertex_t *csmhedge_vertex(struct csmhedge_t *hedge);
const struct csmvertex_t *csmhedge_vertex_const(const struct csmhedge_t *hedge);

void csmhedge_set_vertex(struct csmhedge_t *hedge, struct csmvertex_t *vertex);

struct csmloop_t *csmhedge_loop(struct csmhedge_t *hedge);
void csmhedge_set_loop(struct csmhedge_t *hedge, struct csmloop_t *loop);

// Lista...

struct csmhedge_t *csmhedge_next(struct csmhedge_t *hedge);
void csmhedge_set_next(struct csmhedge_t *hedge, struct csmhedge_t *next_hedge);

struct csmhedge_t *csmhedge_prev(struct csmhedge_t *hedge);
void csmhedge_set_prev(struct csmhedge_t *hedge, struct csmhedge_t *prev_hedge);
