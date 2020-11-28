/*********************************************************
* CPropItem
* New! This is now a CChainLink, which makes managing the
* chain a lot easier.
* $PSchlup 2004-2006 $     $Revision 5 $
*********************************************************/
#include "CPropItem.h"                      // class header

///TODO: On all Active Rects, add right-click menu to copy data!
static CEquation CEq;

CApplication* CPropItem::App(void) { return(pMgrParent ? pMgrParent->GetAppParent() : NULL); };

/**********************************************************
* Constructor
**********************************************************/
CPropItem::CPropItem(CPropMgr *_pMgr, const char *_pszLabel, int _iType, UINT _uID, UINT _uFlags) {
   //---Members-----------------------
   pMgrParent = _pMgr;
   pcActive   =  NULL;
   pszLabel   =  NULL;
   uFlags     = _uFlags;
   xData.pszText = NULL;                    // ensure allocated pointers are null
   SetRect(&rcItem, 0, 0, 0, 0);
   uID        = _uID;
   pVoid      = NULL;
   lpfnItemCallback = NULL;

   //---Label-------------------------
   SetLabel(_pszLabel);                     // store label text

   //---Create Active-----------------
   pcActive = pMgrParent->CreateActive(0, 0, 0, 0, NULL, NULL, NULL, NULL,
      CPropItem::_MouseCallback, (void*) this, 0, NULL);

   //---Item--------------------------
   // Why don't  we just set  iType and the  values here?
   // Because the SetItem  functions  behave similarly to
   // user functions and we want to keep all the flexibi-
   // lity and because the type is part of the uFlags bit
   // mask
   CEq.ParseEquation("0",NULL);
   switch(_iType) {
   case CPIT_HEADING:  SetItemHeading    (NULL);                   break;
   case CPIT_TEXT:     SetItemText       ("");                     break;
   case CPIT_VALUE:    SetItemValue      (0.00);                   break;
   case CPIT_DBLVAL:   SetItemDblValue   (0.00, 0.00);             break;
   case CPIT_SAGTAN:   SetItemSagTanValue(0.00, 0.00);             break;
   case CPIT_QUADVAL:  SetItemQuadValue  (0.00, 0.00, 0.00, 0.00); break;
   case CPIT_EQUATION: SetItemEquation   (NULL);                   break;
   case CPIT_CHECKBOX: SetItemCheckBox   (FALSE);                  break;
   case CPIT_TOGGLE:   SetItemToggle     (NULL);                   break;
   case CPIT_SLIDER:   SetItemSlider     (0.00, 0.00, 0.00);       break;
   case CPIT_HMENU:    SetItemMenu       (NULL);                   break;
   case CPIT_RADIOLIST:SetItemRadioList  (0, "\0");                break;
   case CPIT_DROPLIST: SetItemDropList   (0, "\0");                break;
   case CPIT_COMMAND:  SetItemCommand    ("");                     break;
   }//iType
   UpdateActiveCursors();                   // set correct cursor handles

}

/**********************************************************
* Destructor
**********************************************************/
CPropItem::~CPropItem() {
   if(pcActive) pMgrParent->DeleteActive(pcActive); pcActive = NULL;
   if(pszLabel) free(pszLabel); pszLabel = NULL; // free label text
   FreeItem();                              // free text, if any
}


/*********************************************************
* FreeItem
* Frees the text buffer, if it's been allocated; this can
* also be extended to other things  that the item handles
* itself
*********************************************************/
void CPropItem::FreeItem(void) {
   switch(Type()) {
   case CPIT_HEADING:
   case CPIT_TEXT:
   case CPIT_TOGGLE:
   case CPIT_COMMAND:
      if(xData.pszText) free(xData.pszText); xData.pszText = NULL;
      break;
   case CPIT_RADIOLIST:
   case CPIT_DROPLIST:
      if(xData.pszList) free(xData.pszList); xData.pszList = NULL;
      break;
   }
}


/*********************************************************
* Set Item Type Functions
*********************************************************/
//===SetItemHeading=======================================
void CPropItem::SetItemHeading(const char *_pszHeadingAux) {
   FreeItem();                              // free any allocated resources
   SetType(CPIT_HEADING);                   // set type
   if(_pszHeadingAux==NULL) return;         // ignore further if no heading
   xData.pszText = (char*) malloc((strlen(_pszHeadingAux)+1)*sizeof(char));
   if(xData.pszText) strcpy(xData.pszText, _pszHeadingAux);
}
//===SetItemText==========================================
void CPropItem::SetItemText(const char *_pszText) {
   FreeItem();                              // free any allocated resources
   SetType(CPIT_TEXT);                      // set type
   if(_pszText==NULL) return;               // ignore further if no text
   xData.pszText = (char*) malloc((strlen(_pszText)+1)*sizeof(char));
   if(xData.pszText) strcpy(xData.pszText, _pszText);
}
//===SetItemValue=========================================
void CPropItem::SetItemValue(double _dValue) {
   FreeItem();                              // free any allocated resources
   SetType(CPIT_VALUE);                     // set type
   xData.dVal = _dValue;
}
//===SetItemDblValue======================================
void CPropItem::SetItemDblValue(double _dVal1, double _dVal2) {
   FreeItem();                              // free any allocated resources
   SetType(CPIT_DBLVAL);                    // set type
   xData.dVal1 = _dVal1; xData.dVal2 = _dVal2;
}
//===SetItemSagTanValue===================================
void CPropItem::SetItemSagTanValue(double _dSag, double _dTan) {
   FreeItem();                              // free any allocated resources
   SetType(CPIT_SAGTAN);                    // set type
   xData.dVal1 = _dSag; xData.dVal2 = _dTan;
}
//===SetItemQuadValue=====================================
void CPropItem::SetItemQuadValue(double _dVal1, double _dVal2, double _dVal3, double _dVal4) {
   FreeItem();                              // free any allocated resources
   SetType(CPIT_QUADVAL);                   // set type
   xData.dVal1 = _dVal1; xData.dVal2 = _dVal2;
   xData.dVal3 = _dVal3; xData.dVal4 = _dVal4;
}
//===SetItemEquation======================================
void CPropItem::SetItemEquation(const CEquation *_pEqtn) {
   FreeItem();                              // free any allocated resources
   SetType(CPIT_EQUATION);                  // set type
   xData.pEqtn = (CEquation*) _pEqtn;
}
//===SetItemCheckBox======================================
void CPropItem::SetItemCheckBox(BOOL _tfBool) {
   FreeItem();                              // free any allocated resources
   SetType(CPIT_CHECKBOX);                  // set type
   xData.tfBool = _tfBool;
}
//===SetItemToggle========================================
void CPropItem::SetItemToggle(const char *_pszText) {
   FreeItem();                              // free any allocated resources
   SetType(CPIT_TOGGLE);                    // set type
   if(_pszText==NULL) return;               // ignore further if no text
   xData.pszText = (char*) malloc((strlen(_pszText)+1)*sizeof(char));
   if(xData.pszText) strcpy(xData.pszText, _pszText);
}
//===SetItemSlider========================================
void CPropItem::SetItemSlider(double _dCur, double _dMin, double _dMax) {
   FreeItem();                              // free any allocated resources
   SetType(CPIT_SLIDER);                    // set type
   xData.dCur = _dCur;
   xData.dMin = _dMin;
   xData.dMax = _dMax;
   SetPosition(NULL);                       // update the thumb active position
}
//===SetItemMenu==========================================
void CPropItem::SetItemMenu(HMENU _hMenu) {
   FreeItem();                              // free any allocated resources
   SetType(CPIT_HMENU);                     // set type
   xData.hMenu = _hMenu;
}
//===SetItemRadioList=====================================
// uses \0 as delimiter!
void CPropItem::SetItemRadioList(int _iCur, const char *_pszList) {
   char *psz;                               // pointer for loop counter
   int   k;                                 // loop counter
   int   iLen;                              // length of string
   FreeItem();                              // free allocated resources
   SetType(CPIT_RADIOLIST);                 // set type
   if(_pszList==NULL) return;               // ignore further if no text
   for(xData.iNum=0, iLen=0, psz=(char*)_pszList; psz[0]!=NULL; xData.iNum++, iLen+=strlen(psz)+1, psz+=strlen(psz)+1);
   xData.pszList = (char*) malloc((iLen+1)*sizeof(char));
   if(xData.pszList) memcpy(xData.pszList, _pszList, (iLen+1)*sizeof(char)); // copy whole list of strings
   xData.iCur = _iCur;
   for(k=0, xData.pszCur=xData.pszList; (k<xData.iCur)&&(xData.pszCur[0]); k++) xData.pszCur += strlen(xData.pszCur) + 1;
}
//===SetItemDropList======================================
// Uses \0 as delimiter!
void CPropItem::SetItemDropList(int _iCur, const char *_pszList) {
   char *psz;                               // pointer for loop counter
   int   k;                                 // loop counter
   int   iLen;                              // length of string
   FreeItem();                              // free allocated resources
   SetType(CPIT_DROPLIST);                  // set type
   if(_pszList==NULL) return;               // ignore further if no text
   for(xData.iNum=0, iLen=0, psz=(char*)_pszList; psz[0]!=NULL; xData.iNum++, iLen+=strlen(psz)+1, psz+=strlen(psz)+1);
   xData.pszList = (char*) malloc((iLen+1)*sizeof(char));
   if(xData.pszList) memcpy(xData.pszList, _pszList, (iLen+1)*sizeof(char)); // copy whole list of strings
   xData.iCur = _iCur;
   for(k=0, xData.pszCur=xData.pszList; (k<xData.iCur)&&(xData.pszCur[0]); k++) xData.pszCur += strlen(xData.pszCur) + 1;
}
//===SetItemCommand=======================================
void CPropItem::SetItemCommand(const char *_pszText) {
   FreeItem();                              // free any allocated resources
   SetType(CPIT_COMMAND);                   // set type
   if(_pszText==NULL) return;               // ignore further if no text
   xData.pszText = (char*) malloc((strlen(_pszText)+1)*sizeof(char));
   if(xData.pszText) strcpy(xData.pszText, _pszText);
}

//===SetSagTanStable======================================
void CPropItem::SetItemNaN(BOOL tfNan1, BOOL tfNan2) {
   if(tfNan1) SetBit(CPIF_NAN1); else ClearBit(CPIF_NAN1);
   if(tfNan2) SetBit(CPIF_NAN2); else ClearBit(CPIF_NAN2);
}


/*********************************************************
* SetItemCallback
* The callback function gives vData depending on the type
* of item, and passes back the pVoid argument supplied to
* this call so that the application may identify the item
* that is  returning the callback. The callback  function
* returns FALSE if the edit control should remain visible
* for those types that are Quick-Edit inputs.
*
*   Item Type     vData Cast     Information
*  -----------------------------------------------
*   Heading        NULL         (No callback?)
*   Text          (const char*)  String
*   Value         (double*)      Value
*   DblValue      (double[2])    Two values
*   SagTanValue   (double[2])    Two values
*   QuadValue     (double[4])    Four values
*   Equation      (const char*)  Equation string
*   CheckBox      (BOOL*)        Previous check state
*   Toggle         NULL
*   Slider        (double*)      Slider value
*   Menu           ???           ???
*   RadioList     (int*)         Selection
*   DropList      (int*)         Selection
*   Command        NULL
* The callback is called from MouseCallback.
*********************************************************/
void CPropItem::SetItemCallback(PROPITEMCALLBACK lpfnIn, void *pVoidIn) {
   pVoid            = pVoidIn;
   lpfnItemCallback = lpfnIn;
}

/*********************************************************
* SetLabel
*********************************************************/
void CPropItem::SetLabel(const char *pszLbl) {
   if(pszLabel) free(pszLabel); pszLabel = NULL;
   pszLabel = (char*) malloc((strlen(pszLbl) + 1) * sizeof(char));
   if(pszLabel) strcpy(pszLabel, pszLbl);
}


/*********************************************************
* SetPosition
* Sets the screen position and updates the active rectan-
* gles. If a new rectangle is  not given, uses the previ-
* ous, but updates the active anyway.
* Called from
*    <- CPropMgr::PositionItems
*    <- SetItemSlider (because change in value moves thumb)
*********************************************************/
void CPropItem::SetPosition(LPRECT prc) {
   int iCol;                                // temp value for rectangle
   //===Rectangle========================================
   if(prc!=NULL) CopyRect(&rcItem, prc);    // set new item rectangle

   //---Hidden / collapsed----------------------
   if((CheckBit(CPIF_COLLAPSED) || CheckBit(CPIF_HIDDEN)) && (Type()!=CPIT_HEADING)) {
      pcActive->SetActiveRect(0, 0, 0, 0);
      return;
   }

   //===Update active=====================================
   if(pcActive) {
      iCol = rcItem.left + pMgrParent->ColumnWidth() + 4; // space for vertical divide active
      switch(Type()) {
      case CPIT_HEADING:
      case CPIT_COMMAND:
         pcActive->SetActiveRect(rcItem.left, rcItem.top, rcItem.right, rcItem.bottom);
         break;
      case CPIT_VALUE:
      case CPIT_DBLVAL:
      case CPIT_SAGTAN:
      case CPIT_QUADVAL:
      case CPIT_EQUATION:
      case CPIT_TOGGLE:
      case CPIT_TEXT:
      case CPIT_DROPLIST:
         pcActive->SetActiveRect(iCol, rcItem.top, rcItem.right, rcItem.bottom);
         break;
      case CPIT_CHECKBOX:
         pcActive->SetActiveRect(iCol, rcItem.top, iCol+(rcItem.bottom-rcItem.top), rcItem.bottom);
         break;
      case CPIT_SLIDER:
         pcActive->SetActiveRect(SliderToScreen()-CPIC_SLIDERWIDTH,
            (rcItem.top + rcItem.bottom) / 2 - CPIC_SLIDERHEIGHT,
            SliderToScreen()+CPIC_SLIDERWIDTH,
            (rcItem.top + rcItem.bottom) / 2 + CPIC_SLIDERHEIGHT);
         break;
      case CPIT_HMENU:
         pcActive->SetActiveRect(rcItem.right-8, rcItem.top, rcItem.right, rcItem.bottom);
         break;
      case CPIT_RADIOLIST:
         pcActive->SetActiveRect(iCol, rcItem.top, rcItem.right, rcItem.bottom);
         break;
      }//iType
   }
}


/*********************************************************
* GetText
* Formats the  data to the  provided buffer  depending on
* the type of data is stored
*********************************************************/
void CPropItem::GetText(char *pszOut, size_t len) {
   memset(pszOut, 0x00, len*sizeof(char));     // clear the whole string
   len--;                                   // keep one char for \0

   //===Blank=============================================
    if(CheckBit(CPIF_MULTI)) {
       _snprintf(pszOut, len-1, "...");
       return;
    }

   //===Formatting========================================
   switch(Type()) {
   case CPIT_HEADING:
      pszOut[0] = '\0';                     // no text yet
      if(CheckBit(CPIF_COLLAPSED)) sprintf(pszOut+strlen(pszOut), "+ ");
      _snprintf(pszOut+strlen(pszOut), len-strlen(pszOut), pszLabel, xData.pszText ? xData.pszText : "");
      break;
   case CPIT_TEXT:
   case CPIT_TOGGLE:
      _snprintf(pszOut, len, "%s", xData.pszText ? xData.pszText : "");
      break;

   case CPIT_VALUE:
      if(CheckBit(CPIF_NAN1)) _snprintf(pszOut, len, "---");
      else  _snprintf(pszOut, len, "%lg", xData.dVal);
      break;

   case CPIT_DBLVAL:                        // editable, so use ','
      if(CheckBit(CPIF_NAN1)) _snprintf(pszOut, len, "---, ");
      else  _snprintf(pszOut, len, "%0.3lg, ", xData.dVal1);
      if(CheckBit(CPIF_NAN2)) _snprintf(pszOut+strlen(pszOut), len, "---");
      else  _snprintf(pszOut+strlen(pszOut), len, "%0.3lg", xData.dVal2);
      break;

   case CPIT_SAGTAN:
      if(CheckBit(CPIF_NAN1)) _snprintf(pszOut, len, "---~");
      else  _snprintf(pszOut, len, "%0.3lg~", xData.dVal1);
      if(CheckBit(CPIF_NAN2)) _snprintf(pszOut+strlen(pszOut), len, "---");
      else  _snprintf(pszOut+strlen(pszOut), len, "%0.3lg", xData.dVal2);
      break;

   case CPIT_QUADVAL:
      _snprintf(pszOut, len, "%0.3lg~%0.3lg~%0.3lg~%0.3lg",
         xData.dVal1, xData.dVal2, xData.dVal3, xData.dVal4);
      break;

   case CPIT_EQUATION:
      if(xData.pEqtn==NULL) {
         sprintf(pszOut, pMgrParent->CheckBit(CPMF_EQTNSRC) ? "= ..." : "..."); break;
      } else {
         if(pMgrParent->CheckBit(CPMF_EQTNSRC)) {
            sprintf(pszOut, "= ");
            xData.pEqtn->GetEquationString(pszOut+strlen(pszOut), len-strlen(pszOut));
         } else {
            sprintf(pszOut, "%lg", xData.pEqtn->Answer(App()->Vars()));
         }
      }
      break;

   case CPIT_HMENU:    _snprintf(pszOut, len, "CPI@253:HMenu"); break;
   case CPIT_DROPLIST:
      _snprintf(pszOut, len, xData.pszCur); // uses \0 rather than '~' as delimiter
      break;

   case CPIT_COMMAND: _snprintf(pszOut, len, pszLabel, xData.pszText ? xData.pszText : ""); break;
   }//iType
}


/*########################################################
 ## Specific Functions                                 ##
########################################################*/

/*********************************************************
* CollapseGroup
*********************************************************/
void CPropItem::CollapseGroup(BOOL tfCollapse) {
   CPropItem *pItem;

   if(tfCollapse) SetBit(CPIF_COLLAPSED); else ClearBit(CPIF_COLLAPSED);
   for(pItem=(CPropItem*) Next(); (pItem) && (pItem->Type()!=CPIT_HEADING); pItem=(CPropItem*) pItem->Next()) {
      if(tfCollapse) pItem->SetBit(CPIF_COLLAPSED); else pItem->ClearBit(CPIF_COLLAPSED);
   }
   pMgrParent->OnResize(); // need to update the column active position, too
   InvalidateRect(pMgrParent->Window(), NULL, TRUE);
}


/*********************************************************
* Slider function
*********************************************************/
int CPropItem::SliderToScreen(void) {
   return(
      (xData.dCur-xData.dMin)
      * (rcItem.right-rcItem.left-pMgrParent->ColumnWidth()-2*CPIC_SLIDERSPACE)
      / (xData.dMax-xData.dMin+(((xData.dMax-xData.dMin)==0.00) ? 1.00 : 0.00))
      + rcItem.left+pMgrParent->ColumnWidth()+CPIC_SLIDERSPACE);
}
double CPropItem::ScreenToSlider(int x) {
   return(
      (double) (x - rcItem.left-pMgrParent->ColumnWidth()-CPIC_SLIDERSPACE)
      * (xData.dMax-xData.dMin)
      / (double) (rcItem.right-rcItem.left-pMgrParent->ColumnWidth()-2*CPIC_SLIDERSPACE
         + (((rcItem.right-rcItem.left-pMgrParent->ColumnWidth()-2*CPIC_SLIDERSPACE)==0) ? 1 : 0))
      + xData.dMin
      );
}


/*########################################################
 ## GDI                                                ##
########################################################*/
/*********************************************************
* Height
* Returns the  height of the item. This is  new in rev 5.
* The goal is to provide space for an ABCD matrix display
* that occupies two standard rows. Looking at the old rev
* 4 code, it seems this should be pretty trivial.
*********************************************************/
int CPropItem::Height(void) {
   if(CheckBit(CPIF_HIDDEN)) return(0);     // hidden overrides all

   switch(Type()) {
   case CPIT_HEADING:   return(CPIC_ROWHEIGHT);
   case CPIT_QUADVAL:   return(CheckBit(CPIF_COLLAPSED) ? 0 : 2*CPIC_ROWHEIGHT);
   case CPIT_RADIOLIST: return(CheckBit(CPIF_COLLAPSED) ? 0 : xData.iNum * CPIC_ROWHEIGHT);
   default: return(CheckBit(CPIF_COLLAPSED) ? 0 : CPIC_ROWHEIGHT);
   }
}


/*********************************************************
* PaintItem
* Paint the current item.
*********************************************************/
#define fnLeftTextOut(hdc, prc, szBuf, len) {\
   ExtTextOut(hdc, (prc)->left+4, (prc)->top+1, ETO_CLIPPED | ETO_OPAQUE,\
   prc, szBuf, len, NULL); }
void CPropItem::PaintItem(HDC hdc, BOOL tfTextOnly) {
   char     szBuf[256];                     // formatted text buffer
   char    *psz;                            // temporary pointer
   RECT     rc;                             // rectangle into which to paint
   int      iMid;                           // midpoint
   COLORREF rgbOld;                         // restore colors
   HPEN     hpOld;                          // restore pen
   HBRUSH   hbrOld;                         // restore brush
   SIZE     sz;                             // size for GetTextExtent
   int      k;                              // loop counter


   if(rcItem.bottom < 0) return;            // ignore if above display area
///TODO: if(rcItem.top > ???) return; // ignore if below display area
   if(rcItem.bottom-rcItem.top==0) return;  // ignore zero-height

   iMid = rcItem.left + pMgrParent->ColumnWidth(); // middle of value field

   //===Paint Label=======================================
   if(!tfTextOnly) {
      //---Prepare------------------------------
      switch(Type()) {
      case CPIT_HEADING:                    // heading: text is formatted into heading string %s position
         GetText(szBuf, sizeof(szBuf)/sizeof(char));
         SetRect(&rc, rcItem.left, rcItem.top+1, rcItem.right, rcItem.bottom-1);
         rgbOld = SetBkColor(hdc, pMgrParent->Rgb(CAPPI_RGB3DFACE));
         break;
      case CPIT_COMMAND:
         rgbOld = SetTextColor(hdc, pMgrParent->Rgb(CAPPI_RGBCMD));
         GetText(szBuf, sizeof(szBuf)/sizeof(char));
         SetRect(&rc, rcItem.left, rcItem.top, rcItem.right, rcItem.bottom-1);
         break;
      default:
         sprintf(szBuf, pszLabel);
         SetRect(&rc, rcItem.left, rcItem.top, iMid, rcItem.bottom-1);
         break;
      }//iType

      //---Text---------------------------------
      fnLeftTextOut(hdc, &rc, szBuf, strlen(szBuf));

      //---Cosmetics----------------------------
      switch(Type()) {
      case CPIT_HEADING:
         SetBkColor(hdc, rgbOld);
         hpOld = (HPEN) SelectObject(hdc, pMgrParent->Pen(CAPPI_PEN3DLITE));
         MoveTo(hdc, rcItem.left, rcItem.top);
         LineTo(hdc, rcItem.right, rcItem.top);
         SelectObject(hdc, pMgrParent->Pen(CAPPI_PEN3DDARK));
         MoveTo(hdc, rcItem.left, rcItem.bottom-1);
         LineTo(hdc, rcItem.right, rcItem.bottom-1);
         SelectObject(hdc, hpOld);
         break;
      case CPIT_COMMAND:
         SetTextColor(hdc, rgbOld);
         hpOld = (HPEN) SelectObject(hdc, pMgrParent->Pen(CAPPI_PENCMD));
         GetTextExtentPoint32(hdc, szBuf, strlen(szBuf), &sz);
         MoveTo(hdc, rcItem.left+4,       rcItem.bottom-4);
         LineTo(hdc, rcItem.left+4+sz.cx, rcItem.bottom-4);
         SelectObject(hdc, pMgrParent->Pen(CAPPI_PEN3DFACE));
         MoveTo(hdc, rcItem.left, rcItem.bottom-1);  // horizontal line..
         LineTo(hdc, rcItem.right, rcItem.bottom-1); //..below item
         SelectObject(hdc, hpOld);
         break;
      default:
         hpOld = (HPEN) SelectObject(hdc, pMgrParent->Pen(CAPPI_PEN3DFACE));
         MoveTo(hdc, rcItem.left, rcItem.bottom-1);  // horizontal line..
         LineTo(hdc, rcItem.right, rcItem.bottom-1); //..below item
         MoveTo(hdc, rcItem.left+pMgrParent->ColumnWidth()-1, rcItem.top+2); // vertical divide
         LineTo(hdc, rcItem.left+pMgrParent->ColumnWidth()-1, rcItem.bottom-1);
         SelectObject(hdc, hpOld);
      }
   }

   //===Paint Item========================================
   GetText(szBuf, sizeof(szBuf) / sizeof(char));

   switch(Type()) {
   case CPIT_HEADING: break;

   case CPIT_SAGTAN:
      psz = strchr(szBuf, '~') + 1;
      rgbOld = SetTextColor(hdc, pMgrParent->Rgb(CAPPI_RGBSAG));
      SetRect(&rc, iMid, rcItem.top, (iMid+rcItem.right)/2, rcItem.bottom-1);
      fnLeftTextOut(hdc, &rc, szBuf, psz - szBuf - 1);
      SetTextColor(hdc, pMgrParent->Rgb(CAPPI_RGBTAN));
      SetRect(&rc, (iMid+rcItem.right)/2, rcItem.top, rcItem.right, rcItem.bottom-1);
      fnLeftTextOut(hdc, &rc, psz, strlen(psz));
      SetTextColor(hdc, rgbOld);
      break;

   case CPIT_QUADVAL:
      psz = szBuf;
      SetRect(&rc, iMid, rcItem.top, (iMid+rcItem.right)/2, (rcItem.top+rcItem.bottom)/2);
      fnLeftTextOut(hdc, &rc, psz, strchr(psz, '~')-psz);
      psz = strchr(psz, '~') + 1;
      SetRect(&rc, (iMid+rcItem.right)/2, rcItem.top, rcItem.right, (rcItem.top+rcItem.bottom)/2);
      fnLeftTextOut(hdc, &rc, psz, strchr(psz, '~')-psz);
      psz = strchr(psz, '~') + 1;
      SetRect(&rc, iMid, (rcItem.top+rcItem.bottom)/2, (iMid+rcItem.right)/2, rcItem.bottom-1);
      fnLeftTextOut(hdc, &rc, psz, strchr(psz, '~')-psz);
      psz = strchr(psz, '~') + 1;
      SetRect(&rc, (iMid+rcItem.right)/2, (rcItem.top+rcItem.bottom)/2, rcItem.right, rcItem.bottom-1);
      fnLeftTextOut(hdc, &rc, psz, strlen(psz));
      break;

   case CPIT_EQUATION:
      rgbOld = SetTextColor(hdc, pMgrParent->Rgb(CheckBit(CPIF_READONLY) ? CAPPI_RGB3DGRAY : CAPPI_RGBEQTN));
      SetRect(&rc, iMid, rcItem.top, rcItem.right, rcItem.bottom-1);
      fnLeftTextOut(hdc, &rc, szBuf, strlen(szBuf));
      SetTextColor(hdc, rgbOld);
      break;

   case CPIT_SLIDER:
      hpOld = (HPEN) SelectObject(hdc, pMgrParent->Pen(CAPPI_PEN3DDARK));
      SetRect(&rc, iMid, rcItem.top, rcItem.right, rcItem.bottom-1);
      FillRect(hdc, &rc, pMgrParent->Brush(CAPPI_BRUSHWINDOW));

      SetRect(&rc, iMid+CPIC_SLIDERSPACE, (rcItem.top+rc.bottom)/2-1,
         rcItem.right-CPIC_SLIDERSPACE, (rcItem.top+rcItem.bottom)/2+1);
      FillRect(hdc, &rc, pMgrParent->Brush(CAPPI_BRUSH3DFACE));
      SelectObject(hdc, pMgrParent->Pen(CAPPI_PEN3DDARK));
      MoveTo(hdc, rc.left+1, rc.bottom);
      LineTo(hdc, rc.right, rc.bottom);
      LineTo(hdc, rc.right, rc.top-2);
      SelectObject(hdc, pMgrParent->Pen(CAPPI_PEN3DLITE));
      MoveTo(hdc, rc.left, rc.bottom);
      LineTo(hdc, rc.left, rc.top-1);
      LineTo(hdc, rc.right, rc.top-1);

      SetRect(&rc, SliderToScreen() - CPIC_SLIDERWIDTH,
         (rcItem.top + rcItem.bottom) / 2 - CPIC_SLIDERHEIGHT,
         SliderToScreen() + CPIC_SLIDERWIDTH,
         (rcItem.top + rcItem.bottom) / 2 + CPIC_SLIDERHEIGHT);
      FillRect(hdc, &rc, pMgrParent->Brush(CAPPI_BRUSH3DFACE));
      SelectObject(hdc, pMgrParent->Pen(CAPPI_PEN3DDARK));
      MoveTo(hdc, rc.left+1, rc.bottom);
      LineTo(hdc, rc.right, rc.bottom);
      LineTo(hdc, rc.right, rc.top-2);
      SelectObject(hdc, pMgrParent->Pen(CAPPI_PEN3DLITE));
      MoveTo(hdc, rc.left, rc.bottom);
      LineTo(hdc, rc.left, rc.top-1);
      LineTo(hdc, rc.right, rc.top-1);
      SelectObject(hdc, hpOld);
      break;

   case CPIT_CHECKBOX:
      hpOld = (HPEN) SelectObject(hdc, pMgrParent->Pen(CAPPI_PEN3DTEXT));
      SetRect(&rc, iMid, rcItem.top, rcItem.right, rcItem.bottom-1);
      FillRect(hdc, &rc, pMgrParent->Brush(CAPPI_BRUSHWINDOW));
      SetRect(&rc, iMid+3, rcItem.top+3, iMid+rcItem.bottom-rcItem.top-3, rcItem.bottom-3);
      FillRect(hdc, &rc, pMgrParent->Brush(CAPPI_BRUSH3DFACE));
      MoveTo(hdc, rc.left, rc.top);
      LineTo(hdc, rc.right, rc.top);
      LineTo(hdc, rc.right, rc.bottom);
      LineTo(hdc, rc.left, rc.bottom);
      LineTo(hdc, rc.left, rc.top);
      SelectObject(hdc, pMgrParent->Pen(CAPPI_PEN3DLITE));
      MoveTo(hdc, rc.left+1, rc.bottom);
      LineTo(hdc, rc.right, rc.bottom);
      LineTo(hdc, rc.right, rc.top-1);
      SelectObject(hdc, pMgrParent->Pen(CAPPI_PEN3DDARK));
      MoveTo(hdc, rc.left, rc.bottom);
      LineTo(hdc, rc.left, rc.top);
      LineTo(hdc, rc.right, rc.top);

      if(xData.tfBool) { // recall: last pixel isn't painted!
         SelectObject(hdc, pMgrParent->Pen(CAPPI_PEN3DTEXT));
         MoveTo(hdc, rc.left+3, rc.top+2); LineTo(hdc, rc.right-1, rc.bottom-2);
         MoveTo(hdc, rc.left+2, rc.top+2); LineTo(hdc, rc.right-1, rc.bottom-1);
         MoveTo(hdc, rc.left+2, rc.top+3); LineTo(hdc, rc.right-2, rc.bottom-1);
         MoveTo(hdc, rc.left+2, rc.bottom-3); LineTo(hdc, rc.right-2, rc.top+1);
         MoveTo(hdc, rc.left+2, rc.bottom-2); LineTo(hdc, rc.right-1, rc.top+1);
         MoveTo(hdc, rc.left+3, rc.bottom-2); LineTo(hdc, rc.right-1, rc.top+2);
      }
      SelectObject(hdc, hpOld);
      break;

   case CPIT_RADIOLIST:
      rgbOld = SetTextColor(hdc, pMgrParent->Rgb(CheckBit(CPIF_READONLY) ? CAPPI_RGB3DGRAY : CAPPI_RGB3DTEXT));
      SetRect(&rc, iMid, rcItem.top, iMid+CPIC_ROWHEIGHT, rcItem.bottom-1);
      FillRect(hdc, &rc, pMgrParent->Brush(CAPPI_BRUSHWINDOW)); // background for buttons
      psz=xData.pszList;
      for(k=0; k<xData.iNum; k++) {
         SetRect(&rc, iMid, rcItem.top+k*CPIC_ROWHEIGHT, iMid+CPIC_ROWHEIGHT-1, rcItem.top+(k+1)*CPIC_ROWHEIGHT-1);
         SelectObject(hdc, pMgrParent->Pen(CAPPI_PEN3DDARK)); // start buttons here
         Ellipse(hdc, rc.left+2, rc.top+2, rc.right-3, rc.bottom-3);
         SelectObject(hdc, pMgrParent->Pen(CAPPI_PEN3DLITE));
         Ellipse(hdc, rc.left+3, rc.top+3, rc.right-2, rc.bottom-2);
         hbrOld = (HBRUSH) SelectObject(hdc, GetStockObject(WHITE_BRUSH)); ///TODO: Don't use system GDI here
         SelectObject(hdc, GetStockObject(WHITE_PEN));
         Ellipse(hdc, rc.left+3, rc.top+3, rc.right-3, rc.bottom-3);
         if((k==xData.iCur) && !CheckBit(CPIF_READONLY)) {
            SelectObject(hdc, GetStockObject(BLACK_BRUSH));
            SelectObject(hdc, GetStockObject(BLACK_PEN));
            //Ellipse(hdc, rc.left+5, rc.top+5, rc.right-5, rc.bottom-5);
            Ellipse(hdc, rc.left+6, rc.top+6, rc.right-6, rc.bottom-6);
         }
         SelectObject(hdc, hbrOld);
         SetRect(&rc, iMid+CPIC_ROWHEIGHT, rcItem.top+k*CPIC_ROWHEIGHT, rcItem.right, rcItem.top+(k+1)*CPIC_ROWHEIGHT - ((k<xData.iNum-1) ? 0 : 1));
         fnLeftTextOut(hdc, &rc, psz, strlen(psz));
         psz = psz + strlen(psz)+1;
      }
      SetTextColor(hdc, rgbOld);
      break;
   case CPIT_COMMAND: break;
   case CPIT_DROPLIST:
   case CPIT_TOGGLE:
   case CPIT_HMENU:
   default:
      rgbOld = SetTextColor(hdc, pMgrParent->Rgb(CheckBit(CPIF_READONLY) ? CAPPI_RGB3DGRAY : CAPPI_RGB3DTEXT));
      SetRect(&rc, iMid, rcItem.top, rcItem.right, rcItem.bottom-1);
      fnLeftTextOut(hdc, &rc, szBuf, strlen(szBuf));
      SetTextColor(hdc, rgbOld);
      break;
   }//iType

}

/*########################################################
 ## Mouse                                              ##
########################################################*/
/*********************************************************
* UpdateActiveCursors
* This function need  only be called if  the type of item
* is changed. (Even in those  cases, it's probably better
* delete the item and recreate it.) So this will probably
* only be called when the item is first established.
* It's possible that  this may need  to be  called if the
* item changes its read-only status.
*********************************************************/
void CPropItem::UpdateActiveCursors(void) {
   if(pcActive == NULL) return;             // ignore if no item

   //---Disabled----------------------
   if(CheckBit(CPIF_READONLY)) {
      pcActive->SetCursors(NULL, NULL, NULL, NULL);

   //---Enabled-----------------------
   } else {
      switch(Type()) {
      case CPIT_HEADING:  pcActive->SetCursors(NULL,                         NULL, NULL, NULL); break;
      case CPIT_TEXT:     pcActive->SetCursors(LoadCursor(NULL, IDC_IBEAM),  NULL, NULL, NULL); break;
      case CPIT_VALUE:    pcActive->SetCursors(LoadCursor(NULL, IDC_IBEAM),  NULL, NULL, NULL); break;
      case CPIT_DBLVAL:   pcActive->SetCursors(LoadCursor(NULL, IDC_IBEAM),  NULL, NULL, NULL); break;
      case CPIT_SAGTAN:   pcActive->SetCursors(NULL,                         NULL, NULL, NULL); break;
      case CPIT_QUADVAL:  pcActive->SetCursors(NULL,                         NULL, NULL, NULL); break;
      case CPIT_EQUATION: pcActive->SetCursors(LoadCursor(NULL, IDC_IBEAM),  NULL, NULL, NULL); break;
      case CPIT_CHECKBOX: pcActive->SetCursors(NULL,                         NULL, NULL, NULL); break;
      case CPIT_TOGGLE:   pcActive->SetCursors(LoadCursor(NULL, IDC_HAND),   NULL, NULL, NULL); break;
      case CPIT_SLIDER:   pcActive->SetCursors(LoadCursor(NULL, IDC_SIZEWE), NULL, NULL, NULL); break;
      case CPIT_HMENU:    pcActive->SetCursors(NULL,                         NULL, NULL, NULL); break;
      case CPIT_RADIOLIST:pcActive->SetCursors(LoadCursor(NULL, IDC_HAND)   ,NULL, NULL, NULL); break;
      case CPIT_DROPLIST: pcActive->SetCursors(NULL                         ,NULL, NULL, NULL); break;
      case CPIT_COMMAND:  pcActive->SetCursors(LoadCursor(NULL, IDC_HAND)   ,NULL, NULL, NULL); break;
      }//iType
   }
}


/*********************************************************
* MouseCallback
* This is  called by the  property manager's  mouse layer
* for property items. A pointer  to the item is stored in
* the pVoid data slot.
* This callback function must be declared static.
*********************************************************/
void CPropItem::_MouseCallback(int iMsg, int x, int y, int wKeys, void *pVoid, long int lData) {
UNREFERENCED_PARAMETER(lData);
   ((CPropItem*) pVoid)->MouseCallback(iMsg, x, y, wKeys); // call item
}

/*********************************************************
* MouseCallback - object version
*********************************************************/
void CPropItem::MouseCallback(int iMsg, int x, int y, int wKeys) {
UNREFERENCED_PARAMETER(y);
UNREFERENCED_PARAMETER(wKeys);
   CQuickEdit *pQEdit;                      // property manager's quick edit
   CQuickMenu *pQMenu;                      // property manager's quick dropdown list
   char        szBuf[256];                  // string buffer
   RECT        rc;                          // popup menu position rectangle
   int         k;                           // loop counter
   char       *psz;                         // temporary pointer

   if(CheckBit(CPIF_READONLY)) return;      // ignore read-only

   switch(Type()) {
   case CPIT_HEADING:
      switch(iMsg) {
      case ACSM_LEFT: CollapseGroup(CheckBit(CPIF_COLLAPSED) ? FALSE : TRUE); break;
      }
      break;

   case CPIT_TEXT:
   case CPIT_EQUATION:
   case CPIT_VALUE:
   case CPIT_DBLVAL:
      switch(iMsg) {
      case ACSM_LEFT:
         pQEdit = pMgrParent->QEdit();      // get parent's quick edit
         if(pQEdit==NULL) break;            // ignore if not available
         pQEdit->SetUserData((void*) this, 0L); // point to this item
         pQEdit->SetCallback(CPropItem::_QEditCallback); // callback to this item class
         pQEdit->SetPositionRect(pcActive->ActiveRect()); // define position
         switch(Type()) {                   // set source text
         case CPIT_TEXT:
            sprintf(szBuf, xData.pszText);
            break;
         case CPIT_EQUATION:
            if(xData.pEqtn) xData.pEqtn->GetEquationString(szBuf, sizeof(szBuf)/sizeof(char));
            else sprintf(szBuf, "");
            break;
         default: GetText(szBuf, sizeof(szBuf)/sizeof(char)); break;
         }
         pQEdit->SetString(szBuf);
         pQEdit->Show();                    // display the window
      }
      break;

   case CPIT_SLIDER:
      switch(iMsg) {
      case ACSM_DRAG:
      case ACSM_DEND:
         xData.dCur = ScreenToSlider(x);
         if(xData.dCur < xData.dMin) xData.dCur = xData.dMin;
         if(xData.dCur > xData.dMax) xData.dCur = xData.dMax;
         if(lpfnItemCallback) lpfnItemCallback((void*)&xData.dCur, uID, pVoid);
         if(iMsg==ACSM_DEND) SetPosition(NULL); // update active position
         pMgrParent->OnPaint(TRUE);         // update values only
         break;
      }
      break;

   case CPIT_CHECKBOX:
      switch(iMsg) {
      case ACSM_LEFT:
         if(lpfnItemCallback) lpfnItemCallback((void*)&xData.tfBool, uID, pVoid);
         else {
            xData.tfBool = (xData.tfBool) ? FALSE : TRUE;
            pMgrParent->OnPaint(TRUE);         // update values only
         }
         break;
      }
      break;

   case CPIT_RADIOLIST:
      switch(iMsg) {
      case ACSM_LEFT:
         xData.iCur = (y - rcItem.top) / CPIC_ROWHEIGHT;
         if(lpfnItemCallback) lpfnItemCallback((void*)&xData.iCur, uID, pVoid);
         else {
            pMgrParent->OnPaint(TRUE);      // update values only
         }
         break;
      }
      break;

   case CPIT_DROPLIST:
      switch(iMsg) {
      case ACSM_LEFT:
         pQMenu = pMgrParent->QMenu();      // get parent's quick control
         if(pQMenu==NULL) break;            // ignore if not available
         pcActive->GetActiveRect(&rc);      // get the rectangle
         rc.bottom += xData.iNum * CPIC_ROWHEIGHT; // extend area
         pQMenu->SetUserData((void*) this, 0L); // point to this item
         pQMenu->SetCallback(CPropItem::_QMenuCallback); // callback to this item class
         pQMenu->SetPositionRect(&rc);      // define position
         pQMenu->SetItems(xData.pszList);
         pQMenu->SetSelection(xData.iCur);  // selection
         pQMenu->Show();                    // display the window
      }
      break;

   case CPIT_TOGGLE:
      switch(iMsg) {
      case ACSM_LEFT: if(lpfnItemCallback) lpfnItemCallback((void*)NULL, uID, pVoid); break;
      }
      break;

   case CPIT_COMMAND:
      switch(iMsg) {
      case ACSM_LEFT:
         if(lpfnItemCallback) lpfnItemCallback((void*)&xData.dCur, uID, pVoid);
         pMgrParent->OnPaint(TRUE);         // update values only
         break;
      }
      break;
   }

}

/*########################################################
 ## Quick Controls                                     ##
########################################################*/
/*********************************************************
* QEditCallback
* This is called by a CQuickEdit (popup edit control) for
* editing items.
* The function should return TRUE if the value is accept-
* able and the control can be hidden, or zero to indicate
* a problem with the value.
* This callback function must be declared static.
*********************************************************/
BOOL CPropItem::_QEditCallback(void *pVal, int iNext, void *pVoid, long int lLong) {
UNREFERENCED_PARAMETER(iNext);
UNREFERENCED_PARAMETER(lLong);
   BOOL tfRet;                              // wrapped return value
   tfRet = ((CPropItem*)pVoid)->QEditCallback(pVal);
   return(tfRet);
}

/*********************************************************
* QEditCallback
* The processing here must match the QEdit callup in the
* mouse procedure
*********************************************************/
BOOL CPropItem::QEditCallback(void *pVal) {
   double dVal[4];                          // scanned value(s)
   CEquation Eq;                            // value-parsing equation
   int    nScan;                            // number scanned
   char   szError[256];                     // error for equation

   switch(Type()) {
   case CPIT_TEXT:
      if(lpfnItemCallback) return(lpfnItemCallback((void*)(char*)pVal, uID, pVoid));
      else SetItemText((char*) pVal);
      break;
   case CPIT_VALUE:
      if(Eq.ParseConstantEquation((char*)pVal, &dVal[0]) != EQERR_NONE) return(FALSE);
      if(lpfnItemCallback) return(lpfnItemCallback((void*)&dVal[0], uID, pVoid));
      else SetItemValue(dVal[0]);
      break;
   case CPIT_DBLVAL:
   case CPIT_SAGTAN:
      if(sscanf((char*)pVal, "%lg,%lg", &dVal[0],&dVal[1]) < 2) return(FALSE);
      if(lpfnItemCallback) {
         if(! lpfnItemCallback((void*)dVal, uID, pVoid)) return(FALSE);
      } else {
         if(Type()==CPIT_DBLVAL) SetItemDblValue(dVal[0],dVal[1]);
         else                    SetItemSagTanValue(dVal[0],dVal[1]);
      }
      break;
   case CPIT_EQUATION:
      if(CEq.ParseEquation((char*)pVal, App()->VarsString()) != EQERR_NONE) {
         CEq.LastErrorMessage(szError, sizeof(szError)/sizeof(char), (char*) pVal);
         pMgrParent->SetAppStatusBarPriorityText(szError);
         return(FALSE);
      } else {
         pMgrParent->SetAppStatusBarPriorityText(NULL);
         if(lpfnItemCallback) {
            if(! lpfnItemCallback((void*)(char*)pVal, uID, pVoid)) return(FALSE);
         }
      }
      break;
   }
   pMgrParent->OnPaint(TRUE);               // update text only
   return(TRUE);                            // default -- assume peachy
}

/*********************************************************
* QMenuCallback
* Called by the popup menu / list item control.
* The function returns TRUE  when the control can be hid-
* den, or FALSE if it must retain the focus.
* This callback function must be declared static.
*********************************************************/
BOOL CPropItem::_QMenuCallback(int iSel, int iNext, void *pVoid, long int lLong) {
UNREFERENCED_PARAMETER(iNext);
UNREFERENCED_PARAMETER(lLong);
   BOOL tfRet;                              // wrapped return value
   tfRet = ((CPropItem*)pVoid)->QMenuCallback(iSel);
   return(tfRet);
}

/*********************************************************
* QMenuCallback
* Called from: <- _QMenuCallback
*********************************************************/
BOOL CPropItem::QMenuCallback(int iSel) {
   int k;                                   // loop counter
   if(lpfnItemCallback) {
      return(lpfnItemCallback((void*)&iSel, uID, pVoid));
   } else {                                 // automatic handling if no callback
      /*xData.iCur = iSel;                    // update selection
      for(k=0, xData.pszCur=xData.pszList; (k<xData.iCur)&&(xData.pszCur[0]); k++)
         xData.pszCur += strlen(xData.pszCur) + 1; // update text */
      return(TRUE);
   }
}



