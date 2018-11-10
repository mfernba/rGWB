// Octree...

#include "csmfwddecl.hxx"
#include "csmoctree.hxx"

DLL_RGWB CONSTRUCTOR(struct csmoctree_t *, csmoctree_new, (unsigned long max_occupancy));

DLL_RGWB void csmoctree_free(struct csmoctree_t **octree);

DLL_RGWB void csmoctree_dontuse_append_item(
                        struct csmoctree_t *octree,
                        const struct csmoctree_item_t *octree_item,
                        csmoctree_FPtr_get_extension func_get_extension,
                        csmoctree_FPtr_intersects_with_bbox func_intersects_with_bbox);
#define csmoctree_append_item(\
                        octree,\
                        octree_item, item_type,\
                        func_get_extension,\
                        func_intersects_with_bbox)\
    (/*lint -save -e505*/\
        ((const struct csmoctree_item_t *)octree_item == octree_item),\
        CSMOCTREE_CHECK_FUNC_GET_EXTENSION(func_get_extension, item_type),\
        CSMOCTREE_CHECK_FUNC_INTERSECTS_WITH_BBOX(func_intersects_with_bbox, item_type),\
        csmoctree_dontuse_append_item(\
                        octree,\
                        (const struct csmoctree_item_t *)octree_item,\
                        (csmoctree_FPtr_get_extension)func_get_extension,\
                        (csmoctree_FPtr_intersects_with_bbox)func_intersects_with_bbox)\
    )/*lint -restore*/


DLL_RGWB void csmoctree_build(struct csmoctree_t *octree);
