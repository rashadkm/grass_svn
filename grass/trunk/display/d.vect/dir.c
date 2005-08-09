/* plot1() - Level One vector reading */

#include "gis.h"
#include "Vect.h"
#include "display.h"
#include "raster.h"
#include "plot.h"
#include "glocale.h"


int dir ( struct Map_info *Map, int type, struct cat_list * Clist, int chcat )
{
    int    ltype, dsize;
    double len, x, y, angle, msize;
    struct line_pnts *Points;
    struct line_cats *Cats;

    G_debug (1, "display direction:");
    dsize = 5;
    msize = dsize * ( D_d_to_u_col(2.0) - D_d_to_u_col(1.0) );/* do it better */

    Points = Vect_new_line_struct ();
    Cats = Vect_new_cats_struct ();
    
    Vect_rewind ( Map );
    
    while (1)
     {
	ltype =  Vect_read_next_line (Map, Points, Cats);   
        switch ( ltype )
	{
	case -1:
	    fprintf (stderr, _("\nERROR: vector file - can't read\n" ));
	    return -1;
	case -2: /* EOF */
	    return  0;
	}

	if ( !(type & ltype & GV_LINES )  ) continue;

        if ( chcat ){
	     int i, found = 0;

	     for ( i = 0; i < Cats->n_cats; i++ ) {
		 if ( Cats->field[i] == Clist->field && Vect_cat_in_cat_list ( Cats->cat[i], Clist) ) {
		     found = 1;
		     break;
		 }
	     }
	     if (!found) continue;
        }
	
	len = Vect_line_length ( Points );

	Vect_point_on_line ( Points, len/2, &x, &y, NULL, &angle, NULL );

	G_debug (3, "plot direction: %f, %f", x, y);
        G_plot_icon(x, y, G_ICON_ARROW, angle, msize);
      }
	

    Vect_destroy_line_struct (Points);
    Vect_destroy_cats_struct (Cats);
    
    return 0; /* not reached */
}


