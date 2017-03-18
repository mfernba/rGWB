// Face...

#include "csmfwddecl.hxx"

CONSTRUCTOR(struct csmface_t *, csmface_crea, (struct csmsolid_t *solido, unsigned long *id_nuevo_elemento));

void csmface_destruye(struct csmface_t **face);

unsigned long csmface_id(const struct csmface_t *face);


// Topología...

struct csmsolid_t *csmface_fsolid(struct csmface_t *face);
void csmface_set_fsolid(struct csmface_t *face, struct csmsolid_t *solid);

struct csmloop_t *csmface_flout(struct csmface_t *face);
void csmface_set_flout(struct csmface_t *face, struct csmloop_t *loop);

struct csmloop_t *csmface_floops(struct csmface_t *face);
void csmface_set_floops(struct csmface_t *face, struct csmloop_t *loop);

void csmface_add_loop_while_removing_from_old(struct csmface_t *face, struct csmloop_t *loop);

void csmface_remove_loop(struct csmface_t *face, struct csmloop_t **loop);
