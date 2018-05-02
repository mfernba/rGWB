//
//  sm4dblist.h
//  solidmodeling4
//
//  Created by Manuel Fernandez on 1/2/15.
//  Copyright (c) 2015 manueru. All rights reserved.
//
//  An implementation of Martti Mäntylä Geometric Workbench
//
//  Assertion management module.
//

#ifdef __STANDALONE_DISTRIBUTABLE

#undef assert
#include "csmfwddecl.hxx"

void csmassert_dontuse_assertion_failed(const char *file, int line, const char *assertion);
#define assert(condition) ( (condition)? (void)0 : csmassert_dontuse_assertion_failed(__FILE__, __LINE__, #condition))
#define assert_no_null(condition) assert(condition != NULL)

void csmassert_dontuse_default_error(const char *file, int line) __attribute__ ((noreturn));
#define default_error() csmassert_dontuse_default_error(__FILE__, __LINE__)

#endif
