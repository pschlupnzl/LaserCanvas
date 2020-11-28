/*********************************************************
* CPropMgr
* Property manager  for LaserCanvas 5. It  is anticipated
* that there will be only one PropMgr object instantiated
* per application instance.
*
* This is quite  a doozey, but  the code  is surprisingly
* compact. The solution, adopted from rev 4, is to make a
* pre-defined list of items for the property manager, ra-
* ther than trying to keep a dynamic list of only the ne-
* cessary items.
*
* The callbacks are (approximately) as follows:
*    CQuickEdit
*    -> CMouse
*       -> CMActive
*          -> CPropItem
*             -> CWinSys
*
* Who manages  the visibility  and callback  functions of
* the property items? The  renderer, because the renderer
* type determines which items should be visible at all.
*
* $PSchlup 2000-2006 $     $Revision 5 $
*********************************************************/
#include "CPropMgr.h"                       // class header

/**********************************************************
* Constructor
**********************************************************/
CPropMgr::CPropMgr(CApplication *pApp) {
   WNDCLASS wc;                             // window class
   //---Members---------------------------------
   pAppParent    = pApp;                    // parent application
   pMouse        = NULL;                    // no mouse object
   pItemTop      = NULL;                    // no item chain
   pQEdit        = NULL;                    // no quick edit yet
   pQMenu        = NULL;                    // no quick menu yet
   hwPropMgr     = NULL;                    // no window yet
   uFlags        = 0;                       // no flags
   iWinWidth     = 280;                     // width of window (docked)
   iColWidth     = 140;                      // width of left column
   hwScrollBar   = NULL;                    // no scroll bar yet
   iScrollWid    = 16;                      // default scroll bar width
   iScrollOffset = 0;                       // no scroll offset
   pSysWinActive = NULL;                    // active window

   //---Mouse-----------------------------------
   pMouse = new CMouse();                   // new mouse active for sheet
   pMouse->CreateActive(0, 0, 100, 10,
      LoadCursor(NULL, IDC_SIZEWE), NULL, NULL, NULL,
      CPropMgr::MouseCallback, (void*) this, CPMC_ACT_COLUMN, NULL);
   pMouse->CreateActive(0, 0, 10, 100,
      LoadCursor(NULL, IDC_SIZEWE), LoadCursor(NULL, IDC_HAND), NULL, NULL,
      CPropMgr::MouseCallback, (void*) this, CPMC_ACT_WIDTH, NULL);

   //===Window============================================
   wc.style         = CS_HREDRAW | CS_VREDRAW;
   wc.hbrBackground = (HBRUSH) NULL;//(COLOR_WINDOW + 1);
   wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
   wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
   wc.cbClsExtra    = NULL;
   wc.cbWndExtra    = sizeof(CPropMgr*);
   wc.hInstance     = pApp->GetInstance();
   wc.lpszMenuName  = (LPSTR) NULL;
   wc.lpszClassName = CPM_SZCLASSNAME;
   wc.lpfnWndProc   = CPropMgr::WndProcPropMgr;
   RegisterClass(&wc);

}

/**********************************************************
* Destructor
**********************************************************/
CPropMgr::~CPropMgr() {
   int k;                                   // loop counter

   //---Window----------------------------------
   if(pQEdit)    delete(pQEdit);           pQEdit = NULL;
   if(pQMenu)    delete(pQMenu);           pQMenu = NULL;
   if(hwPropMgr) DestroyWindow(hwPropMgr); hwPropMgr = NULL;

   //---Items-----------------------------------
   if(pMouse) delete(pMouse); pMouse = NULL; // delete mouse and its objects
   while(pItemTop) DeleteItem(pItemTop);
}


/*********************************************************
* CreateMgrWindow
* Creates, or re-creates, the property manager window, in
* either docked or floating view. If the window dock sta-
* tus is to be changed, use
*    SetBit(CPMF_DOCKED);   |   ClearBit(CPMF_DOCKED);
*    CreateMgrWindow();
*********************************************************/
HWND CPropMgr::CreateMgrWindow(void) {
   RECT rc;                                 // to calculate scroll bar width
   if(hwPropMgr != NULL) DestroyMgrWindow(); // destroy before recreating

   //===Property Manager Window===========================
   //---Docked------------------------
   if(CheckBit(CPMF_DOCKED)) {
      hwPropMgr = CreateWindow(CPM_SZCLASSNAME, (LPSTR) NULL,
         WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
         0, 0, iWinWidth, 400, pAppParent->GetWindow(),
         (HMENU) NULL, pAppParent->GetInstance(), (LPVOID) this);
   //---Floating----------------------
   } else {
      hwPropMgr = CreateWindowEx(WS_EX_TOOLWINDOW, CPM_SZCLASSNAME, CPM_SZWINDOWTITLE,
         WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU,
         CW_USEDEFAULT, CW_USEDEFAULT, iWinWidth, 400, pAppParent->GetWindow(),
         (HMENU) NULL, pAppParent->GetInstance(), (LPVOID) this);
   }

   //===Child Windows=====================================
   //---Quick controls--------------------------
   pQEdit = new CQuickEdit(hwPropMgr);
   pQEdit->SetFont(Font());
   pQMenu = new CQuickMenu(hwPropMgr);
   pQMenu->SetFont(Font());

   //---Scroll Bar------------------------------
   hwScrollBar = CreateWindow("SCROLLBAR", (LPSTR) NULL,
      WS_CHILD | SBS_VERT | SBS_RIGHTALIGN | WS_VISIBLE,
      0, 0, 32, 32,
      hwPropMgr, (HMENU) NULL, pAppParent->GetInstance(), NULL);
   GetClientRect(hwScrollBar, &rc);
   iScrollWid = rc.right - rc.left;         // establish scroll bar width

   //===Positioning=======================================
   pAppParent->OnResize();                  // resize all child windows
   ShowWindow(hwPropMgr, SW_SHOW);          // display the window
   return(hwPropMgr);                       // return new handle, in case needed
}


/*********************************************************
* DestroyMgrWindow
*********************************************************/
void CPropMgr::DestroyMgrWindow(void) {
   if(pQEdit)    delete(pQEdit);           pQEdit = NULL;
   if(hwPropMgr) DestroyWindow(hwPropMgr); hwPropMgr = NULL;
}


/*********************************************************
* WndProcPropMgr
* This callback function must be declared static
*********************************************************/
LRESULT CALLBACK CPropMgr::WndProcPropMgr(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
   CPropMgr *pMgr;                          // object
char sz[256];

   pMgr = (CPropMgr*) GetWindowLong(hWnd, 0); // not valid on WM_CREATe
   switch(uMsg) {
   case WM_CREATE:
      pMgr = (CPropMgr*) (((LPCREATESTRUCT) lParam)->lpCreateParams);
      SetWindowLong(hWnd, 0, (LONG) pMgr);
      break;
   case WM_DESTROY:
      if(pMgr->hwScrollBar) DestroyWindow(pMgr->hwScrollBar);
      pMgr->hwScrollBar = NULL;
      break;
   case WM_CLOSE: pMgr->pAppParent->UserViewProperties(FALSE); break;
   case WM_SIZE:  pMgr->OnResize(); break;
   case WM_PAINT: pMgr->OnPaint(FALSE); break;

   case WM_MOUSEMOVE:
   case WM_LBUTTONDOWN:
   case WM_LBUTTONUP:
   case WM_RBUTTONDOWN:
   case WM_RBUTTONUP:
      if(pMgr->pMouse) pMgr->pMouse->MouseProc(hWnd, uMsg, LOWORD(lParam), HIWORD(lParam));
      break;

   case WM_VSCROLL: pMgr->OnScrollBar(LOWORD(wParam), HIWORD(wParam)); break;

   default: return(DefWindowProc(hWnd, uMsg, wParam, lParam));
   }
   return(0L);
}

/*********************************************************
* OnParentResize
* Called when the parent  application window is resized.
* The supplied rectangle is modified to reflect the part
* of the client area that remains after the manager win-
* dow has been placed. See CApplication::OnResize().
*********************************************************/
void CPropMgr::OnParentResize(LPRECT lprc) {
   if(hwPropMgr==NULL) return;              // ignore if no window
   //---Docked----------------------------------
   if(CheckBit(CPMF_DOCKED)) {
      switch(CheckBit(CPMF_DOCKPOS)) {
      case CPMF_DOCKLEFT:
         MoveWindow(hwPropMgr, lprc->left, lprc->top, iWinWidth, lprc->bottom-lprc->top, TRUE);
         SetRect(lprc, lprc->left+iWinWidth, lprc->top, lprc->right, lprc->bottom);
         break;
      case CPMF_DOCKRIGHT:
         MoveWindow(hwPropMgr, lprc->right-iWinWidth, lprc->top, iWinWidth, lprc->bottom-lprc->top, TRUE);
         SetRect(lprc, lprc->left, lprc->top, lprc->right-iWinWidth, lprc->bottom);
         break;
      }
   }
}


/*********************************************************
* OnResize
* Called when the window is resized.
* The dock frame and scroll bar sizing code is duplicated
* here and in PositionItems, but I can't see a way around
* that: It serves different purposes
*********************************************************/
void CPropMgr::OnResize(void) {
   RECT      rc;                           // client rectangle
   CMActive *pActCol, *pActWin;            // actives to modify
   int       iWid;                         // scroll bar width

   if(hwPropMgr==NULL) return;             // ignore if window not visible

   //===Preliminaries=====================================
   GetClientRect(hwPropMgr, &rc);           // get window client area
   if(!CheckBit(CPMF_DOCKED)) iWinWidth = rc.right - rc.left; // track window width

   //---Column width----------------------------
   if(iColWidth > rc.right-48) iColWidth = rc.right-48;
   if(iColWidth < 32) iColWidth = 32;       // set limits to width

   //---Scroll bar------------------------------
   if(hwScrollBar) {
      if(CheckBit(CPMF_DOCKED)) switch(CheckBit(CPMF_DOCKPOS)) {
         case CPMF_DOCKLEFT:  MoveWindow(hwScrollBar, rc.right-CPMC_FRAMEDRAGWID-iScrollWid, rc.top, iScrollWid, rc.bottom-rc.top, TRUE); break;
         case CPMF_DOCKRIGHT: MoveWindow(hwScrollBar, rc.right-iScrollWid, rc.top, iScrollWid, rc.bottom-rc.top, TRUE); break;
      } else MoveWindow(hwScrollBar, rc.right-iScrollWid, rc.top, iScrollWid, rc.bottom-rc.top, FALSE);
   }

   //===Items=============================================
   PositionItems();                         // recalculate item rectangles

   //===Manager Actives===================================
   if(pMouse!=NULL) {                       // ignore if no mouse object
      pActCol = pMouse->FindActiveByData((void*)this, CPMC_ACT_COLUMN);
      pActWin = pMouse->FindActiveByData((void*)this, CPMC_ACT_WIDTH);

      if(CheckBit(CPMF_DOCKED)) switch(CheckBit(CPMF_DOCKPOS)) {
      //---Left---------------------------------
      case CPMF_DOCKLEFT:
         if(pActCol) pActCol->SetActiveRect(rc.left+iColWidth-CPMC_COLDRAGWID, rc.top, rc.left+iColWidth+CPMC_COLDRAGWID, iItemEnd);
         if(pActWin) pActWin->SetActiveRect(rc.right-CPMC_FRAMEDRAGWID, rc.top, rc.right, rc.bottom);
         break;

      //---Right--------------------------------
      case CPMF_DOCKRIGHT:
         if(pActCol) pActCol->SetActiveRect(rc.left+CPMC_FRAMEDRAGWID+iColWidth-CPMC_COLDRAGWID, rc.top, rc.left+CPMC_FRAMEDRAGWID+iColWidth+CPMC_COLDRAGWID, iItemEnd);
         if(pActWin) pActWin->SetActiveRect(rc.left, rc.top, rc.left+CPMC_FRAMEDRAGWID, rc.bottom);
         break;

      //---Floating-----------------------------
      } else {
         if(pActCol) pActCol->SetActiveRect(rc.left+iColWidth-CPMC_COLDRAGWID, rc.top, rc.left+iColWidth+CPMC_COLDRAGWID, iItemEnd);
         if(pActWin) pActWin->SetActiveRect(0, 0, 0, 0);
      }
   }
}


/*********************************************************
* PositionItems
* Called when  the window is  resized, or when  the items
* need to be re-positioned due to e.g. content change.
* The items' positions  are real window  coordinates. The
* positioning code may need to be rendered twice, once to
* establish the  extent and a second time to  ensure that
* the scroll limits are valid.  This may also allow us to
* remove the scroll bar if it's not needed.
* The item's  SetPosition function  modifies the  top and
* bottom members of the rectangle according to its height
* and visibility status
* Called from <- OnResize()
*********************************************************/
void CPropMgr::PositionItems(void) {
   RECT       rc, rcClient;                 // reduced, client rectangle
   CPropItem *pItem;                        // item loop counter
   SCROLLINFO si;                           // information for scroll bar

   //===Prepare===========================================
   if(hwPropMgr==NULL) return;              // ignore if no window present
   GetClientRect(hwPropMgr, &rcClient);     // read full window dimensions

   //---Dock frame reduction--------------------
   CopyRect(&rc, &rcClient);                // start with full window
   if(CheckBit(CPMF_DOCKED)) switch(CheckBit(CPMF_DOCKPOS)) {
   case CPMF_DOCKLEFT:  rc.right -= CPMC_FRAMEDRAGWID; break;
   case CPMF_DOCKRIGHT: rc.left  += CPMC_FRAMEDRAGWID; break;
   }
   if(hwScrollBar) rc.right -= iScrollWid;  // reduce draggers and scroll bar

   //===Item Positions====================================
   //---Item heights----------------------------
   if(iScrollOffset < 0) iScrollOffset = 0; // flush top
   rc.bottom = -iScrollOffset;              // calculate ends only
   for(pItem=pItemTop; pItem; pItem=(CPropItem*)pItem->Next()) {
      rc.bottom += pItem->Height(); // set row height
   }

   //---Remaining space-------------------------
   if((rc.bottom<rcClient.bottom) && (iScrollOffset>0)) { // possibly scroll down
      iScrollOffset -= (rcClient.bottom-rc.bottom); // move right to end
      if(iScrollOffset < 0) iScrollOffset = 0;      // ensure flush at top
   }

   //---Position items-------------------------
   rc.top = -iScrollOffset;                 // start above window, if necessary
   for(pItem=pItemTop; pItem; pItem=(CPropItem*)pItem->Next()) {
      rc.bottom = rc.top + pItem->Height(); // set row height
      pItem->SetPosition(&rc);              // set the item position
      rc.top = rc.bottom;                   // start next at end of previous
   }
   iItemEnd = rc.bottom;                    // end of items vertically

   //===Remaining space===================================
   if(hwScrollBar) {
      memset(&si, 0x00, sizeof(si));
      si.cbSize = sizeof(si);
      si.fMask  = SIF_DISABLENOSCROLL | SIF_PAGE | SIF_POS | SIF_RANGE;
      si.nMin   = 0;
      si.nMax   = iScrollOffset + rc.bottom;
      si.nPage  = rcClient.bottom-rcClient.top;
      si.nPos   = iScrollOffset;
      SetScrollInfo(hwScrollBar, SB_CTL, &si, TRUE);
   }
}

/*********************************************************
* OnScrollBar
* Called for all the messages posted by the scroll bar.
*********************************************************/
void CPropMgr::OnScrollBar(int iCode, int nPos) {
   RECT rcClient;                           // client rectangle

   if(hwPropMgr==NULL) return;              // ignore if no window
   GetClientRect(hwPropMgr, &rcClient);
   switch(iCode) {
      case SB_LINEDOWN: iScrollOffset += CPMC_SCROLLLINE; break;
      case SB_LINEUP:   iScrollOffset -= CPMC_SCROLLLINE; break;
      case SB_PAGEDOWN: iScrollOffset += rcClient.bottom - rcClient.top - CPMC_SCROLLLINE; break;
      case SB_PAGEUP:   iScrollOffset -= rcClient.bottom - rcClient.top - CPMC_SCROLLLINE; break;
      case SB_THUMBTRACK: // comment out to disable live scrolling
      case SB_THUMBPOSITION:
         iScrollOffset = nPos;
         break;
   }
   PositionItems();                         // reposition all items
   InvalidateRect(hwPropMgr, NULL, TRUE);   // full repaint required
}


/*########################################################
 ## Paint                                              ##
 ## This will be long                                  ##
########################################################*/
/*********************************************************
* OnPaint
* Paint the manager window.  Some of the cosmetics depend
* on whether the window is docked or not.
*********************************************************/
#define fnPaintFrame(hdc, prc, x) {\
   SelectObject(hdc, Pen(CAPPI_PEN3DTEXT)); MoveToEx(hdc, x  , (prc)->top,  NULL); LineTo(hdc, x  , (prc)->bottom);\
   SelectObject(hdc, Pen(CAPPI_PEN3DLITE)); MoveToEx(hdc, x+1, (prc)->top,  NULL); LineTo(hdc, x+1, (prc)->bottom);\
   SelectObject(hdc, Pen(CAPPI_PEN3DFACE)); MoveToEx(hdc, x+2, (prc)->top,  NULL); LineTo(hdc, x+2, (prc)->bottom);\
   SelectObject(hdc, Pen(CAPPI_PEN3DFACE)); MoveToEx(hdc, x+3, (prc)->top,  NULL); LineTo(hdc, x+3, (prc)->bottom);\
   SelectObject(hdc, Pen(CAPPI_PEN3DDARK)); MoveToEx(hdc, x+4, (prc)->top,  NULL); LineTo(hdc, x+4, (prc)->bottom);\
   SelectObject(hdc, Pen(CAPPI_PEN3DTEXT)); MoveToEx(hdc, x+5, (prc)->top,  NULL); LineTo(hdc, x+5, (prc)->bottom);\
}
void CPropMgr::OnPaint(BOOL tfTextOnly) {
   PAINTSTRUCT ps;                          // painting structure
   CPropItem  *pItem;                       // item loop counter
   RECT        rcClient, rc;                // client rectangle, reduced by frame
   HDC         hdcMem, hdc;                 // memory, used contex
   HBITMAP     hbmMem, hbmOld;              // memory bitmap, restore

   //===Prepare Text Only=================================
   if(hwPropMgr==NULL) return;              // ignore if no manager

   if(tfTextOnly) {
      hdc = GetDC(hwPropMgr);               // paint directly to window
      SaveDC(hdc);                          // save DC status
      SelectObject(hdc, Font());            // all text in default font

   //===Prepare Full Paint================================
   // Note: OnResize spaces the item rectangles correctly
   // with dragger  and scroll bar, so we must  paint the
   // entire window on the memory DC
   } else {
      //---Device context-----------------------
      GetClientRect(hwPropMgr, &rcClient);  // assume painting whole window
      BeginPaint(hwPropMgr, &ps);           // erase background

      //---Memory DC----------------------------
      hdc = ps.hdc;                         // assume painting directly
      hdcMem = CreateCompatibleDC(ps.hdc);  // create memory DC
      if(hdcMem) {
         hbmMem = CreateCompatibleBitmap(ps.hdc, rcClient.right-rcClient.left, rcClient.bottom-rcClient.top);
         if(hbmMem) {                       // DC, bitmap created ok
            hbmOld = (HBITMAP) SelectObject(hdcMem, hbmMem);
            hdc = hdcMem;                   // use memory context
         } else {                           // if bitmap not ok..
            DeleteDC(hdcMem); hdcMem = NULL; //..paint directly
         }
      }
      SaveDC(hdc);                          // save dc status
      SelectObject(hdc, Font());            // all text in default font

      //---Docked Frame----------------------------
      if(CheckBit(CPMF_DOCKED)) switch(CheckBit(CPMF_DOCKPOS)) {
      case CPMF_DOCKLEFT:  fnPaintFrame(hdc, &rcClient, rcClient.right-CPMC_FRAMEDRAGWID); break;
      case CPMF_DOCKRIGHT: fnPaintFrame(hdc, &rcClient, rcClient.left); break;
      }
      //---Below Elements-----------------------
      if(iItemEnd < rcClient.bottom) {
         SetRect(&rc, rcClient.left, iItemEnd+1, rcClient.right, rcClient.bottom);
         if(CheckBit(CPMF_DOCKED)) switch(CheckBit(CPMF_DOCKPOS)) {
         case CPMF_DOCKLEFT:  rc.right -= CPMC_FRAMEDRAGWID; break;
         case CPMF_DOCKRIGHT: rc.left  += CPMC_FRAMEDRAGWID; break;
         }
         SelectObject(hdc, Pen(CAPPI_PEN3DDARK));
         MoveTo(hdc, rc.left, rc.top-1);
         LineTo(hdc, rc.right, rc.top-1);
         FillRect(hdc, &rc, Brush(CAPPI_BRUSH3DFACE));
      }
   }

   //===Elements==========================================
   for(pItem=pItemTop; pItem; pItem=(CPropItem*)pItem->Next()) {
      pItem->PaintItem(hdc, tfTextOnly);
   }

   //---Mouse-----------------------------------
   //if(pMouse) pMouse->PaintAllActiveRect(hdc);

   //===Finalize==========================================
   //---Text only-------------------------------
   if(tfTextOnly) {
      RestoreDC(hdc, -1);                   // restore DC settings
      ReleaseDC(hwPropMgr, hdc);            // finished painting

   //---Full painting---------------------------
   } else {
      if(hdcMem) {
         BitBlt(ps.hdc, rcClient.left, rcClient.top, rcClient.right-rcClient.left,
            rcClient.bottom-rcClient.top, hdcMem, 0, 0, SRCCOPY);
         SelectObject(hdcMem, hbmOld);
         DeleteObject(hbmMem);
         DeleteDC(hdcMem);
      }
      EndPaint(hwPropMgr, &ps);             // finish painting
   }
}




/*########################################################
 ## Mouse Feedback                                     ##
########################################################*/
/*********************************************************
* Mouse Wrappers
*********************************************************/
//===Create Active========================================
CMActive* CPropMgr::CreateActive(int left, int top, int right, int bottom,
      HCURSOR hcMove, HCURSOR hcMoveCtrl, HCURSOR hcDrag, HCURSOR hcDragCtrl,
      void (*lpfnCallback)(int iMsg, int x, int y, int wKeys, void *pVoid, long int lData),
      void *pVoid, long int lData, HMENU hmContextMenu) {
   if(pMouse==NULL) return(NULL);           // don't create if no mouse
   return( pMouse->CreateActive(            // pass on to mouse to create
      left, top, right, bottom,
      hcMove, hcMoveCtrl, hcDrag, hcDragCtrl,
      lpfnCallback, pVoid, lData, hmContextMenu) );
}

//===Delete Active========================================
void CPropMgr::DeleteActive(CMActive *pActDel) {
   if(pMouse==NULL) return;                 // ignore if no mouse
   pMouse->DeleteActive(pActDel);           // delete the active
}


/*********************************************************
* MouseCallback
* This function is called by the Mouse for mouse messages
* that occur within the column width and window width ac-
* tive rectangles. Here we're only interested in drag re-
* sizing.
* This referenced function must be declared static.
*********************************************************/
void CPropMgr::MouseCallback(int iMsg, int x, int y, int wKeys, void *pData, long int lData) {
UNREFERENCED_PARAMETER(y);
UNREFERENCED_PARAMETER(wKeys);
   CPropMgr *pMgr;                          // pointer to property manager object

   pMgr = (CPropMgr*) pData;                // pointer passed from active callback
   switch(iMsg) {
   case ACSM_DRAG:
      switch(lData) {
      case CPMC_ACT_COLUMN:
         pMgr->iColWidth = x;               // set the new width value
         pMgr->OnResize();                  // refresh active rectangles
         InvalidateRect(pMgr->hwPropMgr, NULL, TRUE); // paint
         break;
      case CPMC_ACT_WIDTH:
         if(pMgr->CheckBit(CPMF_DOCKPOS)==CPMF_DOCKRIGHT) x = pMgr->iWinWidth - x;
         if(x < 96) x = 96;                 // minimum width
         pMgr->iWinWidth = x;               // set the new width value
         pMgr->pAppParent->OnResize();      // parent application needs to resize itself
         InvalidateRect(pMgr->hwPropMgr, NULL, TRUE); // paint
         break;
      }
      break;
   case ACSM_LEFT:                          // Easter egg: Toggle side
      if(wKeys & MK_CONTROL) {
         if(wKeys & MK_SHIFT) {
            pMgr->pAppParent->UserDockProperties(FALSE);
         } else {
            pMgr->ToggleBit(CPMF_DOCKRIGHT);
         }
      }
      pMgr->pAppParent->OnResize();         // parent needs to resize
      pMgr->OnResize();                     // this needs to resize
      InvalidateRect(pMgr->hwPropMgr, NULL, TRUE); // paint
      break;
   }
}


/*########################################################
 ## Items                                              ##
########################################################*/
/*********************************************************
* NewResourceItem
* Creates a  new item from the resource table  at the end
* of the item list.
* The function has a huge argument list, but it's intend-
* ed to satisfy a specific  requirement: To rapidly build
* the property item list. Note  lpfn and pVoid default to
* NULL if not supplied
*********************************************************/
CPropItem* CPropMgr::NewResourceItem(UINT uResourceID, int iType, UINT uFlags, PROPITEMCALLBACK lpfn, void *pVoid) {
   char       szLabel[256];                 // label text from resource
   CPropItem *pNew;                         // newly instantiated item

   sprintf(szLabel, "Resource%d", uResourceID); // default text if not found
   LoadString(pAppParent->GetInstance(), uResourceID, szLabel, sizeof(szLabel) / sizeof(char));
   pNew = new CPropItem(this, szLabel, iType, uResourceID, uFlags | CPIF_HIDDEN);
   pNew->SetItemCallback(lpfn, pVoid);      // set callback and user data
   if(pItemTop==NULL) pItemTop = pNew;      // attach top of chain, if first
   else pItemTop->End()->InsertAfter(pNew); // attach to end of chain
   return(pNew);
}

/*********************************************************
* FindItemByID
*********************************************************/
CPropItem* CPropMgr::FindItemByID(UINT uID) {
   CPropItem *pItem;                        // item loop counter
   for(pItem=pItemTop; pItem; pItem=(CPropItem*) pItem->Next()) if(pItem->ID()==uID) break;
   return(pItem);                           // returns item found, or NULL
}

/*********************************************************
* DeleteItem
*********************************************************/
void CPropMgr::DeleteItem(CPropItem *pItemDel) {
   if(pItemDel==NULL) return;               // ignore no item
   if(pItemTop==pItemDel) pItemTop = (CPropItem*) pItemDel->Next();
   delete(pItemDel);
}


/*########################################################
 ## Access                                             ##
########################################################*/
void CPropMgr::SetAppStatusBarPriorityText(const char *pszError) {
   if((pAppParent==NULL) || (pAppParent->GetStatusBar()==NULL)) return; // ignore if no app or status bar
   pAppParent->GetStatusBar()->SetPriorityText(pszError, 8000); // set the text
}
HINSTANCE CPropMgr::AppInstance(void) { return((pAppParent) ? pAppParent->GetInstance() : NULL); };
COLORREF  CPropMgr::Rgb(int k)        { return((pAppParent) ? pAppParent->Rgb(k)   : RGB(0x00, 0x00, 0x00)); };
HPEN      CPropMgr::Pen(int k)        { return((pAppParent) ? pAppParent->Pen(k)   : (HPEN)GetStockObject(BLACK_PEN)); };
HBRUSH    CPropMgr::Brush(int k)      { return((pAppParent) ? pAppParent->Brush(k) : (HBRUSH)GetStockObject(WHITE_BRUSH)); };
HFONT     CPropMgr::Font(int k)       { return((pAppParent) ? pAppParent->Font(k)  : NULL); };

