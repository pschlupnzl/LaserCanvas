/*********************************************************
* CSysWin3d
* Ok, this is a bit over-the-top.  But I am fascinated by
* the 3d world, and ever since starting rev 5 I wanted to
* include a 3d renderer. So now that the rest of the pro-
* gram is  almost finished, I can try adding  this; if it
* doesn't work, no harm done.
* $PSchlup 2006 $     $Revision 0 $
*********************************************************/
#ifndef CSYSWIN3D_H                         // prevent multiple includes
#define CSYSWIN3D_H

//===Classes Defined======================================
class CSysWin3d;                            // 3d renderer

//===Includes=============================================
#include <windows.h>                        // standard Windows include
#include <stdio.h>                          // standard header
#include <math.h>                           // trig functions
#include "CSysWin.h"                        // base class header
#include "Renderer3d.h"                     // 3d objects and renderers

//===Constants============================================
//---Properties-----------------------
#define C3DI_PROP_ORGX           0          // (index) viewpoint center X
#define C3DI_PROP_ORGZ           1          // (index) viewpoint center Z
#define C3DI_PROP_CAMX           2          // (index) camera X
#define C3DI_PROP_CAMY           3          // (index) camera Y
#define C3DI_PROP_CAMZ           4          // (index) camera Z
#define C3DI_PROP_CAMA           5          // (index) camera view angle
#define C3DI_PROP_OPTICSCALE     6          // (index) optic scale
#define C3DI_PROP_MODESCALE      7          // (index) mode scale
#define C3DI_PROP_FLAGS          8          // (index) (integer) flags
#define C3DI_NUM_PROP            9          // (count) number of properties

//---Flags----------------------------
#define C3DF_OPENGL         0x0001          // (bit) use OpenGL

//---Tools----------------------------
// Although not all tools  are available, we keep the en-
// tire list  anyway, so that all  the menu items  can be
// activated in a loop.
#define C3DC_TOOL_ARROW          0          // select / edit tool
#define C3DC_TOOL_MEASURE        1          // measure tool
#define C3DC_TOOL_ZOOM           2          // zoom tool
#define C3DC_TOOL_PAN            3          // pan tool
#define C3DC_TOOL_ROTATE         4          // rotate tool
#define C3DC_TOOL_MAX            4          // max value for tool

//===Macros===============================================

/*********************************************************
* CSysWin3d Class
*********************************************************/
class CSysWin3d : public CSysWin {
private:
   //---Properties------------------------------
   double ddProp[C3DI_NUM_PROP];            // properties
   CMouse Mouse;                            // mouse (just one active)
   int    iTool;                            // current tool

public:
   CSysWin3d(CSystem *pSys);                // constructor
   ~CSysWin3d();                            // destructor

   void    OnResize(void);                  // resize - move center, actives, etc
   void    OnPaint(HDC hdcIn=NULL);         // painting
   int     Tool(void)                  { return(iTool); } // accessed by App::EnableMenus(-2)
   void    UserSetTool(int k);              // called from tool menu

   //---Overloaded Base Functions---------------
   int     Type(void)                  { return(CSYSWINI_TYPE_3D); };
   void    Refresh(void);                   // refresh the render window
   void    UpdateTitle(int iID);            // update title bar
   void    SaveSysWin(HANDLE hFile);        // save current renderer
   BOOL    LoadSysWin(const char *pszDataFile, char *pszMin, char *pszMax); // load renderer from file
   void    GraphPoint(int iRVar, int iPt);  // update a graphing point
   void    MenuCommand(int iCmd);           // process specific commands
   void    DebugPrint(char *psz, int *pInt); // print some information
   void    Print(HDC hdcPrint);             // print to printer device context

   //---Access----------------------------------
   double  OrgX(void)                  { return(ddProp[C3DI_PROP_ORGX      ]); };
   double  OrgZ(void)                  { return(ddProp[C3DI_PROP_ORGZ      ]); };
   double  CamX(void)                  { return(ddProp[C3DI_PROP_CAMX      ]); };
   double  CamY(void)                  { return(ddProp[C3DI_PROP_CAMY      ]); };
   double  CamZ(void)                  { return(ddProp[C3DI_PROP_CAMZ      ]); };
   double  CamA(void)                  { return(ddProp[C3DI_PROP_CAMA      ]); };
   double  ModeScale(void)             { return(ddProp[C3DI_PROP_MODESCALE ]); };
   double  OpticScale(void)            { return(ddProp[C3DI_PROP_OPTICSCALE]); };
   void SetOrg(double X, double Z);
   void SetCam(double X, double Y, double Z, double A);

   //---Flags-----------------------------------
   // We want to save the flags to file, so it must be a DOUBLE property
   void    SetBit(UINT u)              { ddProp[C3DI_PROP_FLAGS] = (double)(((UINT)ddProp[C3DI_PROP_FLAGS]) | u); };
   void    ClearBit(UINT u)            { ddProp[C3DI_PROP_FLAGS] = (double)(((UINT)ddProp[C3DI_PROP_FLAGS]) &~u); };
   void    ToggleBit(UINT u)           { ddProp[C3DI_PROP_FLAGS] = (double)(((UINT)ddProp[C3DI_PROP_FLAGS]) ^ u); };
   UINT    CheckBit(UINT u)            { return( ((UINT)ddProp[C3DI_PROP_FLAGS]) & u ); };

   //---Properties------------------------------
   void    PrepareProperties(BOOL tfAct);   // show / hide properties
   void    UpdateProperties(void);          // update properties
   BOOL    SysWinPropItemCallback(void *vData, UINT uID); // modified from property manager
   static BOOL _SysWinPropItemCallback(void *vData, UINT uID, void *pVoid); // callback from property manager

   //---Callbacks-------------------------------
   static LRESULT CALLBACK _WndProcSysWin3d(HWND, UINT, WPARAM, LPARAM); // window procedure
   LRESULT CALLBACK WndProcSysWin3d(HWND, UINT, WPARAM, LPARAM); // dereferenced window procedure
   static void _MouseCallback(int,int,int,int,void*,long int); // mouse callback
   void         MouseCallback(int,int,int,int,long int); // dereferenced mouse callback

   //---Tables----------------------------------
   static const UINT CuProperties[];        // properties revealed by this renderer
   static const char* CszSysWin3dProp[C3DI_NUM_PROP]; // names for stored properties
};
#endif//CSYSWIN3D_H
