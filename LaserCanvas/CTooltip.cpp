/*********************************************************
* CTooltip
* $PSchlup 2005-2006 $     $Revision 2 $
* Revision History
*   2  2006oct     Adapted for LaserCanvas
*                  Added SetStringFromResource
*   1  2005        Initial development for Juice
*********************************************************/
#include "CToolTip.h"                       // header file

/*********************************************************
* Constructor
*********************************************************/
CTooltip::CTooltip(HINSTANCE hInst) {
   WNDCLASS wc;                             // window class to register
   hWndTip = NULL;
   szTipString = NULL;                      // null pointer
   tfVisible   = FALSE;                     // not shown
   uVisTimer  = NULL;                      // hide timer not used

   hfTipFont = CreateFont(
      -12, 0,0,0, FW_DONTCARE, 0,0,0, ANSI_CHARSET,
      OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
      FF_SWISS|VARIABLE_PITCH, "MS Sans Serif");

   //---Register Window Class-------------------
   wc.style         = CS_HREDRAW | CS_VREDRAW;
   wc.lpfnWndProc   = CTooltip::TipWndProc;
   wc.cbClsExtra    = 0;
   wc.cbWndExtra    = sizeof(CTooltip*);    // store object pointer on window
   wc.hInstance     = hInst;                // instance passed as argument
   wc.hIcon         = NULL;
   wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
   wc.hbrBackground = (HBRUSH) (COLOR_INFOBK + 1);
   wc.lpszClassName = SZ_WCTOOLTIP;
   wc.lpszMenuName  = NULL;
   RegisterClass(&wc);                      // fails on multiple registration of same name

   //---Create the window-----------------------
   hWndTip = CreateWindowEx(
         WS_EX_TOOLWINDOW,
         SZ_WCTOOLTIP, NULL,
         WS_POPUP | WS_BORDER | WS_DISABLED,
         0, 0, 100, 12,
         NULL, (HMENU) NULL,
         hInst,
         (LPVOID) this);

   SetWindowPos(hWndTip, HWND_TOPMOST, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE);

}


/*********************************************************
* Destructor
*********************************************************/
CTooltip::~CTooltip() {
   if(tfVisible) Hide();                    // hide if visible
   if(szTipString) free(szTipString);       // delete allocated memory
   DestroyWindow(hWndTip);                  // destroy the window
   if(hfTipFont) DeleteObject(hfTipFont); hfTipFont = NULL;
}


/*********************************************************
* SetTipStringFromResource
* Set the tooltip string from the stringtable resource
*********************************************************/
BOOL CTooltip::SetTipStringFromResource(UINT uID, HINSTANCE hInst) {
   char szBuf[256];
   if( LoadString(hInst, uID, szBuf, sizeof(szBuf)/sizeof(char)) <= 0 ) return(FALSE);
   return( SetTipString(szBuf) );
}


/*********************************************************
* SetTipString
* Set the tooltip string
*********************************************************/
BOOL CTooltip::SetTipString(const char *cszString) {
   SIZE  siz;                               // extent of text
   HDC   hdc;                               // window DC for text size
   HFONT hfOld;                             // restore DC font

   hdc = GetDC(hWndTip);                    // get window DC
   hfOld = (HFONT) SelectObject(hdc, hfTipFont);

   if(szTipString) free(szTipString); szTipString = NULL; // free existing buffer
   if(cszString != NULL) {
      szTipString = (char*) malloc(sizeof(char) * (strlen(cszString)+1)); // allocate
   }

   if((cszString==NULL) || (szTipString==NULL)) {
      GetTextExtentPoint32(hdc, " ", 1, &siz); // get size of vacant string
   } else {
      memcpy(szTipString, cszString, sizeof(char) * (strlen(cszString)+1)); // copy
      GetTextExtentPoint32(hdc, szTipString, strlen(szTipString), &siz); // get size
   }

   SelectObject(hdc, hfOld);                // restore DC font
   ReleaseDC(hWndTip, hdc);                 // release window DC

   SetWindowPos(hWndTip, NULL, 0, 0, siz.cx+8, siz.cy+4,
         SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);

   return( szTipString != NULL );           // return FALSE if alloc failed
}

/*********************************************************
* Window manipulation
*********************************************************/
//===MoveTo===============================================
void CTooltip::Position(int x, int y, BOOL tfShowDelayed) {
   SetWindowPos(hWndTip, NULL, x, y, 0, 0,
         SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);

   if(tfShowDelayed) {
      if(tfVisible) Hide();                 // hide if currently visible (clears timer)
      uVisTimer = SetTimer(hWndTip, CTTIP_VISTIMER, 1000, NULL); // set show timer
   }
}

//===Show=================================================
void CTooltip::Show(void) {
   if(uVisTimer != NULL) KillTimer(hWndTip, uVisTimer); // kill existing timer

   //AnimateWindow(hWndTip, 200, AW_BLEND);   // blend in (just 'cos it's cool -- no text visible?!)
   ShowWindow(hWndTip, SW_SHOWNOACTIVATE);
   tfVisible = TRUE;                        // window is visible

   uVisTimer = SetTimer(hWndTip, CTTIP_VISTIMER, 5000, NULL); // set hide timer
}

//===Hide=================================================
void CTooltip::Hide(void) {
   if(uVisTimer != NULL) {KillTimer(hWndTip, uVisTimer); }
   uVisTimer = NULL;                        // no remaining timers

   //AnimateWindow(hWndTip, 1000, AW_BLEND | AW_HIDE);
   ShowWindow(hWndTip, SW_HIDE);
   tfVisible = FALSE;                       // window is hidden
}

/**********************************************************
* TipWndProc
* This callback function must be declared STATIC
**********************************************************/
LRESULT CALLBACK CTooltip::TipWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
   CTooltip *pTip;                          // object called
   PAINTSTRUCT ps;                          // for painting
   HFONT hfOld;                             // restore DC font
   COLORREF rgbBk, rgbFg;                   // previous color values

   pTip = (CTooltip*) GetWindowLong(hWnd, 0); // not valid on WM_CREATE

   switch(uMsg) {
      case WM_CREATE:
         pTip = (CTooltip*) ((LPCREATESTRUCT) lParam)->lpCreateParams;
         SetWindowLong(hWnd, 0, (LONG)(void*)pTip);
         break;

      case WM_PAINT:
         BeginPaint(hWnd, &ps);
         if(pTip->szTipString != NULL) {
            hfOld = (HFONT) SelectObject(ps.hdc, pTip->hfTipFont);
            rgbBk = SetBkColor  (ps.hdc, GetSysColor(COLOR_INFOBK));
            rgbFg = SetTextColor(ps.hdc, GetSysColor(COLOR_INFOTEXT));
            TextOut(ps.hdc, 2,1, pTip->szTipString, strlen(pTip->szTipString));
            SetTextColor(ps.hdc, rgbFg);    // restore DC colors etc
            SetBkColor(ps.hdc, rgbBk);
            SelectObject(ps.hdc, hfOld);
         }
         EndPaint(hWnd, &ps);
         break;

      case WM_TIMER:
         if(pTip->tfVisible) {
            pTip->Hide();
         } else {
            pTip->Show();
         }
         break;

      default:
         return( DefWindowProc(hWnd, uMsg, wParam, lParam) );
   }
   return(0L);
}

