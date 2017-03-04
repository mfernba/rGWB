// Operaciones básicas sobre elementos con dependencias entre sí...

#include "csmfwddecl.hxx"

struct csmhedge_t *csmopbas_mate(struct csmhedge_t *hedge);

struct csmsolid_t *csmopbas_solid_from_hedge(struct csmhedge_t *hedge);

void csmopbas_addhe(
                struct csmedge_t *edge,
                struct csmvertex_t *vertex,
                struct csmhedge_t *hedge,
                enum csmedge_lado_hedge_t lado,
                unsigned long *id_nuevo_elemento,
                struct csmhedge_t **new_hedge_opc);

void csmopbas_delhe(struct csmhedge_t **hedge, struct csmhedge_t **hedge_prev_opc);
