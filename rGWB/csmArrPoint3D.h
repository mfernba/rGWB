// Array de array 3D...

#include "csmfwddecl.hxx"
#include "csmArrPoint3D.hxx"

#ifdef __cplusplus
extern "C" {
#endif

DLL_RGWB CONSTRUCTOR(csmArrPoint3D *, csmArrPoint3D_new, (unsigned long no_elems));

DLL_RGWB CONSTRUCTOR(csmArrPoint3D *, csmArrPoint3D_copy, (const csmArrPoint3D *array));

DLL_RGWB void csmArrPoint3D_free(csmArrPoint3D **array);

DLL_RGWB unsigned long csmArrPoint3D_count(const csmArrPoint3D *array);

DLL_RGWB void csmArrPoint3D_append(csmArrPoint3D *array, double x, double y, double z);

DLL_RGWB void csmArrPoint3D_delete(csmArrPoint3D *array, unsigned long idx);

DLL_RGWB void csmArrPoint3D_set(csmArrPoint3D *array, unsigned long idx, double x, double y, double z);

DLL_RGWB void csmArrPoint3D_get(const csmArrPoint3D *array, unsigned long idx, double *x, double *y, double *z);

DLL_RGWB void csmArrPoint3D_invert(csmArrPoint3D *array);
    
#ifdef __cplusplus
}
#endif
