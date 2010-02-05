
/**************************************************************
 *									
 * MODULE:       v.lidar.edgedetection				
 * 								
 * AUTHOR(S):    Original version in GRASS 5.4 (s.edgedetection):
 * 		 Maria Antonia Brovelli, Massimiliano Cannata, 
 *		 Ulisse Longoni and Mirko Reguzzoni
 *
 *		 Update for GRASS 6.X and improvements:
 * 		 Roberto Antolin and Gonzalo Moreno
 *               							
 * PURPOSE:      Detection of object's edges on a LIDAR data set	
 *               							
 * COPYRIGHT:    (C) 2006 by Politecnico di Milano - 			
 *			     Polo Regionale di Como			
 *									
 *               This program is free software under the 		
 *               GNU General Public License (>=v2). 			
 *               Read the file COPYING that comes with GRASS		
 *               for details.					
 *							
 **************************************************************/

 /*INCLUDES*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <grass/gis.h>
#include <grass/raster.h>
#include <grass/vector.h>
#include <grass/dbmi.h>
    /* #include <grass/PolimiFunct.h> */
#include "edgedetection.h"

int edge_detection(struct Cell_head elaboration_reg, struct bound_box Overlap_Box,
		   double *parBilin, double obsX, double obsY,
		   double *partial, double alpha, double residual,
		   double gradHigh, double gradLow)
{
    /* 1 = PRE_TERRAIN */
    /* 2 = PRE_EDGE */
    /* 3 = PRE_UNKNOWN */

    int c1, c2;
    double g[9][2], gradient[2], gradPto, dirPto;
    extern double passoE, passoN;
    static struct Cell_head Elaboration;

    g[0][0] = partial[0];
    g[0][1] = partial[1];

    gradPto = g[0][0] * g[0][0] + g[0][1] * g[0][1];
    dirPto = atan(g[0][1] / g[0][0]) + M_PI / 2;	/* radiants */

    Elaboration = elaboration_reg;

    if ((gradPto > gradHigh) && (residual > 0))
	return PRE_EDGE;	/* Strong condition for 'edge' points */

    else if ((gradPto > gradLow) && (residual > 0)) {	/* Soft condition for 'edge' points */

	if (Vect_point_in_box(obsX, obsY, 0.0, &Overlap_Box)) {
	    Get_Gradient(Elaboration, obsX + passoE * cos(dirPto),
			     obsY + passoN * sin(dirPto), parBilin, gradient);
	    g[2][0] = gradient[0];
	    g[2][1] = gradient[1];

	    Get_Gradient(Elaboration, obsX + passoE * cos(dirPto + M_PI),
			     obsY + passoN * sin(dirPto + M_PI), parBilin, gradient);
	    g[7][0] = gradient[0];
	    g[7][1] = gradient[1];

	    if ((fabs(atan(g[2][1] / g[2][0]) + M_PI / 2 - dirPto) < alpha) &&
		(fabs(atan(g[7][1] / g[7][0]) + M_PI / 2 - dirPto) < alpha)) {

		Get_Gradient(Elaboration,
				 obsX + passoE * cos(dirPto + M_PI / 4),
				 obsY + passoN * sin(dirPto + M_PI / 4),
				 parBilin, gradient);
		g[1][0] = gradient[0];
		g[1][1] = gradient[1];

		Get_Gradient(Elaboration,
				 obsX + passoE * cos(dirPto - M_PI / 4),
				 obsY + passoN * sin(dirPto - M_PI / 4),
				 parBilin, gradient);
		g[3][0] = gradient[0];
		g[3][1] = gradient[1];

		Get_Gradient(Elaboration,
				 obsX + passoE * cos(dirPto + M_PI / 2),
				 obsY + passoN * sin(dirPto + M_PI / 2),
				 parBilin, gradient);
		g[4][0] = gradient[0];
		g[4][1] = gradient[1];

		Get_Gradient(Elaboration,
				 obsX + passoE * cos(dirPto - M_PI / 2),
				 obsY + passoN * sin(dirPto - M_PI / 2),
				 parBilin, gradient);
		g[5][0] = gradient[0];
		g[5][1] = gradient[1];

		Get_Gradient(Elaboration,
				 obsX + passoE * cos(dirPto + M_PI * 3 / 4),
				 obsY + passoN * sin(dirPto + M_PI * 3 / 4),
				 parBilin, gradient);
		g[6][0] = gradient[0];
		g[6][1] = gradient[1];

		Get_Gradient(Elaboration,
				 obsX + passoE * cos(dirPto - M_PI * 3 / 4),
				 obsY + passoN * sin(dirPto - M_PI * 3 / 4),
				 parBilin, gradient);
		g[8][0] = gradient[0];
		g[8][1] = gradient[1];

		c2 = 0;
		for (c1 = 0; c1 < 9; c1++)
		    if (g[c1][0] * g[c1][0] + g[c1][1] * g[c1][1] >
			gradHigh)
			c2++;

		if (c2 > 2)
		    return PRE_EDGE;
		else
		    return PRE_TERRAIN;
	    }
	    else
		return PRE_TERRAIN;
	}
	else
	    return PRE_UNKNOWN;
    }				/* END ELSE IF */
    else
	return PRE_TERRAIN;
}

int Get_Gradient(struct Cell_head Elaboration, double X, double Y,
		     double *parVect, double *grad)
{
    int row, col, N;
    double csi, eta, d, b, a, c;

    extern int nsply;
    extern double passoN, passoE;

    row = (int)((Y - Elaboration.south) / passoN);
    col = (int)((X - Elaboration.west) / passoE);
    N = nsply * col + row;
    eta = X - (Elaboration.west + (col * passoE));
    csi = Y - (Elaboration.south + (row * passoN));
    d = parVect[N];
    b = parVect[N + 1] - d;
    a = parVect[N + nsply] - d;
    c = parVect[N + 1 + nsply] - a - b - d;
    grad[0] = (a + c * eta);
    grad[1] = (b + c * csi);
    return 0;
}

void classification(struct Map_info *Out, struct Cell_head Elaboration,
		    struct bound_box General, struct bound_box Overlap, double **obs,
		    double *parBilin, double *parBicub, double mean,
		    double alpha, double gradHigh, double gradLow,
		    double overlap, int *line_num, int num_points,
		    dbDriver * driver, char *tabint_name, char *tab_name)
{
    int i, edge;
    double interpolation, weight, residual, eta, csi, gradient[2];

    extern int nsplx, nsply, line_out_counter;
    extern double passoN, passoE;

    struct line_pnts *point;
    struct line_cats *categories;

    point = Vect_new_line_struct();
    categories = Vect_new_cats_struct();

    for (i = 0; i < num_points; i++) {	/* Sparse points */
	G_percent(i, num_points, 2);

	Vect_reset_line(point);
	Vect_reset_cats(categories);

	if (Vect_point_in_box(obs[i][0], obs[i][1], mean, &General)) {
	    interpolation =
		dataInterpolateBicubic(obs[i][0], obs[i][1], passoE, passoN,
				       nsplx, nsply, Elaboration.west,
				       Elaboration.south, parBicub);
	    interpolation += mean;

	    Vect_copy_xyz_to_pnts(point, &obs[i][0], &obs[i][1], &obs[i][2], 1);

	    Get_Gradient(Elaboration, obs[i][0], obs[i][1], parBilin, gradient);

	    *point->z += mean;
	    /*Vect_cat_set (categories, F_INTERPOLATION, line_out_counter); */

	    if (Vect_point_in_box(obs[i][0], obs[i][1], interpolation, &Overlap)) {	/*(5) */

		residual = *point->z - interpolation;
		edge =
		    edge_detection(Elaboration, Overlap, parBilin, *point->x,
				   *point->y, gradient, alpha, residual,
				   gradHigh, gradLow);

		Vect_cat_set(categories, F_EDGE_DETECTION_CLASS, edge);
		Vect_cat_set(categories, F_INTERPOLATION, line_out_counter);
		Vect_write_line(Out, GV_POINT, point, categories);
		Insert_Interpolation(interpolation, line_out_counter, driver,
				     tabint_name);
		line_out_counter++;

	    }
	    else {
		if ((*point->x > Overlap.E) && (*point->x != General.E)) {

		    if ((*point->y > Overlap.N) && (*point->y != General.N)) {	/*(3) */
			csi = (*point->x - Overlap.E) / overlap;
			eta = (*point->y - Overlap.N) / overlap;
			weight = (1 - csi) * (1 - eta);

			gradient[0] *= weight;
			gradient[1] *= weight;
			interpolation *= weight;

			if (Select(&gradient[0], &gradient[1],
			           &interpolation, line_num[i], driver,
				   tab_name) != DB_OK)
			    G_fatal_error(_("Impossible to read from aux table"));

			if (UpDate(gradient[0], gradient[1],
			           interpolation, line_num[i], driver,
			           tab_name) != DB_OK)
			    G_fatal_error(_("Impossible to update aux table"));

		    }
		    else if ((*point->y < Overlap.S) && (*point->y != General.S)) {	/*(1) */
			csi = (*point->x - Overlap.E) / overlap;
			eta = (*point->y - General.S) / overlap;
			weight = (1 - csi) * eta;

			gradient[0] *= weight;
			gradient[1] *= weight;
			interpolation *= weight;

			if (Insert(gradient[0], gradient[1], interpolation,
			           line_num[i], driver, tab_name) != DB_OK)
			    G_fatal_error(_("Impossible to write to aux table"));

		    }
		    else if ((*point->y <= Overlap.N) && (*point->y >= Overlap.S)) {	/*(1) */
			weight = (*point->x - Overlap.E) / overlap;

			gradient[0] *= weight;
			gradient[1] *= weight;
			interpolation *= weight;

			if (Insert(gradient[0], gradient[1], interpolation,
			           line_num[i], driver, tab_name) != DB_OK)
			    G_fatal_error(_("Impossible to write to aux table"));
		    }

		}
		else if ((*point->x < Overlap.W) && (*point->x != General.W)) {

		    if ((*point->y > Overlap.N) && (*point->y != General.N)) {	/*(4) */
			csi = (*point->x - General.W) / overlap;
			eta = (*point->y - Overlap.N) / overlap;
			weight = (1 - eta) * csi;

			gradient[0] *= weight;
			gradient[1] *= weight;
			interpolation *= weight;

			if (Select(&gradient[0], &gradient[1],
			           &interpolation, line_num[i], driver,
			           tab_name) != DB_OK)
			    G_fatal_error(_("Impossible to read from aux table"));

			residual = *point->z - interpolation;
			edge =
			    edge_detection(Elaboration, Overlap, parBilin,
					   *point->x, *point->y, gradient,
					   alpha, residual, gradHigh,
					   gradLow);

			Vect_cat_set(categories, F_EDGE_DETECTION_CLASS,
				     edge);
			Vect_cat_set(categories, F_INTERPOLATION,
				     line_out_counter);
			Vect_write_line(Out, GV_POINT, point, categories);
			Insert_Interpolation(interpolation, line_out_counter,
					     driver, tabint_name);
			line_out_counter++;

		    }
		    else if ((*point->y < Overlap.S) && (*point->y != General.S)) {	/*(2) */
			csi = (*point->x - General.W) / overlap;
			eta = (*point->y - General.S) / overlap;
			weight = csi * eta;

			gradient[0] *= weight;
			gradient[1] *= weight;
			interpolation *= weight;

			if (Select(&gradient[0], &gradient[1],
			           &interpolation, line_num[i], driver,
			           tab_name) != DB_OK)
			    G_fatal_error(_("Impossible to read from aux table"));

			if (UpDate(gradient[0], gradient[1],
			           interpolation, line_num[i], driver,
			    tab_name) != DB_OK)
			    G_fatal_error(_("Impossible to update aux table"));

		    }
		    else if ((*point->y <= Overlap.N) && (*point->y >= Overlap.S)) {	/*(2) */
			weight = (Overlap.W - *point->x) / overlap;

			gradient[0] *= weight;
			gradient[1] *= weight;
			interpolation *= weight;

			if (Select(&gradient[0], &gradient[1],
			           &interpolation, line_num[i], driver,
			           tab_name) != DB_OK)
			    G_fatal_error(_("Impossible to read from aux table"));

			residual = *point->z - interpolation;
			edge =
			    edge_detection(Elaboration, Overlap, parBilin,
					   *point->x, *point->y, gradient,
					   alpha, residual, gradHigh,
					   gradLow);

			Vect_cat_set(categories, F_EDGE_DETECTION_CLASS,
				     edge);
			Vect_cat_set(categories, F_INTERPOLATION,
				     line_out_counter);
			Vect_write_line(Out, GV_POINT, point, categories);
			Insert_Interpolation(interpolation, line_out_counter,
					     driver, tabint_name);
			line_out_counter++;
		    }

		}
		else if ((*point->x <= Overlap.E) && (*point->x >= Overlap.W)) {
		    if ((*point->y > Overlap.N) && (*point->y != General.N)) {	/*(3) */
			weight = (*point->y - Overlap.N) / overlap;

			gradient[0] *= weight;
			gradient[1] *= weight;
			interpolation *= weight;

			if (Select(&gradient[0], &gradient[1],
			           &interpolation, line_num[i], driver,
			           tab_name) != DB_OK)
			    G_fatal_error(_("Impossible to read from aux table"));

			residual = *point->z - interpolation;
			edge =
			    edge_detection(Elaboration, Overlap, parBilin,
					   *point->x, *point->y, gradient,
					   alpha, residual, gradHigh,
					   gradLow);

			Vect_cat_set(categories, F_EDGE_DETECTION_CLASS,
				     edge);
			Vect_cat_set(categories, F_INTERPOLATION,
				     line_out_counter);
			Vect_write_line(Out, GV_POINT, point, categories);
			Insert_Interpolation(interpolation, line_out_counter,
					     driver, tabint_name);
			line_out_counter++;
		    }
		    else if ((*point->y < Overlap.S) && (*point->y != General.S)) {	/*(1) */
			weight = (Overlap.S - *point->y) / overlap;

			gradient[0] *= weight;
			gradient[1] *= weight;
			interpolation *= weight;

			if (Insert(gradient[0], gradient[1], interpolation,
			           line_num[i], driver, tab_name) != DB_OK)
			    G_fatal_error(_("Impossible to write to aux table"));

		    }		/*else (1) */
		}		/*else */
	    }
	}			/*end if obs */
    }				/*end for */
    G_percent(i, num_points, 6); /* finish it */

    Vect_destroy_line_struct(point);
    Vect_destroy_cats_struct(categories);
}				/*end puntisparsi_select */

int Insert(double partialX, double partialY, double Interp,
           int line_num, dbDriver * driver, char *tab_name)
{
    char buf[1024];
    dbString sql;
    int ret;

    db_init_string(&sql);
    sprintf(buf,
	    "INSERT INTO %s (ID, Interp, partialX, partialY)", tab_name);
    db_append_string(&sql, buf);
    sprintf(buf, " VALUES (%d, %lf, %lf, %lf)", line_num, Interp,
            partialX, partialY);
    db_append_string(&sql, buf);

    ret = db_execute_immediate(driver, &sql);
    db_free_string(&sql);

    return ret;
}

int Insert_Interpolation(double Interp, int line_num, dbDriver * driver,
			 char *tab_name)
{
    char buf[1024];
    dbString sql;
    int ret;

    db_init_string(&sql);
    sprintf(buf, "INSERT INTO %s (ID, Interp)", tab_name);
    db_append_string(&sql, buf);
    sprintf(buf, " VALUES (%d, %lf)", line_num, Interp);
    db_append_string(&sql, buf);

    ret = db_execute_immediate(driver, &sql);
    db_free_string(&sql);

    return ret;
}


int UpDate(double partialX, double partialY, double Interp,
	   int line_num, dbDriver * driver, char *tab_name)
{
    char buf[1024];
    dbString sql;
    int ret;

    db_init_string(&sql);
    sprintf(buf,
	    "UPDATE %s SET Interp=%lf, PartialX=%lf, PartialY=%lf WHERE ID=%d",
	    tab_name, Interp, partialX, partialY, line_num);
    db_append_string(&sql, buf);

    ret = db_execute_immediate(driver, &sql);
    db_free_string(&sql);

    return ret;
}

int Select(double *PartialX, double *PartialY, double *Interp,
           int line_num, dbDriver * driver, char *tab_name)
{
    int more;
    char buf[1024];
    dbString sql;
    dbTable *table;
    dbCursor cursor;
    dbColumn *PartialX_col, *PartialY_col, *Interp_col;
    dbValue *PartialX_value, *PartialY_value, *Interp_value;

    db_init_string(&sql);
    sprintf(buf,
	    "SELECT ID, Interp, partialX, partialY FROM %s WHERE ID=%d",
	    tab_name, line_num);
    db_append_string(&sql, buf);

    if (db_open_select_cursor(driver, &sql, &cursor, DB_SEQUENTIAL) != DB_OK)
	return -1;

    table = db_get_cursor_table(&cursor);

    while (db_fetch(&cursor, DB_NEXT, &more) == DB_OK && more) {

	Interp_col = db_get_table_column(table, 1);
	PartialX_col = db_get_table_column(table, 2);
	PartialY_col = db_get_table_column(table, 3);

	if (db_sqltype_to_Ctype(db_get_column_sqltype(Interp_col)) ==
	    DB_C_TYPE_DOUBLE)
	    Interp_value = db_get_column_value(Interp_col);
	else
	    continue;

	if (db_sqltype_to_Ctype(db_get_column_sqltype(PartialX_col)) ==
	    DB_C_TYPE_DOUBLE)
	    PartialX_value = db_get_column_value(PartialX_col);
	else
	    continue;

	if (db_sqltype_to_Ctype(db_get_column_sqltype(PartialY_col)) ==
	    DB_C_TYPE_DOUBLE)
	    PartialY_value = db_get_column_value(PartialY_col);
	else
	    continue;

	*Interp += db_get_value_double(Interp_value);
	*PartialX += db_get_value_double(PartialX_value);
	*PartialY += db_get_value_double(PartialY_value);
    }
    db_close_cursor(&cursor);
    db_free_string(&sql);
    return DB_OK;
}

int Create_AuxEdge_Table(dbDriver * driver, char *tab_name)
{
    dbTable *table;
    dbColumn *ID_col, *Interp_col, *PartialX_col, *PartialY_col;
    int created;

    table = db_alloc_table(4);
    db_set_table_name(table, tab_name);
    db_set_table_description(table,
			     "It is used for the intermediate interpolated and gradient values");

    ID_col = db_get_table_column(table, 0);
    db_set_column_name(ID_col, "ID");
    db_set_column_sqltype(ID_col, DB_SQL_TYPE_INTEGER);

    Interp_col = db_get_table_column(table, 1);
    db_set_column_name(Interp_col, "Interp");
    db_set_column_sqltype(Interp_col, DB_SQL_TYPE_REAL);

    PartialX_col = db_get_table_column(table, 2);
    db_set_column_name(PartialX_col, "PartialX");
    db_set_column_sqltype(PartialX_col, DB_SQL_TYPE_REAL);

    PartialY_col = db_get_table_column(table, 3);
    db_set_column_name(PartialY_col, "PartialY");
    db_set_column_sqltype(PartialY_col, DB_SQL_TYPE_REAL);

    if (db_create_table(driver, table) == DB_OK) {
	G_debug(1, "<%s> table created in database.", db_get_table_name(table));
	created = TRUE;
    }
    else
	return FALSE;

    db_create_index2(driver, tab_name, "ID");

    return created;
}

int Create_Interpolation_Table(dbDriver * driver, char *tab_name)
{
    dbTable *table;
    dbColumn *ID_col, *Interp_col;

    table = db_alloc_table(2);
    db_set_table_name(table, tab_name);
    db_set_table_description(table,
			     "This table is the bicubic interpolation of the input vector");

    ID_col = db_get_table_column(table, 0);
    db_set_column_name(ID_col, "ID");
    db_set_column_sqltype(ID_col, DB_SQL_TYPE_INTEGER);

    Interp_col = db_get_table_column(table, 1);
    db_set_column_name(Interp_col, "Interp");
    db_set_column_sqltype(Interp_col, DB_SQL_TYPE_REAL);

    if (db_create_table(driver, table) == DB_OK) {
	G_debug(1, _("<%s> table created in database."), db_get_table_name(table));
	return DB_OK;
    }
    else
	return DB_FAILED;
}


#ifdef notdef
/*! DEFINITION OF THE SUBZONES 

   -----------------------
   |4|   3   |3|       | |
   -----------------------
   | |       | |       | |
   |2|   5   |1|       | |
   | |       | |       | |
   -----------------------
   |2|   1   |1|       | |
   -----------------------
   | |       | |       | |
   | |       | |       | |
   | |       | |       | |
   -----------------------
   | |       | |       | |
   -----------------------
 */
#endif
