//
//  csmshape2d.h
//  rGWB
//
//  Created by Manuel Fernández on 19/11/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmfwddecl.hxx"

#ifdef __cplusplus
extern "C" {
#endif

DLL_RGWB CONSTRUCTOR(struct csmshape2d_t *, csmshape2d_new, (void));

DLL_RGWB CONSTRUCTOR(struct csmshape2d_t *, csmshape2d_copy, (const struct csmshape2d_t *shape2d));
    
DLL_RGWB void csmshape2d_free(struct csmshape2d_t **shape2d);


// Polygons...
    
DLL_RGWB void csmshape2d_new_polygon(struct csmshape2d_t *shape2d, unsigned long *idx_new_polygon);

DLL_RGWB void csmshape2d_append_point_to_polygon(struct csmshape2d_t *shape2d, unsigned long idx_polygon, double x, double y);

DLL_RGWB void csmshape2d_append_new_polygon_with_points(struct csmshape2d_t *shape2d, csmArrPoint2D **points);
    
DLL_RGWB void csmshape2d_append_new_polygon_with_points_and_normals(struct csmshape2d_t *shape2d, csmArrPoint2D **points, csmArrPoint3D **normals);


// Transformations...

DLL_RGWB void csmshape2d_move(struct csmshape2d_t *shape2d, double dx, double dy);

    
#ifdef __cplusplus
}
#endif
