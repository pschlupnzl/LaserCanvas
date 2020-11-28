/*********************************************************
* CQuickEdit.h
* $PSchlup 2006 $     $Revision 3 $
*********************************************************/
#ifndef CQUICKEDIT_H
#define CQUICKEDIT_H
//===Includes=============================================
#include <windows.h>
#include <stdio.h>

typedef BOOL (*LPQECALLBACK)(void* pszString, int iNext, void *pVoid, long int lLong);

//===Constants============================================
#define CQEDITF_VISIBLE     0x8000          // visible
#define CQEDITF_NOSPACE     0x0100          // accept no spaces
#define CQEDITF_TYPESTR     0x0000          // data is a string
#define CQEDITF_TYPEINT     0x0001          // data is integer
#define CQEDITF_TYPEDBL     0x0002          // data is double
#define CQEDITF_TYPE        0x000F          // data type


/*********************************************************
* CQuickEdit Class
*********************************************************/
class CQuickEdit {
private:
   HWND         hwParent;                   // parent window
   HWND         hwBack;                     // background window
   HWND         hwEdit;                     // custom control
   WNDPROC      lpfnQEditCtl;               // subclassed edit control procedure
   UINT         uFlags;                     // control flags

   void        *pvUser;                     // user data
   long int     lUser;                      // user data
   LPQECALLBACK lpfnCallback;               // callback function

   void Process(int iNext);                 // process control on Enter or Tab
   UINT DataType(void)                 { return(CheckBit(CQEDITF_TYPE)); };
   void SetDataType(UINT uTyp)         { ClearBit(CQEDITF_TYPE); SetBit(uTyp); };
public:
   CQuickEdit(HWND _hwParent);              // constructor
   ~CQuickEdit();                           // destructor

   void SetPosition(RECT *prc);             // set the window position
   void Show(void); //HWND hwDown, WPARAM wParam, LPARAM lParam); // show the edit control, mouse coordinates needed
   void Hide(void);                         // hide the menu, no callbacks

   void SetUserData(void *pvUserIn, long int lUserIn); // set user data
   void SetCallback(LPQECALLBACK lpfnCallbackIn); // set the callback function
   void SetFont(HFONT hFont);               // set control's font
   void SetPosition(int x, int y, int iWid, int iHig); // set the window's position
   void SetPositionRect(LPRECT prc);        // set position to given rectangle
   void SetPositionRectAuto(LPRECT prc);    // set position; height automatic
   void SetString(const char *pszText);     // set item edit text and selection
   void SetDouble(double dVal);             // set a double value input
   void SetInt(int iVal);                   // set integer value input
   void SetNoSpace(BOOL tfNoSp)        { if(tfNoSp) SetBit(CQEDITF_NOSPACE); else ClearBit(CQEDITF_NOSPACE); };

   //---Flags-----------------------------------
   void SetBit(UINT u)                 { uFlags |= u; };
   void ClearBit(UINT u)               { uFlags &=~u; };
   void ToggleBit(UINT u)              { uFlags ^= u; };
   UINT CheckBit(UINT u)               { return(uFlags & u); };

   //---Access----------------------------------
   HWND Window(void)                   { return(hwEdit); };
   BOOL IsVisible(void)                { return(CheckBit(CQEDITF_VISIBLE) ? TRUE : FALSE); };

   //---Callbacks-------------------------------
   static LRESULT CALLBACK _WndProcQEdit(HWND,UINT,WPARAM,LPARAM);
   LRESULT CALLBACK WndProcQEdit(HWND,UINT,WPARAM,LPARAM);

   static LRESULT CALLBACK _WndProcQEditCtl(HWND,UINT,WPARAM,LPARAM);
   LRESULT CALLBACK WndProcQEditCtl(HWND,UINT,WPARAM,LPARAM);
};


#endif//CQUICKEDIT_H
