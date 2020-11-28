/*********************************************************
* CSysWin1d
* System renderer that shows the system mode plotted in a
* straight line. Probably no editing of system here.
* $PSchlup 2006 $     $Revision 0 $
*********************************************************/
#include "CSysWin1d.h"                      // class include

//===Names================================================
const char* CSysWin1d::CszSysWin1dProp[C1DI_NUM_PROP] = { // saved properties
   "XMin", "XMax", "YMin", "YMax", "Flags"};

/*********************************************************
* Constructor
*********************************************************/
CSysWin1d::CSysWin1d(CSystem *pSys) : CSysWin(pSys) {printf("+ CSysWin-1D- Created\n");
   int k;                                   // loop counter

   //===Members===========================================
   for(k=0; k<C1DI_NUM_PROP; k++) ddProp[k] = 0.00;
   for(k=0; k<C1DI_NUM_RECT; k++) SetRect(&rcRect[k], 0, 0, 0, 0);
   pQEdit      = NULL;                      // no quick controls yet
   Axes.SetAxLabel(AX_XAXIS, "Cavity Position (mm)");
   Axes.SetBoxOn();

   //---Mouse-------------------------
   Mouse.CreateActive(0, 0, 0, 0, LoadCursor(NULL, IDC_CROSS ), NULL, NULL, NULL, CSysWin1d::_MouseCallback, (void*) this, C1D_ACT_AXES    , NULL);
   Mouse.CreateActive(0, 0, 0, 0, LoadCursor(NULL, IDC_IBEAM ), NULL, NULL, NULL, CSysWin1d::_MouseCallback, (void*) this, C1D_ACT_XMIN,     NULL);
   Mouse.CreateActive(0, 0, 0, 0, LoadCursor(NULL, IDC_IBEAM ), NULL, NULL, NULL, CSysWin1d::_MouseCallback, (void*) this, C1D_ACT_XMAX,     NULL);
   Mouse.CreateActive(0, 0, 0, 0, LoadCursor(NULL, IDC_IBEAM ), NULL, NULL, NULL, CSysWin1d::_MouseCallback, (void*) this, C1D_ACT_YMIN,     NULL);
   Mouse.CreateActive(0, 0, 0, 0, LoadCursor(NULL, IDC_IBEAM ), NULL, NULL, NULL, CSysWin1d::_MouseCallback, (void*) this, C1D_ACT_YMAX,     NULL);
   Mouse.CreateActive(0, 0, 0, 0, LoadCursor(NULL, IDC_HAND  ), NULL, NULL, NULL, CSysWin1d::_MouseCallback, (void*) this, C1D_ACT_ZOOMBOTH, NULL);
   Mouse.CreateActive(0, 0, 0, 0, LoadCursor(NULL, IDC_SIZEWE), NULL, NULL, NULL, CSysWin1d::_MouseCallback, (void*) this, C1D_ACT_ZOOMMIN , NULL);
   Mouse.CreateActive(0, 0, 0, 0, LoadCursor(NULL, IDC_SIZEWE), NULL, NULL, NULL, CSysWin1d::_MouseCallback, (void*) this, C1D_ACT_ZOOMMAX , NULL);


   //===Create window=====================================
   hwRenderer = pSystem->CreateSysWinWindow(CSZ_WNDCLASS_SYSWIN1D, (LPVOID) this);

   //---Quick Edit control------------
   pQEdit = new CQuickEdit(hwRenderer);     // create the quick-edit control
   if(pQEdit) {
      pQEdit->SetCallback(CSysWin1d::_QEditCallback); // set the callback function
      pQEdit->SetFont(App()->Font());
   }

   //---Finalize----------------------
   SetBit(C1DF_SHOWICONS);                  // default to showing icons
   UserSetXLim(-10.00, pSystem->PhysicalLength()+10.00); // default horizontal scaling
   UserSetYLim(-1000, 1000);                // default vertical scaling
   OnResize();                              // force resize, since not automatic!
}


/*********************************************************
* Destructor
*********************************************************/
CSysWin1d::~CSysWin1d() {
   if(pQEdit) delete(pQEdit); pQEdit = NULL;
printf("- CSysWin -1D- Destroyed\n");}


/*########################################################
 ## Data Files
########################################################*/
/*********************************************************
* SaveSysWin
* Save the current renderer
*********************************************************/
void CSysWin1d::SaveSysWin(HANDLE hFile) {
   char szBuf[256];                         // string to write to buffer
   DWORD dwBytes;                           // number of bytes written
   int   iPrp;                              // property loop counter

   //---Preliminaries---------------------------
   if(hFile == NULL) printf("? CSysWin1d::SaveSysWin@76 Called with hFile=NULL\n");
   if(hFile == NULL) return;                // ignore if no file

   //---Header----------------------------------
   SaveSysWinHeader(hFile, Type());         // write the header

   //---Properties------------------------------
   for(iPrp=0; iPrp<C1DI_NUM_PROP; iPrp++) {
      sprintf(szBuf, "   %s = %lg\r\n", CszSysWin1dProp[iPrp], ddProp[iPrp]);
      WriteFile(hFile, szBuf, strlen(szBuf), &dwBytes, NULL);
   }

   //---Footer----------------------------------
   SaveSysWinFooter(hFile);                 // write the footer
}

/*********************************************************
* LoadSysWin
* Load renderer properties from data file.
* We  cannot finalize the position of the zoom  rectangle
* until the system has finished loading, because Zoom2Wnd
* calls pSystem->PhysicalLength(). This is covered by the
* RefreshZoomActive() call in UpdateProperties().
*********************************************************/
BOOL CSysWin1d::LoadSysWin(const char *pszDataFile, char *pszMin, char *pszMax) {
UNREFERENCED_PARAMETER(pszDataFile);
   char *psz;                               // pointer in file
   int   iPrp;                              // property loop counter

   if(pszMin==NULL) return(FALSE);          // couldn't load if no pointer
   if(pszMax==NULL) pszMax = pszMin + strlen(pszMin); // to end of file if not given

   //---Header------------------------
   LoadSysWinHeader(pszDataFile, pszMin, pszMax);

   //---Specifics---------------------
   for(iPrp=0; iPrp<C1DI_NUM_PROP; iPrp++) {
      psz = strstr(pszMin, CszSysWin1dProp[iPrp]);
      if((psz==NULL) || (psz>pszMax)) continue;
      psz = strchr(psz, '=');               //..find '=' sign
      if((psz==NULL) || (psz>pszMax)) continue;
      sscanf(psz+1, "%lg", &ddProp[iPrp]);
   }

   UserSetXLim(XMin(), XMax());             // update axes limits
   UserSetYLim(YMin(), YMax());
   OnResize();                              // set rectangles to their places
   return(TRUE);
}

/*########################################################
 ## Overloaded Functions                               ##
########################################################*/
/*********************************************************
* Refresh
*********************************************************/
void CSysWin1d::Refresh(void) {
   if(hwRenderer) InvalidateRect(hwRenderer, NULL, TRUE);
}


/*********************************************************
* UpdateTitle
*********************************************************/
void CSysWin1d::UpdateTitle(int iID) {
   char szRC[128];                          // string from resource file
   char szBuf[256];                         // formatted string
   LoadString(App()->GetInstance(), SZ_TITLE_SYSWIN1D, szRC, sizeof(szRC)/sizeof(char));
   sprintf(szBuf, szRC, (pSystem) ? ((pSystem->FileName()) ? pSystem->FileName() : "") : "");
   if(iID > 0) sprintf(szBuf+strlen(szBuf), " (%d)", iID);
   if(hwRenderer) SetWindowText(hwRenderer, szBuf);
}

/*########################################################
 ## Properties                                         ##
########################################################*/
/*********************************************************
* PrepareProperties
*********************************************************/
const UINT CSysWin1d::CuProperties[] = {
   CPS_GRPHEADER     , // "Graph"
   CPS_GRPAXXRNG     , // "X-Axis Range"
   CPS_GRPAXYRNG     , // "Y-Axis Range"
   CPS_CNVHEADER     , // "Canvas"
   CPS_CNVSHOWWAIST  , // "Waist Locations"
   CPS_CNVSHOWICONS  , // "Show Icons"
   0};
//=======================================================
void CSysWin1d::PrepareProperties(BOOL tfAct) {
   App()->PrepareVariableProperties(tfAct);
   App()->PrepareProperties(CuProperties, tfAct, CSysWin1d::_SysWinPropItemCallback, this);
   App()->EnableMenus(tfAct ? CSYSWINI_TYPE_1D : -1);
   pSystem->PrepareProperties(tfAct);
   if(tfAct) UpdateProperties();
}

/*********************************************************
* UpdateProperties
* We call RefreshZoomActive() here  because any number of
* things may cause this to be necessary. The most obvious
* are (i) when  loading a system, and (ii) when variables
* change the system layout.
*********************************************************/
void CSysWin1d::UpdateProperties(void) {
   CPropMgr *pMgr = App()->PropManager();   // assign for code below
   if(pMgr==NULL) return;                   // ignore if no manager
   pMgr->FindItemByID(CPS_GRPAXXRNG)->SetItemDblValue(XMin(), XMax());
   pMgr->FindItemByID(CPS_GRPAXYRNG)->SetItemDblValue(YMin(), YMax());
   pMgr->FindItemByID(CPS_CNVSHOWWAIST)->SetItemCheckBox(CheckBit(C1DF_SHOWWAIST));
   pMgr->FindItemByID(CPS_CNVSHOWICONS)->SetItemCheckBox(CheckBit(C1DF_SHOWICONS));
   //RefreshZoomActive();                     // update the active zoom area
}


/*********************************************************
* SysWinPropItemCallback
* Callback from property manager when an item is modified
* The callback function must be declared static.
*********************************************************/
BOOL CSysWin1d::_SysWinPropItemCallback(void *vData, UINT uID, void *pVoid) {
   return((pVoid) ? ((CSysWin1d*)pVoid)->SysWinPropItemCallback(vData, uID) : TRUE);
}
//******************************************************//
BOOL CSysWin1d::SysWinPropItemCallback(void *vData, UINT uID) {
   switch(uID) {
   case CPS_GRPAXXRNG: UserSetXLim( ((double*)vData)[0], ((double*)vData)[1] ); Refresh(); break;
   case CPS_GRPAXYRNG: UserSetYLim( ((double*)vData)[0], ((double*)vData)[1] ); Refresh(); break;
   case CPS_CNVSHOWICONS: ToggleBit(C1DF_SHOWICONS); UpdateProperties(); Refresh(); break;
   case CPS_CNVSHOWWAIST: ToggleBit(C1DF_SHOWWAIST); UpdateProperties(); Refresh(); break;
   default:
      return(TRUE);                         // accept, no changes
   }
   App()->PropManager()->OnPaint(TRUE);     // update values

   return(TRUE);
}


/*########################################################
 ##
########################################################*/
/*********************************************************
* MenuCommand
* The application calls MenuCommand() of the current ren-
* derer for ANY renderer-specific command.
*********************************************************/
void CSysWin1d::MenuCommand(int iCmd) {
   switch(iCmd) {
   //case CMU_COPYDATA: CopyDataToClipboard(); break;
   }
}

/*********************************************************
* DebugPrint
*********************************************************/
void CSysWin1d::DebugPrint(char *psz, int *pInt) {
   for(int k=0; k<*pInt; k++) sprintf(psz+strlen(psz), "   ");
   sprintf(psz+strlen(psz), "CSysWin-1D Renderer 0x%08lx\n", (long) this);
}

/*********************************************************
* GraphPoint
*********************************************************/
void CSysWin1d::GraphPoint(int iRVar, int iPt) {
UNREFERENCED_PARAMETER(iPt);
   //---Final activation--------------
   if(iRVar < 0) RefreshZoomActive();
}

/*########################################################
 ## Mouse                                              ##
########################################################*/
/*********************************************************
* MouseCallback
* This callback function must be declared static.
*********************************************************/
void CSysWin1d::_MouseCallback(int iMsg, int x, int y, int wKeys, void *pVoid, long int lData) {
   if(pVoid) ((CSysWin1d*)pVoid)->MouseCallback(iMsg, x, y, wKeys, lData);
}
void CSysWin1d::MouseCallback(int iMsg, int x, int y, int wKeys, long int lData) {
UNREFERENCED_PARAMETER(y);
UNREFERENCED_PARAMETER(wKeys);
   double dX, dY;                           // mouse coordinates

   switch(lData) {
   case C1D_ACT_XMIN:
   case C1D_ACT_XMAX:
   case C1D_ACT_YMIN:
   case C1D_ACT_YMAX:
      if(iMsg!=ACSM_LEFT) break;            // ignore everything other than left click
      if(pQEdit==NULL) break;               // ignore if no control
      pQEdit->SetUserData(this, lData);     // set identifiers
      pQEdit->SetPositionRect(Mouse.FindActiveByData(this, lData)->ActiveRect());
      pQEdit->SetDouble(
         (lData==C1D_ACT_XMIN) ? ddProp[C1DI_PROP_XMIN] :
         (lData==C1D_ACT_XMAX) ? ddProp[C1DI_PROP_XMAX] :
         (lData==C1D_ACT_YMIN) ? ddProp[C1DI_PROP_YMIN] :
         (lData==C1D_ACT_YMAX) ? ddProp[C1DI_PROP_YMAX] : 0.00);
      pQEdit->Show();
      break;

   case C1D_ACT_AXES:
      if(iMsg != ACSM_MOVE) break;
      dX = Axes.Client2PlaneX(x);
      dY = Axes.Client2PlaneY(y);
      App()->SetStatusBarInfo(&dX, &dY);
      break;

   case C1D_ACT_ZOOMMIN:
   case C1D_ACT_ZOOMMAX:
   case C1D_ACT_ZOOMBOTH:
      switch(iMsg) {
      case ACSM_DOWN:
         if(lData==C1D_ACT_ZOOMBOTH) iXMouseOffs = x - Zoom2Wnd(XMin());
         break;
      case ACSM_DRAG:
      case ACSM_DEND:
         switch(lData) {
         case C1D_ACT_ZOOMBOTH:
            ddProp[C1DI_PROP_XMAX] += Wnd2Zoom(x - iXMouseOffs) - ddProp[C1DI_PROP_XMIN];
            ddProp[C1DI_PROP_XMIN] = Wnd2Zoom(x - iXMouseOffs);
            break;
         case C1D_ACT_ZOOMMIN: ddProp[C1DI_PROP_XMIN] = Wnd2Zoom(x); break;
         case C1D_ACT_ZOOMMAX: ddProp[C1DI_PROP_XMAX] = Wnd2Zoom(x); break;
         }
         if(ddProp[C1DI_PROP_XMIN]>ddProp[C1DI_PROP_XMAX]-C1DC_MINZOOM) ddProp[C1DI_PROP_XMIN] = ddProp[C1DI_PROP_XMAX]-C1DC_MINZOOM;
         if(ddProp[C1DI_PROP_XMAX]<ddProp[C1DI_PROP_XMIN]+C1DC_MINZOOM) ddProp[C1DI_PROP_XMAX] = ddProp[C1DI_PROP_XMIN]+C1DC_MINZOOM;
         OnResize();                        // re-calculate ticks and scaling
         UpdateProperties();                // update properties in manager
         App()->PropManager()->OnPaint(TRUE); // update values in Property Manager
         Refresh();                         // re-paint my window
         break;
      }
   }
}

/*********************************************************
* QEditCallback
* The callback function must be declared static.
*********************************************************/
BOOL CSysWin1d::_QEditCallback(void *pVal, int iNext, void *pVoid, long int lLong) {
   return( ((CSysWin1d*)pVoid)->QEditCallback(pVal, iNext, lLong) );
}
BOOL CSysWin1d::QEditCallback(void *pVal, int iNext, long int lLong) {
UNREFERENCED_PARAMETER(iNext);
   switch(lLong) {
   case C1D_ACT_XMIN: UserSetXLim(*(double*)pVal, XMax()); break;
   case C1D_ACT_XMAX: UserSetXLim(XMin(), *(double*)pVal); break;
   case C1D_ACT_YMIN: UserSetYLim(*(double*)pVal, YMax()); break;
   case C1D_ACT_YMAX: UserSetYLim(YMin(), *(double*)pVal); break;
   }
   App()->PropManager()->OnPaint(TRUE);     // update the values in the property manager
   Refresh();                               // re-paint the renderer
   return(TRUE);
}



/*********************************************************
* UserSet Limits
* Set the limits, either X or Y axes.
* Just because we can, we ensure they're ascending.
* Called from
*   <- property sheet callback
*   <- on-screen quick edit
*********************************************************/
//===X Limits=============================================
void CSysWin1d::UserSetXLim(double dMn, double dMx) {
   ddProp[C1DI_PROP_XMIN] = (dMn < dMx) ? dMn : dMx;
   ddProp[C1DI_PROP_XMAX] = (dMn < dMx) ? dMx : dMn;
   Axes.SetAxDataLimit(AX_XAXIS, ddProp[C1DI_PROP_XMIN], ddProp[C1DI_PROP_XMAX]);
   Axes.CalcAxTicks(AX_XAXIS);              // re-calculate ticks
   RefreshZoomActive();                     // update the zoom active
   UpdateProperties();
}

//===Y Limits=============================================
void CSysWin1d::UserSetYLim(double dMn, double dMY) {
   ddProp[C1DI_PROP_YMIN] = (dMn < dMY) ? dMn : dMY;
   ddProp[C1DI_PROP_YMAX] = (dMn < dMY) ? dMY : dMn;
   Axes.SetAxDataLimit(AX_YAXIS, ddProp[C1DI_PROP_YMIN], ddProp[C1DI_PROP_YMAX]);
   Axes.CalcAxTicks(AX_YAXIS);
   UpdateProperties();
}


/*########################################################
 ## Window Methods                                     ##
########################################################*/
/*********************************************************
* Mapping functions
*********************************************************/
int CSysWin1d::Zoom2Wnd(double dD) {
   return( rcRect[C1DI_RECT_FULL].left + 32 + dD
      * (rcRect[C1DI_RECT_FULL].right - rcRect[C1DI_RECT_FULL].left - 64)
      / (pSystem->PhysicalLength() + ((pSystem->PhysicalLength()==0.00) ? 1.00 : 0.00))
      );
}
double CSysWin1d::Wnd2Zoom(int id) {
   return(
      (id - rcRect[C1DI_RECT_FULL].left - 32)
      * pSystem->PhysicalLength()
      / (rcRect[C1DI_RECT_FULL].right-rcRect[C1DI_RECT_FULL].left-64+(((rcRect[C1DI_RECT_FULL].right-rcRect[C1DI_RECT_FULL].left-64)==0)?1:0))
   );
}

/*********************************************************
* WndProcSysWin1d
* Since this is  an MDI child window, the  object pointer
* is passed through an  MDICREATESTRUCT on WM_CREATE, and
* we use DefMDIChildProc instead of DefWindowProc. In ad-
* dition, we must pass these  messages, even if we handle
* them:
*    WM_CHILDACTIVATE
*    WM_GETMINMAXINFO
*    WM_MENUCHAR
*    WM_MOVE
*    WM_SETFOCUS
*    WM_SIZE
*    WM_SYSCOMMAND
*********************************************************/
///TODO: Pass messages on!
LRESULT CALLBACK CSysWin1d::WndProcSysWin1d(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
   CSysWin1d *pSWin;                        // owning object
   pSWin = (CSysWin1d*) GetWindowLong(hWnd, 0); // not valid on WM_CREATE
   switch(uMsg) {
   case WM_CREATE:
      pSWin = (CSysWin1d*) ((LPMDICREATESTRUCT) (((LPCREATESTRUCT)lParam)->lpCreateParams))->lParam;
      SetWindowLong(hWnd, 0, (LONG) pSWin);
      break;

   case WM_CLOSE:
      pSWin->pSystem->DeleteSysWin(pSWin);
      break;

   case WM_MDIACTIVATE:
      if(pSWin==NULL) break;
      pSWin->PrepareProperties((hWnd==(HWND)lParam) ? TRUE : FALSE);
      if(hWnd==(HWND)lParam) pSWin->App()->PropManager()->OnPaint(TRUE);
      break;

   case WM_SIZE:  pSWin->OnResize(); break;
   case WM_PAINT: pSWin->OnPaint(); break;

   case WM_MOUSEMOVE:
   case WM_LBUTTONDOWN:
   case WM_LBUTTONUP:
      pSWin->Mouse.MouseProc(hWnd, uMsg, LOWORD(lParam), HIWORD(lParam));
      break;

   default:
      return(DefMDIChildProc(hWnd, uMsg, wParam, lParam));
   }
   return(0L);
}


/*********************************************************
* OnResize
* Called when the window is sized.  We need to update the
* rectangles for the full cavity and the axes.
*********************************************************/
void CSysWin1d::OnResize(void) {
   RECT rc;                                 // window client rectangle
   GetClientRect(hwRenderer, &rc);          // read client rectangle

   //---Zoom bar--------------------------------
   SetRect(&rcRect[C1DI_RECT_FULL], rc.left, rc.top, rc.right, rc.top+C1DC_ZOOMHEIGHT);
   RefreshZoomActive();                     // update the zoom active areas

   //---Axes------------------------------------
   SetRect(&rc, rc.left+64, rc.top+C1DC_ZOOMHEIGHT+12, rc.right-32, rc.bottom-32);
   Axes.SetPosition(&rc);
   Axes.SetAxDataLimit(AX_XAXIS, ddProp[C1DI_PROP_XMIN], ddProp[C1DI_PROP_XMAX]);
   Axes.SetAxMaxVisTick(AX_XAXIS, (rc.right-rc.left) / 60);
   Axes.SetAxMaxVisTick(AX_YAXIS, (rc.bottom-rc.top) / 30);
   Axes.CalcTicks();
   Mouse.FindActiveByData(this, CGVX_ACT_AXES   )->SetActiveRect(&rc);
   Mouse.FindActiveByData(this, CGVX_ACT_XMIN   )->SetActiveRect(rc.left -24, rc.bottom+ 2, rc.left +24, rc.bottom+18);
   Mouse.FindActiveByData(this, CGVX_ACT_XMAX   )->SetActiveRect(rc.right-24, rc.bottom+ 2, rc.right+24, rc.bottom+18);
   Mouse.FindActiveByData(this, CGVX_ACT_YMIN   )->SetActiveRect(rc.left -52, rc.bottom- 8, rc.left - 4, rc.bottom+ 8);
   Mouse.FindActiveByData(this, CGVX_ACT_YMAX   )->SetActiveRect(rc.left -52, rc.top   - 8, rc.left - 4, rc.top   + 8);

}


/*********************************************************
* RefreshZoomActive
* Assumes that RECT_FULL has  been set, so we can measure
* relative to it.
* Called from
* <- OnResize()
* <- UserSetXLim
*    <- Mouse callback
*********************************************************/
void CSysWin1d::RefreshZoomActive(void) {
   RECT rc;                                 // window client rectangle

   CopyRect(&rc, &rcRect[C1DI_RECT_FULL]);  // start from bar rectangle
   SetRect(&rc,
      Zoom2Wnd(ddProp[C1DI_PROP_XMIN]), rc.top+4,
      Zoom2Wnd(ddProp[C1DI_PROP_XMAX]), rc.bottom -4);
   if(rc.left  < rcRect[C1DI_RECT_FULL].left +16) rc.left  = rcRect[C1DI_RECT_FULL].left +16;
   if(rc.right > rcRect[C1DI_RECT_FULL].right-16) rc.right = rcRect[C1DI_RECT_FULL].right-16;
   if(rc.left  > rc.right-8) rc.left  = rc.right-8;
   if(rc.right < rc.left +8) rc.right = rc.left +8;
   CopyRect(&rcRect[C1DI_RECT_ZOOM], &rc);
   Mouse.FindActiveByData(this, C1D_ACT_ZOOMBOTH)->SetActiveRect(rc.left +4, rc.top, rc.right-4, rc.bottom);
   Mouse.FindActiveByData(this, C1D_ACT_ZOOMMIN )->SetActiveRect(rc.left -3, rc.top, rc.left +3, rc.bottom);
   Mouse.FindActiveByData(this, C1D_ACT_ZOOMMAX )->SetActiveRect(rc.right-3, rc.top, rc.right+3, rc.bottom);
}


/*********************************************************
* Print
* This one's a close call -- there's  no good way to slip
* in the printing routine. We will follow the 2d renderer
* method of prepping things and then calling OnPaint with
* a specified device context
*********************************************************/
void CSysWin1d::Print(HDC hdcPrint) {
   char  szBuf[256];                        // formatted text buffer
   RECT  rc;                                // axes rectangle
   SIZE  sz;                                // text size
   int   iPrintWid, iPrintHig;              // printer page sizes (pixels)

   if(hdcPrint==NULL) return;               // ignore bad calls

   //===Prepare===========================================
   //---Scaling---------------------------------
   iPrintWid = GetDeviceCaps(hdcPrint, HORZRES); // print width (pixels)
   iPrintHig = GetDeviceCaps(hdcPrint, VERTRES); // print height (pixels)

   GetTextExtentPoint32(hdcPrint, "1234567", 7, &sz); // get size of generic tick
   sz.cx *= 1.2; sz.cy *= 1.8;                // allow some extra space
   SetRect(&rc,
      0.20*iPrintWid, 0.30*iPrintHig,
      0.80*iPrintWid, 0.70*iPrintHig);
   Axes.SetPosition(&rc);                   // move axes to center of page
   Axes.SetAxMaxVisTick(AX_XAXIS, (rc.right-rc.left)/sz.cx);
   Axes.SetAxMaxVisTick(AX_YAXIS, (rc.bottom-rc.top)/sz.cy);
   Axes.CalcTicks();                        // re-calculate the ticks
   SetRect(&rc,
      0.20*iPrintWid, 0.25*iPrintHig,
      0.80*iPrintWid, 0.75*iPrintHig);

   //===Print=============================================
   OnPaint(hdcPrint);                       // print to printer dc

   //---Catch up remaining----------------------
   RECT rcMain, rcAxes;                     // patch-out rectangles
   SetRect(&rcMain, 0, 0, iPrintWid, iPrintHig);
   Axes.GetPosition(&rcAxes);
   SetRect(&rc, rcMain.left, rcMain.top, rcMain.right, rcAxes.top);
   FillRect(hdcPrint, &rc, App()->Brush(CAPPI_BRUSHWINDOW));
   SetRect(&rc, rcMain.left, rcAxes.bottom, rcMain.right, rcMain.bottom);
   FillRect(hdcPrint, &rc, App()->Brush(CAPPI_BRUSHWINDOW));
   SetRect(&rc, rcMain.left, rcAxes.top, rcAxes.left, rcAxes.bottom);
   FillRect(hdcPrint, &rc, App()->Brush(CAPPI_BRUSHWINDOW));
   SetRect(&rc, rcAxes.right, rcAxes.top, rcMain.right, rcAxes.bottom);
   FillRect(hdcPrint, &rc, App()->Brush(CAPPI_BRUSHWINDOW));
   Axes.Paint((void*) &hdcPrint);                // paint axes

   //===Finalize==========================================
   //---Restore---------------------------------
   OnResize();                              // reposition to window

}


/*********************************************************
* Paint
* This function is used both for printing and painting to
* the window. If hdcIn is non-NULL for printing, then se-
* veral things are omitted, like the full cavity bar.
*********************************************************/
void CSysWin1d::OnPaint(HDC hdcIn) {
   PAINTSTRUCT ps;                          // painting structure
   CVertex *pVx;                            // vertex loop counter
   HDC      hdc;                            // device context to use
   int      iVert;                          // vertical mid-point
   double   dDist;                          // horizontal distance
   RECT     rcClient, rcMain, rcAxes, rc;   // painting, axes rectangle
   HDC      hdcMem;                         // memory DC
   HBITMAP  hbmOld, hbmMem;                 // restore, memory bitmap

   //===Preliminaries=====================================
   //---Printer---------------------------------
   if(hdcIn) {
      hdcMem = NULL;                        // we're not using memory blitting
      hbmMem = NULL;
      hdc    = hdcIn;                       // use the supplied context
      SelectObject(hdc, App()->Font(CAPPI_FONTPRINT)); // use the printer font

   //---Window----------------------------------
   } else {
      BeginPaint(hwRenderer, &ps);
      GetClientRect(hwRenderer, &rcClient); // read full window rectangle
      hdc = ps.hdc;
      do {
         if((hdcMem=CreateCompatibleDC(ps.hdc))==NULL) break;
         if((hbmMem=CreateCompatibleBitmap(ps.hdc, rcClient.right-rcClient.left, rcClient.bottom-rcClient.top))==NULL) break;
         hdc = hdcMem;
         hbmOld = (HBITMAP) SelectObject(hdc, hbmMem);
      } while (0);
      SaveDC(hdc);
      SelectObject(hdc, App()->Font());
      SetRect(&rcMain, rcClient.left, rcClient.top+C1DC_ZOOMHEIGHT, rcClient.right, rcClient.bottom);
   }

   //===Mode==============================================
   // z0 is initial z relative to waist position
   // zMax is a RELATIVE (!) position to the waist
   int ix, iy;                              // positions for Renderer2dMode
   double dz0Sag, dz0Tan, dZMax;            // parameters for Renderer2dMode
   double dLenSeg, dScale;                  // vertical mode scale

   Axes.GetPosition(&rcAxes);               // get axes positions
   dScale = (double)(rcAxes.top - rcAxes.bottom)
      / (YMax() - YMin() + ((YMax()==YMin())?1.00:0.00)); // vertical scale for mode
   if(hdcIn==NULL) FillRect(hdc, &rcAxes, App()->Brush(CAPPI_BRUSHWINDOW)); // blank out central part in window

   if((pSystem->StableABCD(SAG)) || (pSystem->StableABCD(TAN))) {
      dDist = 0.00;
      for(pVx=pSystem->VxTop(); (pVx->Next()) && (dDist < XMax()); dDist += pVx->Dist2Next(), pVx=(CVertex*) pVx->Next()) {
         if((pVx->Next()) && (dDist+pVx->Dist2Next() < XMin())) continue; // skip until first in sight
         //---Start condition---------
         if(dDist >= XMin()) {              // already starting within axis
            ix      = Axes.Plane2ClientX(dDist); // map point to screen
            iy      = Axes.Plane2ClientY(0.00);  // centered on zero
            dz0Sag  = pVx->Q(SAG)->z0();         // starting position in mode
            dz0Tan  = pVx->Q(TAN)->z0();
            dZMax   = -0.00;                     // full length available
            dLenSeg = -Axes.Plane2ClientX(dDist);// start here
         } else {                           // clip part not visible
            ix      = Axes.Plane2ClientX(XMin()); // start at left edge
            iy      = Axes.Plane2ClientY(0.00);   // centered on zero
            dz0Sag  = pVx->Q(SAG)->z0() - (XMin()-dDist)/pVx->RefIndex(); // offset to start in new place
            dz0Tan  = pVx->Q(TAN)->z0() - (XMin()-dDist)/pVx->RefIndex(); ///TODO: Refractive index?!
            dZMax   = -(XMin()-dDist);           // offset end by same amount
            dLenSeg = -Axes.Plane2ClientX(XMin());
         }
         //---End condition-----------
         if(dDist + pVx->Dist2Next() > XMax()) {
            dZMax += XMax() - dDist;
            dLenSeg += Axes.Plane2ClientX(XMax());
         } else {
            dZMax += pVx->Dist2Next();       // plot only as much as fits
            dLenSeg += Axes.Plane2ClientX(dDist + pVx->Dist2Next());
         }

         //---Symmetric---------------
         if(pVx->SymmetricQVx()) {
            if(pSystem->StableABCD(SAG)) {
               SelectObject(hdc, App()->Pen(CAPPI_PENSAGTAN));
               Renderer2dMode(hdc, ix, iy, 0.00, dScale, dLenSeg,
                  pVx->Q(SAG)->W0(), dz0Sag, pVx->Q(SAG)->zR(), dZMax / pVx->RefIndex(),
                  CheckBit(C1DF_SHOWWAIST) ? TRUE : FALSE);
            }

         //---Sag/Tan-----------------
         } else {
            if(pSystem->StableABCD(SAG)) {
               SelectObject(hdc, App()->Pen(CAPPI_PENSAG));
               Renderer2dMode(hdc, ix, iy, 0.00, dScale, dLenSeg,
                  pVx->Q(SAG)->W0(), dz0Sag, pVx->Q(SAG)->zR(), dZMax / pVx->RefIndex(),
                  CheckBit(C1DF_SHOWWAIST) ? TRUE : FALSE);
            }
            if(pSystem->StableABCD(TAN)) {
               SelectObject(hdc, App()->Pen(CAPPI_PENTAN));
               Renderer2dMode(hdc, ix, iy, 0.00, dScale, dLenSeg,
                  pVx->Q(TAN)->W0(), dz0Tan, pVx->Q(TAN)->zR(), dZMax / pVx->RefIndex(),
                  CheckBit(C1DF_SHOWWAIST) ? TRUE : FALSE);
            }
         }
      }
   }


   //===Axes==============================================
   //---Line------------------------------------
   SelectObject(hdc, App()->Pen(CAPPI_PENAXES));
   MoveTo(hdc, Axes.Plane2ClientX(ddProp[C1DI_PROP_XMIN]), Axes.Plane2ClientY(0.00));
   LineTo(hdc, Axes.Plane2ClientX(ddProp[C1DI_PROP_XMAX]), Axes.Plane2ClientY(0.00));

   //---Vertex icons----------------------------
   if(CheckBit(C1DF_SHOWICONS)) {
      if(rcAxes.bottom - rcAxes.top > 0) {
         for(dDist=0.00, pVx=pSystem->VxTop(); pVx; pVx=pVx->Next()) {
            if((dDist >= ddProp[C1DI_PROP_XMIN]) && (dDist<=ddProp[C1DI_PROP_XMAX])) {
               Renderer2dIcon(hdc, pVx,
                  Axes.Plane2ClientX(dDist), Axes.Plane2ClientY(0.00),
                  0.12*(rcAxes.bottom-rcAxes.top),
                  (pVx->Prev()==NULL) ? M_PI : 0.00);
            }
            dDist += pVx->Dist2Next();
            if(dDist > ddProp[C1DI_PROP_XMAX]) break;
         }
      }
   }

   if(hdcIn == NULL) {
      //---Axes frame------------------------------
      // This is a bit cheap, but  Renderer2dMode paints all
      // over the screen, which is a bit ugly so we mask out
      // everything  but the axes here. This also  allows us
      // to use a NULL brush in  the window class, smoothing
      // painting.
      SetRect(&rc, rcMain.left, rcMain.top, rcMain.right, rcAxes.top);
      FillRect(hdc, &rc, App()->Brush(CAPPI_BRUSHWINDOW));
      SetRect(&rc, rcMain.left, rcAxes.bottom, rcMain.right, rcMain.bottom);
      FillRect(hdc, &rc, App()->Brush(CAPPI_BRUSHWINDOW));
      SetRect(&rc, rcMain.left, rcAxes.top, rcAxes.left, rcAxes.bottom);
      FillRect(hdc, &rc, App()->Brush(CAPPI_BRUSHWINDOW));
      SetRect(&rc, rcAxes.right, rcAxes.top, rcMain.right, rcAxes.bottom);
      FillRect(hdc, &rc, App()->Brush(CAPPI_BRUSHWINDOW));
      Axes.Paint((void*) &hdc);                // paint axes

      //===Full Cavity Bar===================================
      //---Background--------------------
      CopyRect(&rc, &rcRect[C1DI_RECT_FULL]);
      FillRect(hdc, &rc, App()->Brush(CAPPI_BRUSH3DFACE));
      SelectObject(hdc, App()->Pen(CAPPI_PEN3DDARK));
      MoveTo(hdc, rc.left,  rc.bottom);
      LineTo(hdc, rc.right, rc.bottom);

      //---Zoom selection----------------
      CopyRect(&rc, &rcRect[C1DI_RECT_ZOOM]);
      //FillRect(hdc, &rc, App()->Brush(CAPPI_BRUSH3DFACE));
      SelectObject(hdc, App()->Pen(CAPPI_PEN3DDARK));
      MoveTo(hdc, rc.left , rc.bottom);
      LineTo(hdc, rc.left , rc.top);
      LineTo(hdc, rc.right, rc.top);
      SelectObject(hdc, App()->Pen(CAPPI_PEN3DLITE));
      LineTo(hdc, rc.right, rc.bottom);
      LineTo(hdc, rc.left , rc.bottom);

      //---Line--------------------------
      CopyRect(&rc, &rcRect[C1DI_RECT_FULL]);
      SetRect(&rc, rc.left+32, rc.top+8, rc.right-32, rc.bottom-8);
      iVert = (rc.top + rc.bottom) / 2;
      SelectObject(hdc, App()->Pen(CAPPI_PEN3DTEXT));
      MoveTo(hdc, rc.left , iVert);
      LineTo(hdc, rc.right, iVert);

   ///TODO: maybe have separate Vx property for position in cavity
      //---Vertices----------------------
      for(dDist=0.00, pVx=pSystem->VxTop(); pVx; pVx=pVx->Next()) {
         Renderer2dIcon(hdc, pVx,
            Zoom2Wnd(dDist), iVert,
            (double)(rc.bottom - rc.top)/2.00,
            (pVx->Prev()==NULL) ? M_PI : 0.00);
         dDist += pVx->Dist2Next();
      }
   }

   //===Finalize==========================================
   //Mouse.PaintAllActiveRect(hdc);
   RestoreDC(hdc, -1);
   if((hdcMem) && (hbmMem)) {
      BitBlt(ps.hdc, rcClient.left, rcClient.top,
         rcClient.right-rcClient.left, rcClient.bottom-rcClient.top,
         hdcMem, 0, 0, SRCCOPY);
      SelectObject(hdcMem, hbmOld);
   }
   if(hdcMem) DeleteDC(hdcMem);
   if(hbmMem) DeleteObject(hbmMem);

   if(hdcIn==NULL) EndPaint(hwRenderer, &ps);
}
