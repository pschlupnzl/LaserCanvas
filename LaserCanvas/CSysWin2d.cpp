/*********************************************************
* CSysWin2d
* This is the main interactive  renderer for the CSystem.
* In common with other renderers, it is part of the CSys-
* Win renderers chain
* $PSchlup 2006 $     $Revision 5 $
*********************************************************/
#include "CSysWin2d.h"                      // class header

const char* CSysWin2d::CszSysWin2dProp[C2DI_NUM_PROP] = { // names for stored properties
   "XMiddle"       ,                        // canvas center
   "YMiddle"       ,                        // canvas center
   "Zoom"          ,                        // zoom scale
   "OpticScale"    ,                        // optic scale
   "ModeScale"     ,                        // mode scale
   "GridSize"      ,                        // grid size
   "Flags"                                  // visibility flags
};


/*********************************************************
* Constructor
*********************************************************/
CSysWin2d::CSysWin2d(CSystem *pSys) : CSysWin(pSys) {
printf("+ Create CSysWin-2D- renderer @0x%08lx\n", this);
   CVertex *pVx;                            // vertex loop counter for auto-centering
   double dXMin, dXMax, dYMin, dYMax;       // extents for auto-centering

   //---Members---------------------------------
   iTool           = C2DC_TOOL_ARROW;       // default tool
   drcInvFrame.Left   = 0.00;
   drcInvFrame.Top    = 0.00;
   drcInvFrame.Right  = 0.00;
   drcInvFrame.Bottom = 0.00;
   iFrameTyp       = C2DC_FRAME_NONE;       // no frame

   ddProp[C2DI_PROP_XMIDDLE   ] =  0.00;      // canvas center
   ddProp[C2DI_PROP_YMIDDLE   ] =  0.00;      // canvas center
   ddProp[C2DI_PROP_ZOOM      ] =  1.00;      // zoom level
   ddProp[C2DI_PROP_OPTICSCALE] = 50.00;      // optic size
   ddProp[C2DI_PROP_MODESCALE ] = 20.00;      // mode size
   ddProp[C2DI_PROP_GRIDSIZE  ] = 10.00;      // grid size
   ddProp[C2DI_PROP_FLAGS     ] = (double) (0x0000 // flags
                                    //| C2DF_SNAPGRID
                                    | C2DF_SHOWDIST
                                    //| C2DF_SHOWANNOT
                                    | C2DF_SHOWWAIST
                                    );
   ixCenter = iyCenter = 0;                 // window center pixel
   //---Center on system------------------------
   if((pVx = pSystem->VxTop()) != NULL) {
      dXMin = dXMax = pVx->X();
      dYMin = dYMax = pVx->Y();
      for(; pVx; pVx=(CVertex*)pVx->Next()) {
         if(pVx->X() < dXMin) dXMin = pVx->X();
         if(pVx->X() > dXMax) dXMax = pVx->X();
         if(pVx->Y() < dYMin) dYMin = pVx->Y();
         if(pVx->Y() > dYMax) dYMax = pVx->Y();
      }
      ddProp[C2DI_PROP_XMIDDLE] = (dXMin + dXMax) / 2.00;
      ddProp[C2DI_PROP_YMIDDLE] = (dYMin + dYMax) / 2.00;
   }

   //---Create window---------------------------
   hwRenderer = pSystem->CreateSysWinWindow(CSZ_WNDCLASS_SYSWIN2D, (LPVOID) this);

   //---Finalize--------------------------------
   CreateAllVxActive();                     // create actives for any existing Vxs
   UserSetTool(iTool);                      // set tool, set check marks
   OnResize();

}


/*********************************************************
* Destructor
* If the base class destructor  is declared VIRTUAL, the
* derived class destructor is executed first
*********************************************************/
CSysWin2d::~CSysWin2d() {
   Mouse.DeleteAllActive();                 // delete any and all actives
printf("- Deleted 2D renderer\n");
}


/*########################################################
 ## Data Files                                         ##
########################################################*/
/*********************************************************
* SaveSysWin
* Save the current renderer
*********************************************************/
void CSysWin2d::SaveSysWin(HANDLE hFile) {
   char szBuf[256];                         // string to write to buffer
   DWORD dwBytes;                           // number of bytes written
   int   iPrp;                              // property loop counter

   //---Preliminaries---------------------------
   if(hFile == NULL) printf("? CSysWin2d::SaveSysWin@30 Called with hFile=NULL\n");
   if(hFile == NULL) return;                // ignore if no file

   //---Header----------------------------------
   SaveSysWinHeader(hFile, Type());         // write the header

   //---Properties------------------------------
   for(iPrp=0; iPrp<C2DI_NUM_PROP; iPrp++) {
      sprintf(szBuf, "   %s = %lg\r\n", CszSysWin2dProp[iPrp], ddProp[iPrp]);
      WriteFile(hFile, szBuf, strlen(szBuf), &dwBytes, NULL);
   }

   //---Footer----------------------------------
   SaveSysWinFooter(hFile);                 // write the footer
}


/*********************************************************
* LoadSysWin
* Load renderer properties from data file
*********************************************************/
BOOL CSysWin2d::LoadSysWin(const char *pszDataFile, char *pszMin, char *pszMax) {
UNREFERENCED_PARAMETER(pszDataFile);
   char *psz;                               // pointer in file
   int   iPrp;                              // property loop counter

   if(pszMin==NULL) return(FALSE);          // couldn't load if no pointer
   if(pszMax==NULL) pszMax = pszMin + strlen(pszMin); // to end of file if not given

   //---Header------------------------
   LoadSysWinHeader(pszDataFile, pszMin, pszMax);

   //---Specifics---------------------
   for(iPrp=0; iPrp<C2DI_NUM_PROP; iPrp++) {
      psz = strstr(pszMin, CszSysWin2dProp[iPrp]);
      if((psz==NULL) || (psz>pszMax)) continue;
      psz = strchr(psz, '=');               //..find '=' sign
      if((psz==NULL) || (psz>pszMax)) continue;
      sscanf(psz+1, "%lg", &ddProp[iPrp]);
   }
   OnResize();

   return(TRUE);
}

/*########################################################
 ## Overloaded Base Functions                          ##
########################################################*/
/*********************************************************
* Refresh
*********************************************************/
void CSysWin2d::Refresh(void) {
   if(hwRenderer) InvalidateRect(hwRenderer, NULL, TRUE);
}


/*********************************************************
* UpdateTitle
*********************************************************/
void CSysWin2d::UpdateTitle(int iID) {
   char szRC[128];                          // string from resource file
   char szBuf[256];                         // formatted string
   LoadString(App()->GetInstance(), SZ_TITLE_SYSWIN2D, szRC, sizeof(szRC)/sizeof(char));
   sprintf(szBuf, szRC, (pSystem) ? ((pSystem->FileName()) ? pSystem->FileName() : "") : "");
   if(iID > 0) sprintf(szBuf+strlen(szBuf), " (%d)", iID);
   if(hwRenderer) SetWindowText(hwRenderer, szBuf);
}

/*********************************************************
* GraphPoint
*********************************************************/
void CSysWin2d::GraphPoint(int iRVar, int iPt) {
UNREFERENCED_PARAMETER(iPt);
   if(iRVar < 0) {                          // end of scan, steady condition
      UpdateAllVxActiveRect();
   }
}


/*########################################################
 ## Properties                                         ##
########################################################*/
/*********************************************************
* PrepareProperties
* For renderers, Prepare also calls Update, because their
* properties aren't updated anywhere else
*********************************************************/
const UINT CSysWin2d::CuProperties[] = {
   CPS_CNVHEADER   ,
   CPS_CNVZOOM     ,
   CPS_CNVMODESIZE ,
   CPS_CNVOPTSIZE  ,
   CPS_CNVGRIDSIZE ,
   CPS_CNVSNAPGRID ,
   CPS_CNVSHOWDIST ,
   CPS_CNVSHOWANNOT,
   CPS_CNVSHOWWAIST,
   0};
//=======================================================
void CSysWin2d::PrepareProperties(BOOL tfAct) {
   //---Prepare---------------------------------
   App()->PrepareVariableProperties(tfAct);
   App()->PrepareProperties(CuProperties, tfAct, CSysWin2d::_SysWinPropItemCallback, this);
   App()->EnableMenus(tfAct ? CSYSWINI_TYPE_2D : -1);
   pSystem->PrepareProperties(tfAct);
   pSystem->PrepareVxProperties(tfAct);

   //---Update----------------------------------
   if(tfAct) UpdateProperties();            // set initial values
}
/*********************************************************
* UpdateProperties
* Called from
* <- PrepareProperties(TRUE)
* <- SysWinPropItemCallback
*********************************************************/
void CSysWin2d::UpdateProperties(void) {
   // recall flag CPIF_NAN1, CPIF_NAN2, CPIF_MULTI, CPIF_READONLY
   CPropMgr *pMgr = App()->PropManager();   // assign for below
   pMgr->FindItemByID( CPS_CNVZOOM      )->SetItemValue(ddProp[C2DI_PROP_ZOOM      ]*100.00);
   pMgr->FindItemByID( CPS_CNVMODESIZE  )->SetItemValue(ddProp[C2DI_PROP_MODESCALE ]);
   pMgr->FindItemByID( CPS_CNVOPTSIZE   )->SetItemValue(ddProp[C2DI_PROP_OPTICSCALE]);
   pMgr->FindItemByID( CPS_CNVGRIDSIZE  )->SetItemValue(ddProp[C2DI_PROP_GRIDSIZE  ]);
   pMgr->FindItemByID( CPS_CNVSNAPGRID  )->SetItemCheckBox(CheckBit(C2DF_SNAPGRID ) ? TRUE : FALSE);
   pMgr->FindItemByID( CPS_CNVSHOWDIST  )->SetItemCheckBox(CheckBit(C2DF_SHOWDIST ) ? TRUE : FALSE);
   pMgr->FindItemByID( CPS_CNVSHOWANNOT )->SetItemCheckBox(CheckBit(C2DF_SHOWANNOT) ? TRUE : FALSE);
   pMgr->FindItemByID( CPS_CNVSHOWWAIST )->SetItemCheckBox(CheckBit(C2DF_SHOWWAIST) ? TRUE : FALSE);
}


/*********************************************************
* SysWinPropItemCallback
* Callback from property manager when an item is modified
* The callback function must be declared static.
*********************************************************/
BOOL CSysWin2d::_SysWinPropItemCallback(void *vData, UINT uID, void *pVoid) {
   return((pVoid) ? ((CSysWin2d*)pVoid)->SysWinPropItemCallback(vData, uID) : TRUE);
}
//******************************************************//
BOOL CSysWin2d::SysWinPropItemCallback(void *vData, UINT uID) {
   //---Process value---------------------------
   switch(uID) {
   case CPS_CNVZOOM:      ddProp[C2DI_PROP_ZOOM      ] = (*(double*) vData)/100.00; break;
   case CPS_CNVMODESIZE:  ddProp[C2DI_PROP_MODESCALE ] = *(double*) vData; break;
   case CPS_CNVOPTSIZE:   ddProp[C2DI_PROP_OPTICSCALE] = *(double*) vData; break;
   case CPS_CNVGRIDSIZE:  ddProp[C2DI_PROP_GRIDSIZE  ] = *(double*) vData; break;
   case CPS_CNVSNAPGRID:  ToggleBit( C2DF_SNAPGRID  ); break;
   case CPS_CNVSHOWDIST:  ToggleBit( C2DF_SHOWDIST  ); break;
   case CPS_CNVSHOWANNOT: ToggleBit( C2DF_SHOWANNOT ); break;
   case CPS_CNVSHOWWAIST: ToggleBit( C2DF_SHOWWAIST ); break;
   default:
      return(TRUE);
   }

   //---Apply-----------------------------------
   UpdateProperties();                      // set the new values
   App()->PropManager()->OnPaint(TRUE);     // update values only
   UpdateAllVxActiveRect();                 // update positions of actives
   Refresh();                               // refresh renderer window
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
void CSysWin2d::MenuCommand(int iCmd) {
   switch(iCmd) {
   //case CMU_COPYDATA: CopyDataToClipboard(); break;
   case CMU_TOOL_ARROW  : UserSetTool(C2DC_TOOL_ARROW  ); break;
   case CMU_TOOL_MEASURE: UserSetTool(C2DC_TOOL_MEASURE); break;
   case CMU_TOOL_ZOOM   : UserSetTool(C2DC_TOOL_ZOOM   ); break;
   case CMU_TOOL_PAN    : UserSetTool(C2DC_TOOL_PAN    ); break;
   case CMU_TOOL_ROTATE : UserSetTool(C2DC_TOOL_ROTATE ); break;
   case CMU_CNV_ZOOMIN  : UserZoom(-1.50); break;
   case CMU_CNV_ZOOMOUT : UserZoom(-0.75); break;
   case CMU_TOOL_DRAFT  :
      if(pSystem==NULL) break;              // ignore if no system
      if(!pSystem->DraftMode()) {           // if not already there..
         switch(Tool()) {
         case C2DC_TOOL_MEASURE:
         case C2DC_TOOL_ROTATE:
            UserSetTool(C2DC_TOOL_ARROW);   // some tools not allowed in draft mode
            break;
         }
         pSystem->UserSetDraftMode(TRUE);   //..toggle to draft mode
      } else {
         pSystem->UserSetDraftMode(FALSE);  // try to bounce out of it
      }
      Refresh();                            // refresh the window
      break;
   }
}


/*********************************************************
* DebugPrint
*********************************************************/
void CSysWin2d::DebugPrint(char *psz, int *pInt) {
   for(int k=0; k<*pInt; k++) sprintf(psz+strlen(psz), "   ");
   sprintf(psz+strlen(psz), "CSysWin-2D Renderer 0x%08lx\n", (long) this);
}


/*########################################################
 ## Mouse and Actives                                  ##
########################################################*/
/*********************************************************
* UserSetTool
* Set the tool; also calls UpdateAllVxActiveRect, because
* the vx actives are only placed for the edit tool.
* Called from
* <- App::Menu callback
*********************************************************/
void CSysWin2d::UserSetTool(int k) {
   CMActive *pAct;                          // Canvas active

   //---Tool guides-----------------------------
   InvertFrame();

   //---New tool--------------------------------
   iTool = k;                               // set the tool
   if(iTool<0) iTool = C2DC_TOOL_MAX;       // wrap backwards
   if(iTool>C2DC_TOOL_MAX) iTool = 0;       // wrap forwards

   //---Actives---------------------------------
   pAct = Mouse.FindActiveByData(this, 0L); // find the active
   if(pAct) {
      switch(iTool) {
      case C2DC_TOOL_ARROW  :
         pAct->SetCursors(App()->Cursor(CAPPI_CUR_ARROW), NULL, NULL, NULL);
         break;
      case C2DC_TOOL_MEASURE:
         pAct->SetCursors(App()->Cursor(CAPPI_CUR_MEAS), NULL, NULL, NULL);
         break;
      case C2DC_TOOL_ZOOM   :
         pAct->SetCursors(App()->Cursor(CAPPI_CUR_ZOOMIN), NULL, NULL, NULL);
         break;
      case C2DC_TOOL_PAN    :
         pAct->SetCursors(App()->Cursor(CAPPI_CUR_PAN_ARROW), NULL, App()->Cursor(CAPPI_CUR_PAN), NULL);
         break;
      case C2DC_TOOL_ROTATE :
         pAct->SetCursors(App()->Cursor(CAPPI_CUR_ROTATE_ARROW), NULL, App()->Cursor(CAPPI_CUR_ROTATE), NULL);
         break;
      }
   }
   UpdateAllVxActiveRect();                 // place vx actives
   Mouse.MouseProc(0, 0, 0, 0);             // update cursors (this works!)

   //---Tool guides-----------------------------
   switch(iTool) {
   case C2DC_TOOL_ROTATE: iFrameTyp = C2DC_FRAME_ROTATE; break;
   case C2DC_TOOL_PAN:    iFrameTyp = C2DC_FRAME_PAN;    break;
   default:               iFrameTyp = C2DC_FRAME_NONE;   break; // no tool frames
   }
   InvertFrame();                           // invert new guide

   //---Menu check marks------------------------
   App()->CheckMenu( CMU_TOOL_ARROW  , Tool()==C2DC_TOOL_ARROW   ? TRUE : FALSE);
   App()->CheckMenu( CMU_TOOL_MEASURE, Tool()==C2DC_TOOL_MEASURE ? TRUE : FALSE);
   App()->CheckMenu( CMU_TOOL_ZOOM   , Tool()==C2DC_TOOL_ZOOM    ? TRUE : FALSE);
   App()->CheckMenu( CMU_TOOL_PAN    , Tool()==C2DC_TOOL_PAN     ? TRUE : FALSE);
   App()->CheckMenu( CMU_TOOL_ROTATE , Tool()==C2DC_TOOL_ROTATE  ? TRUE : FALSE);
}


/*********************************************************
* CreateAllVxActive
* The strategy: To prevent dangling pointers we call this
* function  every time something  changes. It  clears all
* actives and then re-creates them from scratch.
* Advantages:
*  - The Actives are in the same order as the Vx
* Disadvantages:
*  - Time, especially, e.g., when loading files
* Called from:
* <- CSystem::UserInsertVx
*********************************************************/
void CSysWin2d::CreateAllVxActive(void) {
   CVertex *pVx;                            // vertex loop counter
   RECT     rc;                             // window rectangle

   Mouse.DeleteAllActive();                 // clear any previous actives
   //---Canvas------------------------
   GetClientRect(hwRenderer, &rc);
   Mouse.CreateActive(rc.left, rc.top, rc.right, rc.bottom,
      NULL, NULL, NULL, NULL,
      CSysWin2d::_MouseCallbackCanvas, (void*) this, 0L,
      (HMENU) NULL);

   //---Vertices----------------------
   // sort by free-moving (mirrors, sources) and constrained (others):
   // HCURSOR hcMove, HCURSOR hcMoveCtrl, HCURSOR hcDrag, HCURSOR hcDragCtrl,
   for(pVx=pSystem->VxTop(); pVx; pVx=(CVertex*)pVx->Next()) {
      switch(pVx->Type()) {
      case CVX_TYPE_MIRROR:
      case CVX_TYPE_FLATMIRROR:
      case CVX_TYPE_SOURCE:
      case CVX_TYPE_OUTCOUPLER:
         Mouse.CreateActive(0, 0, 0, 0,
            App()->Cursor(CAPPI_CUR_MOVE_ARROW),
            App()->Cursor(CAPPI_CUR_DELETE),
            App()->Cursor(CAPPI_CUR_MOVE),
            App()->Cursor(CAPPI_CUR_NODRAG),
            CSysWin2d::_MouseCallbackVx,
            (void*) this, (long int)(void*) pVx,
            (HMENU) NULL);
         break;
      case CVX_TYPE_LENS:
      case CVX_TYPE_THERMALLENS:
      case CVX_TYPE_INCRYSTAL:
      case CVX_TYPE_OUTCRYSTAL:
      case CVX_TYPE_INBREWSTER:
      case CVX_TYPE_OUTBREWSTER:
      case CVX_TYPE_PRISM1:
      case CVX_TYPE_PRISM2:
      case CVX_TYPE_INPLATE:
      case CVX_TYPE_OUTPLATE:
      case CVX_TYPE_SCREEN:
         Mouse.CreateActive(0, 0, 0, 0,
            App()->Cursor(CAPPI_CUR_STRETCH_ARROW),
            App()->Cursor(CAPPI_CUR_DELETE),
            App()->Cursor(CAPPI_CUR_STRETCH),
            App()->Cursor(CAPPI_CUR_NODRAG),
            CSysWin2d::_MouseCallbackVx,
            (void*) this, (long int)(void*) pVx,
            (HMENU) NULL);
         break;
      }
   }

   UpdateAllVxActiveRect();                 // now set the positions
}


/*********************************************************
* UpdateAllVxActiveRect
* Updates the positions of all the actives.
* Rather than cycling  through vertices and using FindAc-
* tiveByData, we  cycle through the actives and  find the
* vertices from their lData member.
* Called from
* <- OnResize
* <- CreateAllVxActive
* <- GraphPoint (static config)
*********************************************************/
#define C2DC_ACTSIZE             6          // half-size of active
void CSysWin2d::UpdateAllVxActiveRect(void) {
   CMActive *pAct;                          // actives loop counter
   CVertex  *pVx;                           // referenced vertex
   int       ix, iy;                        // canvas positions

   //---Vertices--------------------------------
   for(pAct=Mouse.ActiveTop(); pAct; pAct=(CMActive*) pAct->Next()) {
      pVx = (CVertex*)(void*) pAct->Data(); // pointer stored as data
      if(pVx==NULL) continue;               // ignore canvas
      switch(Tool()) {
      case C2DC_TOOL_ARROW:
      case C2DC_TOOL_MEASURE:
         ix = Cnv2WndX(pVx->X());
         iy = Cnv2WndY(pVx->Y());
         pAct->SetActiveRect(ix-C2DC_ACTSIZE, iy-C2DC_ACTSIZE, ix+C2DC_ACTSIZE, iy+C2DC_ACTSIZE);
         break;
      default:
         pAct->SetActiveRect(0, 0, 0, 0);
      }
   }
}


/*********************************************************
* MouseCallbackCanvas
* Callback procedure for mouse messages in the Canvas ac-
* tive. The behaviour depends on  the current tool. lData
* is always 0L for the Canvas active.
* Called from
* <- Mouse.Active[canvas]
*    <- Mouse
*       <- WM_Mouse messages
* The referenced version must be declared static.
*********************************************************/
void CSysWin2d::_MouseCallbackCanvas(int iMsg, int x, int y, int wKeys, void *pData, long int lData) {
UNREFERENCED_PARAMETER(lData);
   if(pData) ((CSysWin2d*)pData)->MouseCallbackCanvas(iMsg, x, y, wKeys, lData);
}
//******************************************************//
void CSysWin2d::MouseCallbackCanvas(int iMsg, int x, int y, int wKeys, long int lData) {
   char     szBuf[256];                     // formatted text for measure tool
   double   dX, dY, dR;                     // mouse coordinates for info bar
   static double dXDown, dYDown, dRDown, dSDown; // canvas position of mouse down
   int      iTmp;                           // temporary tool
   CVertex *pVx, *pVxMin;                   // vertex loop counter; closest
   double   dSL, dXL, dYL, dSMin, dXMin, dYMin; // distance to line, coordinate of closest

   //===Universal=========================================
   App()->GetStatusBar()->SetStandardText("");

   //---Coordinates-----------------------------
   if(iMsg==ACSM_MOVE) {
      dX = Wnd2CnvX(x); if(fabs(dX) < 1.0e-12) dX = 0.00;
      dY = Wnd2CnvY(y); if(fabs(dY) < 1.0e-12) dY = 0.00;
      App()->SetStatusBarInfo(&dX, &dY);
   }

   //---Common Middle Drag----------------------
   if(iMsg==ACSM_MDDN) {
      iTmp = iTool; iTool = (Mouse.Keys() & MK_CONTROL) ? C2DC_TOOL_ROTATE : C2DC_TOOL_PAN;
      InvertFrame();
      iFrameTyp = (Mouse.Keys() & MK_CONTROL) ? C2DC_FRAME_ROTATE : C2DC_FRAME_ROTATE;
      InvertFrame();
      MouseCallbackCanvas(ACSM_DOWN, x, y, wKeys, lData);
      iTool = iTmp;
      return;
   } else if(iMsg==ACSM_MDRG) {
      iTmp = iTool; iTool = (Mouse.Keys() & MK_CONTROL) ? C2DC_TOOL_ROTATE : C2DC_TOOL_PAN;
      MouseCallbackCanvas(ACSM_DRAG, x, y, wKeys, lData);
      iTool = iTmp;
      return;
   } else if((iMsg==ACSM_MEND) || (iMsg==ACSM_MDCK)) {
      iFrameTyp = (Tool()==C2DC_TOOL_MEASURE) ? C2DC_FRAME_MEASURE : C2DC_FRAME_NONE;
      MouseCallbackCanvas(ACSM_MOVE, x, y, NULL, lData); // move mouse again
      UpdateAllVxActiveRect();
      Refresh();
      return;
   }

   //---Common Compound Tool--------------------
   // Recall: wKeys is state when button goes down
   // Don't pass wKeys on to repeated callback, otherwise
   // we get lost in infinite recursion!
   if(wKeys & MK_CONTROL) {
      switch(iMsg) {
      case ACSM_DEND:
         iFrameTyp =
            (Tool()==C2DC_TOOL_MEASURE) ? C2DC_FRAME_MEASURE :
            (Tool()==C2DC_TOOL_PAN    ) ? C2DC_FRAME_PAN :
            (Tool()==C2DC_TOOL_ROTATE ) ? C2DC_FRAME_ROTATE :
            C2DC_FRAME_NONE;
         MouseCallbackCanvas(ACSM_MOVE, x, y, NULL, lData); // move mouse again
         UpdateAllVxActiveRect();           // update to new positions
         Refresh();
         break;
      default:
         if(iMsg==ACSM_DOWN) { InvertFrame(); iFrameTyp = (wKeys & MK_SHIFT) ? C2DC_FRAME_NONE : C2DC_FRAME_PAN; };
         iTmp = iTool; iTool = (wKeys & MK_SHIFT) ? C2DC_TOOL_ZOOM : C2DC_TOOL_PAN;
         MouseCallbackCanvas(iMsg, x, y, (wKeys & MK_SHIFT), lData);
         iTool = iTmp;
         if(iMsg==ACSM_DOWN) Refresh();     // update new frame
         break;
      }
      return;
   }

   switch(Tool()) {
   //===Arrow=============================================
   case C2DC_TOOL_ARROW:
   case C2DC_TOOL_MEASURE:
      //---Selection----------------------------
      switch(iMsg) {
      case ACSM_DOWN:
         if(!(wKeys & MK_SHIFT)) {
            pSystem->SelectAllVx(FALSE);
            pSystem->PrepareVxProperties(TRUE); // new selection in manager, update, paint
            App()->EnableMenus(CSYSWINI_TYPE_2D); // update menus and buttons
            Refresh();
         }
         InvertFrame();                     // remove the frame
         drcInvFrame.Left = drcInvFrame.Right = Wnd2CnvX(x);
         drcInvFrame.Top  = drcInvFrame.Bottom = Wnd2CnvY(y);
         iFrameTyp = C2DC_FRAME_NONE;       // no frame right now
         break;
      case ACSM_DRAG:
         InvertFrame();                     // clear previous (if set)
         iFrameTyp = C2DC_FRAME_SELECT;     // now we definitely have a frame
         drcInvFrame.Right  = Wnd2CnvX(x);  // update coordinates
         drcInvFrame.Bottom = Wnd2CnvY(y);
         InvertFrame();                     // invert it
         break;
      case ACSM_DEND:
         iFrameTyp = (Tool()==C2DC_TOOL_MEASURE) ? C2DC_FRAME_MEASURE : C2DC_FRAME_NONE;
         pSystem->SelectVxByRect(
            MIN(drcInvFrame.Left, drcInvFrame.Right),
            MIN(drcInvFrame.Top, drcInvFrame.Bottom),
            MAX(drcInvFrame.Left, drcInvFrame.Right),
            MAX(drcInvFrame.Top, drcInvFrame.Bottom),
            (wKeys & MK_SHIFT) ? TRUE : FALSE);
         pSystem->PrepareVxProperties(TRUE); // new selection in manager, update, paint
         App()->EnableMenus(CSYSWINI_TYPE_2D); // update menus and buttons
         Refresh();
         break;

      //===Measure Move================================
      case ACSM_MOVE:
         if(Tool()!=C2DC_TOOL_MEASURE) break;
         //---Find nearest-------------------
         for(pVx=pSystem->VxTop(); pVx->Next(); pVx=(CVertex*) pVx->Next()) {
            dSL = PointNearestLine(
               pVx->X(),         pVx->Y(),
               pVx->Next()->X(), pVx->Next()->Y(),
               dX, dY, &dXL, &dYL);
            if((pVx==pSystem->VxTop()) || (dSL < dSMin)) { // keep closest
               dSMin = dSL; dXMin = dXL; dYMin = dYL; pVxMin = pVx;
            }
         }
         //---Set size-----------------------
         // most of the L variables are misused here
         InvertFrame();                     // clear previous (if set)
         iFrameTyp = C2DC_FRAME_MEASURE;    // now we definitely have a frame
         dSL = SQRT(SQR(dXMin - pVxMin->X()) + SQR(dYMin - pVxMin->Y())); // distance to vx
         dXL = pVxMin->Q(SAG)->W(dSL - pVxMin->Q(SAG)->z0());
         dYL = pVxMin->Q(TAN)->W(dSL - pVxMin->Q(TAN)->z0());
         if(!pSystem->StableABCD(SAG)) dXL = 0.000;
         if(!pSystem->StableABCD(TAN)) dYL = 0.000;
         sprintf(szBuf, "%lg, %lg um", dXL, dYL);
         if(App()->GetStatusBar()) App()->GetStatusBar()->SetStandardText(szBuf);
         dSL = MAX(dXL, dYL) * ModeScale()/1000.00; // get larger of the spots
         dXL = dSL * SIN(pVxMin->SegmentCanvasAngle()); // extrema of mode here
         dYL =-dSL * COS(pVxMin->SegmentCanvasAngle());
         drcInvFrame.Left   = dXMin - dXL;
         drcInvFrame.Top    = dYMin - dYL;
         drcInvFrame.Right  = dXMin + dXL;
         drcInvFrame.Bottom = dYMin + dYL;
         InvertFrame();                     // invert it
         break;
      }
      break;

   //===Zoom Tool=========================================
   // Temporary storage
   //  - dYDown: Window position of mouse down (not canvas!)
   //  - dSDown: Zoom scale at mouse down
   //  - dRDown: Temporary variable to prevent overflow on exp
   case C2DC_TOOL_ZOOM:
      switch(iMsg) {
      case ACSM_LEFT:
         ddProp[C2DI_PROP_ZOOM] *= (wKeys & MK_SHIFT) ? 2.00/3.00 : 1.25;
         ddProp[C2DI_PROP_ZOOM] = floor(ddProp[C2DI_PROP_ZOOM]*100.00 + 0.50)/100.00; // round to 2dp
         if(ddProp[C2DI_PROP_ZOOM] <= 0.00) ddProp[C2DI_PROP_ZOOM] = 1.00;
         UpdateProperties();
         App()->PropManager()->OnPaint(TRUE);     // update values only
         UpdateAllVxActiveRect();
         Refresh();
         break;
      case ACSM_DOWN:
         drcInvFrame.Left = drcInvFrame.Right = Wnd2CnvX(x);
         drcInvFrame.Top  = drcInvFrame.Bottom = Wnd2CnvY(y);
         dYDown = (double) y; dSDown = ddProp[C2DI_PROP_ZOOM];
         break;
      case ACSM_DRAG:
         if(wKeys & MK_SHIFT) {
            dRDown = (dYDown - (double) y) / 100.00;
            if(dRDown < -8.00) dRDown = -8.00;
            if(dRDown >  8.00) dRDown =  8.00;
            ddProp[C2DI_PROP_ZOOM] = dSDown * exp(dRDown);
            UpdateProperties();
            App()->PropManager()->OnPaint(TRUE); // update values only
            UpdateAllVxActiveRect();
            Refresh();
         } else {
            InvertFrame();                  // clear previous (if set)
            iFrameTyp = C2DC_FRAME_ZOOM;    // now we definitely have a frame
            drcInvFrame.Right  = Wnd2CnvX(x);  // update coordinates
            drcInvFrame.Bottom = Wnd2CnvY(y);
            InvertFrame();                  // invert it
         }
         break;
      case ACSM_DEND:
         iFrameTyp = C2DC_FRAME_NONE;
         if(!(wKeys & MK_SHIFT)) ZoomToRect(drcInvFrame.Left, drcInvFrame.Top, drcInvFrame.Right, drcInvFrame.Bottom);
         break;
      }
      break;

   //===Pan Tool==========================================
   case C2DC_TOOL_PAN:
      switch(iMsg) {
      case ACSM_DOWN:
         dXDown = Wnd2CnvX(x); dYDown = Wnd2CnvY(y);  //..coordinates
         break;
      case ACSM_DRAG:
         ddProp[C2DI_PROP_XMIDDLE] -= Wnd2CnvX(x) - dXDown;
         ddProp[C2DI_PROP_YMIDDLE] -= Wnd2CnvY(y) - dYDown;
         dXDown = Wnd2CnvX(x); dYDown = Wnd2CnvY(y);  //..coordinates
         Refresh();
         break;
      case ACSM_DEND:
         UpdateAllVxActiveRect();           // (...although they may not be visible during pan)
         break;
      }
      break;

   //===Rotation Tool=====================================
   // The rotation tool rotates and moves the system ori-
   // gin, but the origin cannot otherwise be accessed by
   // the mouse
   // dXDown: Distance to start of system
   // dSDown: Initial angle to start of system
   // dYDown: Initial angle to mouse
   // dRDown: Initial angle of system
   // We need to call ScanAll() because we modify the ro-
   // tation property of the System.
   case C2DC_TOOL_ROTATE:
      switch(iMsg) {
      case ACSM_DOWN:
         dXDown = SQRT(SQR(pSystem->StartX()-XMiddle()) + SQR(pSystem->StartY()-YMiddle()));
         dYDown = ATAN2((double)(y - iyCenter), (double)(x - ixCenter)); // starting system rotation
         dRDown = pSystem->Rotation() * M_PI/180.00;
         dSDown = ATAN2(pSystem->StartY()-YMiddle(), pSystem->StartX()-XMiddle());
         break;
      case ACSM_DRAG:
         dY = (double) (y - iyCenter);
         dX = (double) (x - ixCenter) + 1e-12*(x==ixCenter);
         dR = dRDown + ATAN2(dY, dX) - dYDown; // system rotation
         while(dR < 0.00     ) dR += 2.00*M_PI; // wrap to 0-2pi
         while(dR > 2.00*M_PI) dR -= 2.00*M_PI;
         if(GetAsyncKeyState(VK_SHIFT) & 0x8000) dR = C2DC_ROTATE_MITRE * floor(dR / C2DC_ROTATE_MITRE + 0.50);
         pSystem->SetRotation(180.00/M_PI * dR);
         pSystem->SetStartPos(
            XMiddle() + dXDown*COS(dSDown - dRDown + dR),
            YMiddle() + dXDown*SIN(dSDown - dRDown + dR));
         App()->ScanAll();
         break;
      case ACSM_DEND:
         break;
      }
      break;
   }

}

/*********************************************************
* MouseCallbackVx
* This is the callback  for the main Vx actives, used for
* selection and dragging.
* The referenced function must be declared static.
* Called from
* <- Mouse/Active
*    <- WM_Mouse Messages
*********************************************************/
void CSysWin2d::_MouseCallbackVx(int iMsg, int x, int y, int wKeys, void *pData, long int lData) {
   if(pData) ((CSysWin2d*)pData)->MouseCallbackVx(iMsg, x, y, wKeys, lData);
}
//******************************************************//
void CSysWin2d::MouseCallbackVx(int iMsg, int x, int y, int wKeys, long int lData) {
   static BOOL tfSelOld;                    // previous selection
   char     szBuf[256];                     // formatted info text
   CVertex *pVx = (CVertex*)(void*) lData;  // value is pointer to Vx object
   double   dX, dY;                         // positions / distance
   switch(iMsg) {
   //---Move------------------------------------
   // We have a status bar that isn't doing much so let's
   // put something down there...
   case ACSM_MOVE:
      sprintf(szBuf, "%s (%s)", pVx->TypeString(), pVx->Tag());
      App()->GetStatusBar()->SetStandardText(szBuf);
      dX = Wnd2CnvX(x); if(fabs(dX) < 1.0e-12) dX = 0.00;
      dY = Wnd2CnvY(y); if(fabs(dY) < 1.0e-12) dY = 0.00;
      App()->SetStatusBarInfo(&dX, &dY);
      break;

   //---Left Down-------------------------------
   case ACSM_DOWN:
      tfSelOld = pVx->Selected();           // different behaviour for click and drag
      if(!(wKeys & MK_SHIFT)) {
         if(!pVx->Selected()) pSystem->SelectAllVx(FALSE);
      }
      pVx->Select(TRUE);
      //---Draft---
      if(pSystem->DraftMode()) {
      //---Normal---
      } else {
         pVx->InitMoveVxTo();                  // prepare for move
         iFrameTyp = C2DC_FRAME_NONE;          // no frame while dragging
         App()->EnableMenus(CSYSWINI_TYPE_2D); // update menus and buttons
      }
      pSystem->PrepareVxProperties(TRUE);   // new selection in manager, update, paint
      Refresh();                            // update the window
      break;

   //---Click-----------------------------------
   case ACSM_LEFT:
      if(wKeys & MK_CONTROL) {
         pSystem->SelectAllVx(FALSE);
         pVx->Select(TRUE);
         pSystem->MenuDeleteVx();
         return;
      } else if(wKeys & MK_SHIFT) {
         pVx->Select(tfSelOld ? FALSE : TRUE); // add or remove from selection
      } else {
         pVx->Select(TRUE);
      }
      //---Draft---
      if(pSystem->DraftMode()) {
         if(pVx->CheckBit(CVXF_DRAFTLINK)) {   // de-select all after this one
            pSystem->UnlinkVx(pVx);         // unlink the vertices
         } else {
            if( pSystem->LinkNextVx(pVx) == TRUE ) { // returns TRUE on last one
               pSystem->UserSetDraftMode(FALSE);
            }
         }
      //---Normal---
      } else {
         pVx->EndMoveVxTo();                   // no move, so finish
      }
      pSystem->PrepareVxProperties(TRUE);   // new selection in manager, update, paint
      App()->EnableMenus(CSYSWINI_TYPE_2D); // update menus and buttons
      Refresh();
      break;


   //---Drag------------------------------------
   case ACSM_DRAG:
      dX = Wnd2CnvX(x);                     // mouse coordinates
      dY = Wnd2CnvY(y);
      if(CheckBit(C2DF_SNAPGRID)) {         // lock to grid
         dX = GridSize() * floor(dX / GridSize() + 0.50);
         dY = GridSize() * floor(dY / GridSize() + 0.50);
      }
      if(fabs(dX) < 1.0e-12) dX = 0.00;
      if(fabs(dY) < 1.0e-12) dY = 0.00;
      App()->SetStatusBarInfo(&dX, &dY);    // update readout
      //---Draft---
      if(pSystem->DraftMode()) {
         pVx->MoveVxToDraft(dX, dY);
      //---Normal---
      } else {
         pVx->MoveVxTo(dX, dY);                // complicated drag function
      }
      App()->ScanAll();                     // now, of course, we have to update the whole thing
      break;

   //---Drag ends-------------------------------
   case ACSM_DEND:
      //---Draft---
      if(pSystem->DraftMode()) {
         UpdateAllVxActiveRect();
      //---Normal---
      } else {
         pVx->EndMoveVxTo();
      }
      break;

   }
}


/*########################################################
 ## Windowing                                          ##
########################################################*/

/*********************************************************
* WndProcSysWin2d
* Since this is  an MDI child window, the  object pointer
* is passed through an  MDICREATESTRUCT on WM_CREATE, and
* we use DefMDIChildProc instead of DefWindowProc. In ad-
* dition, we must pass these  messages, even if we handle
* them:
*  - WM_CHILDACTIVATE
*  - WM_GETMINMAXINFO
*  - WM_MENUCHAR
*  - WM_MOVE
*  - WM_SETFOCUS
*  o WM_SIZE
*  - WM_SYSCOMMAND
* The referenced function must be declared static
*********************************************************/
LRESULT CALLBACK CSysWin2d::_WndProcSysWin2d(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
   switch(uMsg) {
   case WM_CREATE:
      SetWindowLong(hWnd, 0, (LONG) (CSysWin2d*)
         ((LPMDICREATESTRUCT) (((LPCREATESTRUCT)lParam)->lpCreateParams))->lParam);
      return(0L);
   default:
      return( ((CSysWin2d*) GetWindowLong(hWnd, 0))->WndProcSysWin2d(hWnd, uMsg, wParam, lParam) );
   }
}
//******************************************************//
LRESULT CALLBACK CSysWin2d::WndProcSysWin2d(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
   double dVal;                             // temp. value for zooming

   switch(uMsg) {
   case WM_CLOSE: // If we're getting this message, this must be active
      // Other cases: pSWin->pSystem->DeleteSysWin(pSWin);
      App()->UserCloseSystem(System());
      break;

   case WM_MDIACTIVATE:
      PrepareProperties((hWnd==(HWND)lParam) ? TRUE : FALSE);
      if(hWnd==(HWND)lParam) {
         UserSetTool(Tool());
         pSystem->SetDraftMode(pSystem->DraftMode()); // check draft mode menu
         App()->PropManager()->OnPaint(TRUE);
      }
      break;

   case WM_CHAR:
      switch(wParam) {
      case 0x09:                            // tab
         UserSetTool(Tool() + ((GetAsyncKeyState(VK_SHIFT)&0x8000) ? -1 : 1)); // set next tool
         break;
      default:
         return(DefMDIChildProc(hWnd, uMsg, wParam, lParam));
      }
      break;

   case WM_SIZE:  OnResize(); return(DefMDIChildProc(hWnd, uMsg, wParam, lParam));
   case WM_PAINT: OnPaint(); break;

   case WM_LBUTTONDOWN:
   case WM_LBUTTONUP:
   case WM_RBUTTONDOWN:
   case WM_RBUTTONUP:
   case WM_MBUTTONDOWN:
   case WM_MBUTTONUP:
   case WM_MOUSEMOVE:
      Mouse.MouseProc(hWnd, uMsg, LOWORD(lParam), HIWORD(lParam));
      break;

   case WM_MOUSEWHEEL:
      dVal = -(double) (HIWORD(wParam) - ((HIWORD(wParam)&0x8000) ? 65536 : 0)) / WHEEL_DELTA / 8.00;
      if(dVal > 10.00) dVal = 10.00;        // prevent #overflow errors
      UserZoom( -exp(dVal) );
      break;


   default:
      return(DefMDIChildProc(hWnd, uMsg, wParam, lParam));
   }
   return(0L);
}


/*********************************************************
* OnResize
* Called when the  window is resized or  whenever display
* elements need to be repositioned.
*********************************************************/
void CSysWin2d::OnResize(void) {
   RECT rcClient;                           // window rectangle

   if(hwRenderer == NULL) return;           // ignore if no window

   //---Window center---------------------------
   GetClientRect(hwRenderer, &rcClient);
   ixCenter = (rcClient.left + rcClient.right) / 2;
   iyCenter = (rcClient.top + rcClient.bottom) / 2;

   //---Actives---------------------------------
   if(Mouse.FindActiveByData(this, 0L)) {   // canvas active
      Mouse.FindActiveByData(this, 0L)->SetActiveRect(
         rcClient.left, rcClient.top, rcClient.right, rcClient.bottom);
   }
   UpdateAllVxActiveRect();                 // position vx actives
}


/*********************************************************
* ZoomToRect
* The limits do not need to be ascending.
* Called from
* <- MouseCallbackCanvas
*********************************************************/
void CSysWin2d::ZoomToRect(double dXMin, double dYMin, double dXMax, double dYMax) {
   double dSclX, dSclY;                     // scales in both directions
   RECT   rcClient;                         // client rectangle
   if(hwRenderer==NULL) return;             // ignore if no window (unlikely)
   GetClientRect(hwRenderer, &rcClient);    // read client rectangle
   if(dXMax == dXMin) dXMax = dXMin + 1.00; // prevent #DIV/0! errors
   if(dYMax == dYMin) dYMax = dYMin + 1.00; // prevent #DIV/0! errors
   dSclX = fabs((rcClient.right - rcClient.left) / (dXMax - dXMin));
   dSclY = fabs((rcClient.bottom - rcClient.top) / (dYMax - dYMin));
   ddProp[C2DI_PROP_XMIDDLE] = (dXMin + dXMax) / 2.00;
   ddProp[C2DI_PROP_YMIDDLE] = (dYMin + dYMax) / 2.00;
   UserZoom( (dSclX < dSclY) ? dSclX : dSclY ); // zoom, update, done
}


/*********************************************************
* UserZoom
* Sets the zoom, rounds off the value, and updates the
* renderer.
* Negative values: Relative zoom; positive: Absolute
*********************************************************/
void CSysWin2d::UserZoom(double dZm) {
   if(dZm < 0.00) {
      ddProp[C2DI_PROP_ZOOM] *= -dZm;
   } else {
      ddProp[C2DI_PROP_ZOOM]  = dZm;
   }
   ddProp[C2DI_PROP_ZOOM] = floor(ddProp[C2DI_PROP_ZOOM]*100.00 + 0.50)/100.00; // round to 2dp
   if(ddProp[C2DI_PROP_ZOOM] <= 0.00) ddProp[C2DI_PROP_ZOOM] = 1.00;
   UpdateProperties();
   App()->PropManager()->OnPaint(TRUE);     // update values only
   UpdateAllVxActiveRect();
   Refresh();

}

/*********************************************************
* InvertFrame
* Inverts a frame or a line on the current window
*********************************************************/
void CSysWin2d::InvertFrame(void) {
   RECT   rc;                               // inversion part
   HDC    hdc;                              // window device context
   HBRUSH hbrOld;                           // restore previous brush
   int x0,y0,x1,y1, iWid;                   // positions and width

   //===Prepare===========================================
   if(iFrameTyp==C2DC_FRAME_NONE) return;   // ignore null frame
   if(hwRenderer == NULL) return;           // ignore if no renderer
   hdc = GetDC(hwRenderer);                 // retrieve the device context
   if(hdc == NULL) return;                  // skip if problem with dc

   //===Frames============================================
   switch(iFrameTyp) {
   //---Selection rectangles--------------------
   case C2DC_FRAME_SELECT:
   case C2DC_FRAME_ZOOM:
      hbrOld = (HBRUSH) SelectObject(hdc,   // select patterned brush
         (iFrameTyp==C2DC_FRAME_SELECT) ? App()->Brush(CAPPI_BRUSHDASH) :
         (iFrameTyp==C2DC_FRAME_ZOOM)   ? App()->Brush(CAPPI_BRUSHHATCH) :
         App()->Brush(CAPPI_BRUSHDASH));
      iWid =
         (iFrameTyp==C2DC_FRAME_SELECT) ? 1 :
         (iFrameTyp==C2DC_FRAME_ZOOM)   ? 2 :
         1;
      x0 = Cnv2WndX(MIN(drcInvFrame.Left, drcInvFrame.Right));
      y0 = Cnv2WndY(MIN(drcInvFrame.Top, drcInvFrame.Bottom));
      x1 = Cnv2WndX(MAX(drcInvFrame.Left, drcInvFrame.Right));
      y1 = Cnv2WndY(MAX(drcInvFrame.Top, drcInvFrame.Bottom));
      PatBlt(hdc, x0+iWid, y0     , x1-x0-iWid, iWid      , PATINVERT); // ^^
      PatBlt(hdc, x1-iWid, y0+iWid, iWid      , y1-y0-iWid, PATINVERT); // >>
      PatBlt(hdc, x0     , y1-iWid, x1-x0-iWid, iWid      , PATINVERT); // vv
      PatBlt(hdc, x0     , y0     , iWid      , y1-y0-iWid, PATINVERT);      // <<
      SelectObject(hdc, hbrOld);            // restore brush
      break;
   //---Measurement line------------------------
   // Left<->Right and Top<->Bottom may not be ascending;
   // the represent the direction of the line
#define C2DC_INV_SQUARE          3          // radius of inverted rectangle
   case C2DC_FRAME_MEASURE:
      SetRect(&rc,                          // upper
         Cnv2WndX(drcInvFrame.Left  ) - C2DC_INV_SQUARE,
         Cnv2WndY(drcInvFrame.Top   ) - C2DC_INV_SQUARE,
         Cnv2WndX(drcInvFrame.Left  ) + C2DC_INV_SQUARE,
         Cnv2WndY(drcInvFrame.Top   ) + C2DC_INV_SQUARE);
      InvertRect(hdc, &rc);
      SetRect(&rc,                          // middle
         Cnv2WndX((drcInvFrame.Left + drcInvFrame.Right ) / 2.00) - C2DC_INV_SQUARE,
         Cnv2WndY((drcInvFrame.Top  + drcInvFrame.Bottom) / 2.00) - C2DC_INV_SQUARE,
         Cnv2WndX((drcInvFrame.Left + drcInvFrame.Right ) / 2.00) + C2DC_INV_SQUARE,
         Cnv2WndY((drcInvFrame.Top  + drcInvFrame.Bottom) / 2.00) + C2DC_INV_SQUARE);
      InvertRect(hdc, &rc);
      SetRect(&rc,                          // lower
         Cnv2WndX(drcInvFrame.Right ) - C2DC_INV_SQUARE,
         Cnv2WndY(drcInvFrame.Bottom) - C2DC_INV_SQUARE,
         Cnv2WndX(drcInvFrame.Right ) + C2DC_INV_SQUARE,
         Cnv2WndY(drcInvFrame.Bottom) + C2DC_INV_SQUARE);
      InvertRect(hdc, &rc);
      break;

   //---Rotate / pan crosshairs-----------------
   case C2DC_FRAME_PAN:
   case C2DC_FRAME_ROTATE:
      SetRect(&rc, ixCenter, iyCenter-C2DC_ROTATE_CROSS, ixCenter+1, iyCenter+C2DC_ROTATE_CROSS);
      InvertRect(hdc, &rc);
      SetRect(&rc, ixCenter-C2DC_ROTATE_CROSS, iyCenter, ixCenter+C2DC_ROTATE_CROSS, iyCenter+1);
      InvertRect(hdc, &rc);
      break;
   }

   //===Finalize==========================================
   ReleaseDC(hwRenderer, hdc);              // release the window dc again
}


/*********************************************************
* Print
* Since the metafile-to-clipboard thing crapped out, here
* is my first attempt since 2001 to print something.
* The clever thing is that since we have the Cnv2Wnd fun-
* ctions, all we need to do is  replace the zoom and ori-
* gin properties with  printer values, and call the paint
* routine.
*********************************************************/
void CSysWin2d::Print(HDC hdcPrint) {
   double dZoomOld;                         // window zoom value
   int    ixCtrOld, iyCtrOld;               // window center values
   RECT   rcClient;                         // equivalent scaling to window
   int    iPrintWid, iPrintHig;             // printer width/height (pixels)
   double dSclX, dSclY;                     // scaling in axes (smaller used for both)

   if(hdcPrint==NULL) return;               // ignore bad calls

   //---Keep old values-------------------------
   dZoomOld = ddProp[ C2DI_PROP_ZOOM    ];
   ixCtrOld = ixCenter;
   iyCtrOld = iyCenter;

   //---Scaling---------------------------------
   GetClientRect(hwRenderer, &rcClient);    // window for equivalent scaling
   iPrintWid = GetDeviceCaps(hdcPrint, HORZRES);
   iPrintHig = GetDeviceCaps(hdcPrint, VERTRES);
   dSclX = (rcClient.right==rcClient.left) ? 1.00 : (double) iPrintWid / (double) (rcClient.right - rcClient.left);
   dSclY = (rcClient.bottom==rcClient.top) ? 1.00 : (double) iPrintHig / (double) (rcClient.bottom - rcClient.top);

   ixCenter = iPrintWid / 2;
   iyCenter = iPrintHig / 2;
   ddProp[ C2DI_PROP_ZOOM    ] *= (dSclX < dSclY) ? dSclX : dSclY;

   //---Print-----------------------------------
   TextOut(hdcPrint, 0, 0, " ", 1);         // need this for rest to appear (?)
   OnPaint(hdcPrint);                       // print onto printer DC

   //---Restore---------------------------------
   ddProp[ C2DI_PROP_ZOOM    ] = dZoomOld;
   ixCenter = ixCtrOld;
   iyCenter = iyCtrOld;
}


/*********************************************************
* Paint
* For the usual calls, hdcIn = NULL (its default value in
* the declaration). It is used in printing, in which case
* it specifies the device context of the printer, and the
* memory device context shouldn't be used.
*********************************************************/
void CSysWin2d::OnPaint(HDC hdcIn) {
   char    szBuf[256];                      // formatted text buffer
   PAINTSTRUCT ps;                          // painting structure
   HDC      hdc;                            // device context to use
   double   dGrid, dStep;                   // grid lines
   int      iGrid;                          // grid line loop counter
   RECT     rcClient;                       // client rectangle
   CVertex *pVx;                            // vertex loop counter
   HPEN     hpOld;                          // restore pen
   HFONT    hfOld;                          // restore font
   HBRUSH   hbrOld;                         // old brush
   COLORREF rgbOld;                         // restore text color
   HDC      hdcMem;                         // memory DC
   HBITMAP  hbmOld, hbmMem;                 // memory bitmap
   UINT     uTxtAlignOld;                   // restore text alignment

   if(hwRenderer == NULL) return;           // ignore if no window
   if((pSystem==NULL) || (pSystem->VxTop()==NULL)) return; // ignore if no system or Vx (shouldn't happen)

   //===Preliminaries=====================================
   //---Printing--------------------------------
   if(hdcIn != NULL) {
      hdcMem = NULL;                        // not using memory..
      hbmMem = NULL;                        //..device contexts
      hfOld  =  (HFONT) SelectObject(hdc, App()->Font(CAPPI_FONTPRINT));

   //---GDI-------------------------------------
   } else {
      BeginPaint(hwRenderer, &ps);
      GetClientRect(hwRenderer, &rcClient);
      hdc = ps.hdc;                            // paint directly if memory not available
      do {
         if( (hdcMem=CreateCompatibleDC(ps.hdc))==NULL) break;
         if( (hbmMem=CreateCompatibleBitmap(ps.hdc, rcClient.right-rcClient.left, rcClient.bottom-rcClient.top))==NULL) break;
         hdc = hdcMem;                         // memory ok, use it
         hbmOld = (HBITMAP) SelectObject(hdc, hbmMem);
      } while(0);
      hfOld  =  (HFONT) SelectObject(hdc, App()->Font());
      InvertFrame();                        // clear it (in case of partial invalidation rect)
   }

   //---Save state------------------------------
   hpOld  =   (HPEN) SelectObject(hdc, App()->Pen());
   hbrOld = (HBRUSH) SelectObject(hdc, App()->Brush());
   rgbOld = SetTextColor(hdc, App()->Rgb(CAPPI_RGBBLACK));
   uTxtAlignOld = SetTextAlign(hdc, TA_LEFT);

   //---Background------------------------------
   if(hdcIn==NULL) FillRect(hdc, &rcClient, App()->Brush()); // clear window backrgound

   //===Grid==============================================
   // Don't print the grid, only show in window.
   if(CheckBit(C2DF_SNAPGRID) && (GridSize() > 0.00) && (hdcIn==NULL)) {
      SelectObject(hdc, App()->Pen(CAPPI_PENGRID));
      dStep = (ceil((Wnd2CnvX(8)-Wnd2CnvX(0)) / GridSize())) * GridSize();
      if(dStep == 0.00) dStep = GridSize();
      dGrid = 0.00;   while((iGrid=Cnv2WndX(dGrid))<rcClient.right)  { MoveTo(hdc, iGrid, rcClient.top); LineTo(hdc, iGrid, rcClient.bottom); dGrid += dStep; }
      dGrid = -dStep; while((iGrid=Cnv2WndX(dGrid))>rcClient.left)   { MoveTo(hdc, iGrid, rcClient.top); LineTo(hdc, iGrid, rcClient.bottom); dGrid -= dStep; }
      dGrid = 0.00;   while((iGrid=Cnv2WndY(dGrid))<rcClient.bottom) { MoveTo(hdc, rcClient.left, iGrid); LineTo(hdc, rcClient.right, iGrid); dGrid += dStep; }
      dGrid = -dStep; while((iGrid=Cnv2WndY(dGrid))>rcClient.top)    { MoveTo(hdc, rcClient.left, iGrid); LineTo(hdc, rcClient.right, iGrid); dGrid -= dStep; }
      SelectObject(hdc, App()->Pen(CAPPI_PENOPTIC));
      MoveTo(hdc, Cnv2WndX(0.00)-2, Cnv2WndY(0.00)); LineTo(hdc, Cnv2WndX(0.00)+3, Cnv2WndY(0.00));
      MoveTo(hdc, Cnv2WndX(0.00), Cnv2WndY(0.00)-2); LineTo(hdc, Cnv2WndX(0.00), Cnv2WndY(0.00)+3);
   }//if(showgrid)*/

   //===Patches===========================================
   SelectObject(hdc, App()->Brush(CAPPI_BRUSHREFINDEX));
   for(pVx=pSystem->VxTop(); pVx->Next(); pVx=(CVertex*) pVx->Next()) {
      SelectObject(hdc, App()->Pen(CAPPI_PENOPTIC));
      switch(pVx->Type()) {
      case CVX_TYPE_INCRYSTAL:
      case CVX_TYPE_INBREWSTER:
      case CVX_TYPE_INPLATE:
         if(pVx->VxLinked()==NULL) break;   // ignore bad links
         Renderer2dPatchPlate(hdc,
            Cnv2WndX(pVx->X()),
            Cnv2WndY(pVx->Y()),
            pVx->OpticCanvasAngle(),
            pVx->ROC(TAN),
            Cnv2WndX(pVx->VxLinked()->X()),
            Cnv2WndY(pVx->VxLinked()->Y()),
            pVx->VxLinked()->OpticCanvasAngle(),
            pVx->VxLinked()->ROC(TAN),
            Zoom()*OpticScale());
         break;
      case CVX_TYPE_PRISM1:
         if(pVx->VxLinked()==NULL) break;   // ignore bad link
         Renderer2dPatchPrism(hdc,
            Cnv2WndX(pVx->X()),
            Cnv2WndY(pVx->Y()),
            pVx->OpticCanvasAngle(),
            Cnv2WndX(pVx->VxLinked()->X()),
            Cnv2WndY(pVx->VxLinked()->Y()),
            pVx->VxLinked()->OpticCanvasAngle(),
            Zoom()*OpticScale(),
            pVx->_Prop(CVXI_PROP_N));

      }
   }
   SelectObject(hdc, App()->Brush(CAPPI_BRUSHWINDOW));

   //===Distance Markers==================================
   if(CheckBit(C2DF_SHOWDIST) && !pSystem->DraftMode()) {
      SetTextAlign(hdc, TA_CENTER);
      SetTextColor(hdc, App()->Rgb(CAPPI_RGBSEGMENT));
      for(pVx=pSystem->VxTop(); pVx->Next(); pVx=(CVertex*) pVx->Next()) {
         sprintf(szBuf, "%.0f", pVx->Dist2Next());
         TextOut(hdc,
            Cnv2WndX((pVx->X() + pVx->Next()->X()) / 2),
            Cnv2WndY((pVx->Y() + pVx->Next()->Y()) / 2) + 2,
            szBuf, strlen(szBuf));
      }
   }

   //===Info==============================================
   if(CheckBit(C2DF_SHOWANNOT)) {
      SetTextAlign(hdc, TA_LEFT);
      SetTextColor(hdc, App()->Rgb(CAPPI_RGBOPTIC));
      for(pVx=pSystem->VxTop(); pVx; pVx=(CVertex*) pVx->Next()) {
         switch(pVx->Type()) {
         case CVX_TYPE_MIRROR:
            if(pVx->AstigROCFL())
               sprintf(szBuf, "%.0lf / %.0lf", pVx->ROC(SAG), pVx->ROC(TAN));
            else
               sprintf(szBuf, "%.0lf", pVx->ROC(SAG));
            break;
         case CVX_TYPE_LENS:
         case CVX_TYPE_THERMALLENS:
            if(pVx->AstigROCFL())
               sprintf(szBuf, "%.0lf / %.0lf", pVx->FL(SAG), pVx->FL(TAN));
            else
               sprintf(szBuf, "%.0lf", pVx->FL(SAG));
            break;
         case CVX_TYPE_INCRYSTAL:
         case CVX_TYPE_INBREWSTER:
         case CVX_TYPE_INPLATE:
            if(pVx->AstigROCFL())
               sprintf(szBuf, "%.0lf / %.0lf (%.3lf)", pVx->ROC(SAG), pVx->ROC(TAN), pVx->RefIndex());
            else
               sprintf(szBuf, "%.0lf (%.3lf)", pVx->ROC(SAG), pVx->RefIndex());
            break;
         case CVX_TYPE_OUTPLATE:
         case CVX_TYPE_OUTCRYSTAL:
         case CVX_TYPE_OUTBREWSTER:
            if(pVx->AstigROCFL())
               sprintf(szBuf, "%.0lf / %.0lf", pVx->ROC(SAG), pVx->ROC(TAN));
            else
               sprintf(szBuf, "%.0lf", pVx->ROC(SAG));
            break;
         case CVX_TYPE_PRISM1:
            sprintf(szBuf, "%.3lf", pVx->RefIndex());
            break;
         case CVX_TYPE_OUTCOUPLER:
         case CVX_TYPE_FLATMIRROR:
         case CVX_TYPE_SCREEN:
         case CVX_TYPE_SOURCE:
         case CVX_TYPE_PRISM2:
         default:
            sprintf(szBuf, "");
            break;
         }
         TextOut(hdc, Cnv2WndX(pVx->X()) + 8, Cnv2WndY(pVx->Y()) +  8, pVx->Tag(), strlen(pVx->Tag()));
         TextOut(hdc, Cnv2WndX(pVx->X()) + 8, Cnv2WndY(pVx->Y()) + 24, szBuf, strlen(szBuf));
      }
   }

   //===Path==============================================
   // Show selections only in window
   if(!pSystem->DraftMode()) {
      for(pVx=pSystem->VxTop(); pVx->Next(); pVx=(CVertex*) pVx->Next()) {
         if(pVx->Selected() && (hdcIn==NULL)) {
            SelectObject(hdc, App()->Pen(CAPPI_PENSEGMENTSEL));
            MoveTo(hdc, Cnv2WndX(pVx->X()),         Cnv2WndY(pVx->Y()));
            LineTo(hdc, Cnv2WndX(pVx->Next()->X()), Cnv2WndY(pVx->Next()->Y()));
            SelectObject(hdc, App()->Pen(CAPPI_PENSEGMENTOUT));
            MoveTo(hdc, Cnv2WndX(pVx->X()),         Cnv2WndY(pVx->Y()));
            LineTo(hdc, Cnv2WndX(pVx->Next()->X()), Cnv2WndY(pVx->Next()->Y()));
         } else {
            SelectObject(hdc, App()->Pen(CAPPI_PENSEGMENT));
            MoveTo(hdc, Cnv2WndX(pVx->X()),         Cnv2WndY(pVx->Y()));
            LineTo(hdc, Cnv2WndX(pVx->Next()->X()), Cnv2WndY(pVx->Next()->Y()));
         }
      }
   //---Draft mode--------------------
   } else {
      SelectObject(hdc, App()->Pen(CAPPI_PENSEGMENT));
      for(pVx=pSystem->VxTop(); pVx->Next(); pVx=(CVertex*) pVx->Next()) {
         if(pVx->CheckBit(CVXF_DRAFTLINK) && (pVx->Next()->CheckBit(CVXF_DRAFTLINK))) {
            MoveTo(hdc, Cnv2WndX(pVx->X()),         Cnv2WndY(pVx->Y()));
            LineTo(hdc, Cnv2WndX(pVx->Next()->X()), Cnv2WndY(pVx->Next()->Y()));
         }
      }
   }

   //===Vertex============================================
   for(pVx=pSystem->VxTop(); pVx; pVx=(CVertex*) pVx->Next()) {
      //---Draft Mode---------------------------
      if(pSystem->DraftMode()) {
         if(pVx->Selected())
            SelectObject(hdc, App()->Pen(pVx->CheckBit(CVXF_DRAFTLINK) ? CAPPI_PENOPTICSEL : CAPPI_PENDRAFTOPTICSEL));
         else
            SelectObject(hdc, App()->Pen(pVx->CheckBit(CVXF_DRAFTLINK) ? CAPPI_PENOPTIC : CAPPI_PENDRAFTOPTIC));

         Renderer2dIcon(hdc, pVx,
            Cnv2WndX(pVx->X()), Cnv2WndY(pVx->Y()),
            Zoom()*OpticScale(),
            pVx->OpticCanvasAngle());

      //---Normal-------------------------------
      } else {
         //---Vertex icon-------------
         if(pVx->Selected() && (hdcIn==NULL)) SelectObject(hdc, App()->Pen(CAPPI_PENOPTICSEL));
         else                                 SelectObject(hdc, App()->Pen(CAPPI_PENOPTIC));
         Renderer2dIcon(hdc, pVx,
            Cnv2WndX(pVx->X()), Cnv2WndY(pVx->Y()),
            Zoom()*OpticScale(),
            pVx->OpticCanvasAngle());

         //---Spawn-------------------
         if(pVx->SysSpawned()) {
            Renderer2dSysSpawn(hdc,
               Cnv2WndX(pVx->X()), Cnv2WndY(pVx->Y()),
               Zoom()*OpticScale(),
               (pVx->Prev()==NULL) ? pVx->SegmentCanvasAngle()+M_PI : -pVx->Prev()->SegmentCanvasAngle());
         }

         //---Graph-------------------
         /*Renderer2dOpticGraph(hdc,
            Cnv2WndX(pVx->X()), Cnv2WndY(pVx->Y()),
            Zoom()*OpticScale(),
            pVx->OpticCanvasAngle());*/
      }
   }

   //===Mode==============================================
   if(((pSystem->StableABCD(SAG)) || (pSystem->StableABCD(TAN))) && !pSystem->DraftMode()) {
      for(pVx=pSystem->VxTop(); pVx->Next(); pVx=(CVertex*) pVx->Next()) {
         //---Symmetric---------------
         if(pVx->SymmetricQVx()) {
            if(pSystem->StableABCD(SAG)) {
               SelectObject(hdc, App()->Pen(CAPPI_PENSAGTAN));
               Renderer2dMode(hdc,
                  Cnv2WndX(pVx->X()), Cnv2WndY(pVx->Y()),
                  pVx->SegmentCanvasAngle(), Zoom()*ModeScale()/1000.00, Zoom()*pVx->Dist2Next(),
                  pVx->Q(SAG)->W0(), pVx->Q(SAG)->z0(), pVx->Q(SAG)->zR(),
                  pVx->Dist2Next() / pVx->RefIndex(), CheckBit(C2DF_SHOWWAIST) ? TRUE : FALSE);
            }

         //---Sag/Tan-----------------
         } else {
            if(pSystem->StableABCD(SAG)) {
               SelectObject(hdc, App()->Pen(CAPPI_PENSAG));
               Renderer2dMode(hdc,
                  Cnv2WndX(pVx->X()), Cnv2WndY(pVx->Y()),
                  pVx->SegmentCanvasAngle(), Zoom()*ModeScale()/1000.00, Zoom()*pVx->Dist2Next(),
                  pVx->Q(SAG)->W0(), pVx->Q(SAG)->z0(), pVx->Q(SAG)->zR(),
                  pVx->Dist2Next() / pVx->RefIndex(), CheckBit(C2DF_SHOWWAIST) ? TRUE : FALSE);
            }
            if(pSystem->StableABCD(TAN)) {
               SelectObject(hdc, App()->Pen(CAPPI_PENTAN));
               Renderer2dMode(hdc,
                  Cnv2WndX(pVx->X()), Cnv2WndY(pVx->Y()),
                  pVx->SegmentCanvasAngle(), Zoom()*ModeScale()/1000.00, Zoom()*pVx->Dist2Next(),
                  pVx->Q(TAN)->W0(), pVx->Q(TAN)->z0(), pVx->Q(TAN)->zR(),
                  pVx->Dist2Next() / pVx->RefIndex(), CheckBit(C2DF_SHOWWAIST) ? TRUE : FALSE);
            }
         }
      }
   }


   //===Drag Debug========================================
/*   int x, y, x1, y1;                        // window position of vertex

   SelectObject(hdc, App()->Pen(CAPPI_PENDEBUG));
   SelectObject(hdc, (HBRUSH) GetStockObject(NULL_BRUSH));
   //---Drag vertex-----------------------------
   if(pSystem->VxDrag()) {
      x = Cnv2WndX(pSystem->VxDrag()->X()); // vertex coordinates in window
      y = Cnv2WndY(pSystem->VxDrag()->Y());
      Ellipse(hdc, x-8, y-8, x+8, y+8);
   }

   //---Prev Anchor-----------------------------
   if(pSystem->VxAncPrev()) {
      x = Cnv2WndX(pSystem->VxAncPrev()->X());
      y = Cnv2WndY(pSystem->VxAncPrev()->Y());
      MoveTo(hdc, x, y-8);
      LineTo(hdc, x+8, y);
      LineTo(hdc, x, y+8);
      LineTo(hdc, x-8, y);
      LineTo(hdc, x, y-8);
   }

   //---Prev Stretch----------------------------
   if((pSystem->VxStrPrev()) && (pSystem->VxStrPrev()->Next())) {
      x = Cnv2WndX(pSystem->VxStrPrev()->X());
      y = Cnv2WndY(pSystem->VxStrPrev()->Y());
      x1= Cnv2WndX(pSystem->VxStrPrev()->Next()->X());
      y1= Cnv2WndY(pSystem->VxStrPrev()->Next()->Y());
      MoveTo(hdc, x,  y);
      LineTo(hdc, x1, y1);
   }

   //---Next Anchor-----------------------------
   if(pSystem->VxAncNext()) {
      x = Cnv2WndX(pSystem->VxAncNext()->X());
      y = Cnv2WndY(pSystem->VxAncNext()->Y());
      MoveTo(hdc, x, y-16);
      LineTo(hdc, x, y+16);
      MoveTo(hdc, x-16, y);
      LineTo(hdc, x+16, y);
   }

   //---Next Stretch----------------------------
   if((pSystem->VxStrNext()) && (pSystem->VxStrNext()->Next())) {
      x = Cnv2WndX(pSystem->VxStrNext()->X());
      y = Cnv2WndY(pSystem->VxStrNext()->Y());
      x1= Cnv2WndX(pSystem->VxStrNext()->Next()->X());
      y1= Cnv2WndY(pSystem->VxStrNext()->Next()->Y());
      MoveTo(hdc, x,  y);
      LineTo(hdc, x1, y1);
   }
// */

   //===Finalize==========================================
//Mouse.PaintAllActiveRect(hdc);

   //---Restore GDI-----------------------------
   SetTextAlign(hdc, uTxtAlignOld);         // restore alignment
   SetTextColor(hdc, rgbOld);               // restore text color
   SelectObject(hdc, hpOld);                // restore pen
   SelectObject(hdc, hfOld);                // restore font
   SelectObject(hdc, hbrOld);               // restore brush

   //---Blit------------------------------------
   if((hdcMem) && (hbmMem)) {
      BitBlt(ps.hdc, rcClient.left, rcClient.top,
         rcClient.right-rcClient.left, rcClient.bottom-rcClient.top,
         hdcMem, 0, 0, SRCCOPY);
      SelectObject(hdcMem, hbmOld);
   }
   if(hdcMem) DeleteDC(hdcMem);
   if(hbmMem) DeleteObject(hbmMem);

   //---Finish window-only painting-------------
   if(hdcIn==NULL) {
      EndPaint(hwRenderer, &ps);
      InvertFrame();                        // restore the frame, if visible
   }


//===Metafile Test=====================================
/* The theory of operation:
*------------------------------------------------------
*  HDC           hdcMeta;                   // metafile device context
*  HENHMETAFILE  hmfClip;                   // metafile on clipboard
*  HGLOBAL       hglbClip;                  // memory handle for clipboard
*  HENHMETAFILE *phmfGlb;                   // memory pointer for clipboard
*  int fnMapModeOld;
*  hdcMeta = CreateEnhMetaFile(NULL, (LPCTSTR) NULL, (RECT*) NULL, (LPCTSTR) NULL);
*  MoveToEx(hdcMeta, 0, 0, NULL);
*  LineTo(hdcMeta, 10, 10);
*  LineTo(hdcMeta, 10,  0);
*  LineTo(hdcMeta,  0,  0);
*  LineTo(hdcMeta,  1,  5);
*  hmfClip = CloseEnhMetaFile(hdcMeta);
*
*  //---Allocate memory----------------------
*  hglbClip = GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE, sizeof(HENHMETAFILE));
*  phmfGlb  = (HENHMETAFILE*) GlobalLock(hglbClip); // lock global memory
*  memcpy(phmfGlb, &hmfClip, sizeof(HENHMETAFILE)); // copy to global
*  GlobalUnlock(hglbClip);               // unlock the global memory
*
*  //---Copy to clipboard--------------------
*  OpenClipboard(hwRenderer);            // open the clipboard
*  EmptyClipboard();                     // clear anything there now
*  SetClipboardData(CF_ENHMETAFILE, hglbClip); // copy to cliboard
*  CloseClipboard();
*---------------------------------------------------------
* Remember to check things like hdcMeta!=0.
* The metafile part works as witnessed by a correct meta-
* file being created on disk if  the second argument con-
* tains a *.emf file name.
* Copying to clipboard doesn't, for some reason. Which is
* actually REALLY annoying.
* Oh well, let's try printing after all.
*********************************************************/
}
