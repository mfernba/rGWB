// Array de array 3D...

#include "csmfwddecl.hxx"
#include "csmArrPoint2D.hxx"

#ifdef __cplusplus
extern "C" {
#endif

DLL_RGWB CONSTRUCTOR(csmArrPoint2D *, csmArrPoint2D_new, (unsigned long no_elems));

DLL_RGWB CONSTRUCTOR(csmArrPoint2D *, csmArrPoint2D_copy, (const csmArrPoint2D *array));

DLL_RGWB void csmArrPoint2D_free(csmArrPoint2D **array);

DLL_RGWB unsigned long csmArrPoint2D_count(const csmArrPoint2D *array);

DLL_RGWB void csmArrPoint2D_append(csmArrPoint2D *array, double x, double y);

DLL_RGWB void csmArrPoint2D_delete(csmArrPoint2D *array, unsigned long idx);

DLL_RGWB void csmArrPoint2D_set(csmArrPoint2D *array, unsigned long idx, double x, double y);

DLL_RGWB void csmArrPoint2D_get(const csmArrPoint2D *array, unsigned long idx, double *x, double *y);

DLL_RGWB void csmArrPoint2D_invert(csmArrPoint2D *array);
    
DLL_RGWB double csmArrPoint2D_area(const csmArrPoint2D *array);

DLL_RGWB void csmArrPoint2D_move(csmArrPoint2D *array, double dx, double dy);

DLL_RGWB void csmArrPoint2D_append_circle_points(
							csmArrPoint2D *array,
							double x, double y, double radius,
							unsigned long no_points_circle,
							CSMBOOL clockwise_orientation);

DLL_RGWB void csmArrPoint2D_append_elipse_points(
                            csmArrPoint2D *array,
                            double x, double y, double radius_x, double radius_y,
                            unsigned long no_points_circle,
                            CSMBOOL clockwise_orientation);

DLL_RGWB CSMBOOL csmArrPoint2D_equal(const csmArrPoint2D *array1, const csmArrPoint2D *array2, double tolerance);
    
DLL_RGWB CSMBOOL csmArrPoint2D_exists_point(
                            const csmArrPoint2D *array,
                            double x, double y,
                            csmArrPoint2D_FPtr_equal_points func_equal_points,
                            unsigned long *idx_opt);

DLL_RGWB CSMBOOL csmArrPoint2D_exists_point_with_tolerance(
                            const csmArrPoint2D *array,
                            double x, double y,
                            double tolerance,
                            unsigned long *idx_opt);

DLL_RGWB void csmArrPoint2D_bounding_box(const csmArrPoint2D *array, double *x_min, double *y_min, double *x_max, double *y_max);

    
#ifdef __cplusplus
}
#endif

