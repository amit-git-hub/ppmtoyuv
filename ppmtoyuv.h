#ifndef _PPMTOYUV_H_
#define _PPMTOYUV_H_

#include <stdlib.h>
#include <stdio.h>


typedef char			INT8;
typedef unsigned char	UINT8;

typedef short			INT16;
typedef unsigned short	UINT16;

typedef int				INT32;
typedef unsigned int	UINT32;

#define	 MIN(X,Y)	((X) < (Y) ? (X) : (Y))

#define	 MAX(X,Y)	((X) > (Y) ? (X) : (Y))


//select output format
#define		YUV_400				0
#define 	YUV_444			 	1
#define 	YUV_420				2

void downscale_h2v2(UINT8 *h2v2,UINT32 cols, UINT32 rows);

void print_usage();

#endif
