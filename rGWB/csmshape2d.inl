//
//  csmshape2d.inl
//  rGWB
//
//  Created by Manuel Fernández on 19/11/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmfwddecl.hxx"

unsigned long csmshape2d_polygon_count(const struct csmshape2d_t *shape2d);

unsigned long csmshape2d_point_polygon_count(const struct csmshape2d_t *shape2d, unsigned long idx_polygon);

void csmshape2d_point_polygon_coords(
						const struct csmshape2d_t *shape2d,
						unsigned long idx_polygon, unsigned long idx_point,
						double *x, double *y);

CSMBOOL csmshape2d_polygon_is_hole(const struct csmshape2d_t *shape2d, unsigned long idx_polygon);

// Drawing...

void csmshape2d_draw_3D(
                        const struct csmshape2d_t *shape2d,
                        double Xo, double Yo, double Zo,
                        double Ux, double Uy, double Uz, double Vx, double Vy, double Vz,
                        CSMBOOL draw_edges,
                        struct bsgraphics2_t *graficos);

