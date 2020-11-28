/*****************************************************************************
*  CButton.cpp
*  $PSchlup2004 $     $Revision 0.0 $
*****************************************************************************/

#include "cbutton.h"

#define MoveTo(d,x,y)   MoveToEx(d, x, y, NULL)
//#define DEBUGMSG(txt)   MessageBox(NULL, txt, "Debug", MB_OK)
#define DEBUGMSG(txt)   //printf("%s\n", txt);

/*********************************************************
*  Constructor and initialization
*********************************************************/
CButton::CButton(void) {
   DEBUGMSG("CButton::CButton");
   CButtonInit();
}
CButton::CButton(HINSTANCE hInstance, LPCSTR lpszBitmapName, int iLeft, int iTop, UINT uData) {
   DEBUGMSG("CButton::CButton(H,L,i,i)");
   CButtonInit();
   if(lpszBitmapName == NULL) {
      hbmButtonBitmap = NULL;
      hbmButtonBitmapDisabled = NULL;
      SetRect(&rcButtonRect, iLeft, iTop, iLeft+8, iTop+CBUT_BMPSIZE+6);
   } else {
      LoadButtonBitmap(hInstance, lpszBitmapName);
      SetButtonPos(iLeft, iTop);
   }
   SetButtonData(uData);
}

//===Global Initialization================================
// Set ALL values to zero - modify as necessary afterwards
void CButton::CButtonInit(void) {
   //---Initial Values--------------------------
   hbmButtonBitmap = NULL;                  // no bitmap loaded yet
   hbmButtonBitmapDisabled = NULL;          // no bitmap loaded yet
   SetButtonData(0);                        // no message yet
   SetButtonPos(0, 0);                      // define button rectangle
   SetButtonStatus(CBST_FLAT);              // default status
}

/*********************************************************
*  Destructor
*********************************************************/
CButton::~CButton() {
   if(hbmButtonBitmap         != NULL) DeleteObject(hbmButtonBitmap        ); // delete bitmap
   if(hbmButtonBitmapDisabled != NULL) DeleteObject(hbmButtonBitmapDisabled); // delete bitmap
   DEBUGMSG("Destroyed CButton");
}



/*********************************************************
*  Set and Get functions
*********************************************************/
//===Data=================================================
void CButton::SetButtonData(UINT uData)     { uButtonData = uData; }
UINT CButton::GetButtonData(void)           { return(uButtonData); };

//===Position=============================================
void CButton::SetButtonPos(int iLeft, int iTop) {
   SetRect(&rcButtonRect, iLeft, iTop, iLeft+CBUT_BMPSIZE+6, iTop+CBUT_BMPSIZE+6);
}
void CButton::GetButtonRect(RECT *prc)      { CopyRect(prc, &rcButtonRect); }

//===Status===============================================
void CButton::SetButtonStatus(UINT uStat)   { uStatus = uStat; }
UINT CButton::GetButtonStatus(void)         { return(uStatus); }

//===Toggle===============================================
void CButton::ToggleButton(BOOL tfToggle) {
   uStatus = (uStatus & ~CBST_TOGGLE) | (tfToggle ? CBST_TOGGLE : 0);
}
BOOL CButton::IsButtonToggled(void) { return( (uStatus&CBST_TOGGLE) ? TRUE : FALSE ); };

//===Enable===============================================
void CButton::EnableButton(BOOL tfEnable) {
   uStatus = (uStatus & ~CBST_DISABLE) | (tfEnable ? 0 : CBST_DISABLE);
}
BOOL CButton::IsButtonEnabled(void) { return( (uStatus&CBST_DISABLE) ? FALSE : TRUE ); };

/*********************************************************
*  LoadButtonBitmap
*  Loads a bitmap from the resource of hInstance
*********************************************************/
int CButton::LoadButtonBitmap(HINSTANCE hInstance, LPCSTR lpszBitmapName) {
   char szBuf[256];                         // string for disabled bitmap name

   //---Delete existing-------------------------
   if(hbmButtonBitmap != NULL)
      DeleteObject(hbmButtonBitmap);        // delete previously loaded bitmap
   if(hbmButtonBitmapDisabled != NULL)
      DeleteObject(hbmButtonBitmapDisabled); // delete previously loaded bitmap

   //---Load standard---------------------------
//   hbmButtonBitmap = LoadBitmap(hInstance, lpszBitmapName); // load bitmap
   hbmButtonBitmap = (HBITMAP) LoadImage(hInstance, lpszBitmapName,
         IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT);
      // LoadImage with 3DCOLORS and TRANSPARENT: Replaces color table entry
      // of top-left pixel with COLOR_3DFACE

   //---Load disabled image---------------------
   strcpy(szBuf, lpszBitmapName);
   sprintf(szBuf + strlen(szBuf), "_DISABLED");
   hbmButtonBitmapDisabled = (HBITMAP) LoadImage(hInstance, szBuf,
         IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT);

   //---Finish----------------------------------
   return(hbmButtonBitmap != NULL);
}



/*********************************************************
*  OnMouseMove
*********************************************************/
BOOL CButton::OnMouseMove(HDC hdc, int xPos, int yPos, CButton *pButDown) {
   POINT pt;                                // mouse position
   UINT  uStatusOld;                        // track previous display status
   UINT  uBit;                              // bit to set or clear
   BOOL  tfRet;                             // return value from function

   uStatusOld = uStatus;                    // compare to previous status
   pt.x = xPos; pt.y = yPos;                // assemble POINT structure
   uBit = (pButDown==NULL) ? CBST_HOVER : CBST_ACTIVE;
   CLEARBIT(uStatus, CBST_ACTIVE);
   if(PtInRect(&rcButtonRect, pt) && (pButDown==NULL || pButDown==this)) {
      tfRet = TRUE;
      SETBIT(uStatus, uBit);
   } else {
      tfRet = FALSE;
      CLEARBIT(uStatus, uBit);
   }

   if(CHECKBIT(uStatus, CBST_DISABLE)) return(tfRet); // NOP if disabled

   if(uStatus != uStatusOld) {              // paint only if status changed here
      PaintButton(hdc);
   }
   return(tfRet);
}


/*********************************************************
*  PaintButton
*********************************************************/
void CButton::PaintButton(HDC hdc) {
   HDC    hdcBitmap;                        // DC for button bitmap
   HPEN   hpOld, hpPen[4];                  // selected pen, colored pens
   int    iColors[6], iBmpOffs;             // border color index, bitmap offset
   int    k;                                // loop counter

   //---Paint divider---------------------------
   if(hbmButtonBitmap == NULL) {
      hpPen[0] = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
      hpPen[1] = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DHILIGHT));

      FillRect(hdc, &rcButtonRect, GetSysColorBrush(COLOR_3DFACE));

      hpOld = (HPEN) SelectObject(hdc, hpPen[0]); // store old pen
      MoveTo(hdc, rcButtonRect.left+3, rcButtonRect.top);
      LineTo(hdc, rcButtonRect.left+3, rcButtonRect.bottom);
      SelectObject(hdc, hpPen[1]);
      MoveTo(hdc, rcButtonRect.left+4, rcButtonRect.top);
      LineTo(hdc, rcButtonRect.left+4, rcButtonRect.bottom);

      SelectObject(hdc, hpOld);                // restore previous pen
      DeleteObject(hpPen[1]);
      DeleteObject(hpPen[0]);

   //---Paint button----------------------------
   } else {
      hpPen[0] = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DFACE));
      hpPen[1] = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DHILIGHT));
      hpPen[2] = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
      hpPen[3] = (HPEN) GetStockObject(BLACK_PEN); // stock object, doesn't need deleting
      hpOld = (HPEN) SelectObject(hdc, hpPen[3]); // store old pen

      iColors[0] = 0; iColors[1] = 0; iColors[2] = 0;
      iColors[3] = 0; iColors[4] = 0; iColors[5] = 0;
      iBmpOffs = 0;

      if(uStatus&CBST_DISABLE) {
         // keep as flat

      } else if(uStatus & CBST_3D) {
         iColors[0] = 0; iColors[1] = 1; iColors[2] = 0;
         iColors[3] = 0; iColors[4] = 2; iColors[5] = 3;
         iBmpOffs = 0;

      } else if((uStatus & CBST_FRAME) && ((uStatus & CBST_TOGGLE) || (uStatus & CBST_ACTIVE))) {
         iColors[0] = 3; iColors[1] = 2; iColors[2] = 0;
         iColors[3] = 0; iColors[4] = 0; iColors[5] = 3;
         iBmpOffs = 1;
      } else if(uStatus & CBST_FRAME) {
         iColors[0] = 3; iColors[1] = 0; iColors[2] = 0;
         iColors[3] = 0; iColors[4] = 0; iColors[5] = 3;
         iBmpOffs = 0;

      } else if(uStatus & CBST_ACTIVE) {
         iColors[0] = 3; iColors[1] = 2; iColors[2] = 0;
         iColors[3] = 0; iColors[4] = 0; iColors[5] = 1;
         iBmpOffs = 1;
      } else if(uStatus & CBST_TOGGLE) {
         iColors[0] = 0; iColors[1] = 2; iColors[2] = 0;
         iColors[3] = 0; iColors[4] = 0; iColors[5] = 1;
         iBmpOffs = 1;
      } else if(uStatus & CBST_HOVER) {
         iColors[0] = 0; iColors[1] = 1; iColors[2] = 0;
         iColors[3] = 0; iColors[4] = 0; iColors[5] = 2;
         iBmpOffs = 0;
      }

      //---Framing lines---------------------------
      for(k=0; k<6; k++) {
         SelectObject(hdc, hpPen[iColors[k]]); // select pen color
         if(k-iBmpOffs < 3) { //---Top and left---
            // the (k>2) adds pixels where the offset bitmap would leave
            // holes with the simple paint algorithm
            MoveTo(hdc, rcButtonRect.left+k,    rcButtonRect.bottom-k-1+(k>2));
            LineTo(hdc, rcButtonRect.left+k,    rcButtonRect.top+k);
            LineTo(hdc, rcButtonRect.right-k+(k>2), rcButtonRect.top+k);
         } else { //---Bottom and right---
            MoveTo(hdc, rcButtonRect.left+6-k,  rcButtonRect.bottom-6+k);
            LineTo(hdc, rcButtonRect.right-6+k, rcButtonRect.bottom-6+k);
            LineTo(hdc, rcButtonRect.right-6+k, rcButtonRect.top+6-k-1);
         }
      }

      //---Blit bitmap-----------------------------
      hdcBitmap = CreateCompatibleDC(hdc);
      if((hbmButtonBitmapDisabled) && (CHECKBIT(uStatus, CBST_DISABLE)))
         SelectObject(hdcBitmap,  hbmButtonBitmapDisabled);
      else
         SelectObject(hdcBitmap,  hbmButtonBitmap);
      BitBlt(hdc, rcButtonRect.left+3+iBmpOffs, rcButtonRect.top+3+iBmpOffs,
            CBUT_BMPSIZE, CBUT_BMPSIZE, hdcBitmap, 0, 0, SRCCOPY);
      DeleteDC(hdcBitmap);

      SelectObject(hdc, hpOld);                // restore previous pen
      for(k=2; k>=0; k--) { // Black pen is stock object, doesn't need deleting
         DeleteObject(hpPen[k]);
      }
   }//endif(hbmButtonBitmap
}

