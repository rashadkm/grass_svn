
/****************************************************************************
 *
 * MODULE:       r.distance
 *
 * AUTHOR(S):    Michael Shapiro - CERL
 *
 * PURPOSE:      Locates the closest points between objects in two 
 *               raster maps.
 *
 * COPYRIGHT:    (C) 2003 by the GRASS Development Team
 *
 *               This program is free software under the GNU General Public
 *               License (>=v2). Read the file COPYING that comes with GRASS
 *               for details.
 *
 ***************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include "defs.h"
#include <grass/gis.h>
#include <grass/raster.h>
#include <grass/glocale.h>

int main(int argc, char *argv[])
{
    extern void parse();
    extern void find_edge_cells();
    extern void report();
    extern void read_labels();
    struct Parms parms;
    struct GModule *module;

    G_gisinit(argv[0]);

    /* Set description */
    module = G_define_module();
    module->keywords = _("raster");
    module->description =
	_("Locates the closest points between objects in two raster maps.");

    parse(argc, argv, &parms);
    if (parms.labels) {
	read_labels(&parms.map1);
	read_labels(&parms.map2);
    }

    find_edge_cells(&parms.map1);
    find_edge_cells(&parms.map2);

    report(&parms);

    return 0;
}
