#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <grass/gis.h>
#include <grass/Vect.h>
#include <grass/dbmi.h>
#include <grass/raster.h>
#include <grass/display.h>
#include <grass/colors.h>
#include <grass/form.h>
#include "global.h"
#include "proto.h"

int 
write_line ( struct Map_info *Map, int type, struct line_pnts *Points )
{
    int i, field, cat, ret;
    static int first_form = 1;
    char *form;
    struct line_cats *Cats;
    struct field_info *Fi;
    dbString html;
    
    Cats = Vect_new_cats_struct ();
    db_init_string (&html);
    
    cat = var_geti(VAR_CAT);
    if ( var_geti(VAR_CAT_MODE) != CAT_MODE_NO &&  cat > 0 && var_geti(VAR_FIELD) > 0 ) {
        Vect_cat_set ( Cats, var_geti(VAR_FIELD), cat ); 
	
        G_debug (2, "write field = %d cat = %d", var_geti(VAR_FIELD), var_geti(VAR_CAT)); 

        if ( cat_max_get( var_geti(VAR_FIELD) ) < var_geti(VAR_CAT)  ) {
	    cat_max_set( var_geti(VAR_FIELD), var_geti(VAR_CAT) );
        }
    }
    
    ret = Vect_write_line ( Map, type, Points, Cats );

    for ( i = 0; i < Vect_get_num_updated_lines(Map); i++ )
	G_debug (2, "Updated line: %d", Vect_get_updated_line( Map, i ) );

    for ( i = 0; i < Vect_get_num_updated_nodes(Map); i++ )
	G_debug (2, "Updated node: %d", Vect_get_updated_node( Map, i ) );

    /* Reset category (this automaticaly resets cat for next not used) */
    var_seti ( VAR_FIELD, var_geti ( VAR_FIELD ) );

    if ( var_geti(VAR_CAT_MODE) != CAT_MODE_NO && var_geti(VAR_INSERT) && cat > 0 ) {
	G_debug (2, "Insert new record" );
        db_set_string (&html, "<HTML><HEAD><TITLE>Form</TITLE><BODY>");

	field = var_geti(VAR_FIELD);
	ret = new_record(field, cat); 
	if ( ret == -1 ) { 
	    return -1;
	} else if ( ret == 0 ) {
	    db_append_string (&html, "New record was created.<BR>");
	} else { /* record already existed */
	    db_append_string (&html, "Record for this category already existed.<BR>");
	}

	/* Open form */
	Fi = Vect_get_field( Map, field );
	if ( Fi == NULL ) {
	    return -1;   
	}
        F_generate ( Fi->driver, Fi->database, Fi->table, Fi->key, cat, NULL, NULL, 
		                  F_EDIT, F_HTML, &form);
	db_append_string (&html, form );
	db_append_string (&html, "</BODY></HTML>");
    
	/* Note: F_open() must be run first time with closed monitor, otherwise next
	*        attempt to open driver hangs until form child process is killed */
	if ( first_form ) driver_close();
	F_clear ();
	F_open ( "Attributes", db_get_string(&html) );
	if ( first_form ) { driver_open(); first_form = 0; }

        G_free (form);	
	db_free_string (&html);

    }

    return 0;
}

/* Snap to node */
int snap ( double *x, double *y ) 
{
    int node;
    double thresh;
    
    G_debug (2, "snap(): x = %f, y = %f", *x, *y); 
    
    if ( !var_geti(VAR_SNAP) ) return 0;
    
    if ( var_geti(VAR_SNAP_MODE) == SNAP_MAP ) {
	thresh = var_getd(VAR_SNAP_MAP);
    } else {
	thresh = Scale * var_geti(VAR_SNAP_SCREEN);
    }
    
    node = Vect_find_node ( &Map, *x, *y, 0, thresh, 0 );

    if ( node > 0 ) Vect_get_node_coor (&Map, node, x, y, NULL );
	
    G_debug (2, "node = %d x = %f, y = %f", node, *x, *y); 
    return node;
}

/* Digitize new line */
int new_line ( int type )
{
    int i, sxo = 0, syo = 0, sxn, syn;
    int button, first, line, node1, node2;
    double x, y;
    char buf[1000];
    struct line_pnts *Points;
    struct line_cats *Cats;
    
    G_debug (2, "new_line(): type = %d", type);

    Points = Vect_new_line_struct ();
    Cats = Vect_new_cats_struct ();
    
    sprintf ( buf, "Digitize new %s:", get_line_type_name (type) );
    i_prompt ( buf ); 
    i_prompt_buttons ( "New point", "", "Quit tool"); 
    
    i_new_line_options ( 1 );
    
    driver_open();
    
    first = 1; 
    sxn = COOR_NULL; syn = COOR_NULL;
    while ( 1 ) {
	/* Get next coordinate */
        R_set_update_function ( update );
	if ( first ) {
	    R_get_location_with_pointer ( &sxn, &syn, &button); 
            i_prompt_buttons ( "New point", "Undo last point", "Close line"); 
        } else R_get_location_with_line (sxo, syo, &sxn, &syn, &button); 
	
	x =  D_d_to_u_col ( sxn );
	y =  D_d_to_u_row ( syn );
	G_debug (3, "button = %d x = %d = %f y = %d = %f", button, sxn, x, syn, y);

	if ( button == 0 ) break; /* Tool broken by GUI */

	if ( first &&  button == rightb ) { /* Quit tool ( points & lines ), first is always for points */
	    Tool_next = TOOL_NOTHING; 
	    break;
	}

	if ( type & GV_POINTS ) {
	    /* We can get here with button = 1 or 2 -> the same write point */
	    snap ( &x, &y );
            Vect_append_point ( Points, x, y, 0 );	

	    write_line ( &Map, type, Points );
	    updated_lines_and_nodes_erase_refresh_display();
	    break;
	} else { /* GV_LINES */
	    /* Button may be 1,2,3 */
	    if ( button == leftb ) { /* New point */
		snap ( &x, &y );
                Vect_append_point ( Points, x, y, 0 );	
		
		if ( type == GV_LINE ) symb_set_driver_color ( SYMB_LINE );
		else symb_set_driver_color ( SYMB_BOUNDARY_0 );

		display_points ( Points, 1);
		sxo = D_u_to_d_col(x); syo = D_u_to_d_row (y);;
		first = 0;
	    } else if ( button == middleb ) { /* Undo last point */
		if ( Points->n_points >= 1 ) {
		    symb_set_driver_color ( SYMB_BACKGROUND ); 
		    display_points ( Points, 1);
		    Points->n_points--;

		    if ( type == GV_LINE ) symb_set_driver_color ( SYMB_LINE );
		    else symb_set_driver_color ( SYMB_BOUNDARY_0 );

                    display_points ( Points, 1);
		    sxo = D_u_to_d_col ( Points->x[Points->n_points - 1] );
		    syo = D_u_to_d_row ( Points->y[Points->n_points - 1] );
		}
		if ( Points->n_points == 0 ) {
		    i_prompt_buttons ( "New point", "", "Quit tool"); 
		    first = 1;
		}
	    } else { /* button = 3 -> write the line and quit */
		if ( Points->n_points > 1 ) {
		    /* Before the line is written, we must check if connected to existing nodes, if yes,
		     * such nodes must be add to update list before! the line is written (areas/isles */
                    node1 = Vect_find_node(&Map, Points->x[0], Points->y[0], Points->z[0], 0, Vect_is_3d(&Map)); 
		    i = Points->n_points - 1;
                    node2 = Vect_find_node(&Map, Points->x[i], Points->y[i], Points->z[i], 0, Vect_is_3d(&Map)); 
		    
	            G_debug (2, "  old node1 = %d  old node2 = %d", node1, node2);
	            line = write_line ( &Map, type, Points );
		    updated_lines_and_nodes_erase_refresh_display();
		} else G_warning ("Less than 2 points for line -> nothing written" );
		    
		break;
	    }
	    G_debug (2, "n_points = %d", Points->n_points);
	}
    }
    driver_close();
   
    i_prompt (""); 
    i_prompt_buttons ( "", "", ""); 
    i_coor ( COOR_NULL, COOR_NULL); 
    i_new_line_options ( 0 );
    
    G_debug (3, "new_line(): End");

    return 1;
}

/* Continue work on the end of a line */
int edit_line (void) {
    int i,sxn,syn,sxo,syo,button,line,line_type,reversed;
    int node1,node2;
    double x,y,thresh,nodex,nodey,nodez,dist;
    struct line_pnts *Points;
    struct line_cats *Cats;
    
    G_debug (2, "edit_line()");
    
    i_prompt ( "Edit line or boundary:");
    i_prompt_buttons ( "Select", "", "Quit tool");
    
    driver_open();
    
    /* TODO: use some better threshold */
    thresh = fabs ( D_d_to_u_col ( 10 ) - D_d_to_u_col ( 0 ) ) ; 
    G_debug (2, "thresh = %f", thresh );
    
    line = 0;
    sxn = COOR_NULL; syn = COOR_NULL;
    while (line<=0) {
	/* Get next coordinate */
        R_set_update_function ( update );
        R_get_location_with_pointer ( &sxn, &syn, &button); 
	    
	x =  D_d_to_u_col ( sxn );
	y =  D_d_to_u_row ( syn );
	G_debug (3, "button = %d x = %d = %f y = %d = %f", button, sxn, x, syn, y);
        
        if (button==0 || button==rightb) break; /* Quit tool */
        
        if (button!=leftb)
            continue;
        
        /* Find nearest point or line */
        line = Vect_find_line (&Map, x, y, 0, GV_LINE|GV_BOUNDARY, thresh, 0, 0);
        G_debug (2, "line found = %d", line );
        
        /* Display new selected line if any */
        if ( line > 0 ) {
            display_line ( line, SYMB_HIGHLIGHT, 1);
        }
    }
    
    if (line<=0) {
        driver_close();
        
        i_prompt (""); 
        i_prompt_buttons ( "", "", ""); 
        i_coor ( COOR_NULL, COOR_NULL); 
        
        G_debug (3, "edit_line(): End");
    
        return 1;
    }

    Points = Vect_new_line_struct ();
    Cats = Vect_new_cats_struct ();    
    line_type = Vect_read_line (&Map, Points, Cats, line);
    
    reversed = 0;
    
    /* Find out the node nearest to the line */
    Vect_get_line_nodes (&Map, line, &node1, &node2);
    
    Vect_get_node_coor (&Map, node2, &nodex, &nodey, &nodez);
    dist=(x-nodex)*(x-nodex)+(y-nodey)*(y-nodey);
    
    Vect_get_node_coor (&Map, node1, &nodex, &nodey, &nodez);
    if ((x-nodex)*(x-nodex)+(y-nodey)*(y-nodey)<dist) {
        /* The first node is the nearest => reverse the line and remember
         * doing so. */
         Vect_line_reverse (Points);
         reversed = 1;
    }
    
    display_node ( node1, SYMB_BACKGROUND, 1);
    display_node ( node2, SYMB_BACKGROUND, 1);
    i_prompt_buttons ( "New Point", "Undo Last Point", "Close line");
    sxo = D_u_to_d_col ( Points->x[Points->n_points - 1] );
    syo = D_u_to_d_row ( Points->y[Points->n_points - 1] );
    /* Do the actual editing */
    while (1) {
        R_set_update_function ( update );
        R_get_location_with_line (sxo, syo, &sxn, &syn, &button); 
	
	x =  D_d_to_u_col ( sxn );
	y =  D_d_to_u_row ( syn );
	G_debug (3, "button = %d x = %d = %f y = %d = %f", button, sxn, x, syn, y);

	if ( button == 0 || button==rightb ) break; /* Tool broken by GUI */
        if ( button == leftb ) { /* New point */
            snap ( &x, &y );
            Vect_append_point ( Points, x, y, 0 );	
            
            if ( line_type == GV_LINE ) symb_set_driver_color ( SYMB_LINE );
            else symb_set_driver_color ( SYMB_BOUNDARY_0 );

            display_points ( Points, 1);
            sxo=sxn;syo=syn;
            i_prompt_buttons ( "New Point", "Undo Last Point", "Close line");
        } else if ( button == middleb ) { /* Undo last point */
            if ( Points->n_points > 1 ) {
                symb_set_driver_color ( SYMB_BACKGROUND ); 
                display_points ( Points, 1);
                
                Points->n_points--;

                if ( line_type == GV_LINE ) symb_set_driver_color ( SYMB_LINE );
                else symb_set_driver_color ( SYMB_BOUNDARY_0 );

                display_points ( Points, 1);
                sxo = D_u_to_d_col ( Points->x[Points->n_points - 1] );
                syo = D_u_to_d_row ( Points->y[Points->n_points - 1] );
                if (Points->n_points==1)
                    i_prompt_buttons ( "New Point", "", "Delete line and exit");
            }
        }
    }
    
    if (reversed)
        Vect_line_reverse (Points);
    
    if (Points->n_points>1) {
        Vect_rewrite_line (&Map, line, line_type, Points, Cats);
        updated_lines_and_nodes_erase_refresh_display ();
    } else {
        /* delete lines with less than two points */
        Vect_delete_line ( &Map, line ); 
        for ( i = 0 ; i < Cats->n_cats; i++ ) {
            check_record ( Cats->field[i], Cats->cat[i] );
        }
    }
    
    driver_close();
    
    Vect_destroy_line_struct (Points);
    Vect_destroy_cats_struct (Cats);
    
    i_prompt (""); 
    i_prompt_buttons ( "", "", ""); 
    i_coor ( COOR_NULL, COOR_NULL); 
    
    G_debug (3, "edit_line(): End");
    
    return 1;
}

/* Delete line */
int delete_line (void)
{
    int i, sxn, syn, line, last_line, node1, node2;
    int button, first;
    double x, y, thresh;
    struct line_pnts *Points;
    struct line_cats *Cats;
    
    G_debug (2, "delete_line()");

    Points = Vect_new_line_struct ();
    Cats = Vect_new_cats_struct ();
    
    i_prompt ( "Delete point, line, boundary, or centroid:");
    i_prompt_buttons ( "Select", "Unselect", "Quit tool"); 
    
    driver_open();
    
    /* TODO: use some better threshold */
    thresh = fabs ( D_d_to_u_col ( 10 ) - D_d_to_u_col ( 0 ) ) ; 
    G_debug (2, "thresh = %f", thresh );
    
    line = 0;
    first = 1;
    last_line = 0;
    sxn = COOR_NULL; syn = COOR_NULL;
    while ( 1 ) {
	/* Get next coordinate */
        R_set_update_function ( update );
        R_get_location_with_pointer ( &sxn, &syn, &button); 
	    
	x =  D_d_to_u_col ( sxn );
	y =  D_d_to_u_row ( syn );
	G_debug (3, "button = %d x = %d = %f y = %d = %f", button, sxn, x, syn, y);

	/* Display last highlighted in normal color */
	if ( last_line > 0 ) {
	    display_line ( last_line, SYMB_DEFAULT, 1);
	}

	if ( button == 0 || button == rightb ) break; /* Quit tool */

	if ( button == leftb ) { /* Confirm / select */
            /* Delete last if any */
	    if ( last_line > 0 ) { 
		/* Erase line and nodes !!! (because if the line is not connected to any other, nodes will die */
		display_line ( last_line, SYMB_BACKGROUND, 1 );
		Vect_get_line_nodes ( &Map, line, &node1, &node2 ); 
		G_debug (2, "delete line = %d node1 = %d node2 = %d", last_line, node1, node2);

                display_node ( node1, SYMB_BACKGROUND, 1);
                display_node ( node2, SYMB_BACKGROUND, 1);
		
		Vect_read_line ( &Map, NULL, Cats, last_line );
		Vect_delete_line ( &Map, last_line ); 
		for ( i = 0 ; i < Cats->n_cats; i++ ) {
		    check_record ( Cats->field[i], Cats->cat[i] );
		}

		for ( i = 0; i < Vect_get_num_updated_lines(&Map); i++ )
		    G_debug (2, "Updated line: %d", Vect_get_updated_line( &Map, i ) );

		for ( i = 0; i < Vect_get_num_updated_nodes(&Map); i++ )
		    G_debug (2, "Updated node: %d", Vect_get_updated_node( &Map, i ) );

		updated_lines_and_nodes_erase_refresh_display();
	    }
	    
	    /* Find neares point or line */
	    line = Vect_find_line (&Map, x, y, 0, GV_POINT|GV_CENTROID, thresh, 0, 0);
	    G_debug (2, "point found = %d", line );
	    if ( line == 0 ) line = Vect_find_line (&Map, x, y, 0, GV_LINE|GV_BOUNDARY, thresh, 0, 0);
	    G_debug (2, "line found = %d", line );
	    
	    /* Display new selected line if any */
	    if ( line > 0 ) {
		display_line ( line, SYMB_HIGHLIGHT, 1);
	    }
	} else { /* button == 2 -> unselect */
	    line = 0;
	}
	
	if ( line > 0 ) 
	    i_prompt_buttons ( "Confirm and select next", "Unselect", "Quit tool"); 
	else
	    i_prompt_buttons ( "Select", "Unselect", "Quit tool"); 
	
	last_line = line;
	first = 0;
    }

    driver_close();
    
    i_prompt (""); 
    i_prompt_buttons ( "", "", ""); 
    i_coor ( COOR_NULL, COOR_NULL); 
    
    G_debug (3, "delete_line(): End");

    return 1;
}

/* Move line */
int move_line (void)
{
    int i, sxn, syn, sxo = 0, syo = 0, line, last_line, node1, node2, type;
    int button, first;
    double x, y, thresh, xo, yo;
    struct line_pnts *Points;
    struct line_cats *Cats;
    
    G_debug (2, "move_line()");

    Points = Vect_new_line_struct ();
    Cats = Vect_new_cats_struct ();
    
    i_prompt ( "Move point, line, boundary, or centroid:"); 
    
    driver_open();
    
    /* TODO: use some better threshold */
    thresh = fabs ( D_d_to_u_col ( 10 ) - D_d_to_u_col ( 0 ) ) ; 
    G_debug (2, "thresh = %f", thresh );
    
    first = 1; 
    last_line = 0;
    sxn = COOR_NULL; syn = COOR_NULL;
    while ( 1 ) {
	/* Get next coordinate */
        R_set_update_function ( update );
	if ( last_line == 0 ) {
            i_prompt_buttons ( "Select", "", "Quit tool"); 
	    R_get_location_with_pointer ( &sxn, &syn, &button); 
        } else R_get_location_with_line (sxo, syo, &sxn, &syn, &button); 
	
	if ( last_line > 0 ) {
	    display_line ( last_line, SYMB_DEFAULT, 1);
	}

	x =  D_d_to_u_col ( sxn );
	y =  D_d_to_u_row ( syn );
	G_debug (3, "button = %d x = %d = %f y = %d = %f", button, sxn, x, syn, y);

	if ( button == 0 || button == rightb ) break;

	if ( button == leftb ) { /* Select / new location */
	    if ( last_line == 0 ) { /* Select line */ 
	        line = Vect_find_line (&Map, x, y, 0, GV_POINT|GV_CENTROID, thresh, 0, 0);
		G_debug (2, "point found = %d", line );
                if ( line == 0 ) line = Vect_find_line (&Map, x, y, 0, GV_LINE|GV_BOUNDARY, thresh, 0, 0);
                G_debug (2, "line found = %d", line );
                
                /* Display new selected line if any */
                if ( line > 0 ) {
		    display_line ( line, SYMB_HIGHLIGHT, 1);

		    /* Find the nearest point on the line */
		    type = Vect_read_line ( &Map, Points, NULL, line );
		    Vect_line_distance ( Points, x, y, 0, 0, &xo, &yo, NULL, NULL, NULL, NULL );
                    sxo = D_u_to_d_col ( xo ) ; 
		    syo = D_u_to_d_row ( yo );


                    i_prompt_buttons ( "New location", "Unselect", "Quit tool"); 
	        }
		last_line = line;
	    } else { /* Line is already selected */
	        display_line ( last_line, SYMB_BACKGROUND, 1);
		Vect_get_line_nodes ( &Map, last_line, &node1, &node2 ); 
                display_node ( node1, SYMB_BACKGROUND, 1);
                display_node ( node2, SYMB_BACKGROUND, 1);

		type = Vect_read_line ( &Map, Points, Cats, last_line );
		for ( i = 0; i < Points->n_points; i++ ) {
                    Points->x[i] = Points->x[i] + x - xo;
                    Points->y[i] = Points->y[i] + y - yo;
		}

		Vect_rewrite_line(&Map, last_line, type, Points, Cats);

		updated_lines_and_nodes_erase_refresh_display();
		last_line = 0;
	    }

	}
	if ( button == middleb ) { /* Unselect */
	    if ( last_line > 0 ) {
		last_line = 0;
	    }
	}
    }

    driver_close();
    
    i_prompt (""); 
    i_prompt_buttons ( "", "", ""); 
    i_coor ( COOR_NULL, COOR_NULL); 
    
    G_debug (3, "move_line(): End");

    return 1;
}

