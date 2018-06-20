//
//  csmsurface.h
//  rGWB
//
//  Created by Manuel Fernández on 21/11/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmfwddecl.hxx"

CONSTRUCTOR(struct csmsurface_t *, csmsurface_new_undefined, (void));

CONSTRUCTOR(struct csmsurface_t *, csmsurface_new_ellipsoid, (double radius_x, double radius_y, double radius_z));

CONSTRUCTOR(struct csmsurface_t *, csmsurface_new_cylinder, (double heigth, double radius));

CONSTRUCTOR(struct csmsurface_t *, csmsurface_new_cone, (double heigth, double radius));

CONSTRUCTOR(struct csmsurface_t *, csmsurface_new_torus, (double R, double r));

CONSTRUCTOR(struct csmsurface_t *, csmsurface_new_hyperboloid, (double a, double c));

CONSTRUCTOR(struct csmsurface_t *, csmsurface_copy, (const struct csmsurface_t *surface));

void csmsurface_free(struct csmsurface_t **surface);

CONSTRUCTOR(struct csmsurface_t *, csmsurface_read, (struct csmsave_t *csmsave));

void csmsurface_write(const struct csmsurface_t *surface, struct csmsave_t *csmsave);


CSMBOOL csmsurface_surfaces_define_border_edge(
                        const struct csmsurface_t *surface1, double A1, double B1, double C1,
                        const struct csmsurface_t *surface2, double A2, double B2, double C2);

