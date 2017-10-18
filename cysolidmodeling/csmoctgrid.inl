//
//  csmoctree.h
//  cysolidmodeling
//
//  Created by Manuel Fernández on 17/10/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmfwddecl.hxx"

CONSTRUCTOR(struct csmoctgrid_t *, csmoctgrid_build, (struct csmsolid_t *solid_a, struct csmsolid_t *solid_b));

void csmoctgrid_free(struct csmoctgrid_t **octgrid);
