// Loop gluing...

#include "csmfwddecl.hxx"

CYBOOL csmloopglue_can_merge_faces(struct csmface_t *face1, struct csmface_t *face2);

void csmloopglue_merge_faces(struct csmface_t *face1, struct csmface_t **face2);
