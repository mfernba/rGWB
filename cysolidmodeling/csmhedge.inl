// Half-Edge...

#include "csmfwddecl.hxx"

CONSTRUCTOR(struct csmhedge_t *, csmhedge_crea, (unsigned long *id_nuevo_elemento));


// Topología...

struct csmedge_t *csmhedge_edge(struct csmhedge_t *hedge);
void csmhedge_set_edge(struct csmhedge_t *hedge, struct csmedge_t *edge);

struct csmvertex_t *csmhedge_vertex(struct csmhedge_t *hedge);
void csmhedge_set_vertex(struct csmhedge_t *hedge, struct csmvertex_t *vertex);

struct csmloop_t *csmhedge_loop(struct csmhedge_t *hedge);
void csmhedge_set_loop(struct csmhedge_t *hedge, struct csmloop_t *loop);

// Lista...

struct csmhedge_t *csmhedge_next(struct csmhedge_t *hedge);
void csmhedge_set_next(struct csmhedge_t *hedge, struct csmhedge_t *next_hedge);

struct csmhedge_t *csmhedge_prev(struct csmhedge_t *hedge);
void csmhedge_set_prev(struct csmhedge_t *hedge, struct csmhedge_t *prev_hedge);
