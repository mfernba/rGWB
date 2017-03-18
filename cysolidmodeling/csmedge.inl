// Edge...

#include "csmfwddecl.hxx"

CONSTRUCTOR(struct csmedge_t *, csmedge_crea, (unsigned long *id_nuevo_elemento));

void csmedge_destruye(struct csmedge_t **edge);

unsigned long csmedge_id(const struct csmedge_t *edge);


// Topología...

struct csmhedge_t *csmedge_hedge_lado(struct csmedge_t *edge, enum csmedge_lado_hedge_t lado);
void csmedge_set_edge_lado(struct csmedge_t *edge, enum csmedge_lado_hedge_t lado, struct csmhedge_t *hedge);

void csmedge_remove_hedge(struct csmedge_t *edge, struct csmhedge_t *hedge);

struct csmhedge_t *csmedge_mate(struct csmedge_t *edge, const struct csmhedge_t *hedge);
