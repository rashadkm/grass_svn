#!/bin/sh
############################################################################
#
# TOOL:         module_synopsis.sh
# AUTHOR:       M. Hamish Bowman, Dept. Marine Science, Otago Univeristy,
#                 New Zealand
# PURPOSE:      Runs through GRASS modules and creates a synopsis list of
#		  module names and descriptions.
# COPYRIGHT:    (c) 2007 Hamish Bowman, and the GRASS Development Team
#               This program is free software under the GNU General Public
#               License (>=v2). Read the file COPYING that comes with GRASS
#               for details.
#
#############################################################################
#
# PDF output requires the Palatino font.
#

if  [ -z "$GISBASE" ] ; then
    echo "You must be in GRASS GIS to run this program." 1>&2
    exit 1
fi

for FILE in txt tex pdf ; do
    if [ -e "$GISBASE/etc/module_synopsis.$FILE" ] ; then
    #  echo "ERROR: module_synopsis.$FILE already exists" 1>&2
    #  exit 1
       #blank it
       g.message -w "Overwriting \"\$GISBASE/etc/module_synopsis.$FILE\""
       \rm $GISBASE/etc/module_synopsis.$FILE
    fi
done


TMP="`g.tempfile pid=$$`"
if [ $? -ne 0 ] || [ -z "$TMP" ] ; then
    g.message -e "Unable to create temporary files" 
    exit 1
fi


g.message "Generating module synopsis ..."

SYNOP="$GISBASE/etc/module_synopsis.txt"

OLDDIR="`pwd`"
cd "$GISBASE"

# etc/photo.* ps.map
for DIR in bin scripts ; do
  cd $DIR

  for MODULE in ?\.* db.* r3.* ; do
    unset label
    unset desc
#    echo "[$MODULE]"

    case "$MODULE" in
      g.parser | r.mapcalc | r3.mapcalc | mkftcap | p.out.vrml| d.paint.labels)
	continue
	;;
    esac

    eval `$MODULE --tcltk | head -n 3 | tail -n 2 |  tr '"' "'" | \
        sed -e 's/^ //' -e 's/ {/="/' -e 's/}$/"/'`
    if [ -z "$label" ] && [ -z "$desc" ] ; then
	continue
    fi
    if [ -z "$label" ] ; then
	echo "$MODULE: $desc" >> "$TMP"
    else
	echo "$MODULE: $label" >> "$TMP"
    fi
  done

  cd ..
done

# ps.map doesn't jive with the above loop.
for MODULE in ps.map ; do
    unset label
    unset desc

    eval `$MODULE --tcltk | head -n 3 | tail -n 2 |  tr '"' "'" | \
	sed -e 's/^ //' -e 's/ {/="/' -e 's/}$/"/'`
    if [ -z "$label" ] && [ -z "$desc" ] ; then
	continue
    fi
    if [ -z "$label" ] ; then
	echo "$MODULE: $desc" >> "$TMP"
    else
	echo "$MODULE: $label" >> "$TMP"
    fi
done


# these don't use the parser at all.
cat << EOF >> "$TMP"
g.parser: Full parser support for GRASS scripts.
r.mapcalc: Performs arithmetic on raster map layers.
r3.mapcalc: Performs arithmetic on 3D grid volume data.
photo.2image: Marks fiducial or reseau points on an image to be ortho-rectified and then computes the image-to-photo coordinate transformation parameters.
photo.2target: Create control points on an image to be ortho-rectified.
photo.camera: Creates or modifies entries in a camera reference file.
photo.elev: Selects target elevation model for ortho-rectification.
photo.init: Creates or modifies entries in a camera initial exposure station file for imagery group referenced by a sub-block.
photo.rectify: Rectifies an image by using the image to photo coordinate transformation matrix created by photo.2image and the rectification parameters created by photo.2target.
photo.target: Selects target location and mapset for ortho-rectification.
EOF

## with --dictionary-order db.* ends up in the middle of the d.* cmds
#sort --dictionary-order "$TMP" > "$SYNOP"
sort "$TMP" > "$SYNOP"
\rm -f "$TMP"


####### create LaTeX source #######
g.message "Generating LaTeX souce ..."

# add missing periods at end of descriptions
sed -e 's/[^\.]$/&./' "$SYNOP" > "${TMP}.txt"


#### save header
cat << EOF > "${TMP}.tex"
%% Adapted from LyX 1.3 LaTeX export. (c) 2007 The GRASS Development Team
\documentclass[american]{article}
\usepackage{palatino}
\usepackage[T1]{fontenc}
\usepackage[latin1]{inputenc}
\usepackage{a4wide}
\usepackage{graphicx}

\makeatletter
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Textclass specific LaTeX commands.
 \newenvironment{lyxlist}[1]
   {\begin{list}{}
     {\settowidth{\labelwidth}{#1}
      \setlength{\leftmargin}{\labelwidth}
      \addtolength{\leftmargin}{\labelsep}
      \renewcommand{\makelabel}[1]{##1\hfil}}}
   {\end{list}}

\usepackage{babel}
\makeatother
\begin{document}
\begin{center}\includegraphics[%
  scale=0.5]{grasslogo_vector.eps}\end{center}

\begin{center}{\huge `g.version | cut -f1 -d'('` Command list}\end{center}{\huge \par}

\begin{center}{\large `date "+%e %B %Y"`}\end{center}{\large \par}
\bigskip{}


\section*{Command types:}

\begin{lyxlist}{00.00.0000}
\item [d.{*}]display commands
\item [db.{*}]database commands
\item [g.{*}]general commands
\item [i.{*}]imagery commands
\item [m.{*}]miscellanous commands
\item [ps.{*}]postscript commands
\item [r.{*}]raster commands
\item [r3.{*}]raster3D commands
\item [v.{*}]vector commands
\item [gis.m]GUI frontend (Tcl/Tk)
\item [nviz]visualization suite
\item [xganim]raster map slideshow
EOF


#### fill in module entries

for SECTION in d db g i m ps r r3 v ; do
    SEC_TYPE="commands"
    case $SECTION in
      d)
	SEC_NAME="Display" ;;
      db)
	SEC_NAME="Database management" ;;
      g)
	SEC_NAME="General GIS management" ;;
      i)
	SEC_NAME="Imagery" ;;
      m)
	SEC_NAME="Miscellaneous"
	SEC_TYPE="tools" ;;
      ps)
	SEC_NAME="PostScript" ;;
      r)
	SEC_NAME="Raster" ;;
      r3)
	SEC_NAME="Raster 3D" ;;
      v)
	SEC_NAME="Vector" ;;
    esac


    cat << EOF >> "${TMP}.tex"
\end{lyxlist}

\smallskip{}
\section*{$SEC_NAME $SEC_TYPE:}

\begin{lyxlist}{00.00.0000}
EOF

    grep "^${SECTION}\." "${TMP}.txt" | \
      sed -e 's/^/\\item [/' -e 's/: /]/' \
          -e 's/\*/{*}/g' -e 's/_/\\_/g' -e 's/&/\\\&/g' \
	  >> "${TMP}.tex"

    if [ "$SECTION" = "i" ] ; then
	# include imagery photo subsection
	cat << EOF >> "${TMP}.tex"
\end{lyxlist}

\subsubsection*{Imagery photo.{*} commands:}

\begin{lyxlist}{00.00.0000}
EOF

	grep "^photo\." "${TMP}.txt" | \
	  sed -e 's/^/\\item [/' -e 's/: /]/' >> "${TMP}.tex"
    fi

done


#### save footer
cat << EOF >> "${TMP}.tex"
\end{lyxlist}

\end{document}
EOF


\mv "${TMP}.tex" "$GISBASE/etc/module_synopsis.tex"
\rm -f "${TMP}.txt"



g.message "Converting to PDF ..."

# pdflatex and texi2pdf don't like .eps files, so we go the long way aroung.
for PGM in texi2dvi dvips a2ps ps2pdf13 ; do
   if [ ! -x `which $PGM` ] ; then
	g.message -e "texi2dvi, dvips, a2ps, and ps2pdf13 needed for this PDF conversion."
	g.message "Done."
   fi
done

TMPDIR="`dirname "$TMP"`"
cp "$OLDDIR/grasslogo_vector.eps" "$TMPDIR"
cd "$TMPDIR"

texi2dvi --quiet --clean "$GISBASE/etc/module_synopsis.tex"
dvips -q module_synopsis.dvi
a2ps --quiet module_synopsis.ps -o module_synopsis_2.ps
ps2pdf13 module_synopsis_2.ps module_synopsis.pdf


\rm -f module_synopsis.dvi module_synopsis.ps \
       module_synopsis_2.ps grasslogo_vector.eps
\mv module_synopsis.pdf "$GISBASE/etc/"


g.message "Done."
