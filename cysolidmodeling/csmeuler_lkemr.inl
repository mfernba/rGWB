// low level kill edge make ring

#include "csmfwddecl.hxx"

void csmeuler_lkemr(
                struct csmhedge_t **he_to_ring, struct csmhedge_t **he_from_ring,
                unsigned long *id_nuevo_elemento,
                struct csmhedge_t **he_to_ring_next, struct csmhedge_t **he_from_ring_next);
