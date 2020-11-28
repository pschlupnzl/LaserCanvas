/*********************************************************
* CStatusBar.cpp
* Simple status bar class for status texts
* $PSchlup 2004 $     $Revision 6 $
* Revision History
*   6   2006aug19  Added regions
*********************************************************/

#include "CStatusBar.h"                     // include header file

#define MoveTo(h,x,y) MoveToEx(h, x, y, NULL)

/*********************************************************
*
*  CStatusBar
*
*********************************************************/
/*********************************************************
*  Constructor
*********************************************************/
CStatusBar::CStatusBar(HWND hwParent, int iNRgn) : CBasicBar(hwParent) {
   int iRgn;                                // loop counter

   //---Members---------------------------------
   hfStatusBarFont = NULL;                  // no font loaded yet
   for(iRgn=0; iRgn<=CSB_REGIONMAX; iRgn++) pszStandardText[iRgn] = NULL; // no text created yet
   pszPriorityText = NULL;                  // no priority text
   uPriorityTimer  = 0;                     // no priority timer
   iNumRegion      = iNRgn;                 // number of regions
   if(iNumRegion < 1) iNumRegion = 1;       // at least one region
   if(iNumRegion > CSB_REGIONMAX+1) iNumRegion = CSB_REGIONMAX+1;
   for(iRgn=0; iRgn<iNumRegion; iRgn++) dRegionWidth[iRgn] = 1.00 / (double) iNumRegion; // default width: fractional

   //---Load font-------------------------------
   hfStatusBarFont = CreateFont(CSB_FONTHEIGHT, 0,0,0, FW_DONTCARE,
         0,0,0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
         DEFAULT_QUALITY, FF_SWISS|VARIABLE_PITCH, CSB_FONTNAME);

   //---Load bitmaps----------------------------
   hbmStatusBarBmp[CSB_BMPSTOP] = (HBITMAP) LoadImage(GetParentInstance(),
         "BMP_STAT_INFO12", IMAGE_BITMAP, 12, 12, LR_LOADMAP3DCOLORS);

   //---Create pens-----------------------------
   hpBord[CSB_PENLITE ] = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DHILIGHT));
   hpBord[CSB_PENDARK ] = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
   hpBord[CSB_PENFRAME] = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_WINDOWFRAME));

   //---Create bar------------------------------
   // Because, let's face it, a status bar without a win-
   // dow isn't very useful
   CreateBarWindow(TRUE, StatusBarWndProc);

}

/*********************************************************
*  Destructor
*********************************************************/
CStatusBar::~CStatusBar() {
   int k;                                   // loop counter
   //---Ensure window destroyed-----------------
   DestroyBarWindow();

   //---Destroy resources-----------------------
   for(k=CSB_BMPMAX; k>=0; k--) { DeleteObject(hbmStatusBarBmp[k]); hbmStatusBarBmp[k] = NULL; };
   for(k=CSB_PENMAX; k>=0; k--) { DeleteObject(hpBord[k]);          hpBord[k]          = NULL; };
   if(pszPriorityText) { free(pszPriorityText); pszPriorityText = NULL; };
   for(k=CSB_REGIONMAX; k>=0; k--) if(pszStandardText[k]) { free(pszStandardText[k]); pszStandardText[k] = NULL; }
   if(hfStatusBarFont) { DeleteObject(hfStatusBarFont); hfStatusBarFont = NULL; }
}


/*********************************************************
*  ResourcePriorityText
*  Set the priority text from the stringtable in the resource,
*  for button bar and menu item tooltips
*********************************************************/
void CStatusBar::ResourcePriorityText(UINT uCmd) {
   char szBuffer[256];                      // buffer for string resource

   if( LoadString(GetParentInstance(), uCmd, szBuffer, sizeof(szBuffer)) >0) {
      SetPriorityText(szBuffer, CSB_RCTEXTTIMEOUT);
   } else {
      SetPriorityText(NULL, 0);
   }
}

/*********************************************************
*  Set and Get interfaces
*********************************************************/
//===Standard text========================================
BOOL CStatusBar::SetStandardText(LPCSTR lpszText, int iRgn) {
   if((iRgn<0) || (iRgn>=iNumRegion)) return(FALSE);       // ignore out of bounds
   if(pszStandardText[iRgn]) free(pszStandardText[iRgn]); pszStandardText[iRgn] = NULL; // free existing text buffer
   if(lpszText == NULL) return(TRUE);                      // clear text buffer only
   pszStandardText[iRgn] = (char*) malloc((strlen(lpszText)+1) * sizeof(char)); // allocate
   if(pszStandardText[iRgn] == NULL) return(FALSE);        // failed to allocate - return with error
   strcpy(pszStandardText[iRgn], lpszText); // copy text into buffer
   PaintStatusBar(NULL, iRgn);              // update this region only
   return(TRUE);
}

//===Priority text========================================
BOOL CStatusBar::SetPriorityText(LPCSTR lpszText, UINT uTimeOut) {
   //---Free resources----------------
   if(uPriorityTimer) {                     // kill timer, if it exists
      KillTimer(GetBarWindow(), CSB_IDPRIORITYTIMER);
      uPriorityTimer = 0;
   }
   if(pszPriorityText) { free(pszPriorityText); pszPriorityText = NULL; }; // free existing text buffer
   if((lpszText==NULL) || GetBarWindow()==NULL) {
      InvalidateBarRect(NULL);              // nonetheless, update the window
      return(TRUE);                         // return if no text to set or no window to take timer
   }

   //---Text--------------------------
   pszPriorityText = (char*) malloc((strlen(lpszText)+1) * sizeof(char)); // allocate
   if(pszPriorityText == NULL) return(FALSE); // failed to allocate - return with error
   strcpy(pszPriorityText, lpszText);       // copy text into buffer

   //---Timer-------------------------
   if(uTimeOut > 0) {
      uPriorityTimer = SetTimer(GetBarWindow(), CSB_IDPRIORITYTIMER, uTimeOut, (TIMERPROC) NULL);
   }

   //---Refresh window----------------
   InvalidateBarRect(NULL);                 // refresh bar contents
   return(TRUE);
}

/*********************************************************
*  PaintStatusBar
* rev 6: Accepts NULL hdcIn and a specified region
*********************************************************/
void CStatusBar::PaintStatusBar(HDC hdcIn, int iRgnPaint) {
   HFONT    hfOld;                          // restore previous font
   HPEN     hpOld;                          // restore previous pen
   COLORREF rgbBkOld, rgbFgOld;             // restore previous background, foreground
   HDC      hdcBmp;                         // DC for bitmap blit
   RECT     rcWnd, rc;                      // window, text out rectangles
   HDC      hdc;                            // local DC, if not supplied
   int      iRgn;                           // region loop counter

   hdc = (hdcIn==NULL) ? GetDC(GetBarWindow()) : hdcIn; // use supplied DC or get own
   GetClientRect(GetBarWindow(), &rcWnd);   // get bar size
   hfOld = (HFONT) SelectObject(hdc, hfStatusBarFont);
   hpOld = (HPEN) SelectObject(hdc, hpBord[CSB_PENFRAME]);
   rgbBkOld = SetBkColor(hdc, GetSysColor(COLOR_3DFACE));
   rgbFgOld = SetTextColor(hdc, GetSysColor(COLOR_BTNTEXT));

   //===Priority Text=====================================
   if(pszPriorityText != NULL) {
      SetRect(&rc, rcWnd.left, rcWnd.top+2, rcWnd.right, rcWnd.bottom-rcWnd.top);
      ExtTextOut(hdc,
         rc.left + CSB_TEXTXOFFSET, rc.top + CSB_TEXTYOFFSET, ETO_CLIPPED | ETO_OPAQUE, &rc,
            pszPriorityText, strlen(pszPriorityText), NULL);
      SelectObject(hdc, hpBord[CSB_PENFRAME]); MoveTo(hdc, rcWnd.left,  rcWnd.top);   LineTo(hdc, rcWnd.right, rcWnd.top);
      SelectObject(hdc, hpBord[CSB_PENLITE]);  MoveTo(hdc, rcWnd.left,  rcWnd.top+1); LineTo(hdc, rcWnd.right, rcWnd.top+1);

   //===Standard Text(s)==================================
   } else {
      for(iRgn=0; iRgn<iNumRegion; iRgn++) if((iRgn==iRgnPaint) || (iRgnPaint==-1)) {
         SetRect(&rc, iRegionPos[iRgn], rcWnd.top+2,
            (iRgn<iNumRegion-1) ? iRegionPos[iRgn+1] : rcWnd.right, rcWnd.bottom);
         ExtTextOut(hdc,
            rc.left+CSB_TEXTXOFFSET, rc.top+CSB_TEXTYOFFSET,
            ETO_CLIPPED | ETO_OPAQUE, &rc,
            pszStandardText[iRgn], pszStandardText[iRgn] ? strlen(pszStandardText[iRgn]) : 0, NULL);
         SelectObject(hdc, hpBord[CSB_PENFRAME]); MoveTo(hdc, rc.left,  rcWnd.top);   LineTo(hdc, rc.right, rcWnd.top);
         SelectObject(hdc, hpBord[CSB_PENLITE]);  MoveTo(hdc, rc.left,  rcWnd.top+1); LineTo(hdc, rc.right, rcWnd.top+1);
      }
   }

   //===Finalize==========================================
   SetTextColor(hdc, rgbFgOld);
   SetBkColor(hdc, rgbBkOld);
   SelectObject(hdc, hpOld);
   SelectObject(hdc, hfOld);

   if(hdcIn==NULL) ReleaseDC(GetBarWindow(), hdc); // release local DC
}


/*********************************************************
*  StatusBarWndProc
*  This callback function must be declared STATIC
*********************************************************/
LRESULT CALLBACK CStatusBar::StatusBarWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
   CStatusBar *pSB;                         // object of interest
   PAINTSTRUCT ps;                          // for painting

   pSB = (CStatusBar*) GetWindowLong(hWnd, 0); // object associated with window
   if(pSB == NULL) return( DefWindowProc(hWnd, uMsg, wParam, lParam) ); // shouldn't happen

   switch(uMsg) {
   case WM_PAINT:
      BeginPaint(hWnd, &ps);
      pSB->PaintStatusBar(ps.hdc);
      EndPaint(hWnd, &ps);
      break;

   case WM_SIZE: pSB->OnResize(); break;    // update position rectangle positions

   case WM_DESTROY:
      pSB->SetPriorityText(NULL, 0);        // remove timer since its messages are sent to this window
      break;

   case WM_TIMER:
      pSB->SetPriorityText(NULL, 0);        // timer fades priority text
      break;

   default:
      return( DefWindowProc(hWnd, uMsg, wParam, lParam) );
   }
   return(0L);
}


/*********************************************************
* OnParentResize
* Added Rev. 5. When  the parent window  is resized, this
* function positions the status  bar at the BOTTOM of the
* specified client  rectangle, and shrinks  the rectangle
* to not cover the status bar.
*********************************************************/
void CStatusBar::OnParentResize(LPRECT lprc) {
   if(GetBarWindow() == NULL) return;       // ignore if no bar window
   MoveBarWindow(lprc->left, lprc->bottom-GetBarHeight(),
      lprc->right-lprc->left, GetBarHeight());
   SetRect(lprc, lprc->left, lprc->top, lprc->right, lprc->bottom-GetBarHeight());
}


/*********************************************************
* SetRegionWidths
*********************************************************/
void CStatusBar::SetRegionWidths(double *pdWidths, int iNum) {
   int iRgn;                                // loop counter

   if(pdWidths == NULL) return;             // ignore bad pointer
   if(iNum < 1) return;                     // ignore bad element
   if(iNum > CSB_REGIONMAX+1) iNum=CSB_REGIONMAX+1;
   for(iRgn=0; iRgn<iNum; iRgn++) dRegionWidth[iRgn] = pdWidths[iRgn];

   for(iRgn=iNum+1; iRgn<=CSB_REGIONMAX; iRgn++) {
      if(pszStandardText[iRgn]) { free(pszStandardText[iRgn]); pszStandardText[iRgn] = NULL; };
   }
   iNumRegion = iNum;
   OnResize();
}


/*********************************************************
* OnResize                                          rev6
* We have to re-calculate the position of the regions.
* The two options are:
*  - Fractional width of window (width <= 1.00)
*  - Absolute width (width > 1.00)
* Either the  first or last element should be  zero, this
* determines the filling order (left-to-right or right-to
* left)
*********************************************************/
void CStatusBar::OnResize(void) {
   int iRgn;                                // region loop counter
   int iPos;                                // position temp variable
   RECT rcWnd;                              // window rectangle
   double dWndWdth;                         // window width

   GetClientRect(GetBarWindow(), &rcWnd);   // get client rectangle to fill
   //---Empty bar-------------------------------
   if(rcWnd.right - rcWnd.left < 1) {
      for(iRgn=0; iRgn<iNumRegion; iRgn++) iRegionPos[iRgn] = 0;
      return;
   }
   dWndWdth = (double) (rcWnd.right - rcWnd.left); // reciprocal width

   //---Right-to-left---------------------------
   if(dRegionWidth[0] == 0.00) {
      iPos = rcWnd.right;
      for(iRgn=iNumRegion-1; iRgn>=0; iRgn--) {
         iPos -= (int) ((dRegionWidth[iRgn] <= 1.00) ? dWndWdth : 1.00) * dRegionWidth[iRgn];
         iRegionPos[iRgn] = iPos;
      }
      iRegionPos[0] = 0;                    // force start at left end

   //---Left-to-right---------------------------
   } else {
      iPos = rcWnd.left;
      for(iRgn=0; iRgn<iNumRegion; iRgn++) {
         iRegionPos[iRgn] = iPos;
         iPos += (int) ((dRegionWidth[iRgn] <= 1.00) ? dWndWdth : 1.00) * dRegionWidth[iRgn];
      }
   }
}
