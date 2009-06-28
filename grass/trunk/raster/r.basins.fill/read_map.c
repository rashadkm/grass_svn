
/****************************************************************************
 *
 * MODULE:       r.basins.fill
 *
 * AUTHOR(S):    Dale White - Dept. of Geography, Pennsylvania State U.
 *               Larry Band - Dept. of Geography, University of Toronto
 *
 * PURPOSE:      Generates a raster map layer showing watershed subbasins.
 *
 * COPYRIGHT:    (C) 2005 by the GRASS Development Team
 *
 *               This program is free software under the GNU General Public
 *               License (>=v2). Read the file COPYING that comes with GRASS
 *               for details.
 *
****************************************************************************/

#include <grass/gis.h>
#include <grass/raster.h>
#include <grass/glocale.h>
#include "local_proto.h"


CELL *read_map(const char *name, int nomask, int nrows, int ncols)
{
    int fd;
    CELL *map;
    int row;
    int (*get_row)(int, CELL *, int);

    /* allocate entire map */
    map = (CELL *) G_malloc(nrows * ncols * sizeof(CELL));

    /* open the map */
    if ((fd = Rast_open_cell_old(name, "")) < 0)
	G_fatal_error(_("Unable to open <%s>"), name);

    /* read the map */
    G_message(_("Reading <%s> ... "), name);

    if (nomask)
	get_row = Rast_get_map_row_nomask;
    else
	get_row = Rast_get_map_row;

    for (row = 0; row < nrows; row++) {
	G_percent(row, nrows, 10);
	if ((*get_row) (fd, map + row * ncols, row) < 0)
	    G_fatal_error(_("Error reading <%s>"), name);
    }
    G_percent(nrows, nrows, 10);

    Rast_close(fd);

    return map;
}
