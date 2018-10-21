// unsigned long array...

#include "csmfwddecl.hxx"
#include "csmaulong.hxx"

#ifdef __cplusplus
extern "C" {
#endif

DLL_RGWB CONSTRUCTOR(csmArrULong *, csmArrULong_new, (unsigned long no_elems));

DLL_RGWB CONSTRUCTOR(csmArrULong *, csmArrULong_copy, (const csmArrULong *array));

DLL_RGWB void csmArrULong_free(csmArrULong **array);

DLL_RGWB unsigned long csmArrULong_count(const csmArrULong *array);

DLL_RGWB void csmArrULong_append(csmArrULong *array, unsigned long value);

DLL_RGWB void csmArrULong_delete(csmArrULong *array, unsigned long idx);

DLL_RGWB void csmArrULong_set(csmArrULong *array, unsigned long idx, unsigned long value);

DLL_RGWB unsigned long csmArrULong_get(const csmArrULong *array, unsigned long idx);
    
#ifdef __cplusplus
}
#endif

