/*********************************************************
* CSysWinABCDSolver
* System renderer that uses a  minimization solver to op-
* timize a given equation.
* That's the plan, anyway.
* The idea:
* All Vx properties will be accessible by parsing the op-
* timization formula ONLY when  the optimization is star-
* ted; that way, deleting  Vx doesn't need to be tracked.
* The system is then rendered with variations in the glo-
* bal variables, and at each  point the optimization fun-
* ction is evaluated.
* For simplicity, this will probably run in-thread, or at
* least with an application-modal dialog to prevent chan-
* ges during the optimization loop.
*
* $PSchlup 2006 $     $Revision 0 $
*********************************************************/
#include "CSysWinInventory.h"               // class include
/*extern*/ double SimplexMinimise(double(*)(double [],void*),double[],int,int*,double,double,void*); // in NeldMead.cpp


//===Tables===============================================
const char* CSysWinABCDSolver::CszSysWinABCDSolverSaveProp[CSLVC_NUM_PROP] = { // save all properties
   "FunctionTolerance", "MaxIterations", "SolveMode"};

const char* CSysWinABCDSolver::CszSysWinABCDSolverSaveAux[CSLVC_NUM_SAVE] = { // auxiliary saved information
   "Equation"};

const char* CSysWinABCDSolver::CszSolveDirNames = NULL;

const char* CSysWinABCDSolver::CszSolverVariables[CSLVI_PROP_SYSMAX+1] = {
   "w_sag", "R_sag", "w0_sag", "z0_sag", "zR_sag",
   "w_tan", "R_tan", "w0_tan", "z0_tan", "zR_tan",
   "dz0",
   "Lopt", "frep", "G_sag", "G_tan"
};


/*********************************************************
* Constructor
*********************************************************/
CSysWinABCDSolver::CSysWinABCDSolver(CSystem *pSys) : CSysWin(pSys) {printf("+ CSysWin -ABCD Solver- Created\n");
   char  szBuf[256];                        // string table buffer
   char *pszBuf;                            // pointer for list from RC
   int   iLen;                              // string length
   int   k;                                 // loop counter

   //===Members===========================================
   for(k=0; k<CSLVC_NUM_PROP; k++) ddProp[k] = 0.00;
   for(k=0; k<CAPP_NUMVAR; k++) dVarSolve[k] = dVarRestore[k] = 0.00;
   iNumVarSlv    = 0;                       // number of variables USED
   pVxArraySlv   = NULL;                    // vertices used in solver
   piFcnArraySlv = NULL;                    // functions used in solver
   pdValArraySlv = NULL;                    // values of variables

   //---Defaults--------------------------------
   ddProp[CSLVC_PROP_FCNTOL  ] = 1.00e-3;   // solver tolerance
   ddProp[CSLVC_PROP_MAXITER ] = 100;       // maximum iterations
   ddProp[CSLVC_PROP_SOLVEDIR] = 0;         // 0: minimize, 1: maximize

   //---String table----------------------------
   // Here delimited with '~' (including at end!)
   if(CszSolveDirNames==NULL) {
      LoadString(App()->GetInstance(), CPL_SLVDIR, szBuf, sizeof(szBuf)/sizeof(char));
      for(k=0, iLen=strlen(szBuf); k<iLen; k++) if(szBuf[k]=='~') szBuf[k] = '\0';
      pszBuf = (char*) malloc((iLen+1)*sizeof(char)); // allocate buffer
      memset(pszBuf, 0x00, (iLen+1)*sizeof(char));    // empty the buffer
      memcpy(pszBuf, szBuf, iLen*sizeof(char));       // copy the formatted string
      CszSolveDirNames = (const char*) pszBuf;        // fix pointer
   }

   //---Current variables--------------------
   // This used to be executed just before the Solve but-
   // ton was executed in the sense of "Restore to values
   // just before this solve." However, since in practice
   // the solve button is  pressed several times, we here
   // save the values when the solver is first created.
   for(k=0; k<App()->NumVars(); k++) dVarSolve[k] = dVarRestore[k] = App()->Var(k);

   //===Create window=====================================
   hwRenderer = pSystem->CreateSysWinWindow(CSZ_WNDCLASS_SYSWINABCDSOLVER, (LPVOID) this);

   //---Edit Box--------------------------------
   hwEditEqn = CreateWindow("EDIT", NULL,
      WS_CHILD | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL,
      0, 0, 96, 48,
      hwRenderer, (HMENU) NULL, App()->GetInstance(), (void*) NULL);
   if(hwEditEqn) SendMessage(hwEditEqn, WM_SETFONT, (WPARAM) App()->Font(), MAKELPARAM(FALSE, 0));

   //---Solve button----------------------------
   LoadString(App()->GetInstance(), CMU_BUTTON_SOLVE, szBuf, sizeof(szBuf)/sizeof(char));
   hwButSolve = CreateWindow("BUTTON", szBuf,
      WS_CHILD | BS_PUSHBUTTON | BS_DEFPUSHBUTTON,
      0, 0, 64, 24,
      hwRenderer, (HMENU) CMU_BUTTON_SOLVE, App()->GetInstance(), (void*) NULL);

   //---Evaluate button-------------------------
   LoadString(App()->GetInstance(), CMU_BUTTON_EVAL, szBuf, sizeof(szBuf)/sizeof(char));
   hwButEval = CreateWindow("BUTTON", szBuf,
      WS_CHILD | BS_PUSHBUTTON,
      0, 0, 64, 24,
      hwRenderer, (HMENU) CMU_BUTTON_EVAL, App()->GetInstance(), (void*) NULL);

   //---Restore button--------------------------
   LoadString(App()->GetInstance(), CMU_BUTTON_RESTORE, szBuf, sizeof(szBuf)/sizeof(char));
   hwButRestore = CreateWindow("BUTTON", szBuf,
      WS_CHILD | BS_PUSHBUTTON ,
      0, 0, 64, 24,
      hwRenderer, (HMENU) CMU_BUTTON_RESTORE, App()->GetInstance(), (void*) NULL);


   //---Finalize--------------------------------
   if(hwEditEqn   ) SendMessage(hwEditEqn   , WM_SETFONT, (WPARAM) App()->Font(), MAKELPARAM(FALSE, 0));
   if(hwButSolve  ) SendMessage(hwButSolve  , WM_SETFONT, (WPARAM) App()->Font(), MAKELPARAM(FALSE, 0));
   if(hwButEval   ) SendMessage(hwButEval   , WM_SETFONT, (WPARAM) App()->Font(), MAKELPARAM(FALSE, 0));
   if(hwButRestore) SendMessage(hwButRestore, WM_SETFONT, (WPARAM) App()->Font(), MAKELPARAM(FALSE, 0));
   if(hwEditEqn   ) ShowWindow(hwEditEqn   , SW_SHOW);
   if(hwButSolve  ) ShowWindow(hwButSolve  , SW_SHOW);
   if(hwButEval   ) ShowWindow(hwButEval   , SW_SHOW);
   if(hwButRestore) ShowWindow(hwButRestore, SW_SHOW);

   OnResize();                              // force resize, since not automatic!
}


/*********************************************************
* Destructor
*********************************************************/
CSysWinABCDSolver::~CSysWinABCDSolver() {
   //---Equation edit-----------------
   if(hwEditEqn) DestroyWindow(hwEditEqn); hwEditEqn = NULL;

   //---Solver array------------------
   if(pVxArraySlv  ) free(pVxArraySlv  ); pVxArraySlv   = NULL;
   if(piFcnArraySlv) free(piFcnArraySlv); piFcnArraySlv = NULL;
   if(pdValArraySlv) free(pdValArraySlv); pdValArraySlv = NULL;
printf("- CSysWin -ABCD Solver- Destroyed\n");}


/*########################################################
 ## Data Files
########################################################*/
/*********************************************************
* SaveSysWin
* Save the current renderer
* Not much to save for this one; it's dynamically updated
* anyway
*********************************************************/
void CSysWinABCDSolver::SaveSysWin(HANDLE hFile) {
UNREFERENCED_PARAMETER(hFile);
   char  szEqString[4096];                  // solver string
   char  szBuf[256];                        // buffer for writing
   int   iPrp;                              // p
   DWORD dwBytes;                           // number of bytes written

   if(hFile == NULL) printf("? CSysWinABCDSolver::SaveSysWin@80 Called with hFile=NULL\n");
   if(hFile == NULL) return;                // ignore if no file

   SaveSysWinHeader(hFile, Type());         // write the header

   //---Properties------------------------------
   for(iPrp=0; iPrp<CSLVC_NUM_PROP; iPrp++) {
      sprintf(szBuf, "   %s = %lg\r\n", CszSysWinABCDSolverSaveProp[iPrp], ddProp[iPrp]);
      WriteFile(hFile, szBuf, strlen(szBuf), &dwBytes, NULL);
   }


   //---Equation String---------------
   SendMessage(hwEditEqn, WM_GETTEXT, (WPARAM) sizeof(szEqString)/sizeof(char), (LPARAM) szEqString);
   sprintf(szBuf, "   %s = \"", CSysWinABCDSolver::CszSysWinABCDSolverSaveAux[CSLVI_SAVE_EQNSTRING]);
   WriteFile(hFile, szBuf, strlen(szBuf), &dwBytes, NULL);
   WriteFile(hFile, szEqString, strlen(szEqString), &dwBytes, NULL);
   sprintf(szBuf, "\"\r\n");
   WriteFile(hFile, szBuf, strlen(szBuf), &dwBytes, NULL);

   SaveSysWinFooter(hFile);                 // write the footer
   return;                                    // don't save this window
}

/*********************************************************
* LoadSysWin
* Load renderer properties from data file
*********************************************************/
BOOL CSysWinABCDSolver::LoadSysWin(const char *pszDataFile, char *pszMin, char *pszMax) {
   char *pszEqtn, *pszEnd;                  // pointer into data file
   char  szEqString[4096];                  // solver string
   char *psz;                               // pointer in file
   int   iLen;                              // length of buffer to copy
   int   iPrp;                              // property loop counter

   //---Header------------------------
   if(pszMin==NULL) return(FALSE);          // couldn't load if no pointer
   if(pszMax==NULL) pszMax = pszMin + strlen(pszMin); // to end of file if not given

   //---Header------------------------
   LoadSysWinHeader(pszDataFile, pszMin, pszMax);

   //---Specifics---------------------
   for(iPrp=0; iPrp<CSLVC_NUM_PROP; iPrp++) {
      psz = strstr(pszMin, CszSysWinABCDSolverSaveProp[iPrp]);
      if((psz==NULL) || (psz>pszMax)) continue;
      psz = strchr(psz, '=');               //..find '=' sign
      if((psz==NULL) || (psz>pszMax)) continue;
      sscanf(psz+1, "%lg", &ddProp[iPrp]);
   }

   //---Equation string---------------
   do {                                     // multi-attempt structure
      pszEqtn = strstr(pszMin, CSysWinABCDSolver::CszSysWinABCDSolverSaveAux[CSLVI_SAVE_EQNSTRING]); // search for this equation string
      if((pszEqtn==NULL) || (pszEqtn>pszMax)) break;
      pszEqtn = strchr(pszEqtn, '\"');      // look for opening '"'
      if((pszEqtn==NULL) || (pszEqtn>pszMax)) break;
      pszEqtn++;                            // skip over the '"'
      pszEnd = strchr(pszEqtn, '\"');       // find ending '"'
      if((pszEnd ==NULL) || (pszEnd >pszMax)) break;
      memset(szEqString, 0x00, sizeof(szEqString)/sizeof(char)); // erase buffer
      iLen = (int)(pszEnd - pszEqtn);       // length of string to copy
      if(iLen > sizeof(szEqString)/sizeof(char)) iLen = sizeof(szEqString)/sizeof(char);
      strncpy(szEqString, pszEqtn, iLen);
      if(hwEditEqn) SendMessage(hwEditEqn, WM_SETTEXT, (WPARAM) 0, (LPARAM) szEqString);
   } while(0);                              // no repeats

   OnResize();

   return(TRUE);
}

/*########################################################
 ## Overloaded Functions                               ##
########################################################*/
/*********************************************************
* Refresh
*********************************************************/
void CSysWinABCDSolver::Refresh(void) {
   if(hwRenderer) InvalidateRect(hwRenderer, NULL, TRUE);
}


/*********************************************************
* UpdateTitle
*********************************************************/
void CSysWinABCDSolver::UpdateTitle(int iID) {
   char szRC[128];                          // string from resource file
   char szBuf[256];                         // formatted string
   LoadString(App()->GetInstance(), SZ_TITLE_SYSWINABCDSOLVER, szRC, sizeof(szRC)/sizeof(char));
   sprintf(szBuf, szRC, (pSystem) ? ((pSystem->FileName()) ? pSystem->FileName() : "") : "");
   if(iID > 0) sprintf(szBuf+strlen(szBuf), " (%d)", iID);
   if(hwRenderer) SetWindowText(hwRenderer, szBuf);
}

/*********************************************************
* GraphPoint
*********************************************************/
void CSysWinABCDSolver::GraphPoint(int iRVar, int iPt) {
UNREFERENCED_PARAMETER(iRVar);
UNREFERENCED_PARAMETER(iPt);
}


/*########################################################
 ## Properties                                         ##
########################################################*/
/*********************************************************
* PrepareProperties
*********************************************************/
const UINT CSysWinABCDSolver::CuProperties[] = {
   CPS_SLVHEADER     , // "Solver"
   CPS_SLVFCNTOL     , // "Function Tolerance"
   CPS_SLVMAXITER    , // "Max. Iterations"
   CPS_SLVDIR        , // "Solve for"
   0};
//=======================================================
void CSysWinABCDSolver::PrepareProperties(BOOL tfAct) {
   App()->PrepareVariableProperties(tfAct);
   App()->PrepareProperties(CuProperties, tfAct, CSysWinABCDSolver::_SysWinPropItemCallback, this);
   App()->EnableMenus(tfAct ? CSYSWINI_TYPE_ABCDSOLVER : -1);
   pSystem->PrepareProperties(tfAct);

   if(tfAct) UpdateProperties();            // update my properties
}

/*********************************************************
* UpdateProperties
*********************************************************/
void CSysWinABCDSolver::UpdateProperties(void) {
   CPropMgr *pMgr = App()->PropManager();   // assign for code below
   if(pMgr==NULL) return;                   // ignore if no manager
   pMgr->FindItemByID(CPS_SLVFCNTOL )->SetItemValue(FcnTol());
   pMgr->FindItemByID(CPS_SLVMAXITER)->SetItemValue(MaxIter());
   pMgr->FindItemByID(CPS_SLVDIR    )->SetItemRadioList(SolveDir(), CSysWinABCDSolver::CszSolveDirNames);
}


/*********************************************************
* SysWinPropItemCallback
* Callback from property manager when an item is modified
* The callback function must be declared static.
*********************************************************/
BOOL CSysWinABCDSolver::_SysWinPropItemCallback(void *vData, UINT uID, void *pVoid) {
   return((pVoid) ? ((CSysWinABCDSolver*)pVoid)->SysWinPropItemCallback(vData, uID) : TRUE);
}
//******************************************************//
BOOL CSysWinABCDSolver::SysWinPropItemCallback(void *vData, UINT uID) {
   double dVal;                             // double value
   int    iVal;                             // integer value
   switch(uID) {
   case CPS_SLVFCNTOL:  dVal =       *(double*) vData; UserSetFcnTol(dVal);   break;
   case CPS_SLVMAXITER: iVal = (int) *(double*) vData; UserSetMaxIter(iVal);  break;
   case CPS_SLVDIR:     iVal =       *(int*)    vData; UserSetSolveDir(iVal); break;
   }
   return(TRUE);
}

/*********************************************************
* Property Setters
* These set the properties and refresh the property mana-
* ger.
*********************************************************/
void CSysWinABCDSolver::UserSetFcnTol(double dTol) {
   if(dTol < 1.00e-16) dTol = 1.00e-16;
   ddProp[CSLVC_PROP_FCNTOL] = dTol;
   UpdateProperties();
   App()->PropManager()->OnPaint(TRUE);     // update values only
}
void CSysWinABCDSolver::UserSetMaxIter(int iMax) {
   if(iMax < 1) iMax = 1;
   ddProp[CSLVC_PROP_MAXITER] = (double) iMax;
   UpdateProperties();
   App()->PropManager()->OnPaint(TRUE);     // update values only
}
void CSysWinABCDSolver::UserSetSolveDir(int iDir) {
   if(iDir < 0) iDir = 0;
   if(iDir > 1) iDir = 1;
   ddProp[CSLVC_PROP_SOLVEDIR] = (double) iDir;
   UpdateProperties();
   App()->PropManager()->OnPaint(TRUE);     // update values only
}

/*########################################################
 ##
########################################################*/
/*********************************************************
* MenuCommand
* The application calls MenuCommand() of the current ren-
* derer for ANY renderer-specific command.
*********************************************************/
void CSysWinABCDSolver::MenuCommand(int iCmd) {
   switch(iCmd) {
   //case CMU_COPYDATA: CopyDataToClipboard(); break;
   }
}

/*********************************************************
* DebugPrint
*********************************************************/
void CSysWinABCDSolver::DebugPrint(char *psz, int *pInt) {
   for(int k=0; k<*pInt; k++) sprintf(psz+strlen(psz), "   ");
   sprintf(psz+strlen(psz), "CSysWin-ABCDSolver Renderer 0x%08lx\n", (long) this);
}



/*########################################################
 ## Mouse                                              ##
########################################################*/
/*********************************************************
* MouseCallback
* This callback function must be declared static.
*********************************************************/
void CSysWinABCDSolver::_MouseCallback(int iMsg, int x, int y, int wKeys, void *pVoid, long int lData) {
   if(pVoid) ((CSysWinABCDSolver*)pVoid)->MouseCallback(iMsg, x, y, wKeys, lData);
}
void CSysWinABCDSolver::MouseCallback(int iMsg, int x, int y, int wKeys, long int lData) {
UNREFERENCED_PARAMETER(x);
UNREFERENCED_PARAMETER(y);
UNREFERENCED_PARAMETER(wKeys);
UNREFERENCED_PARAMETER(lData);

   switch(iMsg) {
   }
}


/*########################################################
 ## Solution Search                                    ##
########################################################*/
/*********************************************************
* Solve
* This is it.
* It's a bit  optimistic to make a Solve()  function now,
* given  how much still needs to be done, but who  knows?
* Maybe it'll work out.
* This may want to run in a separate THREAD!
*********************************************************/
#define fnAppendVar(psz, pszTag, pszExt) {\
   sprintf((psz), "%s.%s", (pszTag), (pszExt));\
   psz += strlen(psz) + 1;\
}
//******************************************************//
void CSysWinABCDSolver::Solve(BOOL tfEval) {
   char      szEqString[4096];              // source equation string
   char      szBuf[256];                    // formatted / error string
   int       k;                             // loop counter
   int       iNumIter;                      // number of iterations
   CEquation Eq;                            // equation object
   CVertex  *pVx;                           // vertex loop counter
   int       iNumVx;                        // number of vertices
   int       iFcn;                          // function loop counter
   int       iVar;                          // variable loop counter
   int       iErrMsg             = 0;       // what went wrong
   char     *pszVariables        = NULL;    // variables string for CEquations
   char     *psz                 = NULL;    // pointer into variables


   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   // Get equation string
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   if(hwEditEqn==NULL) return;              // ignore if edit control not available
   SendMessage(hwEditEqn, WM_GETTEXT, (WPARAM) sizeof(szEqString)/sizeof(char), (LPARAM) szEqString);
   if(strlen(szEqString) < 1) return;       // ignore if no string to solve
   for(k=0; k<(int)strlen(szEqString); k++)      // replace carriage returns with spaces
      if((szEqString[k]=='\n') | (szEqString[k]=='\r')) szEqString[k] = ' ';

   do {                                     // structure to break out of if problems
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Assess Variables
      // The pszVariables buffer is arbitrarily somewhat-
      // oversized.
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      //===Prepare========================================
      //---Count vertices-----------------------
      for(iNumVx=0, pVx=pSystem->VxTop(); pVx; pVx=(CVertex*)pVx->Next()) iNumVx++;

      //---Allocate-----------------------------
      pszVariables = (char*) malloc(        // assign some buffer that's big enough
         (iNumVx * CSLVC_NUM_VXPROP + CSLVC_NUM_SYSPROP + App()->NumVars()) * (CVXC_MAXTAG+2)*sizeof(char));
      if(pszVariables==NULL) { iErrMsg = CSLVC_ERRALLOC; break; };  // fail on allocation failure
      memset(pszVariables, 0x00,            // clear the whole buffer
         (iNumVx * CSLVC_NUM_VXPROP + CSLVC_NUM_SYSPROP) * (CVXC_MAXTAG+2)*sizeof(char));

      //===Read names=====================================
      psz = pszVariables;                   // start filling at top

      //---Vertex properties--------------------
      for(pVx=pSystem->VxTop(); pVx; pVx=(CVertex*)pVx->Next()) {
         for(iFcn=0; iFcn<CSLVC_NUM_VXPROP; iFcn++) {
            sprintf(psz, "%s.%s", pVx->Tag(), CszSolverVariables[iFcn]);
            psz += strlen(psz) + 1;
         }
      }

      //---System properties--------------------
      for(iFcn=0; iFcn<CSLVC_NUM_SYSPROP; iFcn++) { // system variables at end
         sprintf(psz, "%s", CszSolverVariables[CSLVI_PROP_VXMAX+1+iFcn]);
         psz += strlen(psz) + 1;
      }

      //---Variables----------------------------
      for(iFcn=0; iFcn<App()->NumVars(); iFcn++) {
         sprintf(psz, "%s", App()->VarString(iFcn));
         psz += strlen(psz) + 1;
      }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Parse Equation
      // The strategy is to parse the equation once with ALL
      // available  variables, then pick out those  that are
      // used.
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      //---Parse initial------------------------
      if( Eq.ParseEquation(szEqString,  pszVariables) != EQERR_NONE ) { iErrMsg=CSLVC_ERRPARSE; break; };

      //---Count used variables-----------------
      iNumVarSlv = 0;                       // assume no variables used
      for(iVar=0; iVar<iNumVx*CSLVC_NUM_VXPROP+CSLVC_NUM_SYSPROP+App()->NumVars(); iVar++) {
         if(Eq.ContainsVariable(iVar)) iNumVarSlv++; // count number of used variables
      }
      if(iNumVarSlv==0) { iErrMsg = CSLVC_ERRNOVAR; break; }; // exit if no variables used

      //---Free existing------------------------
      if(pszVariables ) free(pszVariables ); pszVariables  = NULL; // don't need any more
      if(pVxArraySlv  ) free(pVxArraySlv  ); pVxArraySlv   = NULL; // free from previous run
      if(piFcnArraySlv) free(piFcnArraySlv); piFcnArraySlv = NULL;
      if(pdValArraySlv) free(pdValArraySlv); pdValArraySlv = NULL;

      //---Allocate new-------------------------
      pVxArraySlv     = (CVertex**) malloc(iNumVarSlv * sizeof(CVertex*)); // array of vertex pointers
      piFcnArraySlv   =      (int*) malloc(iNumVarSlv * sizeof(int));      // array of functions
      pdValArraySlv   =   (double*) malloc(iNumVarSlv * sizeof(double));   // array of values
      pszVariables    =     (char*) malloc(iNumVarSlv*(CVXC_MAXTAG+2)*sizeof(char)); // strings of variables
      if((pVxArraySlv==NULL) || (piFcnArraySlv==NULL) || (pdValArraySlv==NULL) || (pszVariables==NULL)) { iErrMsg=CSLVC_ERRALLOC; break; }
      memset(pszVariables, 0x00, iNumVarSlv*(CVXC_MAXTAG+2)*sizeof(char)); // clear the whole buffer

      //===Copy Variable Names============================
      // ensure this is SAME ORDER as above!
      psz        = pszVariables;            // start filling at top
      iVar       = 0;                       // count SOURCE variables
      iNumVarSlv = 0;                       // count DESTINATION variables

      //---Vertex properties--------------------
      for(pVx=pSystem->VxTop(); pVx; pVx=(CVertex*)pVx->Next()) {
         for(iFcn=0; iFcn<CSLVC_NUM_VXPROP; iFcn++, iVar++) {
            if(Eq.ContainsVariable(iVar)) {
               sprintf(psz, "%s.%s", pVx->Tag(), CszSolverVariables[iFcn]);
               psz += strlen(psz) + 1;      // add variable name to list
               pVxArraySlv[iNumVarSlv]   = pVx;  // point to vertex
               piFcnArraySlv[iNumVarSlv] = iFcn; // function to use
               iNumVarSlv++;                // count destination variables
            }
         }
      }

      //---System properties--------------------
      for(iFcn=0; iFcn<CSLVC_NUM_SYSPROP; iFcn++, iVar++) { // system variables at end
         if(Eq.ContainsVariable(iVar)) {
            sprintf(psz, "%s", CszSolverVariables[CSLVI_PROP_VXMAX+1+iFcn]);
            psz += strlen(psz) + 1;      // add variable name to list
            pVxArraySlv[iNumVarSlv]   = NULL; // point to system
            piFcnArraySlv[iNumVarSlv] = CSLVI_PROP_VXMAX+1+iFcn; // function to use
            iNumVarSlv++;                // count destination variables
         }
      }

      //---Application Variables----------------
      for(iFcn=0; iFcn<App()->NumVars(); iFcn++, iVar++) {
         if(Eq.ContainsVariable(iVar)) {
            sprintf(psz, "%s", App()->VarString(iFcn));
            psz += strlen(psz) + 1;
            pVxArraySlv[iNumVarSlv] = NULL; // no vertex
            piFcnArraySlv[iNumVarSlv] = CSLVI_PROP_APPVAR + iFcn; // variable number
            iNumVarSlv++;                   // count destination variables
         }
      }

      //===Parse new set==================================
      if( EqSolver.ParseEquation(szEqString, pszVariables) != EQERR_NONE ) { iErrMsg=CSLVC_ERRPARSE; break; }

      //===Current application variables==================
      for(k=0; k<App()->NumVars(); k++) dVarSolve[k] = App()->Var(k);

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Evaluate only
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      if(tfEval) {
         LoadString(App()->GetInstance(), SZMSG_SOLVEEVAL, szBuf, sizeof(szBuf)/sizeof(char));
         sprintf(szEqString, szBuf, EqSolveFcn(dVarSolve));
         MessageBox(hwRenderer, szEqString, "Solver", MB_OK | MB_ICONINFORMATION);

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Simplex Search
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      } else {
         //---Search-------------------------------
         // The X-tolerance isn't used, so it's 0.
         iNumIter = MaxIter();
         SimplexMinimise(CSysWinABCDSolver::_EqSolveFcn, dVarSolve, App()->NumVars(),
            &iNumIter, 0.00, FcnTol(), (void*) this);
      }

      //---Scan all at end----------------------
      for(k=0; k<App()->NumVars(); k++) App()->SetVar(k, dVarSolve[k]);
      App()->UpdateProperties();            // update variables display
      App()->ScanAll();                     // apply to all systems


   } while(0);                              // no repeats


   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   // Finalize
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   switch(iErrMsg) {
   case CSLVC_ERRPARSE:
      Eq.LastErrorMessage(szBuf, sizeof(szBuf)/sizeof(char), szEqString);
      MessageBox(hwRenderer, szBuf, "Solver Error", MB_OK | MB_ICONSTOP);
      break;
   case CSLVC_ERRALLOC:
   case CSLVC_ERRNOVAR:
      LoadString(App()->GetInstance(),
         (iErrMsg==CSLVC_ERRALLOC) ? SZERR_SOLV_ALLOC :
         (iErrMsg==CSLVC_ERRNOVAR) ? SZERR_SOLV_NOVAR :
         SZERR_SOLV_ALLOC, szBuf, sizeof(szBuf)/sizeof(char));
      MessageBox(hwRenderer, szBuf, "Solver Error", MB_OK | MB_ICONSTOP);
      break;
   }

   //---Free------------------------------------
   if(pszVariables ) free(pszVariables ); pszVariables  = NULL;
   if(pVxArraySlv  ) free(pVxArraySlv  ); pVxArraySlv   = NULL;
   if(piFcnArraySlv) free(piFcnArraySlv); piFcnArraySlv = NULL;
   if(pdValArraySlv) free(pdValArraySlv); pdValArraySlv = NULL;
}


/*********************************************************
* EqSolveFcn
* The function  to be  minimized by  SimplexMinimize. The
* prototype is dictated by the minimization algorithm and
* includes a pointer to  CSysWinABCDSolver as the aux va-
* riable, since  this is enough information to  solve the
* specified equation and get to the Application for vari-
* able values.
* The referenced function must be declared static.
*********************************************************/
//double SimplexMinimise(double(*)(double [],void*),double[],int,int*,double,double,void*);
double CSysWinABCDSolver::_EqSolveFcn(double ddVar[], void *pVoid) {
   if(pVoid) return(((CSysWinABCDSolver*)pVoid)->EqSolveFcn(ddVar));
   else return(0.00);
}
//******************************************************//
double CSysWinABCDSolver::EqSolveFcn(double ddVar[]) {
   int    k;                                // equation variable loop counter

   //===Apply Simplex Values==============================
   for(k=0; k<App()->NumVars(); k++) App()->SetVar(k, ddVar[k]);
   App()->ScanAll();                        // apply to all systems

   //===Read Variables====================================
   for(k=0; k<iNumVarSlv; k++) {
      switch(piFcnArraySlv[k]) {
         case CSLVI_PROP_VXWSAG:     pVxArraySlv[k]->FcnValue(CVXI_PROP_MODESAG , &pdValArraySlv[k], NULL); break;
         case CSLVI_PROP_VXRSAG:     pVxArraySlv[k]->FcnValue(CVXI_PROP_CURVSAG , &pdValArraySlv[k], NULL); break;
         case CSLVI_PROP_VXW0SAG:    pVxArraySlv[k]->FcnValue(CVXI_PROP_WAISTSAG, &pdValArraySlv[k], NULL); break;
         case CSLVI_PROP_VXZ0SAG:    pVxArraySlv[k]->FcnValue(CVXI_PROP_WDISTSAG, &pdValArraySlv[k], NULL); break;
         case CSLVI_PROP_VXZRSAG:    pVxArraySlv[k]->FcnValue(CVXI_PROP_ZRSAG   , &pdValArraySlv[k], NULL); break;
         case CSLVI_PROP_VXWTAN:     pVxArraySlv[k]->FcnValue(CVXI_PROP_MODESAG , NULL, &pdValArraySlv[k]); break;
         case CSLVI_PROP_VXRTAN:     pVxArraySlv[k]->FcnValue(CVXI_PROP_CURVSAG , NULL, &pdValArraySlv[k]); break;
         case CSLVI_PROP_VXW0TAN:    pVxArraySlv[k]->FcnValue(CVXI_PROP_WAISTSAG, NULL, &pdValArraySlv[k]); break;
         case CSLVI_PROP_VXZ0TAN:    pVxArraySlv[k]->FcnValue(CVXI_PROP_WDISTSAG, NULL, &pdValArraySlv[k]); break;
         case CSLVI_PROP_VXZRTAN:    pVxArraySlv[k]->FcnValue(CVXI_PROP_ZRSAG   , NULL, &pdValArraySlv[k]); break;
         case CSLVI_PROP_VXASTIG:    pVxArraySlv[k]->FcnValue(CVXI_PROP_ASTIG   , &pdValArraySlv[k], NULL); break;
         case CSLVI_PROP_SYSOPTLEN:  pSystem->FcnValue(CSYSI_PROP_OPTLEN , &pdValArraySlv[k], NULL); break;
         case CSLVI_PROP_SYSMODESP:  pSystem->FcnValue(CSYSI_PROP_MODESPACING, &pdValArraySlv[k], NULL); break;
         case CSLVI_PROP_SYSSTABSAG: pSystem->FcnValue(CSYSI_PROP_STABSAG, &pdValArraySlv[k], NULL); break;
         case CSLVI_PROP_SYSSTABTAN: pSystem->FcnValue(CSYSI_PROP_STABSAG, NULL, &pdValArraySlv[k]); break;
         default:                    pdValArraySlv[k] = App()->Var(piFcnArraySlv[k] - CSLVI_PROP_APPVAR); break;
      }
   }

   //---Mask Stability--------------------------
   for(k=0; k<iNumVarSlv; k++) {
      switch(piFcnArraySlv[k]) {
         case CSLVI_PROP_VXWSAG:
         case CSLVI_PROP_VXRSAG:
         case CSLVI_PROP_VXW0SAG:
         case CSLVI_PROP_VXZ0SAG:
         case CSLVI_PROP_VXZRSAG:
            if(!pSystem->StableABCD(SAG)) pdValArraySlv[k] = (SolveDir()==CSLVI_SOLVE_MIN) ? CSLV_HUGE_VAL : -CSLV_HUGE_VAL;
            break;
         case CSLVI_PROP_VXWTAN:
         case CSLVI_PROP_VXRTAN:
         case CSLVI_PROP_VXW0TAN:
         case CSLVI_PROP_VXZ0TAN:
         case CSLVI_PROP_VXZRTAN:
            if(!pSystem->StableABCD(TAN)) pdValArraySlv[k] = (SolveDir()==CSLVI_SOLVE_MIN) ? CSLV_HUGE_VAL : -CSLV_HUGE_VAL;
            break;
         case CSLVI_PROP_VXASTIG:
            if( (!pSystem->StableABCD(SAG)) || (!pSystem->StableABCD(TAN)) )
               pdValArraySlv[k] = (SolveDir()==CSLVI_SOLVE_MIN) ? CSLV_HUGE_VAL : -CSLV_HUGE_VAL;
            break;
      }
   }



   //===Evaluate Equation=================================
   switch(SolveDir()) {
   case CSLVI_SOLVE_MIN: return(+EqSolver.Answer(pdValArraySlv));
   case CSLVI_SOLVE_MAX: return(-EqSolver.Answer(pdValArraySlv));
   default:              return(+EqSolver.Answer(pdValArraySlv));
   }
}


/*********************************************************
* Restore
*********************************************************/
void CSysWinABCDSolver::Restore(void) {
   int k;                                   // loop counter
   for(k=0; k<App()->NumVars(); k++) App()->SetVar(k, dVarRestore[k]);
   App()->UpdateProperties();               // update variables display
   App()->ScanAll();                        // apply to all systems
}

/*########################################################
 ## Window Methods                                     ##
########################################################*/
/*********************************************************
* WndProcSysWinABCDSolver
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
*********************************************************/
LRESULT CALLBACK CSysWinABCDSolver::WndProcSysWinABCDSolver(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
   CSysWinABCDSolver *pSWin;                 // owning object
   pSWin = (CSysWinABCDSolver*) GetWindowLong(hWnd, 0); // not valid on WM_CREATE
   switch(uMsg) {
   case WM_CREATE:
      pSWin = (CSysWinABCDSolver*) ((LPMDICREATESTRUCT) (((LPCREATESTRUCT)lParam)->lpCreateParams))->lParam;
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

   case WM_SIZE:  pSWin->OnResize(); return(DefMDIChildProc(hWnd, uMsg, wParam, lParam));
   case WM_PAINT: pSWin->OnPaint(); break;

   case WM_MOUSEMOVE:
   case WM_LBUTTONDOWN:
   case WM_LBUTTONUP:
      pSWin->Mouse.MouseProc(hWnd, uMsg, LOWORD(lParam), HIWORD(lParam));
      break;

   case WM_COMMAND:
      switch(LOWORD(wParam)) {
      case CMU_BUTTON_SOLVE:   pSWin->Solve(FALSE); break;
      case CMU_BUTTON_EVAL:    pSWin->Solve(TRUE);  break;
      case CMU_BUTTON_RESTORE: pSWin->Restore();    break;
      }
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
void CSysWinABCDSolver::OnResize(void) {
   RECT rc, rcClient;                       // client rectangle
   GetClientRect(hwRenderer, &rcClient);    // read full window rectangle
   //---Edit control------------------
   if(hwEditEqn) {
      SetRect(&rc, rcClient.left+16, rcClient.top+16, rcClient.right-16, rcClient.bottom-48);
      MoveWindow(hwEditEqn, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, TRUE);
   }
   //---Solve button------------------
   if(hwButSolve) {
      SetRect(&rc, rcClient.right-280, rcClient.bottom-38, rcClient.right-200, rcClient.bottom-16);
      MoveWindow(hwButSolve, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, TRUE);
   }
   //---Evaluate button---------------
   if(hwButEval) {
      SetRect(&rc, rcClient.right-188, rcClient.bottom-38, rcClient.right-108, rcClient.bottom-16);
      MoveWindow(hwButEval, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, TRUE);
   }
   //---Restore button----------------
   if(hwButRestore) {
      SetRect(&rc, rcClient.right-96, rcClient.bottom-38, rcClient.right-16, rcClient.bottom-16);
      MoveWindow(hwButRestore, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, TRUE);
   }
}




/*********************************************************
* Print
*********************************************************/
void CSysWinABCDSolver::Print(HDC hdcPrint) {
UNREFERENCED_PARAMETER(hdcPrint);
   return;
}

/*********************************************************
* Paint
*********************************************************/
void CSysWinABCDSolver::OnPaint(void) {
   char szBuf[256];                         // text buffer
   PAINTSTRUCT ps;                          // painting structure
   HDC      hdc;                            // device context to use
   HPEN     hpOld;                          // restore pen
   HFONT    hfOld;                          // restore font

   //===Preliminaries=====================================
   BeginPaint(hwRenderer, &ps);
   hdc = ps.hdc;
   hpOld =  (HPEN) SelectObject(hdc, App()->Pen(CAPPI_PEN3DDARK));
   hfOld = (HFONT) SelectObject(hdc, App()->Font());

   //===Finalize==========================================
//Mouse.PaintAllActiveRect(hdc);
   SelectObject(hdc, hpOld);                // restore pen
   SelectObject(hdc, hfOld);                // restore font
   EndPaint(hwRenderer, &ps);
}
