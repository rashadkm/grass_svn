/*****************************************************************************
*
* MODULE:       SQL statement parser library 
*   	    	
* AUTHOR(S):    lex.l and yac.y were originaly taken from unixODBC and
*               probably written by Peter Harvey <pharvey@codebydesigns.com>,
*               modifications and other code by Radim Blazek
*
* PURPOSE:      Parse input string containing SQL statement to 
*               SQLPSTMT structure.
*               SQL parser may be used by simple database drivers. 
*
* COPYRIGHT:    (C) 2000 by the GRASS Development Team
*
*               This program is free software under the GNU General Public
*   	    	License (>=v2). Read the file COPYING that comes with GRASS
*   	    	for details.
*
*****************************************************************************/

#define SQLP_MAIN 

#include "sqlp.h"
#include <stdio.h>

/* alloc structure */
SQLPSTMT * sqpInitStmt( void  )
{
    SQLPSTMT *st;
    
    st = (SQLPSTMT *) calloc (1, sizeof (SQLPSTMT));

    return (st);
}

/* allocate space for columns */
int sqpAllocCol(SQLPSTMT *st, int n)
{
    int i;

    if ( n > st->aCol )
      {
	n += 15;      
        st->Col = (SQLPVALUE *) realloc ( st->Col, n * sizeof(SQLPVALUE));
        st->ColType = (int *) realloc ( st->ColType, n * sizeof(int));
        st->ColWidth = (int *) realloc ( st->ColWidth, n * sizeof(int));
        st->ColDecim = (int *) realloc ( st->ColDecim, n * sizeof(int));
	
        for (i = st->nCol; i < n; i++)
	  {
            st->Col[i].s = NULL ;
          }
	    
        st->aCol = n;
      }
   return (1); 
}
    
/* allocate space for values */
int sqpAllocVal(SQLPSTMT *st, int n)
{
    int i;

    if ( n > st->aVal )
      {
	n += 15;      
        st->Val = (SQLPVALUE *) realloc ( st->Val, n * sizeof(SQLPVALUE));
	
        for (i = st->nVal; i < n; i++)
	  {
            st->Val[i].s = NULL ;
          }
	    
        st->aVal = n;
      }
   return (1); 
}

/* allocate space for comparisons */
int sqpAllocCom(SQLPSTMT *st, int n)
{
    int i;

    if ( n > st->aCom )
      {
	n += 15;      
        st->ComCol = (SQLPVALUE *) realloc ( st->ComCol, n * sizeof(SQLPVALUE));
        st->ComOpe = (int *) realloc ( st->ComOpe, n * sizeof(int));
        st->ComVal = (SQLPVALUE *) realloc ( st->ComVal, n * sizeof(SQLPVALUE));
	
        for (i = st->nCom; i < n; i++)
	  {
            st->ComCol[i].s = NULL ;
            st->ComVal[i].s = NULL ;
          }
	    
        st->aCom = n;
      }
   return (1); 
}

/* free space allocated by parser */
int sqpFreeStmt(SQLPSTMT *st)
{
    int i;

    /* columns */
    for (i=0; i < st->aCol; i++)
        free ( st->Col[i].s );

    free ( st->Col );
    free ( st->ColType );
    free ( st->ColWidth );
    free ( st->ColDecim );
    st->aCol = 0;
    st->nCol = 0;
    
    /* values */
    for (i=0; i < st->aVal; i++)
        free ( st->Val[i].s );

    free ( st->Val );
    st->aVal = 0;
    st->nVal = 0;
    
    /* comparisons */
    for (i=0; i < st->aCom; i++)
      {    
        free ( st->ComCol[i].s );
        free ( st->ComVal[i].s );
      }
    free ( st->ComCol );
    free ( st->ComOpe );
    free ( st->ComVal );
    st->aCom = 0;
    st->nCom = 0;

    free ( st );
    return (1);
}
	

