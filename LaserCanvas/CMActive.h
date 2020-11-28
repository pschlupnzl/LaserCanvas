/*********************************************************
*  CMActive.h
*  For mouse active class, LaserCanvas rev. 5
*  $PSchlup 2004-2006 $     $Revision 5 $
*********************************************************/
#ifndef CMACTIVE_H
#define CMACTIVE_H

//===Classes Defined======================================
class CMActive;                             // active regions used with mouse

//===Includes=============================================
#include <windows.h>
#include <stdio.h>

//===Typedefs=============================================
typedef struct tagACTIVECREATESTRUCT { /* acs */
   int      left, top, right, bottom;       // Active rectangle coordinates
   HCURSOR  hcMove, hcMoveCtrl;             // cursors depending on Ctrl state
   HCURSOR  hcDrag, hcDragCtrl;
   void   (*CallbackFcn)(int iMsg, int x, int y, int wKeys, void *pVoid, long int lData);
   void    *pVoid;                          // information passed to callback function
   long int lData;
   HMENU    hmContextMenu;                  // menu to display on right-click
} ACTIVECREATESTRUCT;

//===Class Includes=======================================
#include "CChainLink.h"                     // double-sided chain
#include "CMouse.h"                         // parent mouse class



/*********************************************************
* CMActive Class
*********************************************************/
//===Constants============================================
#define CMAI_CUR_MOVEONLY        0          // (index) cursor move
#define CMAI_CUR_MOVECTRL        1          // (index) cursor Ctrl+move
#define CMAI_CUR_DRAGONLY        2          // (index) cursor drag
#define CMAI_CUR_DRAGCTRL        3          // (index) cursor Ctrl+drag
#define CMAI_CUR_NUM             4          // (count) number of cursors

//===CMActive class=======================================
class CMActive : public CChainLink {
   RECT      rcActive;                      // rectangle of Active
   HCURSOR   hcCursor[CMAI_CUR_NUM];        // context-sensitive cursors
   void    (*lpfnCallback)(int,int,int,int,void*,long int); // callback function pointer
   HMENU     hmContextMenu;                 // context-menu
   void     *pVoid;                         // user-defined information..
   long int  lData;                         //..passed to callback function
public:
   CMActive(                                // constructor and initialization
      int left, int top, int right, int bottom,
      HCURSOR hcMove, HCURSOR hcMoveCtrl, HCURSOR hcDrag, HCURSOR hcDragCtrl,
      void (*lpfnCallback)(int iMsg, int x, int y, int wKeys, void *pVoid, long int lData),
      void *pVoid, long int lData, HMENU hmContextMenu);
   ~CMActive();                             // destructor

   void  SetActiveRect(int left, int top, int right, int bottom); // update an active rectangle
   void  SetActiveRect(const RECT *prc) { SetActiveRect(prc->left, prc->top, prc->right, prc->bottom); };
   void  SetCursors(HCURSOR hcMove, HCURSOR hcMoveCtrl, HCURSOR hcDrag, HCURSOR hcDragCtrl); // update the context cursors
   void  SetCallbackFcn(void(*fcn)(int,int,int,int,void*,long int)); // update the callback function
   void  SetUserData(void *pVoid, long int lData);// update the user data
   HMENU SetContextMenu(HMENU hmContextMenu);// update the context-sensitive menu
   void  CallCallback(int iMsg, int x, int y, int wKeys); // call the callback function
   void  PaintActiveRect(HDC hDC);           // paint this active rect onto DC

   //---Access----------------------------------
   RECT* ActiveRect(void)              { return(&rcActive); };
   void  GetActiveRect(RECT *prc);
   HMENU ContextMenu(void)             { return(hmContextMenu); };
   void* PData(void)                   { return(pVoid); };
   long int Data(void)                   { return(lData); };
};

#endif/*CMACTIVE_H*/
