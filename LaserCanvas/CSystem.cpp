/*********************************************************
* CSystem
*
* This is the New School--the System maintains a chain of
* renderers based on the CSysWin class.
*
* $PSchlup 2006 $     $Revision 5 $
*********************************************************/
#include "CSystem.h"                        // class header

//===Name Table===========================================
const char* CSystem::CszSysType[CSYS_TYPE_MAX+1] = {// saved file header types
      "Propagation", "Resonator", "Outcoupled"};

const char* CSystem::CszVarFormat[CSYSC_NUM_SAVE] = {
   "Variable(%s)"  ,                        // variable values
   "Range(%s)"                              // variable range
};

const char* CSystem::CszSysEquationName[CSYSI_NUM_EQTN] = {// equation names
   "Wavelength", "MSquared", "MSquareTan",
   "InputwSag", "InputwTan", "InputRzSag", "InputRzTan",
   "Rotation", "StartX", "StartY"};

const char* CSystem::CszSysAuxSaveName[CSYSC_NUM_AUXSAVE] = {
   "InputType", "MSqAsymmetric", "DraftMode"};

const char* CSystem::CszSysSaveFcnString[CSYSI_PROP_FCNMAX+1] = {
   "PhysicalLength", "OpticalLength", "ModeSpacing", "Stability"};

const char* CSystem::CszValidNameChar =     // symbols allowed in system and vertex names
   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_";

const char* CSystem::CszSysInputParam = NULL; // names for input parameter specification for (int) ddProp(CSYSI_PROP_INPUTPARM)

const char* CSystem::CszSysFcnNames   = NULL; // plottable properties for CSYSI_PROP

int CSystem::iUntitledCounter = 0;          // enumerate new systems

/*********************************************************
* Constructor
*********************************************************/
CSystem::CSystem(CApplication *pApp) { printf("+ Created CSys\n");
   char  szBuf[512];                        // string table
   char *pszBuf;                            // temporary pointer
   int   iLen;                              // length of string
   int k;                                   // system property loop counter

   //---Invalidate all members------------------
   pAppParent  = pApp;                      // parent application
   memset(szSysTag, 0x00, sizeof(szSysTag)); // clear the tag
   pszFullFile = NULL;                      // no file name yet
   pszFileName = NULL;                      // no file name yet
   pVxTop      = NULL;                      // top of vertex chain
   pVxSpawn    = NULL;                      // spawning (parent) vertex system
   pSysWinTop  = NULL;                      // no renderers
   iSysType    = CSYS_TYPE_PROP;            // default system type
   for(k=0; k<CSYSI_NUM_EQTN; k++) EqSys[k].ParseDoubleEquation(0.00);
   for(k=0; k<CSYSI_NUM_PROP; k++) ddSysProp[k] = 0.0000; // clear properties
   pSysNext    = NULL;                      // system chain (maintained by app)
   iOpticNumber = 0;                        // names for optics

   pVxDrag     = NULL;                      // dragging vertex
   pVxAncPrev  = NULL;                      // previous anchor (rotation)
   pVxStrPrev  = NULL;                      // previous stretch segment
   pVxAncNext  = NULL;                      // next anchor (rotation)
   pVxStrNext  = NULL;                      // next stretch segment

   //---Default values--------------------------
   sprintf(szSysTag, "Sys_%08lx", this);    // default tag string
   EqSys[CSYSI_EQTN_WAVLEN     ].ParseDoubleEquation(1064.00); // [nm] set a default wavelength
   EqSys[CSYSI_EQTN_MSQUARED   ].ParseDoubleEquation(   1.00); // M^2 of beam (both, or sagittal)
   EqSys[CSYSI_EQTN_MSQUAREDTAN].ParseDoubleEquation(   1.00); // M^2 of beam (tangential)
   EqSys[CSYSI_EQTN_INPUTWSAG  ].ParseDoubleEquation( 200.00); // sagittal waist / spot
   EqSys[CSYSI_EQTN_INPUTWTAN  ].ParseDoubleEquation( 300.00); // tangential waist / spot
   EqSys[CSYSI_EQTN_INPUTRZSAG ].ParseDoubleEquation(   0.00); // sagittal curv / dist
   EqSys[CSYSI_EQTN_INPUTRZTAN ].ParseDoubleEquation(   0.00); // tangential curv / dist
   EqSys[CSYSI_EQTN_ROTATION   ].ParseDoubleEquation(   0.00); // rotation
   EqSys[CSYSI_EQTN_STARTX     ].ParseDoubleEquation(   0.00); // start X position
   EqSys[CSYSI_EQTN_STARTY     ].ParseDoubleEquation(   0.00); // start Y position

   //---String table----------------------------
   // Here delimited with '~' (including at end!)
   if(CszSysInputParam==NULL) {
      LoadString(pAppParent->GetInstance(), CPL_SYSINPPARAM, szBuf, sizeof(szBuf)/sizeof(char));
      for(k=0, iLen=strlen(szBuf); k<iLen; k++) if(szBuf[k]=='~') szBuf[k] = '\0';
      pszBuf = (char*) malloc((iLen+1)*sizeof(char)); // allocate buffer
      memset(pszBuf, 0x00, (iLen+1)*sizeof(char));    // empty the buffer
      memcpy(pszBuf, szBuf, iLen*sizeof(char));       // copy the formatted string
      CszSysInputParam = (const char*) pszBuf;        // fix pointer
   }
   if(CszSysFcnNames==NULL) {
      LoadString(pAppParent->GetInstance(), CPL_SYSFCNNAMES, szBuf, sizeof(szBuf)/sizeof(char));
      for(k=0, iLen=strlen(szBuf); k<iLen; k++) if(szBuf[k]=='~') szBuf[k] = '\0';
      pszBuf = (char*) malloc((iLen+1)*sizeof(char)); // allocate buffer
      memset(pszBuf, 0x00, (iLen+1)*sizeof(char));    // empty the buffer
      memcpy(pszBuf, szBuf, iLen*sizeof(char));       // copy the formatted string
      CszSysFcnNames = (const char*) pszBuf;        // fix pointer
   }

}


/*********************************************************
* Destructor
*********************************************************/
///TODO: Remove spawning from parent systems!
CSystem::~CSystem() {
   CVertex *pVx;
   //===Offspring========================================
   for(pVx=pVxTop; pVx!=NULL; pVx=pVx->Next()) {
      if(pVx->SysSpawned()) {
         delete(pVx->SysSpawned());         // recursively delete child systems
      }
   }

   //===Parent===========================================
   if(pVxSpawn != NULL) {
      pVxSpawn->SetSysSpawned(NULL);        // unlink from parent vx
   }

   //===This System======================================
   //---Destroy all renderers-------------------
   while(pSysWinTop) DeleteSysWin(pSysWinTop); // delete whole renderer chain

   //---Kill all vertices-----------------------
   while(pVxTop) DeleteVx(pVxTop);          // delete vertex chain

   //---Buffers---------------------------------
   if(pszFullFile) delete(pszFullFile); pszFullFile = NULL;
   if(pszFileName) delete(pszFileName); pszFileName = NULL;
printf("-CSys deleted\n");
}



/*********************************************************
* Wavelength functions
* Just to make sure that all derived systems use the same
* wavelength, only the top system stores it. Thus the Get
* and Set functions for ANY system must reference the top
* system's value.
*********************************************************/
//===Wavelength===========================================
void CSystem::SetWLen(double dWLen) {
   if(pVxSpawn) pVxSpawn->SysParent()->SetWLen(dWLen); // recursive up the chain
   else         EqSys[CSYSI_EQTN_WAVLEN].ParseDoubleEquation(dWLen);  // set top system's value
}
double CSystem::WLen(void) {
   if(pVxSpawn) return(pVxSpawn->SysParent()->WLen()); // recursive up chain
   else         return(ddSysProp[CSYSI_PROP_WAVLEN]);  // return top system's value
}

//===M^2==================================================
void CSystem::SetMSquared(int iPln, double dMSq) {
   if(pVxSpawn) pVxSpawn->SysParent()->SetMSquared(iPln, dMSq); // recursive up the chain
   else {
      if(MSqAsymmetric())
         EqSys[(iPln==SAG) ? CSYSI_EQTN_MSQUARED : CSYSI_EQTN_MSQUAREDTAN].ParseDoubleEquation(dMSq);
      else
         EqSys[CSYSI_EQTN_MSQUARED   ].ParseDoubleEquation(dMSq);  // set top system's value
         EqSys[CSYSI_EQTN_MSQUAREDTAN].ParseDoubleEquation(dMSq);  // set top system's value
   }
}
double CSystem::MSquared(int iPln) {
   if(pVxSpawn) return(pVxSpawn->SysParent()->MSquared(iPln)); // recursive up chain
   else {
      if(MSqAsymmetric())
         return(ddSysProp[(iPln==SAG) ? CSYSI_PROP_MSQUARED : CSYSI_PROP_MSQUAREDTAN]);
      else
         return(ddSysProp[CSYSI_PROP_MSQUARED]);  // return top system's value
   }
}
//===Asymmetric M^2=======================================
BOOL CSystem::MSqAsymmetric(void) {
   if(pVxSpawn) return(pVxSpawn->SysParent()->MSqAsymmetric());
   else         return(CheckBit(CSYSF_MSQASYM) ? TRUE : FALSE);
}
void CSystem::SetMSqAsymmetric(BOOL tf) {
   if(pVxSpawn) pVxSpawn->SysParent()->SetMSqAsymmetric(tf);
   else       { if(tf) SetBit(CSYSF_MSQASYM); else ClearBit(CSYSF_MSQASYM); }
}
//===Optic Counter========================================
int CSystem::NextOpticNumber(void) {
   if(pVxSpawn) return(pVxSpawn->SysParent()->NextOpticNumber());
   else return(++iOpticNumber);
}


/*********************************************************
* Common
* These parameters are specific to EACH system, including
* spawned ones. These  functions are  usually only called
* from external functions only. We set the properties di-
* rectly here so that a call to App::ScanAll isn't neces-
* sary for a cosmetics-only change.
*********************************************************/
void CSystem::SetRotation(double dRot) {
   EqSys[CSYSI_EQTN_ROTATION].ParseDoubleEquation(dRot);
   ddSysProp[CSYSI_PROP_ROTATION] = dRot;
};
void CSystem::SetStartPos(double dX, double dY) {
   EqSys[CSYSI_EQTN_STARTX].ParseDoubleEquation(dX);
   EqSys[CSYSI_EQTN_STARTY].ParseDoubleEquation(dY);
   ddSysProp[CSYSI_PROP_STARTX] = dX;
   ddSysProp[CSYSI_PROP_STARTY] = dY;
}

//===Input Param==========================================
void CSystem::UserSetInputParam(int iTyp) {
   ClearBit(CSYSF_INPUTPARAM);
   SetBit(iTyp & CSYSF_INPUTPARAM);
   PrepareProperties(TRUE);                 // swap visibility
   App()->ScanAll();
}

/*********************************************************
* UserSetDraftMode
* Draft mode is a configuration where the CVertex chain
* does NOT represent a continuous system; the user can
* place vertices wherever on the Canvas. During draft
* mode, SolveSystem isn't solved.
*********************************************************/
void CSystem::UserSetDraftMode(BOOL tfDraft) {
   char     szBuf[256];                     // error message
   CVertex *pVx;                            // vertex loop counter
   CVertex *pVxAnc, *pVxAncNext;            // next anchor
   double   dAngPrev, dAng, dAngFace;       // angle to anchor

   if(SysType() != CSYS_TYPE_RESO) return;  // ignore all but resonators

   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   // Into Draft Mode
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   if(tfDraft) {
      //---Check cavity-------------------------
      for(pVx=pVxTop; pVx; pVx=(CVertex*) pVx->Next()) {
         if(  (pVx->Type()==CVX_TYPE_PRISM1)
            ||(pVx->Type()==CVX_TYPE_PRISM2)
            ||(pVx->SysSpawned()!=NULL) ) {
            LoadString(App()->GetInstance(), SZERR_SYSDRAFTOPTICS, szBuf, sizeof(szBuf)/sizeof(char));
            MessageBox(App()->GetWindow(), szBuf, CSZ_CAPTION_APPLICATION, MB_OK | MB_ICONSTOP);
            return;
         }
      }
      //---Set all angles-----------------------
      for(pVx=pVxTop; pVx; pVx=(CVertex*) pVx->Next()) {
         switch(pVx->Type()) {
         case CVX_TYPE_MIRROR:
         case CVX_TYPE_FLATMIRROR:
            pVx->SetFaceAngle(0.00);
            break;
         }
         pVx->ApplyEquations(App()->Vars());
         pVx->SetCanvasPosition(pVx->X(), pVx->Y(), 0.00);
      }

      //---Link Flags---------------------------
      SetDraftMode(TRUE);                   // enter draft mode, check menu
      for(pVx=pVxTop; pVx; pVx=(CVertex*) pVx->Next()) {
         pVx->SetBit(CVXF_DRAFTLINK);       // link all to start with
         switch(pVx->Type()) {
         case CVX_TYPE_FLATMIRROR:          // change flats to normal mirrors
            pVx->SetType(CVX_TYPE_MIRROR);
            break;
         case CVX_TYPE_INPLATE:
         case CVX_TYPE_INBREWSTER:
         case CVX_TYPE_INCRYSTAL:
            pVx->MoveVxToDraft(pVx->X(), pVx->Y());
            break;
         }
      }

   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   // Out of Draft Mode
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   // Here, we must check that the system is valid.
   // Note that this may be called from the toolbar WITH-
   // OUT all of them having been locked so set the flags
   // for each optic again here.
   } else {
      //===Finish Linking=================================
      for(pVx=pVxTop; pVx; pVx=(CVertex*) pVx->Next()) {
         if(!pVx->CheckBit(CVXF_DRAFTLINK)) {
            pVx->SetBit(CVXF_DRAFTLINK);
         }
      }
      //===Check Validity=================================
      for(pVx=pVxTop; pVx->Next(); pVx=(CVertex*)pVx->Next());
      if((pVxTop->Type() != CVX_TYPE_MIRROR)
         || (pVx->Type() != CVX_TYPE_MIRROR)) {
         LoadString(App()->GetInstance(), SZERR_SYSDRAFTCAVITY, szBuf, sizeof(szBuf)/sizeof(char));
         MessageBox(App()->GetWindow(), szBuf, CSZ_CAPTION_APPLICATION, MB_OK | MB_ICONSTOP);
         return;
      }

      //===Reconstitute System============================
      //---Starting-----------------------------
      SetStartPos(pVxTop->X(), pVxTop->Y());
      pVxAnc = pVxTop;

      //---Vertices-----------------------------
      // The last vertex necessarily counts as an anchor
      while(pVxAnc) {
         for(pVxAncNext=pVxAnc->Next(); pVxAnc && pVxAncNext->Next(); pVxAncNext=(CVertex*) pVxAncNext->Next()) {
            if(   (pVxAncNext->Type()==CVX_TYPE_MIRROR)
               || (pVxAncNext->Type()==CVX_TYPE_FLATMIRROR) ) break;
         }
         dAng = ATAN2(pVxAncNext->Y() - pVxAnc->Y(), pVxAncNext->X() - pVxAnc->X());

         //---Starting Rotation--------------------
         if(pVxAnc == pVxTop) {
            SetRotation(180.00/M_PI * dAng);

         //---Subsequent angles--------------------
         } else {
            switch(pVxAnc->Type()) {
            case CVX_TYPE_MIRROR:
            case CVX_TYPE_FLATMIRROR:
               dAngFace = (dAngPrev - dAng + M_PI) / 2.00;
               while(dAngFace < -M_PI_2) dAngFace += M_PI;
               while(dAngFace > +M_PI_2) dAngFace -= M_PI;
               pVxAnc->SetFaceAngle(180.00/M_PI * dAngFace);
               break;
            }
         }
         dAngPrev = dAng;                   // keep canvas angle for next segment

         //---Distances----------------------------
         for(pVx=pVxAnc; (pVx!=pVxAncNext) && (pVx->Next()); pVx=(CVertex*) pVx->Next()) {
            pVx->SetDist2Next(SQRT(
               SQR(pVx->Next()->X() - pVx->X())  +  SQR(pVx->Next()->Y() - pVx->Y()) ));
         }

         //---Next Anchor--------------------------
         pVxAnc = pVxAncNext;
         if(pVxAncNext->Next()==NULL) break; // break out once reached last optic
      }

      //===Finalize=======================================
      //---Link flag----------------------------
      SetDraftMode(FALSE);                  // finish draft mode, clear menu
   }

   //===Finish============================================
   App()->EnableMenus(CSYSWINI_TYPE_2D);    // update menus and buttons
   PrepareProperties(TRUE);                 // show / hide as necessary
   PrepareVxProperties(TRUE);               // show / hide as necessary
   App()->PropManager()->OnResize();        // reposition items
   App()->ScanAll();                        // final completion: Re-scan
}

/*********************************************************
* SetDraftMode
* This only checks / clears the menu item and sets the
* flags
*********************************************************/
void CSystem::SetDraftMode(BOOL tfDft) {
   if(tfDft) SetBit(CSYSF_DRAFTMODE); else ClearBit(CSYSF_DRAFTMODE);
   App()->CheckMenu(CMU_TOOL_DRAFT, DraftMode() ? TRUE : FALSE);
}


/*########################################################
 ## Vertex Functions                                   ##
########################################################*/
CSystem* CSystem::GetParentSystem(void) { return((pVxSpawn) ? pVxSpawn->SysParent() : NULL); };
CSystem* CSystem::GetTopSystem(void)    { return((pVxSpawn) ? pVxSpawn->SysParent()->GetTopSystem() : this); };
/*********************************************************
* UserCreateSystem
* Creates a default system of vertices.  Defaults include
* a normal  straight resonator and straight  propagation,
* and maybe later a ring resonator.
*********************************************************/
void CSystem::UserCreateSystem(int iTyp) {
   CVertex *pVx;                            // vertex pointer

   if(pVxTop != NULL) printf("? CSystem::UserCreateSystem@32 pVxTop isn't NULL\n");
   if(pVxTop != NULL) return;               // ignore if system already defined

   iSysType = iTyp;                         // store system type
   pVx = pVxTop;                            // start at top of system
   switch(iSysType) {
   //---Resonator-------------------------------
   case CSYS_TYPE_RESO:
      pVx = CreateVxAfter(pVx, CVX_TYPE_MIRROR);
      pVx->SetROC(200.00, 200.00);
      pVx->SetDist2Next(250.00);
      pVx->Select(TRUE);

      pVx = CreateVxAfter(pVx, CVX_TYPE_MIRROR);
      pVx->SetROC(200.00, 200.00);
      break;

   //---Propagation-----------------------------
   case CSYS_TYPE_PROP:
      pVx = CreateVxAfter(pVx, CVX_TYPE_SOURCE);
      pVx->SetDist2Next(250.00);
      pVx->Select(TRUE);

      pVx = CreateVxAfter(pVx, CVX_TYPE_SCREEN);
      break;

   //---Output coupled--------------------------
   case CSYS_TYPE_SPAWN:
      pVx = CreateVxAfter(pVx, CVX_TYPE_OUTCOUPLER);
      pVx->SetDist2Next(250.00);
      pVx->SetRefIndex(1.50);
      pVx->Select(TRUE);

      pVx = CreateVxAfter(pVx, CVX_TYPE_SCREEN);
      break;
   }

   //---Update scans----------------------------
   pAppParent->ScanAll();
}


/*********************************************************
* MenuInsertVx
* Inserts a Vx after the selection, if it's allowed. Also
* casts from a generic type  (mirror, lens) to restricted
* (flat mirror, thermal lens) if that helps.
* Specific allowances:
*  - Between In an Out: Thermal lens, screen
*  - Between Prism A B: Flat mirror, screen
*
* Note: This is a MENU-CALLED function!
*
* This function DEFINES how the application  will respond
* when Insert is clicked  for a multiple selection. Since
* there seems no reason not to, we  insert an optic after
* each selected vx, if we can.
*
* Called from
* <- App::Menu commands
*********************************************************/
CVertex* CSystem::MenuInsertVx(int iTyp) {
   CVertex *pVxNew1, *pVxNew2;              // new vertex object(s)
   CVertex *pVx, *pVxAux;                   // vertex loop counter
   CVertex *pVxLnk;                         // linked-to vertex
   BOOL     tfOk;                           // check if it's ok to insert
   double   dDist;                          // segment length
   double   dX, dY, dXMin, dXMax, dYMin, dYMax; // limits of vertices
   CSysWin *pSWin;                          // renderers

   if(   (iTyp==CVX_TYPE_SOURCE)            // these types cannot be inserted by user
      || (iTyp==CVX_TYPE_OUTCOUPLER)
      ) return(NULL);

   pVxNew1 = pVxNew2 = NULL;                // none inserted yet

   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   // Draft Mode
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   if(DraftMode()) {
      //===Establish Position=============================
      //---Center coordinate--------------------
      dXMin=dXMax=pVxTop->X();
      dYMin=dYMax=pVxTop->Y();
      for(pVx=pVxTop; pVx; pVx=(CVertex*) pVx->Next()) {
         if(pVx->X() < dXMin) dXMin = pVx->X();
         if(pVx->X() > dXMax) dXMax = pVx->X();
         if(pVx->Y() < dYMin) dYMin = pVx->Y();
         if(pVx->Y() > dYMax) dYMax = pVx->Y();
      }
      dX = (dXMin + dXMax) / 2.00;
      dY = (dYMin + dYMax) / 2.00;

      //---Check overlap------------------------
      pVx = pVxTop;
      while(pVx) {
         if((fabs(pVx->X()-dX)<5.00) && (fabs(pVx->Y()-dY)<5.00)) {
            dX += 20.00;
            dY += 20.00;
            pVx = pVxTop;
         } else {
            pVx = (CVertex*) pVx->Next();
         }
      }
      //===Insert=========================================
      //---Find last Vx-------------------------
      for(pVx=pVxTop; pVx->Next(); pVx=(CVertex*) pVx->Next());

      //---Create-------------------------------
      pVxNew1 = pVxNew2 = NULL;          // track new objects
      switch(iTyp) {
      case CVX_TYPE_INCRYSTAL:
      case CVX_TYPE_OUTCRYSTAL:
         pVxNew1 = CreateVxAfter(pVx,     CVX_TYPE_INCRYSTAL);
         pVxNew2 = CreateVxAfter(pVxNew1, CVX_TYPE_OUTCRYSTAL);
         break;
      case CVX_TYPE_INBREWSTER:
      case CVX_TYPE_OUTBREWSTER:
         pVxNew1 = CreateVxAfter(pVx,     CVX_TYPE_INBREWSTER);
         pVxNew2 = CreateVxAfter(pVxNew1, CVX_TYPE_OUTBREWSTER);
         break;
      case CVX_TYPE_INPLATE:
      case CVX_TYPE_OUTPLATE:
         pVxNew1 = CreateVxAfter(pVx,     CVX_TYPE_INPLATE);
         pVxNew2 = CreateVxAfter(pVxNew1, CVX_TYPE_OUTPLATE);
         break;
      default:
         pVxNew1 = CreateVxAfter(pVx, iTyp);
         break;
      }
      //---Links-----------------------------
      if(pVxNew2!=NULL) {
         pVxNew1->SetLinked(pVxNew2);
         pVxNew2->SetLinked(pVxNew1);
      }

      //---Distances-------------------------
      if(pVxNew2!=NULL) {                // single inserted only
         pVxNew1->SetThickness(10.00);
      }

      //---Specifics-------------------------
      switch(pVxNew1->Type()) {
      case CVX_TYPE_LENS:
         pVxNew1->SetFL(500.00, 500.00);
         break;
      case CVX_TYPE_INCRYSTAL:
      case CVX_TYPE_INBREWSTER:
      case CVX_TYPE_INPLATE:
      case CVX_TYPE_OUTCOUPLER:
         pVxNew1->SetRefIndex( 1.50);
         break;
      }

      //---Position-----------------------------
      pVxNew1->ApplyEquations(App()->Vars());
      pVxNew1->MoveVxToDraft(dX, dY);



   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   // Standard
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   } else {
      //===Through Chain=====================================
      // (excluding last vertex, see below)
      for(pVx=pVxTop; pVx; pVx=(CVertex*)pVx->Next()) {
         //---Check selection----------------------
         if(!pVx->Selected()) continue;        // ignore those that aren't selected
         if(pVx->Next()==NULL) {
            if (pVxNew1 != NULL) continue;     // ignore last if some already inserted
            pVx = pVx->Prev();                 // otherwise insert BEFORE the last optic
         }

         //---Skip ambiguous-----------------------
         pVxAux = pVx;                         // auxiliary checking vertex
         while((pVxAux)                        // skip ambiguous
            &&((pVxAux->Type()==CVX_TYPE_SCREEN)) ) {
            pVxAux = pVxAux->Prev();           // get to the previous
         }

         //===Establish permissions==========================
         pVxLnk = NULL;                        // not linked
         if(pVxAux==NULL) {
            tfOk = TRUE;                       // always allow at top of cavity
         } else {
            switch(pVxAux->Type()) {           // examine auiliary types
            case CVX_TYPE_MIRROR:              // generic optics, post-pairs
            case CVX_TYPE_LENS:
            case CVX_TYPE_OUTCRYSTAL:
            case CVX_TYPE_OUTBREWSTER:
            case CVX_TYPE_PRISM2:
            case CVX_TYPE_OUTPLATE:
            case CVX_TYPE_SOURCE:
            case CVX_TYPE_OUTCOUPLER:
               tfOk = TRUE;                    // generally allowed
               switch(iTyp) {
               case CVX_TYPE_FLATMIRROR:  iTyp = CVX_TYPE_MIRROR;     break;
               case CVX_TYPE_THERMALLENS: iTyp = CVX_TYPE_LENS;       break;
               case CVX_TYPE_OUTCRYSTAL:  iTyp = CVX_TYPE_INCRYSTAL;  break;
               case CVX_TYPE_OUTBREWSTER: iTyp = CVX_TYPE_INBREWSTER; break;
               case CVX_TYPE_PRISM2:      iTyp = CVX_TYPE_PRISM1;     break;
               case CVX_TYPE_OUTPLATE:    iTyp = CVX_TYPE_INPLATE;    break;
               }
               break;

            case CVX_TYPE_INBREWSTER:          // types with one thermal lens or screen only
            case CVX_TYPE_INPLATE:
            case CVX_TYPE_INCRYSTAL:
               tfOk = FALSE;                   // probably not allowed
               pVxLnk = pVxAux;                // link to input vx
               if(pVxAux->Type()==CVX_TYPE_THERMALLENS) pVxLnk = pVxAux->VxLinked();
               if(pVxAux->Next()==NULL) break; // check for bad cavity
               if(   ((pVxAux->Type()==CVX_TYPE_INBREWSTER) && (pVxAux->Next()->Type()==CVX_TYPE_OUTBREWSTER))
                  || ((pVxAux->Type()==CVX_TYPE_INPLATE   ) && (pVxAux->Next()->Type()==CVX_TYPE_OUTPLATE   ))
                  || ((pVxAux->Type()==CVX_TYPE_INCRYSTAL ) && (pVxAux->Next()->Type()==CVX_TYPE_OUTCRYSTAL ))
               ) {                             // allow lens or screen if nothing in between
                  switch(iTyp) {
                  case CVX_TYPE_LENS:   iTyp = CVX_TYPE_THERMALLENS; tfOk = TRUE; break;
                  case CVX_TYPE_SCREEN: tfOk = TRUE; break;
                  }
               }
               break;

            case CVX_TYPE_PRISM1:              // intra-prism types
            case CVX_TYPE_FLATMIRROR:
               tfOk = FALSE;                   // generally not allowed
               if(pVxAux->Type()==CVX_TYPE_FLATMIRROR) pVxLnk = pVxAux->VxLinked();
               else pVxLnk = pVxAux;
               switch(iTyp) {
               case CVX_TYPE_MIRROR: iTyp = CVX_TYPE_FLATMIRROR; tfOk = TRUE; break;
               case CVX_TYPE_SCREEN: tfOk = TRUE; break;
               }
               break;

            case CVX_TYPE_THERMALLENS:         // nothing may follow a thermal lens
            default: tfOk = FALSE;
            }
         }

         //===Insert if allowed==============================
         if(tfOk) {
            pVxNew1 = pVxNew2 = NULL;          // track new objects
            switch(iTyp) {
            case CVX_TYPE_INCRYSTAL:
            case CVX_TYPE_OUTCRYSTAL:
               pVxNew1 = CreateVxAfter(pVx,     CVX_TYPE_INCRYSTAL);
               pVxNew2 = CreateVxAfter(pVxNew1, CVX_TYPE_OUTCRYSTAL);
               break;
            case CVX_TYPE_INBREWSTER:
            case CVX_TYPE_OUTBREWSTER:
               pVxNew1 = CreateVxAfter(pVx,     CVX_TYPE_INBREWSTER);
               pVxNew2 = CreateVxAfter(pVxNew1, CVX_TYPE_OUTBREWSTER);
               break;
            case CVX_TYPE_INPLATE:
            case CVX_TYPE_OUTPLATE:
               pVxNew1 = CreateVxAfter(pVx,     CVX_TYPE_INPLATE);
               pVxNew2 = CreateVxAfter(pVxNew1, CVX_TYPE_OUTPLATE);
               break;
            case CVX_TYPE_PRISM1:
            case CVX_TYPE_PRISM2:
               pVxNew1 = CreateVxAfter(pVx,     CVX_TYPE_PRISM1);
               pVxNew2 = CreateVxAfter(pVxNew1, CVX_TYPE_PRISM2);
               break;
            default:
               pVxNew1 = CreateVxAfter(pVx, iTyp);
               break;
            }
            //---Links-----------------------------
            if(pVxNew2==NULL) {
               pVxNew1->SetLinked(pVxLnk);     // link if necessary
            } else {
               pVxNew1->SetLinked(pVxNew2);
               pVxNew2->SetLinked(pVxNew1);
            }

            //---Distances-------------------------
            dDist = pVx->Dist2Next();          // get current distance
            if(pVxNew2==NULL) {                // single inserted only
               pVx    ->SetDist2Next(dDist / 2.00);
               pVxNew1->SetDist2Next(dDist / 2.00);
            } else {                           // pair inserted
               pVx    ->SetDist2Next(dDist / 2.00 - 5.00);
               pVxNew1->SetThickness(10.00);
               pVxNew2->SetDist2Next(dDist / 2.00 - 5.00);
            }

            //---Specifics-------------------------
            switch(pVxNew1->Type()) {
            case CVX_TYPE_MIRROR:
            case CVX_TYPE_FLATMIRROR:
               pVxNew1->SetFaceAngle(90.00);
               break;
            case CVX_TYPE_LENS:
            case CVX_TYPE_THERMALLENS:
               pVxNew1->SetFL(500.00, 500.00);
               break;
            case CVX_TYPE_INCRYSTAL:
            case CVX_TYPE_INBREWSTER:
            case CVX_TYPE_PRISM1:
            case CVX_TYPE_INPLATE:
            case CVX_TYPE_OUTCOUPLER:
               pVxNew1->SetRefIndex( 1.50);
               break;
            }
         }
      }//endfor(pVx) loop

   }//endif(DraftMode())

   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   // Finalize
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   //===Update All========================================
   //---Renderers-------------------------------
   for(pSWin=pSysWinTop; pSWin; pSWin = (CSysWin*) pSWin->Next()) {
      switch(pSWin->Type()) {
      case CSYSWINI_TYPE_2D:
         ((CSysWin2d*) pSWin)->CreateAllVxActive();
         break;
      }
   }

   //---Selection-------------------------------
   if(pVxNew1) {
      SelectAllVx(FALSE);
      if(pVxNew2) pVxNew2->Select(TRUE); else pVxNew1->Select(TRUE);
   }

   //---Application-----------------------------
   PrepareVxProperties(TRUE);
   App()->ScanAll();
   return(pVxNew1);
}

/*********************************************************
* MenuDeleteVx
* Deletes any and all of the  vertices currently selected
* in the system.
* How's this for crafty: Instead of adding the distances,
* we use the equation  to add it. If there's no variables
* or there's a problem parsing, we use the added distance
* otherwise this should preserve the system equations.
* This is a MENU-RESPONSE function!
*********************************************************/
void CSystem::MenuDeleteVx(void) {
   char     szEqDist[4096];                 // combine strings (!)
   CEquation Eq;                            // temp. equation object
   CVertex *pVx;                            // vertex loop counter
   CVertex *pVxDel, *pVxAux, *pVxLast;      // first and last to delete
   CSysWin *pSWin;                          // renderer update chain
   double   dDist;                          // summed total distance

   //===Through System====================================
   // Note that we always start at the top again when a
   // vertex is deleted.
   for(pVx=pVxTop; pVx; pVx=(CVertex*) pVx->Next()) {
      if(!pVx->Selected()) continue;        // ignore those not selected
      if((pVx->Prev()==NULL) ) continue; // cannot delete first
      if((pVx->Next()==NULL) ) continue; // cannot delete last

      //---Compound objects---------------------
      switch(pVx->Type()) {                 // search links in compound objects
      case CVX_TYPE_INCRYSTAL:
      case CVX_TYPE_INPLATE:
      case CVX_TYPE_INBREWSTER:
      case CVX_TYPE_PRISM1:
         pVxDel = pVx; pVxLast = pVx->VxLinked(); // first to last
         break;
      case CVX_TYPE_OUTCRYSTAL:
      case CVX_TYPE_OUTPLATE:
      case CVX_TYPE_OUTBREWSTER:
      case CVX_TYPE_PRISM2:
         pVxDel = pVx->VxLinked(); pVxLast = pVx; // last and first
         break;
      default:
         pVxDel = pVx; pVxLast = pVx;       // delete on its own
         break;
      }

      //===Delete each====================================
      if(pVxDel == NULL) continue;          // this shouldn't happen
      pVx   = pVxDel->Prev();               // point to just before deletion
      dDist = pVx->Dist2Next();             // sum total lost distance
      sprintf(szEqDist, "(%s)", ((CEquation*)pVx->Eq(CVXI_EQTN_DIST2NEXT))->_GetSrcEqStr());
      do {
         pVxAux = (pVxDel==pVxLast) ? NULL : pVxDel->Next();
         dDist += pVxDel->Dist2Next();      // add this distance to total
         sprintf(szEqDist+strlen(szEqDist), "+(%s)", ((CEquation*)pVxDel->Eq(CVXI_EQTN_DIST2NEXT))->_GetSrcEqStr());
         DeleteVx(pVxDel);                  // delete this vx
         pVxDel = pVxAux;                   // next one in chain to delete
      } while (pVxDel);                     // delete all in the chain
      if( (Eq.ParseEquation(szEqDist, App()->VarsString()) != EQERR_NONE)
         || (!Eq.ContainsVariables())) {
         pVx->SetDist2Next(dDist);             // set total distance
      } else {
         ((CEquation*)pVx->Eq(CVXI_EQTN_DIST2NEXT))->ParseEquation(szEqDist, App()->VarsString());
      }
   }

   //===Draft Mode========================================
   if(DraftMode()) {
      for(pVx=pVxTop; pVx; pVx=(CVertex*) pVx->Next()) pVx->ClearBit(CVXF_DRAFTLINK);
   }

   //===Update All========================================
   //---Renderers-------------------------------
   for(pSWin=pSysWinTop; pSWin; pSWin = (CSysWin*) pSWin->Next()) {
      switch(pSWin->Type()) {
      case CSYSWINI_TYPE_2D:
         ((CSysWin2d*) pSWin)->CreateAllVxActive();
         break;
      }
   }

   //---Application-----------------------------
   PrepareVxProperties(TRUE);               // selection changed, so update
   App()->EnableMenus(CSYSWINI_TYPE_2D);    // update menus and buttons
   App()->ScanAll();

}


/*********************************************************
* CreateVxAfter
* Inserts a vertex  after the pVxAft,  which may be NULL.
* Throughout,the CSystem maintains the chain of vertices,
* although each vertex stores  its nearest neighbours for
* convenience.
* Each new vertex  is created without any  neighbours, so
* the insertion in the chain is done here.
*********************************************************/
CVertex* CSystem::CreateVxAfter(CVertex *pVxAft, int iTyp) {
   CVertex *pVxNew;

   pVxNew = new CVertex(this, iTyp);        // create the vertex

   //---Default insertion-----------------------
   // If no vertex is supplied but the chain already
   // exists, insert at the end of the chain
   if((pVxAft==NULL) && (pVxTop!=NULL)) {
      pVxAft = pVxTop;
      while(pVxAft->Next() != NULL) pVxAft = pVxAft->Next();
   }

   //---Maintain chain--------------------------
   if(pVxAft != NULL) {                     // chain Aft <-[New]-> Next
      //---Following------------------
      if(pVxAft->Next() != NULL) {
         pVxAft->Next()->SetPrev(pVxNew);   // New <- Next
      }
      //---Preceding------------------
      pVxNew->SetNext(pVxAft->Next());      // New -> Next
      pVxAft->SetNext(pVxNew);              // Aft -> New
   }
   pVxNew->SetPrev(pVxAft);                 // Aft <- New

   //---Chain head------------------------------
   if(pVxTop == NULL) pVxTop = pVxNew;

   return(pVxNew);
}


/*********************************************************
* DeleteVx
* Delete the given Vx and  maintain the vertex chain. See
* comments at CreateVxAfter.
* This code does NOT ensure that  a valid system remains!
* Use UserDeleteVx  to check that the  given optical com-
* ponent may be deleted
*********************************************************/
void CSystem::DeleteVx(CVertex *pVxDel) {
   CSysWin *pSWin;                          // renderer loop counter
   if(pVxDel == NULL) printf("? CSystem::DeleteVx@84 Trying to delete a NULL vertex\n");
   if(pVxDel == NULL) return;               // ignore if no Vx to delete

   //===Delete Dependencies===============================
   //---Spawned system--------------------------
   if(pVxDel->SysSpawned() != NULL) {
      delete(pVxDel->SysSpawned());         // ensure spawned systems are deleted
   }

   //---Renderers-------------------------------
   pSWin = pSysWinTop;                      // start at top
   while(pSWin) {                           // loop with while because chain gets changed
      if(pSWin->Type() == CSYSWINI_TYPE_VXGRAPH) { // check it's relevant type
         if(((CSysWinVxGraph*) pSWin)->Vertex() == pVxDel) { // check if it's this vertex
            DeleteSysWin(pSWin);            // delete, maintain renderer chain
            pSWin = pSysWinTop;             // set back to start chain changed)..
            continue;                       //..and start over
         }//endif(this_vertex)
      }//endif(type)
      pSWin = (CSysWin*) pSWin->Next();  // advance to next
   }//endwhile

   //===Maintain Chain====================================
   //---Maintain chain--------------------------
   if(pVxDel->Next() != NULL) {             // Prev <-[Del]-> Next
      pVxDel->Next()->SetPrev(pVxDel->Prev()); // Prev <- Next
   }
   if(pVxDel->Prev() != NULL) {
      pVxDel->Prev()->SetNext(pVxDel->Next());
   }

   //---Chain head------------------------------
   if(pVxTop == pVxDel) pVxTop = pVxDel->Next(); // maintain head of chain
   delete(pVxDel);                          // delete the vertex
}


/*********************************************************
* MenuInsertSpawn
* Called when the insert  spawned system / output coupler
* is selected from the menu.
* Inserts at any available vertex (mirror) that is selec-
* ted.
*
* NOTE ON RECURSION:
* Most of  the functions are  set up to allow  recursion,
* i.e., spawning  spawned systems. However, it seems that
* this is unlikely to be useful, so  here we ARTIFICIALLY
* limit the output coupled systems to just one level. See
* also allowances in App()->EnableMenus().
*********************************************************/
CSystem* CSystem::MenuInsertSpawn(void) {
   CSystem *pSysNew;                        // new, spawned system
   CVertex *pVx;                            // vertex loop counter
   pSysNew = NULL;                          // no new system yet

   if(pVxSpawn) return(NULL);               // don't allow recursive

   //===Scan ThroughSystem================================
   for(pVx=pVxTop; pVx; pVx=(CVertex*) pVx->Next()) {
      if(pVx->Prev()==NULL) continue;       // ignore top optic (sorry, but it's a bitch in SolveSystem, so disallow it here)
      if(!pVx->Selected()) continue;        // ignore all but selection
      if(pVx->Type() != CVX_TYPE_MIRROR) continue; // allow only on mirror
      //---Insert spawned system----------------
      pSysNew = new CSystem(App());         // create empty system
      pVx->SetSysSpawned(pSysNew);          // attach system to spawning vx
      pSysNew->SetVxSpawn(pVx);             // set spawning vx
      pSysNew->UserCreateSystem(CSYS_TYPE_SPAWN); // set type, default optics
      pSysNew->UserCreateSysWin2d();        // create default canvas for system
   }

   //===Update============================================
   App()->ScanAll();                        // update all things

   return(pSysNew);
}



/*********************************************************
* SelectAllVx
*********************************************************/
void CSystem::SelectAllVx(BOOL tfSel) {
   CVertex *pVx;                            // vertex loop counter
   for(pVx=pVxTop; pVx; pVx=(CVertex*)pVx->Next()) {
      pVx->Select(tfSel);
   }
}

/*********************************************************
* SelectVxByRect
* Selects (or inverts  the selection) of  vertices within
* the supplied rectangle.
*********************************************************/
void CSystem::SelectVxByRect(double dXMin, double dYMin, double dXMax, double dYMax, BOOL tfInvert) {
   CVertex *pVx;                            // vertex loop counter
   for(pVx=pVxTop; pVx; pVx=(CVertex*)pVx->Next()) {
      if(   (pVx->X() >= dXMin)
         && (pVx->X() <= dXMax)
         && (pVx->Y() >= dYMin)
         && (pVx->Y() <= dYMax)) {
         if(tfInvert) pVx->Select( pVx->Selected() ? FALSE : TRUE );
         else         pVx->Select( TRUE );
      }
   }
}


/*********************************************************
* NumSelectedVx
* Returns the number of selected vertices. If ppVx is not
* NULL, it contains the first selection
* Called from
* <- PrepareVxProperties
*********************************************************/
int CSystem::NumSelectedVx(CVertex **ppVx) {
   int      iCnt;                           // selection counter
   CVertex *pVx;                            // vertex loop counter
   if(ppVx) *ppVx = NULL;                   // no vertices found yet
   for(pVx=pVxTop, iCnt=0; pVx; pVx=(CVertex*)pVx->Next()) {
      if(pVx->Selected()) {
         if((iCnt==0) && (ppVx!=NULL)) *ppVx = pVx;
         iCnt++;
      }
   }
   return(iCnt);
}


/*********************************************************
* NextSelectedVx
* Returns the  next selected vertex after  the given one.
* If pVxCur is NULL, starts searching from the top of the
* system. Used in PrepareVxProperties to cycle up through
* the selection.
*********************************************************/
CVertex* CSystem::NextSelectedVx(CVertex *pVxCur) {
   CVertex *pVx;                            // selected, returned vertex
   if(pVxCur == NULL) {
      for(pVx=pVxTop; pVx; pVx=(CVertex*)pVx->Next())
         if(pVx->Selected()) break;
   } else {
      for(pVx=(CVertex*) pVxCur->Next(); pVx; pVx=(CVertex*)pVx->Next())
         if(pVx->Selected()) break;
   }
   return(pVx);
}


/*********************************************************
* LinkNextVx
* Places the supplied vertex at the end of the draft mode
* link chain.
* The CVertex::CVXF_DRAFTLINK flag means that THAT vertex
* has been set correctly.
* The plan for now:
*  - User selects Draft Mode to enter draft mode
*  - Vx moved around willy-nilly, no limits on insertions
*  - User clicks Draft mode, enter Link mode
*  - User clicks each vx in turn
*     -> LinkNextVx called for each
*  - Once all Vx are selected, pop out of draft mode.
* Return value: Returns TRUE once there are no more Vx to
* link together.
* Blocks:  Since the link chain is already ok  within the
* block, we just need to link the input and output faces,
* which are themselves linked with VxLink.
*********************************************************/
BOOL CSystem::LinkNextVx(CVertex* pVxLinkIn) {
   CVertex *pVx;                            // loop counter
   CVertex *pVxCur;                         // Vx to insert after
   CVertex *pVxLinkTop, *pVxLinkEnd;        // top, end of linking block

   if(pVxLinkIn==NULL) return(FALSE);       // no vx, return

   //===Refractive Blocks=================================
   //---Default---------------------------------
   pVxLinkTop = pVxLinkEnd = pVxLinkIn;     // assume linking just one

   //---Blocks----------------------------------
   switch(pVxLinkIn->Type()) {
   case CVX_TYPE_INBREWSTER:
   case CVX_TYPE_INCRYSTAL:
   case CVX_TYPE_INPLATE:
      if(pVxLinkIn->VxLinked()) pVxLinkEnd = pVxLinkIn->VxLinked();
      break;
   case CVX_TYPE_OUTBREWSTER:
   case CVX_TYPE_OUTPLATE:
   case CVX_TYPE_OUTCRYSTAL:
   case CVX_TYPE_PRISM2:
   case CVX_TYPE_THERMALLENS:
   case CVX_TYPE_SCREEN:
      if(pVxLinkIn->VxLinked()) {
         pVxLinkTop = pVxLinkIn->VxLinked();
         pVxLinkEnd = pVxLinkIn->VxLinked()->VxLinked();
      }
      break;
   }

   //===First one=========================================
   // :: topCur <--> N   ...   P <--> Lktop..Lkend <--> N
   // :: Lktop..Lkend <--> topCur <--> N   ...   P <--> N
   if(!pVxTop->CheckBit(CVXF_DRAFTLINK)) {
      if(pVxTop != pVxLinkTop) {
         if(pVxLinkTop->Prev()) pVxLinkTop->Prev()->SetNext(pVxLinkEnd->Next());
         if(pVxLinkEnd->Next()) pVxLinkEnd->Next()->SetPrev(pVxLinkTop->Prev());
         pVxTop->SetPrev(pVxLinkEnd);
         pVxLinkEnd->SetNext(pVxTop);
         pVxLinkTop->SetPrev(NULL);
         pVxTop = pVxLinkTop;
      }

   //===Remaining=========================================
   } else {
      //---Find Vx to insert after-----------------
      for(pVxCur=pVxTop; pVxCur; pVxCur=(CVertex*) pVxCur->Next()) {
         if(!pVxCur->CheckBit(CVXF_DRAFTLINK)) break; // stop at first that isn't set
      }
      if(pVxCur==NULL) return(TRUE);           // no more to link, so probably ok
      pVxCur = pVxCur->Prev();                 // go back one

      //---Move Vx up------------------------------
      // P <--> Cur <--> N               ... P <--> Top..End <--> N
      // P <--> Cur <--> Top..End <--> N ... P <--> N
      if(pVxLinkTop->Prev()) pVxLinkTop->Prev()->SetNext(pVxLinkEnd->Next());
      if(pVxLinkEnd->Next()) pVxLinkEnd->Next()->SetPrev(pVxLinkTop->Prev());

      if(pVxCur ->Next()) pVxCur ->Next()->SetPrev(pVxLinkEnd);
      pVxLinkEnd->SetNext(pVxCur->Next());

      pVxLinkTop->SetPrev(pVxCur);
      pVxCur ->SetNext(pVxLinkTop);
   }

   //===Finalize==========================================
   //---Set link flag---------------------------
   for(pVx=pVxLinkTop; pVx; pVx=(CVertex*) pVx->Next()) {
      pVx->SetBit(CVXF_DRAFTLINK);       // this one ok now
      if(pVx==pVxLinkEnd) break;         // stop at end
   }

   //---Check if it's last one------------------
   for(pVx=pVxTop; pVx && pVx->CheckBit(CVXF_DRAFTLINK); pVx=(CVertex*) pVx->Next()); // check remaining ok ones
   return((pVx==NULL) ? TRUE : FALSE);   // returns TRUE once they're all done
}


/*********************************************************
* UnlinkVx
* Sister function to LinkNextVx, here for deselection. It
* is non-trivial because refractive blocks are treated as
* one  entity, and all subsequent  ones must also  be un-
* linked.
*********************************************************/
void CSystem::UnlinkVx(CVertex *pVxUnlink) {
   CVertex *pVx;

   pVx = pVxUnlink;                         // assume starting here

   //---Go to top of blocks---------------------
   switch(pVxUnlink->Type()) {
   case CVX_TYPE_OUTBREWSTER:
   case CVX_TYPE_OUTPLATE:
   case CVX_TYPE_OUTCRYSTAL:
   case CVX_TYPE_PRISM2:
   case CVX_TYPE_THERMALLENS:
   case CVX_TYPE_SCREEN:
      if(pVxUnlink->VxLinked()) pVx = pVxUnlink->VxLinked();
      break;
   }

   //---Unlink----------------------------------
   for(; pVx; pVx=(CVertex*) pVx->Next()) {
      pVx->ClearBit(CVXF_DRAFTLINK);
   }
}

/*########################################################
 ## Physics!                                           ##
########################################################*/
/*********************************************************
* ApplyEquations
* Applies the current variable values to evaluate all the
* vertex ABCD matrices in preparation for SolveSystemABCD
*********************************************************/
void CSystem::ApplyEquations(double *pcdVar) {
   int      iEqn;                           // equation loop counter
   CVertex *pVx;                            // vertex loop counter
   double   dWLen;                          // wavelength, stored in QSysInit
   int      k;                              // sagittal / tangential plane counter

   //---Properties-----------------------------
   if(pVxSpawn==NULL) {                     // top level system
      for(iEqn=0; iEqn<CSYSI_NUM_EQTN; iEqn++) {
         ddSysProp[CSYSI_PROP_EQTN+iEqn] = EqSys[iEqn].Answer(pcdVar);
      }
   } else {                                 // spawned system
      for(iEqn=CSYSI_EQTN_COMMON; iEqn<CSYSI_NUM_EQTN; iEqn++) {
         ddSysProp[CSYSI_PROP_EQTN+iEqn] = EqSys[iEqn].Answer(pcdVar);
      }
   }
   //---Wavelength------------------------------
   for(k=0; k<SAGTAN; k++) QSysInit[k].SetWLen(WLen()); // set each wavelength (WLen() returns parents' if spawned)
   for(k=0; k<SAGTAN; k++) QSysInit[k].SetMSq(MSquared(k)); // set the M^2 parameter

   //---System vertices-------------------------
   for(pVx = pVxTop; pVx; pVx = (CVertex*) pVx->Next()) {
      pVx->ApplyEquations(pcdVar);             // calculate each vertex
      if(pVx->SysSpawned()) pVx->SysSpawned()->ApplyEquations(pcdVar); // solve spawned systems
   }
}


/*********************************************************
* PlaceCanvasVertices
* Places the positions of the vertices on the canvas. The
* first vertex is always at the origin with a zero angle.
*********************************************************/
void CSystem::PlaceCanvasVertices(BOOL tfRecursive) {
   CVertex *pVx;
   double dX, dY, dA, dL;                   // position and angle

   if(DraftMode()) return;                  // ignore in draft mode

   //---Starting condition----------------------
   dX = StartX();                           // canvas X and..
   dY = StartY();                           //..Y position
   dA = Rotation() * M_PI / 180.00;         // canvas segment angle
   dL = 0.00;                               // canvas segment distance

   //---Place each vertex-----------------------
   for(pVx=pVxTop; pVx; pVx=(CVertex*) pVx->Next()) {
      pVx->SetCanvasPosition(dX, dY, dA);   // place with angle relative to previous segment
      dL = pVx->Dist2Next();                // read distance to next optic
      dA += pVx->Angle2Next();              // track path angle change
      dX += dL * COS(dA);                   // move on to..
      dY += dL * SIN(dA);                   //..next vertex
   }

   //---Recursion-------------------------------
   if(tfRecursive) for(pVx=pVxTop; pVx; pVx=(CVertex*) pVx->Next()) {
      if(pVx->SysSpawned()) pVx->SysSpawned()->PlaceCanvasVertices(tfRecursive);
   }
}


/*********************************************************
* GraphRendererPoint
* Passes the renderer point to  all renderers so that the
* graphs can extract their data point. Since this is only
* ever called from the Application level it is always re-
* cursive.
* Called from
*  <- CApp:: When something is changed
*********************************************************/
void CSystem::GraphRendererPoint(int iRVar, int iPt) {
   CSysWin *pSWin;                          // renderer loop counter
   CVertex *pVx;                            // vertex loop counter
   //---Update----------------------------------
   for(pSWin=pSysWinTop; pSWin; pSWin=pSWin->Next()) {
      pSWin->GraphPoint(iRVar, iPt);
   }
   //---Recursion-------------------------------
   for(pVx=pVxTop; pVx; pVx=(CVertex*) pVx->Next()) {
      if(pVx->SysSpawned()) pVx->SysSpawned()->GraphRendererPoint(iRVar, iPt);
   }
}

/*********************************************************
* RefreshRenderers
* Calls Refresh on any of this system's renderers.
*********************************************************/
void CSystem::RefreshRenderers(BOOL tfRecursive) {
   CSysWin *pSWin;                          // renderer loop counter
   CVertex *pVx;                            // vertex loop counter
   //---Update----------------------------------
   for(pSWin=pSysWinTop; pSWin; pSWin=pSWin->Next()) {
      pSWin->Refresh();
   }
   //---Recursion-------------------------------
   if(tfRecursive) for(pVx=pVxTop; pVx; pVx=(CVertex*) pVx->Next()) {
      if(pVx->SysSpawned()) pVx->SysSpawned()->RefreshRenderers(tfRecursive);
   }
}


/*********************************************************
* SolveSystem
* Solves the system ABCD. Although the answer is saved in
* the AbcdSys  matrices, more important is  that the sys-
* tem's initial Q values are stored.
* Each system is solved  invididually first, and then any
* spawned systems are calculated.
*
* SolveResonator
*
* The complex beam parameter q is defined in terms of the
* wavefront radius of curvature  R = R(z), 1/e^2 radius w
* = w(z), and wavelength lambda as
*     1     1        lambda
*    --- = ---  - j --------
*     q     R        pi w^2
* Its propagation in free space from the beam waist value
* q0 where R(0) = inf and w(0) = w0 is
*      q(z) = q0 + z
* The evolution  of the complex beam parameter  through a
* system  with transfer matrix M can be calculated  using
* the matrix elements as
*     1     C + D(1/q)
*    --- = ------------
*     q'    A + B(1/q)
* If M is the complete round-trip transfer matrix, then a
* self-consistent solution  q' = q leads to an expression
* for q at the reference plane of
*     1     D - A       i      _______________
*    --- = -------  - -----   /4 - (A + D)^2  `
*     q      2B        2|B| \/
* Stable resonators can be identified as having (A+D)^2 <
* 4, and the real and imaginary parts can be directly as-
* signed to the 1/q values.
* In addition, we can define  a generalized stability pa-
* rameter g as
*          tr(M)
*     g = -------
*            2
* which requires |g| < 1 to be stable and g ~ 0.5 for op-
* timal stability to disturbances.
*********************************************************/
void CSystem::SolveSystemABCD(void) {
   CMatrix2x2 Mx[SAGTAN];                   // matrices from vertex
   CVertex *pVx;                            // vertex loop counter
   int      k;                              // sagittal/tangential plane counter
   double   AplusD2;                        // term in square root for solving the resonator
   CRecQ    Q[SAGTAN];                      // 1/q through the cavity

   if(DraftMode()) return;                  // ignore in draft mode

   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   // Initial Q
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   switch(SysType()) {
   //===Propagation=======================================
   case CSYS_TYPE_PROP:
      switch(InputParam()) {
      case CSYSF_INPUTW0Z0:
         QSysInit[SAG].SetW0z0(ddSysProp[CSYSI_PROP_INPUTWSAG], ddSysProp[CSYSI_PROP_INPUTRZSAG], WLen(), MSquared(SAG));
         QSysInit[TAN].SetW0z0(ddSysProp[CSYSI_PROP_INPUTWTAN], ddSysProp[CSYSI_PROP_INPUTRZTAN], WLen(), MSquared(TAN));
         break;
      case CSYSF_INPUTWR:
         QSysInit[SAG].SetRW(ddSysProp[CSYSI_PROP_INPUTRZSAG], ddSysProp[CSYSI_PROP_INPUTWSAG], WLen(), MSquared(SAG));
         QSysInit[TAN].SetRW(ddSysProp[CSYSI_PROP_INPUTRZTAN], ddSysProp[CSYSI_PROP_INPUTWTAN], WLen(), MSquared(TAN));
         break;
      }
      SetStableABCD(TRUE, TRUE);         // always render as "stable"
      break;

   //===Spawned===========================================
   case CSYS_TYPE_SPAWN:
      // spawned systems have  their QSysInit defined
      // by the parent system's solver, see below.
      pVxTop->AbcdVx(&Mx[SAG], &Mx[TAN], TRUE);   // get output coupler matrix
      QSysInit[SAG].ApplyAbcd(&Mx[SAG]);    // propagate through output coupler
      QSysInit[TAN].ApplyAbcd(&Mx[TAN]);    // propagate through output coupler
      break;

   //===Resonator=========================================
   // Since the  same vertex angle calculations  are per-
   // formed for both sagittal and  tangential planes, we
   // retrieve both matrices at the same time
   case CSYS_TYPE_RESO:
      //---Obtain Cavity ABCD-----------------------------
      for(k=0; k<SAGTAN; k++) MxAbcdSys[k].Eye();  // initialize to identity
      //---Through system ==> --------
      pVx = pVxTop;
      while(pVx->Next()) {                  // stop at last optic
         pVx->AbcdSp(&Mx[SAG], &Mx[TAN]);   // get Nth space
         MxAbcdSys[SAG].PreMult(&Mx[SAG]);  // Nth space (not last)
         MxAbcdSys[TAN].PreMult(&Mx[TAN]);

         pVx = (CVertex*)pVx->Next();         // up through system
         pVx->AbcdVx(&Mx[SAG], &Mx[TAN], TRUE); // vertex -->--
         MxAbcdSys[SAG].PreMult(&Mx[SAG]);  // apply optic
         MxAbcdSys[TAN].PreMult(&Mx[TAN]);
      }

      //---Through system <== --------
      while(pVx->Prev()) {                    // stop at first optic
         pVx = (CVertex*) pVx->Prev();        // down through system
         pVx->AbcdSp(&Mx[SAG], &Mx[TAN]);     // get (N-1)th space
         MxAbcdSys[SAG].PreMult(&Mx[SAG]);    // (N-1)th space
         MxAbcdSys[TAN].PreMult(&Mx[TAN]);
         pVx->AbcdVx(&Mx[SAG], &Mx[TAN], FALSE); // vertex --<--
         MxAbcdSys[SAG].PreMult(&Mx[SAG]);    // (N-1)th optic
         MxAbcdSys[TAN].PreMult(&Mx[TAN]);
      }

      //---Solve ABCD-------------------------------------
      for(k=0; k<SAGTAN; k++) {
         AplusD2 = SQR(MxAbcdSys[k].A + MxAbcdSys[k].D); // calculate stability term
         if((AplusD2>4.00) || (MxAbcdSys[k].B==0.00)) {
            tfStableAbcd[k] = FALSE;         // flag unstable cavities
         } else {
            tfStableAbcd[k] = TRUE;          // for stable cavities..
            QSysInit[k].SetRealImag(         //..determine the initial 1/q parameter
               (MxAbcdSys[k].D - MxAbcdSys[k].A) / (2.00 * MxAbcdSys[k].B),
               -SQRT(4.00 - AplusD2) / (2.00 * fabs(MxAbcdSys[k].B)),
               WLen(), MSquared(k));         // wavelength remains unchanged
         }//if(stable)
      }//for(plane k)
      break;
   }//switch(SysType())

   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   // Apply
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   //===Parameters========================================
   //---Lengths-----------------------
   ddSysProp[CSYSI_PROP_PHYSLEN] = 0.00;    // prepare..
   ddSysProp[CSYSI_PROP_OPTLEN ] = 0.00;    //..for sum
   for(pVx=pVxTop; pVx->Next(); pVx=(CVertex*) pVx->Next()) {
      ddSysProp[CSYSI_PROP_PHYSLEN] += pVx->Dist2Next();

      switch(pVx->Type()) {                 // opt. length tricky, depends what RefIndex() gives
      case CVX_TYPE_PRISM1:                 // prism may have extra from insertion
      case CVX_TYPE_PRISM2:
///TODO: Prism insertion?
         ddSysProp[CSYSI_PROP_OPTLEN] += pVx->Dist2Next();
         break;
      case CVX_TYPE_OUTCOUPLER:             // output coupler -- don't know how this works yet
         ddSysProp[CSYSI_PROP_OPTLEN ] += pVx->Dist2Next() * pVx->RefIndex();
///TODO: How does output coupler work?
         break;
      default:
         ddSysProp[CSYSI_PROP_OPTLEN] += pVx->Dist2Next() * pVx->RefIndex();
         break;
      }
   }
   //---Stabilities-------------------
   ddSysProp[CSYSI_PROP_STABSAG] = MxAbcdSys[SAG].Trace() / 2.00;
   ddSysProp[CSYSI_PROP_STABTAN] = MxAbcdSys[TAN].Trace() / 2.00;
   ddSysProp[CSYSI_PROP_MODESPACING] = (ddSysProp[CSYSI_PROP_OPTLEN] == 0.00) ? 0.00 :
                                       299792.458/2.00 / ddSysProp[CSYSI_PROP_OPTLEN];

   //===Apply to Vertices=================================
   // Irrespective  of how we  got the initial Q, we  now
   // propagate it through  the system, storing it on the
   // vertices and spawned systems as necessary.
   // If the system is unstable, then  these calculations
   // are bogus, but there's no good way to perform these
   // calculations per-plane. Usually, the system will be
   // stable in any case, so it doesn't much matter.
   Q[SAG] = QSysInit[SAG];                  // start with initial Q
   Q[TAN] = QSysInit[TAN];                  // start with initial Q
   pVx = pVxTop;                            // start at first vertex
   while(pVx) {
      pVx->SetQVx(&Q[SAG], &Q[TAN]);        // apply Q to current Vx
      if(pVx->Next()==NULL) break;          // finish after last optic
      pVx->AbcdSp(&Mx[SAG], &Mx[TAN]);      // get space
      Q[SAG].ApplyAbcd(&Mx[SAG]);           // propagate to next optic
      Q[TAN].ApplyAbcd(&Mx[TAN]);           // propagate to next optic
      pVx = (CVertex*) pVx->Next();         // move to next optic
      if(pVx->SysSpawned()) {               // spawned systems: Store 1/q BEFORE optic
         pVx->SysSpawned()->SetQSysInit(&Q[SAG], &Q[TAN]);
         pVx->SysSpawned()->SetStableABCD(tfStableAbcd[SAG], tfStableAbcd[TAN]);
      }
      pVx->AbcdVx(&Mx[SAG], &Mx[TAN], TRUE); // get optic matrix -->--
      Q[SAG].ApplyAbcd(&Mx[SAG]);           // apply optic
      Q[TAN].ApplyAbcd(&Mx[TAN]);           // apply optic
   } //(vx loop)

   //===Spawned Systems===================================
   for(pVx=pVxTop; pVx; pVx=(CVertex*) pVx->Next()) {
      if(pVx->SysSpawned()) pVx->SysSpawned()->SolveSystemABCD();
   }
}



/*********************************************************
* SetQSysInit
* Sets the  initial 1/q parameter for (spawned) systems.
* Called from
*   <-- CSystem::SolveSystemABCD
*********************************************************/
void CSystem::SetQSysInit(const CRecQ *pcQSag, const CRecQ *pcQTan) {
   CRecQ *pQ;                               // suppress "Non-const function CRecQ::GetR() called for const object"
   if(pcQSag) { pQ = (CRecQ*) pcQSag; QSysInit[SAG].Set(pQ->Get_R(), pQ->Get_v(), pQ->WLen(), pQ->M2()); };
   if(pcQTan) { pQ = (CRecQ*) pcQTan; QSysInit[TAN].Set(pQ->Get_R(), pQ->Get_v(), pQ->WLen(), pQ->M2()); };
}





/*########################################################
 ## Renderers                                          ##
########################################################*/
/*********************************************************
* User Create Functions
* The system can include only  one of each of the follow-
* ing renderers:
*  - 2D Editor
*  - Inventory
*  - Solver (? not necessarily?)
* If the requested type  already exists in chain, the ex-
* isting object is returned.
* Called from:
*  <- CApplication::ProcessCommand
*  <- Loading system
*********************************************************/
//===1D===================================================
CSysWin* CSystem::UserCreateSysWin1d(void) {
   CSysWin *pSWNew;                         // new renderer
   pSWNew = new CSysWin1d(this);            // create new 1d renderer
   AppendSysWin(pSWNew);                    // append to renderer chain
   return(pSWNew);                          // return new object
}

//===2D===================================================
CSysWin* CSystem::UserCreateSysWin2d(void) {
   CSysWin *pSWNew;                         // new or existing renderer
   pSWNew = pSysWinTop;                     // examine chain starting at top
   while(pSWNew != NULL) {
      if(pSWNew->Type() == CSYSWINI_TYPE_2D) return(pSWNew); // found existing, return that
      pSWNew = pSWNew->Next();              // follow down chain
   }
   pSWNew = new CSysWin2d(this);            // none found, so create new
   AppendSysWin(pSWNew);                    // append to renderer chain
   return(pSWNew);                          // return new object
}

//===3D===================================================
CSysWin* CSystem::UserCreateSysWin3d(void) {
   CSysWin *pSWin;                          // new or existing renderer
   pSWin = pSysWinTop;                      // examine chan starting at top
   while(pSWin!=NULL) {
      if(pSWin->Type() == CSYSWINI_TYPE_3D) return(pSWin);
      pSWin = pSWin->Next();                // follow down chain
   }
   pSWin = new CSysWin3d(this);             // none found, so create new
   AppendSysWin(pSWin);                     // append renderer to chain
   return(pSWin);                           // return new object
}

//===System Graph=========================================
CSysWin* CSystem::UserCreateSysWinGraph(void) {
   CSysWin *pSWNew;                         // new renderer
   pSWNew = new CSysWinGraph(this);         // create new renderer
   AppendSysWin(pSWNew);                    // append to renderer chain
   return(pSWNew);                          // return new object
}

//===VxGraph==============================================
CSysWin* CSystem::UserCreateSysWinVxGraph(BOOL tfSel) {
   CVertex *pVx;                            // attach to selected vertices
   CSysWin *pSWNew;                         // new renderer
   //---Attach to selection---------------------
   if(tfSel) {
      for(pVx=pVxTop; pVx; pVx=(CVertex*)pVx->Next()) { // loop over vertices
         if(pVx->Selected()) {                 // attach to any and all that are selected
            pSWNew = new CSysWinVxGraph(this, pVx); // create new vertex graph renderer
            AppendSysWin(pSWNew);              // append to renderer chain
         }
      }
   //---Arbitrary (loading files)---------------
   } else {
      pSWNew = new CSysWinVxGraph(this, pVxTop); // create new vertex graph renderer
      AppendSysWin(pSWNew);              // append to renderer chain
   }
   return(pSWNew);                          // return (last) new object
}

//===Inventory============================================
// Allow only one, only at top-level
CSysWin* CSystem::UserCreateSysWinInventory(void) {
   CSysWin *pSWNew;                         // new or existing renderer
   if(pVxSpawn) return(pVxSpawn->SysParent()->UserCreateSysWinInventory()); // direct up to top level system

   for(pSWNew=pSysWinTop; pSWNew; pSWNew = pSWNew->Next()) { // look through renderer chain
      if(pSWNew->Type() == CSYSWINI_TYPE_INVENTORY) return(pSWNew); // found existing, return that
   }
   pSWNew = new CSysWinInventory(this);     // none found, so create new
   AppendSysWin(pSWNew);                    // append to renderer chain
   return(pSWNew);                          // return new object
}
//===Solver===============================================
// Allow only one per system (top or spawned)
CSysWin* CSystem::UserCreateSysWinABCDSolver(void) {
   CSysWin *pSWNew;                         // new or existing renderer
   for(pSWNew=pSysWinTop; pSWNew; pSWNew = pSWNew->Next()) { // look through renderer chain
      if(pSWNew->Type() == CSYSWINI_TYPE_ABCDSOLVER) return(pSWNew); // found existing, return that
   }
   pSWNew = new CSysWinABCDSolver(this);    // none found, so create new
   AppendSysWin(pSWNew);                    // append to renderer chain
   return(pSWNew);                          // return new object
}

/*********************************************************
* User Delete SysWin
* It will probably be desirable to unload the system when
* the last 2D renderer is destroyed.
*********************************************************/
void CSystem::UserDeleteSysWin(CSysWin *pSWDel) {
///TODO: Check for 2D renderer and delete the whole system when it's closed
   if(pSWDel->Type() == CSYSWINI_TYPE_2D) {
      MessageBox(NULL, "WARNING / TODO: CANNOT remove 2D renderer?!","CSystem@260", MB_OK);
      return;
   }
   DeleteSysWin(pSWDel);
}


/*********************************************************
* AppendSysWin                           <-> DeleteSysWin
* Add a (newly created) CSysWin  object to the end of the
* renderer chain. We need to keep track of the Top here.
*********************************************************/
void CSystem::AppendSysWin(CSysWin *pSWApp) {
   CSysWin *pSWin;                          // loop pointer

   if(pSWApp == NULL) return;               // ignore if no object supplied
   if(pSysWinTop == NULL) {                 // no chain yet..
      pSysWinTop = pSWApp;                  //..so point to new object
   } else {                                 // find end of existing chain
      pSWin = pSysWinTop;                   // start at top and..
      while(pSWin->Next()) pSWin = pSWin->Next(); //..follow down chain..
      pSWin->SetNext(pSWApp);               //..and append to last
   }
   GetTopSystem()->UpdateAllRendererTitle(); // update cardinal counts on windows
}


/*********************************************************
* DeleteSysWin                           <-> AppendSysWin
* This is the sister function to AppendSysWin; it removes
* the specified renderer and  maintains the chain -- then
* deletes the specified renderer
*********************************************************/
void CSystem::DeleteSysWin(CSysWin *pSWDel) {
   CSysWin *pSWin;                          // loop pointer

   if(pSWDel == NULL) return;               // ignore if nothing to delete
   if(pSysWinTop == pSWDel) {               // remove from start of chain
      pSysWinTop = pSWDel->Next();          // point to next in list
   } else {                                 // otherwise find in chain
      pSWin = pSysWinTop;                   // start at top of chain..
      while((pSWin!=NULL) && (pSWin->Next()!=pSWDel))
         pSWin = pSWin->Next();             //..follow down chain
      if(pSWin==NULL) { printf("? CSystem::RemoveSysWin@239 SysWin 0x%08lx not found in renderer chain for system <%s>\n", pSWDel, Tag()); return; }
      pSWin->SetNext(pSWDel->Next());       // maintain chain
   }
   delete(pSWDel);                          // delete the object
   GetTopSystem()->UpdateAllRendererTitle(); // update cardinal counts on windows
}


/*********************************************************
* CreateSysWinWindow
* Create an MDI child window through the Application. The
* function is called  by SysWin renderers  as an indirect
* passage to the  CApplication object. To  set the title,
* call the renderer's  UpdateTitle()  function after cre-
* ating the window.
*********************************************************/
HWND CSystem::CreateSysWinWindow(const char *pszClassName, LPVOID lpVoid, UINT uStyle) {
   if(pAppParent==NULL) return(NULL);       // ignore if no parent
   return(pAppParent->CreateSysWinWindow(pszClassName, lpVoid, uStyle));
}

/*********************************************************
* DestroySysWinWindow
* Call the CApplication to  destroy the MDI child window.
* This function is probably mostly called by renderers as
* an indirect passage to the CApplication object.
*********************************************************/
void CSystem::DestroySysWinWindow(HWND hWnd) {
   if(pAppParent==NULL) return;             // ignore if no parent
   pAppParent->DestroySysWinWindow(hWnd);   // destroy the window
}


/*********************************************************
* PrintAllRenderer
* Calls Print() on all renderers of this system (at least
* all that have a viable Print() function), then recursi-
* vely calls PrintAllRenderer() on all spawned systems.
* Called from
* <- App::MenuPrint
*********************************************************/
void CSystem::PrintAllRenderer(HDC hdcPrint, int *piPage) {
   CSysWin *pSWin;                          // renderer loop counter
   CVertex *pVx;                            // vertex loop counter

   //---Renderers-------------------------------
   for(pSWin=pSysWinTop; pSWin; pSWin=pSWin->Next()) {
      switch(pSWin->Type()) {
      case CSYSWINI_TYPE_1D:
      case CSYSWINI_TYPE_2D:
      case CSYSWINI_TYPE_3D:
      case CSYSWINI_TYPE_GRAPH:
      case CSYSWINI_TYPE_VXGRAPH:
      case CSYSWINI_TYPE_INVENTORY:
         pSWin->Print(hdcPrint);            // print contents
         App()->PrintHeaderFooter(hdcPrint, FileName(), piPage); // print header and footer
         EndPage(hdcPrint);                 // completed this page
         if(piPage) *piPage += 1;           // count printed pages
         break;
      }
  }

   //---Recursion-------------------------------
   for(pVx=pVxTop; pVx; pVx=(CVertex*) pVx->Next()) {
      if(pVx->SysSpawned()) pVx->SysSpawned()->PrintAllRenderer(hdcPrint, piPage);
   }
}


/*********************************************************
* UserSetTag
* Sets the system tag and updates all the existing ren-
* derers' window titles.
*********************************************************/
void CSystem::UserSetTag(const char *psz) {
   strncpy(szSysTag, psz, CSYSC_MAXTAG);    // copy the tag
   UpdateAllRendererTitle();                // legacy: Used to be tag in title bar
}

/*********************************************************
* UpdateAllRendererTitle
* In order to be able to locate  the windows from the MDI
* window menu, the windows must all have unique names. In
* fact they should probably have that anyway, so that the
* user can find them. The strategy is to count the number
* of each type of renderer, and give each renderer window
* a unique identifier  if there's more  than one of them.
* What's worse: We need to do this for the TOP system but
* update ALL CHILD RENDERERs too. Thank goodness for STA-
* TIC and recursion!
* Called from
* <- UserSetTag (legacy)
* <- CSystem::CreateSysWin
* <- App::UserSaveSystem, in case there's a new name
*********************************************************/
void CSystem::UpdateAllRendererTitle(void) {
   static int iNum[CSYSWINI_MAXTYPE+1];     // number of each renderer type
   static int iCnt[CSYSWINI_MAXTYPE+1];     // intex of this renderer
   int k;                                   // loop counter
   CSysWin *pSWin;                          // renderer loop counter
   CVertex *pVx;                            // vertex loop counter

   //===Top Level===============================
   if(pVxSpawn==NULL) {
      //---Count renderers------------
      for(k=0; k<CSYSWINI_MAXTYPE+1; k++) { iNum[k] = 0; iCnt[k] = -1; } // set flag to count for now
      for(pSWin=pSysWinTop; pSWin; pSWin=(CSysWin*) pSWin->Next()) iNum[pSWin->Type()] += 1; // count numbers of each renderer
      for(pVx=pVxTop; pVx; pVx=(CVertex*)pVx->Next()) { if(pVx->SysSpawned()) pVx->SysSpawned()->UpdateAllRendererTitle(); } // count children's

      //---Set titles-----------------
      for(k=0; k<CSYSWINI_MAXTYPE+1; k++) iCnt[k] = 1; // reset count
      for(pSWin=pSysWinTop; pSWin; pSWin=(CSysWin*) pSWin->Next()) {
         if(iNum[pSWin->Type()] < 2) pSWin->UpdateTitle( -1 ); // update, no identifier
         else pSWin->UpdateTitle(iCnt[pSWin->Type()]++); // set number, increment
      }

   //===Child===================================
   } else {
      //---Count renderers------------
      if(iCnt[0] < 0) {
         for(pSWin=pSysWinTop; pSWin; pSWin=(CSysWin*) pSWin->Next())
            iNum[pSWin->Type()] += 1;             // count numbers of each renderer
         for(pVx=pVxTop; pVx; pVx=(CVertex*)pVx->Next())
            if(pVx->SysSpawned()) pVx->SysSpawned()->UpdateAllRendererTitle(); // count children's
      //---Set titles-----------------
      } else {
         for(pSWin=pSysWinTop; pSWin; pSWin=(CSysWin*) pSWin->Next()) {
            if(iNum[pSWin->Type()] < 2) pSWin->UpdateTitle( -1 ); // update, no identifier
            else pSWin->UpdateTitle(iCnt[pSWin->Type()]++); // set number, increment
         }
      }
   }

   //===Recurse Children==================================
   for(pVx=pVxTop; pVx; pVx=(CVertex*)pVx->Next()) { if(pVx->SysSpawned()) pVx->SysSpawned()->UpdateAllRendererTitle(); } // count children's
}

/*********************************************************
* UserCloseSystem
* Called when user wants to close the system, e.g. by re-
* questing to close the 2d renderer window.
* For spawned systems, a dialog  is displayed to query if
* the spawn should really be  removed. For top level sys-
* tems, the  system can just be deleted, with  all of its
* dependents.
* Returns TRUE if the system can be deleted.
*********************************************************/
BOOL CSystem::UserCloseSystem(void) {
   char szBuf[256];                         // string from rc
   //===Spawned===========================================
   if(pVxSpawn != NULL) {
      LoadString(App()->GetInstance(), SZMSG_DELSPAWN, szBuf, sizeof(szBuf)/sizeof(char));
      return(
         (MessageBox(pAppParent->GetWindow(),
            szBuf, CSZ_CAPTION_APPLICATION, MB_OKCANCEL | MB_ICONQUESTION)
         == IDOK) ? TRUE : FALSE);

   //===Top===============================================
   } else {
      return(TRUE);
   }
}

/*########################################################
 ## Properties                                         ##
########################################################*/
/*********************************************************
* PrepareProperties
* This is called AUTOMATICALLY by the renderer
* <- renderer window activation WM_MDIACTIVATE
*********************************************************/
const UINT CSystem::CuSysPropertiesProp[] = {
   CPS_SYSHEADER     , // "System %s"
   //CPS_SYSTAG        , // "Name"
   CPS_SYSWAVELEN    , // "Wavelength (nm)"
   CPS_SYSMSQUARED   , // "Quality M²"
   CPS_SYSMSQUAREDSAG, // "Beam M² (sag)"
   CPS_SYSMSQUAREDTAN, // "Beam M² (tan)"
   CPS_SYSMSQASYM    , // "Beam M2 Asymmetric"
   CPS_SYSROTATION   , // "Rotation (deg)"
   CPS_SYSSTARTX     , // "Start X (mm)"
   CPS_SYSSTARTY     , // "Start Y (mm)"
   CPS_SYSPHYSLEN    , // "Physical Length (mm)"
   CPS_SYSOPTLEN     , // "Optical Length (mm)"
   CPS_SYSINITDATA   , // "Input Parameters"
   0};
const UINT CSystem::CuSysPropertiesPropInitw0z0[] = {
   CPS_SYSISAGWAIST  , // "Sag. Waist (um)"
   CPS_SYSISAGDSTW   , // "Sag. Waist Dist. (mm)"
   CPS_SYSITANWAIST  , // "Tan. Waist (um)"
   CPS_SYSITANDSTW   , // "Tan. Waist Dist. (mm)"
   0};
const UINT CSystem::CuSysPropertiesPropInitwR[] = {
   CPS_SYSISAGSIZE   , // "Sag. Spot Size (um)"
   CPS_SYSISAGCURV   , // "Sag. Curvature (mm)"
   CPS_SYSITANSIZE   , // "Tan. Spot Size (um)"
   CPS_SYSITANCURV   , // "Tan. Curvature (mm)"
   0};

const UINT CSystem::CuSysPropertiesSpawn[] = {
   CPS_SYSHEADER     , // "System %s"
   //CPS_SYSTAG        , // "Name"
   CPS_SYSWAVELEN    , // "Wavelength (nm)"
   CPS_SYSMSQUARED   , // "Quality M²"
   CPS_SYSMSQUAREDSAG, // "Beam M² (sag)"
   CPS_SYSMSQUAREDTAN, // "Beam M² (tan)"
   CPS_SYSMSQASYM    , // "Beam M2 Asymmetric"
   CPS_SYSROTATION   , // "Rotation (deg)"
   CPS_SYSSTARTX     , // "Start X (mm)"
   CPS_SYSSTARTY     , // "Start Y (mm)"
   CPS_SYSPHYSLEN    , // "Physical Length (mm)"
   CPS_SYSOPTLEN     , // "Optical Length (mm)"
   0};

const UINT CSystem::CuSysPropertiesReso[] = {
   CPS_SYSHEADER     , // "System %s"
   //CPS_SYSTAG        , // "Name"
   CPS_SYSWAVELEN    , // "Wavelength (nm)"
   CPS_SYSMSQUARED   , // "Quality M²"
   CPS_SYSMSQUAREDSAG, // "Beam M² (sag)"
   CPS_SYSMSQUAREDTAN, // "Beam M² (tan)"
   CPS_SYSMSQASYM    , // "Beam M2 Asymmetric"
   CPS_SYSROTATION   , // "Rotation (deg)"
   CPS_SYSSTARTX     , // "Start X (mm)"
   CPS_SYSSTARTY     , // "Start Y (mm)"
   CPS_SYSPHYSLEN    , // "Physical Length (mm)"
   CPS_SYSOPTLEN     , // "Optical Length (mm)"
   CPS_SYSMODESPACE  , // "Mode Spacing (MHz)"
   CPS_SYSABCDSAG    , // "ABCD (Sagittal)"
   CPS_SYSABCDTAN    , // "ABCD (Tangential)"
   CPS_SYSSTABILITY  , // "Stability"
   0};

const UINT CSystem::CuSysPropDraftHide[] = {
   CPS_SYSROTATION   , // "Rotation (deg)"
   CPS_SYSSTARTX     , // "Start X (mm)"
   CPS_SYSSTARTY     , // "Start Y (mm)"
   CPS_SYSPHYSLEN    , // "Physical Length (mm)"
   CPS_SYSOPTLEN     , // "Optical Length (mm)"
   CPS_SYSMODESPACE  , // "Mode Spacing (MHz)"
   CPS_SYSABCDSAG    , // "ABCD (Sagittal)"
   CPS_SYSABCDTAN    , // "ABCD (Tangential)"
   CPS_SYSSTABILITY  , // "Stability"
   0};
//========================================================
void CSystem::PrepareProperties(BOOL tfAct) {
   char szBuf[256];
   CSystem  *pSys = this;                   // top-level system for wavelength and M^2
   CPropMgr *pMgr;                          // assign to make source readable
   //---Show / hide-----------------------------
   switch(iSysType) {
   case CSYS_TYPE_PROP:
      LoadString(App()->GetInstance(), CPS_SYSHEADPROP, szBuf, sizeof(szBuf)/sizeof(char));
      if(tfAct) App()->PropManager()->FindItemByID(CPS_SYSHEADER)->SetItemHeading(szBuf);
      App()->PrepareProperties(CuSysPropertiesProp, tfAct, CSystem::_SysPropItemCallback, this);
      App()->PrepareProperties(CuSysPropertiesPropInitw0z0, FALSE, NULL, NULL); // disable all input types
      App()->PrepareProperties(CuSysPropertiesPropInitwR,   FALSE, NULL, NULL);
      switch(CheckBit(CSYSF_INPUTPARAM)) { // then select the visible ones
      case CSYSF_INPUTW0Z0:
         App()->PrepareProperties(CuSysPropertiesPropInitw0z0, tfAct, CSystem::_SysPropItemCallback, this);
         break;
      case CSYSF_INPUTWR:
         App()->PrepareProperties(CuSysPropertiesPropInitwR, tfAct, CSystem::_SysPropItemCallback, this);
         break;
      }
      break;
   case CSYS_TYPE_SPAWN:
      LoadString(App()->GetInstance(), CPS_SYSHEADSPWN, szBuf, sizeof(szBuf)/sizeof(char));
      if(tfAct) App()->PropManager()->FindItemByID(CPS_SYSHEADER)->SetItemHeading(szBuf);
      App()->PrepareProperties(CuSysPropertiesSpawn, tfAct, CSystem::_SysPropItemCallback, this);
      break;
   case CSYS_TYPE_RESO:
      LoadString(App()->GetInstance(), CPS_SYSHEADRESO, szBuf, sizeof(szBuf)/sizeof(char));
      if(tfAct) App()->PropManager()->FindItemByID(CPS_SYSHEADER)->SetItemHeading(szBuf);
      App()->PrepareProperties(CuSysPropertiesReso, tfAct, CSystem::_SysPropItemCallback, this);
      break;
   }

   //---MSquared Asymmetric---------------------
   pMgr = App()->PropManager();    // assign separately for below
   if((tfAct) && (pMgr)) {
      if(MSqAsymmetric()) {
         pMgr->FindItemByID( CPS_SYSMSQUARED    )->SetBit(CPIF_HIDDEN);
         pMgr->FindItemByID( CPS_SYSMSQUAREDSAG )->ClearBit(CPIF_HIDDEN);
         pMgr->FindItemByID( CPS_SYSMSQUAREDTAN )->ClearBit(CPIF_HIDDEN);
      } else {
         pMgr->FindItemByID( CPS_SYSMSQUARED    )->ClearBit(CPIF_HIDDEN);
         pMgr->FindItemByID( CPS_SYSMSQUAREDSAG )->SetBit(CPIF_HIDDEN);
         pMgr->FindItemByID( CPS_SYSMSQUAREDTAN )->SetBit(CPIF_HIDDEN);
      }
   }

   //---Set Lists-------------------------------
   if((tfAct) && (pMgr!=NULL)) {
      pMgr->FindItemByID( CPS_SYSTAG        )->SetItemText(Tag());
      pMgr->FindItemByID( CPS_SYSINITDATA   )->SetItemRadioList(InputParam(), CSystem::CszSysInputParam);
   }

   //===Callbacks=========================================
   // Default callbacks  are set by  CApp::PrepareProper-
   // ties(), so  we only manage the special cases  here.
   // Also, when de-activating, the callbacks are automa-
   // tically removed.
   //---Set Top System Equations----------------
   if((tfAct) && (pMgr)) {                  // set to top-level when activating
      for(pSys=this; pSys->VxSpawn(); pSys = pSys->VxSpawn()->SysParent()); // go to top level
      pMgr->FindItemByID( CPS_SYSWAVELEN    )->SetItemEquation(pSys->SysEquation( CSYSI_EQTN_WAVLEN      ));
      pMgr->FindItemByID( CPS_SYSMSQUARED   )->SetItemEquation(pSys->SysEquation( CSYSI_EQTN_MSQUARED    ));
      pMgr->FindItemByID( CPS_SYSMSQUAREDSAG)->SetItemEquation(pSys->SysEquation( CSYSI_EQTN_MSQUARED    ));
      pMgr->FindItemByID( CPS_SYSMSQUAREDTAN)->SetItemEquation(pSys->SysEquation( CSYSI_EQTN_MSQUAREDTAN ));
      pMgr->FindItemByID( CPS_SYSROTATION   )->SetItemEquation(SysEquation( CSYSI_EQTN_ROTATION    ));
      pMgr->FindItemByID( CPS_SYSSTARTX     )->SetItemEquation(SysEquation( CSYSI_EQTN_STARTX      ));
      pMgr->FindItemByID( CPS_SYSSTARTY     )->SetItemEquation(SysEquation( CSYSI_EQTN_STARTY      ));
      pMgr->FindItemByID( CPS_SYSWAVELEN    )->SetItemCallback(CSystem::_SysPropItemCallback, pSys);
      pMgr->FindItemByID( CPS_SYSMSQUARED   )->SetItemCallback(CSystem::_SysPropItemCallback, pSys);
      pMgr->FindItemByID( CPS_SYSMSQUAREDSAG)->SetItemCallback(CSystem::_SysPropItemCallback, pSys);
      pMgr->FindItemByID( CPS_SYSMSQUAREDTAN)->SetItemCallback(CSystem::_SysPropItemCallback, pSys);
      pMgr->FindItemByID( CPS_SYSROTATION   )->SetItemCallback(CSystem::_SysPropItemCallback, this);
      pMgr->FindItemByID( CPS_SYSSTARTX     )->SetItemCallback(CSystem::_SysPropItemCallback, this);
      pMgr->FindItemByID( CPS_SYSSTARTY     )->SetItemCallback(CSystem::_SysPropItemCallback, this);
   }

   //---Input parameters------------------------
   if(iSysType==CSYS_TYPE_PROP) {
      switch(InputParam()) {
      case CSYSF_INPUTW0Z0:
         pMgr->FindItemByID( CPS_SYSISAGWAIST )->SetItemEquation(pSys->SysEquation( CSYSI_EQTN_INPUTWSAG ));
         pMgr->FindItemByID( CPS_SYSISAGDSTW  )->SetItemEquation(pSys->SysEquation( CSYSI_EQTN_INPUTRZSAG ));
         pMgr->FindItemByID( CPS_SYSITANWAIST )->SetItemEquation(pSys->SysEquation( CSYSI_EQTN_INPUTWTAN ));
         pMgr->FindItemByID( CPS_SYSITANDSTW  )->SetItemEquation(pSys->SysEquation( CSYSI_EQTN_INPUTRZTAN ));
         pMgr->FindItemByID( CPS_SYSISAGWAIST )->SetItemCallback(CSystem::_SysPropItemCallback, this);
         pMgr->FindItemByID( CPS_SYSISAGDSTW  )->SetItemCallback(CSystem::_SysPropItemCallback, this);
         pMgr->FindItemByID( CPS_SYSITANWAIST )->SetItemCallback(CSystem::_SysPropItemCallback, this);
         pMgr->FindItemByID( CPS_SYSITANDSTW  )->SetItemCallback(CSystem::_SysPropItemCallback, this);
         break;
      case CSYSF_INPUTWR:
         pMgr->FindItemByID( CPS_SYSISAGSIZE  )->SetItemEquation(pSys->SysEquation( CSYSI_EQTN_INPUTWSAG ));
         pMgr->FindItemByID( CPS_SYSISAGCURV  )->SetItemEquation(pSys->SysEquation( CSYSI_EQTN_INPUTRZSAG ));
         pMgr->FindItemByID( CPS_SYSITANSIZE  )->SetItemEquation(pSys->SysEquation( CSYSI_EQTN_INPUTWTAN ));
         pMgr->FindItemByID( CPS_SYSITANCURV  )->SetItemEquation(pSys->SysEquation( CSYSI_EQTN_INPUTRZTAN ));
         pMgr->FindItemByID( CPS_SYSISAGSIZE  )->SetItemCallback(CSystem::_SysPropItemCallback, this);
         pMgr->FindItemByID( CPS_SYSISAGCURV  )->SetItemCallback(CSystem::_SysPropItemCallback, this);
         pMgr->FindItemByID( CPS_SYSITANSIZE  )->SetItemCallback(CSystem::_SysPropItemCallback, this);
         pMgr->FindItemByID( CPS_SYSITANCURV  )->SetItemCallback(CSystem::_SysPropItemCallback, this);
         break;
      }
   }

   //---Hide in draft mode----------------------
   if((tfAct) && (pMgr!=NULL) && DraftMode()) {
      App()->PrepareProperties(CuSysPropDraftHide, FALSE, NULL, this);
   }

   //===Values============================================
   // Since this  function may come from a  window change
   // that does not require a re-calculation we also call
   // Update from here
   if(tfAct) { pMgr->OnResize(); }//pMgr->OnPaint(FALSE); }
   UpdateProperties();                      // update the values for this system
}


/*********************************************************
* UpdateProperties
* We must update anything and  everything that can depend
* upon variables, so everything except for the relatively
* constant  things like name, which we will know  when to
* update, and equations, which  are automatically updated
* when the property manager re-paints.
* This is NOT called from the renderers' UpdateProperties
* because not all renderer changes need the CSystem to be
* refreshed.
* Called from
* <- PrepareProperties
* <- CApp::ScanAll
*********************************************************/
void CSystem::UpdateProperties(void) {
   CPropMgr *pMgr;                          // assign to make code smaller
   pMgr = App()->PropManager();             // read manager object from application
   if(pMgr == NULL) return;                 // don't write if no manager

   //---Common----------------------------------
   pMgr->FindItemByID( CPS_SYSPHYSLEN    )->SetItemValue(ddSysProp[CSYSI_PROP_PHYSLEN]);  // "Physical Length (mm)"
   pMgr->FindItemByID( CPS_SYSOPTLEN     )->SetItemValue(ddSysProp[CSYSI_PROP_OPTLEN ]);  // "Optical Length (mm)"
   pMgr->FindItemByID( CPS_SYSABCDSAG    )->SetItemQuadValue(                             // "ABCD (Sagittal)"
      MxAbcdSys[SAG].A, MxAbcdSys[SAG].B, MxAbcdSys[SAG].C, MxAbcdSys[SAG].D);
   pMgr->FindItemByID( CPS_SYSABCDTAN    )->SetItemQuadValue(                            // "ABCD (Tangential)"
      MxAbcdSys[TAN].A, MxAbcdSys[TAN].B, MxAbcdSys[TAN].C, MxAbcdSys[TAN].D);
   pMgr->FindItemByID( CPS_SYSMSQASYM    )->SetItemCheckBox(MSqAsymmetric() ? TRUE : FALSE); // Beam Asymmetric

   switch(iSysType) {
   //---Propagation-----------------------------
   case CSYS_TYPE_PROP:
      pMgr->FindItemByID( CPS_SYSINITDATA   )->SetItemRadioList(InputParam(), CSystem::CszSysInputParam);
      break;

   //---Spawned---------------------------------
   case CSYS_TYPE_SPAWN:
      break;

   //---Resonator-------------------------------
   case CSYS_TYPE_RESO:
      pMgr->FindItemByID( CPS_SYSMODESPACE  )->SetItemValue(ddSysProp[CSYSI_PROP_MODESPACING]);
      pMgr->FindItemByID( CPS_SYSSTABILITY  )->SetItemSagTanValue(ddSysProp[CSYSI_PROP_STABSAG], ddSysProp[CSYSI_PROP_STABTAN]);
      break;
   }

}


/*********************************************************
* SysPropMgrCallback
* Called back from the property manager when a value has
* been changed. Return FALSE to keep the item visible.
* Notes
*  - Equation: The equations have already been tested to
*    parse using the application variables. vData is the
*    new equation string.
* The callback function must be declared static.
*********************************************************/
BOOL CSystem::_SysPropItemCallback(void *vData, UINT uID, void *pVoid) {
   return( (pVoid) ? ((CSystem*)pVoid)->SysPropItemCallback(vData, uID) : TRUE );
}
//******************************************************//
BOOL CSystem::SysPropItemCallback(void *vData, UINT uID) {
   CSystem *pSys;                           // scan to top system
   CPropMgr *pMgr = App()->PropManager();   // assign to make code readable
   //---Process value---------------------------
   switch(uID) {
   case CPS_SYSTAG:
      UserSetTag((const char*)vData);
      pMgr->FindItemByID(CPS_SYSTAG)->SetItemText(Tag());
      pMgr->OnPaint(TRUE);
      return(TRUE);
   case CPS_SYSWAVELEN:      EqSys[CSYSI_EQTN_WAVLEN     ].ParseEquation((const char*)vData, App()->VarsString()); break;
   case CPS_SYSMSQUARED: // this only called if M^2 symmetric, so apply to both!
      EqSys[CSYSI_EQTN_MSQUARED   ].ParseEquation((const char*)vData, App()->VarsString());
      EqSys[CSYSI_EQTN_MSQUAREDTAN].ParseEquation((const char*)vData, App()->VarsString());
      break;
   case CPS_SYSMSQUAREDSAG:  EqSys[CSYSI_EQTN_MSQUARED   ].ParseEquation((const char*)vData, App()->VarsString()); break;
   case CPS_SYSMSQUAREDTAN:  EqSys[CSYSI_EQTN_MSQUAREDTAN].ParseEquation((const char*)vData, App()->VarsString()); break;
   case CPS_SYSMSQASYM:      SetMSqAsymmetric((*(BOOL*)vData) ? FALSE : TRUE); PrepareProperties(TRUE); break;
   case CPS_SYSROTATION:     EqSys[CSYSI_EQTN_ROTATION   ].ParseEquation((const char*)vData, App()->VarsString()); break;
   case CPS_SYSSTARTX:       EqSys[CSYSI_EQTN_STARTX     ].ParseEquation((const char*)vData, App()->VarsString()); break;
   case CPS_SYSSTARTY:       EqSys[CSYSI_EQTN_STARTY     ].ParseEquation((const char*)vData, App()->VarsString()); break;
   case CPS_SYSISAGWAIST:
   case CPS_SYSISAGSIZE :    EqSys[CSYSI_EQTN_INPUTWSAG  ].ParseEquation((const char*)vData, App()->VarsString()); break;
   case CPS_SYSISAGDSTW :
   case CPS_SYSISAGCURV :    EqSys[CSYSI_EQTN_INPUTRZSAG ].ParseEquation((const char*)vData, App()->VarsString()); break;
   case CPS_SYSITANWAIST:
   case CPS_SYSITANSIZE :    EqSys[CSYSI_EQTN_INPUTWTAN  ].ParseEquation((const char*)vData, App()->VarsString()); break;
   case CPS_SYSITANDSTW :
   case CPS_SYSITANCURV :    EqSys[CSYSI_EQTN_INPUTRZTAN ].ParseEquation((const char*)vData, App()->VarsString()); break;
   case CPS_SYSINITDATA :    UserSetInputParam(*(int*)vData); break;
   default:
      return(TRUE);
   }

   //---Refresh all-----------------------------
   // In practice, we may  not have to  scan ALL systems,
   // but this is much easier to implement
   App()->ScanAll();
   return(TRUE);
}



/*########################################################
 ## Vertex Properties                                  ##
########################################################*/
/*********************************************************
* Prepare VxProperties
* This should  also be called whenever the  system selec-
* tion  changes. A distinction is  made depending on  the
* number of  selected vertices. For multiple  selections,
* no values are displayed (it's a  bit complicated to fi-
* gure out) but values entered will be applied to all se-
* lected vertices, so only those properties common to all
* selected vertex types will be displayed.
* The implementation is a little  different to other Pre-
* pareProperties, because of how it's used.
* Called from:
* <- PrepareProperties / renderer WM_MDIACTIVATE
* <- system selection change
*********************************************************/
const UINT CSystem::CuVxProperties[] = {
   CPS_VXHEADER      , // "%s (Optic)"
   CPS_VXTAG         , // "Tag"
   CPS_VXRADCURV     , // "Radius Curvature (mm)"
   CPS_VXRADCURVSAG  , // "Rad. Curvature (sag) (mm)"
   CPS_VXRADCURVTAN  , // "Rad. Curvature (tan) (mm)"
   CPS_VXFOCALLEN    , // "Focal Length (mm)"
   CPS_VXFOCALLENSAG , // "Focal Length (sag) (mm)"
   CPS_VXFOCALLENTAN , // "Focal Length (tan) (mm)"
   CPS_VXROCFLASTIG  , // "Astigmatic"
   CPS_VXFACEANGL    , // "Face Angle (deg)"
   CPS_VXLOCKFACEANGL, // "Locked Angle"
   CPS_VXBLOCKREFINDX, // "Refractive Index"
   CPS_VXTHICKNESS   , // "Thickness (mm)"
   CPS_VXFLIPDIR     , // "Direction"
   CPS_VXDISTNEXT    , // "Distance to Next (mm)"
   CPS_VXSPOTSIZE    , // "Spot Size (um)"
   CPS_VXCURVATURE   , // "Curvature (mm)"
   CPS_VXABCDSAG     , // "ABCD (Sagittal)"
   CPS_VXABCDTAN     , // "ABCD (Tangential)"
   CPS_VXASTIG       , // "Astigmatism (mm)"
   0};
const UINT CSystem::CuVxPropDraftHide[] = {
   CPS_VXFACEANGL    , // "Face Angle (deg)"
   CPS_VXLOCKFACEANGL, // "Locked Angle"
   CPS_VXDISTNEXT    , // "Distance to Next (mm)"
   CPS_VXSPOTSIZE    , // "Spot Size (um)"
   CPS_VXCURVATURE   , // "Curvature (mm)"
   CPS_VXABCDSAG     , // "ABCD (Sagittal)"
   CPS_VXABCDTAN     , // "ABCD (Tangential)"
   CPS_VXASTIG       , // "Astigmatism (mm)"
   0};
//******************************************************//
void CSystem::PrepareVxProperties(BOOL tfAct) {
   CPropMgr *pMgr = App()->PropManager();   // assign for code below
   CVertex  *pVx;                           // vertex loop counter
   int       iNSel;                         // number of selected vertices
   UINT      uPrpMask;                      // property mask
   UINT      uBit;                          // bit sweep through properties
   int       iPrp;                          // property

   if(pMgr==NULL) return;                   // ignore if no property manager

   //===Deselection=======================================
   // De-select all to begin with
   App()->PrepareProperties(CuVxProperties, FALSE, NULL, this);
   if(!tfAct) return;                       // that's it, if we're losing the focus

   //===Common============================================
   pMgr->FindItemByID(CPS_VXHEADER)->ClearBit(CPIF_HIDDEN); // show the header

   pVx = NULL;                              // ensure we're not finding a Vx here
   iNSel = NumSelectedVx(&pVx);             // count and get first Vx
   //===Readout Properties================================
   //---Heading---------------------------------
   if(iNSel==0) {
      pMgr->FindItemByID( CPS_VXHEADER    )->SetItemHeading(CVertex::CszVxTypeNames);
   } else if(iNSel==1) {
      pMgr->FindItemByID( CPS_VXHEADER    )->SetItemHeading(pVx->TypeString());
   } else {
      pMgr->FindItemByID( CPS_VXHEADER    )->SetItemHeading(CVertex::CszVxTypeNames);
   }

   //---Singles---------------------------------
   if(iNSel==1) {
      //---Reveal common--------------
      pMgr->FindItemByID( CPS_VXTAG       )->ClearBit(CPIF_HIDDEN);
      pMgr->FindItemByID( CPS_VXSPOTSIZE  )->ClearBit(CPIF_HIDDEN);
      pMgr->FindItemByID( CPS_VXCURVATURE )->ClearBit(CPIF_HIDDEN);
      pMgr->FindItemByID( CPS_VXABCDSAG   )->ClearBit(CPIF_HIDDEN);
      pMgr->FindItemByID( CPS_VXABCDTAN   )->ClearBit(CPIF_HIDDEN);
      pMgr->FindItemByID( CPS_VXASTIG     )->ClearBit(CPIF_HIDDEN);
      //---Lock angle-----------------
      switch(pVx->Type()) {
      case CVX_TYPE_MIRROR:
      case CVX_TYPE_FLATMIRROR:
         if(pVx->Prev()==NULL) break;
         if(pVx->Next()==NULL) break;
         pMgr->FindItemByID( CPS_VXLOCKFACEANGL )->ClearBit(CPIF_HIDDEN);
      }
      //---Flip-----------------------
      switch(pVx->Type()) {
      case CVX_TYPE_INBREWSTER:
      case CVX_TYPE_PRISM1:
         pMgr->FindItemByID( CPS_VXFLIPDIR      )->ClearBit(CPIF_HIDDEN);
      }
      //---Astigmatism----------------
      switch(pVx->Type()) {
      case CVX_TYPE_MIRROR:
      case CVX_TYPE_LENS:
      case CVX_TYPE_THERMALLENS:
      case CVX_TYPE_INBREWSTER:
      case CVX_TYPE_OUTBREWSTER:
      case CVX_TYPE_INPLATE:
      case CVX_TYPE_OUTPLATE:
      case CVX_TYPE_INCRYSTAL:
      case CVX_TYPE_OUTCRYSTAL:
         pMgr->FindItemByID( CPS_VXROCFLASTIG   )->ClearBit(CPIF_HIDDEN);
      }
      //---Set common callbacks-------
      pMgr->FindItemByID( CPS_VXTAG          )->SetItemCallback(CSystem::_VxPropItemCallback, this);
      pMgr->FindItemByID( CPS_VXFLIPDIR      )->SetItemCallback(CSystem::_VxPropItemCallback, this);
      pMgr->FindItemByID( CPS_VXROCFLASTIG   )->SetItemCallback(CSystem::_VxPropItemCallback, this);
   }

   //===Equations=========================================
   if(iNSel > 0) {
      //---Establish visible--------------------
      // I'd prefer not to make  a special case for LOCK-
      // FACEANGL but since it's not an equation we can't
      // use a CVXB- bit flag.
      pMgr->FindItemByID( CPS_VXLOCKFACEANGL )->ClearBit(CPIF_HIDDEN); // assume visible
      for(uPrpMask=0xFFFF; pVx; pVx=NextSelectedVx(pVx)) {
         uPrpMask &= CVertex::CuVxSavePropertyMask[pVx->Type()];
         if(pVx->Prev()==NULL) uPrpMask &=~ CVXB_EQTN_FACEANGLE; ///TODO: Maybe allow for ring?
         if(pVx->Next()==NULL) uPrpMask &=~(CVXB_EQTN_DIST2NEXT | CVXB_EQTN_FACEANGLE);
         //---Angle Lock--------------
         if((pVx->Prev()==NULL) || (pVx->Next()==NULL)
            || !((pVx->Type()==CVX_TYPE_MIRROR) || (pVx->Type()==CVX_TYPE_FLATMIRROR)))
            pMgr->FindItemByID( CPS_VXLOCKFACEANGL )->SetBit(CPIF_HIDDEN);
      }

      //---Reveal-------------------------------
      if(uPrpMask & CVXB_EQTN_DIST2NEXT) pMgr->FindItemByID( CPS_VXDISTNEXT     )->ClearBit(CPIF_HIDDEN);
      if(uPrpMask & CVXB_EQTN_N        ) pMgr->FindItemByID( CPS_VXBLOCKREFINDX )->ClearBit(CPIF_HIDDEN);
      if(uPrpMask & CVXB_EQTN_FACEANGLE) pMgr->FindItemByID( CPS_VXFACEANGL     )->ClearBit(CPIF_HIDDEN);
      if(uPrpMask & CVXB_EQTN_THICKNESS) pMgr->FindItemByID( CPS_VXTHICKNESS    )->ClearBit(CPIF_HIDDEN);
      if(NextSelectedVx(NULL)->AstigROCFL()) {
         if(uPrpMask & CVXB_EQTN_ROC      ) pMgr->FindItemByID( CPS_VXRADCURVSAG   )->ClearBit(CPIF_HIDDEN);
         if(uPrpMask & CVXB_EQTN_ROCTAN   ) pMgr->FindItemByID( CPS_VXRADCURVTAN   )->ClearBit(CPIF_HIDDEN);
         if(uPrpMask & CVXB_EQTN_FL       ) pMgr->FindItemByID( CPS_VXFOCALLENSAG  )->ClearBit(CPIF_HIDDEN);
         if(uPrpMask & CVXB_EQTN_FLTAN    ) pMgr->FindItemByID( CPS_VXFOCALLENTAN  )->ClearBit(CPIF_HIDDEN);
      } else {
         if(uPrpMask & CVXB_EQTN_ROC      ) pMgr->FindItemByID( CPS_VXRADCURV      )->ClearBit(CPIF_HIDDEN);
         if(uPrpMask & CVXB_EQTN_FL       ) pMgr->FindItemByID( CPS_VXFOCALLEN     )->ClearBit(CPIF_HIDDEN);
      }

      //---Set source equations-----------------
      if(iNSel==1) {
         pVx = NextSelectedVx(NULL);        // get top vertex again
         pMgr->FindItemByID( CPS_VXDISTNEXT     )->SetItemEquation(pVx->Eq(CVXI_EQTN_DIST2NEXT));
         pMgr->FindItemByID( CPS_VXBLOCKREFINDX )->SetItemEquation(pVx->Eq(CVXI_EQTN_N)        );
         pMgr->FindItemByID( CPS_VXFACEANGL     )->SetItemEquation(pVx->Eq(CVXI_EQTN_FACEANGLE));
         pMgr->FindItemByID( CPS_VXRADCURV      )->SetItemEquation(pVx->Eq(CVXI_EQTN_ROC)      );
         pMgr->FindItemByID( CPS_VXRADCURVSAG   )->SetItemEquation(pVx->Eq(CVXI_EQTN_ROC)      );
         pMgr->FindItemByID( CPS_VXRADCURVTAN   )->SetItemEquation(pVx->Eq(CVXI_EQTN_ROCTAN)   );
         pMgr->FindItemByID( CPS_VXFOCALLEN     )->SetItemEquation(pVx->Eq(CVXI_EQTN_FL)       );
         pMgr->FindItemByID( CPS_VXFOCALLENSAG  )->SetItemEquation(pVx->Eq(CVXI_EQTN_FL)       );
         pMgr->FindItemByID( CPS_VXFOCALLENTAN  )->SetItemEquation(pVx->Eq(CVXI_EQTN_FLTAN)    );
         pMgr->FindItemByID( CPS_VXTHICKNESS    )->SetItemEquation(pVx->Eq(CVXI_EQTN_THICKNESS));
      } else {
         pMgr->FindItemByID( CPS_VXDISTNEXT     )->SetItemEquation(NULL);
         pMgr->FindItemByID( CPS_VXBLOCKREFINDX )->SetItemEquation(NULL);
         pMgr->FindItemByID( CPS_VXFACEANGL     )->SetItemEquation(NULL);
         pMgr->FindItemByID( CPS_VXRADCURV      )->SetItemEquation(NULL);
         pMgr->FindItemByID( CPS_VXRADCURVSAG   )->SetItemEquation(NULL);
         pMgr->FindItemByID( CPS_VXRADCURVTAN   )->SetItemEquation(NULL);
         pMgr->FindItemByID( CPS_VXFOCALLEN     )->SetItemEquation(NULL);
         pMgr->FindItemByID( CPS_VXFOCALLENSAG  )->SetItemEquation(NULL);
         pMgr->FindItemByID( CPS_VXFOCALLENTAN  )->SetItemEquation(NULL);
         pMgr->FindItemByID( CPS_VXTHICKNESS    )->SetItemEquation(NULL);
      }
      //---Set callback-------------------------
      pMgr->FindItemByID( CPS_VXDISTNEXT     )->SetItemCallback(CSystem::_VxPropItemCallback, this);
      pMgr->FindItemByID( CPS_VXBLOCKREFINDX )->SetItemCallback(CSystem::_VxPropItemCallback, this);
      pMgr->FindItemByID( CPS_VXFACEANGL     )->SetItemCallback(CSystem::_VxPropItemCallback, this);
      pMgr->FindItemByID( CPS_VXLOCKFACEANGL )->SetItemCallback(CSystem::_VxPropItemCallback, this);
      pMgr->FindItemByID( CPS_VXRADCURV      )->SetItemCallback(CSystem::_VxPropItemCallback, this);
      pMgr->FindItemByID( CPS_VXRADCURVSAG   )->SetItemCallback(CSystem::_VxPropItemCallback, this);
      pMgr->FindItemByID( CPS_VXRADCURVTAN   )->SetItemCallback(CSystem::_VxPropItemCallback, this);
      pMgr->FindItemByID( CPS_VXFOCALLEN     )->SetItemCallback(CSystem::_VxPropItemCallback, this);
      pMgr->FindItemByID( CPS_VXFOCALLENSAG  )->SetItemCallback(CSystem::_VxPropItemCallback, this);
      pMgr->FindItemByID( CPS_VXFOCALLENTAN  )->SetItemCallback(CSystem::_VxPropItemCallback, this);
      pMgr->FindItemByID( CPS_VXTHICKNESS    )->SetItemCallback(CSystem::_VxPropItemCallback, this);
   }

   //---Hide in draft mode----------------------
   if((tfAct) && (pMgr!=NULL) && DraftMode()) {
      App()->PrepareProperties(CuVxPropDraftHide, FALSE, NULL, this);
   }

   //===Finalize==========================================
   UpdateVxProperties();
   pMgr->OnResize();                        // position items and resize actives
   pMgr->OnPaint(FALSE);                    // paint everything
}

/*********************************************************
* UpdateVxPreprties
* This works even  more closely together  with PrepareVx-
* Properties than other  classes, because things are dif-
* ferent  for multiple  selection. Here we are  concerned
* only with non-equation members.
*********************************************************/
void CSystem::UpdateVxProperties(void) {
   CPropMgr *pMgr = App()->PropManager();   // assign for code below
   CVertex  *pVx;                           // current vertex
   CMatrix2x2 MxSag, MxTan;                 // matrices
   int       iNSel;                         // number selected

   if(pMgr==NULL) return;                   // ignore if no manager
   iNSel = NumSelectedVx(&pVx);             // count number, get current vx

   //===Single============================================
   if(iNSel==1) {
      pVx->AbcdVx(&MxSag, &MxTan, TRUE);    // get forward matrix
      pMgr->FindItemByID( CPS_VXTAG          )->SetItemText(pVx->Tag());
      pMgr->FindItemByID( CPS_VXSPOTSIZE     )->SetItemSagTanValue(pVx->Q(SAG)->W(), pVx->Q(TAN)->W());
      pMgr->FindItemByID( CPS_VXCURVATURE    )->SetItemSagTanValue(pVx->Q(SAG)->R(), pVx->Q(TAN)->R());
      pMgr->FindItemByID( CPS_VXABCDSAG      )->SetItemQuadValue(MxSag.A, MxSag.B, MxSag.C, MxSag.D);
      pMgr->FindItemByID( CPS_VXABCDTAN      )->SetItemQuadValue(MxTan.A, MxTan.B, MxTan.C, MxTan.D);
      pMgr->FindItemByID( CPS_VXASTIG        )->SetItemValue(pVx->Q(SAG)->z0() - pVx->Q(TAN)->z0());

      pMgr->FindItemByID( CPS_VXSPOTSIZE     )->SetItemNaN(!StableABCD(SAG), !StableABCD(TAN));
      pMgr->FindItemByID( CPS_VXCURVATURE    )->SetItemNaN(!StableABCD(SAG), !StableABCD(TAN));
      pMgr->FindItemByID( CPS_VXASTIG        )->SetItemNaN(!(StableABCD(SAG) && StableABCD(TAN)), TRUE);

   //===Multi=============================================
   } else if(iNSel > 1) {
   }

   //===Check boxes=======================================
   if(pVx != NULL) {
      pMgr->FindItemByID( CPS_VXLOCKFACEANGL )->SetItemCheckBox(pVx->LockedAngle() ? TRUE : FALSE);
      pMgr->FindItemByID( CPS_VXROCFLASTIG   )->SetItemCheckBox(pVx->AstigROCFL()  ? TRUE : FALSE);
   }
}


/*********************************************************
* VxPropMgrCallback
* Called back from the property manager when a value has
* been changed. Return FALSE to keep the item visible.
*
* The types of callback depend on how many optics are se-
* lected; in particular, some  can only happen for multi-
* ple selection.
*
* The callback function must be declared static.
*********************************************************/
BOOL CSystem::_VxPropItemCallback(void *vData, UINT uID, void *pVoid) {
   return( (pVoid) ? ((CSystem*)pVoid)->VxPropItemCallback(vData, uID) : TRUE );
}
//******************************************************//
BOOL CSystem::VxPropItemCallback(void *vData, UINT uID) {
   char      szBuf[256];                    // formatted error string
   CEquation Eq;                            // prototype parser for multi-select
   CVertex  *pVx;                           // first / selected vx
   int       iNSel;                         // number of selected vx
   int       iEq;                           // equation index

   iNSel = NumSelectedVx(&pVx);             // get count, first selected
   if(pVx==NULL) return(TRUE);              // accept if nothing selected
   switch(uID) {
   //---Tag-------------------------------------
   case CPS_VXTAG:
      if(iNSel > 1) break;                  // set only one at a time
      if(GetTopSystem()->FindVxByTag((const char*) vData)!=NULL) return(FALSE); // allow only unique
      pVx->SetTag((const char*) vData);
      break;
   //---Lock------------------------------------
   case CPS_VXLOCKFACEANGL:
      for(pVx=NextSelectedVx(NULL); pVx; pVx=NextSelectedVx(pVx)) {
         pVx->LockAngle(*(BOOL*)vData ? FALSE : TRUE);
      }
      break;
   //---Flip------------------------------------
   case CPS_VXFLIPDIR:
      for(pVx=NextSelectedVx(NULL); pVx; pVx=NextSelectedVx(pVx)) {
         pVx->FlipVertex(pVx->IsFlipped() ? FALSE : TRUE);
      }
      break;
   //---Equations-------------------------------
   case CPS_VXDISTNEXT     :
   case CPS_VXBLOCKREFINDX :
   case CPS_VXFACEANGL     :
   case CPS_VXRADCURV      :
   case CPS_VXRADCURVSAG   :
   case CPS_VXRADCURVTAN   :
   case CPS_VXFOCALLEN     :
   case CPS_VXFOCALLENSAG  :
   case CPS_VXFOCALLENTAN  :
   case CPS_VXTHICKNESS    :
      if( Eq.ParseEquation((const char*) vData, App()->VarsString()) != EQERR_NONE ) {
         Eq.LastErrorMessage(szBuf, sizeof(szBuf)/sizeof(char), (const char*) vData);
         App()->GetStatusBar()->SetPriorityText(szBuf);
         return(FALSE);
      }
      for(;pVx; pVx=NextSelectedVx(pVx)) {
         switch(uID) {
         case CPS_VXDISTNEXT     : pVx->ParseVxEq(CVXI_EQTN_DIST2NEXT, (const char*) vData); break;
         case CPS_VXBLOCKREFINDX : pVx->ParseVxEq(CVXI_EQTN_N        , (const char*) vData); break;
         case CPS_VXFACEANGL     : pVx->ParseVxEq(CVXI_EQTN_FACEANGLE, (const char*) vData); break;
         case CPS_VXRADCURV      : pVx->ParseVxEq(CVXI_EQTN_ROC      , (const char*) vData); pVx->ParseVxEq(CVXI_EQTN_ROCTAN   , (const char*) vData); break;
         case CPS_VXRADCURVSAG   : pVx->ParseVxEq(CVXI_EQTN_ROC      , (const char*) vData); break;
         case CPS_VXRADCURVTAN   : pVx->ParseVxEq(CVXI_EQTN_ROCTAN   , (const char*) vData); break;
         case CPS_VXFOCALLEN     : pVx->ParseVxEq(CVXI_EQTN_FL       , (const char*) vData); pVx->ParseVxEq(CVXI_EQTN_FLTAN    , (const char*) vData); break;
         case CPS_VXFOCALLENSAG  : pVx->ParseVxEq(CVXI_EQTN_FL       , (const char*) vData); break;
         case CPS_VXFOCALLENTAN  : pVx->ParseVxEq(CVXI_EQTN_FLTAN    , (const char*) vData); break;
         case CPS_VXTHICKNESS    : pVx->ParseVxEq(CVXI_EQTN_THICKNESS, (const char*) vData); break;
         }
      }
      break;
   //---Astigmatic------------------------------
   case CPS_VXROCFLASTIG:
      for(pVx=NextSelectedVx(NULL); pVx; pVx=NextSelectedVx(pVx)) {
         pVx->SetAstigROCFL(*(BOOL*)vData ? FALSE : TRUE);
      }
      PrepareVxProperties(TRUE);
      break;
   }

   App()->ScanAll();
   return(TRUE);
}




/*########################################################
 ## Plotting Functions                                 ##
########################################################*/
/*********************************************************
* SysFcnString
* Returns the string corresponding to the specified plot-
* ting function index.
* Called from
*  <- CSysWinGraph::OnPaint
*********************************************************/
const char* CSystem::SysFcnString(int iIndx) {
   char *psz;                               // string pointer loop counter
   if((iIndx<0) || (iIndx>CSYSI_PROP_FCNMAX)) return(NULL);
   for(psz=(char*)CszSysFcnNames; iIndx>0; iIndx--, psz+=strlen(psz)+1);
   return((const char*)psz);
}

/*********************************************************
* FcnValue
* The iPltFcn takes on one of the  low CSYSI_PROP values.
* For sagittal/tangential  pairs, the "sagittal" property
* refers to both.
* Called from
*  <- SysGraph Renderer : GraphPoint
*     <- App: ScanAll
*********************************************************/
void CSystem::FcnValue(int iPltFcn, double *pdSag, double *pdTan) {
   switch(iPltFcn) {
   case CSYSI_PROP_PHYSLEN:     if(pdSag) *pdSag = ddSysProp[CSYSI_PROP_PHYSLEN]; break;
   case CSYSI_PROP_OPTLEN:      if(pdSag) *pdSag = ddSysProp[CSYSI_PROP_OPTLEN ]; break;
   case CSYSI_PROP_MODESPACING: if(pdSag) *pdSag = ddSysProp[CSYSI_PROP_MODESPACING]; break;
   case CSYSI_PROP_STABSAG:     if(pdSag) *pdSag = ddSysProp[CSYSI_PROP_STABSAG]; if(pdTan) *pdTan = ddSysProp[CSYSI_PROP_STABTAN]; break;
   }
}

/*########################################################
## Data Files                                           ##
########################################################*/
/*********************************************************
* SetFileName
* This is first called from the constructor, to set a de-
* fault name  (path=NULL); and also whenever the  file is
* loaded or saved to reflect the drive path and name.
*********************************************************/
void CSystem::SetFileName(const char *pszFullFileIn, const char *pszFileNameIn) {
   int iLen;                                // length of string
   //---Only set for top system-------
   if(pVxSpawn) {
      pVxSpawn->SysParent()->SetFileName(pszFullFileIn, pszFileNameIn);
      return;
   }
   //---Free--------------------------
   if(pszFullFile) delete(pszFullFile); pszFullFile = NULL;
   if(pszFileName) delete(pszFileName); pszFileName = NULL;
   //---Full path / name--------------
   if(pszFullFileIn) {
      iLen = strlen(pszFullFileIn);         // get length of supplied name
      pszFullFile = (char*) malloc((iLen+1) * sizeof(char)); // allocate the buffer
      if(pszFullFile) {
         memset(pszFullFile, 0x00, (iLen+1)*sizeof(char)); // clear the buffer
         strncpy(pszFullFile, pszFullFileIn, iLen); // copy the name
      }
   }
   //---Title name--------------------
   if(pszFileNameIn) {
      iLen = strlen(pszFileNameIn);         // get length of supplied name
      pszFileName = (char*) malloc((iLen+1) * sizeof(char)); // allocate the buffer
      if(pszFileName) {
         memset(pszFileName, 0x00, (iLen+1)*sizeof(char)); // clear the buffer
         strncpy(pszFileName, pszFileNameIn, iLen); // copy the name
      }
   }
}
//===SetDefaultName=======================================
void CSystem::SetDefaultName(void) {
   char szBuf[256];                         // temporary string
   sprintf(szBuf, "Untitled%d", ++iUntitledCounter);
   SetFileName(NULL, szBuf);                // set on top system
}
//===GetFullFile==========================================
BOOL CSystem::GetFullFile(char *pszBuf, size_t len) {
   //---Indirect to top system--------
   if(pVxSpawn) return(pVxSpawn->SysParent()->GetFullFile(pszBuf, len));

   //---Copy buffer-------------------
   if(pszBuf) strncpy(pszBuf, pszFullFile, len-1);  // copy full name
   return((pszFullFile!=NULL) ? TRUE : FALSE); // return TRUE if set
}

//===FileName=============================================
const char* CSystem::FileName(void) {
   if(pVxSpawn) return(pVxSpawn->SysParent()->FileName());
   return((const char*) pszFileName);
}

/*********************************************************
* FindVxByTag
* Search through the system AND SPAWNED SYSTEMS to find a
* vertex with the  specified tag. This  is necessary when
* loading files, since  renderer pointers refer to vertex
* tags.
* If the Vx cannot be  found in the  current system, this
* function returns NULL.  This also permits searching the
* spawned systems by recursion of this function.
*********************************************************/
CVertex* CSystem::FindVxByTag(const char *pszTag) {
   CVertex *pVx, *pVxFound;                 // vertex loop pointer

   if(pszTag==NULL) return(NULL);           // ignore bad tag pointer
   pVx = pVxTop;                            // start at top of chain
   while(pVx) {
      if(pVx->MatchTag(pszTag)) return(pVx); // found it, so return the pointer
      if(pVx->SysSpawned()) {
         pVxFound = pVx->SysSpawned()->FindVxByTag(pszTag); // search subsystem
         if(pVxFound != NULL) return(pVxFound); // found in subsystem, return the pointer
      }
      pVx = pVx->Next();                    // follow down through chain
   }
   return(NULL);                            // couldn't find the vertex in this system
}


/*********************************************************
* FindSysByTag
* Find system, or  spawned system, whose  tag matches the
* given string. Used when loading files.
*********************************************************/
CSystem* CSystem::FindSysByTag(const char *pszTag) {
   CVertex *pVx;                            // vertex loop pointer
   CSystem *pSys;                           // spawned system pointer

   if(MatchTag(pszTag)) return(this);       // this is the system
   for(pVx=pVxTop; pVx; pVx=pVx->Next()) {  // loop through vertices
      if(pVx->SysSpawned()) {
         pSys = pVx->SysSpawned()->FindSysByTag(pszTag);
         if(pSys != NULL) return(pSys);
      }
   }
   return(NULL);                            // sorry dude, couldn't find it
}

/*********************************************************
* SaveSystem
* The System's tag is written as-is, so the top system is
* no longer  distinguished except by its placement as the
* first system in the file. The equation properties are
* stored for the top system only.
* This function is  initially called  from CApplication::
* UserSaveSystem, then recursively.
*********************************************************/
void CSystem::SaveSystem(HANDLE hFile) {
   char      szBuf[256];                    // text buffer
   DWORD     dwBytes;                       // number of bytes written
   int       iVar;                          // variable loop counter
   int       iSysPrp;                       // system property loop counter
   CVertex  *pVx;                           // vertex loop counter
   CSysWin  *pSWin;                         // renderer loop counter

   if(hFile == NULL) printf("? CSystem::SaveSystem@206 called with hFile=NULL\n");
   if(hFile == NULL) return;                // ignore if no file provided

   //===System Information================================
   sprintf(szBuf, "[%s]\r\n", szSysTag);
   WriteFile(hFile, szBuf, strlen(szBuf), &dwBytes, NULL);
   //---Top System------------------------------
   if(pVxSpawn == NULL) {
      //---Type-------------
      sprintf(szBuf,"%s\r\n", CszSysType[iSysType]);
      WriteFile(hFile, szBuf, strlen(szBuf), &dwBytes, NULL);
      //---Prop input type---
      if(iSysType==CSYS_TYPE_PROP) {
         sprintf(szBuf, "%s = %d\n", CszSysAuxSaveName[CSYSI_AUXSAVE_INPUT], InputParam());
         WriteFile(hFile, szBuf, strlen(szBuf), &dwBytes, NULL);
      }
      //---Asymmetric M^2---
      if(MSqAsymmetric()) {
         sprintf(szBuf, "%s\n", CszSysAuxSaveName[CSYSI_AUXSAVE_MSQASYM]);
         WriteFile(hFile, szBuf, strlen(szBuf), &dwBytes, NULL);
      }
      //---Draft mode---
      if(DraftMode()) {
         sprintf(szBuf, "%s\n", CszSysAuxSaveName[CSYSI_AUXSAVE_DRAFTMODE]);
         WriteFile(hFile, szBuf, strlen(szBuf), &dwBytes, NULL);
      }
      //---Variables--------
      for(iVar=0; iVar<pAppParent->NumVars(); iVar++) {
         sprintf(szBuf, CszVarFormat[CSYSI_SAVE_VARVALUE], pAppParent->VarString(iVar));
         sprintf(szBuf+strlen(szBuf), " = %lg\r\n", pAppParent->Var(iVar));
         WriteFile(hFile, szBuf, strlen(szBuf), &dwBytes, NULL);
         sprintf(szBuf, CszVarFormat[CSYSI_SAVE_VARRANGE], pAppParent->VarString(iVar));
         sprintf(szBuf+strlen(szBuf), " = %lg, %lg\r\n", pAppParent->VarMin(iVar), pAppParent->VarMax(iVar));
         WriteFile(hFile, szBuf, strlen(szBuf), &dwBytes, NULL);
      }
      //---Properties-------
      // This includes all of the equation, even those not actually used
      for(iSysPrp=0; iSysPrp<CSYSI_NUM_EQTN; iSysPrp++) {
         sprintf(szBuf, "%s = ", CszSysEquationName[iSysPrp]);
         WriteFile(hFile, szBuf, strlen(szBuf), &dwBytes, NULL);
         EqSys[iSysPrp].GetEquationString(szBuf, sizeof(szBuf));
         WriteFile(hFile, szBuf, strlen(szBuf), &dwBytes, NULL);
         sprintf(szBuf, "\r\n");
         WriteFile(hFile, szBuf, strlen(szBuf), &dwBytes, NULL);
      }
   //---Spawned system--------------------------
   // Note: Spawned systems are automatically propagation-style systems.
   } else {
      for(iSysPrp=CSYSI_EQTN_COMMON; iSysPrp<CSYSI_NUM_EQTN; iSysPrp++) {
         sprintf(szBuf, "%s = ", CszSysEquationName[iSysPrp]);
         WriteFile(hFile, szBuf, strlen(szBuf), &dwBytes, NULL);
         EqSys[iSysPrp].GetEquationString(szBuf, sizeof(szBuf));
         WriteFile(hFile, szBuf, strlen(szBuf), &dwBytes, NULL);
         sprintf(szBuf, "\r\n");
         WriteFile(hFile, szBuf, strlen(szBuf), &dwBytes, NULL);
      }
   }

   //---Vertices--------------------------------
   for(pVx=pVxTop; pVx; pVx=pVx->Next()) {
      pVx->SaveVertex(hFile);               // save each vertex
   }

   //---Spawned Systems-------------------------
   for(pVx=pVxTop; pVx; pVx=pVx->Next()) {
      if(pVx->SysSpawned() != NULL) pVx->SysSpawned()->SaveSystem(hFile);
   }

   //---Renderers-------------------------------
   for(pSWin=pSysWinTop; pSWin; pSWin=pSWin->Next()) {
      pSWin->SaveSysWin(hFile);             // save each renderer
   }

}


/*********************************************************
* LoadSystem
* Load top or spawned system  from the supplied data buf-
* fer; here separate since it  may be called recursively.
* The szSysName  MUST INCLUDE  the [System]  brackets. If
* szSysName is NULL, we look for the first system heading
* with brackets, and the top system's name is ignored.
* This function is  initially called from  CApplication::
* UserLoadSystem, then recursively
*********************************************************/
BOOL CSystem::LoadSystem(const char *pszDataFile, const char *szSysName) {
   char     szBuf[256];                     // buffer for equation string
   char    *pszSysMin, *pszSysMax;          // limits in system
   char    *pszSysEqtn;                     // system property
   CVertex *pVxPrv;                         // previous vertex
   CVertex *pVxNew;                         // newly created vertex
   CVertex *pVxLnk;                         // searching for linked vx
   int      iVar;                           // variable loop counter
   double   dVar, dVar2;                    // variable value(s)
   int      iSysEqn;                        // system property loop counter
   int      iSWTyp;                         // renderer type loop counter
   char    *pszSWin, *pszSWinSys;           // pointer to renderer heading, system tag
   CSystem *pSysWinSys;                     // system for renderer
   CSysWin *pSysWinNew;                     // new renderer object

   //===Find Section======================================
   //---First system----------------------------
   if(szSysName == NULL) {
      //---Tag------------------------
      pszSysMin = strchr((char*) pszDataFile, '['); // first opening brace
      pszSysMax = strchr((char*) pszDataFile, ']'); // first closing brace
      if((pszSysMin==NULL) || (pszSysMax==NULL)) {  // couldn't find it, error
         App()->LoadErrorMsg(SZERR_LOAD_NOTOPSYS, NULL, NULL);
         return(FALSE);
      }
      memset(szSysTag, 0x00, sizeof(szSysTag));  // clear the buffer
      if(pszSysMax-pszSysMin-1 > CSYSC_MAXTAG) strncpy(szSysTag, pszSysMin+1, CSYSC_MAXTAG);
      else                                     strncpy(szSysTag, pszSysMin+1, pszSysMax-pszSysMin-1);
      pszSysMin = pszSysMax + 1;            // skip past heading
      pszSysMax = strchr(pszSysMin, '[');   // find start of next section
      if(pszSysMax==NULL) pszSysMax = pszSysMin + strlen(pszSysMin); //..or to end of file

      //---System type----------------
      for(iSysType=0; iSysType<=CSYS_TYPE_MAX; iSysType++) {
         pszSysEqtn = strstr(pszSysMin, CszSysType[iSysType]); // search for this system type
         if((pszSysEqtn==NULL) || (pszSysEqtn>pszSysMax)) continue; // ignore if not within bounds
         break;                             // found, so break out of loop
      }
      if(iSysType>CSYS_TYPE_MAX) iSysType = CSYS_TYPE_PROP; // default: propagation

      //---Input param----------------
      if(iSysType==CSYS_TYPE_PROP) {
         sprintf(szBuf, CszVarFormat[CSYSI_SAVE_VARRANGE], pAppParent->VarString(iVar)); // format 'Variable(x)'
         pszSysEqtn = strstr(pszSysMin, CszSysAuxSaveName[CSYSI_AUXSAVE_INPUT]); // search for input type string
         if((pszSysEqtn) && (pszSysEqtn<=pszSysMax)) { // ignore if not within bounds
            pszSysEqtn = strchr(pszSysEqtn, '=');  // find '=' sign
            if((pszSysEqtn) && (pszSysEqtn<=pszSysMax)) {    // ignore if not found with bounds
               if(sscanf(pszSysEqtn+1, "%lg", &dVar) > 0) { ClearBit(CSYSF_INPUTPARAM); SetBit(((int)dVar) & CSYSF_INPUTPARAM); };
            }
         }
      }

      //---Asymmetric M^2-------------
      pszSysEqtn = strstr(pszSysMin, CszSysAuxSaveName[CSYSI_AUXSAVE_MSQASYM]); // search for flag
      if((pszSysEqtn) && (pszSysEqtn <= pszSysMax)) SetMSqAsymmetric(TRUE); // found within bounds, set the flag

      //---Draft mode-----------------
      pszSysEqtn = strstr(pszSysMin, CszSysAuxSaveName[CSYSI_AUXSAVE_DRAFTMODE]); // search for flag
      if((pszSysEqtn) && (pszSysEqtn <= pszSysMax)) SetBit(CSYSF_DRAFTMODE); // found within bounds, set the flag

      //---Variables------------------      // only loaded from file if it's the only system going
      if(pAppParent->GetCurrentSystem()==NULL) {
         //---Ranges---                     // load ranges first so the VALUE is guaranteed
         for(iVar=0; iVar<pAppParent->NumVars(); iVar++) {
            sprintf(szBuf, CszVarFormat[CSYSI_SAVE_VARRANGE], pAppParent->VarString(iVar)); // format 'Variable(x)'
            pszSysEqtn = strstr(pszSysMin, szBuf); // search for this variable string
            if((pszSysEqtn==NULL) || (pszSysEqtn>pszSysMax)) continue; // ignore if not within bounds
            pszSysEqtn = strchr(pszSysEqtn, '=');  // find '=' sign
            if((pszSysEqtn==NULL) || (pszSysEqtn>pszSysMax)) continue;    // ignore if not found with bounds
            if(sscanf(pszSysEqtn+1, "%lg,%lg", &dVar, &dVar2) > 1) {
               pAppParent->SetVarRange(iVar, dVar, dVar2);
            }
         }
         //---Values---
         for(iVar=0; iVar<pAppParent->NumVars(); iVar++) {
            sprintf(szBuf, CszVarFormat[CSYSI_SAVE_VARVALUE], pAppParent->VarString(iVar)); // format 'Variable(x)'
            pszSysEqtn = strstr(pszSysMin, szBuf); // search for this variable string
            if((pszSysEqtn==NULL) || (pszSysEqtn>pszSysMax)) continue; // ignore if not within bounds
            pszSysEqtn = strchr(pszSysEqtn, '=');  // find '=' sign
            if((pszSysEqtn==NULL) || (pszSysEqtn>pszSysMax)) continue;    // ignore if not found with bounds
            if(sscanf(pszSysEqtn+1, "%lg", &dVar) > 0) {
               pAppParent->SetVar(iVar, dVar); // if parsed ok, set parent's value
            }
         }
      }//endif(this is first loaded system)

      //---Equations------------------
      for(iSysEqn=0; iSysEqn<CSYSI_NUM_EQTN; iSysEqn++) {
         pszSysEqtn = strstr(pszSysMin, CszSysEquationName[iSysEqn]);  // find property title
         if((pszSysEqtn==NULL) || (pszSysEqtn>pszSysMax)) continue;    // ignore if not found within bounds
         pszSysEqtn = strchr(pszSysEqtn, '=');     // find '=' sign
         if((pszSysEqtn==NULL) || (pszSysEqtn>pszSysMax)) continue;    // ignore if not found with bounds
         sscanf(pszSysEqtn+1, "%s", szBuf);
         EqSys[iSysEqn].ParseEquation(szBuf, pAppParent->VarsString()); // parse equation with variables
      }

   //---Named System----------------------------
   } else {
      pszSysMin = strstr((char*) pszDataFile, szSysName);     // find section heading
      if(pszSysMin==NULL) {
         App()->LoadErrorMsg(SZERR_LOAD_NOSPAWN, NULL, szSysName);
         return(FALSE);                        // exit if section not found
      }
      memset(szSysTag, 0x00, sizeof(szSysTag)); // clear tag buffer
      if(strlen(szSysName)-2>CSYSC_MAXTAG) strncpy(szSysTag, szSysName+1, CSYSC_MAXTAG);
      else                                 strncpy(szSysTag, szSysName+1, strlen(szSysName)-2);
      pszSysMin += strlen(szSysName);          // skip to end of section heading
      pszSysMax = strchr(pszSysMin, '[');      // find start of next section..
      if(pszSysMax==NULL) pszSysMax = pszSysMin + strlen(pszSysMin);    //.. or end of file
   }

   //===Load Vertices=====================================
   // Maintaining the vertex chain is easier here than in
   // CreateVxAfter because we know that the vertices are
   // loaded in top-down order
   pVxPrv = NULL;                           // start at top of chain
   while(1) {                               // break loop when creation fails
      pVxNew = new CVertex(this, 0);        // create a new blank vertex
      if(pVxNew->LoadVertex(pszDataFile, &pszSysMin, &pszSysMax)) {
         if(pVxTop==NULL) pVxTop = pVxNew;  // maintain chain head
         if(pVxPrv) pVxPrv->SetNext(pVxNew); // Prev->New
         pVxNew->SetPrev(pVxPrv);           // Prev<-New
         pVxPrv = pVxNew;                   // point to new end of chain
      } else {
         delete(pVxNew);                    // delete unnecessary vertex
         break;                             // exit from vertex loading loop
      }
   }

   //---Establish Links-------------------------
   // use pVxNew as loop counter here -- they're still relatively new
   for(pVxNew=pVxTop; pVxNew; pVxNew = (CVertex*) pVxNew->Next()) {
      if(pVxNew->VxLinked()) {
         strcpy(szBuf, (char*) pVxNew->VxLinked()); // copy link tag string to buffer
         free((char*) pVxNew->VxLinked());          // free the temporary buffer
         pVxNew->SetLinked(NULL);                   // clear the pointer
         for(pVxLnk=pVxTop; pVxLnk; pVxLnk = (CVertex*) pVxLnk->Next()) {
            if(strncmp(pVxLnk->Tag(), szBuf, strlen(szBuf)) == 0) { // if found matching link
               pVxNew->SetLinked(pVxLnk);           // link the vertex pointer
               break;                            // and exit out of loop
            }//endif(found linked tag)
         }//for(link)
      }//if(has link)
   }//linked

   //---Check integrity-------------------------
///TODO: Add more physical checks here!
///TODO: Check: Vertices that need links have them
///TODO: Check reciprocal links (?)
   if(pVxTop == NULL) {                     // ensure there's at least one optic
      App()->LoadErrorMsg(SZERR_LOAD_SYSNOOPT, NULL, szSysName);
      return(FALSE);
   }

   //===Load Renderers====================================
   // Do this at the end only, so that all of the systems
   // have already been loaded
   if(szSysName == NULL) {
      pszSWin = (char*) pszDataFile;                  // start searching at beginning of file
      while(1) {                                      // loop until no more found
         pszSWin = strstr(pszSWin, "Renderer");       // search for renderer heading
         if(pszSWin == NULL) break;                   // exit once no more found
         pszSysMax = strchr(pszSWin, '{');            // limit type heading search up to opening brace
         for(iSWTyp=0; iSWTyp<=CSYSWINI_MAXTYPE; iSWTyp++) {    // search for each renderer type in turn
            pszSysMin = strstr(pszSWin, CSysWin::CszSysWinType[iSWTyp]); // find next occurrence of current type heading
            if((pszSysMin==NULL) || (pszSysMin>pszSysMax)) continue; // not found within range
            //---Find System--------------------
            pszSysMin = strchr(pszSWin, '{');         // renderer start
            pszSysMax = strchr(pszSysMin, '}');       // renderer end..
            if(pszSysMax==NULL) pszSysMax = pszSysMin + strlen(pszSysMin); // ..or up to end of file
            pszSWinSys = strstr(pszSysMin, "System"); // find system declarator
            if((pszSWinSys==NULL) || (pszSWinSys > pszSysMax)) { // can't find system declarator?
               App()->LoadErrorMsg(SZERR_LOAD_RNDNOSYS, pszDataFile, pszSWin);
               continue;
            }
            pszSWinSys = strchr(pszSWinSys, '=');     // skip ahead to the '=' sign
            pszSWinSys += strcspn(pszSWinSys, CszValidNameChar); // skip ahead to system tag
            if((pszSWinSys==NULL) || (pszSWinSys > pszSysMax)) { // bad declaration?
               App()->LoadErrorMsg(SZERR_LOAD_RNDBADSYS, pszDataFile, pszSWin);
               continue;
            }
            pSysWinSys = FindSysByTag(pszSWinSys);    // find system, including in spawned
            if(pSysWinSys==NULL) {                    // couldn't find matching tag?
               App()->LoadErrorMsg(SZERR_LOAD_RNDSYSFND, pszDataFile, pszSWinSys);
               continue;
            }

            //---Load renderer------------------
            switch(iSWTyp) {
            case CSYSWINI_TYPE_1D:        pSysWinNew = pSysWinSys->UserCreateSysWin1d();           break;
            case CSYSWINI_TYPE_2D:        pSysWinNew = pSysWinSys->UserCreateSysWin2d();           break;
            case CSYSWINI_TYPE_3D:        pSysWinNew = pSysWinSys->UserCreateSysWin3d();           break;
            case CSYSWINI_TYPE_GRAPH:     pSysWinNew = pSysWinSys->UserCreateSysWinGraph();        break;
            case CSYSWINI_TYPE_VXGRAPH:   pSysWinNew = pSysWinSys->UserCreateSysWinVxGraph(FALSE); break; // vertex loaded below
            case CSYSWINI_TYPE_INVENTORY: pSysWinNew = pSysWinSys->UserCreateSysWinInventory();    break;
            case CSYSWINI_TYPE_ABCDSOLVER:pSysWinNew = pSysWinSys->UserCreateSysWinABCDSolver();   break;
            }
            if(pSysWinNew) {
               if( pSysWinNew->LoadSysWin(pszDataFile, pszSysMin, pszSysMax) == FALSE ) { // load remaining properties
                  pSysWinSys->DeleteSysWin(pSysWinNew); // discard if error loading
               }
            }
         }//endfor(renderer type)
         pszSWin = pszSysMax;               // skip over this renderer
      }//end loop searching through file

   }

   //---Finalize--------------------------------
   UserCreateSysWin2d();                    // ensure that the 2d renderer exists (only one allowed!)
   if(CheckBit(CSYSF_DRAFTMODE)) UserSetDraftMode(TRUE); // enter into draft mode
   return(TRUE);                            // system loaded ok
}




/*########################################################
## Debug code                                           ##
########################################################*/
/*********************************************************
* DebugPrint
* Appends system information to the supplied buffer
*********************************************************/
void CSystem::DebugPrint(char *psz, int *pInt) {
   CVertex *pVx;                            // vertex loop counter
   CSysWin *pSWin;                          // renderer loop counter

   //---System info-----------------------------
   for(int k=0; k<*pInt; k++) sprintf(psz+strlen(psz), "   ");
   sprintf(psz+strlen(psz), "CSystem<%s> 0x%08lx - %lg nm %s%s\n%s \"%s\"\n",
      szSysTag, this, WLen(), pVxSpawn?"Spawned@":"", pVxSpawn?pVxSpawn->Tag():"",
      pszFullFile?pszFullFile:"NULL", pszFileName?pszFileName:"NULL");
   (*pInt)++;                               // increase indent level

   //---Vertices--------------------------------
   pVx = pVxTop;                            // start at top of system
   while(pVx) {
      pVx->DebugPrint(psz, pInt);           // print vx info
      pVx = pVx->Next();                    // step through system
   }

   //---Renderers-------------------------------
   pSWin = pSysWinTop;                      // start at top of chain
   while(pSWin) {
      pSWin->DebugPrint(psz, pInt);         // print renderer info
      pSWin = pSWin->Next();                // follow through chain
   }
   (*pInt)--;                               // restore indent level
}
