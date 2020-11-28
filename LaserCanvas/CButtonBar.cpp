/*********************************************************
*  CButtonBar
* Adapted from rev. 0.1 for LaserCanvas5
*  $PSchlup2004 $     $Revision: 0.1 $
********************************************************/
#include "CButtonBar.h"                       // include header file
#include "resource.h"
/*********************************************************
*  Constructor and initialization
*********************************************************/
CButtonBar::CButtonBar(HWND hwParent) : CBasicBar(hwParent) {
   pcButtonTop       = NULL;                // top of button daisy chain
   pButMouseDown     = NULL;                // button where mouse click down
   pTooltip          = NULL;                // no tooltip yet
   ClearBarStyleBit(CBBMS_MOUSESTATUS);     // remove all mouse status bits

   //---Create the window-----------------------
   CreateBarWindow(TRUE, CButtonBar::ButtonBarWndProc);

   //---Tooltip---------------------------------
   pTooltip = new CTooltip((HINSTANCE) GetWindowLong(hwParent, GWL_HINSTANCE));
}

/*********************************************************
*  Destructor
*********************************************************/
CButtonBar::~CButtonBar() {
   DestroyBarWindow();                      // destroy window, no longer needed

   //---Tooltip---------------------------------
   if(pTooltip) delete(pTooltip); pTooltip = NULL;

   //---Delete buttons and chain----------------
   while(pcButtonTop) {
      pcButtonTop = pcButtonTop->DeleteClassToNext();
   }
   // ~CBasicBar destructor executes after this destructor
}


/*********************************************************
*  AppendButton
*  Create a new button object and add it to the end of
*  the button chain
*********************************************************/
CButton *CButtonBar::AppendButton(LPCSTR lpszButtonName, UINT uButtonData) {
   TChainLink<CButton*> *pcBut;
   CButton *pButNew;
   RECT     rc;

   if(pcButtonTop == NULL) {
      pcBut = pcButtonTop = new TChainLink<CButton*>;
      SetRect(&rc, 0, 0, 0, 0);
   } else {
      pcBut = pcButtonTop->GetEndChain();
      pcBut->Get()->GetButtonRect(&rc);
      pcBut = pcBut->CreateAfter(NULL);
   }
   pButNew = new CButton(GetParentInstance(), lpszButtonName, rc.right, rc.top, uButtonData);
   pcBut->Set(pButNew);
   return(pButNew);
}



/*********************************************************
*  FindButtonByData
*  Find the button object with the corresponding iData
*  Returns the first matching object - ensure iData is
*  unique!
*********************************************************/
CButton* CButtonBar::FindButtonByData(UINT uData) {
   TChainLink<CButton*> *pcBut;

   pcBut = pcButtonTop;
   while(pcBut) {
      if(pcBut->Get()->GetButtonData() == uData) return(pcBut->Get());
      pcBut = pcBut->GetNextChain();
   }
   return(NULL);
}


/*********************************************************
*  OnMouse... Functions
*********************************************************/
//===Mouse Move===========================================
void CButtonBar::OnMouseMove(HWND hWnd, int xPos, int yPos) {
   TChainLink<CButton*> *pcBut;
   HDC        hdc;                          // client DC of window
   BOOL       tfOnBut;                      // track which button has focus
   char      *pszToolTip;                   // tooltip text
   POINT      pt;                           // mouse point
   RECT       rc;                           // window rectangle
   CButton   *pButDnOld;                    // previous button

   pButDnOld = pButMouseDown;               // keep previous button
   if(!CheckBarStyleBit(CBBMS_DRAG)) pButMouseDown = NULL; // clear unless dragging
   hdc = GetDC(hWnd);                       // get client DC..
   SaveDC(hdc);                             //..and save restore point
   pcBut = pcButtonTop;                     // run through chain of buttons
   while(pcBut) {
      tfOnBut = pcBut->Get()->OnMouseMove(hdc, xPos, yPos,
            CheckBarStyleBit(CBBMS_DRAG) ? pButMouseDown : NULL);
      if(!CheckBarStyleBit(CBBMS_DRAG) & tfOnBut) {
         pButMouseDown = pcBut->Get();      // track button with mouse
      }
      pcBut = pcBut->GetNextChain();
   }
   //---Tooltips----------------------
   if((pButMouseDown != NULL) && (pButMouseDown != pButDnOld) && (pTooltip)) {
      pt.x = xPos + 4; pt.y = yPos + 16;    // assemble mouse coordinate point
      ClientToScreen(hWnd, &pt);            // map to screen for tooltip
      pTooltip->SetTipStringFromResource(pButMouseDown->GetButtonData(), GetParentInstance());
      if(pTooltip->IsVisible()) { pTooltip->Position(pt.x, pt.y, FALSE); pTooltip->Show(); } // position, keep visible
      else                        pTooltip->Position(pt.x, pt.y, TRUE); // position, show delayed
   }

   RestoreDC(hdc, -1);
   ReleaseDC(hWnd, hdc);                 // release the DC

   //---Track mouse leaving bar-----------------
   pt.x = xPos; pt.y = yPos;
   GetClientRect(hWnd, &rc);                // retrieve window rectangle
   if(PtInRect(&rc, pt)) {                  // mouse is in window
      if(!CheckBarStyleBit(CBBMS_CAPTURED)) {
         SetCapture(hWnd);                  // capture the mouse..
         SetBarStyleBit(CBBMS_CAPTURED);    //..and set the status bit
      }
   } else {                                 // mouse is outside window
      if(!CheckBarStyleBit(CBBMS_DRAG) && CheckBarStyleBit(CBBMS_CAPTURED)) {
         if(pTooltip) pTooltip->Hide();     // hide tooltip in case it's visible
         ReleaseCapture();                  // release the mouse..
         ClearBarStyleBit(CBBMS_CAPTURED);  //..and clear status bit
      }
   }


}

//===Mouse Down===========================================
void CButtonBar::OnMouseDown(HWND hWnd, int xPos, int yPos) {
   SetBarStyleBit(CBBMS_DRAG);              // set bit indicating button is down
   OnMouseMove(hWnd, xPos, yPos);           // update the graphics
}

//===Mouse Up=============================================
void CButtonBar::OnMouseUp(HWND hWnd, int xPos, int yPos) {
   TChainLink<CButton*> *pcBut;
   POINT pt;
   RECT  rc;

   pt.x = xPos; pt.y = yPos;                // assemble POINT for PtInRect(..)

   //---Find button where mouse released---
   pcBut = pcButtonTop;                     // run through chain of buttons to find..
   while(pcBut) {                           //..if mouse is still on down button
      pcBut->Get()->GetButtonRect(&rc);
      if(PtInRect(&rc, pt)) {
         if(pcBut->Get() != pButMouseDown) pcBut = NULL;
         break;                             // break out of while loop
      }
      pcBut = pcBut->GetNextChain();
   }
   //---Finish and send message---
   if(pTooltip) pTooltip->Hide();           // hide the tooltip (if any)
   ClearBarStyleBit(CBBMS_DRAG);            // release bit that button is moving
   //OnMouseMove(hWnd, xPos, yPos);           // update the graphics
   OnMouseMove(hWnd, 32767, 32767);         // move, pretend mouse is out of range

   if(pcBut && pcBut->Get()->IsEnabled()) {
      PostMessage(GetParentWindow(), WM_COMMAND, pcBut->Get()->GetButtonData(), 0L);
   }
}

/*********************************************************
*  PaintButtonBar
*********************************************************/
void CButtonBar::PaintButtonBar(HDC hdc, LPRECT lprcClient) {
   TChainLink<CButton*> *pcBut;             // loop over buttons
   RECT   rc;                               // button rectangle
   HPEN   hpBord1, hpBord2, hpOld;          // pens to draw border; store previous
   int    x[4], y[4];                       // coordinates for lines

   //---Paint the buttons-----------------------
   pcBut = pcButtonTop;                     // start painting first button
   SetRect(&rc, lprcClient->left, lprcClient->top, lprcClient->left, lprcClient->bottom); // no buttons painted yet
   while(pcBut) {
      pcBut->Get()->PaintButton(hdc);
      pcBut->Get()->GetButtonRect(&rc);     // keep last rectangle
      pcBut = pcBut->GetNextChain();
   }

   //---Fill to end of window-------------------
   if(rc.right < (lprcClient->right - lprcClient->left)) {
      SetRect(&rc, rc.right, rc.top, lprcClient->right, rc.bottom);
      FillRect(hdc, &rc, GetSysColorBrush(COLOR_3DFACE));
   }

   //---Borders---------------------------------
   ///TODO: (maybe) check other bar placement borders!!
   rc.bottom += 2;
   if(CheckBarStyleBit(CBBS_BORDER)) {
      switch(CheckBarStyleBit(CBBS_POSITION)) {
      case CBBS_TOP:
         x[0] = 0;           x[1] = lprcClient->right; x[2] = 0;     x[3] = lprcClient->right;
         y[0] = rc.bottom-2; y[1] = rc.bottom-2; y[2] = rc.bottom-1; y[3] = rc.bottom-1;
         break;
      case CBBS_BOTTOM:
         x[0] = 0;         x[1] = lprcClient->right; x[2] = 0;   x[3] = lprcClient->right;
         y[0] = rc.top-1;  y[1] = rc.top-1;  y[2] = rc.top-2;    y[3] = rc.top-2;
         break;
      case CBBS_LEFT:
         x[0] = rc.left-1; x[1] = rc.left-1; x[2] = rc.left-2;   x[3] = rc.left-2;
         y[0] = 0;         y[1] = lprcClient->bottom; y[2] = 0;  y[3] = lprcClient->bottom;
         break;
      case CBBS_RIGHT:
         x[0] = rc.right;  x[1] = rc.right;  x[2] = rc.right+1;  x[3] = rc.right+1;
         y[0] = 0;         y[1] = lprcClient->bottom; y[2] = 0;  y[3] = lprcClient->bottom;
         break;
      }
      switch(CheckBarStyleBit(CBBS_BORDER)) {
      case CBBS_BORDERFLAT:
         hpBord1 = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DFACE));
         hpBord2 = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DFACE));
         break;
      case CBBS_BORDERRAISED:
         hpBord1 = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
         hpBord2 = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_WINDOWFRAME));
         break;
      }

      hpOld = (HPEN) SelectObject(hdc, hpBord1);
      MoveToEx(hdc, x[0], y[0], NULL);
      LineTo(hdc, x[1], y[1]);

      SelectObject(hdc, hpBord2);
      MoveToEx(hdc, x[2], y[2], NULL);
      LineTo(hdc, x[3], y[3]);

      SelectObject(hdc, hpOld);
      DeleteObject(hpBord2);
      DeleteObject(hpBord1);
   }
}


/*********************************************************
* WndProcButtonBar
* Windows callback function  for button bar. The bar win-
* dow is a child window, so use  DefWindowProc by default
* This callback function must be declared STATIC
*********************************************************/
LRESULT CALLBACK CButtonBar::ButtonBarWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
   CButtonBar *pBB;                         // object of interest
   PAINTSTRUCT ps;                          // for WM_PAINT
   RECT  rcClient;                          // button bar client area
   POINT ptMouse;                           // mouse coordinate

   pBB = (CButtonBar*) GetWindowLong(hWnd, 0); // object belonging to window

   switch(uMsg) {

   case WM_PAINT:
      GetClientRect(hWnd, &rcClient);
      BeginPaint(hWnd, &ps);
      SaveDC(ps.hdc);

      pBB->PaintButtonBar(ps.hdc, &rcClient);

      RestoreDC(ps.hdc, -1);
      EndPaint(hWnd, &ps);
      break;

   case WM_MOUSEMOVE:
   case WM_NCMOUSEMOVE:
      pBB->OnMouseMove(hWnd, LOWORD(lParam), HIWORD(lParam));
      break;

   case WM_LBUTTONDOWN:
      pBB->OnMouseDown(hWnd, LOWORD(lParam), HIWORD(lParam));
      break;
   case WM_LBUTTONUP:
      pBB->OnMouseUp(hWnd, LOWORD(lParam), HIWORD(lParam));
      break;

   case WM_KEYDOWN:
   case WM_KEYUP:
   case WM_CHAR:
   case WM_DEADCHAR:
   case WM_SYSKEYDOWN:
   case WM_SYSKEYUP:
   case WM_SYSCHAR:
   case WM_SYSDEADCHAR:
   case WM_COMMAND:
   case WM_SYSCOMMAND:
      SendMessage(pBB->GetParentWindow(), uMsg, wParam, lParam);
      break;

   default:
      return( DefWindowProc(hWnd, uMsg, wParam, lParam) );
   }
   return(0L);
}


/*********************************************************
* OnResizeParent
* (Rev.5) Called  when the parent window is  resized. The
* button bar is placed at the TOP of the client rectangle
* and lprc is reduced by the space taken up by the bar.
*********************************************************/
void CButtonBar::OnParentResize(LPRECT lprc) {
   if(GetBarWindow()==NULL) return;         // ignore if no bar window
   MoveBarWindow(lprc->left, lprc->top, lprc->right-lprc->left, CBB_BARHEIGHT);
   SetRect(lprc, lprc->left, lprc->top+CBB_BARHEIGHT, lprc->right, lprc->bottom);
}


