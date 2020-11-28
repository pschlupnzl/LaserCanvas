/*********************************************************
* CStatusBar.h
* Simple status bar class for status texts
* $PSchlup 2004 $     $Revision 6 $
*********************************************************/

#ifndef CSTATUSBAR_H
#define CSTATUSBAR_H
class CStatusBar;                           // declare as class

#include "CBasicBar.h"                      // derived from basic bar class

#define CSB_FONTHEIGHT        -12           // font height (<0 pixel size)
#define CSB_FONTNAME      "Tahoma"          // font name (use EnumFontFamilies?)
#define CSB_BARHEIGHT          20           // height of bar
#define CSB_TEXTXOFFSET         4           // x offset of text in status bar
#define CSB_TEXTYOFFSET         1           // y offset of text in status bar
#define CSB_IDPRIORITYTIMER     1           // nIDEvent of priority timer
#define CSB_RCTEXTTIMEOUT    2500           // fade time for ResourcePriorityText

#define CSB_REGIONMAX           3           // maximum number of regions

#define CSB_BMPSTOP             0           // stop bitmap
#define CSB_BMPMAX              0           // maximum number of bitmaps

#define CSB_PENLITE             0           // highlight pen color
#define CSB_PENDARK             1           // dark pen color
#define CSB_PENFRAME            2           // frame pen color
#define CSB_PENMAX              2           // highest pen index

/*********************************************************
*  CStatusBar
*********************************************************/
class CStatusBar : public CBasicBar {
private:
   HFONT  hfStatusBarFont;                  // status bar font
   HBITMAP hbmStatusBarBmp[CSB_BMPMAX+1];   // bitmap resources
   HPEN   hpBord[CSB_PENMAX+1];             // pen resources
   char  *pszStandardText[CSB_REGIONMAX+1]; // standard text
   char  *pszPriorityText;                  // priority text
   UINT   uPriorityTimer;                   // timer for priority text to fade
   int    iNumRegion;                       // number of regions
   double dRegionWidth[CSB_REGIONMAX+1];    // width of regions (last one fills to width)
   int    iRegionPos[CSB_REGIONMAX+1];      // screen position of regions
public:
   CStatusBar::CStatusBar(HWND hwParent, int iNumRgn=1);   // constructor
   CStatusBar::~CStatusBar();               // destructor

   void   OnParentResize(LPRECT lprc);      // (rev.5) position bar, shrink rectangle
   void   OnResize(void);                   // (rev.6) calculate screen positions
   void   SetRegionWidths(double *pdWidths, int iNumRgnWdth=1); // (rev.6) define region widths

   void   ResourcePriorityText(UINT uCmd); // set priority text, for tooltip
   void   PaintStatusBar(HDC hdc, int iRgn=-1); // paint the bar
   int    GetBarHeight(void) { return(CSB_BARHEIGHT); }; // retrieve bar height

   BOOL   SetPriorityText(LPCSTR lpszText, UINT uTimeOut=1500); // set priority text
   void   SetPriorityTimer(UINT u) { uPriorityTimer = u; }; // set priority timer
   BOOL   SetStandardText(LPCSTR lpszText, int iRgn=0); // set the priority text

   char*  GetPriorityText(void)    { return(pszPriorityText); };
   UINT   GetPriorityTimer(void)   { return(uPriorityTimer); };
   char*  GetStandardText(int iRgn=0)    { return(((iRgn>=0)&&(iRgn<iNumRegion)) ? pszStandardText[iRgn] : NULL); };

   static LRESULT CALLBACK StatusBarWndProc(HWND, UINT, WPARAM, LPARAM);
};

#endif/*CSTATUSBAR_H*/
