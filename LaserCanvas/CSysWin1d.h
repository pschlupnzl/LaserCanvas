/*********************************************************
* CSysWin1d
* $PSchlup 2006 $     $Revision 0 $
*********************************************************/
#ifndef CSYSWIN1D_H                         // prevent multiple includes
#define CSYSWIN1D_H

//===Classes Defined======================================
class CSysWin1d;                            // linear system renderer

//===Included Files=======================================
#include <windows.h>                        // standard Windows include
#include <stdio.h>                          // standard header

#include "CSysWin.h"                        // base class header
#include "CSystem.h"                        // system class
#include "Renderer2d.h"                     // renderer icons
#include "CMouse.h"                         // mouse layer class
#include "CQuickEdit.h"                     // quick-edit
#include "caxes.h"                          // axes class

//===Constants============================================
#define C1DC_MINZOOM          1.00          // [mm] minimum zoom distance
#define C1DC_ZOOMHEIGHT         32          // pixel height of zoom bar

//---Properties-----------------------
#define C1DI_PROP_XMIN           0          // (index) min zoom position
#define C1DI_PROP_XMAX           1          // (index) max zoom position
#define C1DI_PROP_YMIN           2          // (index) min vertical scale
#define C1DI_PROP_YMAX           3          // (index) max vertical scale
#define C1DI_PROP_FLAGS          4          // (index) renderer flags
#define C1DI_NUM_PROP            5          // (count) number of properties

//---Rectangles-----------------------
#define C1DI_RECT_FULL           0          // full rectangle
#define C1DI_RECT_ZOOM           1          // selection rectangle in full vie
#define C1DI_NUM_RECT            2          // number of rectangles

//---Mouse----------------------------
#define C1D_ACT_AXES             0          // active for axes
#define C1D_ACT_XMIN             1          // active for x-minimum
#define C1D_ACT_XMAX             2          // active x-maximum
#define C1D_ACT_YMIN             3          // active for y-minimum
#define C1D_ACT_YMAX             4          // active y-maximum
#define C1D_ACT_ZOOMBOTH         5          // active to drag whole zoom rectangle
#define C1D_ACT_ZOOMMIN          6          // active to drag min zoom
#define C1D_ACT_ZOOMMAX          7          // active to drag max zoom

//---GDI------------------------------
#define C1DI_PEN_FULL            0          // (index) pen to frame full view
#define C1DI_PEN_ZOOM            1          // (index) pen to frame selection
#define C1DI_PEN_AXES            2          // (index) pen for axes
#define C1DI_NUM_PEN             3          // (count) number of pens

#define C1DI_BRUSH_FULL          0          // (index) background in full view
#define C1DI_BRUSH_ZOOM          1          // (index) selection in full view
#define C1DI_NUM_BRUSH           2          // (count) number of brushes

//---Flags----------------------------
#define C1DF_SHOWICONS      0x0001          // (bit) show icons
#define C1DF_SHOWWAIST      0x0002          // (bit) show waist locations

/*********************************************************
* CSysWin1d Class
* Why can these derived classes not have destructors?
*********************************************************/
class CSysWin1d : public CSysWin {
private:
   double  ddProp[C1DI_NUM_PROP];           // min, max of selected zoom area
   RECT    rcRect[C1DI_NUM_RECT];           // rectangles
   CAxes   Axes;                            // plotting axes
   CMouse  Mouse;                           // mouse and its actives
   CQuickEdit *pQEdit;                      // quick-edit control
   int     iXMouseOffs;                     // offset from mouse to limit

public:
   CSysWin1d(CSystem *pSys);                // constructor
   ~CSysWin1d();                            // overloaded destructor

   void    OnPaint(HDC hdcIn = NULL);       // paint the window
   void    OnResize(void);                  // resize function
   void    RefreshZoomActive(void);         // refresh just the zoom active position

   //---Limits----------------------------------
   double   XMin(void)                 { return(ddProp[C1DI_PROP_XMIN]); };
   double   XMax(void)                 { return(ddProp[C1DI_PROP_XMAX]); };
   double   YMin(void)                 { return(ddProp[C1DI_PROP_YMIN]); };
   double   YMax(void)                 { return(ddProp[C1DI_PROP_YMAX]); };
   void     UserSetXLim(double dMn, double dMx); // set axis x-limits
   void     UserSetYLim(double dMn, double dMx); // set axis x-limits

   //---Mapping functions-----------------------
   int     Zoom2Wnd(double dD);
   double  Wnd2Zoom(int id);

   //---Overloaded functions--------------------
   int     Type(void)                  { return(CSYSWINI_TYPE_1D); };
   void    Refresh(void);                   // refresh the renderer
   void    UpdateTitle(int iID);            // update title bar
   void    SaveSysWin(HANDLE hFile);        // save current renderer
   BOOL    LoadSysWin(const char *pszDataFile, char *pszMin, char *pszMax); // load renderer from file
   void    GraphPoint(int iRVar, int iPt);  // update a graphing point
   void    MenuCommand(int iCmd);           // process specific commands
   void    DebugPrint(char *psz, int *pInt); // debug text
   void    Print(HDC hdcPrint);             // print to printer
   static LRESULT CALLBACK WndProcSysWin1d(HWND, UINT, WPARAM, LPARAM); // window procedure

   //---Properties------------------------------
   void    PrepareProperties(BOOL tfAct);   // show / hide properties
   void    UpdateProperties(void);          // update properties
   BOOL    SysWinPropItemCallback(void *vData, UINT uID); // modified from property manager
   static BOOL _SysWinPropItemCallback(void *vData, UINT uID, void *pVoid); // callback from property manager

   //---Flags-----------------------------------
   // We want to save the flags to file, so it must be a DOUBLE property
   void    SetBit(UINT u)              { ddProp[C1DI_PROP_FLAGS] = (double)(((UINT)ddProp[C1DI_PROP_FLAGS]) | u); };
   void    ClearBit(UINT u)            { ddProp[C1DI_PROP_FLAGS] = (double)(((UINT)ddProp[C1DI_PROP_FLAGS]) &~u); };
   void    ToggleBit(UINT u)           { ddProp[C1DI_PROP_FLAGS] = (double)(((UINT)ddProp[C1DI_PROP_FLAGS]) ^ u); };
   UINT    CheckBit(UINT u)            { return( ((UINT)ddProp[C1DI_PROP_FLAGS]) & u ); };

   //---Mouse-----------------------------------
   static void _MouseCallback(int iMsg, int x, int y, int wKeys, void *pData, long int lData);
   void MouseCallback(int iMsg, int x, int y, int wKeys, long int lData);
   static BOOL _QEditCallback(void *pVal, int iNext, void *pVoid, long int lLong);
   BOOL QEditCallback(void *pVal, int iNext, long int lLong);

   //---Tables----------------------------------
   static const char *CszSysWin1dProp[];    // properties saved by this renderer
   static const UINT CuProperties[];        // properties revealed by this renderer
};

#endif//CSYSWIN1D_H
