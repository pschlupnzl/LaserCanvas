/*********************************************************
* LCUtil
* Utility functions for LaserCanvas.
* $PSchlup 2001-2006 $     $Revision 5 $
*********************************************************/

#ifndef LCUTIL_H                            // prevent multiple includes
#define LCUTIL_H

//===Includes=============================================
#include <stdio.h>                          // standard header
#include <math.h>                           // mathematical functions

//===Constants and Macros=================================
#ifndef SQR
#define SQR(x) ((x)*(x))
#endif//SQR
#ifndef SQRT
#define SQRT(x) (sqrt(((x)>0.00) ? (x) : (-(x))))
#endif//SQRT

//===Function Prototypes==================================
double PointNearestLine(double dX1, double dY1, double dX2, double dY2, double dXTgt, double dYTgt, double *pdXOut, double *pdYOut);

#endif//LCUTIL_H
