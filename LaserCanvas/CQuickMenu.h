/*********************************************************
* CQuickMenu.h
*********************************************************/
#ifndef CQUICKMENU_H
#define CQUICKMENU_H

//===Includes=============================================
#include <windows.h>
#include <stdio.h>

typedef BOOL (*LPQMCALLBACK)(int iSel, int iNext, void *pVoid, long int lLong);

/*********************************************************
* CQuickMenu Class
*********************************************************/
class CQuickMenu {
private:
   HWND         hwParent;                   // parent window
   HWND         hwBack;                     // background window
   HWND         hwMenu;                     // custom control
   BOOL         tfVisible;                  // track visibility
   void        *pvUser;                     // user data
   long int     lUser;                      // user data
   LPQMCALLBACK lpfnCallback;               // callback function

   void Process(int iNext);                 // process control on Enter or Tab
public:
   CQuickMenu(HWND hwParent);               // constructor
   ~CQuickMenu();

   void SetPosition(RECT *prc);             // set the window position
   void Show(void);                         // show the menu
   void Hide(void);                         // hide the menu, no callbacks

   void SetUserData(void *pvUserIn, long int lUserIn); // set user data
   void SetCallback(LPQMCALLBACK lpfnCallbackIn); // set the callback function
   void SetFont(HFONT hFont);               // set control's font
   void SetPosition(int x, int y, int iWid, int iHig); // set the window's position
   void SetPositionRect(LPRECT prc);        // set position to given rectangle
   void SetPositionRectAuto(LPRECT prc);    // set position; height automatic
   void SetItems(const char *pszItems, int iSel=0); // set item menu texts and selection
   void ClearItems(void);                   // clear all items
   void AddItem(const char *pszItem);       // append an item
   void SetSelection(int iSel);             // set selected item

   //---Access----------------------------------
   HWND Window(void) { return(hwMenu); };
   BOOL IsVisible(void)                { return(tfVisible); };

   //---Callbacks-------------------------------
   static LRESULT CALLBACK _WndProcQMenu(HWND,UINT,WPARAM,LPARAM);
   LRESULT CALLBACK WndProcQMenu(HWND,UINT,WPARAM,LPARAM);
};

#endif//CQUICKMENU_H