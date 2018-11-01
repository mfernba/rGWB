// Write / read to file or other sources...

#include "csmfwddecl.hxx"

#ifndef csmsave_hxx
#define csmsave_hxx

typedef void (*csmsave_FPtr_free_item)(struct csmsave_item_t **csmsave_item);
#define CSMSAVE_CHECK_FUNC_ITEM(function, type) ((void (*)(struct type **))function == function)

typedef void (*csmsave_FPtr_write_bool)(const struct csmsave_item_t *csmsave_item, CSMBOOL value);
#define CSMSAVE_CHECK_FUNC_WRITE_BOOL(function, type) ((void (*)(const struct type *, CSMBOOL))function == function)

typedef void (*csmsave_FPtr_write_uchar)(const struct csmsave_item_t *csmsave_item, unsigned char value);
#define CSMSAVE_CHECK_FUNC_WRITE_UCHAR(function, type) ((void (*)(const struct type *, unsigned char))function == function)

typedef void (*csmsave_FPtr_write_ushort)(const struct csmsave_item_t *csmsave_item, unsigned short value);
#define CSMSAVE_CHECK_FUNC_WRITE_USHORT(function, type) ((void (*)(const struct type *, unsigned short))function == function)

typedef void (*csmsave_FPtr_write_ulong)(const struct csmsave_item_t *csmsave_item, unsigned long value);
#define CSMSAVE_CHECK_FUNC_WRITE_ULONG(function, type) ((void (*)(const struct type *, unsigned long))function == function)

typedef void (*csmsave_FPtr_write_double)(const struct csmsave_item_t *csmsave_item, double value);
#define CSMSAVE_CHECK_FUNC_WRITE_DOUBLE(function, type) ((void (*)(const struct type *, double))function == function)

typedef void (*csmsave_FPtr_write_float)(const struct csmsave_item_t *csmsave_item, float value);
#define CSMSAVE_CHECK_FUNC_WRITE_FLOAT(function, type) ((void (*)(const struct type *, float))function == function)

typedef void (*csmsave_FPtr_write_string)(const struct csmsave_item_t *csmsave_item, const char *value);
#define CSMSAVE_CHECK_FUNC_WRITE_STRING(function, type) ((void (*)(const struct type *, const char *))function == function)


typedef CSMBOOL (*csmsave_FPtr_read_bool)(const struct csmsave_item_t *csmsave_item);
#define CSMSAVE_CHECK_FUNC_READ_BOOL(function, type) ((CSMBOOL (*)(const struct type *))function == function)

typedef unsigned char (*csmsave_FPtr_read_uchar)(const struct csmsave_item_t *csmsave_item);
#define CSMSAVE_CHECK_FUNC_READ_UCHAR(function, type) ((unsigned char (*)(const struct type *))function == function)

typedef unsigned short (*csmsave_FPtr_read_ushort)(const struct csmsave_item_t *csmsave_item);
#define CSMSAVE_CHECK_FUNC_READ_USHORT(function, type) ((unsigned short (*)(const struct type *))function == function)

typedef unsigned long (*csmsave_FPtr_read_ulong)(const struct csmsave_item_t *csmsave_item);
#define CSMSAVE_CHECK_FUNC_READ_ULONG(function, type) ((unsigned long (*)(const struct type *))function == function)

typedef double (*csmsave_FPtr_read_double)(const struct csmsave_item_t *csmsave_item);
#define CSMSAVE_CHECK_FUNC_READ_DOUBLE(function, type) ((double (*)(const struct type *))function == function)

typedef float (*csmsave_FPtr_read_float)(const struct csmsave_item_t *csmsave_item);
#define CSMSAVE_CHECK_FUNC_READ_FLOAT(function, type) ((float (*)(const struct type *))function == function)

typedef char *(*csmsave_FPtr_read_string)(const struct csmsave_item_t *csmsave_item);
#define CSMSAVE_CHECK_FUNC_READ_STRING(function, type) ((char * (*)(const struct type *))function == function)


typedef void (*csmsave_FPtr_write_struct)(const struct csmsave_item_t *item, struct csmsave_t *csmsave);
#define CSMSAVE_CHECK_FUNC_WRITE_STRUCT(function, type) ((void (*)(const struct type *, struct csmsave_t *))function == function)

typedef struct csmsave_item_t *(*csmsave_FPtr_read_struct)(struct csmsave_t *csmsave);
#define CSMSAVE_CHECK_FUNC_READ_STRUCT(function, type) ((struct type *(*)(struct csmsave_t *))function == function)

#endif /* Header_h */
