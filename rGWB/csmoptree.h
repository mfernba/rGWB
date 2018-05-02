//
//  csmoptree.h
//  rGWB
//
//  Created by Manuel Fernández on 15/4/18.
//  Copyright © 2018 Manuel Fernández. All rights reserved.
//
//  csmoptree represents a hierarchical tree for building a solid
//

#include "csmfwddecl.hxx"

#ifdef __cplusplus
extern "C" {
#endif

DLL_RGWB void csmoptree_free(struct csmoptree_t **optree);


// Nodes...

DLL_RGWB CONSTRUCTOR(struct csmoptree_t *, csmoptree_new_node_solid, (struct csmsolid_t **solid));


DLL_RGWB CONSTRUCTOR(struct csmoptree_t *, csmoptree_new_node_boolean_union, (struct csmoptree_t **node1, struct csmoptree_t **node2));
                     
DLL_RGWB CONSTRUCTOR(struct csmoptree_t *, csmoptree_new_node_boolean_intersection, (struct csmoptree_t **node1, struct csmoptree_t **node2));
                     
DLL_RGWB CONSTRUCTOR(struct csmoptree_t *, csmoptree_new_node_boolean_difference_1_minus_2, (struct csmoptree_t **node1, struct csmoptree_t **node2));
                     
                     
DLL_RGWB CONSTRUCTOR(struct csmoptree_t *, csmoptree_new_node_split_and_get_above, (double A, double B, double C, double D, struct csmoptree_t **node));
                     
DLL_RGWB CONSTRUCTOR(struct csmoptree_t *, csmoptree_new_node_split_and_get_below, (double A, double B, double C, double D, struct csmoptree_t **node));
                     

DLL_RGWB CONSTRUCTOR(struct csmoptree_t *, csmoptree_new_node_transform, (
                        double Ux, double Uy, double Uz, double Dx,
                        double Vx, double Vy, double Vz, double Dy,
                        double Wx, double Wy, double Wz, double Dz,
                        struct csmoptree_t **node));

                     
// Evaluation...

DLL_RGWB enum csmoptree_result_t csmoptree_evaluate(struct csmoptree_t *node, struct csmsolid_t **solid);

#ifdef __cplusplus
}
#endif
