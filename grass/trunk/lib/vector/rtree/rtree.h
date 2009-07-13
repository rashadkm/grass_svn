
/****************************************************************************
* MODULE:       R-Tree library 
*              
* AUTHOR(S):    Antonin Guttman - original code
*               Daniel Green (green@superliminal.com) - major clean-up
*                               and implementation of bounding spheres
*               Markus Metz - R*-tree
*               
* PURPOSE:      Multidimensional index
*
* COPYRIGHT:    (C) 2009 by the GRASS Development Team
*
*               This program is free software under the GNU General Public
*               License (>=v2). Read the file COPYING that comes with GRASS
*               for details.
*****************************************************************************/

#include <grass/rtree/card.h>
#include <grass/rtree/index.h>
#include <grass/rtree/split.h>
