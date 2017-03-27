#include "csmfwddecl.hxx"

DLL_CYSOLIDMODELING void csmglue_join_solid2_to_solid1(
                            struct csmsolid_t *solid1, unsigned long id_face_solid1,
                            struct csmsolid_t **solid2, unsigned long id_face_solid2,
                            CYBOOL *could_merge_solids);
