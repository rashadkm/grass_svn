
/*-
 * Written by H. Mitasova, I. Kosinovsky, D. Gerdes Summer 1992
 * Copyright 1992, H. Mitasova 
 * I. Kosinovsky,  and D.Gerdes    
 *
 * modified by McCauley in August 1995
 * modified by Mitasova in August 1995  
 * modified by Mitasova in August 1999 (fix for elev color)
 * modified by Brown in September 1999 (fix for Timestamps)
 * Modified by Mitasova in Nov. 1999 (write given tension into hist)
 * Last modification: 2006-12-13
 *
 */

#include <stdio.h>
#include <math.h>

#include <grass/gis.h>
#include <grass/raster.h>
#include <grass/bitmap.h>
#include <grass/linkm.h>
#include <grass/interpf.h>
#include <grass/glocale.h>

#define MULT 100000


int IL_output_2d(struct interp_params *params, struct Cell_head *cellhd,	/* current region */
		 double zmin, double zmax,	/* min,max input z-values */
		 double zminac, double zmaxac, double c1min, double c1max,	/* min,max interpolated values */
		 double c2min, double c2max, double gmin, double gmax, double ertot,	/* total interplating func. error */
		 char *input,	/* input file name */
		 double dnorm, int dtens, int vect, int n_points)

/*
 * Creates output files as well as history files  and color tables for
 * them.
 */
{
    FCELL *cell1;
    int cf1 = 0, cf2 = 0, cf3 = 0, cf4 = 0, cf5 = 0, cf6 = 0;
    int nrows, ncols;
    int i, ii;
    double zstep;
    FCELL data1, data2;
    struct Colors colors;
    struct History hist, hist1, hist2, hist3, hist4, hist5;
    char *type;
    const char *mapset = NULL;
    int cond1, cond2;
    FCELL dat1, dat2;
    CELL val1, val2;
    
    cond2 = ((params->pcurv != NULL) || (params->tcurv != NULL)
	     || (params->mcurv != NULL));
    cond1 = ((params->slope != NULL) || (params->aspect != NULL) || cond2);

    cell1 = Rast_allocate_f_buf();

    /*
     * G_set_embedded_null_value_mode(1);
     */
    if (params->elev)
	cf1 = Rast_open_fp_new(params->elev);

    if (params->slope)
	cf2 = Rast_open_fp_new(params->slope);

    if (params->aspect)
	cf3 = Rast_open_fp_new(params->aspect);

    if (params->pcurv)
	cf4 = Rast_open_fp_new(params->pcurv);

    if (params->tcurv)
	cf5 = Rast_open_fp_new(params->tcurv);

    if (params->mcurv)
	cf6 = Rast_open_fp_new(params->mcurv);

    nrows = cellhd->rows;
    if (nrows != params->nsizr) {
	G_warning(_("First change your rows number to nsizr! %d %d"),
		  nrows, params->nsizr);
	return -1;
    }

    ncols = cellhd->cols;
    if (ncols != params->nsizc) {
	G_warning(_("First change your cols number to nsizc %d %d"),
		  ncols, params->nsizc);
	return -1;
    }

    if (Rast_set_window(cellhd) < 0)
	return -1;

    if (nrows != G_window_rows()) {
	G_warning(_("Rows changed from %d to %d"), nrows,
		  G_window_rows());
	return -1;
    }

    if (ncols != G_window_cols()) {
	G_warning(_("Cols changed from %d to %d"), ncols,
		  G_window_cols());
	return -1;
    }

    if (params->elev != NULL) {
	G_fseek(params->Tmp_fd_z, 0L, 0);	/* seek to the beginning */
	for (i = 0; i < params->nsizr; i++) {
	    /* seek to the right row */
	    G_fseek(params->Tmp_fd_z, (off_t) (params->nsizr - 1 - i) *
		    params->nsizc * sizeof(FCELL), 0);
	    ii = fread(cell1, sizeof(FCELL), params->nsizc, params->Tmp_fd_z);
	    /*
	     * for(j=0;j<params->nsizc;j++) fprintf(stderr,"%f ",cell1[j]);
	     * fprintf(stderr,"\n");
	     */
	    Rast_put_f_row(cf1, cell1);

	}
    }

    if (params->slope != NULL) {
	G_fseek(params->Tmp_fd_dx, 0L, 0);	/* seek to the beginning */
	for (i = 0; i < params->nsizr; i++) {
	    /* seek to the right row */
	    G_fseek(params->Tmp_fd_dx, (off_t) (params->nsizr - 1 - i) *
		    params->nsizc * sizeof(FCELL), 0);
	    fread(cell1, sizeof(FCELL), params->nsizc, params->Tmp_fd_dx);
	    Rast_put_f_row(cf2, cell1);
	}
    }

    if (params->aspect != NULL) {
	G_fseek(params->Tmp_fd_dy, 0L, 0);	/* seek to the beginning */
	for (i = 0; i < params->nsizr; i++) {
	    /* seek to the right row */
	    G_fseek(params->Tmp_fd_dy, (off_t) (params->nsizr - 1 - i) *
		    params->nsizc * sizeof(FCELL), 0);
	    fread(cell1, sizeof(FCELL), params->nsizc, params->Tmp_fd_dy);
	    Rast_put_f_row(cf3, cell1);
	}
    }

    if (params->pcurv != NULL) {
	G_fseek(params->Tmp_fd_xx, 0L, 0);	/* seek to the beginning */
	for (i = 0; i < params->nsizr; i++) {
	    /* seek to the right row */
	    G_fseek(params->Tmp_fd_xx, (off_t) (params->nsizr - 1 - i) *
		    params->nsizc * sizeof(FCELL), 0);
	    fread(cell1, sizeof(FCELL), params->nsizc, params->Tmp_fd_xx);
	    Rast_put_f_row(cf4, cell1);
	}
    }

    if (params->tcurv != NULL) {
	G_fseek(params->Tmp_fd_yy, 0L, 0);	/* seek to the beginning */
	for (i = 0; i < params->nsizr; i++) {
	    /* seek to the right row */
	    G_fseek(params->Tmp_fd_yy, (off_t) (params->nsizr - 1 - i) *
		    params->nsizc * sizeof(FCELL), 0);
	    fread(cell1, sizeof(FCELL), params->nsizc, params->Tmp_fd_yy);
	    Rast_put_f_row(cf5, cell1);
	}
    }

    if (params->mcurv != NULL) {
	G_fseek(params->Tmp_fd_xy, 0L, 0);	/* seek to the beginning */
	for (i = 0; i < params->nsizr; i++) {
	    /* seek to the right row */
	    G_fseek(params->Tmp_fd_xy, (off_t) (params->nsizr - 1 - i) *
		    params->nsizc * sizeof(FCELL), 0);
	    fread(cell1, sizeof(FCELL), params->nsizc, params->Tmp_fd_xy);
	    Rast_put_f_row(cf6, cell1);
	}
    }

    if (cf1)
	Rast_close(cf1);
    if (cf2)
	Rast_close(cf2);
    if (cf3)
	Rast_close(cf3);
    if (cf4)
	Rast_close(cf4);
    if (cf5)
	Rast_close(cf5);
    if (cf6)
	Rast_close(cf6);


    /* colortable for elevations */
    Rast_init_colors(&colors);
    zstep = (FCELL) (zmaxac - zminac) / 5.;
    for (i = 1; i <= 5; i++) {
	data1 = (FCELL) (zminac + (i - 1) * zstep);
	data2 = (FCELL) (zminac + i * zstep);
	switch (i) {
	case 1:
	    Rast_add_f_color_rule(&data1, 0, 191, 191,
				      &data2, 0, 255, 0, &colors);
	    break;
	case 2:
	    Rast_add_f_color_rule(&data1, 0, 255, 0,
				      &data2, 255, 255, 0, &colors);
	    break;
	case 3:
	    Rast_add_f_color_rule(&data1, 255, 255, 0,
				      &data2, 255, 127, 0, &colors);
	    break;
	case 4:
	    Rast_add_f_color_rule(&data1, 255, 127, 0,
				      &data2, 191, 127, 63, &colors);
	    break;
	case 5:
	    Rast_add_f_color_rule(&data1, 191, 127, 63,
				      &data2, 20, 20, 20, &colors);
	    break;
	}
    }

    if (params->elev != NULL) {
	mapset = G_find_file("cell", params->elev, "");
	if (mapset == NULL) {
	    G_warning(_("Raster map <%s> not found"), params->elev);
	    return -1;
	}
	Rast_write_colors(params->elev, mapset, &colors);
	Rast_quantize_fp_map_range(params->elev, mapset,
				(DCELL) zminac - 0.5, (DCELL) zmaxac + 0.5,
				(CELL) (zminac - 0.5), (CELL) (zmaxac + 0.5));
    }

    /* colortable for slopes */
    if (cond1) {
	if (!params->deriv) {
	    /*
	     * smin = (CELL) ((int)(gmin*scig)); smax = (CELL) gmax; fprintf
	     * (stderr, "min %d max %d \n", smin,smax); Rast_make_rainbow_colors
	     * (&colors,smin,smax);
	     */
	    Rast_init_colors(&colors);
	    val1 = 0;
	    val2 = 2;
	    Rast_add_c_color_rule(&val1, 255, 255, 255, &val2, 255, 255, 0, &colors);
	    val1 = 2;
	    val2 = 5;
	    Rast_add_c_color_rule(&val1, 255, 255, 0, &val2, 0, 255, 0, &colors);
	    val1 = 5;
	    val2 = 10;
	    Rast_add_c_color_rule(&val1, 0, 255, 0, &val2, 0, 255, 255, &colors);
	    val1 = 10;
	    val2 = 15;
	    Rast_add_c_color_rule(&val1, 0, 255, 255, &val2, 0, 0, 255, &colors);
	    val1 = 15;
	    val2 = 30;
	    Rast_add_c_color_rule(&val1, 0, 0, 255, &val2, 255, 0, 255, &colors);
	    val1 = 30;
	    val2 = 50;
	    Rast_add_c_color_rule(&val1, 255, 0, 255, &val2, 255, 0, 0, &colors);
	    val1 = 50;
	    val2 = 90;
	    Rast_add_c_color_rule(&val1, 255, 0, 0, &val2, 0, 0, 0, &colors);
	}
	else {
	    Rast_init_colors(&colors);
	    dat1 = (FCELL) - 5.0;	/* replace by min dx, amin1 (c1min,
					 * c2min); */
	    dat2 = (FCELL) - 0.1;
	    Rast_add_f_color_rule(&dat1, 127, 0, 255,
				      &dat2, 0, 0, 255, &colors);
	    dat1 = dat2;
	    dat2 = (FCELL) - 0.01;
	    Rast_add_f_color_rule(&dat1, 0, 0, 255,
				      &dat2, 0, 127, 255, &colors);
	    dat1 = dat2;
	    dat2 = (FCELL) - 0.001;
	    Rast_add_f_color_rule(&dat1, 0, 127, 255,
				      &dat2, 0, 255, 255, &colors);
	    dat1 = dat2;
	    dat2 = (FCELL) 0.0;
	    Rast_add_f_color_rule(&dat1, 0, 255, 255,
				      &dat2, 200, 255, 200, &colors);
	    dat1 = dat2;
	    dat2 = (FCELL) 0.001;
	    Rast_add_f_color_rule(&dat1, 200, 255, 200,
				      &dat2, 255, 255, 0, &colors);
	    dat1 = dat2;
	    dat2 = (FCELL) 0.01;
	    Rast_add_f_color_rule(&dat1, 255, 255, 0,
				      &dat2, 255, 127, 0, &colors);
	    dat1 = dat2;
	    dat2 = (FCELL) 0.1;
	    Rast_add_f_color_rule(&dat1, 255, 127, 0,
				      &dat2, 255, 0, 0, &colors);
	    dat1 = dat2;
	    dat2 = (FCELL) 5.0;	/* replace by max dx, amax1 (c1max,
				 * c2max); */
	    Rast_add_f_color_rule(&dat1, 255, 0, 0,
				      &dat2, 255, 0, 200, &colors);
	}

	if (params->slope != NULL) {
	    mapset = G_find_file("cell", params->slope, "");
	    if (mapset == NULL) {
		G_warning(_("Raster map <%s> not found"), params->slope);
		return -1;
	    }
	    Rast_write_colors(params->slope, mapset, &colors);
	    Rast_quantize_fp_map_range(params->slope, mapset, 0., 90., 0, 90);

	    type = "raster";
	    Rast_short_history(params->slope, type, &hist1);
	    if (params->elev != NULL)
		sprintf(hist1.edhist[0], "The elevation map is %s",
			params->elev);
	    if (vect)
		sprintf(hist1.datsrc_1, "vector map %s", input);
	    else
		sprintf(hist1.datsrc_1, "site file %s", input);
	    hist1.edlinecnt = 1;

	    Rast_command_history(&hist1);
	    Rast_write_history(params->slope, &hist1);
	    if (params->ts)
		G_write_raster_timestamp(params->slope, params->ts);

	}

	/* colortable for aspect */
	if (!params->deriv) {
	    Rast_init_colors(&colors);
	    val1 = 0;
	    val2 = 0;
	    Rast_add_c_color_rule(&val1, 255, 255, 255, &val2, 255, 255, 255, &colors);
	    val1 = 1;
	    val2 = 90;
	    Rast_add_c_color_rule(&val1, 255, 255, 0, &val2, 0, 255, 0, &colors);
	    val1 = 90;
	    val2 = 180;
	    Rast_add_c_color_rule(&val1, 0, 255, 0, &val2, 0, 255, 255, &colors);
	    val1 = 180;
	    val2 = 270;
	    Rast_add_c_color_rule(&val1, 0, 255, 255, &val2, 255, 0, 0, &colors);
	    val1 = 270;
	    val2 = 360;
	    Rast_add_c_color_rule(&val1, 255, 0, 0, &val2, 255, 255, 0, &colors);
	}
	else {
	    Rast_init_colors(&colors);
	    dat1 = (FCELL) - 5.0;	/* replace by min dy, amin1 (c1min,
					 * c2min); */
	    dat2 = (FCELL) - 0.1;
	    Rast_add_f_color_rule(&dat1, 127, 0, 255,
				      &dat2, 0, 0, 255, &colors);
	    dat1 = dat2;
	    dat2 = (FCELL) - 0.01;
	    Rast_add_f_color_rule(&dat1, 0, 0, 255,
				      &dat2, 0, 127, 255, &colors);
	    dat1 = dat2;
	    dat2 = (FCELL) - 0.001;
	    Rast_add_f_color_rule(&dat1, 0, 127, 255,
				      &dat2, 0, 255, 255, &colors);
	    dat1 = dat2;
	    dat2 = (FCELL) 0.0;
	    Rast_add_f_color_rule(&dat1, 0, 255, 255,
				      &dat2, 200, 255, 200, &colors);
	    dat1 = dat2;
	    dat2 = (FCELL) 0.001;
	    Rast_add_f_color_rule(&dat1, 200, 255, 200,
				      &dat2, 255, 255, 0, &colors);
	    dat1 = dat2;
	    dat2 = (FCELL) 0.01;
	    Rast_add_f_color_rule(&dat1, 255, 255, 0,
				      &dat2, 255, 127, 0, &colors);
	    dat1 = dat2;
	    dat2 = (FCELL) 0.1;
	    Rast_add_f_color_rule(&dat1, 255, 127, 0,
				      &dat2, 255, 0, 0, &colors);
	    dat1 = dat2;
	    dat2 = (FCELL) 5.0;	/* replace by max dy, amax1 (c1max,
				 * c2max); */
	    Rast_add_f_color_rule(&dat1, 255, 0, 0,
				      &dat2, 255, 0, 200, &colors);
	}

	if (params->aspect != NULL) {
	    mapset = G_find_file("cell", params->aspect, "");
	    if (mapset == NULL) {
		G_warning(_("Raster map <%s> not found"), params->aspect);
		return -1;
	    }
	    Rast_write_colors(params->aspect, mapset, &colors);
	    Rast_quantize_fp_map_range(params->aspect, mapset, 0., 360., 0, 360);

	    type = "raster";
	    Rast_short_history(params->aspect, type, &hist2);
	    if (params->elev != NULL)
		sprintf(hist2.edhist[0], "The elevation map is %s",
			params->elev);
	    if (vect)
		sprintf(hist2.datsrc_1, "vector map %s", input);
	    else
		sprintf(hist2.datsrc_1, "site file %s", input);
	    hist2.edlinecnt = 1;

	    Rast_command_history(&hist2);
	    Rast_write_history(params->aspect, &hist2);
	    if (params->ts)
		G_write_raster_timestamp(params->aspect, params->ts);
	}

	/* colortable for curvatures */
	if (cond2) {
	    Rast_init_colors(&colors);
	    dat1 = (FCELL) amin1(c1min, c2min);	/* for derivatives use min
						 * dxx,dyy,dxy */
	    dat2 = (FCELL) - 0.01;
	    Rast_add_f_color_rule(&dat1, 127, 0, 255,
				      &dat2, 0, 0, 255, &colors);
	    dat1 = dat2;
	    dat2 = (FCELL) - 0.001;
	    Rast_add_f_color_rule(&dat1, 0, 0, 255,
				      &dat2, 0, 127, 255, &colors);
	    dat1 = dat2;
	    dat2 = (FCELL) - 0.00001;
	    Rast_add_f_color_rule(&dat1, 0, 127, 255,
				      &dat2, 0, 255, 255, &colors);
	    dat1 = dat2;
	    dat2 = (FCELL) 0.0;
	    Rast_add_f_color_rule(&dat1, 0, 255, 255,
				      &dat2, 200, 255, 200, &colors);
	    dat1 = dat2;
	    dat2 = (FCELL) 0.00001;
	    Rast_add_f_color_rule(&dat1, 200, 255, 200,
				      &dat2, 255, 255, 0, &colors);
	    dat1 = dat2;
	    dat2 = (FCELL) 0.001;
	    Rast_add_f_color_rule(&dat1, 255, 255, 0,
				      &dat2, 255, 127, 0, &colors);
	    dat1 = dat2;
	    dat2 = (FCELL) 0.01;
	    Rast_add_f_color_rule(&dat1, 255, 127, 0,
				      &dat2, 255, 0, 0, &colors);
	    dat1 = dat2;
	    dat2 = (FCELL) amax1(c1max, c2max);	/* for derivatives use max
						 * dxx,dyy,dxy */
	    Rast_add_f_color_rule(&dat1, 255, 0, 0,
				      &dat2, 255, 0, 200, &colors);

	    if (params->pcurv != NULL) {
		mapset = G_find_file("cell", params->pcurv, "");
		if (mapset == NULL) {
		    G_warning(_("Raster map <%s> not found"), params->pcurv);
		    return -1;
		}
		Rast_write_colors(params->pcurv, mapset, &colors);
		Rast_quantize_fp_map_range(params->pcurv, mapset, dat1, dat2,
					(CELL) (dat1 * MULT),
					(CELL) (dat2 * MULT));

		type = "raster";
		Rast_short_history(params->pcurv, type, &hist3);
		if (params->elev != NULL)
		    sprintf(hist3.edhist[0], "The elevation map is %s",
			    params->elev);
		if (vect)
		    sprintf(hist3.datsrc_1, "vector map %s", input);
		else
		    sprintf(hist3.datsrc_1, "site file %s", input);
		hist3.edlinecnt = 1;

		Rast_command_history(&hist3);
		Rast_write_history(params->pcurv, &hist3);
		if (params->ts)
		    G_write_raster_timestamp(params->pcurv, params->ts);
	    }

	    if (params->tcurv != NULL) {
		mapset = G_find_file("cell", params->tcurv, "");
		if (mapset == NULL) {
		    G_warning(_("Raster map <%s> not found"), params->tcurv);
		    return -1;
		}
		Rast_write_colors(params->tcurv, mapset, &colors);
		Rast_quantize_fp_map_range(params->tcurv, mapset, dat1, dat2,
					(CELL) (dat1 * MULT),
					(CELL) (dat2 * MULT));

		type = "raster";
		Rast_short_history(params->tcurv, type, &hist4);
		if (params->elev != NULL)
		    sprintf(hist4.edhist[0], "The elevation map is %s",
			    params->elev);
		if (vect)
		    sprintf(hist4.datsrc_1, "vector map %s", input);
		else
		    sprintf(hist4.datsrc_1, "site file %s", input);
		hist4.edlinecnt = 1;

		Rast_command_history(&hist4);
		Rast_write_history(params->tcurv, &hist4);
		if (params->ts)
		    G_write_raster_timestamp(params->tcurv, params->ts);
	    }

	    if (params->mcurv != NULL) {
		mapset = G_find_file("cell", params->mcurv, "");
		if (mapset == NULL) {
		    G_warning(_("Raster map <%s> not found"), params->mcurv);
		    return -1;
		}
		Rast_write_colors(params->mcurv, mapset, &colors);
		Rast_quantize_fp_map_range(params->mcurv, mapset, dat1, dat2,
					(CELL) (dat1 * MULT),
					(CELL) (dat2 * MULT));

		type = "raster";
		Rast_short_history(params->mcurv, type, &hist5);
		if (params->elev != NULL)
		    sprintf(hist5.edhist[0], "The elevation map is %s",
			    params->elev);
		if (vect)
		    sprintf(hist5.datsrc_1, "vector map %s", input);
		else
		    sprintf(hist5.datsrc_1, "site file %s", input);
		hist5.edlinecnt = 1;

		Rast_command_history(&hist5);
		Rast_write_history(params->mcurv, &hist5);
		if (params->ts)
		    G_write_raster_timestamp(params->mcurv, params->ts);
	    }
	}
    }

    if (params->elev != NULL) {
	mapset = G_find_file("cell", params->elev, "");
	if (mapset == NULL) {
	    G_warning(_("Raster map <%s> not found"), params->elev);
	    return -1;
	}
	type = "raster";
	Rast_short_history(params->elev, type, &hist);

	params->dmin = sqrt(params->dmin);

	/*
	 * sprintf (hist.edhist[0], "tension=%f, smoothing=%f", params->fi *
	 * dnorm / 1000., params->rsm);
	 */

	if (dtens) {
	    if (params->rsm == -1)
		sprintf(hist.edhist[0], "giventension=%f, smoothing att=%d",
			params->fi * 1000. / dnorm, params->smatt);
	    else
		sprintf(hist.edhist[0], "giventension=%f, smoothing=%f",
			params->fi * 1000. / dnorm, params->rsm);
	}
	else {
	    if (params->rsm == -1)
		sprintf(hist.edhist[0], "tension=%f, smoothing att=%d",
			params->fi * 1000. / dnorm, params->smatt);
	    else
		sprintf(hist.edhist[0], "tension=%f, smoothing=%f",
			params->fi, params->rsm);
	}

	sprintf(hist.edhist[1], "dnorm=%f, dmin=%f, zmult=%f",
		dnorm, params->dmin, params->zmult);
	/*
	 * sprintf(hist.edhist[2], "segmax=%d, npmin=%d, errtotal=%f",
	 * params->kmax,params->kmin,ertot);
	 */
	/*
	 * sprintf (hist.edhist[2], "segmax=%d, npmin=%d, errtotal =%f",
	 * params->kmax, params->kmin, sqrt (ertot) / n_points);
	 */

	sprintf(hist.edhist[2], "segmax=%d, npmin=%d, rmsdevi=%f",
		params->kmax, params->kmin, sqrt(ertot / n_points));

	sprintf(hist.edhist[3], "zmin_data=%f, zmax_data=%f", zmin, zmax);
	sprintf(hist.edhist[4], "zmin_int=%f, zmax_int=%f", zminac, zmaxac);

	if ((params->theta) && (params->scalex)) {
	    sprintf(hist.edhist[5], "theta=%f, scalex=%f", params->theta,
		    params->scalex);
	    hist.edlinecnt = 6;
	}
	else
	    hist.edlinecnt = 5;

	if (vect)
	    sprintf(hist.datsrc_1, "vector map %s", input);
	else
	    sprintf(hist.datsrc_1, "site file %s", input);

	Rast_command_history(&hist);
	Rast_write_history(params->elev, &hist);
	if (params->ts)
	    G_write_raster_timestamp(params->elev, params->ts);
    }

    /*
     * if (title) Rast_put_cell_title (output, title);
     */
    return 1;
}
