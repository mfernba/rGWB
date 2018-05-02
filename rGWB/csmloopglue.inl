// Loop gluing...

#include "csmfwddecl.hxx"

CSMBOOL csmloopglue_can_merge_faces(
                        struct csmface_t *face1, struct csmface_t *face2,
                        const struct csmtolerance_t *tolerances);

void csmloopglue_merge_faces(
                        struct csmface_t *face1, struct csmface_t **face2,
                        const struct csmtolerance_t *tolerances);

void csmloopglue_merge_face_loops(
                        struct csmface_t *face,
                        const struct csmtolerance_t *tolerances);
