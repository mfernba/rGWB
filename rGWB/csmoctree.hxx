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

enum csmoctree_bbox_inters_t
{
    CSMOCTREE_BBOX_INTERS_NO,
    CSMOCTREE_BBOX_INTERS_INTERIOR,
    CSMOCTREE_BBOX_INTERS_IN_BBOX_FACE
};

typedef enum csmoctree_bbox_inters_t (*csmoctree_FPtr_intersects_with_bbox)(const struct csmoctree_item_t *item, const struct csmbbox_t *bbox, double tolerance);
#define CSMOCTREE_CHECK_FUNC_INTERSECTS_WITH_BBOX(function, type) ((enum csmoctree_bbox_inters_t (*)(const struct type *, const struct csmbbox_t *, double))function == function)

typedef void (*csmoctree_FPtr_print_debug_info)(const struct csmoctree_item_t *item);
#define CSMOCTREE_CHECK_FUNC_PRINT_DEBUG_INFO(function, type) ((void (*)(const struct type *))function == function)

#endif /* csmoctree_h */
