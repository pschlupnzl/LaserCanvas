/*****************************************************************************
*  CBasicBar.cpp
*  Underlying class for status bar, button bar, etc
*****************************************************************************/
#include "CBasicBar.h"                      // include header

/*********************************************************
*
*  CBasicBar
*
*********************************************************/

/*********************************************************
*  Constructor
*********************************************************/
CBasicBar::CBasicBar(HWND hwParent) {
   WNDCLASS wc;                             // register window class

   hwParentWindow = hwParent;               // parent window
   hwBarWindow    = NULL;                   // window of this bar
   SetRect(&rcBarPosition, 0, 0, 32, 32);   // default position (left, top, wid, hig)
   uBarStyle      = 0;                      // bar style (for use by derived bars)

   //---Register class (if necessary)-----------
   if( !GetClassInfo(GetParentInstance(), CBAR_CLASSNAME, &wc) ) {
      wc.style         = 0;
      wc.lpfnWndProc   = CBasicBar::BarWndProc;
      wc.cbClsExtra    = 0;
      wc.cbWndExtra    = sizeof(LPVOID);
      wc.hInstance     = GetParentInstance();
      wc.hIcon         = NULL;
      wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
      wc.hbrBackground = (HBRUSH) NULL;
      wc.lpszMenuName  = NULL;
      wc.lpszClassName = CBAR_CLASSNAME;
      RegisterClass(&wc);
   }
}

/*********************************************************
*  Destructor
*********************************************************/
CBasicBar::~CBasicBar() {
   DestroyBarWindow();                      // ensure bar window is destroyed
}


/*********************************************************
*  Window functions
* The creation and subclassing is all a bit clever:
* - Since CStatusBar  is derived from CBasicBar,  an ins-
*   tance of a CStatusBar  object has the SAME address as
*   the CBasicBar 'object' referred to by THIS in CBasic-
*   Bar member functions.
* - Thus, CBasicBar handles the  window creation; the ob-
*   ject pointer used is  THIS since it's the same as the
*   derived object pointer
* - The  CBAR_CLASSNAME window  class is  then subclassed
*   with the specified callback function, lpfnProc
* - This specified callback  function, lpfnProc, must ex-
*   pect a pointer to  an object of the  appropriate type
*   to be stored in GetWindowLong(0). The pointer is sto-
*   red by the CBasicBar WM_CREATE handler using THIS
* - If lpfnProc  is NULL, the  window is  not subclassed,
*   since  this would  lead to an  access violation  when
*   Windows attempts to call  a function at address NULL.
*   But passing  a NULL  lpfnProc makes  no sense,  since
*   CBasicBar provides no further functionality and there
*   is no background brush.
* - The subclassing can  only be done once the  window is
*   created. Thus, the initial  WM_CREATE message is han-
*   dled by the CBasicBar::BarWndProc
* - An  'artificial'  WM_CREATE  messages is  sent to the
*   subclassed  window procedure. Other  windows creation
*   messages (in order WM_GETMINMAXINFO, WM_NCCREATE, WM_
*   NCCALCSIZE, WM_CREATE) are NOT passed.
* - Usually when subclassing, messages not handled by the
*   subclassed window procedure  are passed to the origi-
*   nal window  procedure using  CallWindowProc.  In this
*   case, this could lead to an endless loop.
* - Thus: Do NOT use  CallWindowProc, simply call DefWin-
*   dowProc as usual!
**********************************************************/
//===Create===============================================
HWND CBasicBar::CreateBarWindow(BOOL tfShow, WNDPROC lpfnProc) {
   if(hwBarWindow == NULL) {
      hwBarWindow = CreateWindow(CBAR_CLASSNAME, (LPCSTR) NULL,
         WS_CHILD | WS_CLIPSIBLINGS,
         rcBarPosition.left, rcBarPosition.top, rcBarPosition.right, rcBarPosition.bottom, // actually width and height
         GetParentWindow(), (HMENU) NULL, GetParentInstance(), (LPVOID) this);
      if(lpfnProc != NULL) {
         SetWindowLong(hwBarWindow, GWL_WNDPROC, (LONG) lpfnProc); // subclass
      }
   }
   SendMessage(hwBarWindow, WM_CREATE, 0, (LONG)(LPVOID) this);
   ShowBarWindow(tfShow);
   return(hwBarWindow);
}
//===Destroy==============================================
void CBasicBar::DestroyBarWindow(void) {
   if(!hwBarWindow) return;                 // skip if no window exists
   DestroyWindow(hwBarWindow);              // destroy window..
   hwBarWindow = NULL;                      //..and remove handle
}
//===Position=============================================
void CBasicBar::MoveBarWindow(int l, int t, int w, int h) {
   SetRect(&rcBarPosition, l, t, w, h);     // track desired position even without window
   if(hwBarWindow) MoveWindow(hwBarWindow, l, t, w, h, TRUE);
}
//===Show / hide==========================================
void CBasicBar::ShowBarWindow(BOOL tfShow) {
   if(!hwBarWindow) return;                 // skip if no window exists
   ShowWindow(hwBarWindow, tfShow ? SW_SHOW : SW_HIDE);
}

//===Update===============================================
void CBasicBar::InvalidateBarRect(CONST RECT *rcInval) {
   if(!hwBarWindow) return;                 // skip if no window exists
   InvalidateRect(hwBarWindow, rcInval, FALSE); // force re-paint
}

/*********************************************************
*  Set / Get
*********************************************************/
//===Application instance=================================
HINSTANCE CBasicBar::GetParentInstance(void) {
   if(GetParentWindow() == NULL) return(NULL);
   return( (HINSTANCE) GetWindowLong(GetParentWindow(), GWL_HINSTANCE) );
}

//===Current position=====================================
void CBasicBar::SetBarPosition(int l, int t, int w, int h) {
   SetRect(&rcBarPosition, l, t, w, h);
}
// copies rectangle in any case; returns FALSE if window doesn't exist
BOOL CBasicBar::GetBarPosition(LPRECT lprc) {
   CopyRect(lprc, &rcBarPosition);
   return( GetBarWindow()==NULL ? FALSE : TRUE );
}

/*********************************************************
*  BarWndProc - Window callback procedure
*  This callback function must be declared STATIC
**********************************************************/
LRESULT CALLBACK CBasicBar::BarWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
   switch(uMsg) {
   case WM_CREATE:
      SetWindowLong(hWnd, 0, (LONG) (((LPCREATESTRUCT)lParam)->lpCreateParams) );
      break;
   default:
      return( DefWindowProc(hWnd, uMsg, wParam, lParam) );
   }
   return(0L);
}


