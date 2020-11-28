/*****************************************************************************
*  CBasicBar.cpp
*  Underlying class for status bar, button bar, etc
* Data Members
* ------------
*  The CBasicBar class stores information about
*   - The parent window
*     The application's instance handle can thus also be retrieved.
*   - The bar window
*     The window is  created and destroyed  as necessary if the bar becomes vis-
*     ible or hidden.
*   - The window position
*     Even when no window exists, the desired window  position is stored so that
*     a new bar window is created in the correct place
*   - Additional parameter uBarStyle
*     The parameter is intended to be used as a bit field and supports Set, Get,
*     SetBit,  ClearBit,  ToggleBit  and  CheckBit  methods, but is  not used by
*     CBasicBar itself.
*
* Derived Classes
* ---------------
*  To derive a specific class  based on CBasicBar,  an application must call the
*  CBasicBar constructor.  Suppose the derived bar takes argument uStyle in add-
*  ition to hwParent. The constructor declaration might be
*
*     CDerivedBar::CDerivedBar(HWND hwParent, UINT uStyle);
*
*  and the definition
*
*   CDerivedBar::CDerivedBar(HWND hwParent, UINT uStyle) : CBasicBar(hwParent) {
*      // initialization
*      SetBarStyle(uStyle); // set bar style, for example
*   }
*
* Window Management
* -----------------
*   To create a new window, call  CDerivedBar::CreateBarWindow(..).  The derived
*   bar class must  provide a callback  function to  handle the window messages.
*   See also the  CreateBarWindow(..) source code, and DefWindowProc in the Win-
*   dows API for more information about message handling.
*    - The callback function must be declared as
*
*         static LRESULT CALLBACK DerivedBarWndProc(HWND, UINT, WPARAM, LPARAM);
*
*      The name of  the callback  function is arbitrary.  The callback  function
*      address is passed as the second argument to CreateBarWindow(..):
*         pBar->CreateBarWindow(TRUE, CDerivedBar::DerivedBarWndProc);
*
*   - The bar window creation process works as follows:
*      - The bar is created by CBasicBar using CreateWindow.
*        The initial messages issued  before CreateWindow returns are handled by
*        the CBasicBar window callback function.
*      - During the initial WM_CREATE message, a pointer to the class is  stored
*        in the window's data structure.  Space for a pointer is reserved in the
*        cbWndExtra  parameter  of  the  WNDCLASS  structure  registered  by the
*        CBasicBar constructor.  The processing  exploits the fact that the THIS
*        keyword, when used in CBasicBar member functions, points to the derived
*        class object (!).  The object pointer is passed as the lpVoid parameter
*        in the call to CreateWindow(..).
*      - The bar  window class is  then subclassed.  The window is reconfigured,
*        using SetWindowLong(GWL_WNDPROC..), to use the lpfnProc window callback
*        function to  process window  messages.  The subclassed  window callback
*        function should, at the minimum, process the WM_PAINT message. Messages
*        not handled by the subclassed window callback function should be passed
*        to DefWindowProc(..).  This is different to the common practice of sub-
*        classed windows, where unhandled  messages are passed to the base class
*        message handler using CallWindowProc(..).  DO NOT use CallWindowProc().
*        It is not necessary here, since CBasicBar does not provide further fun-
*        ctionality.
*      - Before the  window is displayed, a  WM_CREATE message  is generated and
*        sent to the subclassed window callback function. This allows the window
*        callback functions of derived bar classes to receive WM_CREATE messages
*        as though the derived class had created its own window.
*      - Since the derived class  window callback function is declared as STATIC
*        so that its address may be taken,  the specific object instance must be
*        extracted from the  window data before member  functions can be called.
*        This is done in the callback function using
*           pBar = (CDerivedBar*) GetWindowLong(hWnd, 0);
*
*   - The above technical details should be transparent in practice.
*
* Example
* -------
* This example shows the minimum processing required by a derived class
*
*    class CDerivedBar : public CBasicBar {
*    public:
*       CDerivedBar::CDerivedBar(HWND);
*       static LRESULT CALLBACK DerivedBarWndProc(HWND, UINT, WPARAM, LPARAM);
*    };
*
*    CDerivedBar::CDerivedBar(HWND hwParent) : CBasicBar(hwParent) {
*       // additional initialization
*    }
*
*    LRESULT CALLBACK DerivedBarWndProc(
*          HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam
*    ) {
*       switch(uMsg) {
*       case WM_PAINT:
*          // paint the bar
*          break;
*       default:
*          return( DefWindowProc(hWnd, uMsg, wParam, lParam) );
*       }
*       return(0L);
*    }
*
* In the main body of the application, a new bar would be created, and its
* window created and shown, using
*    pBar = new CDerivedBar(hWnd); // hWnd is main application window
*    pBar->CreateBarWindow(TRUE, CDerivedBar::DerivedBarWndProc);
*
* $PSchlup 2004 $     $Revision 1.0 $
*****************************************************************************/

#ifndef CBASICBAR_H                         // prevent multiple includes
#define CBASICBAR_H
class CBasicBar;                            // declare as a class

#include <windows.h>                        // windows header file

#define CBAR_CLASSNAME "CBasicBarWindow"    // window class; subclassed for individual bar types

/*********************************************************
*  CBasicBar
*  Underlying class type for bar with rudimentary methods
*********************************************************/
class CBasicBar {
private:
   HWND hwParentWindow;                     // parent window
   HWND hwBarWindow;                        // window of this bar
   RECT rcBarPosition;                      // window position even when no window exists (l, t, w, h)
   UINT uBarStyle;                          // bar style (for use in derived bars)
public:
   CBasicBar::CBasicBar(HWND hwParent);     // constructor
   CBasicBar::~CBasicBar();                 // destructor

   HWND CreateBarWindow(BOOL tfShow, WNDPROC lpfnProc); // create a subclassed bar window
   void DestroyBarWindow(void);             // destroy the window
   void MoveBarWindow(int l, int t, int w, int h); // position the window
   void ShowBarWindow(BOOL tfShow);         // show or hide the window
   void InvalidateBarRect(CONST RECT *lprcInval); // invalidate part of window

   UINT      GetBarStyle(void)           { return(uBarStyle); };
   UINT      SetBarStyle(UINT uSty)      { UINT uPrv = uBarStyle; uBarStyle = uSty; return(uPrv); };
   UINT      SetBarStyleBit(UINT uBit)   { uBarStyle |= uBit; return(uBarStyle); };
   UINT      ClearBarStyleBit(UINT uBit) { uBarStyle &= ~uBit; return(uBarStyle); };
   UINT      ToggleBarStyleBit(UINT uBit){ uBarStyle ^= uBit; return(uBarStyle); };
   UINT      CheckBarStyleBit(UINT uBit) { return( uBarStyle & uBit ); };

   void      SetParentWindow(HWND hw) { hwParentWindow = hw; };
   void      SetBarPosition(int l, int t, int w, int h);
   void      SetBarPosition(const LPRECT lprc) { SetBarPosition(lprc->left, lprc->top, lprc->right, lprc->bottom); };
   HWND      GetBarWindow(void)       { return(hwBarWindow); };
   HWND      GetParentWindow(void)    { return(hwParentWindow); };
   HINSTANCE GetParentInstance(void);       // retrieve instance of parent app
   BOOL      GetBarPosition(LPRECT lprc);   // retrieve current window position (if window exists)
   int       GetBarHeight(void)       { return(rcBarPosition.bottom); };
   int       GetBarWidth(void)        { return(rcBarPosition.right); };
   static LRESULT CALLBACK BarWndProc(HWND, UINT, WPARAM, LPARAM); // window callback function

};

#endif/*CBASICBAR_H*/
