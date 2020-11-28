/*********************************************************
* CPropMgr.h
* Header file for PropertyManager for LaserCanvas
* $PSchlup 2000-2006 $      $Revision 5 $
*********************************************************/
#ifndef CPROPMGR_H                          // prevent multiple includes
#define CPROPMGR_H
//===Classes Defined======================================
class CPropMgr;                             // property manager class

//===Includes=============================================
#include <windows.h>                        // standard Windows include
#include <stdio.h>                          // standard header

#include "CApplication.h"                   // application class
#include "CSysWin.h"                        // renderers base class
#include "CMouse.h"                         // mouse and actives
#include "CPropItem.h"                      // item class
#include "CQuickEdit.h"                     // manager quick-edit
#include "CQuickMenu.h"                     // manager quick-menu

//===Constants============================================
#define CPM_SZCLASSNAME "CPropMgr_Class"
#define CPM_SZWINDOWTITLE "Properties"

//---Flags--------------------------------------
#define CPMF_DOCKLEFT   0x00000000          // window docked at left
#define CPMF_DOCKRIGHT  0x00010000          // window docked at right
#define CPMF_DOCKPOS    0x00010000          // (mask) docking position
#define CPMF_DOCKED     0x00040000          // window is docked
#define CPMF_EQTNSRC    0x00080000          // show equations as source

//---Active constants---------------------------
#define CPMC_ACT_COLUMN         -1          // column width active
#define CPMC_ACT_WIDTH          -2          // window width active
#define CPMC_FRAMEDRAGWID        6          // width of window dragger
#define CPMC_COLDRAGWID          3          // half-width of column dragger
#define CPMC_SCROLLLINE          8          // scroll line step size

//---GDI----------------------------------------
#define CPMC_FONT                1          // match this constant to the default CAPPI_FONT_ for Font() below

/*********************************************************
* CPropMgr Class
*********************************************************/
class CPropMgr {
private:                                    // private members
   CApplication *pAppParent;                // parent application
   CMouse       *pMouse;                    // mouse actives
   CPropItem    *pItemTop;                  // top of item chain
   CQuickEdit   *pQEdit;                    // one quick-edit per prop manager
   CQuickMenu   *pQMenu;                    // quick-menu item
   HWND          hwPropMgr;                 // property manager window
   UINT          uFlags;                    // manager flags
   int           iWinWidth;                 // width of window (docked)
   int           iColWidth;                 // column width
   int           iItemEnd;                  // end of items (vertical)
   HWND          hwScrollBar;               // scroll bar control
   int           iScrollWid;                // scroll bar width
   int           iScrollOffset;             // vertical scroll offset
   CSysWin      *pSysWinActive;             // active renderer

public:                                     // public functions
   CPropMgr(CApplication *pApp);            // constructor
   ~CPropMgr();                             // destructor

   HWND   CreateMgrWindow(void);            // create the window
   void   DestroyMgrWindow(void);           // destroy the window
   void   OnParentResize(LPRECT lprc);      // called when parent window is resized
   void   OnResize(void);                   // resize function - move scroll bar
   void   OnPaint(BOOL tfTextOnly);         // paint the manager window
   void   OnScrollBar(int iCode, int nPos); // called when scroll bar is changed

   //OBSOLETE void   SetSysWinActive(CSysWin *pSWin, BOOL tfRepaint);  // sets the active renderer window

   //---Item list-------------------------------
   CPropItem* NewResourceItem(UINT uResourceID, int iType, UINT uFlags, PROPITEMCALLBACK lpfn=NULL, void *pVoid=NULL); // create a new item from resource file
   CPropItem* FindItemByID(UINT uID);        // find item by its ID (or iResource in NewResourceItem)
   void       DeleteItem(CPropItem *pItemDel); // delete item, maintain chain
   void       PositionItems(void);          // calculate position rectangles

   //---Mouse functions-------------------------
   CMActive*  CreateActive(int,int,int,int,HCURSOR,HCURSOR,HCURSOR,HCURSOR,void(*)(int,int,int,int,void*,long int),void*, long int,HMENU);
   void       DeleteActive(CMActive *pDel);

   //---Access----------------------------------
   CApplication *GetAppParent(void)    { return(pAppParent); };
   HWND        Window(void)            { return(hwPropMgr); };
   int         WindowWidth(void)       { return(iWinWidth); };
   int         ColumnWidth(void)       { return(iColWidth); };
   CQuickEdit* QEdit(void)             { return(pQEdit); };
   CQuickMenu* QMenu(void)             { return(pQMenu); };
   //---Routed to CApplication---
   void        SetAppStatusBarPriorityText(const char *pszText); // routed to CApp
   HINSTANCE   AppInstance(void);           // routed from CApp
   COLORREF    Rgb(int k);
   HPEN        Pen(int k);
   HBRUSH      Brush(int k);
   HFONT       Font(int k=CPMC_FONT);

   //---Flags-----------------------------------
   void SetBit(UINT u)                 { uFlags |= u; };
   void ClearBit(UINT u)               { uFlags &=~u; };
   void ToggleBit(UINT u)              { uFlags ^= u; };
   UINT CheckBit(UINT u)               { return(uFlags & u); };

   //---Callbacks-------------------------------
   static LRESULT CALLBACK WndProcPropMgr(HWND, UINT, WPARAM, LPARAM);
   static void MouseCallback(int iMsg, int x, int y, int wKeys, void *pData, long int lData);
};

#endif//CPROPMGR_H
