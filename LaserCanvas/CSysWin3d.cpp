/*********************************************************
* CSysWin3d
* 3d renderer class. It is intended that the 3d rendering
* be done using OpenGL. However, a wireframe version must
* be included in case OpenGL is not available (or not de-
* sired by the user).
* $PSchlup 2006 $     $Revision 0 $
*********************************************************/
#include "CSysWin3d.h"

const char* CSysWin3d::CszSysWin3dProp[C3DI_NUM_PROP] = { // names for stored properties
   "OriginX"       ,                        // viewpoint center X
   "OriginZ"       ,                        // viewpoint center Z
   "CameraX"       ,                        // camera X
   "CameraY"       ,                        // camera Y
   "CameraZ"       ,                        // camera Z
   "CameraAngle"   ,                        // camera view angle
   "OpticScale"    ,                        // optic scale
   "ModeScale"     ,                        // mode scale
   "Flags"                                  // visibility flags
};





/*********************************************************
* Constructor
*********************************************************/
CSysWin3d::CSysWin3d(CSystem *pSys) : CSysWin(pSys) {
printf("+ Create CSysWin-3D- renderer @0x%08lx\n", this);
   CVertex *pVx;                            // loop counter for auto zoom
   double  dXMin, dXMax, dYMin, dYMax;      // system bounds

   //---Members---------------------------------
   iTool = C3DC_TOOL_ROTATE;
   ddProp[ C3DI_PROP_ORGX       ] =  0.00;  // (index) viewpoint center X
   ddProp[ C3DI_PROP_ORGZ       ] =  0.00;  // (index) viewpoint center Z
   ddProp[ C3DI_PROP_CAMX       ] =  0.00;  // (index) camera X
   ddProp[ C3DI_PROP_CAMY       ] = 10.00;  // (index) camera Y
   ddProp[ C3DI_PROP_CAMZ       ] =  0.00;  // (index) camera Z
   ddProp[ C3DI_PROP_CAMA       ] = 60.00;  // (index) camera view angle
   ddProp[ C3DI_PROP_OPTICSCALE ] = 25.00;  // (index) optic scale
   ddProp[ C3DI_PROP_MODESCALE  ] = 10.00;  // (index) mode scale
   ddProp[ C3DI_PROP_FLAGS      ] = (double) 0x0000;   // (index) (integer) flags

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
      ddProp[C3DI_PROP_ORGX] = (dYMin + dYMax) / 2.00;
      ddProp[C3DI_PROP_ORGZ] = (dXMin + dXMax) / 2.00;
      ddProp[C3DI_PROP_CAMX] = ddProp[C3DI_PROP_ORGX];
      ddProp[C3DI_PROP_CAMY] = 2.00*(dXMax-dXMin) / tan(ddProp[C3DI_PROP_CAMA]*M_PI/180.00);
      ddProp[C3DI_PROP_CAMZ] = ddProp[C3DI_PROP_ORGZ] - 1e-6;
   }


   //---Mouse-----------------------------------
   Mouse.CreateActive(0, 0, 0, 0, NULL, NULL, NULL, NULL, CSysWin3d::_MouseCallback, (void*) this, 0, (HMENU) NULL);

   //---Create window---------------------------
   hwRenderer = pSystem->CreateSysWinWindow(CSZ_WNDCLASS_SYSWIN3D, (LPVOID) this);

   //---Finalize--------------------------------
   UserSetTool(iTool);                      // set tool, set check marks
   OnResize();

}


/*********************************************************
* Destructor
* If the base class destructor  is declared VIRTUAL, the
* derived class destructor is executed first
*********************************************************/
CSysWin3d::~CSysWin3d() {
printf("- Deleted 3D renderer\n");
}


/*########################################################
 ## Data Files                                         ##
########################################################*/
/*********************************************************
* SaveSysWin
* Save the current renderer
*********************************************************/
void CSysWin3d::SaveSysWin(HANDLE hFile) {
   char szBuf[256];                         // string to write to buffer
   DWORD dwBytes;                           // number of bytes written
   int   iPrp;                              // property loop counter

   //---Preliminaries---------------------------
   if(hFile == NULL) printf("? CSysWin3d::SaveSysWin@30 Called with hFile=NULL\n");
   if(hFile == NULL) return;                // ignore if no file

   //---Header----------------------------------
   SaveSysWinHeader(hFile, Type());         // write the header

   //---Properties------------------------------
   for(iPrp=0; iPrp<C3DI_NUM_PROP; iPrp++) {
      sprintf(szBuf, "   %s = %lg\r\n", CszSysWin3dProp[iPrp], ddProp[iPrp]);
      WriteFile(hFile, szBuf, strlen(szBuf), &dwBytes, NULL);
   }

   //---Footer----------------------------------
   SaveSysWinFooter(hFile);                 // write the footer
}


/*********************************************************
* LoadSysWin
* Load renderer properties from data file
*********************************************************/
BOOL CSysWin3d::LoadSysWin(const char *pszDataFile, char *pszMin, char *pszMax) {
UNREFERENCED_PARAMETER(pszDataFile);
   char *psz;                               // pointer in file
   int   iPrp;                              // property loop counter

   if(pszMin==NULL) return(FALSE);          // couldn't load if no pointer
   if(pszMax==NULL) pszMax = pszMin + strlen(pszMin); // to end of file if not given

   //---Header------------------------
   LoadSysWinHeader(pszDataFile, pszMin, pszMax);

   //---Specifics---------------------
   for(iPrp=0; iPrp<C3DI_NUM_PROP; iPrp++) {
      psz = strstr(pszMin, CszSysWin3dProp[iPrp]);
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
void CSysWin3d::Refresh(void) {
   if(hwRenderer) InvalidateRect(hwRenderer, NULL, TRUE);
}


/*********************************************************
* UpdateTitle
*********************************************************/
void CSysWin3d::UpdateTitle(int iID) {
   char szRC[128];                          // string from resource file
   char szBuf[256];                         // formatted string
   LoadString(App()->GetInstance(), SZ_TITLE_SYSWIN3D, szRC, sizeof(szRC)/sizeof(char));
   sprintf(szBuf, szRC, (pSystem) ? ((pSystem->FileName()) ? pSystem->FileName() : "") : "");
   if(iID > 0) sprintf(szBuf+strlen(szBuf), " (%d)", iID);
   if(hwRenderer) SetWindowText(hwRenderer, szBuf);
}

/*********************************************************
* GraphPoint
*********************************************************/
void CSysWin3d::GraphPoint(int iRVar, int iPt) {
UNREFERENCED_PARAMETER(iPt);
   if(iRVar < 0) {                          // end of scan, steady condition
      Refresh();
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
const UINT CSysWin3d::CuProperties[] = {
   CPS_CNVHEADER     ,
   CPS_CNVMODESIZE   ,
   CPS_CNVOPTSIZE    ,
   CPS_CNV3DORG      , // "Look At"
   CPS_CNV3DCAM      , // "Camera"
   CPS_CNV3DCAMELEV  , // "Elevation"
   CPS_CNV3DCAMANGL  , // "Angle"

   // UseOpenGL
   // ShowPedestals (?!)
   0};
//=======================================================
void CSysWin3d::PrepareProperties(BOOL tfAct) {
   //---Prepare---------------------------------
   App()->PrepareVariableProperties(tfAct);
   App()->PrepareProperties(CuProperties, tfAct, CSysWin3d::_SysWinPropItemCallback, this);
   App()->EnableMenus(tfAct ? CSYSWINI_TYPE_3D : -1);
   pSystem->PrepareProperties(tfAct);

   //---Update----------------------------------
   if(tfAct) UpdateProperties();            // set initial values
}
/*********************************************************
* UpdateProperties
* Called from
* <- PrepareProperties(TRUE)
* <- SysWinPropItemCallback
*********************************************************/
void CSysWin3d::UpdateProperties(void) {
   CPropMgr *pMgr = App()->PropManager();   // assign for below
   pMgr->FindItemByID( CPS_CNVMODESIZE  )->SetItemValue(ddProp[C3DI_PROP_MODESCALE ]);
   pMgr->FindItemByID( CPS_CNVOPTSIZE   )->SetItemValue(ddProp[C3DI_PROP_OPTICSCALE]);
   pMgr->FindItemByID( CPS_CNV3DORG     )->SetItemDblValue(OrgX(), OrgZ());
   pMgr->FindItemByID( CPS_CNV3DCAM     )->SetItemDblValue(CamX(), CamZ());
   pMgr->FindItemByID( CPS_CNV3DCAMELEV )->SetItemValue(CamY());
   pMgr->FindItemByID( CPS_CNV3DCAMANGL )->SetItemValue(CamA());
   //pMgr->FindItemByID( CPS_CNVSNAPGRID  )->SetItemCheckBox(CheckBit(C2DF_SNAPGRID ) ? TRUE : FALSE);
}


/*********************************************************
* SysWinPropItemCallback
* Callback from property manager when an item is modified
* The callback function must be declared static.
*********************************************************/
BOOL CSysWin3d::_SysWinPropItemCallback(void *vData, UINT uID, void *pVoid) {
   return((pVoid) ? ((CSysWin3d*)pVoid)->SysWinPropItemCallback(vData, uID) : TRUE);
}
//******************************************************//
BOOL CSysWin3d::SysWinPropItemCallback(void *vData, UINT uID) {
   //---Process value---------------------------
   switch(uID) {
   case CPS_CNVMODESIZE:  ddProp[C3DI_PROP_MODESCALE ] = *(double*) vData; break;
   case CPS_CNVOPTSIZE:   ddProp[C3DI_PROP_OPTICSCALE] = *(double*) vData; break;
   case CPS_CNV3DORG:     SetOrg( ((double*)vData)[0], ((double*)vData)[1] ); break;
   case CPS_CNV3DCAM:     SetCam( ((double*)vData)[0], CamY(), ((double*)vData)[1], CamA()); break;
   case CPS_CNV3DCAMELEV: SetCam( CamX(), *(double*)vData, CamZ(), CamA()); break;
   case CPS_CNV3DCAMANGL: SetCam( CamX(), CamY(), CamZ(), *(double*)vData); break;
   //case CPS_CNVSNAPGRID:  ToggleBit( C2DF_SNAPGRID  ); break;
   default:
      return(TRUE);
   }

   //---Apply-----------------------------------
   UpdateProperties();                      // set the new values
   App()->PropManager()->OnPaint(TRUE);     // update values only
   Refresh();                               // refresh renderer window
   return(TRUE);
}

/*########################################################
 ## Mouse
########################################################*/
/*********************************************************
* UserSetTool
* Set the tool; also calls UpdateAllVxActiveRect, because
* the vx actives are only placed for the edit tool.
* This is almost identical to the 2d renderer version ex-
* cept that some tools are not allowed.
* Called from
* <- App::Menu callback
*********************************************************/
void CSysWin3d::UserSetTool(int k) {
   CMActive *pAct;                          // Canvas active

   //---New tool--------------------------------
   iTool = k;                               // set the tool
   if(iTool<0) iTool = C3DC_TOOL_MAX;       // wrap backwards
   if(iTool>C3DC_TOOL_MAX) iTool = 0;       // wrap forwards

   //---Limit selection-------------------------
   switch(iTool) {
   case C3DC_TOOL_ARROW:
   case C3DC_TOOL_MEASURE:
      iTool = C3DC_TOOL_ROTATE;
      break;
   }

   //---Actives---------------------------------
   pAct = Mouse.FindActiveByData(this, 0L); // find the active
   if(pAct) {
      switch(iTool) {
      case C3DC_TOOL_ARROW  :
         pAct->SetCursors(App()->Cursor(CAPPI_CUR_ARROW), NULL, NULL, NULL);
         break;
      case C3DC_TOOL_MEASURE:
         pAct->SetCursors(App()->Cursor(CAPPI_CUR_MEAS), NULL, NULL, NULL);
         break;
      case C3DC_TOOL_ZOOM   :
         pAct->SetCursors(App()->Cursor(CAPPI_CUR_ZOOMIN), NULL, NULL, NULL);
         break;
      case C3DC_TOOL_PAN    :
         pAct->SetCursors(App()->Cursor(CAPPI_CUR_PAN_ARROW), NULL, App()->Cursor(CAPPI_CUR_PAN), NULL);
         break;
      case C3DC_TOOL_ROTATE :
         pAct->SetCursors(App()->Cursor(CAPPI_CUR_ROTATE_ARROW), NULL, App()->Cursor(CAPPI_CUR_ROTATE), NULL);
         break;
      }
   }
   Mouse.MouseProc(0, 0, 0, 0);             // update cursors (this works!)

   //---Menu check marks------------------------
   App()->CheckMenu( CMU_TOOL_ARROW  , Tool()==C3DC_TOOL_ARROW   ? TRUE : FALSE);
   App()->CheckMenu( CMU_TOOL_MEASURE, Tool()==C3DC_TOOL_MEASURE ? TRUE : FALSE);
   App()->CheckMenu( CMU_TOOL_ZOOM   , Tool()==C3DC_TOOL_ZOOM    ? TRUE : FALSE);
   App()->CheckMenu( CMU_TOOL_PAN    , Tool()==C3DC_TOOL_PAN     ? TRUE : FALSE);
   App()->CheckMenu( CMU_TOOL_ROTATE , Tool()==C3DC_TOOL_ROTATE  ? TRUE : FALSE);
}

/*********************************************************
* MouseCallback
* The referenced version must be declared static
*********************************************************/
void CSysWin3d::_MouseCallback(int iMsg, int x, int y, int wKeys, void *pVoid, long int lData) {
   ((CSysWin3d*)pVoid)->MouseCallback(iMsg, x, y, wKeys, lData);
}
void CSysWin3d::MouseCallback(int iMsg, int x, int y, int wKeys, long int lData) {
UNREFERENCED_PARAMETER(lData);
   static double SdR, SdPhi, SdTheta, SdOrgX, SdOrgZ; // source distance, angles
   double dR, dPhi, dTheta;                 // new angles
   int xDn, yDn;                            // mouse down coordinates
   int iTmp;                                // temporary tool

   //===Starting location=================================
   // This is common to all tools
   if(iMsg==ACSM_DOWN) {
      SdR     = SQRT(SQR(CamX()-OrgX()) + SQR(CamY()) + SQR(CamZ()-OrgZ()));
      SdTheta = ATAN2(SQRT(SQR(CamX()-OrgX()) + SQR(CamZ()-OrgZ())), CamY());
      SdPhi   = ATAN2(CamZ()-OrgZ(), CamX()-OrgX());
      SdOrgX  = OrgX();
      SdOrgZ  = OrgZ();
      return;
   }

   //===Common Compound Tool==============================
   // Recall: wKeys is state when button goes down
   // Don't pass wKeys on to repeated callback, otherwise
   // we get lost in infinite recursion!
   if(wKeys & MK_CONTROL) {
      iTmp = iTool; iTool = (wKeys & MK_SHIFT) ? C3DC_TOOL_ZOOM : C3DC_TOOL_PAN;
      MouseCallback(iMsg, x, y, (wKeys & MK_SHIFT), lData);
      iTool = iTmp;
      return;
   }

   //===Tools=============================================
   switch(iMsg) {
   case ACSM_DRAG:
      Mouse.GetDownLocation(&xDn, &yDn);
      dR     = SdR;
      dTheta = SdTheta;
      dPhi   = SdPhi;

      switch(Tool()) {
      //===Zoom===========================================
      case C3DC_TOOL_ZOOM:
         dR = (double)(y-yDn) / 100;
         if(dR < -8.00) dR = -8.00;
         if(dR >  8.00) dR =  8.00;
         dR = SdR * exp(dR);
         break;

      //===Pan============================================
      case C3DC_TOOL_PAN:
         SetOrg(
            SdOrgX + (x-xDn)*SIN(SdPhi) + (y-yDn)*COS(SdPhi)*((SdTheta>M_PI_2)?-1.00:1.00),
            SdOrgZ + (x-xDn)*COS(SdPhi) - (y-yDn)*SIN(SdPhi)*((SdTheta>M_PI_2)?-1.00:1.00));
         break;
      //===Rotate 3D======================================
      default:
         dTheta -= 0.02 * (y-yDn);
         dPhi   += 0.02 * (x-xDn);
      }
      if(dTheta <  1e-6) dTheta =  1e-6;
      if(dTheta >  M_PI) dTheta =  M_PI;

      SetCam(
         OrgX() + dR * SIN(dTheta) * COS(dPhi),
         0.00   + dR * COS(dTheta),
         OrgZ() + dR * SIN(dTheta) * SIN(dPhi),
         CamA());
      UpdateProperties();
      App()->PropManager()->OnPaint(TRUE);
      Refresh();
      break;
   }

}

/*########################################################
 ## Window Callback
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
LRESULT CALLBACK CSysWin3d::_WndProcSysWin3d(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
   switch(uMsg) {
   case WM_CREATE:
      SetWindowLong(hWnd, 0, (LONG) (CSysWin3d*)
         ((LPMDICREATESTRUCT) (((LPCREATESTRUCT)lParam)->lpCreateParams))->lParam);
      return(0L);
   default:
      return( ((CSysWin3d*) GetWindowLong(hWnd, 0))->WndProcSysWin3d(hWnd, uMsg, wParam, lParam) );
   }
}
//******************************************************//
LRESULT CALLBACK CSysWin3d::WndProcSysWin3d(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
   switch(uMsg) {
   case WM_CLOSE:
      pSystem->DeleteSysWin(this);
      break;

   case WM_MDIACTIVATE:
      PrepareProperties((hWnd==(HWND)lParam) ? TRUE : FALSE);
      if(hWnd==(HWND)lParam) {
         UserSetTool(Tool());
         App()->PropManager()->OnPaint(TRUE);
      }
      break;

   case WM_SIZE:  OnResize(); return(DefMDIChildProc(hWnd, uMsg, wParam, lParam));
   case WM_PAINT: OnPaint(); break;

   case WM_CHAR:
      switch(wParam) {
      case 0x09:                            // tab
         UserSetTool(Tool() + ((GetAsyncKeyState(VK_SHIFT)&0x8000) ? -1 : 1)); // set next tool
         break;
      default:
         return(DefMDIChildProc(hWnd, uMsg, wParam, lParam));
      }
      break;

   case WM_MOUSEMOVE:
   case WM_LBUTTONDOWN:
   case WM_LBUTTONUP:
   case WM_RBUTTONDOWN:
   case WM_RBUTTONUP:
      Mouse.MouseProc(hWnd, uMsg, LOWORD(lParam), HIWORD(lParam));
      break;

   default:
      return(DefMDIChildProc(hWnd, uMsg, wParam, lParam));
   }
   return(0L);
}

/*########################################################
 ##
########################################################*/
/*********************************************************
* MenuCommand
* The application calls MenuCommand() of the current ren-
* derer for ANY renderer-specific command.
*********************************************************/
void CSysWin3d::MenuCommand(int iCmd) {
   double dScl;                             // scale factor
   switch(iCmd) {
   case CMU_TOOL_ZOOM   : UserSetTool(C3DC_TOOL_ZOOM   ); break;
   case CMU_TOOL_PAN    : UserSetTool(C3DC_TOOL_PAN    ); break;
   case CMU_TOOL_ROTATE : UserSetTool(C3DC_TOOL_ROTATE ); break;
   case CMU_CNV_ZOOMIN  :
   case CMU_CNV_ZOOMOUT :
      dScl = (iCmd==CMU_CNV_ZOOMIN) ? 0.75 : 1.50;
      SetCam(
         OrgX() + dScl*(CamX() - OrgX()),
         dScl*CamY(),
         OrgZ() + dScl*(CamZ() - OrgZ()),
         CamA());
      Refresh();
      break;
   }
}


/*********************************************************
* DebugPrint
*********************************************************/
void CSysWin3d::DebugPrint(char *psz, int *pInt) {
   for(int k=0; k<*pInt; k++) sprintf(psz+strlen(psz), "   ");
   sprintf(psz+strlen(psz), "CSysWin-3D Renderer 0x%08lx\n", (long) this);
}

/*********************************************************
* Print
*********************************************************/
void CSysWin3d::Print(HDC hdcPrint) {
   if(hdcPrint==NULL) return;               // ignore bad calls

   //---Print-----------------------------------
   TextOut(hdcPrint, 0, 0, " ", 1);         // need this for rest to appear (?)
   OnPaint(hdcPrint);                       // print onto printer DC

}

/*********************************************************
* User Set Functions
*********************************************************/
//===Origin===============================================
void CSysWin3d::SetOrg(double X, double Z) {
   ddProp[C3DI_PROP_ORGX] = X;
   ddProp[C3DI_PROP_ORGZ] = Z;
}

//===Camera===============================================
void CSysWin3d::SetCam(double X, double Y, double Z, double A) {
   ddProp[C3DI_PROP_CAMX] = X;
   ddProp[C3DI_PROP_CAMY] = Y;
   ddProp[C3DI_PROP_CAMZ] = Z;
   ddProp[C3DI_PROP_CAMA] = A;
   if(ddProp[C3DI_PROP_CAMA] <= 0.00) ddProp[C3DI_PROP_CAMA] = 1.00;
}



/*********************************************************
* OnResize
* Called whenever the window is resized. This is more im-
* portant for the OpenGL version than for the wireframe.
*********************************************************/
void CSysWin3d::OnResize(void) {
   RECT rcClient;                           // window client rectangle
   if(hwRenderer==NULL) return;             // ignore bad calls
   GetClientRect(hwRenderer, &rcClient);
   Mouse.FindActiveByData(this, 0)->SetActiveRect(&rcClient);
}


/*########################################################
 ## Paint
########################################################*/
/*********************************************************
* OnPaint
*********************************************************/
void CSysWin3d::OnPaint(HDC hdcIn) {
   char    szBuf[256];                      // formatted text buffer
   PAINTSTRUCT ps;                          // painting structure
   HDC      hdc;                            // device context to use
   RECT     rcClient;                       // client rectangle
   CVertex *pVx;                            // vertex loop counter
   HPEN     hpOld;                          // restore pen
   HDC      hdcMem;                         // memory DC
   HBITMAP  hbmOld, hbmMem;                 // memory bitmap

   if(hwRenderer == NULL) return;           // ignore if no window
   if((pSystem==NULL) || (pSystem->VxTop()==NULL)) return; // ignore if no system or Vx (shouldn't happen)

   //===Preliminaries=====================================
   //---Printing--------------------------------
   if(hdcIn != NULL) {
      hdcMem = NULL;                        // not using memory..
      hbmMem = NULL;                        //..device contexts
      rcClient.right  = GetDeviceCaps(hdcIn, HORZRES);
      rcClient.bottom = GetDeviceCaps(hdcIn, VERTRES);
      rcClient.left   = 0.15*rcClient.right;
      rcClient.top    = 0.15*rcClient.bottom;
      rcClient.right  = 0.85*rcClient.right;
      rcClient.bottom = 0.85*rcClient.bottom;
      hdc = hdcIn;

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
   }

   //---Save state------------------------------
   hpOld  =   (HPEN) SelectObject(hdc, App()->Pen());

   //---Background------------------------------
   if(hdcIn==NULL) FillRect(hdc, &rcClient, App()->Brush()); // clear window backrgound

   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   // Wireframe Rendering
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   double xyzLoc[3];
   double xyzLkAt[3];
   xyzLoc[0]  = CamX(); xyzLoc[1]  = CamY(); xyzLoc[2]  = CamZ();
   xyzLkAt[0] = OrgX(); xyzLkAt[1] = 0.00;   xyzLkAt[2] = OrgZ();

   //---Baseplate-------------------------------
   SelectObject(hdc, App()->Pen(CAPPI_PEN3DDARK));
   Renderer3dBaseplate(pSystem->VxTop(), OpticScale());
   Renderer3dWireframe(hdc, xyzLoc, xyzLkAt, CamA(), &rcClient);


   for(pVx=pSystem->VxTop(); pVx; pVx=(CVertex*)pVx->Next()) {
      //===Line===========================================
      if(!pSystem->DraftMode()) {
         SelectObject(hdc, App()->Pen(CAPPI_PENSEGMENT));
         Renderer3dPath(pVx);
         Renderer3dWireframe(hdc, xyzLoc, xyzLkAt, CamA(), &rcClient);
      }

      //===Vertex=========================================
      SelectObject(hdc, App()->Pen(CAPPI_PENOPTIC));
      Renderer3dIcon(pVx, OpticScale());
      Renderer3dWireframe(hdc, xyzLoc, xyzLkAt, CamA(), &rcClient);

      //===Mode===========================================
      if(pSystem->StableABCD(SAG) && pSystem->StableABCD(TAN) && !pSystem->DraftMode()) {
         SelectObject(hdc, App()->Pen(CAPPI_PENSAGTAN));
         Renderer3dMode(pVx, ModeScale());
         Renderer3dWireframe(hdc, xyzLoc, xyzLkAt, CamA(), &rcClient);
      }
   }


   //===Finalize==========================================
//Mouse.PaintAllActiveRect(hdc);
//sprintf(szBuf, "(%lg, %lg, %lg) --> (%lg, %lg)", CamX(), CamY(), CamZ(), OrgX(), OrgZ()); TextOut(hdc, 0, 0, szBuf, strlen(szBuf));

   //---Restore GDI-----------------------------
   SelectObject(hdc, hpOld);                // restore pen

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
   }

}
