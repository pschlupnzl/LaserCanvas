/*********************************************************
*  CAxes.cpp
*  Axes class implementation, Win32
*  This software was developed entirely without resources
*  from the Universities of Otago, Macquarie, St Andrews,
*  and the ETH.
*  $PSchlup 2004aug01 (ZH) $      $Revision 3 $
*********************************************************/
#include "caxes.h"

/*********************************************************
*  Constructor
*********************************************************/
CAxes::CAxes(void) {
   for(int iAx=0; iAx<AX_NUMAXES; iAx++) {
      uFlags[iAx] = 0x0000;
   }
   SetTickIn();
   SetBoxOff();
   SetTickSize(5);
   SetTickAuto();
   SetAxLocation(AX_XAXIS, AXS_AXISBOTTOM);
   SetAxLocation(AX_YAXIS, AXS_AXISLEFT);
   SetAxLabel(AX_XAXIS, "");
   SetAxLabel(AX_YAXIS, "");
   SetAxDataLimit(AX_XAXIS, 0.00, 1.00);
   SetAxDataLimit(AX_YAXIS, 0.00, 1.00);
}


/*********************************************************
*  Virtual functions
*********************************************************/
inline BOOL CAxes::GDIMoveTo(int x, int y, void *pVoid) {
   return(
      MoveToEx(*((HDC*)pVoid), x, y, NULL)
   );
}

inline BOOL CAxes::GDILineTo(int x, int y, void *pVoid) {
   return(
      LineTo(*((HDC*)pVoid), x, y)
   );
}

inline BOOL CAxes::GDIGetTextExtent(char *szStr, int iLen, int *pCx, int *pCy, void *pVoid) {
   SIZE sSize;
   BOOL tfRet;

   tfRet = GetTextExtentPoint32(*((HDC*)pVoid), szStr, iLen, &sSize);
   *pCx = sSize.cx;
   *pCy = sSize.cy;
   return(tfRet);
}

inline BOOL CAxes::GDITextOut(int x, int y, char *szStr, int iLen, void *pVoid) {
   return(
      TextOut(*((HDC*)pVoid), x, y, szStr, iLen)
   );
}

inline BOOL CAxes::GDIPolygon(POINT pts[], int iNm, void *pVoid) {
   return(
      Polygon(*((HDC*)pVoid), pts, iNm)
   );
}


/*********************************************************
*  Paint                                          (rev3)
* Additional argument to specify if the axis limits should
* be painted as well as the normal ticks
*********************************************************/
void CAxes::Paint(void *pVoid, BOOL tfLim) {
   int    x0 = 0;                           // RESERVED Graphics offset for..
   int    y0 = 0;                           // RESERVED..memory DC painting (see StepWin)
   int    i;                                // tick loop counter
   double dRecipLen;
   double dDelta;                           // spacing of ticks
   int    tickStart, tickStop, tickPos, tickStart2, tickStop2;
   char   szTickLabel[20];                  // buffer for tick string
   int    iMaxLabelSize;                    // maximum width of Y axis label
   int    iStrLen;
   int    iLabelWid, iLabelHig;             // label width / height; eliminated SIZE struct

   if((rcPos.right-rcPos.left==0) && (rcPos.bottom-rcPos.top==0)) return; // ignore if no space

   //---X-axis-----------------------------------
   //SetAxMaxVisTick(AX_XAXIS, (int) ((rcPos.right - rcPos.left) / 50)); // assure minimum tick spacing
   //if(uFlags[AX_XAXIS] & AXS_TICKAUTO) CalcAxTicks(AX_XAXIS);
   if((dDelta = (mnxLim[AX_XAXIS].Max - mnxLim[AX_XAXIS].Min)) == 0.00) dDelta = 1.00;		// prevent #DIV/0! errors
   dRecipLen = (rcPos.right - rcPos.left) / dDelta;
   tickStart = rcPos.bottom - ( (uFlags[AX_XAXIS] & AXS_TICKIN) ? iTickSize[AX_XAXIS] : 0 );
   tickStop  = rcPos.bottom + ( (uFlags[AX_XAXIS] & AXS_TICKOUT)? iTickSize[AX_XAXIS] : 0 );
   tickStart2= rcPos.top    - ( (uFlags[AX_XAXIS] & AXS_TICKOUT)? iTickSize[AX_XAXIS] : 0 );
   tickStop2 = rcPos.top    + ( (uFlags[AX_XAXIS] & AXS_TICKIN) ? iTickSize[AX_XAXIS] : 0 );
   //---Display Label---
   iStrLen = strlen(szLabel[AX_XAXIS]);
   if(iStrLen > 0) {
      GDIGetTextExtent(szLabel[AX_XAXIS], iStrLen, &iLabelWid, &iLabelHig, pVoid); // get size of label
      if(uFlags[AX_XAXIS] & AXS_AXISTOP) {
         GDITextOut(
               (rcPos.left + rcPos.right - iLabelWid) / 2 - x0,
               tickStart2 - 2*iLabelHig - y0,
               szLabel[AX_XAXIS], iStrLen, pVoid
         );
      } else if(uFlags[AX_XAXIS] & AXS_AXISBOTTOM) {
         GDITextOut(
               (rcPos.left + rcPos.right - iLabelWid) / 2 - x0,
               tickStop + iLabelHig - y0,
               szLabel[AX_XAXIS], iStrLen, pVoid
         );
      }
   }

   //---Draw Labels and Ticks---
   for(i=0; i < iNumTick[AX_XAXIS]; i++) {
      tickPos = rcPos.left + (dRecipLen * (dTick[AX_XAXIS][i] - mnxLim[AX_XAXIS].Min));
      if((tickPos >= rcPos.left-1) && (tickPos <= rcPos.right+1)) {
         sprintf(szTickLabel,"%g", dTick[AX_XAXIS][i]); // copy label string
         iStrLen = strlen(szTickLabel);
         GDIGetTextExtent(szTickLabel, iStrLen, &iLabelWid, &iLabelHig, pVoid); // determine label dimensions
         if(uFlags[AX_XAXIS] & AXS_AXISTOP) {
            GDITextOut(
               tickPos - (iLabelWid/2) - x0,
               tickStart2 - (iLabelHig) - y0,
               szTickLabel, iStrLen, pVoid
            );
         }
         if(uFlags[AX_XAXIS] & AXS_AXISBOTTOM) {
            GDITextOut(
               tickPos  - (iLabelWid/2) - x0,
               tickStop - y0,
               szTickLabel, iStrLen, pVoid
            );
         }
         if((uFlags[AX_XAXIS] & AXS_AXISTOP) || (uFlags[AX_XAXIS] & AXS_BOXON)) {
            GDIMoveTo(tickPos - x0, tickStart2 - y0, pVoid);
            GDILineTo(tickPos - x0, tickStop2 - y0, pVoid);
         }
         if((uFlags[AX_XAXIS] & AXS_AXISBOTTOM) || (uFlags[AX_XAXIS] & AXS_BOXON)){
            GDIMoveTo(tickPos - x0, tickStart - y0, pVoid);
            GDILineTo(tickPos - x0, tickStop - y0, pVoid);
         }
      }
   }
   if(tfLim) {
      sprintf(szTickLabel, " %g ", mnxLim[AX_XAXIS].Min);
      iStrLen = strlen(szTickLabel);
      GDIGetTextExtent(szTickLabel, iStrLen, &iLabelWid, &iLabelHig, pVoid);
      if(uFlags[AX_XAXIS] & AXS_AXISTOP)    GDITextOut(rcPos.left - (iLabelWid/2) - x0, tickStart2 - (iLabelHig) - y0, szTickLabel, iStrLen, pVoid);
      if(uFlags[AX_XAXIS] & AXS_AXISBOTTOM) GDITextOut(rcPos.left - (iLabelWid/2) - x0, tickStop                 - y0, szTickLabel, iStrLen, pVoid);
      sprintf(szTickLabel, " %g ", mnxLim[AX_XAXIS].Max);
      iStrLen = strlen(szTickLabel);
      GDIGetTextExtent(szTickLabel, iStrLen, &iLabelWid, &iLabelHig, pVoid);
      if(uFlags[AX_XAXIS] & AXS_AXISTOP)    GDITextOut(rcPos.right - (iLabelWid/2) - x0, tickStart2 - (iLabelHig) - y0, szTickLabel, iStrLen, pVoid);
      if(uFlags[AX_XAXIS] & AXS_AXISBOTTOM) GDITextOut(rcPos.right - (iLabelWid/2) - x0, tickStop                 - y0, szTickLabel, iStrLen, pVoid);
   }
   if((uFlags[AX_XAXIS] & AXS_AXISTOP) || (uFlags[AX_XAXIS] & AXS_BOXON)) {
      GDIMoveTo(rcPos.left - x0, rcPos.top - y0, pVoid);
      GDILineTo(rcPos.right - x0, rcPos.top - y0, pVoid);
   }
   if((uFlags[AX_XAXIS] & AXS_AXISBOTTOM) || (uFlags[AX_XAXIS] & AXS_BOXON)) {
      GDIMoveTo(rcPos.left - y0, rcPos.bottom - y0, pVoid);
      GDILineTo(rcPos.right - y0, rcPos.bottom - y0, pVoid);
   }

   //---Y-axis-----------------------------------
   //SetAxMaxVisTick(AX_YAXIS, (int) ((rcPos.bottom - rcPos.top) / 35)); // assure minimum tick spacing
   //if(uFlags[AX_YAXIS]) CalcAxTicks(AX_YAXIS);
   if((dDelta = (mnxLim[AX_YAXIS].Max - mnxLim[AX_YAXIS].Min)) == 0.00) dDelta = 1.00; // prevent #DIV/0! errors
   dRecipLen = (rcPos.top - rcPos.bottom) / dDelta;
   tickStart = rcPos.left  - ( (uFlags[AX_YAXIS] & AXS_TICKOUT)? iTickSize[AX_YAXIS] : 0 );
   tickStop  = rcPos.left  + ( (uFlags[AX_YAXIS] & AXS_TICKIN) ? iTickSize[AX_YAXIS] : 0 );
   tickStart2= rcPos.right - ( (uFlags[AX_YAXIS] & AXS_TICKIN) ? iTickSize[AX_YAXIS] : 0 );
   tickStop2 = rcPos.right + ( (uFlags[AX_YAXIS] & AXS_TICKOUT)? iTickSize[AX_YAXIS] : 0 );
   //---Draw Ticks and Labels---
   iMaxLabelSize = 0;                       // record longest tick label length
   for(i=0; i < iNumTick[AX_YAXIS]; i++) {
      tickPos = rcPos.bottom + (dRecipLen * (dTick[AX_YAXIS][i] - mnxLim[AX_YAXIS].Min));
      if((tickPos >= rcPos.top-1) && (tickPos <= rcPos.bottom+1)) {
         sprintf(szTickLabel,"%g", dTick[AX_YAXIS][i]); // copy label string
         iStrLen = strlen(szTickLabel);
         GDIGetTextExtent(szTickLabel, iStrLen, &iLabelWid, &iLabelHig, pVoid);
         if(uFlags[AX_YAXIS] & AXS_AXISRIGHT) {
            GDITextOut(
               tickStop2 + (iLabelHig/2) - x0,
               tickPos - (iLabelHig/2) - y0,
               szTickLabel, iStrLen, pVoid
            );
         }
         if(uFlags[AX_YAXIS] & AXS_AXISLEFT) {
            GDITextOut(
               tickStart - iLabelWid - iLabelHig/2 - x0,
               tickPos - (iLabelHig/2) - y0,
               szTickLabel, iStrLen, pVoid
            );
         }
         if((uFlags[AX_YAXIS] & AXS_AXISRIGHT) || (uFlags[AX_YAXIS] & AXS_BOXON)) {
            GDIMoveTo(tickStart2 - x0, tickPos - y0, pVoid);
            GDILineTo(tickStop2 - x0, tickPos - y0, pVoid);
         }
         if((uFlags[AX_YAXIS] & AXS_AXISLEFT) || (uFlags[AX_YAXIS] & AXS_BOXON)) {
            GDIMoveTo(tickStart - x0, tickPos - y0, pVoid);
            GDILineTo(tickStop - x0, tickPos - y0, pVoid);
         }
         if(iLabelWid > iMaxLabelSize) iMaxLabelSize = iLabelWid; // record longest tick label length
      }
   }
   if(tfLim) {
      sprintf(szTickLabel, "%g", mnxLim[AX_YAXIS].Min);
      iStrLen = strlen(szTickLabel);
      GDIGetTextExtent(szTickLabel, iStrLen, &iLabelWid, &iLabelHig, pVoid);
      if(uFlags[AX_YAXIS] & AXS_AXISRIGHT) GDITextOut(tickStop2             + (iLabelHig/2) - x0, rcPos.bottom - (iLabelHig/2) - y0, szTickLabel, iStrLen, pVoid);
      if(uFlags[AX_YAXIS] & AXS_AXISLEFT)  GDITextOut(tickStart - iLabelWid - (iLabelHig/2) - x0, rcPos.bottom - (iLabelHig/2) - y0, szTickLabel, iStrLen, pVoid);
      sprintf(szTickLabel, "%g", mnxLim[AX_YAXIS].Max);
      iStrLen = strlen(szTickLabel);
      GDIGetTextExtent(szTickLabel, iStrLen, &iLabelWid, &iLabelHig, pVoid);
      if(uFlags[AX_YAXIS] & AXS_AXISRIGHT) GDITextOut(tickStop2             + (iLabelHig/2) - x0, rcPos.top - (iLabelHig/2) - y0, szTickLabel, iStrLen, pVoid);
      if(uFlags[AX_YAXIS] & AXS_AXISLEFT)  GDITextOut(tickStart - iLabelWid - (iLabelHig/2) - x0, rcPos.top - (iLabelHig/2) - y0, szTickLabel, iStrLen, pVoid);
   }
   if((uFlags[AX_YAXIS] & AXS_AXISRIGHT) || (uFlags[AX_YAXIS] & AXS_BOXON)) {
      GDIMoveTo(rcPos.right - x0, rcPos.top - y0, pVoid);
      GDILineTo(rcPos.right - x0, rcPos.bottom - y0, pVoid);
   }
   if((uFlags[AX_YAXIS] & AXS_AXISLEFT) || (uFlags[AX_YAXIS] & AXS_BOXON)) {
      GDIMoveTo(rcPos.left - x0, rcPos.top - y0, pVoid);
      GDILineTo(rcPos.left - x0, rcPos.bottom - y0, pVoid);
   }
   //---Y Label---
   iStrLen = strlen(szLabel[AX_YAXIS]);
   GDIGetTextExtent(szLabel[AX_YAXIS], iStrLen, &iLabelWid, &iLabelHig, pVoid);
   iMaxLabelSize += iLabelHig;              // introduce some space
   if(iStrLen > 0) {
      if(uFlags[AX_YAXIS] & AXS_AXISRIGHT) {
         GDITextOut(
            tickStop2 +iMaxLabelSize - x0,
            (rcPos.top + rcPos.bottom - iLabelHig)/2 - y0,
            szLabel[AX_YAXIS], iStrLen, pVoid
         );
      } else if(uFlags[AX_YAXIS] & AXS_AXISLEFT) {
         GDITextOut(
            tickStart - iMaxLabelSize - iLabelWid - x0,
            (rcPos.top + rcPos.bottom - iLabelHig)/2 - y0,
            szLabel[AX_YAXIS], iStrLen, pVoid
         );
      }
   }
}


/*********************************************************
* PaintData                                         rev 3
* This function has a number of extra features
*  - Automatically chooses between point-by-point and ex-
*    trema range painting
*  - Allows X and / or Y to be normalized
*  - Accepts NULL  for X vector, in which case  the index
*    is used
*  - Allows flags for additionally painting with markers
* Adapted from PaintDoubleDataMX
*********************************************************/
#define fnXData(iPt) ((dXData) ? (dXData[iPt]) : (iPt)) // accept NULL x-vectors
void CAxes::PaintData(double dXData[], double dYData[], int iNumPts, void *pVoid, UINT uFlags) {
   int    iPt;                              // loop counter
   int    iPtMin, iPtMax;                   // min and max plotting point indices
   double dSclX, dSclY;                     // scales for plotting data
   int    x, xPrv, y, yMn, yMx;             // coordinates
   double YMn, YMx;                         // limits
   double YMnPrev, YMxPrev;                 // previous limits
   BOOL   tfMove;                           // move / line clip to rectangle
   double dAxisXMin, dAxisXMax, dAxisYMin, dAxisYMax; // set / restore ranges for Marker line too

   double dMinX = mnxLim[AX_XAXIS].Min;     // start with axes limits
   double dMaxX = mnxLim[AX_XAXIS].Max;
   double dMinY = mnxLim[AX_YAXIS].Min;
   double dMaxY = mnxLim[AX_YAXIS].Max;
   LPRECT prc   = &rcPos;

   if(iNumPts == 0) return;                 // ignore if there's no data

   //===Preliminaries=====================================
   //---Normalize if needed-----------
   if(uFlags & AX_NORMX) for(iPt=1, dMinX=dMaxX=fnXData(0); iPt<iNumPts; iPt++) {
      if(fnXData(iPt) < dMinX) dMinX = fnXData(iPt);
      if(fnXData(iPt) > dMaxX) dMaxX = fnXData(iPt);
   }
   if(uFlags & AX_NORMY) for(iPt=1, dMinY=dMaxY=dYData[0]; iPt<iNumPts; iPt++) {
      if(dYData[iPt] < dMinY) dMinY = dYData[iPt];
      if(dYData[iPt] > dMaxY) dMaxY = dYData[iPt];
   }

   //---Scaling-----------------------
   if(fabs(dMaxY-dMinY) < 1e-8) dMaxY = dMinY + 1.00;
   dSclY = (prc->bottom - prc->top) / (dMaxY - dMinY);
   if(fabs(dMaxX-dMinX) < 1e-8) dMaxX = dMinX + 1.00;
   dSclX = (prc->right - prc->left) / (dMaxX - dMinX);

   //---Points to plot---------------
   for(iPtMin=0; iPtMin<iNumPts; iPtMin++) { // find first point to plot
      x = prc->left + (int) (dSclX*(fnXData(iPtMin)-dMinX));
      if((x>=prc->left) && (x<=prc->right)) break; // exit at first point to plot
   }
   for(iPtMax=iNumPts-1; iPtMax>=0; iPtMax--) { // find last point to plot
      x = prc->left + (int) (dSclX*(fnXData(iPtMax)-dMinX));
      if((x>=prc->left) && (x<=prc->right)) break; // exit at first point to plot
   }

   //===Marker============================================
   // This is cheap and nasty: We briefly change the axes
   // limits  for the call to PaintDataMarker so  it uses
   // the correct scaling.
   if(uFlags & AX_MKR) {
      dAxisXMin = mnxLim[AX_XAXIS].Min;
      dAxisXMax = mnxLim[AX_XAXIS].Max;
      dAxisYMin = mnxLim[AX_YAXIS].Min;
      dAxisYMax = mnxLim[AX_YAXIS].Max;
      PaintDataMarker(dXData, dYData, iNumPts, pVoid, 1, uFlags & AX_MKR);
      mnxLim[AX_XAXIS].Min = dAxisXMin;
      mnxLim[AX_XAXIS].Max = dAxisXMax;
      mnxLim[AX_YAXIS].Min = dAxisYMin;
      mnxLim[AX_YAXIS].Max = dAxisYMax;
   }
   //===Plot Data=========================================
   xPrv = prc->left-1;

   //---Min/Max method--------------------------
   if((iPtMax-iPtMin) > 2*(prc->right-prc->left)) {
      YMn = YMx = YMnPrev = YMxPrev = dYData[0];
      for(iPt=iPtMin; iPt<iPtMax; iPt++) {
         x = prc->left + (int) (dSclX*(fnXData(iPt)-dMinX));
         if((x < prc->left) || (x > prc->right)) continue;
         if(x == xPrv) {
            if(dYData[iPt] < YMn) YMn = dYData[iPt];
            if(dYData[iPt] > YMx) YMx = dYData[iPt];
         } else {
            if((xPrv>=prc->left) && (xPrv<=prc->right)) {
               //---Catch previous-------
               if(YMxPrev < YMn) YMn = YMxPrev;
               if(YMnPrev > YMx) YMx = YMnPrev;
               //---Map to screen--------
               yMn = prc->bottom - (int)(dSclY * (YMn - dMinY));
               yMx = prc->bottom - (int)(dSclY * (YMx - dMinY));
               //if((yMn<prc->top) && (yMx<prc->top)) continue;
               //if((yMn>prc->bottom) && (yMx>prc->bottom)) continue;
               if(yMn < prc->top)    yMn = prc->top;
               if(yMn > prc->bottom) yMn = prc->bottom;
               if(yMx < prc->top)    yMx = prc->top;
               if(yMx > prc->bottom) yMx = prc->bottom;
               GDIMoveTo(x, yMx, pVoid);
               GDILineTo(x, yMn+1, pVoid); // note: last pixel not painted!
            }
            YMnPrev = YMn; YMxPrev = YMx;      // store for next time
            YMn = YMx = dYData[iPt];           // start next scan
            xPrv = x;
         }
      }
   //---Standard--------------------------------
   } else {
      tfMove = TRUE;
      for(iPt=iPtMin; iPt<=iPtMax; iPt++) {
         if((dXData!=NULL) && (fnXData(iPt)==0.00) && (dYData[iPt]==0.00)) {
            tfMove = TRUE;                     // skip (0, 0) unfilled points
            continue;
         }
         x = prc->left + (int) (dSclX * (fnXData(iPt) - dMinX));
         if(x == xPrv) continue;
         if((x >= prc->left) && (x <= prc->right)) {
            y = prc->bottom - (int)(dSclY * (dYData[iPt] - dMinY));
            if((y <= prc->bottom) && (y >= prc->top)) {
               if(tfMove) {
                  GDIMoveTo(x, y, pVoid);
               } else {
                  GDILineTo(x, y, pVoid);
               }
               tfMove = FALSE;
            } else tfMove = TRUE;
            xPrv = x;                    // track pixels lined to
         } else tfMove = TRUE;
      }
   }
}


/*********************************************************
*  PaintData functions
*********************************************************/
//===Data Line============================================
void CAxes::PaintDataLine(double dXData[], double dYData[], int iNm, void *pVoid, int iStp) {
   int    x0 = 0;                           // RESERVED Graphics offset for..
   int    y0 = 0;                           // RESERVED..memory DC painting (see StepWin)
   int    iPt;                              // point loop counter

   for(iPt = 0; iPt < (iNm-1)*iStp; iPt += iStp) {
      if((fnXData(iPt)>=mnxLim[AX_XAXIS].Min) && (fnXData(iPt)<=mnxLim[AX_XAXIS].Max) && (fnXData(iPt)!=AX_NAN)
            &&(dYData[iPt]>=mnxLim[AX_YAXIS].Min) && (dYData[iPt]<=mnxLim[AX_YAXIS].Max) && (dYData[iPt]!=AX_NAN)
            &&(fnXData(iPt+iStp)>=mnxLim[AX_XAXIS].Min) && (fnXData(iPt+iStp)<=mnxLim[AX_XAXIS].Max) && (fnXData(iPt+iStp)!=AX_NAN)
            &&(dYData[iPt+iStp]>=mnxLim[AX_YAXIS].Min) && (dYData[iPt+iStp]<=mnxLim[AX_YAXIS].Max) && (dYData[iPt+iStp]!=AX_NAN)
      ){//*/
         GDIMoveTo(Plane2ClientX(fnXData(iPt))     - x0, Plane2ClientY(dYData[iPt])     - y0, pVoid);
         GDILineTo(Plane2ClientX(fnXData(iPt+iStp))- x0, Plane2ClientY(dYData[iPt+iStp])- y0, pVoid);
      }//endif(not-nan, bounded)
   }//endfor(iPt)
}

//===Data Marker==========================================
const int nxyPls[] = { 6,   0,-1000,  0,1000,   0,   0,-1000,  0,1000,   0,   0,   0};
const int nxyCrs[] = { 6,-707,-707, 707, 707,   0,   0,-707, 707, 707,-707,   0,   0};
const int nxyCir[] = {15,1000,   0, 901, 434, 623, 782, 223, 975,-223, 975,-623, 782,-901, 434,-1000,  0,-901,-434,-623,-782,-223,-975, 223,-975, 623,-782, 901,-434,1000,  -0};
const int nxySqr[] = {5, -900,-900, 900,-900, 900, 900,-900, 900,-900,-900};
const int nxyDia[] = {5,    0,1000,-1000,  0,   0,-1000,1000,  0,   0,1000};
const int nxyVup[] = {4,    0,1000,-800,-600, 800,-600,   0,1000};
const int nxyVdn[] = {4,    0,-1000,-800,600, 800, 600,   0,-1000};

void CAxes::PaintDataMarker(double dXData[], double dYData[], int iNm, void *pVoid, int iStp, int iMrkr, int iSze) {
   int    x0 = 0;                           // RESERVED Graphics offset for..
   int    y0 = 0;                           // RESERVED..memory DC painting (see StepWin)
   int    iPt;                              // data point loop counter
   int    iNumMk, iMk;                      // marker vertex counter
   int   *pxy;                              // pointer into array
   int    iXd[15], iYd[15];                 // recalculated offsets (ensure big enough!)

   //---Prepare offset array--------------------
   pxy = (int*) (
         (iMrkr==AX_MKRPLUS)    ? nxyPls :
         (iMrkr==AX_MKRCROSS)   ? nxyCrs :
         (iMrkr==AX_MKRCIRC)    ? nxyCir :
         (iMrkr==AX_MKRSQUARE)  ? nxySqr :
         (iMrkr==AX_MKRDIAMOND) ? nxyDia :
         (iMrkr==AX_MKRVUP)     ? nxyVup :
         (iMrkr==AX_MKRVDOWN)   ? nxyVdn : nxyPls );
   iNumMk = *pxy++;
   for(iMk=0; iMk<iNumMk; iMk++) {
      iXd[iMk] = (*pxy++) * iSze / 1000;
      iYd[iMk] = (*pxy++) * iSze / 1000;
   }

   for(iPt = 0; iPt < iNm*iStp; iPt += iStp) {
      if((fnXData(iPt)>=mnxLim[AX_XAXIS].Min) && (fnXData(iPt)<=mnxLim[AX_XAXIS].Max) && (fnXData(iPt)!=AX_NAN)
            &&(dYData[iPt]>=mnxLim[AX_YAXIS].Min) && (dYData[iPt]<=mnxLim[AX_YAXIS].Max) && (dYData[iPt]!=AX_NAN)
      ){
         GDIMoveTo(Plane2ClientX(fnXData(iPt)) + iXd[0]- x0, Plane2ClientY(dYData[iPt]) + iYd[0] - y0, pVoid);
         for(iMk=1; iMk<iNumMk; iMk++) {
            GDILineTo(Plane2ClientX(fnXData(iPt)) + iXd[iMk]- x0, Plane2ClientY(dYData[iPt]) + iYd[iMk] - y0, pVoid);
         }//endfor(iMk)
      }//endif(non-nan, bounded)
   }//endfor(iPt)
}

//===Data Envelope========================================
void CAxes::PaintDataEnvelope(double dXData[], double dYData[], int iNm, void *pVoid, int iStp) {
   int      x0 = 0;                         // RESERVED Graphics offset for..
   int      y0 = 0;                         // RESERVED..memory DC painting (see StepWin)
   int      iScrx;                          // client coordinate
   int      iIndx;                          // index into data
   AXMINMAX mnxY;                           // extremes of data in current range

   iIndx = 0;
   for(iScrx = rcPos.left; (iScrx<=rcPos.right)&&(iIndx<iNm*iStp) ; iScrx++) {
      if((iScrx >= Plane2ClientX(mnxData[AX_XAXIS].Min)) && (iScrx <= Plane2ClientX(mnxData[AX_XAXIS].Max))) {
         mnxY.Min = mnxY.Max = dYData[iIndx];
         while((Plane2ClientX(fnXData(iIndx))<=iScrx) && (iIndx<iNm*iStp)) {
            if(dYData[iIndx] < mnxY.Min) mnxY.Min = dYData[iIndx];
            if(dYData[iIndx] > mnxY.Max) mnxY.Max = dYData[iIndx];
            iIndx += iStp;                     // move along array as long as within sampling point
         }
         GDIMoveTo(iScrx - x0, Plane2ClientY(mnxY.Min)   - y0, pVoid);
         GDILineTo(iScrx - x0, Plane2ClientY(mnxY.Max)-1 - y0, pVoid);
         if(iIndx > iStp) iIndx -= iStp;       // make line continuous
      }
   }
}


/*********************************************************
*  PaintPolygon
*********************************************************/
void CAxes::PaintDataPolygon(double dXData[], double dYData[], int iNm, void *pVoid, int iStp) {
   int    x0 = 0;                           // RESERVED Graphics offset for..
   int    y0 = 0;                           // RESERVED..memory DC painting (see StepWin)
   int    i;                                // point loop counter
   POINT *pts;                              // array data coordinates

   pts = (POINT*) malloc(iNm * sizeof(POINT)); // assign data array
   if(!pts) return;                         // abort if there's a problem

   for(i=0; i<iNm; i++) {
      pts[i].x = Plane2ClientX(fnXData(i*iStp)) - x0;
      pts[i].y = Plane2ClientY(dYData[i*iStp]) - y0;
   }

   GDIPolygon(pts, iNm, pVoid);

   free(pts);                               // free allocated memory
   return;
}


/*********************************************************
* PaintArrow                                         rev6
* Useful for painting FWHM arrows. Currently only horizon-
* tal arrows are supported. Use negative size to reverse
* direction of arrow. The arrow size is in screen units.
*********************************************************/
void CAxes::PaintArrow(double dX, double dY, void *pVoid, double dSize) {
   int iX = Plane2ClientX(dX);
   int iY = Plane2ClientY(dY);

   if((iX >= rcPos.left) && (iX < rcPos.right) && (iY >= rcPos.top) && (iY < rcPos.bottom)) {
      GDIMoveTo(iX, iY, pVoid); GDILineTo(iX-(int)     dSize , iY-(int)(0.8*dSize), pVoid);
      GDIMoveTo(iX, iY, pVoid); GDILineTo(iX-(int)     dSize , iY+(int)(0.8*dSize), pVoid);
      GDIMoveTo(iX, iY, pVoid); GDILineTo(iX-(int)(2.5*dSize), iY                 , pVoid);
   }
}


/*********************************************************
* PaintAxisMarker                                    rev6
*********************************************************/
const int CiXAxMark[] = {0,  4, -4, 0};
const int CiYAxMark[] = {0, -8, -8, 0};
void CAxes::PaintAxisMarker(int iAx, double dVal, BOOL tfOpposite, void *pVoid) {
   int *piX, *piY;                          // distributed marker indexing
   int  x, y;                               // starting points
   int  iSgn;                               // direction of arrow
   int  k;                                  // loop counter

   switch(iAx) {
   case AX_XAXIS:
      if((dVal<mnxLim[iAx].Min) || (dVal>mnxLim[iAx].Max)) return;
      x    = Plane2ClientX(dVal);
      y    = tfOpposite ? rcPos.top-2 : rcPos.bottom+2;
      piX  = (int*) CiXAxMark; piY = (int*) CiYAxMark;
      iSgn = tfOpposite ? 1 : -1;
      break;
   case AX_YAXIS:
      if((dVal<mnxLim[iAx].Min) || (dVal>mnxLim[iAx].Max)) return;
      x    = tfOpposite ? rcPos.right+2 : rcPos.left-2;
      y    = Plane2ClientY(dVal);
      piX  = (int*) CiYAxMark; piY = (int*) CiXAxMark;
      iSgn = tfOpposite ? -1 : 1;
      break;
   }
   GDIMoveTo(x, y, pVoid);
   for(k=0; k<4; k++) GDILineTo(x+iSgn*piX[k], y+iSgn*piY[k], pVoid);
}



/*********************************************************
*  Mapping functions
*********************************************************/
int CAxes::Plane2ClientX(double X) {
   double dDelta, dAxScale;
   if((dDelta = (mnxLim[AX_XAXIS].Max - mnxLim[AX_XAXIS].Min)) == 0.00) dDelta = 1.00;
   dAxScale = (rcPos.right - rcPos.left) / dDelta;
   return(rcPos.left   + (X-mnxLim[AX_XAXIS].Min) * dAxScale);
}

int CAxes::Plane2ClientY(double Y) {
   double dDelta, dAxScale;
   if((dDelta = (mnxLim[AX_YAXIS].Max - mnxLim[AX_YAXIS].Min)) == 0.00) dDelta = 1.00;
   dAxScale = (rcPos.top - rcPos.bottom) / dDelta;
   return(rcPos.bottom + (Y-mnxLim[AX_YAXIS].Min) * dAxScale);
}

double CAxes::Client2PlaneX(int x) {
   int    iDelta;
   double dAxScale;
   if((iDelta = (rcPos.right - rcPos.left)) == 0) iDelta = 1;
   dAxScale = (mnxLim[AX_XAXIS].Max - mnxLim[AX_XAXIS].Min) / ((double) iDelta);
   return(mnxLim[AX_XAXIS].Min + (x - rcPos.left) * dAxScale);
}

double CAxes::Client2PlaneY(int y) {
   int    iDelta;
   double dAxScale;
   if((iDelta = (rcPos.top - rcPos.bottom)) == 0) iDelta = 1;
   dAxScale = (mnxLim[AX_YAXIS].Max - mnxLim[AX_YAXIS].Min) / ((double) iDelta);
   return(mnxLim[AX_YAXIS].Min + (y - rcPos.bottom) * dAxScale);
}


/*********************************************************
*  Position functions
*********************************************************/
void CAxes::SetPosition(LPRECT lprc) {
   SetPosition(lprc->left, lprc->top, lprc->right, lprc->bottom);
}

void CAxes::SetPosition(int iLeft, int iTop, int iRight, int iBottom) {
   rcPos.left   = iLeft;
   rcPos.top    = iTop;
   rcPos.right  = iRight;
   rcPos.bottom = iBottom;
}

LPRECT CAxes::GetPosition(void) {
   return((LPRECT) &rcPos);
}

void CAxes::GetPosition(LPRECT lprcOut) {
   lprcOut->left   = rcPos.left;
   lprcOut->top    = rcPos.top;
   lprcOut->right  = rcPos.right;
   lprcOut->bottom = rcPos.bottom;
}


/********************************************************
*  Axes limits functions
********************************************************/
BOOL CAxes::SetAxLim(int iAx, AXMINMAX mnxLimIn) {
   return( SetAxLim(iAx, mnxLimIn.Min, mnxLimIn.Max) );
}

BOOL CAxes::SetAxLim(int iAx, double dMn, double dMx) {
   if((iAx < 0) || (iAx >= AX_NUMAXES)) return(FALSE);
   mnxLim[iAx].Min = dMn;
   mnxLim[iAx].Max = dMx;
   return(TRUE);
}

AXMINMAX CAxes::GetAxLim(int iAx) {
   AXMINMAX mnx;
   mnx.Min = mnx.Max = 0.00;
   GetAxLim(iAx, &mnx);
   return(mnx);
}

BOOL CAxes::GetAxLim(int iAx, AXMINMAX *lpmnxOut) {
   if((iAx < 0) || (iAx >= AX_NUMAXES)) return(FALSE);
   lpmnxOut->Min = mnxLim[iAx].Min;
   lpmnxOut->Max = mnxLim[iAx].Max;
   return(TRUE);
}

/*********************************************************
*  Data limits functions
*********************************************************/
BOOL CAxes::SetDataLimit(double dXData[], double dYData[], int iNm, int iStp) {
   BOOL tfRet = TRUE;
   tfRet &= SetAxDataLimit(AX_XAXIS, dXData, iNm, iStp);
   tfRet &= SetAxDataLimit(AX_YAXIS, dYData, iNm, iStp);
   return(tfRet);
}

BOOL CAxes::SetAxDataLimit(int iAx, double dMin, double dMax) {
   if((iAx < 0) || (iAx >= AX_NUMAXES)) return(FALSE);
   mnxData[iAx].Min = dMin;
   mnxData[iAx].Max = dMax;
   return(TRUE);
}

BOOL CAxes::SetAxDataLimit(int iAx, AXMINMAX mnxData) {
   return( SetAxDataLimit(iAx, mnxData.Min, mnxData.Max) );
}

BOOL CAxes::SetAxDataLimit(int iAx, double dData[], int iNm, int iStp) {
   double dMn, dMx;
   int iPt;

   if((iAx < 0) || (iAx >= AX_NUMAXES)) return(FALSE);
   dMn = dMx = dData[0];
   for(iPt=iStp; iPt<iNm*iStp; iPt+=iStp) {
      if(dData[iPt] < dMn) dMn = dData[iPt];
      if(dData[iPt] > dMx) dMx = dData[iPt];
   }
   return( SetAxDataLimit(iAx, dMn, dMx) );
}

AXMINMAX CAxes::GetAxDataLimit(int iAx) {
   AXMINMAX mnx;
   mnx.Min = mnx.Max = 0.00;
   GetAxDataLimit(iAx, &mnx);
   return(mnx);
}

BOOL CAxes::GetAxDataLimit(int iAx, AXMINMAX *lpmnxOut) {
   if((iAx < 0) || (iAx >= AX_NUMAXES)) return(FALSE);
   lpmnxOut->Min = mnxData[iAx].Min;
   lpmnxOut->Max = mnxData[iAx].Max;
   return(TRUE);
}


/*********************************************************
*  Tick functions
*********************************************************/
//===Calculate and set ticks==============================
void CAxes::CalcTicks(void) {
   int iAx;
   for(iAx=0; iAx<AX_NUMAXES; iAx++) CalcAxTicks(iAx);
}

/*=========================================================
 CalcAxTicks - calculate ticks on an axis
 64-bit version (uses DOUBLE)
==========================================================*/
void CAxes::CalcAxTicks(int iAx) {
   AXMINMAX mnxAx;                          // local limits of axes
   double dDelta;                           // numerical extent of axes
   double dTemp;                            // temporary storage
   int    iLog;                             // logarithm of fDelta to eliminate sizes
   double dNormDelta;                       // normalised delta that ticks are assigned to
   double dvDiv[AX_NUMFVDIV];               // allowed normalised tick spacings
   int    iDivIndx;                         // index to tick spacing to be used
   double dUseDiv;                          // normalised tick spacing to be used
   double dAxMin, dAxMax, dAxMag;           // lowest, highest, and magnitude order
   int    iTicks;                           // the number of ticks assigned
   int    i;                                // generic loop counter
   double dTick;                            // current tick
   BOOL   tfUseClose;                       // if TRUE, use close fitting, otherwise loose

   if((iAx < 0) || (iAx >= AX_NUMAXES)) return; // catch poor axis number
   if(!(GetAxTickMode(iAx) & AXS_TICKAUTO)) return; // ignore if manual ticks
   //---Determine axis limit style---------------
   tfUseClose = (GetAxFit(iAx) == AXS_FITCLOSE) ? TRUE : FALSE; // get fitting style
   mnxAx      = GetAxDataLimit(iAx);        // get limits of data on axis

   //---Allowed normalised tick spacings---------
   dvDiv[0]  = 0.01;
   dvDiv[1]  = 0.02;
   dvDiv[2]  = 0.05;
   dvDiv[3]  = 0.10;
   dvDiv[4]  = 0.20;
   dvDiv[5]  = 0.50;
   dvDiv[6]  = 1.00;
   dvDiv[7]  = 2.00;
   dvDiv[8]  = 5.00;

   dDelta = fabs(mnxAx.Max - mnxAx.Min);
   dAxMag = fabs(mnxAx.Max) > fabs(mnxAx.Min) ? fabs(mnxAx.Max) : fabs(mnxAx.Min);
   if(dAxMag < 1e-3) dAxMag = 1.00;         // prevent #DIV/0! errors
   if((dDelta / dAxMag) < 1e-5) {           // if there is no spread,..
      mnxAx.Min = mnxAx.Min - 0.50;         //..move min and..
      mnxAx.Max = mnxAx.Max + 0.50;         //..max apart..
      dDelta = 1.00;                        //..to get a difference
   }
   if(fabs(dDelta) <= 1e-3) dDelta = 1.00;  // prevent #DIV/0! errors
   iLog = round(log10(dDelta));             // order of magnitude of dDelta
   dTemp = pow(10.00, iLog);
   if(dTemp < 1e-3) dTemp = 1.00;           // prevent #DIV/0! errors
   dNormDelta = dDelta / dTemp;             // normalise delta..
   if(dNormDelta < 1.00) {                  //..to be between 1 and 10
      dNormDelta *= 10.00;
      iLog -= 1;
   }

   //---Determine tick spacing-------------------
   if(GetAxMaxVisTick(iAx) < 2) SetAxMaxVisTick(iAx, 2); // NOT NECESSARY? *DEBUG*
   if(GetAxMaxVisTick(iAx) > AX_MAXTICKS) SetAxMaxVisTick(iAx, AX_MAXTICKS); // limited by AXES storage space

   iDivIndx = -1;                           // search bottom-up through array of allowed values
   do {
      iDivIndx += 1;                        // try next division value
      if(iDivIndx >= AX_NUMFVDIV) break;    // ignore if it's beyond end ****THIS MIGHT BE A PROBLEM****
      dUseDiv = dvDiv[iDivIndx] * pow(10.00,iLog);
      dAxMin = GETAXMIN(mnxAx.Min, dUseDiv, tfUseClose);
      dAxMax = GETAXMAX(mnxAx.Max, dUseDiv, tfUseClose);
      if(dUseDiv == 0.00) dUseDiv = 1.00;   // prevent #DIV/0! errors
      iTicks = round((dAxMax - dAxMin) / dUseDiv);
   } while(iTicks > GetAxMaxVisTick(iAx));

   if(iTicks > AX_MAXTICKS) iTicks = AX_MAXTICKS;	// this should never happen

   SetAxLim(iAx,
      tfUseClose ? mnxAx.Min : dAxMin,
      tfUseClose ? mnxAx.Max : dAxMax);
   SetAxNumTick(iAx, iTicks+1);

   for(i=0; i <= iTicks; i++) {
      dTick = dAxMin + i*dUseDiv;
      if(fabs(dTick / dAxMag) < 1e-5) dTick = 0.00; // crop small numbers to 0.00
      SetAxTick(iAx, i, dTick);             // set each tick
   }
   return;
}

BOOL CAxes::SetAxTick(int iAx, int iTk, double dVal) {
   if((iAx < 0) || (iAx >= AX_NUMAXES)) return(FALSE);
   if((iTk < 0) || (iTk > AX_MAXTICKS)) return(FALSE);
   dTick[iAx][iTk] = dVal;
   if(GetAxNumTick(iAx) < iTk) SetAxNumTick(iAx, iTk);
   return(TRUE);
}

BOOL CAxes::SetAxTickList(int iAx, int iNm, double dVal[]) {
   if((iAx < 0) || (iAx >= AX_NUMAXES)) return(FALSE);
   for(int i=0; (i<iNm) && (i<=AX_MAXTICKS); i++) {
      dTick[iAx][i] = dVal[i];
   }
   if(GetAxNumTick(iAx) < iNm) SetAxNumTick(iAx, iNm);
   return(TRUE);
}

BOOL CAxes::SetAxNumTick(int iAx, int iNm) {
   if((iAx < 0) || (iAx >= AX_NUMAXES)) return(0);
   iNumTick[iAx] = iNm;
   if(iNumTick[iAx] > AX_MAXTICKS+1) {
      iNumTick[iAx] = AX_MAXTICKS+1;
      return(FALSE);
   }
   return(TRUE);
}

int CAxes::GetAxNumTick(int iAx) {
   if((iAx < 0) || (iAx >= AX_NUMAXES)) return(0);
   return(iNumTick[iAx]);
}

//===Tick Sizes===========================================
BOOL CAxes::SetTickSize(int iTkSz) {
   BOOL tfRet = TRUE;
   for(int iAx=0; iAx<AX_NUMAXES; iAx++) {
      tfRet &= SetAxTickSize(iAx, iTkSz);
   }
   return(tfRet);
}

BOOL CAxes::SetAxTickSize(int iAx, int iTkSz) {
   if((iAx<0) || (iAx>=AX_NUMAXES)) return(FALSE);
   iTickSize[iAx] = iTkSz;
   return(TRUE);
}

int CAxes::GetAxTickSize(int iAx) {
   if((iAx<0) || (iAx>=AX_NUMAXES)) return(0);
   return(iTickSize[iAx]);
}

//===Tick Modes===========================================
BOOL CAxes::SetTickMode(UINT uTs) {
   BOOL tfRet = TRUE;
   for(int iAx=0; iAx<AX_NUMAXES; iAx++) {
      tfRet &= SetAxTickMode(iAx, uTs);
   }
   return(tfRet);
}

BOOL CAxes::SetTickAuto(void) {
   BOOL tfRet = TRUE;
   for(int iAx=0; iAx<AX_NUMAXES; iAx++) {
      tfRet &= SetAxTickAuto(iAx);
   }
   return(tfRet);
}

BOOL CAxes::SetTickManual(void) {
   BOOL tfRet = TRUE;
   for(int iAx=0; iAx<AX_NUMAXES; iAx++) {
      tfRet &= SetAxTickManual(iAx);
   }
   return(tfRet);
}

BOOL CAxes::SetAxTickMode(int iAx, UINT uTs) {
   if((iAx < 0) || (iAx >= AX_NUMAXES)) return(0);
   if(uTs) {
      uFlags[iAx] |= AXS_TICKAUTO;
   } else {
      uFlags[iAx] &= ~AXS_TICKCALCSTYLE;
   }
   return(TRUE);
}

BOOL CAxes::SetAxTickAuto(int iAx) {
   return(SetAxTickMode(iAx, AXS_TICKAUTO));
}

BOOL CAxes::SetAxTickManual(int iAx) {
   return(SetAxTickMode(iAx, AXS_TICKMANUAL));
}

UINT CAxes::GetAxTickMode(int iAx) {
   if((iAx < 0) || (iAx >= AX_NUMAXES)) return(0);
   return(uFlags[iAx] & AXS_TICKCALCSTYLE);
}

//===Tick Styles==========================================
// styles AXS_TICKIN, AXS_TICKOUT, AXS_TICKCROSS
BOOL CAxes::SetTickStyle(UINT uTkSty) {
   BOOL tfRet = TRUE;
   for(int iAx=0; iAx<AX_NUMAXES; iAx++) {
      tfRet &= SetAxTickStyle(iAx, uTkSty);
   }
   return(tfRet);
}

BOOL CAxes::SetTickNone(void) {
   return(SetTickStyle(AXS_TICKNONE));
}
BOOL CAxes::SetTickIn(void) {
   return(SetTickStyle(AXS_TICKIN));
}
BOOL CAxes::SetTickOut(void) {
   return(SetTickStyle(AXS_TICKOUT));
}
BOOL CAxes::SetTickCross(void) {
   return(SetTickStyle(AXS_TICKCROSS));
}

BOOL CAxes::SetAxTickStyle(int iAx, UINT uTkSty) {
   if((iAx<0) || (iAx>=AX_NUMAXES)) return(FALSE);
   uFlags[iAx] &= ~AXS_TICKSTYLE;
   uFlags[iAx] |= (uTkSty & AXS_TICKSTYLE);
   return(TRUE);
}

BOOL CAxes::SetAxTickNone(int iAx) {
   return(SetAxTickStyle(iAx, AXS_TICKNONE));
}
BOOL CAxes::SetAxTickIn(int iAx) {
   return(SetAxTickStyle(iAx, AXS_TICKIN));
}
BOOL CAxes::SetAxTickOut(int iAx) {
   return(SetAxTickStyle(iAx, AXS_TICKOUT));
}
BOOL CAxes::SetAxTickCross(int iAx) {
   return(SetAxTickStyle(iAx, AXS_TICKCROSS));
}

UINT CAxes::GetAxTickStyle(int iAx) {
   if((iAx<0) || (iAx>=AX_NUMAXES)) return(0);
   return(uFlags[iAx] & AXS_TICKSTYLE);
}

//===Visible Ticks========================================
// MaxVisTick is used by CalcAxTick
BOOL CAxes::SetAxMaxVisTick(int iAx, int iNmTk) {
   if((iAx < 0) || (iAx >= AX_NUMAXES)) return(FALSE);
   iMaxNumVisTick[iAx] = iNmTk;
   if(iMaxNumVisTick[iAx] > AX_MAXTICKS+1) {
      iMaxNumVisTick[iAx] = AX_MAXTICKS+1;
      return(FALSE);
   }
   return(TRUE);
}

int CAxes::GetAxMaxVisTick(int iAx) {
   if((iAx < 0) || (iAx >= AX_NUMAXES)) return(0);
   return(iMaxNumVisTick[iAx]);
}

/*********************************************************
*  Fit functions
*********************************************************/
BOOL CAxes::SetFit(UINT uFt) {
   BOOL tfRet = TRUE;
   for(int iAx=0; iAx<AX_NUMAXES; iAx++) {
      tfRet &= SetAxFit(iAx, uFt);
   }
   return(tfRet);
}

BOOL CAxes::SetFitClose(void) {
   BOOL tfRet = TRUE;
   for(int iAx=0; iAx<AX_NUMAXES; iAx++) {
      tfRet &= SetAxFitClose(iAx);
   }
   return(tfRet);
}

BOOL CAxes::SetFitLoose(void) {
   BOOL tfRet = TRUE;
   for(int iAx=0; iAx<AX_NUMAXES; iAx++) {
      tfRet &= SetAxFitLoose(iAx);
   }
   return(tfRet);
}

BOOL CAxes::SetAxFit(int iAx, UINT uFt) {
   if((iAx < 0) || (iAx >= AX_NUMAXES)) return(FALSE);
   if(uFt) {
      uFlags[iAx] |= AXS_FITLOOSE;          // clear loose-fit bit
   } else {
      uFlags[iAx] &= ~AXS_FITSTYLE;         // set loose-fit bit
   }
   return(TRUE);
}

BOOL CAxes::SetAxFitClose(int iAx) {
   return(SetAxFit(iAx, AXS_FITCLOSE));
}

BOOL CAxes::SetAxFitLoose(int iAx) {
   return(SetAxFit(iAx, AXS_FITLOOSE));
}

UINT CAxes::GetAxFit(int iAx) {
   if((iAx < 0) || (iAx >= AX_NUMAXES)) return(0);
   return(uFlags[iAx] & AXS_FITSTYLE);
}


/*********************************************************
*  Location functions
*********************************************************/
BOOL CAxes::SetAxLocation(int iAx, UINT uLoc) {
   if((iAx < 0) || (iAx >= AX_NUMAXES)) return(0);
   uFlags[iAx] &= ~AXS_AXISLOCSTYLE;        // clear the location bits
   uFlags[iAx] |= (uLoc & AXS_AXISLOCSTYLE); // set location bits
   return(TRUE);
}

UINT CAxes::GetAxLocation(int iAx) {
   if((iAx < 0) || (iAx >= AX_NUMAXES)) return(0);
   return(uFlags[iAx] & AXS_AXISLOCSTYLE);
}


/*********************************************************
*  Box functions
*********************************************************/
BOOL CAxes::SetBox(UINT uBx) {
   BOOL tfRet = TRUE;
   for(int iAx=0; iAx<AX_NUMAXES; iAx++) {
      tfRet &= SetAxBox(iAx, uBx);
   }
   return(tfRet);
}

BOOL CAxes::SetBoxOn(void) {
   BOOL tfRet = TRUE;
   for(int iAx=0; iAx<AX_NUMAXES; iAx++) {
      tfRet &= SetAxBoxOn(iAx);
   }
   return(tfRet);
}

BOOL CAxes::SetBoxOff(void) {
   BOOL tfRet = TRUE;
   for(int iAx=0; iAx<AX_NUMAXES; iAx++) {
      tfRet &= SetAxBoxOff(iAx);
   }
   return(tfRet);
}

BOOL CAxes::SetAxBox(int iAx, UINT uBx) {
   if((iAx < 0) || (iAx >= AX_NUMAXES)) return(FALSE);
   if(uBx) {
      uFlags[iAx] |= AXS_BOXON;
   } else {
      uFlags[iAx] &= ~AXS_BOXSTYLE;
   }
   return(TRUE);
}

BOOL CAxes::SetAxBoxOn(int iAx) {
   return(SetAxBox(iAx, AXS_BOXON));
}

BOOL CAxes::SetAxBoxOff(int iAx) {
   return(SetAxBox(iAx, AXS_BOXOFF));
}

UINT CAxes::GetAxBox(int iAx) {
   if((iAx < 0) || (iAx >= AX_NUMAXES)) return(0x0000);
   return(uFlags[iAx] & AXS_BOXSTYLE);
}


/*********************************************************
*  Label functions
*********************************************************/
BOOL CAxes::SetLabel(const char *cszXLab, const char *cszYLab) {
   BOOL tfRet = TRUE;
   tfRet &= SetAxLabel(AX_XAXIS, cszXLab);
   tfRet &= SetAxLabel(AX_YAXIS, cszYLab);
   return(tfRet);
}

BOOL CAxes::SetAxLabel(int iAx, const char *cszLab) {
   if((iAx < 0) || (iAx >= AX_NUMAXES)) return(FALSE);
   strncpy(szLabel[iAx], cszLab, AX_MAXLABELLEN);
   return(TRUE);
}

void CAxes::GetAxLabel(int iAx, char *szBuffer, size_t iBufSize) {
   if((iAx < 0) || (iAx >= AX_NUMAXES)) {
      *szBuffer = '\0';
      return;
   }
   strncpy(szBuffer, szLabel[iAx], iBufSize);
   return;
}


/*********************************************************
*  Flag functions
*********************************************************/
BOOL CAxes::SetAxFlags(int iAx, UINT uFlgs) {
   if((iAx<0) || (iAx>=AX_NUMAXES)) return(FALSE);
   uFlags[iAx] = uFlgs;
   return(TRUE);
}

UINT CAxes::GetAxFlags(int iAx) {
   if((iAx<0) || (iAx>=AX_NUMAXES)) return(0x0000);
   return(uFlags[iAx]);
}

/*********************END CAXES.CPP**********************/
