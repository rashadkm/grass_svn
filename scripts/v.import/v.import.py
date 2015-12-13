#!/usr/bin/env python

############################################################################
#
# MODULE:       v.import
#
# AUTHOR(S):    Markus Metz
#
# PURPOSE:      Import and reproject vector data on the fly
#
# COPYRIGHT:    (C) 2015 by GRASS development team
#
#               This program is free software under the GNU General
#               Public License (>=v2). Read the file COPYING that
#               comes with GRASS for details.
#
#############################################################################

#%module
#% description: Imports vector data into a GRASS vector map using OGR library and reprojects on the fly.
#% keyword: vector
#% keyword: import
#% keyword: projection
#%end
#%option
#% key: input
#% type: string
#% required: yes
#% description: Name of OGR datasource to be imported
#% gisprompt: old,datasource,datasource
#% guisection: Input
#%end
#%option
#% key: layer
#% type: string
#% multiple: yes
#% description: OGR layer name. If not given, all available layers are imported
#% guisection: Input
#% gisprompt: old,datasource_layer,datasource_layer
#%end
#%option G_OPT_V_OUTPUT
#% description: Name for output vector map (default: input)
#% required: no
#% guisection: Output
#%end
#%option
#% key: extent
#% type: string
#% options: input,region
#% answer: input
#% description: Output vector map extent
#% descriptions: input;extent of input map;region;extent of current region
#% guisection: Output
#%end
#%option
#% key: encoding
#% type: string
#% label: Encoding value for attribute data
#% descriptions: Overrides encoding interpretation, useful when importing ESRI Shapefile
#% guisection: Output
#%end
#%option
#% key: snap
#% type: double
#% label: Snapping threshold for boundaries (map units)
#% description: '-1' for no snap
#% answer: 1e-13
#% guisection: Output
#%end
#%option
#% key: epsg
#% type: integer
#% options: 1-1000000
#% guisection: Input SRS
#% description: EPSG projection code
#%end
#%option
#% key: datum_trans
#% type: integer
#% options: -1-100
#% guisection: Input SRS
#% label: Index number of datum transform parameters
#% description: -1 to list available datum transform parameters
#%end
#%flag
#% key: f
#% description: List supported OGR formats and exit
#% suppress_required: yes
#%end
#%flag
#% key: l
#% description: List available OGR layers in data source and exit
#%end


import sys
import os
import atexit

import grass.script as grass
from grass.exceptions import CalledModuleError

# initialize global vars
TMPLOC = None
SRCGISRC = None
GISDBASE = None


def cleanup():
    # remove temp location
    if TMPLOC:
        grass.try_rmdir(os.path.join(GISDBASE, TMPLOC))
    if SRCGISRC:
        grass.try_remove(SRCGISRC)


def main():
    global TMPLOC, SRCGISRC, GISDBASE

    # list formats and exit
    if flags['f']:
        grass.run_command('v.in.ogr', flags='f')
        return 0

    # list layers and exit
    if flags['l']:
        try:
            grass.run_command('v.in.ogr', flags='l', input=options['input'])
        except CalledModuleError:
            return 1
        return 0

    OGRdatasource = options['input']
    output = options['output']
    layers = options['layer']

    vflags = None
    if options['extent'] == 'region':
        vflags = 'r'
    vopts = {}
    if options['encoding']:
        vopts['encoding'] = options['encoding']

    if options['datum_trans'] and options['datum_trans'] == '-1':
        # list datum transform parameters
        if not options['epsg']:
            grass.fatal(_("Missing value for parameter <%s>") % 'epsg')
        
        return grass.run_command('g.proj', epsg=options['epsg'],
                                 datum_trans=options['datum_trans'])
    
    grassenv = grass.gisenv()
    tgtloc = grassenv['LOCATION_NAME']
    tgtmapset = grassenv['MAPSET']
    GISDBASE = grassenv['GISDBASE']
    tgtgisrc = os.environ['GISRC']
    SRCGISRC = grass.tempfile()

    TMPLOC = 'temp_import_location_' + str(os.getpid())

    f = open(SRCGISRC, 'w')
    f.write('MAPSET: PERMANENT\n')
    f.write('GISDBASE: %s\n' % GISDBASE)
    f.write('LOCATION_NAME: %s\n' % TMPLOC)
    f.write('GUI: text\n')
    f.close()

    tgtsrs = grass.read_command('g.proj', flags='j', quiet=True)

    # create temp location from input without import
    grass.verbose(_("Creating temporary location for <%s>...") % OGRdatasource)
    if layers:
        vopts['layer'] = layers
    if output:
        vopts['output'] = output
    vopts['snap'] = options['snap']
    try:
        grass.run_command('v.in.ogr', input=OGRdatasource,
                          location=TMPLOC, flags='i', quiet=True, **vopts)
    except CalledModuleError:
        grass.fatal(_("Unable to create location from OGR datasource <%s>") % OGRdatasource)

    # switch to temp location
    os.environ['GISRC'] = str(SRCGISRC)

    if options['epsg']: # force given EPSG
        kwargs = {}
        if options['datum_trans']:
            kwargs['datum_trans'] = options['datum_trans']
        grass.run_command('g.proj', flags='c', epsg=options['epsg'], **kwargs)
        
    # switch to target location
    os.environ['GISRC'] = str(tgtgisrc)

    # try v.in.ogr directly
    if grass.run_command('v.in.ogr', input=OGRdatasource, flags='j',
                         errors='status', quiet=True) == 0:
        try:
            grass.run_command('v.in.ogr', input=OGRdatasource,
                              flags=vflags, **vopts)
            grass.message(_("Input <%s> successfully imported without reprojection") % OGRdatasource)
            return 0
        except CalledModuleError:
            grass.fatal(_("Unable to import <%s>") % OGRdatasource)

    # make sure target is not xy
    if grass.parse_command('g.proj', flags='g')['name'] == 'xy_location_unprojected':
        grass.fatal(_("Coordinate reference system not available for current location <%s>") % tgtloc)

    # switch to temp location
    os.environ['GISRC'] = str(SRCGISRC)

    # print projection at verbose level
    grass.verbose(grass.read_command('g.proj', flags='p').rstrip(os.linesep))
    
    # make sure input is not xy
    if grass.parse_command('g.proj', flags='g')['name'] == 'xy_location_unprojected':
        grass.fatal(_("Coordinate reference system not available for input <%s>") % OGRdatasource)

    if options['extent'] == 'region':
        # switch to target location
        os.environ['GISRC'] = str(tgtgisrc)

        # v.in.region in tgt
        vreg = 'vreg_' + str(os.getpid())
        grass.run_command('v.in.region', output=vreg, quiet=True)

        # reproject to src
        # switch to temp location
        os.environ['GISRC'] = str(SRCGISRC)
        try:
            grass.run_command('v.proj', input=vreg, output=vreg,
                              location=tgtloc, mapset=tgtmapset, quiet=True)
        except CalledModuleError:
            grass.fatal(_("Unable to reproject to source location"))

        # set region from region vector
        grass.run_command('g.region', res='1')
        grass.run_command('g.region', vector=vreg)

    # import into temp location
    grass.message(_("Importing <%s> ...") % OGRdatasource)
    try:
        grass.run_command('v.in.ogr', input=OGRdatasource,
                          flags=vflags, **vopts)
    except CalledModuleError:
        grass.fatal(_("Unable to import OGR datasource <%s>") % OGRdatasource)

    # if output is not define check source mapset
    if not output:
        output = grass.list_grouped('vector')['PERMANENT'][0]

    # switch to target location
    os.environ['GISRC'] = str(tgtgisrc)

    # check if map exists
    if not grass.overwrite() and \
       grass.find_file(output, element='vector', mapset='.')['mapset']:
        grass.fatal(_("option <%s>: <%s> exists.") % ('output', output))

    if options['extent'] == 'region':
        grass.run_command('g.remove', type='vector', name=vreg,
                          flags='f', quiet=True)

    # v.proj
    grass.message(_("Reprojecting <%s>...") % output)
    try:
        grass.run_command('v.proj', location=TMPLOC,
                          mapset='PERMANENT', input=output)
    except CalledModuleError:
        grass.fatal(_("Unable to to reproject vector <%s>") % output)

    return 0

if __name__ == "__main__":
    options, flags = grass.parser()
    atexit.register(cleanup)
    sys.exit(main())