/*********************************************************
* CAxes.h
*
* A class for displaying axes and arrays of data.
* The class  CAxes and its member functions  form an easy
* interface  for displaying,  for example  onto a  device
* context, simple 2D plotting axes. The axes offer a num-
* ber of common options including axes placement and tick
* styles. The ticks can be calculated automatically or be
* assigned manually.
*
* The class  implements primitive  2D plotting  functions
* for  arrays of data.  Alternatively, translation  func-
* tions map plotting points  to window client coordinates
* and vice versa.
*
* The graphics output functions  are declared virtual and
* can be  overloaded for different display  environments.
* This flexibility  is implemented with the  GDI___ func-
* tions. If  the functions are not overloaded,  a pointer
* to the current DC must be passed in pVoid.
*
* Example code:

  CAxes ax;
  HDC hdc;
  double xdata[] = {-4.,-3.,-2.,-1., 0., 1., 2., 3., 4.};
  double ydata[] = {-6.,-3.,-1., 0., 0., 1., 2., 4., 7.};
  ax.SetPosition(30, 20, 230, 170);
  ax.SetAxLabel(AX_XAXIS, "X Coordinate");
  ax.SetAxLimFromData(AX_XAXIS, xdata, 9);
  ax.SetAxLimFromData(AX_YAXIS, ydata, 9);
  ax.CalcTicks();
  ax.PaintDataLine(xdata, ydata, 9, (void*) &hdc);
  ax.Paint((void*) &hdc);

*  $PSchlup 2000-2006 $      $Revision 3 $
* Revision History:
*    3  Addition of PaintData, an  all-inclusive function
*       including  normalization and flags. AX_MKR  cons-
*       tants extended to  become flags (values changed),
*       added limits options to Paint[axes]
*    2  Inclusion in LaserCanv5
*  0.4  Added PaintDataMarker primitive
*  0.3  Included PaintDataLine and PaintDataPolygon
*       with strides
*  0.2  Port to Win32 (Borland C++BuilderX)
*  Original AXES structure written in St Andrews
*********************************************************/

#ifndef _CAXES
#define _CAXES

#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

class CAxes;

/*********************************************************
*  Constants and Macros
*********************************************************/
//===Inline Functions============================
#ifndef round
#define round(a) ((int) floor(a+0.5))
#endif /*round*/

//-Tick Fit Functions----------------------------
#define GETAXMIN(fMin,UseDiv,tfUseClose) \
 (tfUseClose? ceil(fMin/UseDiv)*UseDiv : floor(fMin/UseDiv)*UseDiv)
#define GETAXMAX(fMax,UseDiv,tfUseClose) \
 (tfUseClose? floor(fMax/UseDiv)*UseDiv : ceil(fMax/UseDiv)*UseDiv)

//---Axes Style Constants------------------------
#define AX_NAN            HUGE_VAL          // these points aren't plotted

#define AX_XAXIS                 0          // X-axis index
#define AX_YAXIS                 1          // Y-axis index

#define AX_NUMAXES               2          // number of axes per axes class
#define AX_MAXTICKS             47          // max number of ticks per axis
#define AX_MAXLABELLEN          31          // maximum string length for label
#define AX_NUMFVDIV              9          // for CalcAxTicks: Number of normalized ticks

#define AXS_TICKNONE        0x0000          // iTickStyle: No ticks
#define AXS_TICKIN          0x0001          //  Ticks inside axis
#define AXS_TICKOUT         0x0002          //  Ticks outside axis
#define AXS_TICKCROSS       0x0003          //  Ticks cross axis
#define AXS_TICKSTYLE       0x000F          // bit mask for tick style

#define AXS_AXISNONE        0x0000          // i#AxisLocation: no axes
#define AXS_AXISLEFT        0x0100          //  Y axis at left
#define AXS_AXISRIGHT       0x0200          //  Y axis at right
#define AXS_AXISBOTTOM      0x0100          //  X axis at bottom
#define AXS_AXISTOP         0x0200          //  X axis at top
#define AXS_AXISLOCSTYLE    0x0300          // bit mask for axis location

#define AXS_FITCLOSE        0x0000          // #AxisLimStyle: fit close to data points
#define AXS_FITLOOSE        0x0400          //  Fit loosely to data points (extend to next nearest tick)
#define AXS_FITSTYLE        0x0400          // bit mask for axis limit style

#define AXS_TICKMANUAL      0x0000          // set ticks manually
#define AXS_TICKAUTO        0x0800          // always auto-calculate ticks
#define AXS_TICKCALCSTYLE   0x0800          // bit mask for tick calculation

#define AXS_BOXOFF          0x0000          // #AxisBox: no box
#define AXS_BOXON           0x1000          //  box on
#define AXS_BOXSTYLE        0x1000          // bit mask for box style

//---For PaintData--------------------
// AX_MKR also for PaintDataMarker
#define AX_MKRNONE          0x0000          // no marker
#define AX_MKRPLUS          0x0002          // + marker
#define AX_MKRCROSS         0x0003          // X marker
#define AX_MKRCIRC          0x0004          // O marker
#define AX_MKRSQUARE        0x0005          // square marker
#define AX_MKRDIAMOND       0x0006          // diamond marker
#define AX_MKRVUP           0x0007          // triangle up marker
#define AX_MKRVDOWN         0x0008          // triangle down marker
#define AX_MKR              0x00FF          // marker mask

#define AX_NORMX            0x0100          // normalize X-data
#define AX_NORMY            0x0200          // normalize Y-data

//===Typedefs=============================================
typedef int AXLIMSTYLE;

typedef struct tagAXMINMAX { /*amnx*/
   double Min;
   double Max;
} AXMINMAX, *PAXMINMAX;

/*********************************************************
*  CAxes Class Declaration
*********************************************************/
class CAxes {
private:
   RECT       rcPos;                        // axes position in screen units
   AXMINMAX   mnxLim[AX_NUMAXES];           // min and max values of axis
   AXMINMAX   mnxData[AX_NUMAXES];          // min and max values of data
   AXLIMSTYLE AxisLimStyle[AX_NUMAXES];     // close or loose axes to data points
   int        iNumTick[AX_NUMAXES];         // number of ticks on axis
   int        iMaxNumVisTick[AX_NUMAXES];   // maximum number of ticks visible on screen (used in CalcTick)
   double     dTick[AX_NUMAXES][AX_MAXTICKS+1];// position of ticks in axis units
   UINT       uFlags[AX_NUMAXES];           // axes characteristics
   char       szLabel[AX_NUMAXES][AX_MAXLABELLEN+1]; // axis label
   int        iTickSize[AX_NUMAXES];        // length of ticks in screen units
public:
   CAxes(void);                             // constructor (set defaults)

   //---GDI functions---------------------------
   // Overload these functions for custom graphics implementations
   // Use argument pVoid when calling Paint___(..) for application-
   // specific data
   virtual BOOL GDIMoveTo(int x, int y, void *pVoid);
   virtual BOOL GDILineTo(int x, int y, void *pVoid);
   virtual BOOL GDIGetTextExtent(char *szStr, int iLen, int *pCx, int *pCy, void *pVoid);
   virtual BOOL GDITextOut(int x, int y, char *szStr, int iLen, void *pVoid);
   virtual BOOL GDIPolygon(POINT pts[], int iNm, void *pVoid);

   //---Paint functions-------------------------
   void   Paint(void *pVoid, BOOL tfLimits=FALSE); // paint the axes
   void   PaintData(double dXData[], double dYData[], int iNm, void *pVoid, UINT uFlags=0x0000); // rev. 3 new paint routine
   void   PaintDataLine(double dXData[], double dYData[], int iNm, void *pVoid, int iStp=1); // paint a series of data points
   void   PaintDataMarker(double dXData[], double dYData[], int iNm, void *pVoid, int iStp=1, int iMrkr=0, int iSze=4); // paint a series of data points as marker
   void   PaintDataEnvelope(double dXData[], double dYData[], int iNm, void *pVoid, int iStp=1); // paint an envelope of data - use when array length >> window size
   void   PaintDataPolygon(double dXData[], double dYData[], int iNm, void *pVoid, int iStp=1); // paint a polygon through data points
   void   PaintArrow(double dX, double dY, void *pVoid, double dSize=6.00); // use negative size for left arrow
   void   PaintAxisMarker(int iAx, double dVal, BOOL tfOpp, void *pVoid); // paint a marker just outside the axis

   //---Mapping functions-----------------------
   int    Plane2ClientX(double X);          // client pixel for given drawing plane coordinate
   int    Plane2ClientY(double Y);          // client pixel for given drawing plane coordinate
   double Client2PlaneX(int x);             // drawing plane for given client pixel coordinate
   double Client2PlaneY(int y);             // drawing plane for given client pixel coordinate

   //---Position functions----------------------
   void   SetPosition(LPRECT lprc);         // set position to a rectangle
   void   SetPosition(int iLeft, int iTop, int iRight, int iBottom); // set position of axes
   LPRECT GetPosition(void);                // retrieve position rectangle
   void   GetPosition(LPRECT lprcOut);      // retrieve position rectangle

   //---Axes limits functions-------------------
   BOOL   SetAxLim(int iAx, AXMINMAX mnxLim); // set axes limits
   BOOL   SetAxLim(int iAx, double dMn, double dMx); // set axes limits
   AXMINMAX GetAxLim(int iAx);              // retrieve axes limits
   BOOL   GetAxLim(int iAx, AXMINMAX *lpmnxOut); // retrieve axes limits

   //---Data limits functions-------------------
   BOOL   SetDataLimit(double dXData[], double dYData[], int iNm, int iStp=1); // calculate limits from data
   BOOL   SetAxDataLimit(int iAx, double dMin, double dMax); // set axis data limit
   BOOL   SetAxDataLimit(int iAx, AXMINMAX mnxData); // set axis data limit
   BOOL   SetAxDataLimit(int iAx, double dData[], int iNm, int iStp=1); // set limits of data on an axis
   AXMINMAX GetAxDataLimit(int iAx);        // retrieve data limit for axis
   BOOL   GetAxDataLimit(int iAx, AXMINMAX *lpmnxOut); // retrieve data limit for axis

   //---Tick functions--------------------------
   void   CalcTicks(void);                  // calculate ticks for all axes
   void   CalcAxTicks(int iAx);             // calculate tick positions for axis
   BOOL   SetAxTick(int iAx, int iTk, double dVal); // set a specific tick value
   BOOL   SetAxTickList(int iAx, int iNm, double dVal[]); // set a number of ticks from array
   BOOL   SetAxNumTick(int iAx, int iNmTk); // set number of ticks on axis (internal)
   int    GetAxNumTick(int iAx);            // retrieve number of ticks on axis

   BOOL   SetTickSize(int iTkSz);           // set screen size of ticks for all axes
   BOOL   SetAxTickSize(int iAx, int iTkSz); // set screen size of ticks for one axis
   int    GetAxTickSize(int iAx);           // retrieve screen tick size of axis

   // TickMode: AXS_TICKAUTO, AXS_TICKMANUAL
   BOOL   SetTickMode(UINT uTs);            // set all axes tick mode
   BOOL   SetTickAuto(void);                // set all axes tick modes to auto
   BOOL   SetTickManual(void);              // set all axes tick modes to manual
   BOOL   SetAxTickMode(int iAx, UINT uTs); // set axis tick mode
   BOOL   SetAxTickAuto(int iAx);           // set axis tick mode to auto
   BOOL   SetAxTickManual(int iAx);         // set axis tick mode to manual
   UINT   GetAxTickMode(int iAx);           // retrieve axis tick mode

   // TickStyle: AXS_TICKNONE, AXS_TICKIN, AXS_TICKOUT, AXS_TICKCROSS
   BOOL   SetTickStyle(UINT uSty);          // set all axes tick styles
   BOOL   SetTickNone(void);                // set all ticks to none
   BOOL   SetTickIn(void);                  // set all ticks in
   BOOL   SetTickOut(void);                 // set all ticks out
   BOOL   SetTickCross(void);               // set all ticks cross
   BOOL   SetAxTickStyle(int iAx, UINT uSty); // set axis tick style
   BOOL   SetAxTickNone(int iAx);           // set axis tick style to none
   BOOL   SetAxTickIn(int iAx);             // set axis tick style in
   BOOL   SetAxTickOut(int iAx);            // set axis tick style out
   BOOL   SetAxTickCross(int iAx);          // set axis tick style cross
   UINT   GetAxTickStyle(int iAx);          // retrieve axis tick style

   BOOL   SetAxMaxVisTick(int iAx, int iNmTk); // set maximum number of ticks for auto axis tick calc (int. use)
   int    GetAxMaxVisTick(int iAx);         // retrieve maximum number of ticks allowed (int. use)

   //---Fit functions---------------------------
   // Fit: AXS_FITCLOSE, AXS_FITLOOSE
   BOOL   SetFit(UINT uFt);                 // set fit style for all axes
   BOOL   SetFitClose(void);                // set tight-fitting axis limits in auto tick calc
   BOOL   SetFitLoose(void);                // set loose fit axis limits in auto tick calc
   BOOL   SetAxFit(int iAx, UINT uFt);      // set fit style for axis
   BOOL   SetAxFitClose(int iAx);           // set tight-fitting axis limits in auto tick calc
   BOOL   SetAxFitLoose(int iAx);           // set loose fit axis limits in auto tick calc
   UINT   GetAxFit(int iAx);                // retrieve axis fitting style for auto tick calc

   //---Location functions----------------------
   // Location: AXS_AXISBOTTOM, AXS_AXISTOP, AXS_AXISLEFT, AXS_AXISRIGHT
   BOOL   SetAxLocation(int iAx, UINT uLoc); // set axis location
   UINT   GetAxLocation(int iAx);           // retrieve location of axis

   //---Box functions---------------------------
   // Box: AXS_BOXON, AXS_BOXOFF
   BOOL   SetBox(UINT uBx);                 // set box style for all axes
   BOOL   SetBoxOn(void);                   // set box style on for all axes
   BOOL   SetBoxOff(void);                  // set box style off for all axes
   BOOL   SetAxBox(int iAx, UINT uBx);      // set box style for one axis
   BOOL   SetAxBoxOn(int iAx);              // set box style on for one axis
   BOOL   SetAxBoxOff(int iAx);             // set box style off for one axis
   UINT   GetAxBox(int iAx);                // retrieve box style for one axis

   //---Label functions-------------------------
   BOOL   SetLabel(const char *cszXLab, const char *cszYLab); // set X and Y axes labels
   BOOL   SetAxLabel(int iAx, const char *cszLab); // set an axis label string (NOTE: AX_MAXLABELLEN)
   void   GetAxLabel(int iAx, char *szBuffer, size_t iBufSize); // retrieve axis label string

   //---Flags functions-------------------------
   // Note: Flag values are not guaranteed to be forwards-compatible
   BOOL   SetAxFlags(int iAx, UINT uFlgs);  // set all axes flags
   UINT   GetAxFlags(int iAx);              // retrieve complete flags structure
};

#endif /*_CAXES*/
