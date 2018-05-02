//
//  csmfacbrep2solid.h
//  rGWB
//
//  Created by Manuel Fernandez on 21/4/18.
//  Copyright © 2018 Manuel Fernández. All rights reserved.
//

#include "csmfwddecl.hxx"
#include "csmfacbrep2solid.hxx"

#ifdef __cplusplus
extern "C" {
#endif

DLL_RGWB CONSTRUCTOR(struct csmfacbrep2solid_t *, csmfacbrep2solid_new, (double equal_points_tolerance, CSMBOOL face_normal_point_out_of_solid));

DLL_RGWB void csmfacbrep2solid_free(struct csmfacbrep2solid_t **builder);


// Faces...
    
DLL_RGWB void csmfacbrep2solid_append_face(struct csmfacbrep2solid_t *builder, struct csmfacbrep2solid_face_t **face);
    
    
DLL_RGWB CONSTRUCTOR(struct csmfacbrep2solid_face_t *, csmfacbrep2solid_new_face, (void));

DLL_RGWB void csmfacbrep2solid_append_outer_loop_to_face(struct csmfacbrep2solid_face_t *face, struct csmfacbrep2solid_loop_t **outer_loop);

DLL_RGWB void csmfacbrep2solid_append_inner_loop_to_face(struct csmfacbrep2solid_face_t *face, struct csmfacbrep2solid_loop_t **inner_loop);
    

DLL_RGWB CONSTRUCTOR(struct csmfacbrep2solid_loop_t *, csmfacbrep2solid_new_loop, (void));
    
DLL_RGWB void csmfacbrep2solid_append_point_to_loop(struct csmfacbrep2solid_loop_t *loop, double x, double y, double z);

DLL_RGWB void csmfacbrep2solid_reverse(struct csmfacbrep2solid_loop_t *loop);


// Build...
    
DLL_RGWB enum csmfacbrep2solid_result_t csmfacbrep2solid_build(struct csmfacbrep2solid_t *builder, struct csmsolid_t **solid);
    
    
#ifdef __cplusplus
}
#endif
