//
//  csmglue.c
//  cysolidmodeling
//
//  Created by Manuel Fernández on 27/3/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmglue.h"

#include "csmsolid.inl"
#include "csmeuler_lkfmrh.inl"

#include "cyassert.h"
#include "cypespy.h"

// ----------------------------------------------------------------------------------------------------

static CYBOOL i_is_possible_to_merge_faces(struct csmface_t *face1, struct csmface_t *face2)
{
    return FALSO;
}

// ----------------------------------------------------------------------------------------------------

void csmglue_join_solid2_to_solid1(
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

    if (i_is_possible_to_merge_faces(face1, face2) == FALSO)
    {
        could_merge_solids_loc = FALSO;
    }
    else
    {
        struct csmsolid_t *solid2_loc;
        
        solid2_loc = ASIGNA_PUNTERO_PP_NO_NULL(solid2, struct csmsolid_t);
        csmsolid_merge_solids(solid1, solid2_loc);
    
        csmeuler_lkfmrh(face1, &face2);
                        
        csmsolid_destruye(&solid2_loc);
    }
    
    *could_merge_solids = could_merge_solids_loc;
}

