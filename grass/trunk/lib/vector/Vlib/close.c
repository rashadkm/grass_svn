/*
****************************************************************************
*
* MODULE:       Vector library 
*   	    	
* AUTHOR(S):    Original author CERL, probably Dave Gerdes or Mike Higgins.
*               Update to GRASS 5.7 Radim Blazek and David D. Gray.
*
* PURPOSE:      Higher level functions for reading/writing/manipulating vectors.
*
* COPYRIGHT:    (C) 2001 by the GRASS Development Team
*
*               This program is free software under the GNU General Public
*   	    	License (>=v2). Read the file COPYING that comes with GRASS
*   	    	for details.
*
*****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "Vect.h"

static int
clo_dummy () {
    return -1;
}

static int format () { G_fatal_error ("Requested format is not compiled in this version"); return 0; }

static int (*Close_array[][2]) () =
{
     { clo_dummy, V1_close_nat }
#ifdef HAVE_OGR
   , { clo_dummy, V1_close_ogr }
#else
   ,{ clo_dummy, format }
#endif
};


/*!
 \fn int Vect_close (struct Map_info *Map)
 \brief close vector data file
 \return 0 on success, non-zero on error
 \param Map_info structure
*/
int 
Vect_close (struct Map_info *Map)
{
    struct Coor_info CInfo;
    
    G_debug (1, "Vect_close(): name = %s, mapset = %s, format = %d, level = %d",
	         Map->name, Map->mapset, Map->format, Map->level);
    
    /* Store support files if in write mode on level 2 */
    if ( strcmp(Map->mapset,G_mapset()) == 0 && Map->support_updated && Map->plus.built == GV_BUILD_ALL) {
        char buf[1000];
        char file_path[2000];
        struct stat info;

	/* Delete old support files if available */
        sprintf (buf, "%s/%s", GRASS_VECT_DIRECTORY, Map->name);

        G__file_name ( file_path, buf, GV_TOPO_ELEMENT, G_mapset ());
        if (stat (file_path, &info) == 0)      /* file exists? */
	    unlink (file_path);

        G__file_name ( file_path, buf, GV_SIDX_ELEMENT, G_mapset ());
        if (stat (file_path, &info) == 0)      /* file exists? */
	    unlink (file_path);

        G__file_name ( file_path, buf, GV_CIDX_ELEMENT, G_mapset ());
        if (stat (file_path, &info) == 0)      /* file exists? */
	    unlink (file_path);

	Vect_coor_info ( Map, &CInfo);
	Map->plus.coor_size = CInfo.size;
	Map->plus.coor_mtime = CInfo.mtime;

	Vect_save_topo ( Map );

	/* Spatial index is not saved */
	/* Vect_save_spatial_index ( Map ); */

	Vect_cidx_save ( Map );

#ifdef HAVE_OGR
	if ( Map->format == GV_FORMAT_OGR ) 
	    V2_close_ogr ( Map );
#endif
    }
    
    if ( Map->level == 2 && Map->plus.release_support ) {
        G_debug (1, "free topology" );
	dig_free_plus ( &(Map->plus) );

	if ( !Map->head_only ) {
            G_debug (1, "free spatial index" );
	    dig_spidx_free ( &(Map->plus) );
	}

        G_debug (1, "free category index" );
	dig_cidx_free ( &(Map->plus) );

    }

    if ( Map->format == GV_FORMAT_NATIVE ) {
        G_debug (1, "close history file" );
	if ( Map->hist_fp != NULL ) fclose ( Map->hist_fp );
    }

    /* Close level 1 files / data sources if not head_only */
    if ( !Map->head_only ) {
	if (  ((*Close_array[Map->format][1]) (Map)) != 0 ) { 
	    G_warning ("Cannot close vector '%s'", Vect_get_full_name(Map) );
	    return 1;
	}
    }

    free (Map->name); Map->name = NULL;
    free (Map->mapset); Map->mapset = NULL;

    Map->open = VECT_CLOSED_CODE;

    return 0;
}

