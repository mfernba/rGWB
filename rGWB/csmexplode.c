// Explode a solid into several shells (if they exist...)

#include "csmexplode.h"

#include "csmarrayc.inl"
#include "csmdebug.inl"
#include "csmhashtb.inl"
#include "csmsetopcom.inl"
#include "csmsolid.h"
#include "csmsolid.inl"

#ifdef __STANDALONE_DISTRIBUTABLE
#include "csmassert.inl"
#else
#include "cyassert.h"
#endif

// --------------------------------------------------------------------------------

csmArrayStruct(csmsolid_t) *csmexplode_explode_shells(const struct csmsolid_t *solid)
{
    csmArrayStruct(csmsolid_t) *solids;
    struct csmsolid_t *work_solid;
    CSMBOOL did_move_faces;
    
    solids = csmarrayc_new_st_array(0, csmsolid_t);
    
    work_solid = csmsolid_duplicate(solid);
    did_move_faces = CSMFALSE;
    
    if (csmdebug_debug_enabled() == CSMTRUE)
        csmsolid_print_debug(work_solid, CSMTRUE);
    
    do
    {
        struct csmhashtb_iterator(csmface_t) *face_iterator;
        
        face_iterator = csmsolid_face_iterator(work_solid);
        did_move_faces = CSMFALSE;
        
        while (csmhashtb_has_next(face_iterator, csmface_t) == CSMTRUE)
        {
            struct csmsolid_t *independent_shell;
            struct csmface_t *face;
            
            csmhashtb_next_pair(face_iterator, NULL, &face, csmface_t);
            
            independent_shell = csmsolid_crea_vacio(0);
            csmsolid_set_name(independent_shell, "Independent shell");
            csmsetopcom_move_face_to_solid(0, face, work_solid, independent_shell);
            csmsetopcom_cleanup_solid(work_solid, independent_shell);
            
            if (csmdebug_debug_enabled() == CSMTRUE)
            {
                csmsolid_print_debug(work_solid, CSMTRUE);
                csmsolid_print_debug(independent_shell, CSMTRUE);
            }
            
            csmarrayc_append_element_st(solids, independent_shell, csmsolid_t);
            did_move_faces = CSMTRUE;
            break;
        }
        
        csmhashtb_free_iterator(&face_iterator, csmface_t);
        
    } while (did_move_faces == CSMTRUE);
    
    csmsolid_free(&work_solid);
    
    return solids;
}
