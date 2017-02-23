// Edge...

#include "csmfwddecl.hxx"

CONSTRUCTOR(struct csmedge_t *, csmedge_crea, (unsigned long *id_nuevo_elemento));


// Topología...

struct csmhedge_t *csmedge_hedge_lado(struct csmedge_t *edge, enum csmedge_lado_hedge_t lado);

void csmedge_set_edge_lado(struct csmedge_t *edge, enum csmedge_lado_hedge_t lado, struct csmhedge_t *hedge);


// Lista...

struct csmedge_t *csmedge_next(struct csmedge_t *edge);
void csmedge_set_next(struct csmedge_t *edge, struct csmedge_t *next_edge);

struct csmedge_t *csmedge_prev(struct csmedge_t *edge);
void csmedge_set_prev(struct csmedge_t *edge, struct csmedge_t *prev_edge);
