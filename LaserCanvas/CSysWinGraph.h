/*********************************************************
* CSysWinGraph
* Plotting of system properties, such as GDD or stability
* $PSchlup 2006 $     $Revision 0 $
*********************************************************/
#ifndef CSYSWINGRAPH_H                      // prevent multiple includes
#define CSYSWINGRAPH_H
//===Classes Defined======================================
class CSysWinGraph;                         // system graph

//===Includes=============================================
#include <windows.h>                        // standard Windows include
#include <stdio.h>                          // standard header

#include "CSysWin.h"                        // base renderer class header
#include "CSystem.h"                        // system class
#include "CAxes.h"                          // axes plotting class
#include "CMouse.h"                         // renderer mouse
#include "CQuickEdit.h"                     // quick edit control
#include "CQuickMenu.h"                     // quick menu control

//===Constants============================================
//---Properties-----------------------
#define CGRPI_PROP_XMIN          0          // x-axis minimum
#define CGRPI_PROP_XMAX          1          // x-axis maximum
#define CGRPI_PROP_YMIN          2          // y-axis minimum
#define CGRPI_PROP_YMAX          3          // y-axis maximum
#define CGRPI_PROP_RUNVAR        4          // variable plotted against
#define CGRPI_PROP_PLTFCN        5          // function to be plotted
#define CGRPI_NUM_PROP           6          // poperties

//---Rectangles-----------------------
#define CGRPI_RECT_RUNVAR        0          // (index) rectangle for variable
#define CGRPI_RECT_PLTFCN        1          // (index) rectangle for y label
#define CGRPI_NUM_RECT           2          // (count) number of rectangles

//---Mouse----------------------------
#define CGRP_ACT_AXES            0          // active for axes
#define CGRP_ACT_XMIN            1          // active for x-minimum
#define CGRP_ACT_XMAX            2          // active x-maximum
#define CGRP_ACT_YMIN            3          // active for y-minimum
#define CGRP_ACT_YMAX            4          // active y-maximum
#define CGRP_ACT_RUNVAR          5          // active run variable (X-label)
#define CGRP_ACT_PLTFCN          6          // active plotting function
#define CGRP_ACT_VARVAL          7          // active for running variable value

/*********************************************************
* CSysWinGraph Class
* Why can these derived classes not have destructors?
*********************************************************/
class CSysWinGraph : public CSysWin {
private:
   CAxes       Axes;                        // plotting axes
   double      ddProp[CGRPI_NUM_PROP];      // properties
   RECT        rcRect[CGRPI_NUM_RECT];      // rectangles
   CMouse      Mouse;                       // renderer mouse
   HMENU       hmAxesPopup;                 // axes popup menu
   CQuickEdit *pQEdit;                      // quick-edit control
   CQuickMenu *pQMenu;                      // quick-menu control

   //---Data--------------------------
   double     *pdXData;                     // line X data (variable values)
   double     *pdSData;                     // line sagittal data (function values)
   double     *pdTData;                     // line tangential data (function values)
   int         iDataPts;                    // number of data points

public:
   CSysWinGraph(CSystem *pSys);             // constructor
   CSysWinGraph::~CSysWinGraph();           // destructor
   void    OnResize(void);                  // resizing of window
   void    OnPaint(void);                   // paint the renderer
   void    CopyDataToClipboard(void);       // copy the graph data to clipboard

   //---Access----------------------------------
   double  XMin(void)                  { return(ddProp[CGRPI_PROP_XMIN]); };
   double  XMax(void)                  { return(ddProp[CGRPI_PROP_XMAX]); };
   double  YMin(void)                  { return(ddProp[CGRPI_PROP_YMIN]); };
   double  YMax(void)                  { return(ddProp[CGRPI_PROP_YMAX]); };
   void    UserSetXLim(double dMn, double dMx); // set axis x-limits
   void    UserSetYLim(double dMn, double dMx); // set axis x-limits
   int     RunVar(void);                    // returns running variable
   void    UserSetRunVar(int k);            // set running variable, updates
   int     PltFcn(void);                    // retruns plotting function
   void    UserSetPltFcn(int k);            // sets plotting function, updates
   void    SetAutoLimits(int iAx);          // set the limits to include all data


   //---Overloaded Base Functions---------------
   int     Type(void)                  { return(CSYSWINI_TYPE_GRAPH); };
   void    Refresh(void);                   // refresh the render window
   void    UpdateTitle(int iID);            // update title bar
   void    SaveSysWin(HANDLE hFile);        // save current renderer
   BOOL    LoadSysWin(const char *pszDataFile, char *pszMin, char *pszMax); // load renderer from file
   void    GraphPoint(int iRVar, int iPt);  // update a graphing point
   void    MenuCommand(int iCmd);           // process specific commands
   void    PrepareProperties(BOOL tfAct);   // show / hide relevant properties
   void    Print(HDC hdcPrint);             // print to printer device context
   static LRESULT CALLBACK WndProcSysWinGraph(HWND, UINT, WPARAM, LPARAM); // window procedure

   //---Properties------------------------------
   void    UpdateProperties(void);          // update properties
   void    DebugPrint(char *psz, int *pInt); // print some information
   BOOL    SysWinPropItemCallback(void *vData, UINT uID); // modified from property manager
   static BOOL _SysWinPropItemCallback(void *vData, UINT uID, void *pVoid); // callback from property manager

   //---Mouse-----------------------------------
   static void _MouseCallback(int iMsg, int x, int y, int wKeys, void *pData, long int lData);
   void MouseCallback(int iMsg, int x, int y, int wKeys, long int lData);
   static BOOL _QEditCallback(void *pVal, int iNext, void *pVoid, long int lLong);
   BOOL QEditCallback(void *pVal, int iNext, long int lLong);
   static BOOL _QMenuCallback(int iSel, int iNext, void *pVoid, long int lLong);
   BOOL QMenuCallback(int iSel, int iNext, long int lLong);
   void    RefreshDraggerActive(void);      // refresh the dragger active rectangle

   //---Name Tables-----------------------------
   static const UINT  CuProperties[];       // properties to show / hide
   static const char* CszSysWinGraphProp[]; // names for stored properties
};
#endif//CSYSWINGRAPH_H
