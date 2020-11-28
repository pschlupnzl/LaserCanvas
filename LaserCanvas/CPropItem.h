/*********************************************************
* CPropItem.h
* Note: It's a CChainLink double-sided chain
* $PSchlup 2004-2006 $     $Revision 5 $
*********************************************************/
#ifndef CPROPITEM_H                         // prevent multiple includes
#define CPROPITEM_H

//===Classes Defined======================================
class CPropItem;                            // individual property item

//===Includes=============================================
#include <windows.h>                        // standard Windows header
#include <stdio.h>                          // standard header

#include "CLCEqtn.h"                        // equations class

typedef union tagPROPITEMDATA { //pidt
   double     dVal;                         // single value
   char      *pszText;                      // string buffer
   struct   { double dVal1, dVal2, dVal3, dVal4; }; // double and quad values
   struct   { double dCur, dMin, dMax; };   // slider ranges
   BOOL       tfBool;                       // boolean value
   struct   { HMENU hMenu; UINT uItem; };   // popup menus
   CEquation *pEqtn;                        // equation object
   struct   { int iCur, iNum; char *pszList, *pszCur; }; // list item, dropdown list
} PROPITEMDATA, FAR *LPPROPITEMDATA;

typedef BOOL (*PROPITEMCALLBACK)(void *vData, UINT uID, void *pVoid); // callback from item changes

#include "CChainLink.h"                     // item list is double-sided chain
#include "resource.h"                       // resource constants
#include "CPropMgr.h"                       // property manager
#include "CMActive.h"                       // mouse active rectangle

//===Constants=============================================
#define CPIC_ROWHEIGHT          18          // row height
#define CPIC_SLIDERSPACE         8          // space at ends of slider
#define CPIC_SLIDERWIDTH         3          // half-width of slider
#define CPIC_SLIDERHEIGHT        5          // half-height of slider

//---Types / flags------------------------------
#define CPIT_HEADING        0x0000          // group heading
#define CPIT_TEXT           0x0001          // text
#define CPIT_VALUE          0x0002          // single value, process as equation
#define CPIT_DBLVAL         0x0003          // dual value
#define CPIT_SAGTAN         0x0004          // sagittal / tangential dual value
#define CPIT_QUADVAL        0x0005          // quad value (ABCD)
#define CPIT_EQUATION       0x0006          // equation
#define CPIT_CHECKBOX       0x0007          // check box CONTROL!
#define CPIT_TOGGLE         0x0008          // toggle field
#define CPIT_SLIDER         0x0009          // slider value
#define CPIT_HMENU          0x000A          // pointer to menu
#define CPIT_RADIOLIST      0x000B          // list of radio buttons
#define CPIT_DROPLIST       0x000C          // drop-down list CONTROL!
#define CPIT_COMMAND        0x000D          // single-click command
#define CPIF_TYPE           0x00FF          // bit-mask for type

#define CPIF_NONE           0x0000          // no extra flags
#define CPIF_READONLY       0x0100          // read only
#define CPIF_COLLAPSED      0x0200          // item group collapsed
#define CPIF_HIDDEN         0x0400          // item is hidden even in expanded state
#define CPIF_MULTI          0x0800          // multiple selection - empty window
#define CPIF_NAN1           0x1000          // first (sag/tan) value is invalid
#define CPIF_NAN2           0x2000          // second (sag/tan) value is invalid

/**********************************************************
* CPropItem Class
**********************************************************/
class CPropItem : public CChainLink {
private:
   CPropMgr *pMgrParent;                    // parent manager
   CMActive *pcActive;                      // this active
   char     *pszLabel;                      // label
   RECT      rcItem;                        // item screen rectangle
   UINT      uFlags;                        // type and flags of active
   //---Item data---------------------
   UINT             uID;                    // identifier
   void            *pVoid;                  // user data
   PROPITEMDATA     xData;                  // data
   PROPITEMCALLBACK lpfnItemCallback;       // callback function
public:
   CPropItem(CPropMgr *pMgr, const char *pszLbl, int iType, UINT uID, UINT uFlags); // constructor
   ~CPropItem();                            // destructor

   void   SetLabel(const char *pszLbl);     // set the label text
   void   SetPosition(LPRECT prc);          // set the position / update active
   void   CollapseGroup(BOOL tfCollapse);   // collapse whole group

   void   GetText(char *psz, size_t len);   // format string
   void   PaintItem(HDC hdc, BOOL tfTextOnly); // paint the item

   //---Set types-------------------------------
   void   FreeItem(void);                   // free text, if any
   void   SetItemHeading(const char *_pszHeadingAux);
   void   SetItemText(const char *_pszText);
   void   SetItemValue(double _dValue);
   void   SetItemDblValue(double _dVal1, double _dVal2);
   void   SetItemSagTanValue(double _dSag, double _dTan);
   void   SetItemQuadValue(double _dVal1, double _dVal2, double _dVal3, double _dVal4);
   void   SetItemEquation(const CEquation *_pEqtn);
   void   SetItemCheckBox(BOOL _tfBool);
   void   SetItemToggle(const char *_pszText);
   void   SetItemSlider(double _dCur, double _dMin, double _dMax);
   void   SetItemMenu(HMENU _hMenu);
   void   SetItemRadioList(int _iCur, const char *_pszList);
   void   SetItemDropList(int _iCur, const char *_pszList);
   void   SetItemCommand(const char *_pszText);
   void   SetItemCallback(PROPITEMCALLBACK lpfn, void *pVoid);

   void   SetItemNaN(BOOL tfNan1, BOOL tfNan2); // set NaN bits

   //---Mouse-----------------------------------
   void   UpdateActiveCursors(void);        // set the item's active's cursors
   static void _MouseCallback(int iMsg, int x, int y, int wKeys, void *pVoid, long int lData);
   void   MouseCallback(int iMsg, int x, int y, int wKeys);

   //---Quick controls--------------------------
   static BOOL _QEditCallback(void *pVal, int iNext, void *pVoid, long int lLong);
   BOOL   QEditCallback(void *pVal);
   static BOOL _QMenuCallback(int iSel, int iNext, void *pVoid, long int lLong);
   BOOL   QMenuCallback(int iSel);

   //---Flags-----------------------------------
   void SetBit(UINT u)                 { uFlags |= u; };
   void ClearBit(UINT u)               { uFlags &=~u; };
   void ToggleBit(UINT u)              { uFlags ^= u; };
   UINT CheckBit(UINT u)               { return(uFlags & u); };

   //---Access----------------------------------
   CApplication* App(void);                 // returns application from parent manager
   int    Height(void);                     // return height of item
   UINT   ID(void)                     { return(uID); };
   UINT   Type(void)                   { return(CheckBit(CPIF_TYPE)); };
   void   SetType(UINT uTyp)           { ClearBit(CPIF_TYPE); SetBit(uTyp); };

   //---Specifics-------------------------------
   int    SliderToScreen(void);             // convert slider value to screen coordinate
   double ScreenToSlider(int x);            // convert screen coordinate to slider-like value
};

#endif//CPROPITEM_H
