// Operaciones básicas sobre elementos con dependencias entre sí...

#include "csmfwddecl.hxx"

// Half-Edges...

struct csmhedge_t *csmopbas_mate(struct csmhedge_t *hedge);

struct csmface_t *csmopbas_face_from_hedge(struct csmhedge_t *hedge);

struct csmsolid_t *csmopbas_solid_from_hedge(struct csmhedge_t *hedge);

struct csmsolid_t *csmopbas_solid_from_hedges(struct csmhedge_t *he1, struct csmhedge_t *he2);


void csmopbas_addhe(
                struct csmedge_t *edge,
                struct csmvertex_t *vertex,
                struct csmhedge_t *hedge,
                enum csmedge_lado_hedge_t lado,
                unsigned long *id_nuevo_elemento,
                struct csmhedge_t **new_hedge_opc);

void csmopbas_delhe(struct csmhedge_t **hedge, struct csmhedge_t **hedge_prev_opc, struct csmhedge_t **hedge_next_opc);


// Geometry...

CYBOOL csmopbas_is_wide_hedge(
                        struct csmhedge_t *hedge,
                        double *Ux_bisec_opt, double *Uy_bisec_opt, double *Uz_bisec_opt);
