#include <stdlib.h>
#include <grass/gis.h>
#include "globals.h"
#include <grass/display.h>
#include "local_proto.h"

int draw_cell(View * view, int overlay)
{
    int fd;
    int left, top;
    int ncols, nrows;
    int row;
    DCELL *dcell;
    struct Colors colr;
    char msg[100];


    if (!view->cell.configured)
	return 0;
    if (Rast_read_colors(view->cell.name, view->cell.mapset, &colr) < 0)
	return 0;

    if (overlay == OVER_WRITE)
	display_title(view);

    Rast_set_window(&view->cell.head);
    nrows = G_window_rows();
    ncols = G_window_cols();

    left = view->cell.left;
    top = view->cell.top;

    R_standard_color(BLUE);
    Outline_box(top, top + nrows - 1, left, left + ncols - 1);

    if (getenv("NO_DRAW")) {
	Rast_free_colors(&colr);
	return 1;
    }

    fd = Rast_open_old(view->cell.name, view->cell.mapset);
    if (fd < 0) {
	Rast_free_colors(&colr);
	return 0;
    }
    dcell = Rast_allocate_d_buf();
    
    sprintf(msg, "Plotting %s ...", view->cell.name);
    Menu_msg(msg);

    D_set_overlay_mode(!overlay);
    D_cell_draw_setup(top, top + nrows, left, left + ncols);
    for (row = 0; row < nrows; row++) {
	if (Rast_get_d_row_nomask(fd, dcell, row) < 0)
	    break;
	D_draw_d_raster(row, dcell, &colr);
    }
    D_cell_draw_end();
    Rast_close(fd);
    G_free(dcell);
    Rast_free_colors(&colr);

    return row == nrows;
}
