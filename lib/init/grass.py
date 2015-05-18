#!/usr/bin/env python
#############################################################################
#
# MODULE:       GRASS initialization script
# AUTHOR(S):    Original author unknown - probably CERL
#               Andreas Lange <andreas.lange rhein-main.de>
#               Huidae Cho <grass4u gmail.com>
#               Justin Hickey <jhickey hpcc.nectec.or.th>
#               Markus Neteler <neteler osgeo.org>
#               Hamish Bowman <hamish_b yahoo.com>
#
#               GRASS 7: converted to Python (based on init.sh shell
#               script from GRASS 6) by Glynn Clements
#               Martin Landa <landa.martin gmail.com>
#               Luca Delucchi <lucadeluge gmail.com>
# PURPOSE:      Sets up environment variables, parses any remaining
#               command line options for setting the GISDBASE, LOCATION,
#               and/or MAPSET. Finally it starts GRASS with the appropriate
#               user interface and cleans up after it is finished.
# COPYRIGHT:    (C) 2000-2015 by the GRASS Development Team
#
#               This program is free software under the GNU General
#               Public License (>=v2). Read the file COPYING that
#               comes with GRASS for details.
#
#############################################################################

import sys
import os
import atexit
import string
import subprocess
import re
import platform
import tempfile

# Variables substituted during build process
if 'GISBASE' in os.environ:
    gisbase = os.environ['GISBASE']
else:
    gisbase = "@GISBASE@"
cmd_name = "@START_UP@"
grass_version = "@GRASS_VERSION_NUMBER@"
ld_library_path_var = '@LD_LIBRARY_PATH_VAR@'
if 'GRASS_PROJSHARE' in os.environ:
    config_projshare = os.environ['GRASS_PROJSHARE']
else:
    config_projshare = "@CONFIG_PROJSHARE@"

# configuration directory
grass_env_file = None  # see check_shell()
if sys.platform == 'win32':
    grass_config_dirname = "GRASS7"
    grass_config_dir = os.path.join(os.getenv('APPDATA'), grass_config_dirname)
else:
    grass_config_dirname = ".grass7"
    grass_config_dir = os.path.join(os.getenv('HOME'), grass_config_dirname)

gisbase = os.path.normpath(gisbase)

# i18N
import gettext
gettext.install('grasslibs', os.path.join(gisbase, 'locale'))

tmpdir = None
lockfile = None
remove_lockfile = True
location = None
grass_gui = None


def warning(text):
    sys.stderr.write(_("WARNING") + ': ' + text + os.linesep)


def try_remove(path):
    try:
        os.remove(path)
    except:
        pass


def try_rmdir(path):
    try:
        os.rmdir(path)
    except:
        pass


def clean_env(gisrc):
    env_curr = read_gisrc(gisrc)
    env_new = {}
    for k,v in env_curr.iteritems():
        if 'MONITOR' not in k:
            env_new[k] = v

    write_gisrc(env_new, gisrc)


def cleanup_dir(path):
    if not path:
        return

    for root, dirs, files in os.walk(path, topdown=False):
        for name in files:
            try_remove(os.path.join(root, name))
        for name in dirs:
            try_rmdir(os.path.join(root, name))


def cleanup(tmpdir):
    global lockfile, remove_lockfile
    # all exits after setting up tmp dirs (system/location) should
    # also tidy it up
    cleanup_dir(tmpdir)
    try_rmdir(tmpdir)
    if location:
        tmpdir_loc = os.path.join(location, ".tmp")
        cleanup_dir(tmpdir_loc)
        try_rmdir(tmpdir_loc)

    # remove lock-file if requested
    if lockfile and remove_lockfile:
        try_remove(lockfile)


def fatal(msg):
    sys.stderr.write("%s: " % _('ERROR') + msg + os.linesep)
    sys.exit(_("Exiting..."))


def message(msg):
    sys.stderr.write(msg + "\n")
    sys.stderr.flush()


# mechanism meant for debugging this script (only)
# private global to store if we are debugging
_DEBUG = None


def is_debug():
    """Returns True if we are in debug mode

    For debug messages use ``debug()``.
    """
    global _DEBUG
    if _DEBUG is not None:
        return _DEBUG
    _DEBUG = os.getenv('GRASS_DEBUG')
    # translate to bool (no or empty variable means false)
    if _DEBUG:
        _DEBUG = True
    else:
        _DEBUG = False
    return _DEBUG


def debug(msg):
    """Print a debug message if in debug mode

    Do not use translatable strings for debug messages.
    """
    if is_debug():
        sys.stderr.write("DEBUG: %s\n" % msg)
        sys.stderr.flush()


def readfile(path):
    f = open(path, 'r')
    s = f.read()
    f.close()
    return s


def writefile(path, s):
    f = open(path, 'w')
    f.write(s)
    f.close()


def call(cmd, **kwargs):
    if windows:
        kwargs['shell'] = True
    return subprocess.call(cmd, **kwargs)


def Popen(cmd, **kwargs):
    if windows:
        kwargs['shell'] = True
    return subprocess.Popen(cmd, **kwargs)


def gpath(*args):
    """Costruct path to file or directory in GRASS GIS installation

    Can be called only after gisbase was set.
    """
    return os.path.join(gisbase, *args)


# for wxpath
_WXPYTHON_BASE = None


def wxpath(*args):
    """Costruct path to file or directory in GRASS wxGUI

    Can be called only after gisbase was set.

    This function does not check if the directories exist or if GUI works
    this must be done by the caller if needed.
    """
    global _WXPYTHON_BASE
    if not _WXPYTHON_BASE:
        # this can be called only after gisbase was set
        _WXPYTHON_BASE = gpath("gui", "wxpython")
    return os.path.join(_WXPYTHON_BASE, *args)


# using format for most but leaving usage of template for the dynamic ones
# two different methods are easy way to implement two phase construction
help_text = r"""GRASS GIS $VERSION_NUMBER
Geographic Resources Analysis Support System (GRASS GIS).

{usage}:
  $CMD_NAME [-h | -help | --help | --h] [-v | --version]
          [-c | -c geofile | -c EPSG:code[:datum_trans]]
          [-e] [-f] [-text | -gtext | -gui] [--config param]
          [[[GISDBASE/]LOCATION_NAME/]MAPSET]
  $CMD_NAME [FLAG]... GISDBASE/LOCATION_NAME/MAPSET exec MODULE [PARAM]...

{flags}:
  -h or -help or --help or --h   {help_flag}
  -v or --version                {version_flag}
  -c                             {create}
  -e                             {exit_after}
  -f                             {force_removal}
  -text                          {text}
                                   {text_detail}
  -gtext                         {gtext}
                                   {gtext_detail}
  -gui                           {gui}
                                   {gui_detail}
  --config                       {config}
                                   {config_detail}
  exec MODULE                    {exec_}
                                   {exec_detail}

{params}:
  GISDBASE                       {gisdbase}
                                   {gisdbase_detail}
  LOCATION_NAME                  {location}
                                   {location_detail}
  MAPSET                         {mapset}

  GISDBASE/LOCATION_NAME/MAPSET  {full_mapset}

{env_vars}:
  GRASS_GUI                      {gui_var}
  GRASS_HTML_BROWSER             {html_var}
  GRASS_ADDON_PATH               {addon_path_var}
  GRASS_ADDON_BASE               {addon_base_var}
  GRASS_BATCH_JOB                {batch_var}
  GRASS_PYTHON                   {python_var}
""".format(
    usage=_("Usage"),
    flags=_("Flags"),
    help_flag=_("print this help message"),
    version_flag=_("show version information and exit"),
    create=_("create given database, location or mapset if it doesn't exist"),
    exit_after=_("exit after creation of location or mapset. Only with -c flag"),
    force_removal=_("force removal of .gislock if exists (use with care!). Only with -text flag"),
    text=_("use text based interface (skip welcome screen)"),
    text_detail=_("and set as default"),
    gtext=_("use text based interface (show welcome screen)"),
    gtext_detail=_("and set as default"),
    gui=_("use $DEFAULT_GUI graphical user interface"),
    gui_detail=_("and set as default"),
    config=_("print GRASS configuration parameters"),
    config_detail=_("options: arch,build,compiler,path,revision"),
    params=_("Parameters"),
    gisdbase=_("initial GRASS GIS database directory"),
    gisdbase_detail=_("directory containing Locations"),
    location=_("initial GRASS Location"),
    location_detail=_("directory containing Mapsets with one common coordinate system (projection)"),
    mapset=_("initial GRASS Mapset"),
    full_mapset=_("fully qualified initial Mapset directory"),
    env_vars=_("Environment variables relevant for startup"),
    gui_var=_("select GUI (text, gui, gtext)"),
    html_var=_("set html web browser for help pages"),
    addon_path_var=_("set additional path(s) to local GRASS modules or user scripts"),
    addon_base_var=_("set additional GISBASE for locally installed GRASS Addons"),
    batch_var=_("shell script to be processed as batch job"),
    python_var=_("set python shell name to override 'python'"),
    exec_=_("execute GRASS module or script"),
    exec_detail=_("any executable will be executed in GRASS session"),
    )


def help_message():
    t = string.Template(help_text)
    s = t.substitute(CMD_NAME=cmd_name, DEFAULT_GUI=default_gui,
                     VERSION_NUMBER=grass_version)
    sys.stderr.write(s)


def create_tmp(user, gis_lock):
    """Create temporary directory

    :param user: user name to be used in the directory name
    :param gis_lock: session lock filename to be used in the directory name
    """
    # use $TMPDIR if it exists, then $TEMP, otherwise /tmp
    tmp = os.getenv('TMPDIR')
    if not tmp:
        tmp = os.getenv('TEMP')
    if not tmp:
        tmp = os.getenv('TMP')
    if not tmp:
        tmp = tempfile.gettempdir()

    if tmp:
        tmpdir = os.path.join(tmp, "grass7-%(user)s-%(lock)s" % {'user': user,
                                                             'lock': gis_lock})
        try:
            os.mkdir(tmpdir, 0700)
        except:
            tmp = None

    if not tmp:
        for ttmp in ("/tmp", "/var/tmp", "/usr/tmp"):
            tmp = ttmp
            tmpdir = os.path.join(tmp, "grass7-%(user)s-%(lock)s" % {
                                              'user': user, 'lock': gis_lock})
            try:
                os.mkdir(tmpdir, 0700)
            except:
                tmp = None
            if tmp:
                break

    if not tmp:
        fatal(_("Unable to create temporary directory <grass7-%(user)s-"
                "%(lock)s>! Exiting.") % {'user': user, 'lock': gis_lock})

    # promoting the variable even if it was not defined before
    os.environ['TMPDIR'] = tmpdir

    return tmpdir


def create_gisrc(tmpdir, gisrcrc):
    # Set the session grassrc file
    gisrc = os.path.join(tmpdir, "gisrc")
    os.environ['GISRC'] = gisrc

    # remove invalid GISRC file to avoid disturbing error messages:
    try:
        s = readfile(gisrcrc)
        if "UNKNOWN" in s:
            try_remove(gisrcrc)
            s = None
    except:
        s = None

    # Copy the global grassrc file to the session grassrc file
    if s:
        writefile(gisrc, s)
    return gisrc


def read_gisrc(filename):
    kv = {}
    try:
        f = open(filename, 'r')
    except IOError:
        return kv

    for line in f:
        try:
            k, v = line.split(':', 1)
        except ValueError as e:
            warning(_("Invalid line in RC file ({file}):"
                      " '{line}' ({error})\n").format(
                          line=line, error=e, file=filename))
            continue
        kv[k.strip()] = v.strip()
    f.close()

    return kv


def read_env_file(path):
    kv = {}
    f = open(path, 'r')
    for line in f:
        k, v = line.split(':', 1)
        kv[k.strip()] = v.strip()
    f.close()
    return kv


def write_gisrc(kv, filename):
    f = open(filename, 'w')
    for k, v in kv.iteritems():
        f.write("%s: %s\n" % (k, v))
    f.close()


def read_gui(default_gui):
    grass_gui = None
    # At this point the GRASS user interface variable has been set from the
    # command line, been set from an external environment variable, or is 
    # not set. So we check if it is not set
    # Check for a reference to the GRASS user interface in the grassrc file
    if os.access(gisrc, os.R_OK):
        kv = read_gisrc(gisrc)
        if 'GRASS_GUI' in os.environ:
            grass_gui = os.environ['GRASS_GUI']
        elif 'GUI' in kv:
            grass_gui = kv['GUI']
        elif 'GRASS_GUI' in kv:
            # For backward compatibility (GRASS_GUI renamed to GUI)
            grass_gui = kv['GRASS_GUI']
        else:
            # Set the GRASS user interface to the default if needed
            grass_gui = default_gui

    if not grass_gui:
        grass_gui = default_gui

    if grass_gui == 'gui':
        grass_gui = default_gui

    # FIXME oldtcltk, gis.m, d.m no longer exist (remove this around 7.2)
    if grass_gui in ['d.m', 'gis.m', 'oldtcltk', 'tcltk']:
        warning(_("GUI <%s> not supported in this version") % grass_gui)
        grass_gui = default_gui

    return grass_gui


def path_prepend(dir, var):
    path = os.getenv(var)
    if path:
        path = dir + os.pathsep + path
    else:
        path = dir
    os.environ[var] = path


def path_append(dir, var):
    path = os.getenv(var)
    if path:
        path = path + os.pathsep + dir
    else:
        path = dir
    os.environ[var] = path


def set_paths():
    # addons (path)
    addon_path = os.getenv('GRASS_ADDON_PATH')
    if addon_path:
        for path in addon_path.split(os.pathsep):
            path_prepend(addon_path, 'PATH')

    # addons (base)
    addon_base = os.getenv('GRASS_ADDON_BASE')
    if not addon_base:
        addon_base = os.path.join(grass_config_dir, 'addons')
        os.environ['GRASS_ADDON_BASE'] = addon_base
    if not windows:
        path_prepend(os.path.join(addon_base, 'scripts'), 'PATH')
    path_prepend(os.path.join(addon_base, 'bin'), 'PATH')
    
    # standard installation
    if not windows:
        path_prepend(gpath('scripts'), 'PATH')
    path_prepend(gpath('bin'), 'PATH')

    # Set PYTHONPATH to find GRASS Python modules
    if os.path.exists(gpath('gui', 'wxpython')):
        path_prepend(gpath('gui', 'wxpython'), 'PYTHONPATH')
    if os.path.exists(gpath('etc', 'python')):
        path_prepend(gpath('etc', 'python'), 'PYTHONPATH')

    # set path for the GRASS man pages
    grass_man_path = gpath('docs', 'man')
    addons_man_path = os.path.join(addon_base, 'docs', 'man')
    man_path = os.getenv('MANPATH')
    sys_man_path = None
    if man_path:
        path_prepend(addons_man_path, 'MANPATH')
        path_prepend(grass_man_path, 'MANPATH')
    else:
        try:
            nul = open(os.devnull, 'w')
            p = Popen(['manpath'], stdout=subprocess.PIPE, stderr=nul)
            nul.close()
            s = p.stdout.read()
            p.wait()
            sys_man_path = s.strip()
        except:
            pass

        if sys_man_path:
            os.environ['MANPATH'] = sys_man_path
            path_prepend(addons_man_path, 'MANPATH')
            path_prepend(grass_man_path, 'MANPATH')
        else:
            os.environ['MANPATH'] = addons_man_path
            path_prepend(grass_man_path, 'MANPATH')


def find_exe(pgm):
    for dir in os.getenv('PATH').split(os.pathsep):
        path = os.path.join(dir, pgm)
        if os.access(path, os.X_OK):
            return path
    return None


def set_defaults():
    # GRASS_PAGER
    if not os.getenv('GRASS_PAGER'):
        if find_exe("more"):
            pager = "more"
        elif find_exe("less"):
            pager = "less"
        elif windows:
            pager = "more"
        else:
            pager = "cat"
        os.environ['GRASS_PAGER'] = pager

    # GRASS_PYTHON
    if not os.getenv('GRASS_PYTHON'):
        if windows:
            os.environ['GRASS_PYTHON'] = "python.exe"
        else:
            os.environ['GRASS_PYTHON'] = "python"

    # GRASS_GNUPLOT
    if not os.getenv('GRASS_GNUPLOT'):
        os.environ['GRASS_GNUPLOT'] = "gnuplot -persist"

    # GRASS_PROJSHARE
    if not os.getenv('GRASS_PROJSHARE'):
        os.environ['GRASS_PROJSHARE'] = config_projshare

    
def set_browser():
    # GRASS_HTML_BROWSER
    browser = os.getenv('GRASS_HTML_BROWSER')
    if not browser:
        if macosx:
            # OSX doesn't execute browsers from the shell PATH - route thru a
            # script
            browser = gpath('etc', "html_browser_mac.sh")
            os.environ['GRASS_HTML_BROWSER_MACOSX'] = "-b com.apple.helpviewer"

        if windows:
            browser = "start"
        elif cygwin:
            browser = "explorer"
        else:
            # the usual suspects
            browsers = ["xdg-open", "x-www-browser", "htmlview", "konqueror", "mozilla",
                        "mozilla-firefox", "firefox", "iceweasel", "opera",
                        "netscape", "dillo", "lynx", "links", "w3c"]
            for b in browsers:
                if find_exe(b):
                    browser = b
                    break

    elif macosx:
        # OSX doesn't execute browsers from the shell PATH - route thru a
        # script
        os.environ['GRASS_HTML_BROWSER_MACOSX'] = "-b %s" % browser
        browser = gpath('etc', "html_browser_mac.sh")

    if not browser:
        warning(_("Searched for a web browser, but none found"))
        # even so we set konqueror to make lib/gis/parser.c happy:
        # TODO: perhaps something more probable would be better, e.g. xdg-open
        browser = "konqueror"

    os.environ['GRASS_HTML_BROWSER'] = browser


def create_initial_gisrc(filename):
    # for convenience, define GISDBASE as pwd:
    s = r"""GISDBASE: %s
LOCATION_NAME: <UNKNOWN>
MAPSET: <UNKNOWN>
""" % os.getcwd()
    writefile(filename, s)


def check_gui(expected_gui):
    grass_gui = expected_gui
    # Check if we are running X windows by checking the DISPLAY variable
    if os.getenv('DISPLAY') or windows or macosx:
        # Check if python is working properly
        if expected_gui in ('wxpython', 'gtext'):
            nul = open(os.devnull, 'w')
            p = Popen([os.environ['GRASS_PYTHON']], stdin=subprocess.PIPE,
                      stdout=nul, stderr=nul)
            nul.close()
            p.stdin.write("variable=True")
            p.stdin.close()
            p.wait()
            if p.returncode != 0:
                # Python was not found - switch to text interface mode
                warning(_("The python command does not work as expected!\n"
                          "Please check your GRASS_PYTHON environment variable.\n"
                          "Use the -help option for details.\n"
                          "Switching to text based interface mode.\n\n"
                          "Hit RETURN to continue.\n"))
                sys.stdin.readline()
                grass_gui = 'text'

    else:
        # Display a message if a graphical interface was expected
        if expected_gui != 'text':
            # Set the interface mode to text
            warning(_("It appears that the X Windows system is not active.\n"
                      "A graphical based user interface is not supported.\n"
                      "(DISPLAY variable is not set.)\n"
                      "Switching to text based interface mode.\n\n"
                      "Hit RETURN to continue.\n"))
            sys.stdin.readline()
            grass_gui = 'text'
    return grass_gui


def save_gui(gisrc, grass_gui):
    # Save the user interface variable in the grassrc file - choose a temporary
    # file name that should not match another file
    if os.access(gisrc, os.F_OK):
        kv = read_gisrc(gisrc)
        kv['GUI'] = grass_gui
        write_gisrc(kv, gisrc)


def create_location(gisdbase, location, geostring):
    """Create GRASS Location using georeferenced file or EPSG

    EPSG code format is ``EPSG:code`` or ``EPSG:code:datum_trans``.

    :param gisdbase: Path to GRASS GIS database directory
    :param location: name of new Location
    :param geostring: path to a georeferenced file or EPSG code
    """
    if gpath('etc', 'python') not in sys.path:
        sys.path.append(gpath('etc', 'python'))
    from grass.script import core as gcore  # pylint: disable=E0611

    try:
        if geostring and geostring.upper().find('EPSG:') > -1:
            # create location using EPSG code
            epsg = geostring.split(':', 1)[1]
            if ':' in epsg:
                epsg, datum_trans = epsg.split(':', 1)
            else:
                datum_trans = None
            gcore.create_location(gisdbase, location,
                                  epsg=epsg, datum_trans=datum_trans)
        else:
            # create location using georeferenced file
            gcore.create_location(gisdbase, location,
                                  filename=geostring)
    except gcore.ScriptError as e:
        fatal(e.value.strip('"').strip("'").replace('\\n', os.linesep))


# interface created according to the current usage
def is_mapset_valid(full_mapset):
    """Return True if GRASS Mapset is valid"""
    return os.access(os.path.join(full_mapset, "WIND"), os.R_OK)


def is_location_valid(gisdbase, location):
    """Return True if GRASS Location is valid

    :param gisdbase: Path to GRASS GIS database directory
    :param location: name of a Location
    """
    return os.access(os.path.join(gisdbase, location,
                     "PERMANENT", "DEFAULT_WIND"), os.F_OK)


# basically checking location, possibly split into two functions
# (mapset one can call location one)
def get_mapset_invalid_reason(gisdbase, location, mapset):
    """Returns a message describing what is wrong with the Mapset

    :param gisdbase: Path to GRASS GIS database directory
    :param location: name of a Location
    :param mapset: name of a Mapset
    :returns: translated message
    """
    full_location = os.path.join(gisdbase, location)
    if not os.path.exists(full_location):
        return _("Location <%s> doesn't exist") % full_location
    elif 'PERMANENT' not in os.listdir(full_location):
        return _("<%s> is not a valid GRASS Location"
                 " because PERMANENT Mapset is missing") % full_location
    elif not os.path.isdir(os.path.join(full_location, 'PERMANENT')):
        return _("<%s> is not a valid GRASS Location"
                 " because PERMANENT is not a directory") % full_location
    elif not os.path.isfile((os.path.join(full_location,
                                          'PERMANENT', 'DEFAULT_WIND'))):
        return _("<%s> is not a valid GRASS Location"
                 " because PERMANENT Mapset does not have a DEFAULT_WIND file"
                 " (default computational region)") % full_location
    else:
        return _("Mapset <{mapset}> doesn't exist in GRASS Location <{loc}>. "
                 "A new mapset can be created by '-c' switch.").format(
                     mapset=mapset, loc=location)


def set_mapset(arg, geofile=None, create_new=False):
    """Selected Location and Mapset are checked and created if requested

    The gisrc (GRASS environment file) is written at the end.
    """
    global gisdbase, location_name, mapset, location
    # Try non-interactive startup
    l = None

    if arg == '-':
        if location:
            l = location
    else:
        l = arg

    if l:
        if l == '.':
            l = os.getcwd()
        elif not os.path.isabs(l):
            l = os.path.abspath(l)

        l, mapset = os.path.split(l)
        if not mapset:
            l, mapset = os.path.split(l)
        l, location_name = os.path.split(l)
        gisdbase = l
    
    if gisdbase and location_name and mapset:
        location = os.path.join(gisdbase, location_name, mapset)

        # check if 'location' is a valid GRASS location/mapset
        if not is_mapset_valid(location):
            if not create_new:
                # 'location' is not valid, check if 'location_name' is
                # a valid GRASS location
                fatal(get_mapset_invalid_reason(gisdbase, location_name, mapset))
            else:
                # 'location' is not valid, the user wants to create
                # mapset on the fly
                if not is_location_valid(gisdbase, location_name):
                    # 'location_name' is not a valid GRASS location,
                    # create new location and 'PERMANENT' mapset
                    gisdbase = os.path.join(gisdbase, location_name)
                    location_name = mapset
                    mapset = "PERMANENT"
                    if is_location_valid(gisdbase, location_name):
                        fatal(_("Failed to create new location. "
                                "The location <%s> already exists." % location_name))
                    create_location(gisdbase, location_name, geofile)
                else:
                    # 'location_name' is a valid GRASS location,
                    # create new mapset
                    os.mkdir(location)
                    # copy PERMANENT/DEFAULT_WIND to <mapset>/WIND
                    s = readfile(os.path.join(gisdbase, location_name,
                                              "PERMANENT", "DEFAULT_WIND"))
                    writefile(os.path.join(location, "WIND"), s)
                    message(_("Missing WIND file fixed"))

        if os.access(gisrc, os.R_OK):
            kv = read_gisrc(gisrc)
        else:
            kv = {}

        kv['GISDBASE'] = gisdbase
        kv['LOCATION_NAME'] = location_name
        kv['MAPSET'] = mapset
        write_gisrc(kv, gisrc)
    else:
        fatal(_("GISDBASE, LOCATION_NAME and MAPSET variables not set properly.\n"
                "Interactive startup needed."))


def set_mapset_interactive(grass_gui):
    """User selects Location and Mapset in an interative way

    The gisrc (GRASS environment file) is written at the end.
    """
    # Check for text interface
    if grass_gui == 'text':
        pass
    # Check for GUI
    elif grass_gui in ('gtext', 'wxpython'):
        gui_startup(grass_gui)
    else:
        # Shouldn't need this but you never know
        fatal(_("Invalid user interface specified - <%s>. " 
                "Use the --help option to see valid interface names.") % grass_gui)


def gui_startup(grass_gui):
    if grass_gui in ('wxpython', 'gtext'):
        ret = call([os.getenv('GRASS_PYTHON'), wxpath("gis_set.py")])

    if ret == 0:
        pass
    elif ret == 1:
        # The startup script printed an error message so wait
        # for user to read it
        message(_("Error in GUI startup. If necessary, please "
                  "report this error to the GRASS developers.\n"
                  "Switching to text mode now.\n\n"
                  "Hit RETURN to continue..."))
        sys.stdin.readline()

        os.execlp(cmd_name, "-text")
        sys.exit(1)
    elif ret == 2:
        # User wants to exit from GRASS
        message(_("Received EXIT message from GUI.\nGRASS is not started. Bye."))
        sys.exit(0)
    else:
        fatal(_("Invalid return code from GUI startup script.\n"
                "Please advise GRASS developers of this error."))


# we don't follow the LOCATION_NAME legacy naming here but we have to still
# translate to it, so always double check
class MapsetSettings(object):
    """Holds GRASS GIS database directory, Location and Mapset

    Provides few convenient functions.
    """
    def __init__(self):
        self.gisdbase = None
        self.location = None
        self.mapset = None
        self._full_mapset = None

    # TODO: perhaps full_mapset would be better as mapset_path
    # TODO: works only when set for the first time
    # this follows the current usage but we must invalidate when
    # the others are changed (use properties for that)
    @property
    def full_mapset(self):
        if self._full_mapset is None:
            self._full_mapset = os.path.join(self.gisdbase, self.location,
                                             self.mapset)
        return self._full_mapset

    def is_valid(self):
        return self.gisdbase and self.location and self.mapset


def load_gisrc(gisrc):
    """Get the settings of Location and Mapset from the gisrc file

    :returns: MapsetSettings object
    """
    mapset_settings = MapsetSettings()
    kv = read_gisrc(gisrc)
    mapset_settings.gisdbase = kv.get('GISDBASE')
    mapset_settings.location = kv.get('LOCATION_NAME')
    mapset_settings.mapset = kv.get('MAPSET')
    if not mapset_settings.is_valid():
        fatal(_("Error reading data path information from g.gisenv.\n"
                "GISDBASE={gisbase}\n"
                "LOCATION_NAME={location}\n"
                "MAPSET={mapset}\n\n"
                "Check the <{file}> file.").format(
                    gisbase=mapset_settings.gisdbase,
                    location=mapset_settings.location,
                    mapset=mapset_settings.mapset,
                    file=gisrcrc))
    return mapset_settings


# load environmental variables from grass_env_file
def load_env():
    if not os.access(grass_env_file, os.R_OK):
        return

    for line in readfile(grass_env_file).split(os.linesep):
        try:
            k, v = map(lambda x: x.strip(), line.strip().split(' ', 1)[1].split('=', 1))
        except:
            continue
        os.environ[k] = v
    # Allow for mixed ISIS-GRASS Environment
    if os.getenv('ISISROOT'):
        isis = os.getenv('ISISROOT')
        os.environ['ISIS_LIB'] = isis + os.sep + "lib"
        os.environ['ISIS_3RDPARTY'] = isis + os.sep + "3rdParty" + os.sep + "lib"
        os.environ['QT_PLUGIN_PATH'] = isis + os.sep + "3rdParty" + os.sep + "plugins"
        #os.environ['ISIS3DATA'] = isis + "$ISIS3DATA"
        libpath = os.getenv('LD_LIBRARY_PATH', '')
        isislibpath = os.getenv('ISIS_LIB')
        isis3rdparty = os.getenv('ISIS_3RDPARTY')
        os.environ['LD_LIBRARY_PATH'] = libpath + os.pathsep + isislibpath + os.pathsep + isis3rdparty


def set_language():
    # This function is used to override system default language and locale
    # Such override can be requested only from wxGUI
    # An override if user has provided correct environmental variables as
    # LC_MESSAGES or LANG is not necessary.
    # Unfortunately currently a working solution for Windows is lacking
    # thus it always on Vista and XP will print an error.
    # See discussion for Windows not following its own documentation and
    # not accepting ISO codes as valid locale identifiers http://bugs.python.org/issue10466
    import locale
    
    language = 'None' # Such string sometimes is present in wx file
    encoding = None
    
    # Override value is stored in wxGUI preferences file.
    # As it's the only thing required, we'll just grep it out.
    try:
        fd = open(os.path.join(grass_config_dir, 'wx'), 'r')
    except:
        # Even if there is no override, we still need to set locale.
        pass
    else:
        for line in fd:
            if re.search('^language', line):
                line = line.rstrip(' %s' % os.linesep)
                language = ''.join(line.split(';')[-1:])
                break
        fd.close()
    
    if language == 'None' or language == '' or not language:
        # Language override is disabled (system language specified)
        # As by default program runs with C locale, but users expect to
        # have their default locale, we'll just set default locale
        try:
            locale.setlocale(locale.LC_ALL, '')
        except locale.Error as e:
            # If we get here, system locale settings are terribly wrong
            # There is no point to continue as GRASS/Python will fail
            # in some other unpredictable way.
            print "System locale is not usable. It indicates misconfigured environment."
            print "Reported error message: %s" % e
            sys.exit("Fix system locale settings and then try again.")
        
        language, encoding = locale.getdefaultlocale()
        if not language:
            warning(_("Default locale settings are missing. GRASS running with C locale."))
            return
    
    else:
        debug("A language override has been requested."
              " Trying to switch GRASS into '%s'..." % language)
        try:
            locale.setlocale(locale.LC_ALL, language)
        except locale.Error as e:
            try:
                # Locale lang.encoding might be missing. Let's try
                # UTF-8 encoding before giving up as on Linux systems
                # lang.UTF-8 locales are more common than legacy
                # ISO-8859 ones.
                encoding = 'UTF-8'
                normalized = locale.normalize('%s.%s' % (language, encoding))
                locale.setlocale(locale.LC_ALL, normalized)
            except locale.Error as e:
                # The last attempt...
                try:
                    encoding = locale.getpreferredencoding()
                    normalized = locale.normalize('%s.%s' % (language, encoding))
                    locale.setlocale(locale.LC_ALL, normalized)
                except locale.Error as e:
                    # If we got so far, attempts to set up language and locale have failed
                    # on this system
                    sys.stderr.write("Failed to enforce user specified language '%s' with error: '%s'\n" % (language, e))
                    sys.stderr.write("A LANGUAGE environmental variable has been set.\nPart of messages will be displayed in the requested language.\n")
                    # Even if setting locale will fail, let's set LANG in a hope,
                    # that UI will use it GRASS texts will be in selected language,
                    # system messages (i.e. OK, Cancel etc.) - in system default
                    # language
                    os.environ['LANGUAGE'] = language
                    return
    
    # Set up environment for subprocesses
    os.environ['LANGUAGE'] = language
    os.environ['LANG'] = language
    if encoding:
        normalized = locale.normalize('%s.%s' % (language, encoding))
    else:
        normalized = language
    for lc in ('LC_CTYPE', 'LC_MESSAGES', 'LC_TIME', 'LC_COLLATE',
               'LC_MONETARY', 'LC_PAPER', 'LC_NAME', 'LC_ADDRESS',
               'LC_TELEPHONE', 'LC_MEASUREMENT', 'LC_IDENTIFICATION'):
        os.environ[lc] = normalized

    # Some code in GRASS might not like other decimal separators than .
    # Other potential sources for problems are: LC_TIME LC_CTYPE
    locale.setlocale(locale.LC_NUMERIC, 'C')
    os.environ['LC_NUMERIC'] = 'C'
    if os.getenv('LC_ALL'):
        del os.environ['LC_ALL']  # Remove LC_ALL to not override LC_NUMERIC
    
    # From now on enforce the new language
    if encoding: 
        gettext.install('grasslibs', gpath('locale'), codeset=encoding)
    else:
        gettext.install('grasslibs', gpath('locale'))

def check_lock(force_gislock_removal):
    global lockfile
    if not os.path.exists(location):
        fatal(_("Path '%s' doesn't exist") % location)

    # Check for concurrent use
    lockfile = os.path.join(location, ".gislock")
    ret = call([gpath("etc", "lock"), lockfile, "%d" % os.getpid()])
    msg = None
    if ret == 2:
        if not force_gislock_removal:
            msg = _("%(user)s is currently running GRASS in selected mapset (" \
                    "file %(file)s found). Concurrent use not allowed.\nYou can force launching GRASS using -f flag " \
                    "(note that you need permission for this operation). Have another look in the processor " \
                    "manager just to be sure..." % {
                        'user': user, 'file': lockfile})
        else:
            try_remove(lockfile)
            message(_("%(user)s is currently running GRASS in selected mapset (" \
                      "file %(file)s found). Forcing to launch GRASS..." % {
                          'user': user, 'file': lockfile}))
    elif ret != 0:
        msg = _("Unable to properly access '%s'.\n"
                "Please notify system personel.") % lockfile

    if msg:
        if grass_gui == "wxpython":
            call([os.getenv('GRASS_PYTHON'), wxpath("gis_set_error.py"), msg])
        else:
            global remove_lockfile
            remove_lockfile = False
            fatal(msg)


def make_fontcap():
    fc = os.getenv('GRASS_FONT_CAP')
    if fc and not os.access(fc, os.R_OK):
        message(_("Building user fontcap..."))
        call(["g.mkfontcap"])


def check_shell():
    global sh, shellname, grass_env_file
    # cygwin has many problems with the shell setup
    # below, so i hardcoded everything here.
    if sys.platform == 'cygwin':
        sh = "cygwin"
        shellname = "GNU Bash (Cygwin)"
        os.environ['SHELL'] = "/usr/bin/bash.exe"
        os.environ['OSTYPE'] = "cygwin"
    else:
        # in docker the 'SHELL' variable may not be
        # visible in a Python session
        try:
            sh = os.path.basename(os.getenv('SHELL'))
        except:
            sh = 'sh'
            os.environ['SHELL'] = sh
        
        if windows and sh:
            sh = os.path.splitext(sh)[0]
        
        if sh == "ksh":
            shellname = "Korn Shell"
        elif sh == "csh":
            shellname = "C Shell"
        elif sh == "tcsh":
            shellname = "TC Shell"
        elif sh == "bash":
            shellname = "Bash Shell"
        elif sh == "sh":
            shellname = "Bourne Shell"
        elif sh == "zsh":
            shellname = "Z Shell"
        elif sh == "cmd":
            shellname = "Command Shell"
        else:
            shellname = "shell"

    if sh in ['csh', 'tcsh']:
        grass_env_file = os.path.join(grass_config_dir, 'cshrc')
    elif sh in ['bash', 'msh', 'cygwin', 'sh']:
        grass_env_file = os.path.join(grass_config_dir, 'bashrc')
    elif sh == 'zsh':
        grass_env_file = os.path.join(grass_config_dir, 'zshrc')
    elif sh == 'cmd':
        grass_env_file = os.path.join(grass_config_dir, 'env.bat')
    else:
        grass_env_file = os.path.join(grass_config_dir, 'bashrc')
        warning(_("Unsupported shell <%(sh)s>: %(env)s") % {'sh': sh,
                                                       'env': grass_env_file})

    # check for SHELL
    if not os.getenv('SHELL'):
        fatal(_("The SHELL variable is not set"))


def get_batch_job():
    # hack to process batch jobs:
    return os.getenv('GRASS_BATCH_JOB')


def run_batch_job(batch_job):
    batch_job_string = batch_job
    if not isinstance(batch_job, basestring):
        batch_job_string = ' '.join(batch_job)
    message(_("Executing <%s> ...") % batch_job_string)
    if isinstance(batch_job, basestring):
        proc = Popen(batch_job, shell=True)
    else:
        try:
            proc = Popen(batch_job, shell=False)
        except OSError as error:
            fatal(_("Execution of <{cmd}> failed:\n"
                    "{error}").format(cmd=batch_job_string, error=error))
    returncode = proc.wait()
    message(_("Execution of <%s> finished.") % batch_job_string)
    return returncode


def start_gui(grass_gui):
    # Start the chosen GUI but ignore text
    debug("GRASS GUI should be <%s>" % grass_gui)

    # Check for gui interface
    if grass_gui == "wxpython":
        Popen([os.getenv('GRASS_PYTHON'), wxpath("wxgui.py")])


def clear_screen():
    global windows
    if windows:
        pass
    # TODO: uncomment when PDCurses works.
    #   cls
    else:
        if not is_debug():
            call(["tput", "clear"])


def show_banner():
    sys.stderr.write(r"""
          __________  ___   __________    _______________
         / ____/ __ \/   | / ___/ ___/   / ____/  _/ ___/
        / / __/ /_/ / /| | \__ \\_  \   / / __ / / \__ \ 
       / /_/ / _, _/ ___ |___/ /__/ /  / /_/ // / ___/ / 
       \____/_/ |_/_/  |_/____/____/   \____/___//____/  

""")


def say_hello():
    sys.stderr.write(_("Welcome to GRASS GIS %s") % grass_version)
    if grass_version.endswith('svn'):
        try:
            filerev = open(gpath('etc', 'VERSIONNUMBER'))
            linerev = filerev.readline().rstrip('\n')
            filerev.close()
            
            revision = linerev.split(' ')[1]
            sys.stderr.write(' (' + revision + ')')
        except:
            pass
    
def show_info():
    sys.stderr.write(
r"""
%-41shttp://grass.osgeo.org
%-41s%s (%s)
%-41sg.manual -i
%-41sg.version -c
""" % (_("GRASS GIS homepage:"),
       _("This version running through:"),
       shellname, os.getenv('SHELL'),
       _("Help is available with the command:"),
       _("See the licence terms with:")))
    
    if grass_gui == 'wxpython':
        message("%-41sg.gui wxpython" % _("If required, restart the GUI with:"))
    else:
        message("%-41sg.gui %s" % (_("Start the GUI with:"), default_gui))
    
    message("%-41sexit" % _("When ready to quit enter:"))
    message("")


def csh_startup():
    global exit_val

    userhome = os.getenv('HOME')      # save original home
    home = location
    os.environ['HOME'] = home

    cshrc = os.path.join(home, ".cshrc")
    tcshrc = os.path.join(home, ".tcshrc")
    try_remove(cshrc)
    try_remove(tcshrc)

    f = open(cshrc, 'w')
    f.write("set home = %s\n" % userhome)
    f.write("set history = 3000 savehist = 3000  noclobber ignoreeof\n")
    f.write("set histfile = %s\n" % os.path.join(os.getenv('HOME'),
                                                 ".history"))

    f.write("set prompt = '\\\n")
    f.write("Mapset <%s> in Location <%s> \\\n" % (mapset, location_name))
    f.write("GRASS GIS %s > '\n" % grass_version)
    f.write("set BOGUS=``;unset BOGUS\n")

    path = os.path.join(userhome, ".grass.cshrc") # left for backward compatibility
    if os.access(path, os.R_OK):
        f.write(readfile(path) + '\n')
    if os.access(grass_env_file, os.R_OK):
        f.write(readfile(grass_env_file) + '\n')

    mail_re = re.compile(r"^ *set  *mail *= *")

    for filename in [".cshrc", ".tcshrc", ".login"]:
        path = os.path.join(userhome, filename)
        if os.access(path, os.R_OK):
            s = readfile(path)
            lines = s.splitlines()
            for l in lines:
                if mail_re.match(l):
                    f.write(l)

    path = os.getenv('PATH').split(':')
    f.write("set path = ( %s ) \n" % ' '.join(path))

    f.close()
    writefile(tcshrc, readfile(cshrc))

    exit_val = call([gpath("etc", "run"), os.getenv('SHELL')])

    os.environ['HOME'] = userhome


def bash_startup():
    global exit_val

    # save command history in mapset dir and remember more
    os.environ['HISTFILE'] = os.path.join(location, ".bash_history")
    if not os.getenv('HISTSIZE') and not os.getenv('HISTFILESIZE'):
        os.environ['HISTSIZE'] = "3000"

    # instead of changing $HOME, start bash with: --rcfile "$LOCATION/.bashrc" ?
    #   if so, must care be taken to explicity call .grass.bashrc et al for
    #   non-interactive bash batch jobs?
    userhome = os.getenv('HOME')      # save original home
    home = location                   # save .bashrc in $LOCATION
    os.environ['HOME'] = home

    bashrc = os.path.join(home, ".bashrc")
    try_remove(bashrc)

    f = open(bashrc, 'w')
    f.write("test -r ~/.alias && . ~/.alias\n")
    if os.getenv('ISISROOT'):
        f.write("PS1='ISIS-GRASS %s (%s):\w > '\n" % (grass_version, location_name))
    else:
        f.write("PS1='GRASS %s (%s):\w > '\n" % (grass_version, location_name))
    
    f.write("""grass_prompt() {
	LOCATION="`g.gisenv get=GISDBASE,LOCATION_NAME,MAPSET separator='/'`"
	if test -d "$LOCATION/grid3/G3D_MASK" && test -f "$LOCATION/cell/MASK" ; then
		echo [%s]
	elif test -f "$LOCATION/cell/MASK" ; then
		echo [%s]
	elif test -d "$LOCATION/grid3/G3D_MASK" ; then
		echo [%s]
	fi
}
PROMPT_COMMAND=grass_prompt\n""" % (_("2D and 3D raster MASKs present"),
                                    _("Raster MASK present"),
                                    _("3D raster MASK present")))

    # read environmental variables
    path = os.path.join(userhome, ".grass.bashrc") # left for backward compatibility
    if os.access(path, os.R_OK):
        f.write(readfile(path) + '\n')
    if os.access(grass_env_file, os.R_OK):
        f.write(readfile(grass_env_file) + '\n')

    f.write("export PATH=\"%s\"\n" % os.getenv('PATH'))
    f.write("export HOME=\"%s\"\n" % userhome) # restore user home path

    f.close()

    exit_val = call([gpath("etc", "run"), os.getenv('SHELL')])

    os.environ['HOME'] = userhome


def default_startup():
    global exit_val

    if windows:
        os.environ['PS1'] = "GRASS %s> " % (grass_version)
        # "$ETC/run" doesn't work at all???
        exit_val = subprocess.call([os.getenv('SHELL')])
        cleanup_dir(os.path.join(location, ".tmp"))  # remove GUI session files from .tmp
    else:
        os.environ['PS1'] = "GRASS %s (%s):\w > " % (grass_version, location_name)
        exit_val = call([gpath("etc", "run"), os.getenv('SHELL')])

    if exit_val != 0:
        fatal(_("Failed to start shell '%s'") % os.getenv('SHELL'))


def done_message():
    # here was something for batch job but it was never called
    message(_("Done."))
    message("")
    message(_("Goodbye from GRASS GIS"))
    message("")


def clean_temp():
    message(_("Cleaning up temporary files..."))
    nul = open(os.devnull, 'w')
    call([gpath("etc", "clean_temp")], stdout=nul)
    nul.close()


def grep(string,list):
    expr = re.compile(string)
    return [elem for elem in list if expr.match(elem)]


def print_params():
    plat = gpath('include', 'Make', 'Platform.make')
    if not os.path.exists(plat):
        fatal(_("Please install the GRASS GIS development package"))
    fileplat = open(plat)
    linesplat = fileplat.readlines()
    fileplat.close()

    params = sys.argv[2:]
    if not params:
        params = ['arch', 'build', 'compiler', 'path', 'revision']
    
    for arg in params:
        if arg == 'path':
            sys.stdout.write("%s\n" % gisbase)
        elif arg == 'arch':
            val = grep('ARCH',linesplat)
            sys.stdout.write("%s\n" % val[0].split('=')[1].strip())
        elif arg == 'build':
            build = gpath('include', 'grass', 'confparms.h')
            filebuild = open(build)
            val = filebuild.readline()
            filebuild.close()
            sys.stdout.write("%s\n" % val.strip().strip('"').strip())
        elif arg == 'compiler':
            val = grep('CC',linesplat)
            sys.stdout.write("%s\n" % val[0].split('=')[1].strip())
        elif arg == 'revision':
            rev = gpath('include', 'grass', 'gis.h')
            filerev = open(rev)
            linesrev = filerev.readlines()
            val = grep('#define GIS_H_VERSION', linesrev)
            filerev.close()
            sys.stdout.write("%s\n" % val[0].split(':')[1].rstrip('$"\n').strip())
        else:
            message(_("Parameter <%s> not supported") % arg)


def get_username():
    """Get name of the current user"""
    if windows:
        user = os.getenv('USERNAME')
        if not user:
            user = "user_name"
    else:
        user = os.getenv('USER')
        if not user:
            user = os.getenv('LOGNAME')
        if not user:
            try:
                p = Popen(['whoami'], stdout = subprocess.PIPE)
                s = p.stdout.read()
                p.wait()
                user = s.strip()
            except:
                pass
        if not user:
            user = "user_%d" % os.getuid()
    return user


class Parameters:
    # we don't need to define any methods
    # pylint: disable=R0903
    def __init__(self):
        self.grass_gui = None
        self.create_new = None
        self.exit_grass = None
        self.force_gislock_removal = None
        self.mapset = None
        self.geofile = None


def parse_cmdline(argv):
    params = Parameters()
    args = []
    for i in argv:
        # Check if the user asked for the version
        if i in ["-v", "--version"]:
            message("GRASS GIS %s" % grass_version)
            message('\n' + readfile(gpath("etc", "license")))
            sys.exit()
        # Check if the user asked for help
        elif i in ["help", "-h", "-help", "--help", "--h"]:
            help_message()
            sys.exit()
        # Check if the -text flag was given
        elif i in ["-text", "--text"]:
            params.grass_gui = 'text'
        # Check if the -gtext flag was given
        elif i in ["-gtext", "--gtext"]:
            params.grass_gui = 'gtext'
        # Check if the -gui flag was given
        elif i in ["-gui", "--gui"]:
            params.grass_gui = default_gui
        # Check if the -wxpython flag was given
        elif i in ["-wxpython", "-wx", "--wxpython", "--wx"]:
            params.grass_gui = 'wxpython'
        # Check if the user wants to create a new mapset
        elif i == "-c":
            params.create_new = True
        elif i == "-e":
            params.exit_grass = True
        elif i == "-f":
            params.force_gislock_removal = True
        elif i == "--config":
            print_params()
            sys.exit()
        else:
            args.append(i)
    if len(args) > 1:
        params.mapset = args[1]
        params.geofile = args[0]
    elif len(args) == 1:
        params.mapset = args[0]
    else:
        params.mapset = None
    return params


### MAIN script starts here

# Get the system name
windows = sys.platform == 'win32'
cygwin = "cygwin" in sys.platform
macosx = "darwin" in sys.platform

### commented-out: broken winGRASS 
# if 'GISBASE' in os.environ:
#     sys.exit(_("ERROR: GRASS GIS is already running "
#                "(environmental variable GISBASE found)"))
# this is not really an issue, we should be able to overpower another session

# Set GISBASE
os.environ['GISBASE'] = gisbase

# set HOME
if windows and not os.getenv('HOME'):
    os.environ['HOME'] = os.path.join(os.getenv('HOMEDRIVE'),
                                      os.getenv('HOMEPATH'))

# set SHELL
if windows:
    if os.getenv('GRASS_SH'):
        os.environ['SHELL'] = os.getenv('GRASS_SH')
    if not os.getenv('SHELL'):
        os.environ['SHELL'] = os.getenv('COMSPEC', 'cmd.exe')

# Set default GUI
default_gui = "wxpython"

# explain what is happening in debug mode
debug("GRASS_DEBUG environmental variable is set. It is meant to be"
      " an internal variable for debugging only this script.\n"
      " Use 'g.gisenv set=\"DEBUG=[0-5]\"'"
      " to turn GRASS GIS debug mode on if you wish to do so.")

# Set GRASS version number for R interface etc (must be an env_var for MS-Windows)
os.environ['GRASS_VERSION'] = grass_version

# Set the GIS_LOCK variable to current process id
gis_lock = str(os.getpid())
os.environ['GIS_LOCK'] = gis_lock

if not os.path.exists(grass_config_dir):
    os.mkdir(grass_config_dir)

batch_job = get_batch_job()

# variable defined, but user might not have been careful enough
if batch_job:
    if not os.access(batch_job, os.F_OK):
        # wrong file
        fatal(_("Job file <%s> has been defined in "
                "the 'GRASS_BATCH_JOB' variable but not found. Exiting.\n\n"
                "Use 'unset GRASS_BATCH_JOB' to disable batch job processing.") % batch_job)
    elif not os.access(batch_job, os.X_OK):
        # right file, but ...
        fatal(_("Change file permission to 'executable' for <%s>") % batch_job)


# Set the global grassrc file
if batch_job:
    gisrcrc = os.path.join(grass_config_dir, "rc.%s" % platform.node())
    if not os.access(gisrcrc, os.R_OK):
        gisrcrc = os.path.join(grass_config_dir, "rc")
else:
    gisrcrc = os.path.join(grass_config_dir, "rc")

# Parse the command-line options and set several global variables
BATCH_EXEC_SUBCOMMAND = 'exec'

try:
    # raises ValueError when not found
    index = sys.argv.index(BATCH_EXEC_SUBCOMMAND)
    batch_job = sys.argv[index + 1:]
    clean_argv = sys.argv[1:index]
    params = parse_cmdline(clean_argv)
except ValueError:
    params = parse_cmdline(sys.argv[1:])

grass_gui = params.grass_gui

if params.exit_grass and not params.create_new:
    fatal(_("Flag -e requires also flag -c"))

# Set the username
user = get_username()

# Set language
# This has to be called before any _() function call!
# Subsequent functions are using _() calls and
# thus must be called only after Language has been set.
set_language()

# Create the temporary directory and session grassrc file
tmpdir = create_tmp(user, gis_lock)
atexit.register(cleanup, tmpdir)

# Create the session grassrc file
gisrc = create_gisrc(tmpdir, gisrcrc)

# Set shell (needs to be called before load_env())
check_shell()

# Load environmental variables from the file
load_env()

# Ensure GUI is set
if batch_job:
    grass_gui = 'text'
elif not grass_gui:
    # if GUI was not set previously (e.g. command line),
    # get it from rc file or env variable
    grass_gui = read_gui(default_gui)

# Set PATH, PYTHONPATH
set_paths()

# Set LD_LIBRARY_PATH (etc) to find GRASS shared libraries
path_prepend(gpath("lib"), ld_library_path_var)

# Set GRASS_PAGER, GRASS_PYTHON, GRASS_GNUPLOT, GRASS_PROJSHARE
set_defaults()

# Set GRASS_HTML_BROWSER
set_browser()

#predefine monitor size for certain architectures
if os.getenv('HOSTTYPE') == 'arm':
    # small monitor on ARM (iPAQ, zaurus... etc)
    os.environ['GRASS_RENDER_HEIGHT'] = "320"
    os.environ['GRASS_RENDER_WIDTH'] = "240"

# First time user - GISRC is defined in the GRASS script
if not os.access(gisrc, os.F_OK):
    if grass_gui == 'text' and not params.mapset:
        fatal(_("Unable to start GRASS GIS. You have the choice to:\n"
                " - Launch the GRASS GIS interface with the '-gui' switch (`%s -gui`)\n"
                " - Launch GRASS GIS directly with path to "
                "the location/mapset as an argument (`%s /path/to/location/mapset`)\n"
                " - Create manually the GISRC file (%s)") % (cmd_name, cmd_name, gisrcrc))
    create_initial_gisrc(gisrc)
else:
    clean_temp()

if params.create_new:
    message(_("Creating new GRASS GIS location/mapset..."))
else:
    message(_("Starting GRASS GIS..."))

# check that the GUI works but only if not doing a batch job
if not batch_job:
    grass_gui = check_gui(expected_gui=grass_gui)
    save_gui(gisrc, grass_gui)

# Parsing argument to get LOCATION
if not params.mapset:
    # Try interactive startup
    # User selects LOCATION and MAPSET if not set
    set_mapset_interactive(grass_gui)
else:
    if params.create_new and params.geofile:
        set_mapset(params.mapset, params.geofile, create_new=True)
    else:
        set_mapset(params.mapset, create_new=params.create_new)

# Set GISDBASE, LOCATION_NAME, MAPSET, LOCATION from $GISRC
# e.g. wxGUI startup screen writes to the gisrc file,
# so loading it is the only universal way to obtain the values
# this suppose that both programs share the right path to gisrc file
mapset_settings = load_gisrc(gisrc)

gisdbase = mapset_settings.gisdbase
location_name = mapset_settings.location
mapset = mapset_settings.mapset
location = mapset_settings.full_mapset

# Check .gislock file
check_lock(params.force_gislock_removal)

# build user fontcap if specified but not present
make_fontcap()

# predefine default driver if DB connection not defined
#  is this really needed?? Modules should call this when/if required.
if not os.access(os.path.join(location, "VAR"), os.F_OK):
    call(['db.connect', '-c', '--quiet'])

# Display the version and license info
# only non-error, interactive version continues from here
if batch_job:
    returncode = run_batch_job(batch_job)
    clean_temp()
    try_remove(lockfile)
    sys.exit(returncode)
elif params.exit_grass:
    clean_temp()
    try_remove(lockfile)
    sys.exit(0)
else:
    start_gui(grass_gui)
    clear_screen()
    show_banner()
    say_hello()
    show_info()
    if grass_gui == "wxpython":
        message(_("Launching <%s> GUI in the background, please wait...") % grass_gui)

if sh in ['csh', 'tcsh']:
    csh_startup()
elif sh in ['bash', 'msh', 'cygwin']:
    bash_startup()
else:
    default_startup()

# here we are at the end of grass session

clear_screen()

clean_temp()

try_remove(lockfile)

# save 'last used' GISRC after removing variables which shouldn't be saved
clean_env(gisrc)
writefile(gisrcrc, readfile(gisrc))

# here was cleanup function call but it is already registered at exit

# After this point no more grass modules may be called

done_message()
