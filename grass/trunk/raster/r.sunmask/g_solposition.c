/*
* $Id$
*
* G_calc_solar_position() calculates solar position parameters from
* given position, date and time
*
* Written by Markus Neteler <neteler@geog.uni-hannover.de>
* with kind help from Morten Hulden
*
*----------------------------------------------------------------------------
* using solpos.c with permission from
*   From rredc@nrel.gov Wed Mar 21 18:37:25 2001
*   Message-Id: <v04220805b6de9b1ad6ff@[192.174.39.30]>
*   Mary Anderberg
*   http://rredc.nrel.gov
*   National Renewable Energy Laboratory
*   1617 Cole Boulevard
*   Golden, Colorado, USA 80401
*
*   http://rredc.nrel.gov/solar/codes_algs/solpos/
*
*  G_calc_solar_position is based on: soltest.c
*    by
*    Martin Rymes
*    National Renewable Energy Laboratory
*    25 March 1998
*----------------------------------------------------------------------------*/

/* uncomment to get debug output */
/*#define DEBUG */

#include <math.h>
#include <string.h>
#include <stdio.h>
#include "gis.h"
#include "projects.h"
#include "solpos00.h"

struct posdata pd, *pdat; /* declare solpos data struct and a pointer for it */


long G_calc_solar_position (double longitude, double latitude, double timezone, 
                int year, int month, int day, int hour, int minute, int second)
{

    /* Note: this code is valid from year 1950 to 2050 (solpos restriction)

     - the algorithm will compensate for leap year.
     - longitude, latitude: decimal degree
     - timezone: DO NOT ADJUST FOR DAYLIGHT SAVINGS TIME.
     - timezone: negative for zones west of Greenwich
     - lat/long: east and north positive
     - the atmospheric refraction is calculated for 1013hPa, 15�C
     - time: local time from your watch
     */
				    
    long retval;             /* to capture S_solpos return codes */
    struct Key_Value *in_proj_info, *in_unit_info; /* projection information of input map */
    struct pj_info iproj;    /* input map proj parameters  */
    struct pj_info oproj;    /* output map proj parameters  */
    extern struct Cell_head window;
    int inside;
        
    pdat = &pd;   /* point to the structure for convenience */

    /* Initialize structure to default values. (Optional only if ALL input
       parameters are initialized in the calling code, which they are not
       in this example.) */

    S_init (pdat);

   /* check if given point is in current window */
#ifdef DEBUG
fprintf(stderr, "window.north: %f, window.south: %f\n", window.north, window.south);
fprintf(stderr, "window.west:  %f, window.east : %f\n", window.west, window.east);
#endif
    inside=0;
    if (latitude >= window.south && latitude  <= window.north &&
        longitude >= window.west && longitude <= window.east)
        	inside=1;
    if (! inside)
       G_warning("Specified point %f, %f outside of current window, is that intended? Anyway, it will be used.\n", longitude, latitude);

    /* if coordinates are not in lat/long format, transform them: */
    if ((G_projection() != PROJECTION_LL))
    {
#ifdef DEBUG
fprintf(stderr, "Transforming input coordinates to lat/long (req. for solar position)\n");
#endif

     /* read current projection info */
     if ((in_proj_info = G_get_projinfo()) == NULL)
        G_fatal_error("Can't get projection info of current location");

     if ((in_unit_info = G_get_projunits()) == NULL)
        G_fatal_error("Can't get projection units of current location");

     if (pj_get_kv(&iproj, in_proj_info, in_unit_info) < 0)
        G_fatal_error("Can't get projection key values of current location");

#ifdef DEBUG
fprintf(stderr, "Projection found in location:\n");
fprintf(stderr, "IN: meter: %f zone: %i proj: %s (iproj struct)\n", iproj.meters, iproj.zone, iproj.proj);
fprintf(stderr, "IN: ellps: a: %f  es: %f\n", iproj.pj->a, iproj.pj->es);
fprintf(stderr, "IN: x0: %f y0: %f\n", iproj.pj->x0, iproj.pj->y0);
fprintf(stderr, "IN: lam0: %f phi0: %f\n", iproj.pj->lam0, iproj.pj->phi0);
fprintf(stderr, "IN: k0: %f\n", iproj.pj->k0);
fprintf(stderr, "IN coord: longitude: %f, latitude: %f\n", longitude, latitude);
/* see src/include/projects.h, struct PJconsts*/
#endif

     /* set output projection to lat/long for solpos*/
     pj_zero_proj(&oproj);
     sprintf(oproj.proj, "%s", "ll");

     /* XX do the transform 
      *               outx        outy    in_info  out_info */
     if(pj_do_proj(&longitude, &latitude, &iproj, &oproj) < 0)
     {
       fprintf(stderr,"Error in pj_do_proj (projection of input coordinate pair)\n");
       exit(0);
     }

#ifdef DEBUG
  fprintf(stderr, "Transformation to lat/long:\n");
  fprintf(stderr, "OUT: longitude: %f, latitude: %f\n", longitude, latitude);
#endif
  

    } /* transform if not LL*/
    
    pdat->longitude = longitude;  /* Note that latitude and longitude are  */
    pdat->latitude  = latitude;   /*   in DECIMAL DEGREES, not Deg/Min/Sec */
    pdat->timezone  = timezone;   /* DO NOT ADJUST FOR DAYLIGHT SAVINGS TIME. */

    pdat->year      = year;       /* The year */
    pdat->function &= ~S_DOY;
    pdat->month     = month;
    pdat->day       = day;        /* the algorithm will compensate for leap year, so
                                     you just count days). S_solpos can be
                                     configured to accept day-of-the year */

    /* The time of day (STANDARD (GMT) time)*/

    pdat->hour      = hour;
    pdat->minute    = minute;
    pdat->second    = second;

    /* Let's assume that the temperature is 20 degrees C and that
       the pressure is 1013 millibars.  The temperature is used for the
       atmospheric refraction correction, and the pressure is used for the
       refraction correction and the pressure-corrected airmass. */

    pdat->temp      =   20.0;
    pdat->press     = 1013.0;

    /* Finally, we will assume that you have a flat surface
       facing nowhere, tilted at latitude. */

    pdat->tilt      = pdat->latitude;  /* tilted at latitude */
    pdat->aspect    = 180.0;

    /* perform the calculation */
    retval = S_solpos (pdat);  /* S_solpos function call: returns long value */
    S_decode(retval, pdat);    /* prints an error in case of problems */

    return retval;

}
