/* Copyright (C) 1999 Aladdin Enterprises.  All rights reserved.
  
  This software is provided AS-IS with no warranty, either express or
  implied.
  
  This software is distributed under license and may not be copied,
  modified or distributed except as expressly authorized under the terms
  of the license contained in the file LICENSE in this distribution.
  
  For more information about licensing, please refer to
  http://www.ghostscript.com/licensing/. For information on
  commercial licensing, go to http://www.artifex.com/licensing/ or
  contact Artifex Software, Inc., 101 Lucas Valley Road #110,
  San Rafael, CA  94903, U.S.A., +1(415)492-9861.
*/

/* $Id: gxdhtres.h,v 1.5 2002/06/16 08:45:43 lpd Exp $ */
/* Definitions for precompiled halftone resources */

#ifndef gxdhtres_INCLUDED
#  define gxdhtres_INCLUDED

#include "stdpre.h"

/*
 * Precompiled halftones generated by genht #include this file.
 */
#ifndef gx_device_halftone_resource_DEFINED
#  define gx_device_halftone_resource_DEFINED
typedef struct gx_device_halftone_resource_s gx_device_halftone_resource_t;
#endif

struct gx_device_halftone_resource_s {
    const char *rname;
    int HalftoneType;
    int Width;
    int Height;
    int num_levels;
    const unsigned int *levels;
    const void *bit_data;
    int elt_size;
};

#define DEVICE_HALFTONE_RESOURCE_PROC(proc)\
  const gx_device_halftone_resource_t *const *proc(void)

#endif /* gxdhtres_INCLUDED */
