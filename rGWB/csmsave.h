// Write / read to file or other sources...

#include "csmsave.hxx"

#ifdef __cplusplus
extern "C" {
#endif

DLL_RGWB CONSTRUCTOR(struct csmsave_t *, csmsave_dontuse_new, (
                        struct csmsave_item_t **csmsave_item,
                        csmsave_FPtr_free_item func_free_item,
                        csmsave_FPtr_write_bool func_write_bool,
                        csmsave_FPtr_write_uchar func_write_uchar,
                        csmsave_FPtr_write_ushort func_write_ushort,
                        csmsave_FPtr_write_ulong func_write_ulong,
                        csmsave_FPtr_write_double func_write_double,
                        csmsave_FPtr_write_float func_write_float,
                        csmsave_FPtr_write_string func_write_string,
                        csmsave_FPtr_read_bool func_read_bool,
                        csmsave_FPtr_read_uchar func_read_uchar,
                        csmsave_FPtr_read_ushort func_read_ushort,
                        csmsave_FPtr_read_ulong func_read_ulong,
                        csmsave_FPtr_read_double func_read_double,
                        csmsave_FPtr_read_float func_read_float,
                        csmsave_FPtr_read_string func_read_string,
                        csmsave_FPtr_write_string func_write_st_mark,
                        csmsave_FPtr_read_string func_read_st_mark));
#define csmsave_new(\
                    csmsave_item, item_type,\
                    func_free_item,\
                    func_write_bool,\
                    func_write_uchar,\
                    func_write_ushort,\
                    func_write_ulong,\
                    func_write_double,\
                    func_write_float,\
                    func_write_string,\
                    func_read_bool,\
                    func_read_uchar,\
                    func_read_ushort,\
                    func_read_ulong,\
                    func_read_double,\
                    func_read_float,\
                    func_read_string,\
                    func_write_st_mark,\
                    func_read_st_mark)\
    (/*lint -save -e505*/\
        ((struct item_type **)csmsave_item == csmsave_item),\
        CSMSAVE_CHECK_FUNC_ITEM(func_free_item, item_type),\
        CSMSAVE_CHECK_FUNC_WRITE_BOOL(func_write_bool, item_type),\
        CSMSAVE_CHECK_FUNC_WRITE_UCHAR(func_write_uchar, item_type),\
        CSMSAVE_CHECK_FUNC_WRITE_USHORT(func_write_ushort, item_type),\
        CSMSAVE_CHECK_FUNC_WRITE_ULONG(func_write_ulong, item_type),\
        CSMSAVE_CHECK_FUNC_WRITE_DOUBLE(func_write_double, item_type),\
        CSMSAVE_CHECK_FUNC_WRITE_FLOAT(func_write_float, item_type),\
        CSMSAVE_CHECK_FUNC_WRITE_STRING(func_write_string, item_type),\
        CSMSAVE_CHECK_FUNC_READ_BOOL(func_read_bool, item_type),\
        CSMSAVE_CHECK_FUNC_READ_UCHAR(func_read_uchar, item_type),\
        CSMSAVE_CHECK_FUNC_READ_USHORT(func_read_ushort, item_type),\
        CSMSAVE_CHECK_FUNC_READ_ULONG(func_read_ulong, item_type),\
        CSMSAVE_CHECK_FUNC_READ_DOUBLE(func_read_double, item_type),\
        CSMSAVE_CHECK_FUNC_READ_FLOAT(func_read_float, item_type),\
        CSMSAVE_CHECK_FUNC_READ_STRING(func_read_string, item_type),\
        CSMSAVE_CHECK_FUNC_WRITE_STRING(func_write_st_mark, item_type),\
        CSMSAVE_CHECK_FUNC_READ_STRING(func_read_st_mark, item_type),\
        csmsave_dontuse_new(\
                        (struct csmsave_item_t **)csmsave_item,\
                        (csmsave_FPtr_free_item)func_free_item,\
                        (csmsave_FPtr_write_bool)func_write_bool,\
                        (csmsave_FPtr_write_uchar)func_write_uchar,\
                        (csmsave_FPtr_write_ushort)func_write_ushort,\
                        (csmsave_FPtr_write_ulong)func_write_ulong,\
                        (csmsave_FPtr_write_double)func_write_double,\
                        (csmsave_FPtr_write_float)func_write_float,\
                        (csmsave_FPtr_write_string)func_write_string,\
                        (csmsave_FPtr_read_bool)func_read_bool,\
                        (csmsave_FPtr_read_uchar)func_read_uchar,\
                        (csmsave_FPtr_read_ushort)func_read_ushort,\
                        (csmsave_FPtr_read_ulong)func_read_ulong,\
                        (csmsave_FPtr_read_double)func_read_double,\
                        (csmsave_FPtr_read_float)func_read_float,\
                        (csmsave_FPtr_read_string)func_read_string,\
                        (csmsave_FPtr_write_string)func_write_st_mark,\
                        (csmsave_FPtr_read_string)func_read_st_mark)\
    )/*lint -restore*/

DLL_RGWB void csmsave_free(struct csmsave_t **csmsave);

#ifdef __cplusplus
}
#endif



