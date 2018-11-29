// Octree...

#include "csmfwddecl.hxx"
#include "csmoctree.hxx"

#ifdef __cplusplus
extern "C" {
#endif

DLL_RGWB CONSTRUCTOR(struct csmoctree_t *, csmoctree_dontuse_new, (
                        unsigned long max_occupancy,
                        double tolerance_margin,
                        struct csmbbox_t **octree_bbox,
                        csmoctree_FPtr_intersects_with_bbox func_intersects_with_bbox));
#define csmoctree_new(max_occupancy, tolerance_margin, octree_bbox, func_intersects_with_bbox, type)\
    (/*lint -save -e505*/\
        CSMOCTREE_CHECK_FUNC_INTERSECTS_WITH_BBOX(func_intersects_with_bbox, type),\
        (struct csmoctree(type) *)csmoctree_dontuse_new(max_occupancy, tolerance_margin, octree_bbox, (csmoctree_FPtr_intersects_with_bbox)func_intersects_with_bbox)\
    )/*lint -restore*/

DLL_RGWB void csmoctree_dontuse_free(struct csmoctree_t **octree);
#define csmoctree_free(octree, type)\
    (/*lint -save -e505*/\
        ((struct csmoctree(type) **)octree == octree),\
        csmoctree_dontuse_free((struct csmoctree_t **)octree)\
    )/*lint -restore*/

DLL_RGWB void csmoctree_dontuse_append_item(struct csmoctree_t *octree, const struct csmoctree_item_t *octree_item);
#define csmoctree_append_item(octree, type, octree_item)\
    (/*lint -save -e505*/\
        ((struct csmoctree(type) *)octree == octree),\
        ((const struct type *)octree_item == octree_item),\
        csmoctree_dontuse_append_item((struct csmoctree_t *)octree, (const struct csmoctree_item_t *)octree_item)\
    )/*lint -restore*/


DLL_RGWB CONSTRUCTOR(const csmArrayStruct(csmoctree_item_t) *, csmoctree_dontuse_get_bbox_neighbors, (struct csmoctree_t *octree, const struct csmbbox_t *bbox));
#define csmoctree_get_bbox_neighbors(octree, type, bbox)\
    (/*lint -save -e505*/\
        ((struct csmoctree(type) *)octree == octree),\
        (const csmArrayStruct(type) *)csmoctree_dontuse_get_bbox_neighbors((struct csmoctree_t *)octree, bbox)\
    )/*lint -restore*/


DLL_RGWB void csmoctree_dontuse_print(struct csmoctree_t *octree);
#define csmoctree_print(octree, type)\
    (/*lint -save -e505*/\
        ((struct csmoctree(type) *)octree == octree),\
        csmoctree_dontuse_print((struct csmoctree_t *)octree)\
    )/*lint -restore*/
    
#ifdef __cplusplus
}
#endif
