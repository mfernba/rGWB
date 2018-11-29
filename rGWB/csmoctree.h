// Octree...

#include "csmfwddecl.hxx"
#include "csmoctree.hxx"

DLL_RGWB CONSTRUCTOR(struct csmoctree_t *, csmoctree_new, (unsigned long max_occupancy));

DLL_RGWB void csmoctree_free(struct csmoctree_t **octree);

DLL_RGWB void csmoctree_dontuse_append_item(
                        struct csmoctree_t *octree,
                        const struct csmoctree_item_t *octree_item, const char *octree_element_type,
                        csmoctree_FPtr_intersects_with_bbox func_intersects_with_bbox,
                        csmoctree_FPtr_maximize_bbox func_maximize_bbox);
#define csmoctree_append_item(\
                        octree,\
                        octree_item, item_type, item_common_data_type,\
                        func_intersects_with_bbox,\
                        func_maximize_bbox)\
    (/*lint -save -e505*/\
        ((const struct item_type *)octree_item == octree_item),\
        CSMOCTREE_CHECK_FUNC_INTERSECTS_WITH_BBOX(func_intersects_with_bbox, item_type, item_common_data_type),\
        CSMOCTREE_CHECK_FUNC_MAXIMIZE_BBOX(func_maximize_bbox, item_type, item_common_data_type),\
        csmoctree_dontuse_append_item(\
                        octree,\
                        (const struct csmoctree_item_t *)octree_item, #item_type,\
                        (csmoctree_FPtr_intersects_with_bbox)func_intersects_with_bbox,\
                        (csmoctree_FPtr_maximize_bbox)func_maximize_bbox)\
    )/*lint -restore*/


DLL_RGWB void csmoctree_build(struct csmoctree_t *octree);

DLL_RGWB CONSTRUCTOR(const csmArrayStruct(csmoctree_element_t) *, csmoctree_get_bbox_neighbors, (const struct csmoctree_t *octree, const struct csmbbox_t *bbox));

DLL_RGWB const struct csmoctree_item_t *csmoctree_dontuse_octree_item_cast_to(const struct csmoctree_element_t *octree_element, const char *octree_element_type);
#define csmoctree_octree_item_cast_to(octree_element, type) (const struct type *)csmoctree_dontuse_octree_item_cast_to(octree_element, #type)
