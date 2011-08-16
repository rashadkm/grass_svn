/* plot1() - Level One vector reading */

#include <string.h>
#include <math.h>
#include <grass/gis.h>
#include <grass/raster.h>
#include <grass/vector.h>
#include <grass/display.h>
#include "plot.h"
#include "local_proto.h"
#include <grass/symbol.h>
#include <grass/glocale.h>
#include <grass/dbmi.h>

#define RENDER_POLYLINE 0
#define RENDER_POLYGON  1

int palette_ncolors = 16;

struct rgb_color palette[16] = {
    {198, 198, 198},		/*  1: light gray */
    {127, 127, 127},		/*  2: medium/dark gray */
    {255, 0, 0},		/*  3: bright red */
    {139, 0, 0},		/*  4: dark red */
    {0, 255, 0},		/*  5: bright green */
    {0, 139, 0},		/*  6: dark green */
    {0, 0, 255},		/*  7: bright blue */
    {0, 0, 139},		/*  8: dark blue   */
    {255, 255, 0},		/*  9: yellow */
    {139, 126, 10},		/* 10: olivey brown */
    {255, 165, 0},		/* 11: orange */
    {255, 192, 203},		/* 12: pink   */
    {255, 0, 255},		/* 13: magenta */
    {139, 0, 139},		/* 14: dark magenta */
    {0, 255, 255},		/* 15: cyan */
    {0, 139, 139}		/* 16: dark cyan */
};

int display_lines(struct Map_info *Map, int type, struct cat_list *Clist,
		  const struct color_rgb *color, const struct color_rgb *fcolor, int chcat,
		  const char *symbol_name, double size, int sqrt_flag,
		  int id_flag, int cats_color_flag, 
		  int default_width, double width_scale,
		  struct Colors* zcolors,
		  dbCatValArray *cvarr_rgb, struct Colors *colors,
		  dbCatValArray *cvarr_width, int nrec_width,
		  dbCatValArray *cvarr_size, int nrec_size,
		  dbCatValArray *cvarr_rot, int nrec_rot)

{
    int i, ltype, nlines, line, cat, found;
    double *x, *y;
    struct line_pnts *Points;
    struct line_cats *Cats;
    double x0, y0;

    int nerror_rgb;
    int n_points, n_lines, n_centroids, n_boundaries, n_faces;
    
    int custom_rgb;
    int red, grn, blu;
    RGBA_Color *line_color, *fill_color, *primary_color;
    int width;
    SYMBOL *Symb;
    double var_size, rotation;
    
    Symb = NULL;
    cat = -1;
    nlines = 0;
    
    if (id_flag && Vect_level(Map) < 2) {
	G_warning(_("Unable to display lines by id, topology not available. "
		    "Please try to rebuild topology using "
		    "v.build or v.build.all."));
    }
  
    var_size = size;
    rotation = 0.0;
    nerror_rgb = 0;

    line_color = G_malloc(sizeof(RGBA_Color));
    fill_color = G_malloc(sizeof(RGBA_Color));
    primary_color = G_malloc(sizeof(RGBA_Color));
    primary_color->a = RGBA_COLOR_OPAQUE;

    /* change function prototype to pass RGBA_Color instead of color_rgb? */
    if (color) {
	line_color->r = color->r;
	line_color->g = color->g;
	line_color->b = color->b;
	line_color->a = RGBA_COLOR_OPAQUE;
    }
    else
	line_color->a = RGBA_COLOR_NONE;

    if (fcolor) {
	fill_color->r = fcolor->r;
	fill_color->g = fcolor->g;
	fill_color->b = fcolor->b;
	fill_color->a = RGBA_COLOR_OPAQUE;
    }
    else
	fill_color->a = RGBA_COLOR_NONE;

    Points = Vect_new_line_struct();
    Cats = Vect_new_cats_struct();

    /* dynamic symbols for points */
    if (!(nrec_size || nrec_rot)) {
	Symb = S_read(symbol_name);
	if (!Symb)
	    G_warning(_("Unable to read symbol <%s>, unable to display points"),
		      symbol_name);
	else
	    S_stroke(Symb, size, 0.0, 0);
    }
    
    Vect_rewind(Map);

    if (color && !cvarr_rgb && !cats_color_flag)
	D_RGB_color(color->r, color->g, color->b);

    if (Vect_level(Map) >= 2)
	nlines = Vect_get_num_lines(Map);

    line = 0;
    n_points = n_lines = 0;
    n_centroids = n_boundaries = 0;
    n_faces = 0;
    while (TRUE) {
	line++;
	custom_rgb = FALSE;
	
	if (Vect_level(Map) >= 2) {
	    if (line > nlines)
		break;
	    if (!Vect_line_alive(Map, line))
		continue;
	    ltype = Vect_read_line(Map, Points, Cats, line);
	}
	else {
	    ltype = Vect_read_next_line(Map, Points, Cats);
	    if (ltype == -1) {
		G_fatal_error(_("Unable to read vector map"));
	    }
	    else if (ltype == -2) { /* EOF */
		break;
	    }
	}

	if (!(type & ltype))
	    continue;

	if (Points->n_points == 0)
	    continue;
	
	found = FALSE;
	if (chcat) {
	    if (id_flag) {	/* use line id */
		if (!(Vect_cat_in_cat_list(line, Clist)))
		    continue;
	    }
	    else {
		for (i = 0; i < Cats->n_cats; i++) {
		    if (Cats->field[i] == Clist->field &&
			Vect_cat_in_cat_list(Cats->cat[i], Clist)) {
			found = TRUE;
			break;
		    }
		}
		if (!found)
		    continue;
	    }
	}
	else if (Clist->field > 0) {
	    for (i = 0; i < Cats->n_cats; i++) {
		if (Cats->field[i] == Clist->field) {
		    found = TRUE;
		    break;
		}
	    }
	    /* lines with no category will be displayed */
	    if (Cats->n_cats > 0 && !found)
		continue;
	}

	G_debug(3, "\tdisplay feature %d, cat %d", line, cat);
	
	/* z height colors */
	if (zcolors) {
	    if (Rast_get_d_color(&Points->z[0], &red, &grn, &blu, zcolors) == 1)
		custom_rgb = TRUE;
	    else
		custom_rgb = FALSE;
	}

	if (colors || cvarr_rgb ||
	    nrec_width > 0 || nrec_size > 0 || nrec_rot > 0)
	    /* only first category */
	    Vect_cat_get(Cats, 
			 (Clist->field > 0 ? Clist->field : (Cats->n_cats > 0 ? Cats->field[0] : 1)),
			 &cat);
	
	/* custom colors */
	if (colors || cvarr_rgb) {
	    custom_rgb = get_table_color(cat, line, colors, cvarr_rgb,
					 &red, &grn, &blu, &nerror_rgb);
	}
	
	/* random colors */
	if (cats_color_flag) {
	    custom_rgb = get_cat_color(line, Cats, Clist,
				       &red, &grn, &blu);
	}

	/* line width */
	if (nrec_width) {
	    width = (int) get_property(cat, line, cvarr_width,
				       (double) width_scale,
				       (double) default_width);
	    
	    D_line_width(width);
	}

	/* enough of the prep work, lets start plotting stuff */
	x = Points->x;
	y = Points->y;

	if ((ltype & GV_POINTS) && (Symb != NULL || (nrec_size || nrec_rot)) ) {
	    if (!(color || fcolor || custom_rgb))
		continue;

	    x0 = x[0];
	    y0 = y[0];

	    /* skip if the point is outside of the display window */
	    /* xy < 0 tests make it go ever-so-slightly faster */
	    if (x0 > D_get_u_east() || x0 < D_get_u_west() ||
		y0 < D_get_u_south() || y0 > D_get_u_north())
		continue;

	    /* dynamic symbol size */
	    if (nrec_size)
		var_size = get_property(cat, line, cvarr_size, size, size);
	    
            if (sqrt_flag)
                var_size = sqrt(var_size);

	    /* dynamic symbol rotation */
	    if (nrec_rot)
		rotation = get_property(cat, line, cvarr_rot, 1.0, 0.0);
	    
	    if(nrec_size || nrec_rot) {
		G_debug(3, "\tdynamic symbol: cat=%d  size=%.2f  rotation=%.2f",
			cat, var_size, rotation);

		/* symbol stroking is cumulative, so we need to reread it each time */
		if(Symb) /* unclean free() on first iteration if variables are not init'd to NULL? */
		    G_free(Symb);
		Symb = S_read(symbol_name);
		if (Symb == NULL)
		    G_warning(_("Unable to read symbol <%s>, unable to display points"),
			      symbol_name);
		else
		    S_stroke(Symb, var_size, rotation, 0);
	    }
	    
	    /* use random or RGB column color if given, otherwise reset */
	    /* centroids always use default color to stand out from underlying area */
	    if (custom_rgb && (ltype != GV_CENTROID)) {
		primary_color->r = (unsigned char)red;
		primary_color->g = (unsigned char)grn;
		primary_color->b = (unsigned char)blu;
		D_symbol2(Symb, x0, y0, primary_color, line_color);
	    }
	    else
		D_symbol(Symb, x0, y0, line_color, fill_color);

	    /* reset to defaults */
	    var_size = size;
	    rotation = 0.0;
	}
	else if (color || custom_rgb || zcolors) {
	    if (!cvarr_rgb && !cats_color_flag && !zcolors && !colors)
		D_RGB_color(color->r, color->g, color->b);
	    else {
		if (custom_rgb)
		    D_RGB_color((unsigned char)red, (unsigned char)grn,
				(unsigned char)blu);
		else
		    D_RGB_color(color->r, color->g, color->b);
	    }

	    /* Plot the lines */
	    if (Points->n_points == 1)	/* line with one coor */
		D_polydots_abs(x, y, Points->n_points);
	    else		/* use different user defined render methods */
		D_polyline_abs(x, y, Points->n_points);
	}
	
	switch (ltype) {
	case GV_POINT:
	    n_points++;
	    break;
	case GV_LINE:
	    n_lines++;
	    break;
	case GV_CENTROID:
	    n_centroids++;
	    break;
	case GV_BOUNDARY:
	    n_boundaries++;
	    break;
	case GV_FACE:
	    n_faces++;
	    break;
	default:
	    break;
	}
    }
    
    /*
    if (nerror_rgb > 0) {
	G_warning(_("Error in color definition column '%s': %d features affected"),
		  rgb_column, nerror_rgb);
    }
    */
    
    if (n_points > 0) 
	G_verbose_message(_("%d points plotted"), n_points);
    if (n_lines > 0) 
	G_verbose_message(_("%d lines plotted"), n_lines);
    if (n_centroids > 0) 
	G_verbose_message(_("%d centroids plotted"), n_centroids);
    if (n_boundaries > 0) 
	G_verbose_message(_("%d boundaries plotted"), n_boundaries);
    if (n_faces > 0) 
	G_verbose_message(_("%d faces plotted"), n_faces);
    
    Vect_destroy_line_struct(Points);
    Vect_destroy_cats_struct(Cats);
    G_free(line_color);
    G_free(fill_color);
    G_free(primary_color);

    return 0;
}
