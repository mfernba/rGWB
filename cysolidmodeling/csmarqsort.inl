//
//  csmarqsort.h
//  rGWB
//
//  Created by Manuel Fernández on 6/10/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include <stdlib.h>

void csmarqsort(void *aa, size_t n, size_t es, void *cmp_data, int (*cmp)(const void *, const void *, const void *));
