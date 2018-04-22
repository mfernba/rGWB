// Convenience operator. Moves a loop from one face to other...

#include "csmfwddecl.hxx"

void csmeuler_lringmv(struct csmloop_t *loop, struct csmface_t *face, CSMBOOL as_outer_loop);

void csmeuler_lringmv_move_all_loops_from_face1_to_face2(struct csmface_t *face1, struct csmface_t *face2);
