/*********************************************************
* CSysWin2d
* $PSchlup 2006 $     $Revision 5 $
*********************************************************/
#ifndef CSYSWIN2D_H                         // prevent multiple includes
#define CSYSWIN2D_H

//===Classes Defined======================================
class CSysWin2d;                            // 2d interactive renderer

//===Includes=============================================
#include <windows.h>                        // standard Windows include
#include <stdio.h>                          // standard header
#include <math.h>                           // trig functions
#include "CSysWin.h"                        // base class header
#include "Renderer2d.h"                     // icons
#include "LCUtil.h"                         // utility functions
#include "CMouse.h"                         // mouse class

//===Constants============================================
typedef struct tagDBLRECT {
   double Left, Top, Right, Bottom;
} DBLRECT;

#define C2DC_ROTATE_CROSS       16          // half-size of rotation crosshair
#define C2DC_ROTATE_MITRE (15.00*M_PI/180.00) // rotation mitre with CTRL key

//---Properties-----------------------
#define C2DI_PROP_XMIDDLE        0          // (index) canvas center
#define C2DI_PROP_YMIDDLE        1          // (index) canvas center
#define C2DI_PROP_ZOOM           2          // (index) zoom scale
#define C2DI_PROP_OPTICSCALE     3          // (index) optic scale
#define C2DI_PROP_MODESCALE      4          // (index) mode scale
#define C2DI_PROP_GRIDSIZE       5          // (index) grid size
#define C2DI_PROP_FLAGS          6          // (index) (integer) flags
#define C2DI_NUM_PROP            7          // (count) number of properties

//---Flags----------------------------
#define C2DF_SNAPGRID       0x0001          // (bit) snap to canvas grid
#define C2DF_SHOWDIST       0x0002          // (bit) show distance markers
#define C2DF_SHOWANNOT      0x0004          // (bit) show optic annotations
#define C2DF_SHOWWAIST      0x0008          // (bit) show waist positions

//---Tools----------------------------
#define C2DC_TOOL_ARROW          0          // select / edit tool
#define C2DC_TOOL_MEASURE        1          // measure tool
#define C2DC_TOOL_ZOOM           2          // zoom tool
#define C2DC_TOOL_PAN            3          // pan tool
#define C2DC_TOOL_ROTATE         4          // rotate tool
#define C2DC_TOOL_MAX            4          // max value for tool

enum C2DT_FRAME {
   C2DC_FRAME_NONE             = 0,         // (enum) no frame
   C2DC_FRAME_SELECT           = 1,         // (enum) selection frame (dashed)
   C2DC_FRAME_ZOOM             = 2,         // (enum) zoom selection frame (hatched)
   C2DC_FRAME_MEASURE          = 3,         // (enum) simple line frame (non-native on Windows)
   C2DC_FRAME_PAN              = 4,         // (enum) panning center
   C2DC_FRAME_ROTATE           = 5          // (enum) rotation center
};

//===Macros===============================================

/*********************************************************
* CSysWin2d Class
*********************************************************/
class CSysWin2d : public CSysWin {
private:
   int    iTool;                            // current tool
   DBLRECT   drcInvFrame;                   // inversion frame
   C2DT_FRAME iFrameTyp;                    // type of frame

   //---Canvas----------------------------------
   double ddProp[C2DI_NUM_PROP];            // properties

   //---GDI-------------------------------------
   int     ixCenter, iyCenter;              // window center pixel
   CMouse  Mouse;                           // mouse object
public:
   CSysWin2d(CSystem *pSys);                // constructor
   ~CSysWin2d();                            // destructor

   void    OnResize(void);                  // resize - move center, actives, etc
   void    OnPaint(HDC hdcIn=NULL);         // painting
   void    CreateAllVxActive(void);         // create actives for all Vxs
   void    UpdateAllVxActiveRect(void);     // reposition all the Vx actives
   void    ZoomToRect(double dXMin, double dYMin, double dXMax, double dYMax);
   void    UserZoom(double dZm);            // zoom absolute or relative, refresh
   void    InvertFrame(void);               // invert the standard frame

   //---Overloaded Base Functions---------------
   int     Type(void)                  { return(CSYSWINI_TYPE_2D); };
   void    Refresh(void);                   // refresh the render window
   void    UpdateTitle(int iID);            // update title bar
   void    SaveSysWin(HANDLE hFile);        // save current renderer
   BOOL    LoadSysWin(const char *pszDataFile, char *pszMin, char *pszMax); // load renderer from file
   void    GraphPoint(int iRVar, int iPt);  // update a graphing point
   void    MenuCommand(int iCmd);           // process specific commands
   void    DebugPrint(char *psz, int *pInt); // print some information
   void    Print(HDC hdcPrint);             // print to printer device context

   //---Access----------------------------------
   double  XMiddle(void)               { return(ddProp[C2DI_PROP_XMIDDLE   ]); };
   double  YMiddle(void)               { return(ddProp[C2DI_PROP_YMIDDLE   ]); };
   double  Zoom(void)                  { return(ddProp[C2DI_PROP_ZOOM      ]); };
   double  OpticScale(void)            { return(ddProp[C2DI_PROP_OPTICSCALE]); };
   double  ModeScale(void)             { return(ddProp[C2DI_PROP_MODESCALE ]); };
   double  GridSize(void)              { return(ddProp[C2DI_PROP_GRIDSIZE  ]); };
   int     Tool(void)                  { return(iTool); } // accessed by App::EnableMenus(-2)
   void    UserSetTool(int k);              // called from tool menu

   //---Flags-----------------------------------
   // We want to save the flags to file, so it must be a DOUBLE property
   void    SetBit(UINT u)              { ddProp[C2DI_PROP_FLAGS] = (double)(((UINT)ddProp[C2DI_PROP_FLAGS]) | u); };
   void    ClearBit(UINT u)            { ddProp[C2DI_PROP_FLAGS] = (double)(((UINT)ddProp[C2DI_PROP_FLAGS]) &~u); };
   void    ToggleBit(UINT u)           { ddProp[C2DI_PROP_FLAGS] = (double)(((UINT)ddProp[C2DI_PROP_FLAGS]) ^ u); };
   UINT    CheckBit(UINT u)            { return( ((UINT)ddProp[C2DI_PROP_FLAGS]) & u ); };

   //---Inlines---------------------------------
   inline int    Cnv2WndX(double X) { return(Zoom() * (X-XMiddle()) + ixCenter); };
   inline int    Cnv2WndY(double Y) { return(Zoom() * (Y-YMiddle()) + iyCenter); };
   inline double Wnd2CnvX(int x)    { return(((double)x-ixCenter) / (Zoom() + ((Zoom()==0.00)?1.00:0.00)) + XMiddle()); };
   inline double Wnd2CnvY(int y)    { return(((double)y-iyCenter) / (Zoom() + ((Zoom()==0.00)?1.00:0.00)) + YMiddle()); };

   //---Properties------------------------------
   void    PrepareProperties(BOOL tfAct);   // show / hide properties
   void    UpdateProperties(void);          // update properties
   BOOL    SysWinPropItemCallback(void *vData, UINT uID); // modified from property manager
   static BOOL _SysWinPropItemCallback(void *vData, UINT uID, void *pVoid); // callback from property manager

   //---Callbacks-------------------------------
   static LRESULT CALLBACK _WndProcSysWin2d(HWND, UINT, WPARAM, LPARAM); // window procedure
   LRESULT CALLBACK WndProcSysWin2d(HWND, UINT, WPARAM, LPARAM); // dereferenced window procedure
   static void _MouseCallbackVx(int,int,int,int,void*,long int); // callback from Vx actives
   void         MouseCallbackVx(int,int,int,int,long int); // dereferenced version
   static void _MouseCallbackCanvas(int,int,int,int,void*,long int); // canvas callback
   void         MouseCallbackCanvas(int,int,int,int,long int); // dereferenced canvas callback

   //---Tables----------------------------------
   static const UINT CuProperties[];        // properties revealed by this renderer
   static const char* CszSysWin2dProp[C2DI_NUM_PROP]; // names for stored properties
};
#endif//CSYSWIN2D_H
