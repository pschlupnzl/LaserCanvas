/*********************************************************
* CSysWinVxGraph
* Vertex property graph. This plots the behaviour of sys-
* tem properties such optical length against variable va-
* lues.
* Most (~98%!) of this code is the same as that for CSys-
* WinGraph, the system property renderer. However, in the
* spirit of the Renderers, I've made a completely second
* class despite the duplication. The hope is that this is
* "safer" for future adaptations.
* $PSchlup 2006 $     $Revision 0 $
*********************************************************/
#include "CSysWinVxGraph.h"                 // class header

/*********************************************************
* Constructor
* In order to be able to  load this renderer from a file,
* we allow pVx to be NULL, so must  pass the system sepa-
* rately.
*********************************************************/
CSysWinVxGraph::CSysWinVxGraph(CSystem *pSys, CVertex *pVx) : CSysWin(pSys) {
printf("+ CSysWinVxGraph created 0x%08lx\n",this);
   char szBuf[256];                         // string table buffer
   int k;                                   // loop counter

   //===Members===========================================
   pVertex = pVx;                           // vertex associated with renderer

   for(k=0; k<CGVXI_NUM_PROP; k++) ddProp[k] = 0.00;
   for(k=0; k<CGVXI_NUM_RECT; k++) SetRect(&rcRect[k], 0, 0, 0, 0);
   hmAxesPopup = NULL;                      // no menu yet
   pQEdit      = NULL;                      // no quick controls yet
   pQMenu      = NULL;
   pdXData     = NULL;                      // no data allocated yet
   pdSData     = NULL;
   pdTData     = NULL;
   iDataPts    = 0;

   //---Hook variable-----------------
   App()->AddVarHook(RunVar()); // insert default hook

   //---Default axes limits-----------
   Axes.SetBoxOn();
   UserSetXLim(App()->VarMin(RunVar()), App()->VarMax(RunVar()));
   UserSetYLim(0.00, 500.00);

   //---Mouse-------------------------
   hmAxesPopup = CreatePopupMenu();         // create (empty) menu
   LoadString(App()->GetInstance(), CMU_POPUP_COPYDATA, szBuf, sizeof(szBuf)/sizeof(char));
   AppendMenu(hmAxesPopup, MF_STRING, CMU_POPUP_COPYDATA, szBuf);
   Mouse.CreateActive(0, 0, 0, 0, LoadCursor(NULL, IDC_CROSS ), NULL, NULL, NULL, CSysWinVxGraph::_MouseCallback, (void*) this, CGVX_ACT_AXES,   hmAxesPopup);
   Mouse.CreateActive(0, 0, 0, 0, LoadCursor(NULL, IDC_IBEAM ), NULL, NULL, NULL, CSysWinVxGraph::_MouseCallback, (void*) this, CGVX_ACT_XMIN,   NULL);
   Mouse.CreateActive(0, 0, 0, 0, LoadCursor(NULL, IDC_IBEAM ), NULL, NULL, NULL, CSysWinVxGraph::_MouseCallback, (void*) this, CGVX_ACT_XMAX,   NULL);
   Mouse.CreateActive(0, 0, 0, 0, LoadCursor(NULL, IDC_IBEAM ), NULL, NULL, NULL, CSysWinVxGraph::_MouseCallback, (void*) this, CGVX_ACT_YMIN,   NULL);
   Mouse.CreateActive(0, 0, 0, 0, LoadCursor(NULL, IDC_IBEAM ), NULL, NULL, NULL, CSysWinVxGraph::_MouseCallback, (void*) this, CGVX_ACT_YMAX,   NULL);
   Mouse.CreateActive(0, 0, 0, 0, LoadCursor(NULL, IDC_HAND  ), NULL, NULL, NULL, CSysWinVxGraph::_MouseCallback, (void*) this, CGVX_ACT_RUNVAR, NULL);
   Mouse.CreateActive(0, 0, 0, 0, LoadCursor(NULL, IDC_HAND  ), NULL, NULL, NULL, CSysWinVxGraph::_MouseCallback, (void*) this, CGVX_ACT_PLTFCN, NULL);
   Mouse.CreateActive(0, 0, 0, 0, LoadCursor(NULL, IDC_SIZEWE), NULL, NULL, NULL, CSysWinVxGraph::_MouseCallback, (void*) this, CGVX_ACT_VARVAL, NULL);

   //===Create window=====================================
   hwRenderer = pSystem->CreateSysWinWindow(CSZ_WNDCLASS_SYSWINVXGRAPH, (LPVOID) this);

   //---Quick Edit control------------
   pQEdit = new CQuickEdit(hwRenderer);     // create the quick-edit control
   if(pQEdit) {
      pQEdit->SetCallback(CSysWinVxGraph::_QEditCallback); // set the callback function
      pQEdit->SetFont(App()->Font());
   }

   //---Quick menu control------------
   pQMenu = new CQuickMenu(hwRenderer);     // create the quick-menu control
   if(pQMenu) {
      pQMenu->SetCallback(CSysWinVxGraph::_QMenuCallback); // set the callback function
      pQMenu->SetFont(App()->Font());
   }

   OnResize();                              // force resize, since not automatic!
}

/*********************************************************
* Destructor
*********************************************************/
CSysWinVxGraph::~CSysWinVxGraph() {
   //---Unhook variable-------------------------
   App()->RemVarHook(RunVar());

   //---Quick controls--------------------------
   if(pQEdit) delete(pQEdit); pQEdit = NULL;
   if(pQMenu) delete(pQMenu); pQMenu = NULL;
   if(hmAxesPopup) DestroyMenu(hmAxesPopup); hmAxesPopup = NULL;

   //---Data------------------------------------
   if(pdXData) free(pdXData); pdXData = NULL;
   if(pdSData) free(pdSData); pdSData = NULL;
   if(pdTData) free(pdTData); pdTData = NULL;
   iDataPts = 0;
printf("- CSysWin -1D- Destroyed\n");
}


/*########################################################
 ## Data Files
########################################################*/
/*********************************************************
* SaveSysWin
* Save the current renderer
*********************************************************/
const char* CSysWinVxGraph::CszSysWinVxGraphProp[] = {"XMin", "XMax", "YMin", "YMax", "Variable", "Function"};

void CSysWinVxGraph::SaveSysWin(HANDLE hFile) {
   char szBuf[256];                         // string to write to buffer
   DWORD dwBytes;                           // number of bytes written
   int   iPrp;                              // property loop counter

   if(hFile == NULL) printf("? CSysWinVxGraph::SaveSysWin@318 Called with hFile=NULL\n");
   if(hFile == NULL) return;                // ignore if no file

   //---Header------------------------
   SaveSysWinHeader(hFile, Type());         // write the header

   //---Properties--------------------
   sprintf(szBuf, "   Vertex = %s\r\n", pVertex->Tag()); // write vertex handle
   WriteFile(hFile, szBuf, strlen(szBuf), &dwBytes, NULL);
   for(iPrp=0; iPrp<CGVXI_NUM_PROP; iPrp++) {
      sprintf(szBuf, "   %s = ", CszSysWinVxGraphProp[iPrp]);
      switch(iPrp) {
      case CGVXI_PROP_RUNVAR:
         sprintf(szBuf+strlen(szBuf), "%s", App()->VarString(RunVar()));
         break;
      case CGVXI_PROP_PLTFCN:
         sprintf(szBuf+strlen(szBuf), "%s", CVertex::CszVxSaveFcnString[PltFcn()]);
         break;
      default:
         sprintf(szBuf+strlen(szBuf), "%lg", ddProp[iPrp]);
         break;
      }
      sprintf(szBuf+strlen(szBuf), "\r\n");
      WriteFile(hFile, szBuf, strlen(szBuf), &dwBytes, NULL);
   }

   //---Footer------------------------
   SaveSysWinFooter(hFile);                 // write the footer
}


/*********************************************************
* LoadSysWin
* Load renderer properties from data file
*********************************************************/
BOOL CSysWinVxGraph::LoadSysWin(const char *pszDataFile, char *pszMin, char *pszMax) {
   char    *pszVx;                          // pointer to vertex tag
   CVertex *pVx;                            // vertex pointed to
   char    *psz;                            // pointer in file
   int      iVar;                           // variable loop counter
   int      iPrp;                           // property loop counter

   if(pszMin==NULL) return(FALSE);          // couldn't load if no pointer
   if(pszMax==NULL) pszMax = pszMin + strlen(pszMin); // to end of file if not given
   App()->RemVarHook(RunVar()); // remove the default hook

   //---Header------------------------
   LoadSysWinHeader(pszDataFile, pszMin, pszMax);

   //---Find vertex pointer-----------
   pszVx = strstr(pszMin, "Vertex");        // search for vertex declarator
   pszVx = strchr(pszVx, '=');              // skip ahead to the '=' sign
   if(pszVx) pszVx += strcspn(pszVx, CSystem::CszValidNameChar); // skip to start of tag
   if((pszVx==NULL) || (pszVx>pszMax)) {    // any problems finding tag?
      App()->LoadErrorMsg(SZERR_LOAD_GRPHBADVX, pszDataFile, pszMin);
      return(FALSE);
   }
   pVx = pSystem->FindVxByTag(pszVx);       // find the vertex by its tag
   if(pVx==NULL) {
      App()->LoadErrorMsg(SZERR_LOAD_NOGRPHVX, pszDataFile, pszVx);
      return(FALSE);
   }
   if(pVx->SysParent() != pSystem) {           // ensure vertex is in THIS system - not spawned
      App()->LoadErrorMsg(SZERR_LOAD_GRPHQSYS, pszDataFile, pszVx);
      return(FALSE);
   }

   //---Save vertex pointer-----------
   pVertex = pVx;                           // this is the vertex we're all about

   //---Properties--------------------
   for(iPrp=0; iPrp<CGVXI_NUM_PROP; iPrp++) {
      psz = strstr(pszMin, CszSysWinVxGraphProp[iPrp]);
      if((psz==NULL) || (psz>pszMax)) continue;
      psz = strchr(psz, '=');               //..find '=' sign
      if((psz==NULL) || (psz>pszMax)) continue;
      switch(iPrp) {
      case CGVXI_PROP_RUNVAR:               // search for variable string
         do { psz++; } while(*psz==' ');    // skip blanks
         for(iVar=0; iVar<App()->NumVars(); iVar++) {
            if(strncmp(psz, App()->VarString(iVar),
               strlen(App()->VarString(iVar))) == 0) {
                  ddProp[CGVXI_PROP_RUNVAR] = (double) iVar;
                  break;
            }
         }
         break;
      case CGVXI_PROP_PLTFCN:               // search for plotting functino string
         do { psz++; } while(*psz==' ');    // skip blanks
         for(iVar=0; iVar<CVertex::NumVxFcn(); iVar++) {
            if(strncmp(psz, CVertex::CszVxSaveFcnString[iVar],
               strlen(CVertex::CszVxSaveFcnString[iVar])) == 0) {
                  ddProp[CGVXI_PROP_PLTFCN] = (double) iVar;
                  break;
            }
         }
         break;
      default:
         sscanf(psz+1, "%lg", &ddProp[iPrp]);
         break;
      }
   }

   //---Running variable--------------
   if((int)ddProp[CGVXI_PROP_RUNVAR] < 0) ddProp[CGVXI_PROP_RUNVAR] = (double) 0;
   if((int)ddProp[CGVXI_PROP_RUNVAR] > (App()->NumVars()-1))
      ddProp[CGVXI_PROP_RUNVAR] = (double) (App()->NumVars()-1);
   App()->AddVarHook(RunVar()); // add hook for this variable

   //---Update------------------------
   UserSetXLim(XMin(), XMax());             // apply loaded axes limits
   UserSetYLim(YMin(), YMax());
   OnResize();

   return(TRUE);
}

/*########################################################
 ## Overloaded Functions                               ##
########################################################*/
/*********************************************************
* Refresh
*********************************************************/
void CSysWinVxGraph::Refresh(void) {
   if(hwRenderer) InvalidateRect(hwRenderer, NULL, TRUE);
}


/*********************************************************
* UpdateTitle
*********************************************************/
void CSysWinVxGraph::UpdateTitle(int iID) {
   char szRC[128];                          // string from resource file
   char szBuf[256];                         // formatted string
   LoadString(App()->GetInstance(), SZ_TITLE_SYSWINVXGRAPH, szRC, sizeof(szRC)/sizeof(char));
   sprintf(szBuf, szRC, (pSystem) ? ((pSystem->FileName()) ? pSystem->FileName() : "") : "");
   if(iID > 0) sprintf(szBuf+strlen(szBuf), " (%d)", iID);
   if(hwRenderer) SetWindowText(hwRenderer, szBuf);
}

/*********************************************************
* GraphPoint
* When setting the first  data point of our running vari-
* able we check that the allocated arrays are big enough.
* (We could also implement a function to allocate the me-
* mory whenever the point count changes, but this is may-
* be more robust and  in any case doesn't cost much over-
* head.)
*********************************************************/
void CSysWinVxGraph::GraphPoint(int iRVar, int iPt) {
   int iNPts;                               // number of points

   //===Standard Configuration============================
   if(iRVar < 0) {                          // called after scan with standard config
      RefreshDraggerActive();               // call to update the dragger's position
      return;
   }

   //===Sweeping==========================================
   if(iRVar != RunVar()) return;            // ignore if it's not our variable

   //---Check array size------------------------
   if(iPt==0) {
      iNPts = App()->VarPoints(iRVar); // number of points that app will scan
      if(iDataPts != iNPts) {
         if(pdXData) free(pdXData); pdXData = NULL;
         if(pdSData) free(pdSData); pdSData = NULL;
         if(pdTData) free(pdTData); pdTData = NULL;
         pdXData = (double*) malloc(iNPts * sizeof(double));
         pdSData = (double*) malloc(iNPts * sizeof(double));
         pdTData = (double*) malloc(iNPts * sizeof(double));
         iDataPts = ((pdXData!=NULL) && (pdSData!=NULL) && (pdTData!=NULL)) ? iNPts : 0;
      }
   }

   //---Data------------------------------------
   if(pdXData) {                            // point X coordinate
      pdXData[iPt] = App()->Var(iRVar);
   }
   pVertex->FcnValue(
      PltFcn(),
      (pdSData!=NULL) ? &pdSData[iPt] : NULL,
      (pdTData!=NULL) ? &pdTData[iPt] : NULL);
}

/*########################################################
 ## Properties                                         ##
########################################################*/
/*********************************************************
* PrepareProperties
*********************************************************/
const UINT CSysWinVxGraph::CuProperties[] = {
   CPS_GRPHEADER     , // "Graph"
   //CPS_GRPSYSFCN     , // "System Property"
   CPS_GRPVXFCN      , // "Vertex Property"
   CPS_GRPAXXRNG     , // "X-Axis Range"
   CPS_GRPAXYRNG     , // "Y-Axis Range"
   CPS_GRPNUMSTP     , // "Points"
   CPS_GRPRUNVAR     , // "Ordinate Variable"
   //CPS_GRPSHOWICONS  , // "Show Icons"
   0};
//=======================================================
void CSysWinVxGraph::PrepareProperties(BOOL tfAct) {
   //---Show / hide-----------------------------
   App()->PrepareVariableProperties(tfAct);
   App()->PrepareProperties(CuProperties, tfAct, CSysWinVxGraph::_SysWinPropItemCallback, this);
   App()->EnableMenus(tfAct ? CSYSWINI_TYPE_VXGRAPH : -1);
   pSystem->PrepareProperties(tfAct);

   if(tfAct) UpdateProperties();            // refresh the values
}

/*********************************************************
* UpdateProperties
* We don't really need to update this every time but it's
* not much time and easier that figuring out what happens
* and where. See comments CSysWinGraph::UpdateProperties.
* Called from:
* <- PrepareProperties
* <- CApp::ScanAll (if current renderer)
* <- UserSetXLim, UserSetYLim
*********************************************************/
void CSysWinVxGraph::UpdateProperties(void) {
   CPropMgr *pMgr = App()->PropManager();   // assign for code below
   if(pMgr==NULL) return;                   // ignore if no manager
   pMgr->FindItemByID(CPS_GRPVXFCN )->SetItemRadioList(PltFcn(), CVertex::CszVxFcnNames);
   pMgr->FindItemByID(CPS_GRPRUNVAR)->SetItemRadioList(RunVar(), App()->VarsString());
   pMgr->FindItemByID(CPS_GRPAXXRNG)->SetItemDblValue(XMin(), XMax());
   pMgr->FindItemByID(CPS_GRPAXYRNG)->SetItemDblValue(YMin(), YMax());
   pMgr->FindItemByID(CPS_GRPNUMSTP)->SetItemValue(App()->VarPoints(RunVar()));
}


/*********************************************************
* SysWinPropItemCallback
* Callback from property manager when an item is modified
* The callback function must be declared static.
*********************************************************/
BOOL CSysWinVxGraph::_SysWinPropItemCallback(void *vData, UINT uID, void *pVoid) {
   return((pVoid) ? ((CSysWinVxGraph*)pVoid)->SysWinPropItemCallback(vData, uID) : TRUE);
}
//******************************************************//
BOOL CSysWinVxGraph::SysWinPropItemCallback(void *vData, UINT uID) {
   switch(uID) {
   case CPS_GRPVXFCN:  UserSetPltFcn(*(int*) vData); break;
   case CPS_GRPRUNVAR: UserSetRunVar(*(int*) vData); break;
   case CPS_GRPAXXRNG: UserSetXLim( ((double*)vData)[0], ((double*)vData)[1] ); Refresh(); break;
   case CPS_GRPAXYRNG: UserSetYLim( ((double*)vData)[0], ((double*)vData)[1] ); Refresh(); break;
   case CPS_GRPNUMSTP:
      App()->SetVarPoints(RunVar(), (int)(*(double*) vData));
      App()->ScanAll();
      return(TRUE);
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
* DebugPrint
*********************************************************/
void CSysWinVxGraph::DebugPrint(char *psz, int *pInt) {
   for(int k=0; k<*pInt; k++) sprintf(psz+strlen(psz), "   ");
   sprintf(psz+strlen(psz), "CSysWin-VxGraph 0x%08lx for vertex <%s>\n",
      (long) this, (pVertex) ? pVertex->Tag() : "NULL");
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
void CSysWinVxGraph::UserSetXLim(double dMn, double dMx) {
   ddProp[CGVXI_PROP_XMIN] = (dMn < dMx) ? dMn : dMx;
   ddProp[CGVXI_PROP_XMAX] = (dMn < dMx) ? dMx : dMn;
   Axes.SetAxDataLimit(AX_XAXIS, ddProp[CGVXI_PROP_XMIN], ddProp[CGVXI_PROP_XMAX]);
   Axes.CalcAxTicks(AX_XAXIS);              // re-calculate ticks
   RefreshDraggerActive();                  // call to update the dragger's position
   UpdateProperties();
}

//===Y Limits=============================================
void CSysWinVxGraph::UserSetYLim(double dMn, double dMY) {
   ddProp[CGVXI_PROP_YMIN] = (dMn < dMY) ? dMn : dMY;
   ddProp[CGVXI_PROP_YMAX] = (dMn < dMY) ? dMY : dMn;
   Axes.SetAxDataLimit(AX_YAXIS, ddProp[CGVXI_PROP_YMIN], ddProp[CGVXI_PROP_YMAX]);
   Axes.CalcAxTicks(AX_YAXIS);
   UpdateProperties();
}

/*********************************************************
* RunVar
* These functions maintain the CGVXI_PROP_RUNVAR property
* and update the relevant hooks in the parent application
* variable list
*********************************************************/
//===RunVar===============================================
int CSysWinVxGraph::RunVar(void) {
   return((int) ddProp[CGVXI_PROP_RUNVAR]);
};
//===UserSetRunVar========================================
void CSysWinVxGraph::UserSetRunVar(int iVar) {
   if((iVar<0) || (iVar>=App()->NumVars())) return; // ignore out of bounds

   //---Change variable---------------
   App()->RemVarHook(RunVar());             // remove previous hook
   ddProp[CGVXI_PROP_RUNVAR] = (double) iVar; // set variable
   App()->AddVarHook(RunVar());             // add new hook
   //---Scan and update---------------
   App()->ScanAll();                        // refresh all data
   SetAutoLimits(AX_XAXIS);                 // zoom plot to whole data
}
//===PltFcn===============================================
int CSysWinVxGraph::PltFcn(void) {
   return((int) ddProp[CGVXI_PROP_PLTFCN]);
}
//===UserSetPltFcn========================================
void CSysWinVxGraph::UserSetPltFcn(int iFcn) {
   ddProp[CGVXI_PROP_PLTFCN] = (double) iFcn; // set plot function
   App()->ScanAll();                        // refresh all data
   SetAutoLimits(AX_YAXIS);                 // zoom plot to whole data
}

/*********************************************************
* SetAutoLimits
* Updates the  (internal ddProp) axis limits to  span all
* variable ranged data. Call this  when the running vari-
* able or plot function is changed. It's a bit cumbersome
* because the number of lines depends on plot function.
* Called from
* <- UserSetRunVar
* <- UserSetPltFcn
*********************************************************/
void CSysWinVxGraph::SetAutoLimits(int iAx) {
   int k;                                   // loop counter
   double dMin, dMax;                       // limits on change
   //---X Limits----------------------
   if(((iAx<0) || (iAx==AX_XAXIS)) && (pdXData)) {
      for(dMin=dMax=pdXData[0], k=1; k<iDataPts; k++) {
         if(pdXData[k] < dMin) dMin = pdXData[k];
         if(pdXData[k] > dMax) dMax = pdXData[k];
      }
      UserSetXLim(dMin, dMax);
   }

   //---Y Limits----------------------
   // Not so useful. See CSysWinGraph::SetAutoLimits.
}

/*########################################################
 ## Mouse / Quick Controls                             ##
########################################################*/
/*********************************************************
* MouseCallback
* This callback function must be declared static.
*********************************************************/
void CSysWinVxGraph::_MouseCallback(int iMsg, int x, int y, int wKeys, void *pVoid, long int lData) {
   if(pVoid) ((CSysWinVxGraph*)pVoid)->MouseCallback(iMsg, x, y, wKeys, lData);
}
void CSysWinVxGraph::MouseCallback(int iMsg, int x, int y, int wKeys, long int lData) {
UNREFERENCED_PARAMETER(wKeys);
   char szBuf[256];                         // formatted text
   double dX, dY;                           // plane coordinates

   switch(lData) {
   case CGVX_ACT_AXES:                      // status bar info on move
      switch(iMsg) {
      case ACSM_MOVE:
         dX = Axes.Client2PlaneX(x);
         dY = Axes.Client2PlaneY(y);
         App()->SetStatusBarInfo(&dX, &dY);
         break;
      }
      break;

   case CGVX_ACT_XMIN:
   case CGVX_ACT_XMAX:
   case CGVX_ACT_YMIN:
   case CGVX_ACT_YMAX:
      if(iMsg!=ACSM_LEFT) break;            // ignore everything other than left click
      if(pQEdit==NULL) break;               // ignore if no control
      pQEdit->SetUserData(this, lData);     // set identifiers
      pQEdit->SetPositionRect(Mouse.FindActiveByData(this, lData)->ActiveRect());
      pQEdit->SetDouble(
         (lData==CGVX_ACT_XMIN) ? ddProp[CGVXI_PROP_XMIN] :
         (lData==CGVX_ACT_XMAX) ? ddProp[CGVXI_PROP_XMAX] :
         (lData==CGVX_ACT_YMIN) ? ddProp[CGVXI_PROP_YMIN] :
         (lData==CGVX_ACT_YMAX) ? ddProp[CGVXI_PROP_YMAX] : 0.00);
      pQEdit->Show();
      break;

   case CGVX_ACT_RUNVAR:
   case CGVX_ACT_PLTFCN:
      if(iMsg!=ACSM_LEFT) break;            // ignore all but left click
      if(pQMenu==NULL) break;               // ignore if no control
      pQMenu->SetUserData(this, lData);     // set identifiers
      switch(lData) {
      case CGVX_ACT_RUNVAR: pQMenu->SetItems(App()->VarsString(), RunVar()); break;
      case CGVX_ACT_PLTFCN: pQMenu->SetItems(CVertex::CszVxFcnNames, PltFcn()); break;
      }
      pQMenu->SetPositionRectAuto(Mouse.FindActiveByData(this, lData)->ActiveRect());
      pQMenu->Show();
      break;

   case CGVX_ACT_VARVAL:
      switch(iMsg) {
      case ACSM_MOVE:
      case ACSM_DRAG:
      case ACSM_DEND:
         dX = Axes.Client2PlaneX(x);        // map to variable value
         dY = Axes.Client2PlaneY(y);        // map position value
         App()->SetStatusBarInfo(&dX, &dY); // update info
         if(iMsg == ACSM_MOVE) break;       // done if only moving
         if(dX < App()->VarMin(RunVar())) dX = App()->VarMin(RunVar());
         if(dX > App()->VarMax(RunVar())) dX = App()->VarMax(RunVar());
         App()->SetVar(RunVar(), dX);       // set the new value
         App()->UpdateProperties();         // update variable displays
         App()->ScanAll();                  // refresh all scans
         break;
      }
      break;
   }
}


/*********************************************************
* QEditCallback
* The callback function must be declared static.
*********************************************************/
BOOL CSysWinVxGraph::_QEditCallback(void *pVal, int iNext, void *pVoid, long int lLong) {
   return( ((CSysWinVxGraph*)pVoid)->QEditCallback(pVal, iNext, lLong) );
}
BOOL CSysWinVxGraph::QEditCallback(void *pVal, int iNext, long int lLong) {
UNREFERENCED_PARAMETER(iNext);
   switch(lLong) {
   case CGVX_ACT_XMIN: UserSetXLim(*(double*)pVal, XMax()); break;
   case CGVX_ACT_XMAX: UserSetXLim(XMin(), *(double*)pVal); break;
   case CGVX_ACT_YMIN: UserSetYLim(*(double*)pVal, YMax()); break;
   case CGVX_ACT_YMAX: UserSetYLim(YMin(), *(double*)pVal); break;
   }
   App()->PropManager()->OnPaint(TRUE);     // update the values in the property manager
   Refresh();                               // re-paint the renderer
   return(TRUE);
}


/*********************************************************
* QMenuCallback
* The callback function must be declared static.
*********************************************************/
BOOL CSysWinVxGraph::_QMenuCallback(int iSel, int iNext, void *pVoid, long int lLong) {
   return( ((CSysWinVxGraph*)pVoid)->QMenuCallback(iSel, iNext, lLong) );
}
BOOL CSysWinVxGraph::QMenuCallback(int iSel, int iNext, long int lLong) {
UNREFERENCED_PARAMETER(iNext);
   switch(lLong) {
   case CGVX_ACT_RUNVAR: UserSetRunVar(iSel); break;
   case CGVX_ACT_PLTFCN: UserSetPltFcn(iSel); break;
   }
   InvalidateRect(hwRenderer, NULL, TRUE);
   return(TRUE);
}


/*********************************************************
* CopyDataToClipboard
* We will use the standard  clipboard format CF_TEXT, for
* which the system automatically frees the associated me-
* mory using  GlobalFree() once the data is no  longer in
* use. Alternatively, we could handle the WM_DESTROYCLIP-
* BOARD message in the main handler.
* Called from
* <- (Message handler)
*    <- CMouse::TrackPopupMenu
*       <- this mouse handler
*********************************************************/
void CSysWinVxGraph::CopyDataToClipboard(void) {
   char    szBuffer[4096];                  // formatted buffer
   HGLOBAL hglbClip;                        // global memory
   void   *pvClip;                          // cast pointer
   int     k;                               // loop counter

   if((pdXData==NULL) || (pdSData==NULL) || (pdTData==NULL) || (iDataPts==0)) return; // ignore if no data

   //---Open clipboard--------------------------
   if(! OpenClipboard(hwRenderer)) return;  // open clipboard, terminate on fail
   EmptyClipboard();                        // clear current contents

   //---Format data-----------------------------
   sprintf(szBuffer, "");
   for(k=0; k<iDataPts; k++) {
      switch(PltFcn()) {
      case CVXI_PROP_ASTIG:
         sprintf(szBuffer+strlen(szBuffer), "%lg\t%lg\r\n", pdXData[k], pdSData[k]);
         break;
      default:
         sprintf(szBuffer+strlen(szBuffer), "%lg\t%lg\t%lg\r\n", pdXData[k], pdSData[k], pdTData[k]);
         break;
      }
   }

   //---Copy to clipboard-----------------------
   // use GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE) for CF_TEXT
   do {
      hglbClip = GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE | GMEM_ZEROINIT, (strlen(szBuffer)+1) * sizeof(char));
      if(hglbClip==NULL) break;
      pvClip = (void*) GlobalLock(hglbClip);   // lock handle to get pointer
      if(pvClip==NULL) break;
      memcpy(pvClip, szBuffer, strlen(szBuffer)*sizeof(char)); // copy data
   } while(0);                              // do once
   GlobalUnlock(hglbClip);                  // release memory handle
   SetClipboardData(CF_TEXT, hglbClip);     // set clipboard data
   CloseClipboard();                        // release the clipboard
}


/*********************************************************
* MenuCommand
* The application calls MenuCommand() of the current ren-
* derer for ANY renderer-specific command.
*********************************************************/
void CSysWinVxGraph::MenuCommand(int iCmd) {
   switch(iCmd) {
   case CMU_COPYDATA: CopyDataToClipboard(); break;
   }
}


/*########################################################
 ## Window routines
########################################################*/
/*********************************************************
* WndProcSysWinGraph
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
LRESULT CALLBACK CSysWinVxGraph::WndProcSysWinVxGraph(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
   CSysWinVxGraph *pSWin;                        // owning object
   pSWin = (CSysWinVxGraph*) GetWindowLong(hWnd, 0); // not valid on WM_CREATE
   switch(uMsg) {
   case WM_CREATE:
      pSWin = (CSysWinVxGraph*) ((LPMDICREATESTRUCT) (((LPCREATESTRUCT)lParam)->lpCreateParams))->lParam;
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

   case WM_MOUSEMOVE:
   case WM_LBUTTONDOWN:
   case WM_LBUTTONUP:
   case WM_RBUTTONDOWN:
   case WM_RBUTTONUP:
      pSWin->Mouse.MouseProc(hWnd, uMsg, LOWORD(lParam), HIWORD(lParam));
      break;

   case WM_COMMAND:
      switch(LOWORD(wParam)) {
      case CMU_POPUP_COPYDATA: SendMessage(pSWin->App()->GetWindow(), WM_COMMAND, CMU_COPYDATA, 0L); break;
      }
      break;

   default:
      return(DefMDIChildProc(hWnd, uMsg, wParam, lParam));
   }
   return(0L);
}


/*********************************************************
* OnResize
*********************************************************/
void CSysWinVxGraph::OnResize(void) {
   RECT rcClient;                           // window client area rectangle
   RECT rc;                                 // sub-rectangle

   GetClientRect(hwRenderer, &rcClient);    // get the client area
   //---Axes--------------------------
   SetRect(&rc, rcClient.left+48, rcClient.top+32, rcClient.right-32, rcClient.bottom-42);
   Axes.SetPosition(&rc);                   // set the axes' position
   Axes.SetAxMaxVisTick(AX_XAXIS, (rc.right-rc.left)/60);
   Axes.SetAxMaxVisTick(AX_YAXIS, (rc.bottom-rc.top)/30);
   Axes.CalcTicks();                        // re-calculate the ticks

   //---Mouse Actives-----------------
   Mouse.FindActiveByData(this, CGVX_ACT_AXES   )->SetActiveRect(&rc);
   Mouse.FindActiveByData(this, CGVX_ACT_XMIN   )->SetActiveRect(rc.left -24, rc.bottom+ 2, rc.left +24, rc.bottom+18);
   Mouse.FindActiveByData(this, CGVX_ACT_XMAX   )->SetActiveRect(rc.right-24, rc.bottom+ 2, rc.right+24, rc.bottom+18);
   Mouse.FindActiveByData(this, CGVX_ACT_YMIN   )->SetActiveRect(rc.left -52, rc.bottom- 8, rc.left - 4, rc.bottom+ 8);
   Mouse.FindActiveByData(this, CGVX_ACT_YMAX   )->SetActiveRect(rc.left -52, rc.top   - 8, rc.left - 4, rc.top   + 8);
   SetRect(&rcRect[CGVXI_RECT_RUNVAR], rc.left+24, rc.bottom+18, rc.right-24, rc.bottom+32);
   Mouse.FindActiveByData(this, CGVX_ACT_RUNVAR )->SetActiveRect(&rcRect[CGVXI_RECT_RUNVAR]);
   SetRect(&rcRect[CGVXI_RECT_PLTFCN], rc.left, rc.top-24, rc.right, rc.top-8);
   Mouse.FindActiveByData(this, CGVX_ACT_PLTFCN )->SetActiveRect(&rcRect[CGVXI_RECT_PLTFCN]);
   RefreshDraggerActive();                  // call to update the dragger's position
}

/*********************************************************
* RefreshDraggerActive
* Moves the active  for the variable  dragger to the cor-
* rect place.  Since this is called from  OnResize, which
* is called before the Mouse Actives are instantiated, we
* have to check that we have an active to move.
* Called from
* <- OnResize
* <- GraphPoint(iRVar=-1)
*    <- App::ScanAll
*********************************************************/
void CSysWinVxGraph::RefreshDraggerActive(void) {
   CMActive *pAct;
   RECT rc;                                 // axes rectangle
   int ix;                                  // screen co-ordinate

   //---Check active------------------
   pAct = Mouse.FindActiveByData(this, CGVX_ACT_VARVAL);
   if(pAct==NULL) return;                   // ignore if active not available yet

   //---Get position------------------
   ix = Axes.Plane2ClientX(App()->Var(RunVar())); // get current value
   Axes.GetPosition(&rc);                   // read position rectangle
   if(ix < rc.left) ix = rc.left;           // limit to rectangle
   if(ix > rc.right) ix = rc.right;

   //---Place-------------------------
   pAct->SetActiveRect(ix-3, rc.top, ix+4, rc.bottom);
}


/*********************************************************
* Print
* For graph renderers, we could replace the rectangles of
* the axes and labels and call Paint. However, since the
* CAxes class does its own painting very nicely, we will
* instead DUPLICATE the painting code!
*********************************************************/
void CSysWinVxGraph::Print(HDC hdcPrint) {
   char  szBuf[256];                        // formatted text buffer
   HFONT hfOld;                             // restore font
   RECT  rc;                                // axes rectangle
   SIZE  sz;                                // text size
   int   iPrintWid, iPrintHig;              // printer width/height (pixels)
   UINT  uTxtAln;                           // restore text alignment
   int   iMkrSize;                          // printed marker size

   if(hdcPrint==NULL) return;               // ignore bad calls

   //===Prepare===========================================
   //---Keep old values-------------------------
   hfOld = (HFONT) SelectObject(hdcPrint, App()->Font(CAPPI_FONTPRINT));

   //---Scaling---------------------------------
   iPrintWid = GetDeviceCaps(hdcPrint, HORZRES); // print width (pixels)
   iPrintHig = GetDeviceCaps(hdcPrint, VERTRES); // print height (pixels)
   iMkrSize  = 4*GetDeviceCaps(hdcPrint, LOGPIXELSX)/72; // n-pt markers

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
   //TextOut(hdcPrint, 0, 0, " ", 1);         // need this for rest to appear (?)
   //---Data------------------------------------
   switch(PltFcn()) {
   case CVXI_PROP_ASTIG:
      SelectObject(hdcPrint, App()->Pen(CAPPI_PENSAGTAN));
      Axes.PaintDataMarker(pdXData, pdSData, iDataPts, &hdcPrint, 1, AX_MKRSQUARE, iMkrSize);
      Axes.PaintData(pdXData, pdSData, iDataPts, &hdcPrint);
      break;
   default:
      SelectObject(hdcPrint, App()->Pen(CAPPI_PENSAG));
      Axes.PaintDataMarker(pdXData, pdSData, iDataPts, &hdcPrint, 1, AX_MKRPLUS, iMkrSize);
      Axes.PaintData(pdXData, pdSData, iDataPts, &hdcPrint);
      SelectObject(hdcPrint, App()->Pen(CAPPI_PENTAN));
      Axes.PaintDataMarker(pdXData, pdTData, iDataPts, &hdcPrint, 1, AX_MKRCROSS, iMkrSize);
      Axes.PaintData(pdXData, pdTData, iDataPts, &hdcPrint);
      break;
   }

   //---Axes------------------------------------
   SelectObject(hdcPrint, App()->Pen(CAPPI_PENAXES));
   Axes.Paint(&hdcPrint);

   //---Labels----------------------------------
   uTxtAln = SetTextAlign(hdcPrint, TA_CENTER);
   sprintf(szBuf, "%s (%s): %s", pVertex->TypeString(), pVertex->Tag(), CVertex::VxFcnString(PltFcn()));
   TextOut(hdcPrint, (rc.left+rc.right)/2, rc.top, szBuf, strlen(szBuf));
   sprintf(szBuf, "%s", (pSystem==NULL) ? "" : App()->VarString(RunVar()));
   TextOut(hdcPrint, (rc.left+rc.right)/2, rc.bottom, szBuf, strlen(szBuf));
   SetTextAlign(hdcPrint, uTxtAln);

   //===Finalize==========================================
   //---Restore---------------------------------
   SelectObject(hdcPrint, hfOld);           // restore font
   OnResize();                              // reposition to window

}


/*********************************************************
* OnPaint
*********************************************************/
void CSysWinVxGraph::OnPaint(void) {
   char        szBuf[256];                  // formatted string
   RECT        rcClient, rc;                // axes rectangle
   int         ix;                          // current x point
   double      dX, dS, dT;                  // current point
   PAINTSTRUCT ps;                          // painting structure
   HDC         hdc;                         // DC to use
   HFONT       hfOld;                       // restore font
   HPEN        hpOld;                       // restore pen
   UINT        uTxtAln;                     // restore text alignment
   HDC         hdcMem;                      // memory DC
   HBITMAP     hbmOld, hbmMem;              // restore, memory bitmap

   //---Prepare---------------------------------
   BeginPaint(hwRenderer, &ps);
   GetClientRect(hwRenderer, &rcClient);
   hdc = ps.hdc;
   do {
      if((hdcMem=CreateCompatibleDC(ps.hdc))==NULL) break;
      if((hbmMem=CreateCompatibleBitmap(ps.hdc, rcClient.right-rcClient.left, rcClient.bottom-rcClient.top))==NULL) break;
      hdc = hdcMem;
      hbmOld = (HBITMAP) SelectObject(hdcMem, hbmMem);
   } while(0);

   hpOld =  (HPEN) SelectObject(hdc, App()->Pen());
   hfOld = (HFONT) SelectObject(hdc, App()->Font());

   FillRect(hdc, &rcClient, App()->Brush());

   //---Dragger---------------------------------
   SelectObject(hdc, App()->Pen(CAPPI_PEN3DFACE));
   Axes.GetPosition(&rc);                   // axes screen position
   ix = Axes.Plane2ClientX(App()->Var(RunVar())); // current value
   if((ix >= rc.left) && (ix <= rc.right)) {
      MoveToEx(hdc, ix, rc.top, NULL);
      LineTo(hdc, ix, rc.bottom);
   }

   //---Data------------------------------------
   switch(PltFcn()) {
   case CVXI_PROP_ASTIG:
      SelectObject(hdc, App()->Pen(CAPPI_PENSAGTAN));
      Axes.PaintData(pdXData, pdSData, iDataPts, &hdc, AX_MKRSQUARE);
      break;
   default:
      SelectObject(hdc, App()->Pen(CAPPI_PENSAG));
      Axes.PaintData(pdXData, pdSData, iDataPts, &hdc, AX_MKRPLUS);
      SelectObject(hdc, App()->Pen(CAPPI_PENTAN));
      Axes.PaintData(pdXData, pdTData, iDataPts, &hdc, AX_MKRCROSS);
      break;
   }

   //---Current---------------------------------
   SelectObject(hdc, App()->Pen(CAPPI_PENAXES));
   dX = App()->Var(RunVar());
   pVertex->FcnValue(PltFcn(), &dS, &dT);
   switch(PltFcn()) {
   case CVXI_PROP_ASTIG:
      if((dS>ddProp[CGRPI_PROP_YMIN]) && (dS<ddProp[CGRPI_PROP_YMAX])) {
         Axes.PaintDataMarker(&dX, &dS, 1, &hdc, 1, AX_MKRCIRC, 6);
      }
      break;
   default:
      if( ((dS>ddProp[CGRPI_PROP_YMIN]) && (dS<ddProp[CGRPI_PROP_YMAX]))
            || ((dS>ddProp[CGRPI_PROP_YMIN]) && (dS<ddProp[CGRPI_PROP_YMAX])) ) {
         Axes.PaintDataMarker(&dX, &dS, 1, &hdc, 1, AX_MKRSQUARE, 6);
         Axes.PaintDataMarker(&dX, &dT, 1, &hdc, 1, AX_MKRDIAMOND, 6);
      }
      break;
   }

   //---Axes------------------------------------
   SelectObject(hdc, App()->Pen(CAPPI_PENAXES));
   Axes.Paint(&hdc);

   //---Labels----------------------------------
   uTxtAln = SetTextAlign(hdc, TA_CENTER);
   sprintf(szBuf, "%s", (pSystem==NULL) ? "" : App()->VarString(RunVar()));
   ExtTextOut(hdc, (rcRect[CGVXI_RECT_RUNVAR].left+rcRect[CGVXI_RECT_RUNVAR].right)/2, rcRect[CGVXI_RECT_RUNVAR].top+1, ETO_CLIPPED | ETO_OPAQUE, &rcRect[CGVXI_RECT_RUNVAR], szBuf, strlen(szBuf), NULL);

   sprintf(szBuf, "%s (%s): %s", pVertex->TypeString(), pVertex->Tag(), CVertex::VxFcnString(PltFcn()));
   ExtTextOut(hdc, (rcRect[CGVXI_RECT_PLTFCN].left+rcRect[CGVXI_RECT_PLTFCN].right)/2, rcRect[CGVXI_RECT_PLTFCN].top+1, ETO_CLIPPED | ETO_OPAQUE, &rcRect[CGVXI_RECT_PLTFCN], szBuf, strlen(szBuf), NULL);
   SetTextAlign(hdc, uTxtAln);

   //---Finalize--------------------------------
   //Mouse.PaintAllActiveRect(hdc);        //**DEBUG** paint mouse rectangles
   SelectObject(hdc, hfOld);             // restore previous font
   SelectObject(hdc, hpOld);             // restore previous pen

   if((hdcMem) && (hbmMem)) {
      BitBlt(ps.hdc, rcClient.left, rcClient.top,
         rcClient.right-rcClient.left, rcClient.bottom-rcClient.top,
         hdcMem, 0, 0, SRCCOPY);
      SelectObject(hdcMem, hbmOld);
   }
   if(hdcMem) DeleteDC(hdcMem);
   if(hbmMem) DeleteObject(hbmMem);

   EndPaint(hwRenderer, &ps);
}
