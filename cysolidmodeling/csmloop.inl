// Loop...

#include "csmfwddecl.hxx"

struct csmloop_t *csmloop_crea(unsigned long *id_nuevo_elemento);

void csmloop_destruye(struct csmloop_t **loop);


// Topología...

struct csmhedge_t *csmloop_ledge(struct csmloop_t *loop);
void csmloop_set_ledge(struct csmloop_t *loop, struct csmhedge_t *ledge);

struct csmface_t *csmloop_face(struct csmloop_t *loop);
void csmloop_set_face(struct csmloop_t *loop, struct csmface_t *face);


// Lista...

struct csmloop_t *csmloop_next(struct csmloop_t *loop);
void csmloop_set_next(struct csmloop_t *loop, struct csmloop_t *next_loop);

struct csmloop_t *csmloop_prev(struct csmloop_t *loop);
void csmloop_set_prev(struct csmloop_t *loop, struct csmloop_t *prev_loop);

