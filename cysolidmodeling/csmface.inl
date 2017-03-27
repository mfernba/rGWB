// Face...

#include "csmfwddecl.hxx"

CONSTRUCTOR(struct csmface_t *, csmface_crea, (struct csmsolid_t *solido, unsigned long *id_nuevo_elemento));

CONSTRUCTOR(struct csmface_t *, csmface_duplicate, (
                        struct csmface_t *face,
                        struct csmsolid_t *fsolid,
                        unsigned long *id_nuevo_elemento,
                        struct csmhashtb(csmvertex_t) *relation_svertexs_old_to_new,
                        struct csmhashtb(csmhedge_t) *relation_shedges_old_to_new));

void csmface_destruye(struct csmface_t **face);

unsigned long csmface_id(const struct csmface_t *face);

void csmface_reassign_id(struct csmface_t *face, unsigned long *id_nuevo_elemento, unsigned long *new_id_opc);


// Topología...

struct csmsolid_t *csmface_fsolid(struct csmface_t *face);
void csmface_set_fsolid(struct csmface_t *face, struct csmsolid_t *solid);

struct csmloop_t *csmface_flout(struct csmface_t *face);
void csmface_set_flout(struct csmface_t *face, struct csmloop_t *loop);

struct csmloop_t *csmface_floops(struct csmface_t *face);
void csmface_set_floops(struct csmface_t *face, struct csmloop_t *loop);

void csmface_add_loop_while_removing_from_old(struct csmface_t *face, struct csmloop_t *loop);

void csmface_remove_loop(struct csmface_t *face, struct csmloop_t **loop);
