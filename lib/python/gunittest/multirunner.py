# -*- coding: utf-8 -*-

from __future__ import print_function

import sys
import os
import argparse
import itertools
import subprocess


def main():
    parser = argparse.ArgumentParser(
        description='Run tests with new')
    parser.add_argument('--location', '-l', required=True, action='append',
                        dest='locations', metavar='LOCATION',
                        help='Directories with reports')
    parser.add_argument('--location-type', '-t', action='append',
                        dest='location_types',
                    default=[], metavar='TYPE',
                    help='Add repeated values to a list',
                    )
    parser.add_argument('--grassbin', required=True,
                        help='Use file timestamp instead of date in test summary')
    # TODO: rename since every src can be used?
    parser.add_argument('--grasssrc', required=True,
                        help='GRASS GIS source code (to take tests from)')
    parser.add_argument('--grassdata', required=True,
                        help='GRASS GIS data base (GISDBASE)')
    parser.add_argument('--create-main-report',
                        help='Create also main report for all tests',
                        action="store_true", default=False, dest='main_report')

    args = parser.parse_args()
    gisdb = args.grassdata
    locations = args.locations
    locations_types = args.location_types

    # TODO: if locations empty or just one we can suppose the same all the time
    if len(locations) != len(locations_types):
        print("ERROR: Number of locations and their tags must be the same", file=sys.stderr)
        return 1
    

    main_report = args.main_report
    grasssrc = args.grasssrc  # TODO: can be guessed from dist    
    # TODO: create directory accoring to date and revision and create reports there

    # some predefined variables, name of the GRASS launch script + location/mapset
    #grass7bin = 'C:\Program Files (x86)\GRASS GIS 7.0.svn\grass70svn.bat'
    grass7bin = args.grassbin  # TODO: can be used if pressent

    ########### SOFTWARE
    # query GRASS 7 itself for its GISBASE
    # we assume that GRASS GIS' start script is available and in the PATH
    # the shell=True is here because of MS Windows? (code taken from wiki)
    startcmd = grass7bin + ' --config path'
    p = subprocess.Popen(startcmd, shell=True, 
                         stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = p.communicate()
    if p.returncode != 0:
        print("ERROR: Cannot find GRASS GIS 7 start script (%s):\n%s" % (startcmd, err), file=sys.stderr)
        return 1
    gisbase = out.strip('\n')

    # set GISBASE environment variable
    os.environ['GISBASE'] = gisbase
    # define GRASS Python environment
    grass_python_dir = os.path.join(gisbase, "etc", "python")
    sys.path.append(grass_python_dir)

    ########### DATA
    # define GRASS DATABASE
    
    # Set GISDBASE environment variable
    os.environ['GISDBASE'] = gisdb

    # import GRASS Python package for initialization
    import grass.script.setup as gsetup

    # launch session
    # we need some location and mapset here
    # TODO: can init work without it or is there some demo location in dist?
    location = locations[0].split(':')[0]
    mapset = 'PERMANENT'
    gsetup.init(gisbase, gisdb, location, mapset)

    reports = []
    for location, location_type in itertools.izip(locations, locations_types):
        # here it is quite a good place to parallelize
        # including also type to make it unique and preserve it for sure
        report = 'report_for_' + location + '_' + location_type
        absreport = os.path.abspath(report)
        p = subprocess.Popen([sys.executable, '-tt',
                              '-m', 'grass.gunittest.main',
                              '--grassdata', gisdb, '--location', location,
                              '--location-type', location_type,
                              '--output', absreport],
                              cwd=grasssrc)
        returncode = p.wait()
        reports.append(report)

    if main_report:
        # TODO: solve the path to source code (work now only for grass source code)
        arguments = [sys.executable, grasssrc + '/lib/python/guittest/' + 'multireport.py', '--timestapms']
        arguments.extend(reports)
        p = subprocess.Popen(arguments)
        returncode = p.wait()
        if returncode != 0:
            print("ERROR: Creation of main report failed.", file=sys.stderr)
            return 1

    return 0

if __name__ == '__main__':
    sys.exit(main())
