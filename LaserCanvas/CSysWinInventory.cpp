/*********************************************************
* CSysWinInventory
* System renderer that lists all  of the optics and their
* properties.  This is probably most useful as  a printed
* document but might be interesting on-screen as well. It
* seems to make sense to have only ONE inventory for each
* MODEL, at the top-level system, and list spawned system
* components below the main  one. This means that the in-
* ventory list will look similar to saved files.
* $PSchlup 2006 $     $Revision 0 $
*********************************************************/
#include "CSysWinInventory.h"               // class include

const char* CSysWinInventory::CszInvVxColumnSource = NULL;
const char* CSysWinInventory::CszInvVxColumn[2*CINI_NUMVXCOL];
const char* CSysWinInventory::CszInvSysRowSource = NULL;
const char* CSysWinInventory::CszInvSysRow[CINI_NUMSYSROW];

/*********************************************************
* Constructor
*********************************************************/
CSysWinInventory::CSysWinInventory(CSystem *pSys) : CSysWin(pSys) {printf("+ CSysWin -Inventory- Created\n");
   char  szBuf[512];                        // string table
   char *psz;                               // string loop pointer
   int   k;                                 // loop counter
   int   iLen;                              // length of RC string

   //===Members===========================================
   iRowHeight = 16;
   iOffsetX = 0;
   iOffsetY = 0;
   for(k=0; k<CINI_NUM_PROP;  k++) ddProp[k]   = 0.00;
   for(k=0; k<CINI_NUMVXCOL; k++) SetColWidth(k, 80);

   //---String table----------------------------
   // Here delimited with '~' (including at end!)
   if(CszInvVxColumnSource==NULL) {
      LoadString(App()->GetInstance(), CPL_INVVXCOLS, szBuf, sizeof(szBuf)/sizeof(char));
      for(k=0, iLen=strlen(szBuf); k<iLen; k++) if(szBuf[k]=='~') szBuf[k] = '\0';
      psz = (char*) malloc((iLen+1)*sizeof(char));    // allocate buffer
      memset(psz, 0x00, (iLen+1)*sizeof(char));       // empty the buffer
      memcpy(psz, szBuf, iLen*sizeof(char));          // copy the formatted string
      CszInvVxColumnSource = (const char*) psz;       // fix pointer
      for(k=0; k< 2*CINI_NUMVXCOL; k++) {             // 2 rows!
         CszInvVxColumn[k] = (const char*) psz;
         psz += strlen(psz) + 1;
      }
   }
   if(CszInvSysRowSource==NULL) {
      LoadString(App()->GetInstance(), CPL_INVSYSROWS, szBuf, sizeof(szBuf)/sizeof(char));
      for(k=0, iLen=strlen(szBuf); k<iLen; k++) if(szBuf[k]=='~') szBuf[k] = '\0';
      psz = (char*) malloc((iLen+1)*sizeof(char));    // allocate buffer
      memset(psz, 0x00, (iLen+1)*sizeof(char));       // empty the buffer
      memcpy(psz, szBuf, iLen*sizeof(char));          // copy the formatted string
      CszInvSysRowSource = (const char*) psz;       // fix pointer
      for(k=0; k< 2*CINI_NUMSYSROW; k++) {
         CszInvSysRow[k] = (const char*) psz;
         psz += strlen(psz) + 1;
      }
   }

   //---Mouse-------------------------
   for(k=0; k<CINI_NUMVXCOL; k++) {
      Mouse.CreateActive(0, 0, 0, 0, LoadCursor(NULL, IDC_SIZEWE), NULL, NULL, NULL, CSysWinInventory::_MouseCallback, (void*) this, k, NULL);
   }

   //===Create window=====================================
   hwRenderer = pSystem->CreateSysWinWindow(CSZ_WNDCLASS_SYSWININVENTORY, (LPVOID) this, WS_HSCROLL|WS_VSCROLL);
   OnResize();                              // force resize, since not automatic!

}


/*********************************************************
* Destructor
*********************************************************/
CSysWinInventory::~CSysWinInventory() {

printf("- CSysWin -Inventory- Destroyed\n");}


/*########################################################
 ## Data Files
########################################################*/
/*********************************************************
* SaveSysWin
* Save the current renderer
* Not much to save for this one; it's dynamically updated
* anyway
*********************************************************/
const char* CSysWinInventory::CszSysWinInventoryProp[CINI_PROP_COLWIDTH+1] = {"XScroll","YScroll","Width"};
void CSysWinInventory::SaveSysWin(HANDLE hFile) {
   char szBuf[256];                         // string to write to buffer
   DWORD dwBytes;                           // number of bytes written
   int   iPrp;                              // property loop counter

   if(hFile == NULL) printf("? CSysWinInventory::SaveSysWin@76 Called with hFile=NULL\n");
   if(hFile == NULL) return;                // ignore if no file

   SaveSysWinHeader(hFile, Type());         // write the header
   //---Properties------------------------------
   for(iPrp=0; iPrp<CINI_PROP_COLWIDTH; iPrp++) {
      sprintf(szBuf, "   %s = %lg\r\n", CszSysWinInventoryProp[iPrp], ddProp[iPrp]);
      WriteFile(hFile, szBuf, strlen(szBuf), &dwBytes, NULL);
   }

   //---Column widths---------------------------
   for(iPrp=0; iPrp<CINI_NUMVXCOL; iPrp++) {
      sprintf(szBuf, "   %s%d = %d\r\n", CszSysWinInventoryProp[CINI_PROP_COLWIDTH], iPrp, ColWidth(iPrp));
      WriteFile(hFile, szBuf, strlen(szBuf), &dwBytes, NULL);
   }

   SaveSysWinFooter(hFile);                 // write the footer
}

/*********************************************************
* LoadSysWin
* Load renderer properties from data file
*********************************************************/
BOOL CSysWinInventory::LoadSysWin(const char *pszDataFile, char *pszMin, char *pszMax) {
   char szBuf[256];                         // formatted search string
   char *psz;                               // pointer in file
   int   iPrp;                              // property loop counter
   int   w;                                 // read column width

   //---Header------------------------
   LoadSysWinHeader(pszDataFile, pszMin, pszMax);


   //---Properties--------------------
   for(iPrp=0; iPrp<CINI_PROP_COLWIDTH; iPrp++) {
      psz = strstr(pszMin, CszSysWinInventoryProp[iPrp]);
      if((psz==NULL) || (psz>pszMax)) continue;
      psz = strchr(psz, '=');
      if((psz==NULL) || (psz>pszMax)) continue;
      sscanf(psz+1, "%lg", &ddProp[iPrp]);
   }

   //---Specifics---------------------
   for(iPrp=0; iPrp<CINI_NUMVXCOL; iPrp++) {
      sprintf(szBuf, "%s%d", CszSysWinInventoryProp[CINI_PROP_COLWIDTH], iPrp);
      psz = strstr(pszMin, szBuf);
      if((psz==NULL) || (psz>pszMax)) continue;
      psz = strchr(psz, '=');               //..find '=' sign
      if((psz==NULL) || (psz>pszMax)) continue;
      if(sscanf(psz+1, "%d", &w) > 0) SetColWidth(iPrp, w);
   }
   OnResize();

   return(TRUE);
}

/*########################################################
 ## Overloaded Functions                               ##
########################################################*/
/*********************************************************
* Refresh
*********************************************************/
void CSysWinInventory::Refresh(void) {
   if(hwRenderer) InvalidateRect(hwRenderer, NULL, TRUE);
}


/*********************************************************
* UpdateTitle
*********************************************************/
void CSysWinInventory::UpdateTitle(int iID) {
   char szRC[128];                          // string from resource file
   char szBuf[256];                         // formatted string
   LoadString(App()->GetInstance(), SZ_TITLE_SYSWININVENTORY, szRC, sizeof(szRC)/sizeof(char));
   sprintf(szBuf, szRC, (pSystem) ? ((pSystem->FileName()) ? pSystem->FileName() : "") : "");
   if(iID > 0) sprintf(szBuf+strlen(szBuf), " (%d)", iID);
   if(hwRenderer) SetWindowText(hwRenderer, szBuf);
}


/*********************************************************
* GraphPoint
*********************************************************/
void CSysWinInventory::GraphPoint(int iRVar, int iPt) {
UNREFERENCED_PARAMETER(iRVar);
UNREFERENCED_PARAMETER(iPt);
}


/*########################################################
 ## Properties                                         ##
########################################################*/
/*********************************************************
* PrepareProperties
*********************************************************/
const UINT CSysWinInventory::CuProperties[] = {
   0};
//=======================================================
void CSysWinInventory::PrepareProperties(BOOL tfAct) {
   App()->PrepareVariableProperties(tfAct);
   App()->PrepareProperties(CuProperties, tfAct, CSysWinInventory::_SysWinPropItemCallback, this);
   App()->EnableMenus(tfAct ? CSYSWINI_TYPE_INVENTORY : -1);
   pSystem->PrepareProperties(tfAct);
}

/*********************************************************
* UpdateProperties
*********************************************************/
void CSysWinInventory::UpdateProperties(void) {
}


/*********************************************************
* SysWinPropItemCallback
* Callback from property manager when an item is modified
* The callback function must be declared static.
*********************************************************/
BOOL CSysWinInventory::_SysWinPropItemCallback(void *vData, UINT uID, void *pVoid) {
   return((pVoid) ? ((CSysWinInventory*)pVoid)->SysWinPropItemCallback(vData, uID) : TRUE);
}
//******************************************************//
BOOL CSysWinInventory::SysWinPropItemCallback(void *vData, UINT uID) {
UNREFERENCED_PARAMETER(vData);
UNREFERENCED_PARAMETER(uID);
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
void CSysWinInventory::MenuCommand(int iCmd) {
   switch(iCmd) {
   //case CMU_COPYDATA: CopyDataToClipboard(); break;
   }
}

/*********************************************************
* DebugPrint
*********************************************************/
void CSysWinInventory::DebugPrint(char *psz, int *pInt) {
   for(int k=0; k<*pInt; k++) sprintf(psz+strlen(psz), "   ");
   sprintf(psz+strlen(psz), "CSysWin-Inventory Renderer 0x%08lx\n", (long) this);
}

/*********************************************************
* DebugCheckCRecQ
* Debug routine to check the  CRecQ values. We include it
* in this renderer since it has access to CSystem and al-
* ready performed some debugguing during development.
*  void   SetWLen(double dWLen)        { _L = (dWLen==0.00) ? 1.00 : dWLen; }; // set the wavelength only
*  void   ApplyAbcd(const CMatrix2x2 *pMx); // applies ABCD matrix
*
*  //---Set functions-----------------
*  void   Set(double R, double v, double L); // set members directly
*  void   SetRealImag(double dReal, double dImag, double dWLen); // set by complex parts
*  void   SetRw(double dRz, double dwz, double dWLen); // set by curvature and mode size
*  void   Setw0z0(double dw0, double dz0, double dWLen); // set by waist and distance to waist
*
*  //---Get functions-----------------
*  double R(void);                          // R at plane
*  double w2(void);                         // w^2 at plane
*  double w02(void);                        // waist squared
*  double w0(void);                         // waist size
*  double R(double z);                      // curvature at given position
*  double w(double z);                      // mode size at given position
*  double z0(void);                         // relative distane to the waist
*  double zR(void);                         // Rayleigh length
*
* This all seems to work as expected.
*********************************************************/
#define fnPrintfQ(pFile, Q, k) {\
   fprintf(pFile, "R()     "); for(k=0; k<NUMQ; k++) fprintf(pFile,"\t%lg", Q[k].R()     ); fprintf(pFile, "\n");\
   fprintf(pFile, "w2()    "); for(k=0; k<NUMQ; k++) fprintf(pFile,"\t%lg", Q[k].w2()    ); fprintf(pFile, "\n");\
   fprintf(pFile, "w02()   "); for(k=0; k<NUMQ; k++) fprintf(pFile,"\t%lg", Q[k].w02()   ); fprintf(pFile, "\n");\
   fprintf(pFile, "w0()    "); for(k=0; k<NUMQ; k++) fprintf(pFile,"\t%lg", Q[k].w0()    ); fprintf(pFile, "\n");\
   fprintf(pFile, "z0()    "); for(k=0; k<NUMQ; k++) fprintf(pFile,"\t%lg", Q[k].z0()    ); fprintf(pFile, "\n");\
   fprintf(pFile, "zR()    "); for(k=0; k<NUMQ; k++) fprintf(pFile,"\t%lg", Q[k].zR()    ); fprintf(pFile, "\n");\
   fprintf(pFile, "w(10.00)"); for(k=0; k<NUMQ; k++) fprintf(pFile,"\t%lg", Q[k].w(10.00)); fprintf(pFile, "\n");\
   fprintf(pFile, "R(10.00)"); for(k=0; k<NUMQ; k++) fprintf(pFile,"\t%lg", Q[k].R(10.00)); fprintf(pFile, "\n");\
   fprintf(pFile, "w(10.00)"); for(k=0; k<NUMQ; k++) fprintf(pFile,"\t%lg", Q[k].w(10.00)); fprintf(pFile, "\n");\
   fprintf(pFile, "R(20.00)"); for(k=0; k<NUMQ; k++) fprintf(pFile,"\t%lg", Q[k].R(20.00)); fprintf(pFile, "\n");\
   fprintf(pFile, "\n");\
}
/********************************************************/
/*void DebugCheckCRecQ(void) {
   #define NUMQ 4                           // number of cases tested
   CRecQ Q[NUMQ];                           // testing class
   FILE *pFile;                             // save results to file
   int   k;                                 // loop counter

   pFile = fopen("CRecQTest.txt", "wt");    // open for text writing
   if(pFile==NULL) return;                  // ignore if file couldn't be opened

   //---Setting parameters at waist-------------
   // w0 = 50 um, R = $\infty$, $\lambda$ = 800 nm
   fprintf(pFile, "Waist Parameters\n");
   fprintf(pFile, "\tRealImag\tSet\tSetRw\tSetw0z0\n");
   Q[0].SetRealImag(0.00, -0.101859, 800.00);
   Q[1].Set(0.00, +9.81748, 800.00);
   Q[2].SetRw(0.00, 50.00, 800.00);
   Q[3].Setw0z0(50.00, 0.00, 800.00);
   fnPrintfQ(pFile, Q, k);

   //---Setting parameters at 10 mm-------------
   // w = 71.371 um, R = 19.638 mm^-1, $\lambda$ = 800 nm
   fprintf(pFile, "Parameters at 10 mm\n");
   fprintf(pFile, "\tRealImag\tSet\tSetRw\tSetw0z0\n");
   Q[0].SetRealImag(0.05092094203228, -0.04999151793828, 800.00);
   Q[1].Set(0.05092094203228, 20.00339340034940, 800.00);
   Q[2].SetRw(19.63828554793883, 71.37101863672569, 800.00);
   Q[3].Setw0z0(50.00, 10.00, 800.00);
   fnPrintfQ(pFile, Q, k);

   //---Setting parameters at 20 mm-------------
   // w = 113.4693 um, R = 24.8191 mm^-1, $\lambda$ = 800 nm
   fprintf(pFile, "Parameters at 20 mm\n");
   fprintf(pFile, "\tRealImag\tSet\tSetRw\tSetw0z0\n");
   Q[0].SetRealImag(0.04029148021377, -0.01977803410028, 800.00);
   Q[1].Set(0.04029148021377, 50.56114247399331, 800.00);
   Q[2].SetRw(24.81914277396941, 113.4693315613315, 800.00);
   Q[3].Setw0z0(50.00, 20.00, 800.00);
   fnPrintfQ(pFile, Q, k);

   fclose(pFile);
   MessageBox(NULL, "DebugCheckCRecQ has finished writing the file.", "CSysWinInventory", MB_OK);
}
*/
/*########################################################
 ## Mouse                                              ##
########################################################*/
/*********************************************************
* MouseCallback
* This callback function must be declared static.
*********************************************************/
void CSysWinInventory::_MouseCallback(int iMsg, int x, int y, int wKeys, void *pVoid, long int lData) {
   if(pVoid) ((CSysWinInventory*)pVoid)->MouseCallback(iMsg, x, y, wKeys, lData);
}
void CSysWinInventory::MouseCallback(int iMsg, int x, int y, int wKeys, long int lData) {
UNREFERENCED_PARAMETER(y);
UNREFERENCED_PARAMETER(wKeys);
   int k;                                   // column loop counter

   switch(iMsg) {
   case ACSM_DRAG:
      x += XOffset();
      for(k=0; k<(int) lData; k++) x -= ColWidth(k);
      if(x < 0) break;
      SetColWidth((int) lData, x);
      Refresh();
      break;
   case ACSM_DEND:
      OnResize();
      break;
   }
}

/*########################################################
 ## Window Methods                                     ##
########################################################*/
/*********************************************************
* WndProcSysWinInventory
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
LRESULT CALLBACK CSysWinInventory::WndProcSysWinInventory(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
   CSysWinInventory *pSWin;                 // owning object

   pSWin = (CSysWinInventory*) GetWindowLong(hWnd, 0); // not valid on WM_CREATE
   switch(uMsg) {
   case WM_CREATE:
      pSWin = (CSysWinInventory*) ((LPMDICREATESTRUCT) (((LPCREATESTRUCT)lParam)->lpCreateParams))->lParam;
      SetWindowLong(hWnd, 0, (LONG) pSWin);
      break;

   case WM_CLOSE:
      pSWin->pSystem->DeleteSysWin(pSWin);
      break;

   case WM_MDIACTIVATE:
      if(pSWin) pSWin->PrepareProperties((hWnd==(HWND)lParam) ? TRUE : FALSE);
      break;

   case WM_SIZE:  pSWin->OnResize(); return(DefMDIChildProc(hWnd, uMsg, wParam, lParam));
   case WM_PAINT: pSWin->OnPaint(); break;
   case WM_HSCROLL:
      switch(LOWORD(wParam)) {
      case SB_LEFT:       pSWin->SetXOffset(0); break;
      case SB_RIGHT:      pSWin->SetXOffset(32767); break;
      case SB_LINELEFT:   pSWin->SetXOffset(pSWin->XOffset()-8); break;
      case SB_LINERIGHT:  pSWin->SetXOffset(pSWin->XOffset()+8); break;
      case SB_PAGELEFT:   pSWin->SetXOffset(pSWin->XOffset()-80); break;
      case SB_PAGERIGHT:  pSWin->SetXOffset(pSWin->XOffset()+80); break;
      case SB_THUMBTRACK: pSWin->SetXOffset(HIWORD(wParam)); break;
      }
      pSWin->OnResize();
      InvalidateRect(hWnd, NULL, TRUE);
      break;
   case WM_VSCROLL:
      switch(LOWORD(wParam)) {
      case SB_TOP:        pSWin->SetYOffset(0); break;
      case SB_BOTTOM:     pSWin->SetYOffset(32767); break;
      case SB_LINEUP:     pSWin->SetYOffset(pSWin->YOffset()-8); break;
      case SB_LINEDOWN:   pSWin->SetYOffset(pSWin->YOffset()+8); break;
      case SB_PAGEUP:     pSWin->SetYOffset(pSWin->YOffset()-80); break;
      case SB_PAGEDOWN:   pSWin->SetYOffset(pSWin->YOffset()+80); break;
      case SB_THUMBTRACK: pSWin->SetYOffset(HIWORD(wParam)); break;
      }
      pSWin->OnResize();
      InvalidateRect(hWnd, NULL, TRUE);
      break;

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
* This will probably require scroll bars, too.
*********************************************************/
void CSysWinInventory::OnResize(void) {
   CVertex *pVx;                            // vertex loop counter
   int      iPrp;                           // mouse active loop counter
   RECT     rc, rcClient;                   // active rectangle
   int      x, y;                           // dimensions used space
   SCROLLINFO si;                           // info for scroll bars


   //===Scroll Bar========================================
   GetClientRect(hwRenderer, &rcClient);    // client area of window
   memset(&si, 0x00, sizeof(SCROLLINFO));   // clear structure
   si.cbSize = sizeof(SCROLLINFO);          // structure size
   si.fMask  = SIF_ALL;                     // update all parameters

   //---Vertical scroll bar---------------------
   y = 0;
   y += (CINI_NUMSYSROW)*iRowHeight+iRowHeight/4; // system rows
   y += 2*iRowHeight;                       // vx column headers
   for(pVx=pSystem->VxTop(); pVx; pVx=(CVertex*) pVx->Next())
      y += 2*iRowHeight;                    // each vertex
   if(YOffset() > (y-(rcClient.bottom-rcClient.top))) SetYOffset(y-(rcClient.bottom-rcClient.top));
   if(YOffset() < 0) SetYOffset(0);
   si.nMin  = 0;
   si.nMax  = y;
   si.nPage = rcClient.bottom - rcClient.top;
   si.nPos  = YOffset();
   SetScrollInfo(hwRenderer, SB_VERT, &si, TRUE);

   //---Horizontal scroll bar-------------------
   // New auto-width scaling: Ignore horizontal bar
   /*x = 0;
   for(iPrp=0; iPrp<CINI_NUMVXCOL; iPrp++) x += ColWidth(iPrp);
   if(XOffset() > (x-(rc.right-rc.left))) SetXOffset(x-(rc.right-rc.left));
   if(XOffset() < 0) SetXOffset(0);*/
   si.nMin  = 0;//0;
   si.nMax  = 0;//x;
   si.nPage = 0;//rc.right - rc.left;
   si.nPos  = 0;//XOffset();
   SetScrollInfo(hwRenderer, SB_HORZ, &si, TRUE);

   //===Actives===========================================
   //---Offset from top-------------------------
   y = (CINI_NUMSYSROW)*iRowHeight+iRowHeight/4 - YOffset();
   SetRect(&rc, -XOffset()-3, y, -XOffset()+3, y+2*iRowHeight);

   //---Set positions---------------------------
   for(x=0, iPrp=0; iPrp<CINI_NUMVXCOL; iPrp++) x += ColWidth(iPrp);
   if(x==0) x = 1;                          // prevent #DIV/0! errors
   for(iPrp=0; iPrp<CINI_NUMVXCOL; iPrp++) {
      SetRect(&rc, rc.left+ColWidth(iPrp)*(rcClient.right-rcClient.left)/x, rc.top,
         rc.right+ColWidth(iPrp)*(rcClient.right-rcClient.left)/x, rc.bottom);
      Mouse.FindActiveByData((void*) this, iPrp)->SetActiveRect(&rc);
   }

}


/*********************************************************
* Print
*********************************************************/
void CSysWinInventory::Print(HDC hdcPrint) {
   HFONT hfOld;                             // restore font
   int iPrintWid, iPrintHig;                // print page size (pixels)
   RECT  rc;
   SIZE  sz;                                // size for row height
   int   iPrp;                              // loop counter
   int   iRowHeightOld;                     // restore row height
   int   iColWidthOld[CINI_NUMVXCOL];       // restore column widths
   int   iXOffsetOld, iYOffsetOld;          // restore offsets

   //===Prepare===========================================
   if(hdcPrint==NULL) return;               // ignore bad calls
   //---Keep old values-------------------------
   iRowHeightOld = iRowHeight;
   iXOffsetOld = XOffset();
   iYOffsetOld = YOffset();
   for(iPrp=0; iPrp<CINI_NUMVXCOL; iPrp++) iColWidthOld[iPrp] = ColWidth(iPrp);
   hfOld = (HFONT) SelectObject(hdcPrint, App()->Font(CAPPI_FONTPRINT));

   //---Page Size-------------------------------
   iPrintWid = GetDeviceCaps(hdcPrint, HORZRES); // print width (pixels)
   iPrintHig = GetDeviceCaps(hdcPrint, VERTRES); // print height (pixels)
   SetXOffset(-0.10 * iPrintWid);
   SetYOffset(-0.15 * iPrintHig);

   //---Row height------------------------------
   GetTextExtentPoint32(hdcPrint, "X", 1, &sz);
   iRowHeight = (int) (1.20 * sz.cy);            // row height just larger than X height

   //---Column width----------------------------
   sz.cx = 0;
   for(iPrp=0; iPrp<CINI_NUMVXCOL; iPrp++) sz.cx += ColWidth(iPrp);
   if(sz.cx == 0) sz.cx = 1;
   for(iPrp=0; iPrp<CINI_NUMVXCOL; iPrp++) SetColWidth(iPrp, 0.80*iPrintWid*ColWidth(iPrp)/sz.cx);

   //===Paint=============================================
   OnPaint(hdcPrint);

   //===Finish============================================
   SelectObject(hdcPrint, hfOld);

   //---Restore screen values-------------------
   iRowHeight = iRowHeightOld;
   SetXOffset(iXOffsetOld);
   SetYOffset(iYOffsetOld);
   for(iPrp=0; iPrp<CINI_NUMVXCOL; iPrp++) SetColWidth(iPrp, iColWidthOld[iPrp]);

}

/*********************************************************
* Paint
* If hdcUse is non-NULL, it is assumed that the function
* is being called for a print DC
*********************************************************/
void CSysWinInventory::OnPaint(HDC hdcPrint) {
   char szBuf[256];                         // text buffer
   PAINTSTRUCT ps;                          // painting structure
   HDC      hdcMem;                         // memory device context
   HBITMAP  hbmMem, hbmOld;                 // memory painting
   HDC      hdc;                            // device context to use
   RECT     rcClient, rc;                   // painting rectangle
   HPEN     hpOld;                          // restore pen
   HFONT    hfOld;                          // restore font
   COLORREF rgbTxOld, rgbBkOld;             // restore background color

   CVertex *pVx;                            // vertex loop counter
   int      iPrp;                           // property loop counter
   int      iRow, iCol;                     // double-row loop counter
   int      iScrY;                          // row position
   int      iTotalWid;                      // total width of columns


   //===Preliminaries=====================================
   //---Screen painting-------------------------
   if(hdcPrint==NULL) {
      BeginPaint(hwRenderer, &ps);
      GetClientRect(hwRenderer, &rcClient); // client area
      for(iPrp=0, iTotalWid=0; iPrp<CINI_NUMVXCOL; iPrp++) iTotalWid += ColWidth(iPrp);
      if(iTotalWid==0) iTotalWid = 1;          // prevent #DIV/0! errors
      hdc = ps.hdc;                         // default context
      do {
         if( (hdcMem=CreateCompatibleDC(ps.hdc)) == NULL) break;
         if( (hbmMem=CreateCompatibleBitmap(ps.hdc, rcClient.right-rcClient.left, rcClient.bottom-rcClient.top)) == NULL) break;
         hbmOld = (HBITMAP) SelectObject(hdcMem, hbmMem);
         hdc = hdcMem;
      } while(0);
      hfOld = (HFONT) SelectObject(hdc, App()->Font());
      FillRect(hdc, &rcClient, App()->Brush()); // clear window backrgound

   //---Printing--------------------------------
   // Due to the new scaling in the window, we have to be
   // a bit crafty during printing.
   } else {
      hdc = hdcPrint;
      for(iPrp=0, iTotalWid=0; iPrp<CINI_NUMVXCOL; iPrp++) iTotalWid += ColWidth(iPrp);
      SetRect(&rcClient, 0, 0, iTotalWid, 1);
   }
   hpOld =  (HPEN) SelectObject(hdc, App()->Pen(CAPPI_PEN3DDARK));
   rgbTxOld = SetTextColor(hdc, App()->Rgb(CAPPI_RGB3DTEXT));
   rgbBkOld =   SetBkColor(hdc, App()->Rgb(CAPPI_RGB3DFACE));
   iScrY = -YOffset();                       // start of top row

   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   // System
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   double   dVal;

   for(iPrp=0; iPrp<CINI_NUMSYSROW; iPrp++) {
      for(iCol=0; iCol<3; iCol++) {
         SetRect(&rc, -XOffset()+iCol*(rcClient.right-rcClient.left)/3, iScrY,
            -XOffset()+(iCol+1)*(rcClient.right-rcClient.left)/3, iScrY+iRowHeight);
         sprintf(szBuf, "");                // default: no text
         //---Headers---------------------------
         if(iCol==0) {
            sprintf(szBuf, "%s", CszInvSysRow[iPrp]);
            SetTextColor(hdc, App()->Rgb(CAPPI_RGB3DTEXT));
            SetBkColor(hdc, App()->Rgb(CAPPI_RGB3DFACE));
            ExtTextOut(hdc, rc.left+2, rc.top+1, ETO_CLIPPED | ETO_OPAQUE, &rc, szBuf, strlen(szBuf), NULL);
            SelectObject(hdc, App()->Pen(CAPPI_PEN3DLITE));
            MoveTo(hdc, rc.left, rc.bottom-1);
            LineTo(hdc, rc.left, rc.top);
            LineTo(hdc, rc.right-1, rc.top);
            SelectObject(hdc, App()->Pen(CAPPI_PEN3DDARK));
            MoveTo(hdc, rc.right-1, rc.top);
            LineTo(hdc, rc.right-1, rc.bottom-1);
            MoveTo(hdc, rc.left, rc.bottom-1);
            LineTo(hdc, rc.right-1, rc.bottom-1);

         //---Values----------------------------
         } else {
            switch(iPrp) {
            case CINI_SYSWLEN:
               if(iCol==1) sprintf(szBuf, "%lg", pSystem->WLen());
               break;
            case CINI_SYSMSQUARED:
               sprintf(szBuf, "%lg", pSystem->MSquared((iCol==1) ? SAG : TAN));
               break;
            case CINI_SYSLENGTH:
               if(pSystem->DraftMode()) break;
               sprintf(szBuf, "%lg", (iCol==1) ? pSystem->PhysicalLength() : pSystem->OpticalLength());
               break;
            case CINI_SYSMODESPACING:
               if(pSystem->DraftMode()) break;
               if(iCol==1) sprintf(szBuf, "%lg", pSystem->ModeSpacing());
               break;
            case CINI_SYSSTAB:
               if(pSystem->DraftMode()) break;
               pSystem->FcnValue(CSYSI_PROP_STABSAG, (iCol==1)?&dVal:NULL, (iCol==1)?NULL:&dVal); sprintf(szBuf, "%lg", dVal);
               break;
            }
            SetTextColor(hdc, App()->Rgb(CAPPI_RGBBLACK));
            SetBkColor(hdc, App()->Rgb(CAPPI_RGBWINDOW));
            ExtTextOut(hdc, rc.left+2, rc.top+1, ETO_CLIPPED | ETO_OPAQUE, &rc, szBuf, strlen(szBuf), NULL);
            SelectObject(hdc, App()->Pen());
            MoveTo(hdc, rc.left, rc.bottom-1);
            LineTo(hdc, rc.right-1, rc.bottom-1);
            LineTo(hdc, rc.right-1, rc.top-1);
         }
         //---Print value-----------------------
      }
      iScrY += iRowHeight;
   }
   iScrY += iRowHeight/4;
//*/
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   // Vertices
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   //===Headings==========================================
   SetRect(&rc, -XOffset(), iScrY, -XOffset(), iScrY);
   SetTextColor(hdc, App()->Rgb(CAPPI_RGB3DTEXT));
   SetBkColor(hdc, App()->Rgb(CAPPI_RGB3DFACE));
   //---Titles----------------------------------
   for(iRow=0; iRow<2; iRow++) {
      SetRect(&rc, -XOffset(), rc.bottom, -XOffset(), rc.bottom+iRowHeight);
      for(iPrp=0; iPrp<CINI_NUMVXCOL; iPrp++) {
         SetRect(&rc, rc.right, rc.top, rc.right+ColWidth(iPrp)*(rcClient.right-rcClient.left)/iTotalWid, rc.bottom);
         if(iPrp==CINI_NUMVXCOL-1) rc.right = rcClient.right-XOffset();
         sprintf(szBuf, "%s", CszInvVxColumn[2*iPrp+iRow]);
         ExtTextOut(hdc, rc.left+2, rc.top+1, ETO_CLIPPED | ETO_OPAQUE, &rc, szBuf, strlen(szBuf), NULL);
         //---Border---
         if(iRow > 0) {
            SelectObject(hdc, App()->Pen(CAPPI_PEN3DLITE));
            MoveTo(hdc, rc.left, rc.bottom-1);
            LineTo(hdc, rc.left, iScrY);
            LineTo(hdc, rc.right-1, iScrY);
            SelectObject(hdc, App()->Pen(CAPPI_PEN3DDARK));
            MoveTo(hdc, rc.right-1, iScrY);
            LineTo(hdc, rc.right-1, rc.bottom-1);
            MoveTo(hdc, rc.left, rc.bottom-1);
            LineTo(hdc, rc.right-1, rc.bottom-1);
         }
      }
   }
   //---Filler----------------------------------
   /*if((hdcPrint==NULL) && (rc.right < rcClient.right)) {
      SetRect(&rc, rc.right, iScrY, rcClient.right, iScrY+2*iRowHeight);
      FillRect(hdc, &rc, App()->Brush(CAPPI_BRUSH3DFACE));
      SelectObject(hdc, App()->Pen(CAPPI_PEN3DDARK));
      MoveTo(hdc, rc.left, rc.bottom-1);
      LineTo(hdc, rc.right, rc.bottom-1);
   }//*/
   iScrY += 2*iRowHeight;

   //---Top of table----------------------------
   SelectObject(hdc, App()->Pen());

   //===Properties========================================
   SetTextColor(hdc, App()->Rgb(CAPPI_RGBBLACK));
   SetBkColor(hdc, App()->Rgb(CAPPI_RGBWINDOW));
   for(pVx=pSystem->VxTop(); pVx; pVx=(CVertex*) pVx->Next()) {
      for(iRow=0; iRow<2; iRow++) {
         SetRect(&rc, -XOffset(), iScrY+iRow*iRowHeight, -XOffset(), iScrY+(iRow+1)*iRowHeight);
         for(iPrp=0; iPrp<CINI_NUMVXCOL; iPrp++) {
            SetRect(&rc, rc.right, rc.top, rc.right+ColWidth(iPrp)*(rcClient.right-rcClient.left)/iTotalWid, rc.bottom);
            if(iPrp==CINI_NUMVXCOL-1) rc.right = rcClient.right-XOffset();
            sprintf(szBuf, "");
            switch(iPrp) {
            case CINI_VXCOLTAG:    sprintf(szBuf, "%s",  (iRow==0) ? pVx->TypeString() : pVx->Tag());  break;
            case CINI_VXCOLCOORD:  sprintf(szBuf, "%lg", (iRow==0) ? pVx->X()          : pVx->Y());    break;
            case CINI_VXCOLROCFL:
               if((iRow > 0) && !(pVx->AstigROCFL())) break; // ignore second row if not astigmatic
               switch(pVx->Type()) {
               case CVX_TYPE_MIRROR:
               case CVX_TYPE_INCRYSTAL:
               case CVX_TYPE_OUTCRYSTAL:
               case CVX_TYPE_INBREWSTER:
               case CVX_TYPE_OUTBREWSTER:
               case CVX_TYPE_INPLATE:
               case CVX_TYPE_OUTPLATE:
               case CVX_TYPE_OUTCOUPLER:
                  sprintf(szBuf, "%lg", pVx->ROC((iRow==0) ? SAG : TAN));
                  break;
               case CVX_TYPE_LENS:
               case CVX_TYPE_THERMALLENS:
                  sprintf(szBuf, "%lg", pVx->FL((iRow==0) ? SAG : TAN));
                  break;
               }
               break;
            case CINI_VXCOLANGLE:
               if(iRow>0) break;            // first row only
               switch(pVx->Type()) {
               case CVX_TYPE_MIRROR:
               case CVX_TYPE_INCRYSTAL:
               case CVX_TYPE_INPLATE:
               case CVX_TYPE_FLATMIRROR:
                  if(pSystem->DraftMode()) break;
                  if(iRow==0) sprintf(szBuf, "%lg", pVx->FaceAngle()*180.00/M_PI);
                  break;
               }
               break;
            case CINI_VXCOLDISTN:
               if((iRow==0) && (pSystem->DraftMode())) break;
               if(iRow==0) { if(pVx->Next()) sprintf(szBuf, "%lg", pVx->Dist2Next()); }
               else switch(pVx->Type()) {
                  case CVX_TYPE_INCRYSTAL:
                  case CVX_TYPE_INBREWSTER:
                  case CVX_TYPE_INPLATE:
                  case CVX_TYPE_OUTCOUPLER:
                     sprintf(szBuf, "%lg", pVx->RefIndex());
                     break;
                  case CVX_TYPE_PRISM1:
                  case CVX_TYPE_PRISM2:
                     sprintf(szBuf, "%lg", pVx->_Prop(CVXI_PROP_N)); break;
               }
               break;
            case CINI_VXCOLMODE:
            case CINI_VXCOLCURV:
               if(pSystem->DraftMode()) break;
               if(pSystem->StableABCD((iRow==0)?SAG:TAN))
                  sprintf(szBuf, "%lg",
                     (iPrp==CINI_VXCOLMODE) ? pVx->Q((iRow==0)?SAG:TAN)->W() :
                     (iPrp==CINI_VXCOLCURV) ? pVx->Q((iRow==0)?SAG:TAN)->R() :
                     0.00);
               else sprintf(szBuf, "---");
               break;
            }
            ExtTextOut(hdc, rc.left+2, rc.top+1, ETO_CLIPPED|ETO_OPAQUE, &rc, szBuf, strlen(szBuf), NULL);

            //---Divider---------------------------
            if(iRow > 0) {
               MoveTo(hdc, rc.left, rc.bottom-1);
               LineTo(hdc, rc.right-1, rc.bottom-1);
               LineTo(hdc, rc.right-1, iScrY-1);
            }

         }//endfor(iPrp)
      }//endfor(iRow)
      iScrY += 2 * iRowHeight;
   }//endfor(pVx)

   //===Finalize==========================================
//Mouse.PaintAllActiveRect(hdc);
   SetTextColor(hdc, rgbTxOld);
   SetBkColor(hdc, rgbBkOld);
   SelectObject(hdc, hpOld);                // restore pen

   //---Screen painting-------------------------
   if(hdcPrint==NULL) {
      SelectObject(hdc, hfOld);                // restore font
      if(hbmMem) {
         BitBlt(ps.hdc, rcClient.left, rcClient.top, rcClient.right-rcClient.left, rcClient.bottom-rcClient.top,
            hdcMem, 0, 0, SRCCOPY);
         SelectObject(hdcMem, hbmOld);
      }
      if(hbmMem) DeleteObject(hbmMem); hbmMem = NULL;
      if(hdcMem) DeleteDC(hdcMem); hdcMem = NULL;
      EndPaint(hwRenderer, &ps);
   }
}
