#include "Vect.h"
#include "gis.h"

int bin_to_asc(
    FILE *ascii,
    struct Map_info *Map,
    int ver,
    int pnt)
{
	int type, ctype, i;
	double *xptr, *yptr, *zptr;
	static struct line_pnts *Points;
	char buf1[100], buf2[100];
        struct line_cats *Cats;

	Points = Vect_new_line_struct ();	/* init line_pnts struct */
        Cats = Vect_new_cats_struct (); 


	/* by default, read_next_line will NOT read Dead lines */
	/*  but we can override that (in Level I only) by specifying */
	/*  the type  -1, which means match all line types */

	while(1)
	{
	        if (-1 == (type = Vect_read_next_line (Map, Points, Cats)))
		    return (-1);

		if (type == -2)	/* EOF */
	            return (0);

		if ( pnt && !(type & GV_POINT) ) continue;

		switch(type)
		{
		case GV_BOUNDARY:
			ctype = 'B';
			break;
		case GV_CENTROID:
			if ( ver < 5 ) { continue; }
			ctype = 'C';			
			break;			
		case GV_LINE:
			ctype = 'L';
			break;
		case GV_POINT:
			ctype = 'P';
			break;
		default:
			ctype = 'X';
			fprintf (stderr, "got type %d\n", (int) type);
			break;
		}

		if ( pnt ) {
		    /* fprintf(ascii, "%c", ctype); */
		    G_format_easting (Points->x[0], buf1, -1);
		    G_format_northing (Points->y[0], buf2, -1);
		    if ( Map->head.with_z  && ver == 5 ) {
			fprintf(ascii, "%s|%s|%f", buf1, buf2, Points->z[0]);
		    }
		    else {
			fprintf(ascii, "%s|%s", buf1, buf2);
		    }
		    if ( Cats->n_cats > 0 )		
	                fprintf(ascii, "|%d", Cats->cat[0]);
	            fprintf(ascii, "\n");
		} else { 
		    if ( ver == 5 && Cats->n_cats > 0 )
			fprintf(ascii, "%c  %d %d\n", ctype, Points->n_points, Cats->n_cats);		
		    else 
			fprintf(ascii, "%c  %d\n", ctype, Points->n_points);

		    xptr = Points->x;
		    yptr = Points->y;
		    zptr = Points->z;
		    
		    while (Points->n_points--)
		    {
			    G_format_northing (*yptr++, buf1, -1);
			    G_format_easting (*xptr++, buf2, -1);
			    
			    
			    if ( Map->head.with_z  && ver == 5 ) {
				fprintf(ascii, " %-12s %-12s %-12f\n", buf1, buf2, *zptr);
			    }
			    else {
				fprintf(ascii, " %-12s %-12s\n", buf1, buf2);
			    }
			    *zptr++;    
		    }

		    if ( ver == 5 ) {
			for ( i=0; i< Cats->n_cats; i++ ) {		
			    fprintf(ascii, " %-5d %-10d\n", Cats->field[i], Cats->cat[i]);
			}
		    }
		}
	}

	/* not reached */
}

