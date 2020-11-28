/*********************************************************
* CSysWin.h
* Since this is a base class,  the compiler needs to know
* the definition of the class before it can be derived by
* other classes. Hence the weird ordering here.
* $PSchlup 2006 $    $Revision 5 $
*********************************************************/
#ifndef CSYSWIN_H                           // prevent multiple includes
#define CSYSWIN_H

//===Classes Defined======================================
class CSysWin;                              // Windows rendering class

//---Classes defined elsewhere------------------
// Because of the  way this base class is  inherited, the
// other file headers have to be included after the CSys-
// Win class is defined. For that  reason, we have to de-
// clare classes referenced within CSysWin here.
// Overall it just convinces me that class inheritance is
// not really worth it.
class CApplication;                         // application class
class CSystem;                              // system class

//===Includes=============================================
#include <windows.h>                        // standard Windows include
#include <stdio.h>                          // standard header

#include "resource.h"                       // includes also window class names

//===Constants============================================
// These are indices into, e.g., save-type string array
#define CSYSWINI_TYPE_1D         0          // (index) renderer 1D
#define CSYSWINI_TYPE_2D         1          // (index) renderer 2D
#define CSYSWINI_TYPE_3D         2          // (index) renderer 3D
#define CSYSWINI_TYPE_GRAPH      3          // (index) renderer Graph (of system)
#define CSYSWINI_TYPE_VXGRAPH    4          // (index) renderer VxGraph
#define CSYSWINI_TYPE_INVENTORY  5          // (index) renderer VxGraph
#define CSYSWINI_TYPE_ABCDSOLVER 6          // (index) renderer ABCD solver
#define CSYSWINI_MAXTYPE         6          // (index) highest type index

/*********************************************************
* CSysWin Class
*********************************************************/
class CSysWin {
protected:
   HWND     hwRenderer;                     // renderer window
   CSystem *pSystem;                        // relevant system
   CSysWin *pSysWinNext;                    // next renderer in chain
public:
   CSysWin(CSystem *pSys);                  // constructor
   virtual ~CSysWin();                      // destructor

   //---Overloaded functions--------------------
   virtual int  Type(void)             = 0; // return type of renderer
   virtual void Refresh()              = 0; // refresh the renderer window
   virtual void UpdateTitle(int iID)   = 0; // update the title bar
   virtual void SaveSysWin(HANDLE hFile) = 0; // save current renderer
   virtual BOOL LoadSysWin(const char *pszDataFile, char *pszMin, char *pszMax) = 0; // load a renderer from file
   virtual void UpdateProperties(void)=0;     // update properties
   virtual void GraphPoint(int iRVar, int iPt) = 0; // update one data point
   virtual void MenuCommand(int iCmd) = 0;  // specific commands
   virtual void Print(HDC hdcPrint)   = 0;  // print to printer device context
   virtual void DebugPrint(char *psz, int *pInt) = 0; // print some debug information

   //---Access----------------------------------
   HWND     Window(void)               { return(hwRenderer); };
   CSysWin* Next(void)                 { return(pSysWinNext); };
   void     SetNext(CSysWin *pSWin)    { pSysWinNext = pSWin; };
   CSystem* System(void)               { return(pSystem); };
   CApplication* App(void);

   //---Data files------------------------------
   void     SaveSysWinHeader(HANDLE hFile, int iType); // write the generic header
   void     SaveSysWinFooter(HANDLE hFile); // write the generic footer
   void     LoadSysWinHeader(const char *pszDataFile, char *pszMin, char *pszMax); // load generic header (not System=!)

   //---Name Table------------------------------
   static const char *CszSysWinType[];      // names for renderers
};


/*********************************************************
* Remaining includes
*********************************************************/
#include "CSysWin1d.h"                      // 1d renderer
#include "CSysWin2d.h"                      // 2d renderer
#include "CSysWin3d.h"                      // 3d renderer
#include "CSysWinGraph.h"                   // system graph renderer
#include "CSysWinVxGraph.h"                 // VxGraph renderer
#include "CSysWinInventory.h"               // inventory renderer
#include "CSysWinABCDSolver.h"              // solver renderer

#include "CSystem.h"                        // system class
#include "CVertex.h"                        // vertex class (no direct access?)

#endif//CSYSWIN_H
