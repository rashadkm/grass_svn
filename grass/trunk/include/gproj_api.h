/******************************************************************************
 * $Id$
 *
 * Project:  PROJ.4
 * Purpose:  Public (application) include file for PROJ.4 API, and constants.
 * Author:   Frank Warmerdam, <warmerdam@pobox.com>
 *
 ******************************************************************************
 * Copyright (c) 2001, Frank Warmerdam <warmerdam@pobox.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ******************************************************************************
 *
 * $Log$
 * Revision 2.0  2004-11-09 13:05:07  bernhard
 * copied within CVS repository from grass/src/include/gproj_api.h
 *
 * Revision 1.2  2003/04/16 12:09:26  paul
 * New function to report actual projection parameters being used by proj
 *
 * Revision 1.1  2003/04/07 15:15:19  paul
 * External PROJ.4 (libproj) support
 *
 * Revision 1.1  2002/04/21 14:14:40  roger
 * Initial addition.  For update to proj4.4.5
 *
 * Revision 1.5  2002/01/09 14:36:22  warmerda
 * updated to 4.4.5
 *
 * Revision 1.4  2001/09/15 22:55:28  warmerda
 * final prep for 4.4.4 release
 *
 * Revision 1.3  2001/08/23 20:25:55  warmerda
 * added pj_set_finder function
 *
 * Revision 1.2  2001/06/02 03:35:36  warmerda
 * added pj_get_errno_ref()
 *
 * Revision 1.1  2001/04/06 01:24:22  warmerda
 * New
 *
 */

/* General projections header file */
#ifndef PROJ_API_H
#define PROJ_API_H

/* standard inclusions */
#include <math.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Try to update this every version! */
#define PJ_VERSION 445

extern char const pj_release[]; /* global release id string */

#define RAD_TO_DEG	57.29577951308232
#define DEG_TO_RAD	.0174532925199432958


extern int pj_errno;	/* global error return code */

#if !defined(PROJECTS_H)
    typedef struct { double u, v; } projUV;
    typedef void *projPJ;
    #define projXY projUV
    #define projLP projUV
#else
    typedef PJ *projPJ;
#   define projXY	XY
#   define projLP       LP
#endif

/* procedure prototypes */

projXY pj_fwd(projLP, projPJ);
projLP pj_inv(projXY, projPJ);

int pj_transform( projPJ src, projPJ dst, long point_count, int point_offset,
                  double *x, double *y, double *z );
int pj_datum_transform( projPJ src, projPJ dst, long point_count, int point_offset,
                        double *x, double *y, double *z );
int pj_geocentric_to_geodetic( double a, double ra,
                               long point_count, int point_offset,
                               double *x, double *y, double *z );
int pj_geodetic_to_geocentric( double a, double ra,
                               long point_count, int point_offset,
                               double *x, double *y, double *z );
int pj_compare_datums( projPJ srcdefn, projPJ dstdefn );
int pj_apply_gridshift( const char *, int, 
                        long point_count, int point_offset,
                        double *x, double *y, double *z );
void pj_deallocate_grids();
int pj_is_latlong(projPJ);
void pj_pr_list(projPJ);
void pj_free(projPJ);
void pj_set_finder( const char *(*)(const char *) );
projPJ pj_init(int, char **);
projPJ pj_init_plus(const char *);
char *pj_get_def(projPJ, int);
projPJ pj_latlong_from_proj( projPJ );
void *pj_malloc(size_t);
void pj_dalloc(void *);
char *pj_strerrno(int);
int *pj_get_errno_ref();

#ifdef __cplusplus
}
#endif

#endif /* ndef PROJ_API_H */

