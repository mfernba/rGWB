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

typedef CSMBOOL (*csmarrayc_FPtr_match_condition)(const unsigned char *element, const unsigned char *data);
#define CSMARRAYC_CHECK_FUNC_MATCH_CONDITION(function, type, comparison_type) (void)((CSMBOOL (*)(const struct type *, const comparison_type *))function == function)

typedef void (*csmarrayc_FPtr_free_struct)(unsigned char **element);
#define CSMARRAYC_CHECK_FUNC_FREE_STRUCT(function, type) (void)((void (*)(struct type **))function == function)

typedef void *(*csmarrayc_FPtr_copy_struct)(const unsigned char *element);
#define CSMARRAYC_CHECK_FUNC_COPY_STRUCT(function, type) (void)((struct type *(*)(const struct type *))function == function)

typedef enum csmcompare_t (*csmarrayc_FPtr_compare)(const unsigned char *element1, const unsigned char *element2);
#define CSMARRAYC_CHECK_FUNC_COMPARE_ST(function, type) (void)((enum csmcompare_t (*)(const struct type *, const struct type *))function == function)

typedef enum csmcompare_t (*csmarrayc_FPtr_compare_1_extra)(const unsigned char *element1, const unsigned char *element2, const struct csmarrayc_extra_item_t *extra_item);
#define CSMARRAYC_CHECK_FUNC_COMPARE_ST_1_EXTRA(function, type, extra_type) (void)((enum csmcompare_t (*)(const struct type *, const struct type *, const extra_type *))function == function)

#endif /* csmarrayc_hxx */
