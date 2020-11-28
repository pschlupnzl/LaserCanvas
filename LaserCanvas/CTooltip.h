/*********************************************************
* CTooltip
* A trial object to implement tooltips
*********************************************************/
#ifndef CTOOLTIP_H                          // prevent multiple includes
#define CTOOLTIP_H
//===Includes=============================================
#include <windows.h>                        // Windows header
#include <stdio.h>                          // standard header

//===Constants and Macros=================================
#define SZ_WCTOOLTIP "SpiderTooltipWindowClass"
#define CTTIP_VISTIMER           1          // timer for toggling visibility

/*********************************************************
* CToolTip Class
*********************************************************/
class CTooltip {
private:
   HWND       hWndTip;                      // tooltip window
   char      *szTipString;                  // tooltip string
   HFONT      hfTipFont;                    // tooltip font
   BOOL       tfVisible;                    // is tooltip window currently shown
   UINT       uVisTimer;                    // visibility timer (automatic on show)

public:
   CTooltip(HINSTANCE hInst);               // constructor and initialization
   ~CTooltip();                             // destructor

   BOOL SetTipString(const char *cszString); // set the string
   BOOL SetTipStringFromResource(UINT uID, HINSTANCE hInst); // set string from stringtable resource
   void Position(int x, int y, BOOL tfShowDelayed); // move to a position
   void Show(void);                         // show the tooltip
   void Hide(void);                         // hide the tooltip

   //---Property access---------------
   HWND       GetTipWindow(void)       { return(hWndTip); };
   BOOL       IsVisible(void)          { return(tfVisible); };

   static LRESULT CALLBACK TipWndProc(HWND, UINT, WPARAM, LPARAM); // window callback
};

#endif//CTOOLTIP.H
