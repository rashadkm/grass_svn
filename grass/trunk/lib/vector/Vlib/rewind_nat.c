/*!
   \file lib/vector/Vlib/rewind.c

   \brief Vector library - rewind data (native format)

   Higher level functions for reading/writing/manipulating vectors.

   (C) 2001-2009 by the GRASS Development Team

   This program is free software under the GNU General Public License
   (>=v2).  Read the file COPYING that comes with GRASS for details.

   \author Original author CERL, probably Dave Gerdes or Mike Higgins.
   \author Update to GRASS 5.7 Radim Blazek and David D. Gray.
*/

#include <grass/vector.h>

/*!
  \brief Rewind vector data file to cause reads to start at beginning (level 1)

  \param Map vector map

  \return 0 on success
  \return -1 on error
*/
int V1_rewind_nat(struct Map_info *Map)
{
    return (dig_fseek(&(Map->dig_fp), Map->head.head_size, SEEK_SET));
}

/*!
  \brief Rewind vector data file to cause reads to start at beginning (level 2)

  \param Map vector map

  \return 0 on success
  \return -1 on error
*/
int V2_rewind_nat(struct Map_info *Map)
{
    Map->next_line = 1;
    return V1_rewind_nat(Map);	/* make sure level 1 reads are reset too */
}
