/*********************************************************
* LCUtil
* Utility functions for LaserCanvas.
* $PSchlup 2001-2006 $     $Revision 5 $
*********************************************************/
#include "LCUtil.h"                         // header


/*********************************************************
*  PointNearestLine
*         dptUV
* pt1_______.________ pt2
*     \_    :
*       \_  : dDU
*         \_:
*          Target
*  Vx-->VxNext = V; Vx-->Target = U
*  dptUV = (U . V / V . V) V
* Returns the distance from the line
*********************************************************/
double PointNearestLine(double dX1, double dY1, double dX2, double dY2, double dXTgt, double dYTgt, double *pdXOut, double *pdYOut) {
   double    dXUV, dYUV;                    // projection of target onto segment
   double    dUV, dVV;                      // (U . V) and (V . V)
   double    dUV_VV;                        // (U . V / V . V)
   double    dDU;                           // (squared) distance from line

   dVV = (SQR(dX2 - dX1) + SQR(dY2 - dY1));
   if(dVV != 0.00) {                        // ignore if Vx are coincident
      dUV = ((dXTgt - dX1)*(dX2 - dX1) + (dYTgt - dY1)*(dY2 - dY1));
      dUV_VV = dUV / dVV;
      if(dUV_VV < 0.00) dUV_VV = 0.00;      // constrain to segment
      if(dUV_VV > 1.00) dUV_VV = 1.00;
      dXUV = (1.00 - dUV_VV) * dX1 + dUV_VV * dX2;
      dYUV = (1.00 - dUV_VV) * dY1 + dUV_VV * dY2;
   } else { //if(dVV==0.00)
      dXUV = dX1;
      dYUV = dY1;
   }
   dDU = SQRT( // calculate distance from target to point on line
         SQR(dXTgt - dXUV) + SQR(dYTgt - dYUV)
   );

   if(pdXOut) *pdXOut = dXUV;               // return point if required
   if(pdYOut) *pdYOut = dYUV;
   return(dDU);                             // return distance
}

