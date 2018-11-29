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
struct csmoctree_element_t;
struct csmoctree_item_common_data_t;
csmArrayStruct(csmoctree_element_t);

typedef CSMBOOL (*csmoctree_FPtr_intersects_with_bbox)(const struct csmoctree_item_t *item, const struct csmbbox_t *bbox, const struct csmoctree_item_common_data_t *item_common_data);
#define CSMOCTREE_CHECK_FUNC_INTERSECTS_WITH_BBOX(function, type, common_data_type) ((CSMBOOL (*)(const struct type *, const struct csmbbox_t *, const struct common_data_type *))function == function)

typedef void (*csmoctree_FPtr_maximize_bbox)(const struct csmoctree_item_t *item, struct csmbbox_t *bbox, const struct csmoctree_item_common_data_t *item_common_data);
#define CSMOCTREE_CHECK_FUNC_MAXIMIZE_BBOX(function, type, common_data_type) ((void (*)(const struct type *, struct csmbbox_t *, const struct common_data_type *))function == function)

#endif /* csmoctree_h */
