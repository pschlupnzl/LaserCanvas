/*********************************************************
*  CMouse.cpp
* 	Mouse class, with active bits and callbacks and menus.
*  All very exciting.  This works surprisingly well.
*  $PSchlup 2004-2006 $     $Revision 6 $
* Revision History
*   6  2006oct22   Added middle button support
*********************************************************/
#include "CMouse.h"

/*********************************************************
*  ::CMouse
*  Constructor
*********************************************************/
CMouse::CMouse(void) {
   //---Members---------------------------------
   pActTop          = NULL;                 // set all variables to null
   pActCurrent      = NULL;
   tfDown = tfDrag  = FALSE;
   tfMDown          = FALSE;
   hwCapture        = NULL;
   ptDownLocation.x = 0;
   ptDownLocation.y = 0;
   wKeysDown        = NULL;
   ptAuxData.x = ptAuxData.y = 0;
}


/*********************************************************
*  ::~CMouse
*  Destructor
*********************************************************/
CMouse::~CMouse() {
   if(hwCapture != NULL) CMReleaseCapture();

   //---Delete all actives----------------------
   while(pActTop) DeleteActive(pActTop);
}

/*********************************************************
*  ::CMSetCapture
*  ::CMReleaseCapture
*  Set / release the mouse capture window
*  This is used during a drag to ensure BUTTONUP messages
*  are recorded
*********************************************************/
void CMouse::CMSetCapture(HWND hWnd) {
   if(hwCapture != hWnd) {
      hwCapture = hWnd;
      SetCapture(hwCapture);
   }
}
void CMouse::CMReleaseCapture(void) {
   if((hwCapture != NULL) && (GetCapture() == hwCapture)) {
      ReleaseCapture();
   }
   hwCapture = NULL;
}


/*********************************************************
*  ::CreateActive
* Create a new active region and return pointer to it.
* The pointer is needed  when the Active is to be deleted
* or when  its properties need updating, although  it can
* also be retrieved  using FindActive. The  new active is
* inserted at the top of the chain.
*********************************************************/
CMActive* CMouse::CreateActive(
      int left, int top, int right, int bottom,
      HCURSOR hcMove, HCURSOR hcMoveCtrl, HCURSOR hcDrag, HCURSOR hcDragCtrl,
      void (*CallbackFcn)(int iMsg, int x, int y, int wKeys, void *pVoid, long int lData),
      void *pVoid, long int lData, HMENU hmContextMenu
) {
   CMActive *pNewAct;                       // new active element

   pNewAct = new CMActive(                  // create the Active
      left, top, right, bottom,
      hcMove, hcMoveCtrl, hcDrag, hcDragCtrl,
      CallbackFcn, pVoid, lData, hmContextMenu);

   if(pNewAct == NULL) return(NULL);        // failed to create -- this shouldn't happen

   if(pActTop != NULL) pActTop->InsertBefore(pNewAct); // insert new active at top of chain
   pActTop = pNewAct;               // point to top of chain
   return(pNewAct);
}

/*********************************************************
*  ::CreateActiveIndirect
*  Create a new Active using the create structure
*********************************************************/
CMActive* CMouse::CreateActiveIndirect(ACTIVECREATESTRUCT *acs) {
   return(
      CreateActive(acs->left, acs->top, acs->right, acs->bottom,
         acs->hcMove, acs->hcMoveCtrl, acs->hcDrag, acs->hcDragCtrl,
         acs->CallbackFcn, acs->pVoid, acs->lData, acs->hmContextMenu)
   );
}

/*********************************************************
* ::DeleteAllActive
* Deletes all of the actives in the chain.
* ::DeleteActive
* Delete an active region
* Use this method to delete  Actives created with Create-
* Active or CreateActiveIndirect.  Note that the destruc-
* tor of the CChainLink class  maintains the chain on de-
* letion.
*********************************************************/
//===DeleteAllActive======================================
void CMouse::DeleteAllActive(void) {
   while(pActTop) DeleteActive(pActTop);
}
//===DeleteActive=========================================
void CMouse::DeleteActive(CMActive *pDelAct) {
   if(pDelAct == NULL) return;
   if(pDelAct == pActTop) {                 // deleting first: point to next
      pActTop = (CMActive*) pActTop->Next();
   }
   delete pDelAct;                          // delete object - the chaindestructor maintains chain
}

/*********************************************************
*  ::PaintAllActiveRect
*  Paints all the active rectangles onto a device context
*  This is a debugging tool
*********************************************************/
void CMouse::PaintAllActiveRect(HDC hDC) {
   for(CMActive *pAct=pActTop; pAct; pAct=(CMActive*)pAct->Next()) {
      pAct->PaintActiveRect(hDC);
   }
}


/*********************************************************
*  ::MouseProc
* This function should be called for any Windows messages
* pertaining to mouse movements, including
*     WM_MOUSEMOVE
*     WM_LBUTTONDOWN
*     WM_LBUTTONUP
*     WM_LBUTTONDBLCLK
*     WM_RBUTTONDOWN
*     WM_RBUTTONDBLCLK
*     WM_MBUTTONUP
*     WM_MBUTTONDOWN
* Keyboard messages are currently NOT handled; the states
* of control and shift keys are read with GetKeyState.
* The function returns TRUE if  the message has been han-
* dled, and zero if the message did not interact with the
* CMouse object, although these values are a little vague
*********************************************************/
BOOL CMouse::MouseProc(HWND hWnd, UINT uMsg, int x, int y) {
   POINT pt;                                // screen coordinate for popup menu
   int   wKeys;                             // status of shift and control keys

   //===Preliminaries=====================================
   if(x > 32767) x -= 65536;                // wrap window boundaries
   if(y > 32767) y -= 65536;                // wrap window boundaries
   pt.x = x; pt.y = y;                      // assemble point

   //---Keys------------------------------------
   if(tfDown) {
      wKeys = wKeysDown;                    // maintain keys at press time
   } else {
      wKeys = ((GetKeyState(VK_SHIFT)&0x8000) ? MK_SHIFT : 0)
         | ((GetKeyState(VK_CONTROL)&0x8000) ? MK_CONTROL : 0);
   }

   switch(uMsg) {
   case WM_MOUSEMOVE:
      //===Drag===========================================
      if(tfDown || tfMDown) {
         if(!tfDrag) {                      // snap before dragging
            if(   (x-ptDownLocation.x) > CM_DRAGTHRESHOLD
               || (ptDownLocation.x-x) > CM_DRAGTHRESHOLD
               || (y-ptDownLocation.y) > CM_DRAGTHRESHOLD
               || (ptDownLocation.y-y) > CM_DRAGTHRESHOLD) {
               tfDrag = TRUE;
            }
         }                                  // if it's a drag now, call the callback, if any
         if(pActCurrent && tfDrag) {
            pActCurrent->CallCallback(
               tfDown  ? ACSM_DRAG :
               tfMDown ? ACSM_MDRG : ACSM_DRAG,
               x, y, wKeys);
         }

      //===Move===========================================
      } else {                              // standard move
         for(pActCurrent=pActTop; pActCurrent; pActCurrent=(CMActive*)pActCurrent->Next()) {
            if(PtInRect(pActCurrent->ActiveRect(), pt)) break; // stop if point found
         }
         if(pActCurrent) pActCurrent->CallCallback(ACSM_MOVE, x, y, wKeys);
      }
      break;

   //===Left Down=========================================
   case WM_LBUTTONDOWN:
      if(tfMDown) break;                    // ignore if left is down
      ptDownLocation.x = x;                 // record location
      ptDownLocation.y = y;
      tfDown = TRUE;                        // track mouse button status
      wKeysDown = wKeys;                    // store down action keys
      CMSetCapture(hWnd);                   // capture mouse to window
      if(pActCurrent) pActCurrent->CallCallback(ACSM_DOWN, x, y, wKeys);
      break;

   //===Left Up===========================================
   case WM_LBUTTONUP:
      if(tfMDown) break;                    // ignore if left is down
      if(tfDown) CMReleaseCapture();        // release capture first
      if(pActCurrent) pActCurrent->CallCallback(
         tfDrag ? ACSM_DEND : ACSM_LEFT, x, y, wKeys);
      tfDown = FALSE;                       // track mouse button..
      tfDrag = FALSE;                       //..and dragging status
      for(pActCurrent=pActTop; pActCurrent; pActCurrent=(CMActive*)pActCurrent->Next()) {
         if(PtInRect(pActCurrent->ActiveRect(), pt)) break; // stop if point found
      }                                     // find current active
      break;

   //===Left Double Click=================================
   // The window class must be registered with CS_DBLCLKS
   case WM_LBUTTONDBLCLK:
      if(pActCurrent) pActCurrent->CallCallback(ACSM_DBLK, x, y, wKeys);
      break;

   //===Middle Button=====================================
   case WM_MBUTTONDOWN:
      if(tfDown) break;                     // ignore if left is down
      ptDownLocation.x = x;                 // record location
      ptDownLocation.y = y;
      tfMDown = TRUE;                       // track mouse button status
      wKeysDown = wKeys;                    // store down action keys
      CMSetCapture(hWnd);                   // capture mouse to window
      if(pActCurrent) pActCurrent->CallCallback(ACSM_MDDN, x, y, wKeys);
      break;

   //===Left Up===========================================
   case WM_MBUTTONUP:
      if(tfDown) break;                     // ignore if left is down
      if(tfMDown) CMReleaseCapture();       // release capture first
      if(pActCurrent) pActCurrent->CallCallback(
         tfDrag ? ACSM_MEND : ACSM_MDCK, x, y, wKeys);
      tfMDown = FALSE;                      // track mouse button..
      tfDrag = FALSE;                       //..and dragging status
      for(pActCurrent=pActTop; pActCurrent; pActCurrent=(CMActive*)pActCurrent->Next()) {
         if(PtInRect(pActCurrent->ActiveRect(), pt)) break; // stop if point found
      }                                     // find current active
      break;


   //===Right Down========================================
   // We'd have the  CMActive manage  the popup menu, but
   // it's much easier to gain access to the window here.
   case WM_RBUTTONDOWN:
      if(pActCurrent==NULL) return(FALSE);  // no current active, so no change
      pActCurrent->CallCallback(ACSM_RBDN, x, y, wKeys);
      if(pActCurrent->ContextMenu()!=NULL) {
         ClientToScreen(hWnd, &pt);         // map the coordinates to the screen
         TrackPopupMenu(pActCurrent->ContextMenu(),
            TPM_LEFTALIGN, pt.x, pt.y, 0, hWnd, NULL);
      }
      break;

   //===Others============================================
   default:
      if(pActCurrent) pActCurrent->CallCallback(ACSM_NONE, x, y, wKeys);
      return(FALSE);
   }

   return(TRUE);
}

/*********************************************************
*  ::FindActiveByData
*  Returns a pointer to the first Active element in the
*  chain whose data members match the arguments
*********************************************************/
CMActive* CMouse::FindActiveByData(void *pVoidIn, long int lDataIn) {
   CMActive *pAct;
   for(pAct=pActTop; pAct; pAct=(CMActive*)pAct->Next()) {
      if((pAct->PData() == pVoidIn) && (pAct->Data() == lDataIn)) break;
   }
   return(pAct);
}

