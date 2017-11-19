#include "csmfwddecl.hxx"

DLL_RGWB void csmglue_join_solid2_to_solid1_given_equal_faces(
                        struct csmsolid_t *solid1, unsigned long id_face_solid1,
                        struct csmsolid_t **solid2, unsigned long id_face_solid2,
                        CSMBOOL *could_merge_solids);

