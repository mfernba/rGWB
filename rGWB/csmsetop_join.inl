//
//  csmsetop_join.h
//  rGWB
//
//  Created by Manuel Fernández on 6/3/18.
//  Copyright © 2018 Manuel Fernández. All rights reserved.
//

#include "csmfwddecl.hxx"

void csmsetop_join_null_edges(
                        struct csmsolid_t *solid_A, csmArrayStruct(csmedge_t) *set_of_null_edges_A,
                        struct csmsolid_t *solid_B, csmArrayStruct(csmedge_t) *set_of_null_edges_B,
                        const struct csmtolerance_t *tolerances,
                        csmArrayStruct(csmface_t) **set_of_null_faces_A,
                        csmArrayStruct(csmface_t) **set_of_null_faces_B,
                        CSMBOOL *did_join_all_null_edges);

