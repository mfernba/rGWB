//
//  csmglue.c
//  rGWB
//
//  Created by Manuel Fernández on 27/3/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmglue.h"

#include "csmassert.inl"
#include "csmloopglue.inl"
#include "csmmem.inl"
#include "csmsolid.h"
#include "csmsolid.inl"
#include "csmtolerance.inl"

// ----------------------------------------------------------------------------------------------------

void csmglue_join_solid2_to_solid1_given_equal_faces(
                        struct csmsolid_t *solid1, unsigned long id_face_solid1,
                        struct csmsolid_t **solid2, unsigned long id_face_solid2,
                        CSMBOOL *could_merge_solids)
{
    CSMBOOL could_merge_solids_loc;
    struct csmface_t *face1, *face2;
    struct csmtolerance_t *tolerances;
    
    assert_no_null(solid2);
    assert_no_null(could_merge_solids);
    
    face1 = csmsolid_get_face(solid1, id_face_solid1);
    face2 = csmsolid_get_face(*solid2, id_face_solid2);
    
    tolerances = csmtolerance_new();

    if (csmloopglue_can_merge_faces(face1, face2, tolerances) == CSMFALSE)
    {
        could_merge_solids_loc = CSMFALSE;
    }
    else
    {
        struct csmsolid_t *solid2_loc;
        
        could_merge_solids_loc = CSMTRUE;
        
        solid2_loc = ASIGNA_PUNTERO_PP_NO_NULL(solid2, struct csmsolid_t);
        
        csmsolid_merge_solids(solid1, solid2_loc);
        csmloopglue_merge_faces(face1, &face2, tolerances);
                        
        csmsolid_free(&solid2_loc);
    }
    
    csmtolerance_free(&tolerances);
    
    *could_merge_solids = could_merge_solids_loc;
}

