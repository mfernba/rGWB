//
//  csmoctree.hxx
//  rGWB
//
//  Created by Manuel Fernandez on 10/11/18.
//  Copyright © 2018 Manuel Fernández. All rights reserved.
//

#ifndef csmoctree_hxx
#define csmoctree_hxx

#include "csmarrayc.hxx"

struct csmoctree_item_t;
csmArrayStruct(csmoctree_item_t);

#define csmoctree(type) csmoctree_##type

typedef CSMBOOL (*csmoctree_FPtr_intersects_with_bbox)(const struct csmoctree_item_t *item, const struct csmbbox_t *bbox);
#define CSMOCTREE_CHECK_FUNC_INTERSECTS_WITH_BBOX(function, type) ((CSMBOOL (*)(const struct type *, const struct csmbbox_t *))function == function)

#endif /* csmoctree_h */
