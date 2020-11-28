/*********************************************************
*  CMActive.cpp
*  Mouse active area. Re-worked for LaserCanvas rev. 5
*  $PSchlup 2004-2006 $     $Revision 6 $
* Revision History
*    6  2006oct21   Data to LONG INT so can take pointers
*********************************************************/
#include "CMActive.h"                       // include header file

/*********************************************************
*  ::CMActive
*  Constructor
*********************************************************/
CMActive::CMActive(
      int left, int top, int right, int bottom,
      HCURSOR hcMove, HCURSOR hcMoveCtrl, HCURSOR hcDrag, HCURSOR hcDragCtrl,
      void (*lpfnCallback)(int iMsg, int x, int y, int wKeys, void *pVoid, long int lData),
      void *pVoid, long int lData, HMENU hmRightClick
) {
   SetActiveRect(left, top, right, bottom); // assign the given rectangle
   SetCursors(hcMove, hcMoveCtrl, hcDrag, hcDragCtrl); // assign cursors
   SetCallbackFcn(lpfnCallback);            // assign the callback function
   SetUserData(pVoid, lData);               // assign the data
   SetContextMenu(hmRightClick);            // assign the context menu
}

/*********************************************************
*  ::~CMActive
*  Destructor.
* Note that the CChainLink destructor maintains the chain
*********************************************************/
CMActive::~CMActive() {
   //DBGMSG("CMActive::~CMActive");
}

/*********************************************************
*  ::SetActiveRect
*  Set the rectangle of the Active
*********************************************************/
void CMActive::SetActiveRect(int left, int top, int right, int bottom) {
   rcActive.left   = left;
   rcActive.top    = top;
   rcActive.right  = right;
   rcActive.bottom = bottom;
}

/*********************************************************
*  ::GetActiveRect
*  Returns the current active rectangle
*********************************************************/
void CMActive::GetActiveRect(RECT *prc) {
   CopyRect(prc, &rcActive);
}

/*********************************************************
*  ::SetCursors
*  Set the cursors belonging to the Active
*********************************************************/
void CMActive::SetCursors(HCURSOR hcMove, HCURSOR hcMoveCtrl, HCURSOR hcDrag, HCURSOR hcDragCtrl) {
   hcCursor[CMAI_CUR_MOVEONLY] = hcMove;
   hcCursor[CMAI_CUR_MOVECTRL] = hcMoveCtrl;
   hcCursor[CMAI_CUR_DRAGONLY] = hcDrag;
   hcCursor[CMAI_CUR_DRAGCTRL] = hcDragCtrl;
}

/*********************************************************
*  ::SetCallbackFcn
*  Set the callback function for the Active
*********************************************************/
void CMActive::SetCallbackFcn(void(*fcn)(int,int,int,int,void*,long int)) {
   lpfnCallback = fcn;
}

/*********************************************************
*  ::SetUserData
*  Set the user data passed to the callback function
*********************************************************/
void CMActive::SetUserData(void *_pVoid, long int _lData) {
   pVoid = _pVoid;
   lData = _lData;
}

/*********************************************************
*  ::SetContextMenu
*  Set the context menu belonging to the Active
*  Returns the previous context menu
*********************************************************/
HMENU CMActive::SetContextMenu(HMENU hmCntxt) {
   HMENU hmOld;
   hmOld = hmContextMenu;
   hmContextMenu = hmCntxt;
   return(hmOld);
}


/*********************************************************
* CallCallback
* Call the callback  function. The information  passed to
* the callback is assembled from CMouse information, such
* as mouse coordinates  and key states, and  the CMActive
* user parameters pVoid and iData.
* Since all messages  pass through here,  we also set the
* mouse cursors here. If specialized cursors are not pro-
* scribed, the MOVEONLY cursor is used.
*********************************************************/
void CMActive::CallCallback(int iMsg, int x, int y, int wKeys) {
   int   iCur;                              // cursor index
   //---Cursors-----------------------
   iCur = (iMsg==ACSM_DRAG) ?
      ((wKeys & MK_CONTROL) ? CMAI_CUR_DRAGCTRL : CMAI_CUR_DRAGONLY) :
      ((wKeys & MK_CONTROL) ? CMAI_CUR_MOVECTRL : CMAI_CUR_MOVEONLY);
   if(hcCursor[iCur])                   SetCursor(hcCursor[iCur]);
   else if(hcCursor[CMAI_CUR_MOVEONLY]) SetCursor(hcCursor[CMAI_CUR_MOVEONLY]);
   //else                                SetCursor(LoadCursor(NULL, IDC_ARROW));

   //---Callback----------------------
   if(lpfnCallback) (*lpfnCallback)(iMsg, x, y, wKeys, pVoid, lData);
}


/*********************************************************
*  ::PaintActiveRect
*  Paint the active rectangle onto a device context
*********************************************************/
void CMActive::PaintActiveRect(HDC hDC) {
   HGDIOBJ hgdiPen;
   HGDIOBJ hgdiBrush;
   HPEN    hpAct;

   if(!IsRectEmpty(&rcActive)) {
      // create, select, draw, deselect, delete:
      hpAct = CreatePen(PS_DOT, 1, RGB(0xCC, 0x33, 0x33));
      hgdiPen = SelectObject(hDC, hpAct);
      hgdiBrush = SelectObject(hDC, (HBRUSH) GetStockObject(NULL_BRUSH));
      Rectangle(hDC, rcActive.left, rcActive.top,
         rcActive.right, rcActive.bottom);
      SelectObject(hDC, hgdiBrush);
      SelectObject(hDC, hgdiPen);
      DeleteObject(hpAct);
   }
}
