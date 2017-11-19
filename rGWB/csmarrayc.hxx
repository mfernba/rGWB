//
//  csmarrayc.hxx
//  rGWB
//
//  Created by Manuel Fernández on 6/10/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#ifndef csmarrayc_hxx
#define csmarrayc_hxx

#include "csmtypes.hxx"

struct csmarrayc_extra_item_t;

#define csmArrayStruct(tipo) struct csmArrayStruct_##tipo
typedef unsigned char csmarrayc_byte;

typedef CSMBOOL (*csmarrayc_FPtr_match_condition)(const void *element, const void *data);
#define CSMARRAYC_CHECK_FUNC_MATCH_CONDITION(function, type, comparison_type) (void)((CSMBOOL (*)(const struct type *, const comparison_type *))function == function)

typedef void (*csmarrayc_FPtr_free_struct)(void **element);
#define CSMARRAYC_CHECK_FUNC_FREE_STRUCT(function, type) (void)((void (*)(struct type **))function == function)

typedef void *(*csmarrayc_FPtr_copy_struct)(const void *element);
#define CSMARRAYC_CHECK_FUNC_COPY_STRUCT(function, type) (void)((struct type *(*)(const struct type *))function == function)

typedef enum csmcompare_t (*csmarrayc_FPtr_compare)(const void *element1, const void *element2);
#define CSMARRAYC_CHECK_FUNC_COMPARE_ST(function, type) (void)((enum csmcompare_t (*)(const struct type *, const struct type *))function == function)

typedef enum csmcompare_t (*csmarrayc_FPtr_compare_1_extra)(const void *element1, const void *element2, const struct csmarrayc_extra_item_t *extra_item);
#define CSMARRAYC_CHECK_FUNC_COMPARE_ST_1_EXTRA(function, type, extra_type) (void)((enum csmcompare_t (*)(const struct type *, const struct type *, const extra_type *))function == function)

#endif /* csmarrayc_hxx */
