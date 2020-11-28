/*********************************************************
* CQuickMenu
* $PSchlup 2006 $     $Revision 1 $
*********************************************************/
#include "CQuickMenu.h"

#define CSZ_QMENU_CLASSNAME "CQuickMenu_CLASS"

/*********************************************************
* Constructor
*********************************************************/
CQuickMenu::CQuickMenu(HWND _hwParent) {
   WNDCLASS wc;

   //---Members---------------------------------
   hwParent     = _hwParent;
   tfVisible    = FALSE;
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
   wc.lpszClassName = CSZ_QMENU_CLASSNAME;
   wc.lpszMenuName  = NULL;
   wc.style         = CS_HREDRAW | CS_VREDRAW;
   wc.lpfnWndProc   = CQuickMenu::_WndProcQMenu;
   RegisterClass(&wc);

   hwBack = CreateWindow(CSZ_QMENU_CLASSNAME, NULL,
      WS_CHILD,
      0, 0, 10, 10,
      hwParent, (HMENU) NULL, (HINSTANCE) GetWindowLong(hwParent, GWL_HINSTANCE), (LPVOID) this);

   hwMenu = CreateWindow("COMBOBOX", NULL,
      WS_CHILD | CBS_DROPDOWNLIST,
      0, 0, 100, 24,
      hwBack, (HMENU) NULL, (HINSTANCE) GetWindowLong(hwParent, GWL_HINSTANCE), (LPVOID) NULL);
   
}


/*********************************************************
* Destructor
*********************************************************/
CQuickMenu::~CQuickMenu() {
   //---Windows---------------------------------
   if(hwMenu) DestroyWindow(hwMenu); hwMenu = NULL;
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
void CQuickMenu::SetPosition(int x, int y, int iWid, int iHig) {
   int iItemHig;                            // height of items

   if(hwMenu == NULL) return;               // ignore if no menu
   iItemHig = SendMessage(hwMenu, CB_GETITEMHEIGHT, (WPARAM)0, 0L);
   MoveWindow(hwBack, x, y, iWid, iItemHig+6, TRUE);
   MoveWindow(hwMenu, 0, 0, iWid, iHig, TRUE);
}
//===SetPositionRect======================================
void CQuickMenu::SetPositionRect(LPRECT prc) {
   SetPosition(prc->left, prc->top, prc->right-prc->left, prc->bottom-prc->top);
}
//===SetPositionRectAuto==================================
void CQuickMenu::SetPositionRectAuto(LPRECT prc) {
   int iCt;                                 // number of items
   int iItemHig;                            // item height

   if(hwMenu == NULL) return;               // ignore if no menu
   iCt = (int) SendMessage(hwMenu, CB_GETCOUNT, 0, 0L);
   iItemHig = (int) SendMessage(hwMenu, CB_GETITEMHEIGHT, 0, 0L);
   SetPosition(prc->left, prc->top, prc->right-prc->left, prc->bottom + iItemHig*iCt);
}

/*********************************************************
* Show
*********************************************************/
void CQuickMenu::Show(void) {
   if(hwMenu == NULL) return;               // ignore if no menu
   ShowWindow(hwBack, SW_SHOW);             // show the control
   ShowWindow(hwMenu, SW_SHOW);             // show the control
   SetFocus(hwMenu);
   SendMessage(hwMenu, CB_SHOWDROPDOWN, (WPARAM) (BOOL) TRUE, (LPARAM) 0L);
   tfVisible = TRUE;                        // track visibility
}


/*********************************************************
* Hide
* With the extra argument to specify whether to apply the
* new value or not.
*********************************************************/
void CQuickMenu::Hide(void) {
   if(hwMenu == NULL) return;               // ignore if no menu
   ShowWindow(hwMenu, SW_HIDE);             // hide control
   ShowWindow(hwBack, SW_HIDE);             // hide control
   tfVisible = FALSE;                       // track visibility
}

/*********************************************************
* Process
*********************************************************/
void CQuickMenu::Process(int iNext) {
   int iSel;                                // selected item
   iSel = SendMessage(hwMenu, CB_GETCURSEL, 0, 0L);
   if((lpfnCallback==NULL)
      || (!lpfnCallback(iSel, iNext, pvUser, lUser)==0)) { // callback ok..
      Hide();                               // hide the control
      return;
   }
}

/*********************************************************
* SetUserData
*********************************************************/
void CQuickMenu::SetUserData(void *pvUserIn, long int lUserIn) {
   pvUser = pvUserIn;
   lUser  = lUserIn;
}


/*********************************************************
* SetCallback
* The callback function has the declaration
* BOOL Callback(int iSel, int iNext, void *pVoid, long int lLong)
* The function should return TRUE if the control can be
* hidden.
*********************************************************/
void CQuickMenu::SetCallback(LPQMCALLBACK lpfnCallbackIn) {
   lpfnCallback = lpfnCallbackIn;
}

/*********************************************************
* SetItems
* The pszItems string is a zero-terminated list of zero-
* terminated strings, such as "one\0two\0"\0.
*********************************************************/
void CQuickMenu::SetItems(const char *pszItems, int iSel) {
   char *psz;                            // walk through item list
   if(pszItems) {
      ClearItems();                      // clear the list
      for(psz=(char*) pszItems; psz[0]!='\0'; psz+=strlen(psz)+1) {
         AddItem(psz);
      }
   }
   SetSelection(iSel);
}


/*********************************************************
* Items
*********************************************************/
//===ClearItems===========================================
void CQuickMenu::ClearItems(void) {
   if(hwMenu) SendMessage(hwMenu, CB_RESETCONTENT, 0, 0L);
}
//===AddItem==============================================
void CQuickMenu::AddItem(const char *pszItem) {
   if(hwMenu) SendMessage(hwMenu, CB_ADDSTRING, 0, (LPARAM) pszItem);
}
//===SetSelection=========================================
void CQuickMenu::SetSelection(int iSel) {
   if(hwMenu) SendMessage(hwMenu, CB_SETCURSEL, (WPARAM) iSel, 0L);
}

/*********************************************************
* SetFont
* Ensure the control is deleted before the font is!
*********************************************************/
void CQuickMenu::SetFont(HFONT hFont) {
   if(hwMenu) SendMessage(hwMenu, WM_SETFONT, (WPARAM) hFont, MAKELPARAM(TRUE, 0));
}




/*********************************************************
* WndProc
* This callback function must be declared static
*********************************************************/
LRESULT CALLBACK CQuickMenu::_WndProcQMenu(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
   switch(uMsg) {
   case WM_CREATE:
      SetWindowLong(hWnd, GWL_USERDATA, (LONG)(CQuickMenu*)(((LPCREATESTRUCT) lParam)->lpCreateParams));
      break;
   default:
      return( ((CQuickMenu*) GetWindowLong(hWnd, GWL_USERDATA))->WndProcQMenu(hWnd, uMsg, wParam, lParam) );
   }
   return(0L);
}
//******************************************************//
LRESULT CALLBACK CQuickMenu::WndProcQMenu(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
   switch(uMsg) {
   case WM_COMMAND:
      switch(HIWORD(wParam)) {
      case CBN_SELENDOK:     Process(0); break;
      case CBN_SELENDCANCEL: Hide();     break;
      case CBN_KILLFOCUS:    Hide();     break;
      }
      break;
   }
   return(DefWindowProc(hWnd, uMsg, wParam, lParam));
}


