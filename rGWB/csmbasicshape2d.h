//
//  csmbasicshape2d.h
//  rGWB
//
//  Created by Manuel Fernández on 19/11/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmfwddecl.hxx"

#ifdef __cplusplus
extern "C" {
#endif

DLL_RGWB CONSTRUCTOR(struct csmshape2d_t *, csmbasicshape2d_rectangular_shape, (double dim_x, double dim_y));

DLL_RGWB CONSTRUCTOR(struct csmshape2d_t *, csmbasicshape2d_rectangular_hollow_shape, (double dim_x, double dim_y, double dim_x_h, double dim_y_h));

DLL_RGWB CONSTRUCTOR(struct csmshape2d_t *, csmbasicshape2d_circular_shape, (double radius, unsigned long no_points_circle));

DLL_RGWB CONSTRUCTOR(struct csmshape2d_t *, csmbasicshape2d_circular_hollow_shape, (double outer_radius, double inner_radius, unsigned long no_points_circle));

DLL_RGWB CONSTRUCTOR(struct csmshape2d_t *, csmbasicshape2d_C_shape, (double dim_x, double dim_y));

DLL_RGWB CONSTRUCTOR(struct csmshape2d_t *, csmbasicshape2d_L_shape, (double dim_x, double dim_y));

DLL_RGWB CONSTRUCTOR(struct csmshape2d_t *, csmbasicshape2d_I_shape, (
                        double canto_alma, double espesor_alma,
                        double canto_ala, double espesor_ala));
    
#ifdef __cplusplus
}
#endif
