/*
 * $Id$
 *
 * d.path
*/

#include <stdlib.h>
#include <string.h>
#include "gis.h"
#include "raster.h"
#include "display.h"
#include "colors.h"
#include "Vect.h"
#include "dbmi.h"
#include "proto.h"

int main(int argc, char **argv)
{
    struct Option *map, *afield_opt, *nfield_opt, *afcol, *abcol, *ncol, *type_opt;
    struct Option *color_opt, *hcolor_opt, *bgcolor_opt;
    struct GModule *module;
    char   *mapset;
    struct Map_info Map;
    int    type, afield, nfield, color, hcolor, bgcolor;
    int    r, g, b, colornum = MAX_COLOR_NUM;

    /* Initialize the GIS calls */
    G_gisinit (argv[0]) ;

    module = G_define_module();
    module->description = 
    "Find shortest path for selected starting and ending node";

    map = G_define_standard_option(G_OPT_V_MAP);

    type_opt =  G_define_standard_option(G_OPT_V_TYPE);
    type_opt->options    = "line,boundary";
    type_opt->answer     = "line,boundary";
    type_opt->description = "Arc type";

    afield_opt = G_define_standard_option(G_OPT_V_FIELD);
    afield_opt->key = "afield";
    afield_opt->answer = "0";
    afield_opt->description = "Arc field";
    
    nfield_opt = G_define_standard_option(G_OPT_V_FIELD);
    nfield_opt->key = "nfield";
    nfield_opt->answer = "0";
    nfield_opt->description = "Node field";

    afcol = G_define_option() ;
    afcol->key         = "afcol" ;
    afcol->type        = TYPE_STRING ;
    afcol->required    = NO ; 
    afcol->description = "Arc forward/both direction(s) cost column" ;
    
    abcol = G_define_option() ;
    abcol->key         = "abcol" ;
    abcol->type        = TYPE_STRING ;
    abcol->required    = NO ; 
    abcol->description = "Arc backward direction cost column" ;
    
    ncol = G_define_option() ;
    ncol->key         = "ncol" ;
    ncol->type        = TYPE_STRING ;
    ncol->required    = NO ; 
    ncol->description = "Node cost column" ;
    
    color_opt = G_define_option() ;
    color_opt->key        = "color" ;
    color_opt->type       = TYPE_STRING ;
    color_opt->answer     = "white" ;
    color_opt->description= "Original line color" ;
	
    hcolor_opt = G_define_option() ;
    hcolor_opt->key        = "hcolor" ;
    hcolor_opt->type       = TYPE_STRING ;
    hcolor_opt->answer     = "red" ;
    hcolor_opt->description= "Highlight color" ;
    
    bgcolor_opt = G_define_option() ;
    bgcolor_opt->key        = "bgcolor" ;
    bgcolor_opt->type       = TYPE_STRING ;
    bgcolor_opt->answer     = "black" ;
    bgcolor_opt->description= "Background color" ;
    
    if(G_parser(argc,argv))
        G_fatal_error("");

    type = Vect_option_to_types ( type_opt ); 
    afield = atoi (afield_opt->answer);
    nfield = atoi (nfield_opt->answer);

    color = WHITE;
    if ( G_str_to_color(color_opt->answer, &r, &g, &b) ) {
        colornum++;
	R_reset_color (r, g, b, colornum); 
	color = colornum;
    }

    hcolor = RED;
    if ( G_str_to_color(hcolor_opt->answer, &r, &g, &b) ) {
        colornum++;
	R_reset_color (r, g, b, colornum); 
	hcolor = colornum;
    }
    
    bgcolor = BLACK;
    if ( G_str_to_color(bgcolor_opt->answer, &r, &g, &b) ) {
        colornum++;
	R_reset_color (r, g, b, colornum); 
	bgcolor = colornum;
    }
    
    mapset = G_find_vector2 (map->answer, NULL); 
      
    if ( mapset == NULL) 
      G_fatal_error ("Could not find input %s\n", map->answer);

    Vect_set_open_level(2);
    Vect_open_old (&Map, map->answer, mapset); 

    if (R_open_driver() != 0) {
    G_fatal_error ("No graphics device selected");
    Vect_close(&Map);
    }
    D_setup(0);

    G_setup_plot (D_get_d_north(), D_get_d_south(),
		D_get_d_west(), D_get_d_east(),
		D_move_abs, D_cont_abs);

    Vect_net_build_graph ( &Map, type , afield, nfield, afcol->answer, abcol->answer, ncol->answer, 0 );
    path ( &Map, color, hcolor, bgcolor ); 


    R_close_driver();

    Vect_close(&Map);

    exit(0);
}



