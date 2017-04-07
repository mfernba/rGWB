//
//  csmglue.c
//  cysolidmodeling
//
//  Created by Manuel Fernández on 27/3/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmglue.h"

#include "csmloopglue.inl"

#include "csmsolid.inl"

#include "cyassert.h"
#include "cypespy.h"

// ----------------------------------------------------------------------------------------------------

void csmglue_join_solid2_to_solid1_given_equal_faces(
                        struct csmsolid_t *solid1, unsigned long id_face_solid1,
                        struct csmsolid_t **solid2, unsigned long id_face_solid2,
                        CYBOOL *could_merge_solids)
{
    CYBOOL could_merge_solids_loc;
    struct csmface_t *face1, *face2;
    
    assert_no_null(solid2);
    assert_no_null(could_merge_solids);
    
    face1 = csmsolid_get_face(solid1, id_face_solid1);
    face2 = csmsolid_get_face(*solid2, id_face_solid2);

    if (csmloopglue_can_merge_faces(face1, face2) == FALSO)
    {
        could_merge_solids_loc = FALSO;
    }
    else
    {
        struct csmsolid_t *solid2_loc;
        
        could_merge_solids_loc = CIERTO;
        
        solid2_loc = ASIGNA_PUNTERO_PP_NO_NULL(solid2, struct csmsolid_t);
        
        csmsolid_merge_solids(solid1, solid2_loc);
        //csmsolid_print_debug(solid1, CIERTO);
        
        csmloopglue_merge_faces(face1, &face2);
                        
        csmsolid_destruye(&solid2_loc);
    }
    
    *could_merge_solids = could_merge_solids_loc;
}
