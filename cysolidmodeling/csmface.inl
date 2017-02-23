// Face...

#include "csmfwddecl.hxx"

CONSTRUCTOR(struct csmface_t *, csmface_crea, (struct csmsolid_t *fsolid, unsigned long *id_nuevo_elemento));


// Topología...

struct csmsolid_t *csmface_fsolid(struct csmface_t *face);
void csmface_set_fsolid(struct csmface_t *face, struct csmsolid_t *solid);

struct csmloop_t *csmface_flout(struct csmface_t *face);
void csmface_set_flout(struct csmface_t *face, struct csmloop_t *loop);

struct csmloop_t *csmface_floops(struct csmface_t *face);
void csmface_set_floops(struct csmface_t *face, struct csmloop_t *loop);


// Lista...

struct csmface_t *csmface_next(struct csmface_t *face);
void csmface_set_next(struct csmface_t *face, struct csmface_t *next_face);

struct csmface_t *csmface_prev(struct csmface_t *face);
void csmface_set_prev(struct csmface_t *face, struct csmface_t *prev_face);
