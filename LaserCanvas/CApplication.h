/*********************************************************
* CApplication.h
* $PSchlup 2006 $     $Revision 5 $
*********************************************************/
#ifndef CAPPLICATION_H                      // prevent multiple includes
#define CAPPLICATION_H
//===Classes defined======================================
class CApplication;                         // application class

//===Includes=============================================
#include <windows.h>                        // standard Windows include
#include <stdio.h>                          // standard header

//===Constants============================================
//---GDI----------------------------------------
#define CAPPI_RGBBLACK           0          // (index) standard color
#define CAPPI_RGBWINDOW          1          // (index) window color
#define CAPPI_RGB3DLITE          2          // (index) 3d highlight color
#define CAPPI_RGB3DDARK          3          // (index) 3d shadow color
#define CAPPI_RGB3DFACE          4          // (index) 3d face color
#define CAPPI_RGB3DTEXT          5          // (index) 3d button face TEXT color
#define CAPPI_RGB3DGRAY          6          // (index) 3d button face DISABLED TEXT color
#define CAPPI_RGBSAG             7          // (index) sagittal color scheme
#define CAPPI_RGBTAN             8          // (index) tangential color scheme
#define CAPPI_RGBSAGTAN          9          // (index) symmetric sagittal/tangential color scheme
#define CAPPI_RGBOPTIC          10          // (index) optic
#define CAPPI_RGBSEGMENT        11          // (index) segment
#define CAPPI_RGBEQTN           12          // (index) equation color scheme
#define CAPPI_RGBCMD            13          // (index) color for commands / links
#define CAPPI_RGBAXES           14          // (index) color for axes lines
#define CAPPI_RGB3DMARKER       15          // (index) color marker in 3d btn face patches
#define CAPPI_RGBREFINDEX       16          // (index) color for refractive blocks
#define CAPPI_RGBGRID           17          // (index) color for grid
#define CAPPI_RGBDRAFT          18          // (index) draft optic color
#define CAPPC_NUMRGB            19          // (count) number of colorrefs

///TODO: Order these so they make sense!
#define CAPPI_PEN                0          // (index) default pen
#define CAPPI_PEN3DLITE          1          // (index) 3d highlight color pen
#define CAPPI_PEN3DDARK          2          // (index) 3d shadow color pen
#define CAPPI_PEN3DFACE          3          // (index) 3d face color pen
#define CAPPI_PEN3DTEXT          4          // (index) 3d button face TEXT color pen
#define CAPPI_PENCMD             5          // (index) command underline pen in prop manager
#define CAPPI_PENSAG             6          // (index) sagittal color pen
#define CAPPI_PENTAN             7          // (index) tangential color pen
#define CAPPI_PENSAGTAN          8          // (index) symmetric sagittal/tangential color pen
#define CAPPI_PENOPTIC           9          // (index) optic
#define CAPPI_PENSEGMENT        10          // (index) segment
#define CAPPI_PENOPTICSEL       11          // (index) selected optic
#define CAPPI_PENSEGMENTSEL     12          // (index) selected segment
#define CAPPI_PENSEGMENTOUT     13          // (index) selected segment whiteout
#define CAPPI_PENAXES           14          // (index) axes pens
#define CAPPI_PENGRID           15          // (index) grid pen
#define CAPPI_PENDEBUG          16          // (index) pen for debugging
#define CAPPI_PENDRAFTOPTIC     17          // (index) draft mode optics pen
#define CAPPI_PENDRAFTOPTICSEL  18          // (index) draft mode selected optic
#define CAPPC_NUMPEN            19          // (count) number of pens

#define CAPPI_BRUSH              0          // (index) default brush
#define CAPPI_BRUSHWINDOW        0          // (index) window color brush
#define CAPPI_BRUSH3DFACE        1          // (index) 3d face brush
#define CAPPI_BRUSH3DMARKER      2          // (index) selection in full view
#define CAPPI_BRUSHREFINDEX      3          // (index) brush for blocks with refractive indices
#define CAPPI_BRUSHDASH          4          // (index) patterned brush for dashed lines
#define CAPPI_BRUSHHATCH         5          // (index) patterned brush for hatched lines
#define CAPPC_NUMBRUSH           6          // (count) number of brushes

#define CAPPI_FONT               0          // (index) default font for renderers
#define CAPPI_FONTPROPMGR        1          // (index) property manager font
#define CAPPI_FONTPRINT          2          // (index) "screen" font for printing
#define CAPPC_NUMFONT            3          // (count) number of fonts

#define CAPPI_CUR_ARROW          0          // (index) Alternative to IDC_ARROW
#define CAPPI_CUR_DELETE         1          // (index) Cross (cf. Powerpoint Edit Points)
#define CAPPI_CUR_MEAS_LOCK      2          // (index) Arrow with small ruler and circle
#define CAPPI_CUR_MEAS           3          // (index) Arrow with small ruler
#define CAPPI_CUR_MOVE_ARROW     4          // (index) Arrow with NESW move arrows
#define CAPPI_CUR_MOVE           5          // (index) NESW move arrows
#define CAPPI_CUR_NODRAG         6          // (index) Black circle with slash through it
#define CAPPI_CUR_PAN_ARROW      7          // (index) Open hand
#define CAPPI_CUR_PAN            8          // (index) Closed hand
#define CAPPI_CUR_ROTATE_ARROW   9          // (index) Arrow with rotation arrow
#define CAPPI_CUR_ROTATE        10          // (index) Rotation arrow
#define CAPPI_CUR_STRETCH_ARROW 11          // (index) Arrow with constrained move arrows
#define CAPPI_CUR_STRETCH       12          // (index) Constrained move arrows
#define CAPPI_CUR_ZOOMIN        13          // (index) Magnifying glass with '+'
#define CAPPI_CUR_ZOOMOUT       14          // (index) Magnifying glass with '-'
#define CAPPC_NUM_CUR           15          // (count) number of cursors

#define CAPPC_NUM_BMP            1          // (count) number of bitmaps (NOT USED? - Must be at least 1)

//---Equation variables-------------------------
#define CAPP_NUMVAR              3          // number of variables


//===Other Includes=======================================
#include "resource.h"                       // resource file constants
#include "CPropMgr.h"                       // property manager class
#include "CPropItem.h"                      // property manager item
#include "CStatusBar.h"                     // status bar class
#include "CButtonBar.h"                     // button bar class
#include "CSystem.h"                        // system class
#include "CSysWin.h"                        // renderer base class


/*********************************************************
* CApplication Class
*********************************************************/
class CApplication {
private:
   HINSTANCE   hInstance;                   // application instance
   HWND        hwApp;                       // application window
   CPropMgr   *pPropMgr;                    // property manager
   CStatusBar *pStatusBar;                  // status bar
   CButtonBar *pButtonBar;                  // button bar
   HWND        hwMDIClient;                 // mdi client window
   UINT        uFlags;                      // application status flags
   CSystem    *pSysTop;                     // top of system chain

   //---Global use items------------------------
   COLORREF    rgbColor[CAPPC_NUMRGB];      // global color scheme (except 3d and standard)
   HPEN        hpAppPen[CAPPC_NUMPEN];      // global pens
   HBRUSH      hbrAppBrush[CAPPC_NUMBRUSH]; // global brushes
   HFONT       hfAppFont[CAPPC_NUMFONT];    // global fonts
   HCURSOR     hcAppCursor[CAPPC_NUM_CUR];  // global cursors
   HBITMAP     hbmAppBitmap[CAPPC_NUM_BMP]; // global bitmaps

   //---Parameters------------------------------
   int         iCurrentVariable;            // currently active variable in Property Manager
   double      dVar[CAPP_NUMVAR];           // current variable values
   double      dVarMin[CAPP_NUMVAR];        // minimum variable range
   double      dVarMax[CAPP_NUMVAR];        // maximum variable range
   int         iVarHooks[CAPP_NUMVAR];      // number of renderers hooked on each variable
   int         iVarPoints[CAPP_NUMVAR];     // number of points to scan each variable

public:
   CApplication(HINSTANCE hInst, int iShow); // constructor
   ~CApplication();                         // destructor
   int    MessageLoop(void);                // Windows message loop
   void   ProcessCommand(int iCmd);         // process WM_COMMAND commands
   void   OnResize(void);                   // window is resized
   void   Help(const char *szHelp);         // launch the HTML help app
   void   About(void);                      // display the About dialog

   //---Updating Chain--------------------------
   void   ScanAll(void);                    // scan over all variables, render to canvas
   void   ApplyAllEquations(double *pcdVar); // update all systems' equations
   void   SolveAllSystemABCD(void);         // solve the system ABCDs
   void   GraphAllRendererPoint(int iRVar, int iPt); // graph this point on all renderers
   void   PlaceAllCanvasVertices(void);     // position onto canvas
   void   RefreshAllRenderers(void);        // refresh renderers

   //---User Functions--------------------------
   CSystem* UserNewSystem(void);            // create system, maintain chain
   void   UserDeleteSystem(CSystem *pSys);  // delete system, maintain chain
   BOOL   UserClose(void);                  // user wants to exit application
   void   UserViewPropEqtn(BOOL tfEqtn);    // show or hide equations in properties
   void   UserViewProperties(BOOL tfView);  // show or hide properties
   void   UserDockProperties(BOOL tfDock);  // dock prop manager, show window
   BOOL   UserCloseSystem(CSystem *pSysClose); // user wants to close a system
   void   MenuPrint(BOOL tfAll);            // print dialog and print current renderer
   void   PrintHeaderFooter(HDC hdcPrint, const char *pszFileName, int *piPage);  // print header and footer

   //---Files---
   void   UserSaveSystem(BOOL tfSaveAs);    // save current system
   void   UserLoadSystem(void);             // load a system
   void   LoadErrorMsg(int iErr, const char *pszDataFile, const char *pszErr); // display an error message

   //---Flags-----------------------------------
   void SetBit(UINT u)                 { uFlags |= u; };
   void ClearBit(UINT u)               { uFlags &=~u; };
   void ToggleBit(UINT u)              { uFlags ^= u; };
   UINT CheckBit(UINT u)               { return(uFlags & u); };

   //---MDI Child-------------------------------
   HWND   CreateSysWinWindow(const char *pszClassName, LPVOID lpVoid, UINT uStyle=0x0000); // create a new MDI window
   void   DestroySysWinWindow(HWND hWnd);   // destroy a MDI child window
   void   ActivateSysWinWindow(HWND hWnd);  // activate the given MDI child window
   CSysWin*    GetCurrentRenderer(void);    // get current renderer window
   CSystem*    GetCurrentSystem(void);      // establish currently active system
   void        EnableMenus(int iRenderer);  // enable menu items depending on renderer
   void        CheckMenu(int iCmd, BOOL tfChecked); // check / uncheck a menu item
   void        MenuItemSelect(UINT uCmd);   // call for WM_MENUSELECT

   //---Property Manager------------------------
   void        PrepareProperties(const UINT *puIDList, BOOL tfShow, PROPITEMCALLBACK lpfnCallback, void *pVoid); // show or hide list of items
   void        PrepareVariableProperties(BOOL tfShow); // show or hide the equation properties
   void        UpdateProperties(void);           // update application properties in the manager
   static BOOL _PropItemCallback(void *vData, UINT uID, void *pVoid);
   BOOL        PropItemCallback(void *vData, UINT uID); // callback from property manager items

   //---Status bar------------------------------
   void        SetStatusBarInfo(double *pdX, double *pdY); // set the info window on the status bar

   //---Access----------------------------------
   HINSTANCE   GetInstance(void)      { return(hInstance); };
   HWND        GetWindow(void)        { return(hwApp); };
   HWND        GetMDIClient(void)     { return(hwMDIClient); };
   CStatusBar* GetStatusBar(void)     { return(pStatusBar); };
   CPropMgr*   PropManager(void)      { return(pPropMgr); };
   CSystem*    SysTop(void)           { return(pSysTop); };

   //---GDI-------------------------------------
   COLORREF    Rgb(int k)             { return(((k>=0)&&(k<CAPPC_NUMRGB  )) ? rgbColor[k]     : RGB(0x00, 0x00, 0x00)); };
   HPEN        Pen(int k=CAPPI_PEN)   { return(((k>=0)&&(k<CAPPC_NUMPEN  )) ? hpAppPen[k]     : (HPEN)GetStockObject(BLACK_PEN)); };
   HBRUSH      Brush(int k=CAPPI_BRUSH){return(((k>=0)&&(k<CAPPC_NUMBRUSH)) ? hbrAppBrush[k]  : (HBRUSH)GetStockObject(WHITE_BRUSH)); };
   HFONT       Font(int k=CAPPI_FONT) { return(((k>=0)&&(k<CAPPC_NUMFONT )) ? hfAppFont[k]    : NULL); };
   HCURSOR     Cursor(int k)          { return(((k>=0)&&(k<CAPPC_NUM_CUR )) ? hcAppCursor[k]  : LoadCursor(NULL, IDC_ARROW)); };
   HBITMAP     Bitmap(int k)          { return(((k>00)&&(k<CAPPC_NUM_BMP )) ? hbmAppBitmap[k] : NULL); };

   //---Variables-------------------------------
   void        SetVar(int iIndx, double dVal); // set a variable
   void        SetVarRange(int iIndx, double dMin, double dMax); // set range
   const char* VarsString(void);            // return string of variable strings
   const char* VarString(int k);            // return given variable string
   int         NumVars(void)          { return(CAPP_NUMVAR); };
   double*     Vars(void)             { return(dVar); }; // array for equation evaluation
   double      Var(int k)             { return(((k>=0)&&(k<CAPP_NUMVAR)) ? dVar[k] : 0.00); };
   double      VarMin(int k)          { return(((k>=0)&&(k<CAPP_NUMVAR)) ? dVarMin[k] : 0.00); };
   double      VarMax(int k)          { return(((k>=0)&&(k<CAPP_NUMVAR)) ? dVarMax[k] : 0.00); };
   void        AddVarHook(int k)      { if((k>=0)&&(k<CAPP_NUMVAR)) iVarHooks[k] += 1; };
   void        RemVarHook(int k)      { if((k>=0)&&(k<CAPP_NUMVAR)) iVarHooks[k] -= 1; }
   int         VarHooks(int k)        { return(((k>=0)&&(k<CAPP_NUMVAR)) ? iVarHooks[k] : -32767); };
   int         VarPoints(int k)       { return(((k>=0)&&(k<CAPP_NUMVAR)) ? iVarPoints[k] : 0); };
   void        SetVarPoints(int k, int N) { if((k<0)||(k>=CAPP_NUMVAR)) return; iVarPoints[k] = (N>1) ? N : 1; };

   //---Callbacks-------------------------------
   static LRESULT CALLBACK WndProcMain(HWND, UINT, WPARAM, LPARAM);
   static LRESULT CALLBACK AboutProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

   //---Name Table------------------------------
   static const char *CpszVar;              // string of variables
};
#endif//CAPPLICATION_H
