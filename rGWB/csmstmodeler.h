//
//  csmstmodeler.h
//  rGWB
//
//  Created by Manuel Fernández on 16/4/18.
//  Copyright © 2018 Manuel Fernández. All rights reserved.
//
// A stack based modeler
//

#include "csmfwddecl.hxx"
#include "csmstmodeler.hxx"

#ifdef __cplusplus
extern "C" {
#endif

DLL_RGWB CONSTRUCTOR(struct csmstmodeler_t *, csmstmodeler_new, (void));

DLL_RGWB void csmstmodeler_free(struct csmstmodeler_t **modeler);


// Solid stack manipulation...

DLL_RGWB void csmstmodeler_clear_stack(struct csmstmodeler_t *modeler);

DLL_RGWB unsigned long csmstmodeler_push_solid(struct csmstmodeler_t *modeler, struct csmsolid_t **solid);

DLL_RGWB enum csmstmodeler_result_t csmstmodeler_push_solid_by_id(struct csmstmodeler_t *modeler, unsigned long solid_id);

DLL_RGWB enum csmstmodeler_result_t csmstmodeler_duplicate_solid(struct csmstmodeler_t *modeler, unsigned long solid_id, unsigned long *solid_id_opt);
    
DLL_RGWB enum csmstmodeler_result_t csmstmodeler_get_solid_reference(const struct csmstmodeler_t *modeler, unsigned long solid_id, struct csmsolid_t **solid_reference);
                     
DLL_RGWB enum csmstmodeler_result_t csmstmodeler_get_solid_copy(const struct csmstmodeler_t *modeler, unsigned long solid_id, struct csmsolid_t **solid_copy);


// Transforms (applied to the top of the stack solid)...

DLL_RGWB enum csmstmodeler_result_t csmstmodeler_apply_move(struct csmstmodeler_t *modeler, double dx, double dy, double dz);

DLL_RGWB enum csmstmodeler_result_t csmstmodeler_apply_scale(struct csmstmodeler_t *modeler, double sx, double sy, double sz);

DLL_RGWB enum csmstmodeler_result_t csmstmodeler_apply_rotation(
                        struct csmstmodeler_t *modeler,
                        double angle_radians,
                        double Xo, double Yo, double Zo, double Ux, double Uy, double Uz);

DLL_RGWB enum csmstmodeler_result_t csmstmodeler_apply_general_transform(
                        struct csmstmodeler_t *modeler,
                        double Ux, double Uy, double Uz, double Dx,
                        double Vx, double Vy, double Vz, double Dy,
                        double Wx, double Wy, double Wz, double Dz);


// Split (applied to the top of the stack solid, at least needs one operand in the stack)...

DLL_RGWB enum csmstmodeler_result_t csmstmodeler_do_split_and_push_above(
                        struct csmstmodeler_t *modeler,
                        double A, double B, double C, double D,
                        unsigned long *solid_id_opt);

DLL_RGWB enum csmstmodeler_result_t csmstmodeler_do_split_and_push_below(
                        struct csmstmodeler_t *modeler,
                        double A, double B, double C, double D,
                        unsigned long *solid_id_opt);
    

// Boolean operations (applied to the two top of the stack solids, at least needs two operands in the stack)...

DLL_RGWB enum csmstmodeler_result_t csmstmodeler_do_union(struct csmstmodeler_t *modeler, unsigned long *solid_id_opt);

DLL_RGWB enum csmstmodeler_result_t csmstmodeler_do_intersection(struct csmstmodeler_t *modeler, unsigned long *solid_id_opt);

DLL_RGWB enum csmstmodeler_result_t csmstmodeler_do_difference(struct csmstmodeler_t *modeler, unsigned long *solid_id_opt);


#ifdef __cplusplus
}
#endif


