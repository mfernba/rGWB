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

#undef assert
#include "csmfwddecl.hxx"

void csmassert_dontuse_assertion_failed(CSMBOOL condition, const char *file, int line, const char *assertion);
#define assert(condition) csmassert_dontuse_assertion_failed(condition, __FILE__, __LINE__, #condition)

void csmassert_dontuse_assertion_not_null_failed(const void *ptr, const char *file, int line, const char *assertion);
#define assert_no_null(condition) csmassert_dontuse_assertion_not_null_failed(condition, __FILE__, __LINE__, #condition)

void csmassert_dontuse_default_error(const char *file, int line) __attribute__ ((noreturn));
#define default_error() csmassert_dontuse_default_error(__FILE__, __LINE__)
