// Write / read to file or other sources...

#include "csmfwddecl.hxx"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*csmsave_FPtr_write_struct)(const struct csmsave_item_t *item, struct csmsave_t *csmsave);
#define CSMSAVE_CHECK_FUNC_WRITE_STRUCT(function, type)  ((void (*)(const struct type *, struct csmsave_t *))function == function)

typedef struct csmsave_item_t *(*csmsave_FPtr_read_struct)(struct csmsave_t *csmsave);
#define CSMSAVE_CHECK_FUNC_READ_STRUCT(function, type)  ((struct type *(*)(struct csmsave_t *))function == function)


DLL_RGWB CONSTRUCTOR(struct csmsave_t *, csmsave_new_file_writer, (const char *file_path));

DLL_RGWB CONSTRUCTOR(struct csmsave_t *, csmsave_new_file_reader, (const char *file_path));

DLL_RGWB void csmsave_free(struct csmsave_t **csmsave);


// Write...

DLL_RGWB void csmsave_write_bool(struct csmsave_t *csmsave, CSMBOOL value);

DLL_RGWB void csmsave_write_uchar(struct csmsave_t *csmsave, unsigned char value);

DLL_RGWB void csmsave_write_ushort(struct csmsave_t *csmsave, unsigned short value);
#define csmsave_write_enum(csmsave, value) csmsave_write_ushort(csmsave, (unsigned short)value)

DLL_RGWB void csmsave_write_ulong(struct csmsave_t *csmsave, unsigned long value);

DLL_RGWB void csmsave_write_double(struct csmsave_t *csmsave, double value);

DLL_RGWB void csmsave_write_float(struct csmsave_t *csmsave, float value);

DLL_RGWB void csmsave_write_string(struct csmsave_t *csmsave, const char *value);

DLL_RGWB void csmsave_write_string_optional(struct csmsave_t *csmsave, const char *value);

DLL_RGWB void csmsave_write_arr_ulong(struct csmsave_t *csmsave, const csmArrULong *array);

DLL_RGWB void csmsave_dontuse_write_arr_st(
                        struct csmsave_t *csmsave,
                        const csmArrayStruct(csmsave_item_t) *array, const char *type_name,
                        csmsave_FPtr_write_struct func_write_struct);
#define csmsave_write_arr_st(csmsave, array, func_write_struct, type)\
(/*lint -save -e505*/\
    ((const csmArrayStruct(type) *)array == array),\
    CSMSAVE_CHECK_FUNC_WRITE_STRUCT(func_write_struct, type),\
    csmsave_dontuse_write_arr_st(csmsave, (const csmArrayStruct(csmsave_item_t) *)array, #type, (csmsave_FPtr_write_struct)func_write_struct)\
)/*lint -restore*/

DLL_RGWB void csmsave_dontuse_write_st(
                        struct csmsave_t *csmsave,
                        const struct csmsave_item_t *item, const char *type_name,
                        csmsave_FPtr_write_struct func_write_struct);
#define csmsave_write_st(csmsave, item, func_write_struct, type)\
(/*lint -save -e505*/\
    ((const struct type *)item == item),\
    CSMSAVE_CHECK_FUNC_WRITE_STRUCT(func_write_struct, type),\
    csmsave_dontuse_write_st(csmsave, (const struct csmsave_item_t *)item, #type, (csmsave_FPtr_write_struct)func_write_struct)\
)/*lint -restore*/

DLL_RGWB void csmsave_dontuse_write_optional_st(
                        struct csmsave_t *csmsave,
                        const struct csmsave_item_t *item, const char *type_name,
                        csmsave_FPtr_write_struct func_write_struct);
#define csmsave_write_optional_st(csmsave, item, func_write_struct, type)\
(/*lint -save -e505*/\
    ((const struct type *)item == item),\
    CSMSAVE_CHECK_FUNC_WRITE_STRUCT(func_write_struct, type),\
    csmsave_dontuse_write_optional_st(csmsave, (const struct csmsave_item_t *)item, #type, (csmsave_FPtr_write_struct)func_write_struct)\
)/*lint -restore*/



// Read...

DLL_RGWB CSMBOOL csmsave_read_bool(struct csmsave_t *csmsave);

DLL_RGWB unsigned char csmsave_read_uchar(struct csmsave_t *csmsave);

DLL_RGWB unsigned short csmsave_read_ushort(struct csmsave_t *csmsave);
#define csmsave_read_enum(csmsave, enum_type) (enum enum_type)csmsave_read_ushort(csmsave)

DLL_RGWB unsigned long csmsave_read_ulong(struct csmsave_t *csmsave);

DLL_RGWB double csmsave_read_double(struct csmsave_t *csmsave);

DLL_RGWB float csmsave_read_float(struct csmsave_t *csmsave);

DLL_RGWB CONSTRUCTOR(char *, csmsave_read_string, (struct csmsave_t *csmsave));

DLL_RGWB CONSTRUCTOR(char *, csmsave_read_string_optional, (struct csmsave_t *csmsave));

DLL_RGWB CONSTRUCTOR(csmArrULong *, csmsave_read_arr_ulong, (struct csmsave_t *csmsave));

DLL_RGWB CONSTRUCTOR(csmArrayStruct(csmsave_item_t) *, csmsave_dontuse_read_arr_st, (struct csmsave_t *csmsave, const char *type_name, csmsave_FPtr_read_struct func_read_struct));
#define csmsave_read_arr_st(csmsave, func_read_struct, type)\
(/*lint -save -e505*/\
    CSMSAVE_CHECK_FUNC_READ_STRUCT(func_read_struct, type),\
    (csmArrayStruct(type) *)csmsave_dontuse_read_arr_st(csmsave, #type, (csmsave_FPtr_read_struct)func_read_struct)\
)/*lint -restore*/

DLL_RGWB CONSTRUCTOR(struct csmsave_item_t *, csmsave_dontuse_read_st, (struct csmsave_t *csmsave, const char *type_name, csmsave_FPtr_read_struct func_read_struct));
#define csmsave_read_st(csmsave, func_read_struct, type)\
(/*lint -save -e505*/\
    CSMSAVE_CHECK_FUNC_READ_STRUCT(func_read_struct, type),\
    (struct type *)csmsave_dontuse_read_st(csmsave, #type, (csmsave_FPtr_read_struct)func_read_struct)\
)/*lint -restore*/

DLL_RGWB CONSTRUCTOR(struct csmsave_item_t *, csmsave_dontuse_read_optional_st, (struct csmsave_t *csmsave, const char *type_name, csmsave_FPtr_read_struct func_read_struct));
#define csmsave_read_optional_st(csmsave, func_read_struct, type)\
(/*lint -save -e505*/\
    CSMSAVE_CHECK_FUNC_READ_STRUCT(func_read_struct, type),\
    (struct type *)csmsave_dontuse_read_optional_st(csmsave, #type, (csmsave_FPtr_read_struct)func_read_struct)\
)/*lint -restore*/

#ifdef __cplusplus
}
#endif



