#!/usr/bin/env python
# -*- coding: utf-8 -*-
############################################################################
#
# MODULE:	t.rast.univar
# AUTHOR(S):	Soeren Gebbert
#
# PURPOSE:	Calculates univariate statistics from the non-null cells for each registered raster map of a space time raster dataset
# COPYRIGHT:	(C) 2011-2014, Soeren Gebbert and the GRASS Development Team
#
#		This program is free software under the GNU General Public
#		License (version 2). Read the file COPYING that comes with GRASS
#		for details.
#
#############################################################################

#%module
#% description: Calculates univariate statistics from the non-null cells for each registered raster map of a space time raster dataset.
#% keyword: temporal
#% keyword: statistics
#% keyword: raster
#% keyword: time
#%end

#%option G_OPT_STRDS_INPUT
#%end

#%option G_OPT_F_OUTPUT
#% required: no
#%end

#%option G_OPT_T_WHERE
#% guisection: Selection
#%end

#%option G_OPT_F_SEP
#% label: Field separator character between the output columns
#% guisection: Formatting
#%end

#%flag
#% key: e
#% description: Calculate extended statistics
#%end

#%flag
#% key: r
#% description: Ignore the current region settings and use the raster map regions for univar statistical calculation
#%end

#%flag
#% key: s
#% description: Suppress printing of column names
#% guisection: Formatting
#%end

import grass.script as grass
import grass.temporal as tgis

############################################################################


def main():

    # Get the options
    input = options["input"]
    output = options["output"]
    where = options["where"]
    extended = flags["e"]
    no_header = flags["s"]
    rast_region = bool(flags["r"])
    separator = grass.separator(options["separator"])

    # Make sure the temporal database exists
    tgis.init()

    if not output:
        output = None
    if output == "-":
        output = None

    tgis.print_gridded_dataset_univar_statistics(
        "strds", input, output, where, extended, no_header, separator, rast_region)

if __name__ == "__main__":
    options, flags = grass.parser()
    main()
