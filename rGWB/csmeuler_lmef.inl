// Low level make edge-face...

#include "csmfwddecl.hxx"

void csmeuler_lmef(
                struct csmhedge_t *he1, struct csmhedge_t *he2,
                struct csmface_t **new_face_opc,
                struct csmhedge_t **new_he_pos_opc, struct csmhedge_t **new_he_neg_opc);
