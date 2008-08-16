
/****************************************************************************
 *
 * MODULE:       r.surf.fractal
 * AUTHOR(S):    Jo Wood, 19th October, 1994
 * PURPOSE:      GRASS module to manipulate a raster map layer.
 * COPYRIGHT:    (C) 2005 by the GRASS Development Team
 *
 *               This program is free software under the GNU General Public
 *               License (>=v2). Read the file COPYING that comes with GRASS
 *               for details.
 *
 *****************************************************************************/

#include <grass/glocale.h>
#include "frac.h"

char
 *rast_out_name,		/* Name of the raster output file.      */
 *mapset_out;

int
  fd_out,			/* File descriptor of output raster     */
  Steps;			/* Number of intermediate images.       */

double H;			/* Hausdorff-Besickovitch dimension.    */

int main(int argc, char *argv[])
{
    struct GModule *module;
    struct Option *rast_out;	/* Structure for output raster     */
    struct Option *frac_dim;	/* Fractal dimension of surface.   */
    struct Option *num_images;	/* Number of images to produce.    */

    G_gisinit(argv[0]);		/* Link with GRASS interface.      */

    module = G_define_module();
    module->keywords = _("raster");
    module->description =
	_("Creates a fractal surface of a given fractal dimension.");

    rast_out = G_define_option();
    rast_out->key = "out";
    rast_out->description = _("Name of fractal surface raster layer");
    rast_out->type = TYPE_STRING;
    rast_out->required = YES;

    frac_dim = G_define_option();
    frac_dim->key = "d";
    frac_dim->description = _("Fractal dimension of surface (2 < D < 3)");
    frac_dim->type = TYPE_DOUBLE;
    frac_dim->required = NO;
    frac_dim->answer = "2.05";

    num_images = G_define_option();
    num_images->key = "n";
    num_images->description = _("Number of intermediate images to produce");
    num_images->type = TYPE_INTEGER;
    num_images->required = NO;
    num_images->answer = "0";

    if (G_parser(argc, argv))	/* Performs the prompting for      */
	exit(EXIT_FAILURE);	/* keyboard input.                 */

    rast_out_name = rast_out->answer;
    sscanf(frac_dim->answer, "%lf", &H);
    H = 3.0 - H;
    Steps = atoi(num_images->answer) + 1;

    G_message(_("Steps=%d"), Steps);

    mapset_out = G_mapset();	/* Set output to current mapset.  */


    /*--------------------------------------------------------------------*/
    /*               CHECK FRACTAL DIMENSION IS WITHIN LIMITS             */

    /*--------------------------------------------------------------------*/

    if ((H <= 0) || (H >= 1)) {
	G_fatal_error(_("Fractal dimension of [%.2lf] must be between 2 and 3."),
		      3.0 - H);
    }

    process();

    return EXIT_SUCCESS;
}
