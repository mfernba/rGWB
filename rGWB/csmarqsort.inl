//
//  csmarqsort.h
//  rGWB
//
//  Created by Manuel Fernández on 6/10/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmfwddecl.hxx"

void csmarqsort(void *const pbase, size_t total_elems, size_t size, void *arg, int (*cmp)(const void *e1, const void *e2, const void *data));
