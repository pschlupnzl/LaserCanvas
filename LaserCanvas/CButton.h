/*****************************************************************************
*  CButton.h
*  $PSchlup2004 $     $Revision 0.0 $
*****************************************************************************/

#ifndef CBUTTON_H
#define CBUTTON_H
class CButton;

#include <windows.h>
#include <stdio.h>

#define SETBIT(x,b)         x |= b
#define CLEARBIT(x,b)      x &= ~b
#define CHECKBIT(x,b)   ((x&b)==b)


//---Constants and Macros-----------------------
#define CBUT_BMPSIZE            16          // size of button bitmaps
#define CBST_NULL           0x0000
#define CBST_FLAT           0x0000
#define CBST_HOVER          0x0001          // bit 0: mouse over
#define CBST_ACTIVE         0x0002          // bit 1: mouse down
#define CBST_TOGGLE         0x0004          // bit 2: button is toggled on
#define CBST_3D             0x0100          // bit 8: make old-fashioned 3d button
#define CBST_FRAME          0x0200          // bit 9: use frame-style button
#define CBST_DISABLE        0x8000          // bit 15: disable button


class CButton {
private:
   HBITMAP hbmButtonBitmap;                 // bitmap on the button
   HBITMAP hbmButtonBitmapDisabled;         // disabled bitmap for button
   RECT    rcButtonRect;                    // rectangle occupied by button
   UINT    uStatus;                         // button status
   UINT    uButtonData;                     // message / ID of button

   void CButtonInit(void);                  // internal constructor
public:
   CButton(void);                           // constructor
   CButton(HINSTANCE hInstance, LPCSTR lpszBitmapName, int iLeft, int iTop, UINT uData); // full initialization
   ~CButton();                              // destructor
   int  LoadButtonBitmap(HINSTANCE hInstance, LPCSTR lpszBitmapName); // load a bitmap
   void PaintButton(HDC hdc);               // paint the button
   BOOL OnMouseMove(HDC hdc, int xPos, int yPos, CButton *pButDown); // process WM_MOUSEMOVE

   void SetButtonData(UINT uData);          // set the button data
   void SetButtonPos(int iLeft, int iTop);  // set top left corner of button
   void SetButtonStatus(UINT uStat);        // set button status bits
   void ToggleButton(BOOL tfToggle);        // set the button toggle bit
   void EnableButton(BOOL tfEnable);        // enable or disable button

   UINT GetButtonData(void);                // retrieve button data
   void GetButtonRect(RECT *prc);           // retrieve button rectangle
   UINT GetButtonStatus(void);              // get button status bits
   BOOL IsButtonToggled(void);              // return TRUE or FALSE depending on toggle
   BOOL IsButtonEnabled(void);              // returns TRUE if button is enabled

   BOOL IsEnabled(void) { return(CHECKBIT(uStatus, CBST_DISABLE) ? FALSE : TRUE); };
};
#endif/*CBUTTON_H*/
