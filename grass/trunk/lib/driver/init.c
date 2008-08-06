
/****************************************************************************
 *
 * MODULE:       driver
 * AUTHOR(S):    Glynn Clements <glynn gclements.plus.com> (original contributor)
 *               Huidae Cho <grass4u gmail.com>
 * PURPOSE:      
 * COPYRIGHT:    (C) 2006-2006 by the GRASS Development Team
 *
 *               This program is free software under the GNU General Public
 *               License (>=v2). Read the file COPYING that comes with GRASS
 *               for details.
 *
 *****************************************************************************/
#include <grass/config.h>

#include <stdio.h>
#include <stdlib.h>

#include <grass/gis.h>
#include <grass/freetypecap.h>
#include "driverlib.h"
#include "driver.h"

const struct driver *driver;

struct GFONT_CAP *ftcap;

int NCOLORS;

int screen_left;
int screen_right;
int screen_bottom;
int screen_top;

int cur_x;
int cur_y;

double text_size_x;
double text_size_y;
double text_rotation;

int LIB_init(const struct driver *drv, int argc, char **argv)
{
    const char *p;

    driver = drv;
    ftcap = parse_freetypecap();

    /* initialize graphics */

    p = getenv("GRASS_WIDTH");
    screen_left = 0;
    screen_right = (p && atoi(p)) ? atoi(p) : DEF_WIDTH;

    p = getenv("GRASS_HEIGHT");
    screen_top = 0;
    screen_bottom = (p && atoi(p)) ? atoi(p) : DEF_HEIGHT;

    if (COM_Graph_set(argc, argv) < 0)
	exit(1);

    return 0;
}
