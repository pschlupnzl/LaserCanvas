/*********************************************************
* CQuickEdit
* $PSchlup 2006 $     $Revision 3 $
*********************************************************/
#include "CQuickEdit.h"

#define CSZ_QEDIT_CLASSNAME "CQuickEdit_CLASS"

/*********************************************************
* Constructor
*********************************************************/
CQuickEdit::CQuickEdit(HWND _hwParent) {
   WNDCLASS wc;

   //---Members---------------------------------
   hwParent     = _hwParent;
   uFlags       = 0x0000;
   pvUser       = NULL;
   lUser        = NULL;
   lpfnCallback = NULL;

   //---Create windows--------------------------
   wc.hInstance     = (HINSTANCE) GetWindowLong(hwParent, GWL_HINSTANCE);
   wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
   wc.hIcon         = (HICON) NULL;
   wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
   wc.cbClsExtra    = 0;
   wc.cbWndExtra    = 0;
   wc.lpszClassName = CSZ_QEDIT_CLASSNAME;
   wc.lpszMenuName  = NULL;
   wc.style         = CS_HREDRAW | CS_VREDRAW;
   wc.lpfnWndProc   = CQuickEdit::_WndProcQEdit;
   RegisterClass(&wc);

   hwBack = CreateWindow(CSZ_QEDIT_CLASSNAME, NULL,
      WS_CHILD,
      0, 0, 10, 10,
      hwParent, (HMENU) NULL, (HINSTANCE) GetWindowLong(hwParent, GWL_HINSTANCE), (LPVOID) this);

   hwEdit = CreateWindow("EDIT", NULL,
      WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
      0, 0, 100, 24,
      hwBack, (HMENU) NULL, (HINSTANCE) GetWindowLong(hwParent, GWL_HINSTANCE), (LPVOID) NULL);
   lpfnQEditCtl = (WNDPROC) GetWindowLong(hwEdit, GWL_WNDPROC);
   SetWindowLong(hwEdit, GWL_USERDATA, (LONG) this);
   SetWindowLong(hwEdit, GWL_WNDPROC, (LONG)CQuickEdit::_WndProcQEditCtl);

}

/*********************************************************
* Destructor
*********************************************************/
CQuickEdit::~CQuickEdit() {
   //---Windows---------------------------------
   if(hwEdit) DestroyWindow(hwEdit); hwEdit = NULL;
   if(hwBack) DestroyWindow(hwBack); hwBack = NULL;
}


/*********************************************************
* SetPosition
* The background window hwBack should only occupy as much
* space as is needed for the  collapsed version. The menu
* window should probably have enough  space for all items
* to be visible.
*********************************************************/
//===SetPosition==========================================
void CQuickEdit::SetPosition(int x, int y, int iWid, int iHig) {
   if(hwEdit == NULL) return;               // ignore if no menu
   MoveWindow(hwBack, x, y, iWid, iHig, TRUE);
   MoveWindow(hwEdit, 0, 0, iWid, iHig, TRUE);
}
//===SetPositionRect======================================
void CQuickEdit::SetPositionRect(LPRECT prc) {
   SetPosition(prc->left, prc->top, prc->right-prc->left, prc->bottom-prc->top);
}
//===SetPositionRectAuto==================================
void CQuickEdit::SetPositionRectAuto(LPRECT prc) {
   RECT rcFmt;                              // formatting rectangle

   if(hwEdit == NULL) return;               // ignore if no menu
   SendMessage(hwEdit, EM_GETRECT, (WPARAM)0, (LPARAM)(LPRECT)&rcFmt);
   SetPosition(prc->left, prc->top, prc->right-prc->left, prc->top + rcFmt.bottom-rcFmt.top);
}

/*********************************************************
* Show
*********************************************************/
void CQuickEdit::Show(void) {//HWND hwDown, WPARAM wParam, LPARAM lParam) {
   POINT pt;                                // mouse coordinates
   RECT  rc;                                // window rectangles
   if(hwEdit == NULL) return;               // ignore if no menu

   ////---Click in source window------------------
   //if(hwDown != NULL) {                     // complete the mouse click
   //   SendMessage(hwDown, WM_LBUTTONUP, wParam, lParam);
   //}

   //---Show the control------------------------
   SetFocus(hwEdit);                        // select the edit control
   SetBit(CQEDITF_VISIBLE);                 // track visibility
   ShowWindow(hwBack, SW_SHOW);             // show the control
   ShowWindow(hwEdit, SW_SHOW);             // show the control
   SetCapture(hwEdit);                      // capture all mouse to the control
   SendMessage(hwEdit, EM_SETSEL, (WPARAM) 0, (LPARAM) -1); // select all text

   ////---Click in control------------------------
   //if(hwDown != NULL) {
   //   pt.x = LOWORD(lParam); pt.y = HIWORD(lParam);
   //   GetWindowRect(hwDown, &rc);
   //   pt.x += rc.left; pt.y += rc.top;
   //   GetWindowRect(hwEdit, &rc);
   //   pt.x -= rc.left; pt.y -= rc.top;
   //   SendMessage(hwEdit, WM_LBUTTONDOWN, wParam, MAKELPARAM(pt.x, pt.y));
   //} else {
   //   SendMessage(hwEdit, EM_SETSEL, (WPARAM) 0, (LPARAM) -1); // select all text
   //}


}


/*********************************************************
* Hide
* With the extra argument to specify whether to apply the
* new value or not.
*********************************************************/
void CQuickEdit::Hide(void) {
   if(hwEdit == NULL) return;               // ignore if no menu
   ClearBit(CQEDITF_VISIBLE);               // track visibility (first due to KILLFOCUS handling)
   ShowWindow(hwEdit, SW_HIDE);             // hide control
   ShowWindow(hwBack, SW_HIDE);             // hide control
   ReleaseCapture();                        // release the mouse
}

/*********************************************************
* Process
*********************************************************/
void CQuickEdit::Process(int iNext) {
   int   iLen;                              // length of string
   char *pszText;                           // buffer
   double dVal;                             // returned integer value
   int    iVal;                             // return int value
   BOOL   tfHide;                           // flag to hide or not

   //---No callback-------------------
   if(lpfnCallback == NULL) { Hide(); return; }; // no callback: ok, hide it

   //---Copy text---------------------
   iLen = SendMessage(hwEdit, WM_GETTEXTLENGTH, 0, 0L); // read text length
   pszText = (char*) malloc((iLen+1) * sizeof(char));   // allocate the buffer
   if(pszText) SendMessage(hwEdit, WM_GETTEXT, (WPARAM) iLen+1, (LPARAM) pszText); // copy text to buffer

   //---Check internals---------------
   tfHide = FALSE;                          // assume it's not ok to hide it
   switch(DataType()) {
   case CQEDITF_TYPEDBL:
      if(sscanf(pszText, "%lg", &dVal) < 1) break;
      tfHide = lpfnCallback((void*) &dVal, iNext, pvUser, lUser);
      break;
   case CQEDITF_TYPEINT:
      if(sscanf(pszText, "%d", &iVal) < 1) break;
      tfHide = lpfnCallback((void*) &iVal, iNext, pvUser, lUser);
      break;
   case CQEDITF_TYPESTR:
      tfHide = lpfnCallback((void*) pszText, iNext, pvUser, lUser );
      break;
   }

   //---Finalize----------------------
   if(tfHide) Hide();                       // hide the control
   else       SendMessage(hwEdit, EM_SETSEL, (WPARAM) 0, (LPARAM) -1); // select all text

   if(pszText) free(pszText); pszText = NULL; // free allocated buffer
}

/*********************************************************
* SetUserData
*********************************************************/
void CQuickEdit::SetUserData(void *pvUserIn, long int lUserIn) {
   pvUser = pvUserIn;
   lUser  = lUserIn;
}


/*********************************************************
* SetCallback
* The callback function has the declaration
* BOOL Callback(const char *pszText, int iNext, void *pVoid, long int lLong)
* The function should return TRUE if the control can be
* hidden.
*********************************************************/
void CQuickEdit::SetCallback(LPQECALLBACK lpfnCallbackIn) {
   lpfnCallback = lpfnCallbackIn;
}

/*********************************************************
* SetString
*********************************************************/
//===SetString============================================
void CQuickEdit::SetString(const char *pszText) {
   SetDataType(CQEDITF_TYPESTR);
   if(hwEdit) SendMessage(hwEdit, WM_SETTEXT, (WPARAM) 0, (LPARAM) pszText);
}
//===SetDouble============================================
void CQuickEdit::SetDouble(double dVal) {
   char szBuf[256];
   sprintf(szBuf, "%lg", dVal);
   SetDataType(CQEDITF_TYPEDBL);
   if(hwEdit) SendMessage(hwEdit, WM_SETTEXT, (WPARAM) 0, (LPARAM) szBuf);
}
//===SetInt===============================================
void CQuickEdit::SetInt(int iVal) {
   char szBuf[256];
   sprintf(szBuf, "%d", iVal);
   SetDataType(CQEDITF_TYPEINT);
   if(hwEdit) SendMessage(hwEdit, WM_SETTEXT, (WPARAM) 0, (LPARAM) szBuf);
}

/*********************************************************
* SetFont
* Ensure the control is deleted before the font is!
*********************************************************/
void CQuickEdit::SetFont(HFONT hFont) {
   if(hwEdit) SendMessage(hwEdit, WM_SETFONT, (WPARAM) hFont, MAKELPARAM(TRUE, 0));
}




/*********************************************************
* WndProc
* This callback function must be declared static
*********************************************************/
LRESULT CALLBACK CQuickEdit::_WndProcQEdit(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
   switch(uMsg) {
   case WM_CREATE:
      SetWindowLong(hWnd, GWL_USERDATA, (LONG)(CQuickEdit*)(((LPCREATESTRUCT) lParam)->lpCreateParams));
      break;
   default:
      return( ((CQuickEdit*) GetWindowLong(hWnd, GWL_USERDATA))->WndProcQEdit(hWnd, uMsg, wParam, lParam) );
   }
   return(0L);
}
//******************************************************//
LRESULT CALLBACK CQuickEdit::WndProcQEdit(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
   switch(uMsg) {
   case WM_COMMAND:
      switch(HIWORD(wParam)) {
      case EN_KILLFOCUS: if(IsVisible()) Hide(); break;
      }
      break;
   }
   return(DefWindowProc(hWnd, uMsg, wParam, lParam));
}





/*********************************************************
* WndProcQEditCtl
* This is  the subclassed control window  procedure. Con-
* trol branches that process messages return 0 from with-
* in the switchyard so that all  non-interfering messages
* fall through to the subclassed call at the end.
* The referenced function must be declared static
*********************************************************/
LRESULT CALLBACK CQuickEdit::_WndProcQEditCtl(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
   return(((CQuickEdit*) GetWindowLong(hWnd, GWL_USERDATA))->WndProcQEditCtl(hWnd, uMsg, wParam, lParam));
}
//******************************************************//
LRESULT CALLBACK CQuickEdit::WndProcQEditCtl(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
   LRESULT lRet;                            // return value
   POINT pt;                                // mouse coordinate
   RECT  rc;                                // window rectangles
   HWND  hwNew;                             // window to receive new focus

   switch(uMsg) {
   case WM_LBUTTONDOWN:
      pt.x = LOWORD(lParam); pt.y = HIWORD(lParam);
      GetClientRect(hWnd, &rc);
      if(!PtInRect(&rc, pt)) {              // click down outside window
         Process(0);                        // call callback, if necessary
         if(!IsVisible()) {                 // if hidden ok
            SendMessage(hWnd, WM_LBUTTONUP, wParam, lParam); // complete this control's click
            GetWindowRect(hWnd, &rc);       // offset mouse..
            pt.x += rc.left; pt.y += rc.top;
            hwNew = WindowFromPoint(pt);
            GetWindowRect(hwNew, &rc);      //..to new window
            pt.x -= rc.left; pt.y -= rc.top;
            SendMessage(hwNew, WM_LBUTTONDOWN, wParam, MAKELPARAM(pt.x, pt.y)); // send new window's down
            SetFocus(hwNew);                // focus on it
         } else {
            lRet = lpfnQEditCtl(hWnd, uMsg, wParam, lParam);
            SetCapture(hWnd);                  //..keep the mouse capture, damnit
            SendMessage(hWnd, EM_SETSEL, (WPARAM) 0, (LPARAM) -1);
            return(lRet);
         }
      }
      break;
   case WM_LBUTTONUP:
      pt.x = LOWORD(lParam); pt.y = HIWORD(lParam);
      GetClientRect(hWnd, &rc);
      if(PtInRect(&rc, pt)) {               // if clicked within edit control..
         lRet = lpfnQEditCtl(hWnd, uMsg, wParam, lParam);
         SetCapture(hWnd);                  //..keep the mouse capture, damnit
         return(lRet);
      }
      break;

   case WM_CHAR:
      switch(wParam) {
      case VK_RETURN: Process(0); return(0L);
      case VK_ESCAPE: Hide();     return(0L);
      case VK_TAB:    Process((GetKeyState(VK_SHIFT)&0x8000) ? -1 : 1); return(0L);
      case VK_SPACE:  if(CheckBit(CQEDITF_NOSPACE)) return(0L);
      }
      break;
   }
   return(lpfnQEditCtl(hWnd, uMsg, wParam, lParam));
}
