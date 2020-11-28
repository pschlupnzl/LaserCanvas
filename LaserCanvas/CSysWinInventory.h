/*********************************************************
* CSysWinInventory
* $PSchlup 2006 $     $Revision 0 $
*********************************************************/
#ifndef CSYSWININVENTORY_H                  // prevent multiple includes
#define CSYSWININVENTORY_H

//===Classes Defined======================================
class CSysWinInventory;                     // inventory system renderer

//===Included Files=======================================
#include <windows.h>                        // standard Windows include
#include <stdio.h>                          // standard header

#include "CSysWin.h"                        // base class header
#include "CSystem.h"                        // system class
#include "CMouse.h"                         // mouse layer class

//===Constants============================================
//---System properties----------------
// These are really only loop counters
#define CINI_SYSWLEN             0          // (counter) system wavelength
#define CINI_SYSMSQUARED         1          // (counter) System M^2
#define CINI_SYSLENGTH           2          // (counter) physical / optical length
#define CINI_SYSMODESPACING      3          // (counter) mode spacing (MHz)
#define CINI_SYSSTAB             4          // (counter) system stability
#define CINI_NUMSYSROW           5          // (count) number of system rows
//---Vertex columns-------------------
#define CINI_VXCOLTAG            0          // (index) type / tag
#define CINI_VXCOLCOORD          1          // (index) coord x / coord y
#define CINI_VXCOLROCFL          2          // (index) ROC/FL (sag/tan)
#define CINI_VXCOLANGLE          3          // (index) angle
#define CINI_VXCOLDISTN          4          // (index) distance / ref index
#define CINI_VXCOLMODE           5          // (index) mode sagittal / tangential
#define CINI_VXCOLCURV           6          // (index) curvature sagittal / tangential
#define CINI_NUMVXCOL            7          // (count) number of vx columns

//---Properties-----------------------
#define CINI_PROP_OFFSETX        0          // (index) scroll x offset
#define CINI_PROP_OFFSETY        1          // (index) scroll y offset
#define CINI_PROP_COLWIDTH       2          // (index) OFFSET to column width values
#define CINI_NUM_PROP  CINI_PROP_COLWIDTH+CINI_NUMVXCOL   // (count) number of properties


//---Mouse----------------------------

/*********************************************************
* CSysWinInventory Class
*********************************************************/
class CSysWinInventory : public CSysWin {
private:
   double  ddProp[CINI_NUM_PROP];           // min, max of selected zoom area
   CMouse  Mouse;                           // mouse and its actives
   int     iRowHeight;                      // height of single rows
   int     iOffsetX, iOffsetY;              // painting scroll offsets

public:
   CSysWinInventory(CSystem *pSys);         // constructor
   ~CSysWinInventory();                     // overloaded destructor

   void    OnPaint(HDC hdcPrint=NULL);      // paint / print the window
   void    OnResize(void);                  // resize function

   //---Access----------------------------------
   int     ColWidth(int iPrp)          { return(((iPrp>=0) && (iPrp<CINI_NUMVXCOL)) ? (int) ddProp[CINI_PROP_COLWIDTH+iPrp] : 0); };
   int     XOffset(void)               { return((int) ddProp[CINI_PROP_OFFSETX]); };
   int     YOffset(void)               { return((int) ddProp[CINI_PROP_OFFSETY]); };
   void    SetColWidth(int iPrp, int w){ if((iPrp>=0) && (iPrp<CINI_NUMVXCOL)) ddProp[CINI_PROP_COLWIDTH+iPrp] = (double) (w>=0) ? w : 0; };
   void    SetXOffset(int x)           { ddProp[CINI_PROP_OFFSETX] = (double) x; };
   void    SetYOffset(int y)           { ddProp[CINI_PROP_OFFSETY] = (double) y; };

   //---Overloaded functions--------------------
   int     Type(void)                  { return(CSYSWINI_TYPE_INVENTORY); };
   void    Refresh(void);                   // refresh the renderer
   void    UpdateTitle(int iID);            // update title bar
   void    SaveSysWin(HANDLE hFile);        // save current renderer
   BOOL    LoadSysWin(const char *pszDataFile, char *pszMin, char *pszMax); // load renderer from file
   void    GraphPoint(int iRVar, int iPt);  // update a graphing point
   void    MenuCommand(int iCmd);           // process specific commands
   void    DebugPrint(char *psz, int *pInt); // debug text
   void    Print(HDC hdcPrint);             // print to printer device context
   static LRESULT CALLBACK WndProcSysWinInventory(HWND, UINT, WPARAM, LPARAM); // window procedure

   //---Properties------------------------------
   void    PrepareProperties(BOOL tfAct);   // show / hide properties
   void    UpdateProperties(void);          // update properties
   BOOL    SysWinPropItemCallback(void *vData, UINT uID); // modified from property manager
   static BOOL _SysWinPropItemCallback(void *vData, UINT uID, void *pVoid); // callback from property manager

   //---Mouse-----------------------------------
   static void _MouseCallback(int iMsg, int x, int y, int wKeys, void *pData, long int lData);
   void MouseCallback(int iMsg, int x, int y, int wKeys, long int lData);

   //---Tables----------------------------------
   static const UINT CuProperties[];        // properties revealed by this renderer
   static const char* CszInvVxColumnSource;          // source for column headings
   static const char* CszInvVxColumn[2*CINI_NUMVXCOL]; // additional column headings
   static const char* CszInvSysRowSource;           // source for row headings
   static const char* CszInvSysRow[CINI_NUMSYSROW]; // additional row headings
   static const char* CszSysWinInventoryProp[CINI_PROP_COLWIDTH+1];
};

#endif//CSYSWININVENTORY_H
