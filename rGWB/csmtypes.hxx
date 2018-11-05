//
//  csmtypes.hxx
//  rGWB
//
//  Created by Manuel Fernández on 6/10/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#ifndef csmtypes_hxx
#define csmtypes_hxx

typedef unsigned short CSMBOOL;
#define CSMTRUE (CSMBOOL)1
#define CSMFALSE (CSMBOOL)0

enum csmcompare_t
{
    CSMCOMPARE_FIRST_LESS = -1,
    CSMCOMPARE_EQUAL = 0,
    CSMCOMPARE_FIRST_GREATER = 1
};

#ifdef RGWB_STANDALONE_DISTRIBUTABLE
#include "csmapto2d.hxx"
#include "csmapto3d.hxx"
#endif

#include "csmaulong.hxx"

#endif /* csmtypes_h */
