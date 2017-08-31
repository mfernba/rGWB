// Convenience operator. Moves a loop from one face to other...

#include "csmfwddecl.hxx"

void csmeuler_laringmv(struct csmface_t *face1, struct csmface_t *face2);

void csmeuler_laringmv_from_face1_to_2_if_fits_in_face(
                        struct csmface_t *face1, struct csmface_t *face2,
                        CYBOOL *did_move_some_loop_opt);
