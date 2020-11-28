/*****************************************************************************
*  CButtonBar.h
*  $PSchlup2004 $     $Revision 5 $
*****************************************************************************/
#ifndef CBUTTONBAR_H
#define CBUTTONBAR_H

class CButtonBar;                           // declare class

#include <windows.h>                        // Windows header file
#include <stdio.h>                          // printf etc

#include "CBasicBar.h"                      // base bar class
#include "CStatusBar.h"                     // status bar object
#include "CButton.h"                        // individual buttons
#include "CTooltip.h"                       // button tooltip

/*********************************************************
* TChainLink
* From TChanLnk.cpp
* $PSchlup 2004-2006 $     $Revision 5 $
*********************************************************/
template<class T> class TChainLink;
template<class T> class TChainLink {
private:
   TChainLink<T> *pPrev;
   TChainLink<T> *pNext;
   T              TObj;
public:
   TChainLink<T>(void) { pPrev = pNext = NULL; TObj = NULL; };
   TChainLink<T>(T _TObj) { pPrev = pNext = NULL; TObj = _TObj; };
   ~TChainLink() {
      if(pPrev) pPrev->SetNextChain(pNext);
      if(pNext) pNext->SetPrevChain(pPrev);
   }
   TChainLink<T>* CreateAfter(T _TObj) {
      TChainLink<T> *pclNew;
      pclNew = new TChainLink<T>(_TObj);
      InsertAfter(pclNew);
      return(pclNew);
   }
   TChainLink<T>* CreateBefore(T _TObj) {
      TChainLink<T> *pclNew;
      pclNew = new TChainLink<T>(_TObj);
      InsertBefore(pclNew);
      return(pclNew);
   }
   void InsertAfter(TChainLink<T> *_pcT) {
      if(_pcT == NULL) return;
      _pcT->SetPrevChain(this);
      _pcT->SetNextChain(pNext);
      if(pNext) pNext->SetPrevChain(_pcT);
      pNext = _pcT;
   }
   void InsertBefore(TChainLink<T> *_pcT) {
      if(_pcT == NULL) return;
      _pcT->SetNextChain(this);
      _pcT->SetPrevChain(pPrev);
      if(pPrev) pPrev->SetNextChain(_pcT);
      pPrev = _pcT;
   }
   TChainLink<T>* DeleteToNext(void) {
      TChainLink<T> *pclRet;
      pclRet = pNext;
      delete(this);
      return(pclRet);
   }
   TChainLink<T>* DeleteToPrev(void) {
      TChainLink<T> *pclRet;
      pclRet = pPrev;
      delete(this);
      return(pclRet);
   }
   TChainLink<T>* DeleteClassToNext(void) {
      delete( Get() );
      return( DeleteToNext() );
   }
   TChainLink<T>* DeleteClassToPrev(void) {
      delete( Get() );
      return( DeleteToPrev() );
   }
   TChainLink<T>* GetTopChain(void) {
      TChainLink<T> *pclRet;
      pclRet = this;
      while(pclRet->GetPrevChain()) pclRet = pclRet->GetPrevChain();
      return(pclRet);
   }
   TChainLink<T>* GetEndChain(void) {
      TChainLink<T> *pclRet;
      pclRet = this;
      while(pclRet->GetNextChain()) pclRet = pclRet->GetNextChain();
      return(pclRet);
   }
   T GetTop(void) { return(GetTopChain()->Get()); };
   T GetEnd(void) { return(GetEndChain()->Get()); };

   TChainLink<T>* FindChain(T _Tdat) {      // find first matching object
      TChainLink<T> *pclRet = GetTopChain();
      while(pclRet) {
         if(pclRet->Get() == _Tdat) break;
         pclRet = pclRet->GetNextChain();
      }
      return(pclRet);
   }
   void           SetNextChain(TChainLink<T> *pNxt) { pNext = pNxt; };
   void           SetPrevChain(TChainLink<T> *pPrv) { pPrev = pPrv; };
   void           Set(T _TObj) { TObj = _TObj; };
   TChainLink<T>* GetNextChain(void) { return(pNext); };
   TChainLink<T>* GetPrevChain(void) { return(pPrev); };
   T              Get(void) { return(TObj); };
};


/*********************************************************
* CButtonBar Class
*********************************************************/
#define CBB_BARHEIGHT         24

//---Styles-------------------------------------
#define CBBS_NULL           0x0000          // no style
#define CBBS_DEFAULT        0x0000          // default style

#define CBBS_POSITION       0x0003          // position styles (not used?)
#define CBBS_TOP            0x0000
#define CBBS_BOTTOM         0x0001
#define CBBS_LEFT           0x0002
#define CBBS_RIGHT          0x0003

#define CBBS_BORDER         0x0030          // border styles (far side only)
#define CBBS_BORDERNONE     0x0000          // no border
#define CBBS_BORDERFLAT     0x0010          // flat border
#define CBBS_BORDERRAISED   0x0020          // raised border

#define CBBMS_NULL          0x0000          // no mouse status
#define CBBMS_MOUSESTATUS   0xF000          // bits for mouse status
#define CBBMS_DRAG          0x1000          // flag while mouse button is down
#define CBBMS_CAPTURED      0x8000          // mouse is captured to window

class CButtonBar : public CBasicBar {
private:
   TChainLink<CButton*> *pcButtonTop;       // top of button chain
   CButton  *pButMouseDown;                 // button where mouse went down
   CTooltip *pTooltip;                      // tooltip object
public:
   CButtonBar(HWND hwParent);               // constructor
   ~CButtonBar();                           // destructor

   CButton *AppendButton(LPCSTR lpszButtonName, UINT uButtonData); // add button to end
   CButton *FindButtonByData(UINT uData);   // return handle to (first) button matching iData
   void     PaintButtonBar(HDC hdc, LPRECT lprcClient); // paint the buttons

   void     OnMouseMove(HWND hWnd, int xPos, int yPos); // process WM_MOUSEMOVE
   void     OnMouseDown(HWND hWnd, int xPos, int yPos); // process WM_LBUTTONDOWN
   void     OnMouseUp(HWND hWnd, int xPos, int yPos); // process WM_LBUTTONUP

   CButton *GetButMouseDown(void)    { return(pButMouseDown); }; // get mouse-down button

   void     OnParentResize(LPRECT lprc);    // (rev.5) resized parent window

   static LRESULT CALLBACK ButtonBarWndProc(HWND, UINT, WPARAM, LPARAM);
};
#endif/*CBUTTONBAR_H*/
