#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <rpc/types.h>
#include <rpc/xdr.h>
#include "G3d_intern.h"

/*---------------------------------------------------------------------------*/

int
G3d_g3dType2cellType  (int g3dType)

{
  if (g3dType == FCELL_TYPE) return FCELL_TYPE;
  return DCELL_TYPE;
}

/*---------------------------------------------------------------------------*/

void
G3d_copyFloat2Double  (float *src, int offsSrc, double *dst, int offsDst, int nElts)

{
  float *srcStop;

  src += offsSrc;
  dst += offsDst;
  srcStop = src + nElts;
  while (src != srcStop) *dst++ = *src++;
}

/*---------------------------------------------------------------------------*/

void
G3d_copyDouble2Float  (double *src, int offsSrc, float *dst, int offsDst, int nElts)

{
  double *srcStop;

  src += offsSrc;
  dst += offsDst;
  srcStop = src + nElts;
  while (src != srcStop) *dst++ = *src++;
}

/*---------------------------------------------------------------------------*/

void
G3d_copyValues  (char *src, int offsSrc, int typeSrc, char *dst, int offsDst, int typeDst, int nElts)

{
  char *srcStop;
  int eltLength;

  if ((typeSrc == FCELL_TYPE) && (typeDst == DCELL_TYPE)) {
    G3d_copyFloat2Double ((float *) src, offsSrc, (double *) dst,
			  offsDst, nElts);
    return;
  }
  
  if ((typeSrc == DCELL_TYPE) && (typeDst == FCELL_TYPE)) {
    G3d_copyDouble2Float ((double *) src, offsSrc, (float *) dst,
			  offsDst, nElts);
    return;
  }

  eltLength = G3d_length (typeSrc);

  src += eltLength * offsSrc;
  dst += eltLength * offsDst;

  srcStop = src + nElts * eltLength;
  while (src != srcStop) *dst++ = *src++;
}

/*---------------------------------------------------------------------------*/

int
G3d_length  (int t)

{
  if (! G3D_IS_CORRECT_TYPE (t)) G3d_fatalError ("G3d_length: invalid type");

  if (t == FCELL_TYPE) return sizeof (float);
  if (t == DCELL_TYPE) return sizeof (double);
  return 0;
}

int
G3d_externLength  (int t)

{
  if (! G3D_IS_CORRECT_TYPE (t)) G3d_fatalError ("G3d_externLength: invalid type");

  if (t == FCELL_TYPE) return G3D_XDR_FLOAT_LENGTH;
  if (t == DCELL_TYPE) return G3D_XDR_DOUBLE_LENGTH;
  return 0;
}
