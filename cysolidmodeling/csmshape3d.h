//
//  csmshape3d.h
//  cysolidmodeling
//
//  Created by Manuel Fernández on 25/9/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmfwddecl.hxx"

DLL_CYSOLIDMODELING CONSTRUCTOR(struct csmsolid_t *, csmshape3d_create_torus, (
                        double R, unsigned long no_points_circle_R,
                        double r, unsigned long no_points_circle_r,
                        double x_center, double y_center, double z_center,
                        double Ux, double Uy, double Uz, double Vx, double Vy, double Vz,
                        unsigned long start_id_of_new_element));
