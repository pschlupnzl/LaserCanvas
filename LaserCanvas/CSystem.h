/*********************************************************
* CSystem.h
* $PSchlup 2006 $     $Revision 5 $
*********************************************************/
#ifndef CSYSTEM_H                           // prevent multiple includes
#define CSYSTEM_H
//===Classes defined======================================
class CSystem;                              // optical system class

//===Includes=============================================
#include <windows.h>                        // standard Windows include
#include <stdio.h>                          // standard header

#include "CApplication.h"                   // application class
#include "CLCEqtn.h"                        // equation class
#include "CVertex.h"                        // vertex classes
#include "CSysWin.h"                        // system renderers
#include "CAbcd.h"                          // ABCD solver and classes
#include "CMouse.h"                         // mouse class

//===Constants============================================
#define CSYSC_MAXTAG            31          // system tag length
#define CSYS_TYPE_PROP           0          // (index!) propagation system
#define CSYS_TYPE_RESO           1          // (index!) standing wave resonator
#define CSYS_TYPE_SPAWN          2          // (index) spawned propagation system
#define CSYS_TYPE_MAX            2          // highest system type index

//---Auxiliary saved parameters-----------------
#define CSYSI_SAVE_VARVALUE      0          // (index) variable values
#define CSYSI_SAVE_VARRANGE      1          // (index) variable range
#define CSYSC_NUM_SAVE           2          // (count) number of variable saves

#define CSYSI_AUXSAVE_INPUT      0          // (index) type of input
#define CSYSI_AUXSAVE_MSQASYM    1          // (index) asymmetric M^2
#define CSYSI_AUXSAVE_DRAFTMODE  2          // (index) system is in draft mode
#define CSYSC_NUM_AUXSAVE        3          // (count) number of auxiliary saves

//---Properties---------------------------------
// Properties are not written to file
// The plottable properties are listed first! - Names listed in the RC file
// Sagittal values refer to sag/tan value pairs for plotting
#define CSYSI_PROP_PHYSLEN       0          // (index) [mm] physical length
#define CSYSI_PROP_OPTLEN        1          // (index) [mm] optical length
#define CSYSI_PROP_MODESPACING   2          // (index) [MHz] mode spacing
#define CSYSI_PROP_STABSAG       3          // (index) stability parameter sagittal
//   .   .   .   .   .   .   .  |||  .   .   .   .   .
#define CSYSI_PROP_FCNMAX        3          // (index) highest allowed graphing function
//   .   .   .   .   .   .   .  \|/  .   .   .   .   .
#define CSYSI_PROP_STABTAN       4          // (index) stability parameter tangential
#define CSYSI_PROP_EQTN          5          // (index) start of equation mapped parameters
//   .   .   .   .   .   .   .  |||  .   .   .   .   .
#define CSYSI_PROP_WAVLEN        5          // (index) (CSYSI_EQTN_WAVLEN)
#define CSYSI_PROP_MSQUARED      6          // (index) (CSYSI_EQTN_MSQUARED)
#define CSYSI_PROP_MSQUAREDTAN   7          // (index) (CSYSI_EQTN_MSQUAREDTAN)
#define CSYSI_PROP_INPUTWSAG     8          // (index) (CSYSI_EQTN_INPUTWSAG)
#define CSYSI_PROP_INPUTWTAN     9          // (index) (CSYSI_EQTN_INPUTWTAN)
#define CSYSI_PROP_INPUTRZSAG   10          // (index) (CSYSI_EQTN_INPUTRZSAG)
#define CSYSI_PROP_INPUTRZTAN   11          // (index) (CSYSI_EQTN_INPUTRZTAN)
#define CSYSI_PROP_ROTATION     12          // (index) (CSYSI_EQTN_ROTATION)
#define CSYSI_PROP_STARTX       13          // (index) (CSYSI_EQTN_STARTX)
#define CSYSI_PROP_STARTY       14          // (index) (CSYSI_EQTN_STARTY)
//   .   .   .   .   .   .   .  \|/  .   .   .   .   .
#define CSYSI_PROP_FLAGS        15          // (index) [int] flags (was INPUTPARAM)
#define CSYSI_NUM_PROP          16          // (count) number of properties

//---Equations----------------------------------
#define CSYSI_EQTN_WAVLEN        0          // (index) [nm] wavelength
#define CSYSI_EQTN_MSQUARED      1          // (index) beam M^2 parameter
#define CSYSI_EQTN_MSQUAREDTAN   2
#define CSYSI_EQTN_INPUTWSAG     3          // (index) input sagittal waist or spot
#define CSYSI_EQTN_INPUTWTAN     4          // (index) input tangential waist or spot
#define CSYSI_EQTN_INPUTRZSAG    5          // (index) input sagittal dist or curv
#define CSYSI_EQTN_INPUTRZTAN    6          // (index) input tangential dist or curv
//   .   .   .   .   .   .   .  \|/  .   .   .   .   .
#define CSYSI_EQTN_COMMON        7          // (indexer) following are available to all systems
//   .   .   .   .   .   .   .  |||  .   .   .   .   .
#define CSYSI_EQTN_ROTATION      7          // (index) system rotation
#define CSYSI_EQTN_STARTX        8          // (index) system start position
#define CSYSI_EQTN_STARTY        9          // (index) system start position
#define CSYSI_NUM_EQTN          10          // (count) number of equations

//---Flags--------------------------------------
//previously: CSYSI_INPUT_W0Z0         0          // waist and distance
//previously: CSYSI_INPUT_WR           1          // radius and curvature
#define CSYSF_INPUTW0Z0     0x0000          // (bit-INDEX!) input as w0 / z0
#define CSYSF_INPUTWR       0x0001          // (bit-INDEX!) input as w / R
#define CSYSF_INPUTPARAM    0x0001          // (mask) input type
#define CSYSF_MSQASYM       0x0010          // (bit) asymmetric M^2 parameters
#define CSYSF_LINKMODE      0x4000          // (bit) linking mode in draft mode
#define CSYSF_DRAFTMODE     0x8000          // (bit) system is in draft mode

//===Class================================================
class CSystem {
private:
   CApplication *pAppParent;                // parent application
   char          szSysTag[CSYSC_MAXTAG+1];  // system tag
   char         *pszFullFile;               // path and name of file on disk
   char         *pszFileName;               // name / title of file
   CVertex      *pVxTop;                    // top of vertex chain
   CVertex      *pVxSpawn;                  // spawning (parent) vertex
   CSysWin      *pSysWinTop;                // top of renderer chain
   int           iSysType;                  // type of system
   CEquation     EqSys[CSYSI_NUM_EQTN];     // system parameter equations
   CSystem      *pSysNext;                  // next system in chain (NOT maintained by CSystem)
   int           iOpticNumber;              // counter for optic labeling
   static int    iUntitledCounter;          // untitled system name counter

public:///TODO: make private
   double        ddSysProp[CSYSI_NUM_PROP]; // system properties

   //---ABCD------------------------------------
public:///TODO: Make private
   CRecQ         QSysInit[SAGTAN];          // system's initial Q value
   CMatrix2x2    MxAbcdSys[SAGTAN];         // system's ABCD matrices (e.g. for display)
   BOOL          tfStableAbcd[SAGTAN];      // system is stable for each plane

   //---Dragging--------------------------------
private:
   CVertex      *pVxDrag;                   // vx currently being dragged
   CVertex      *pVxAncPrev;                // previous anchor (rotation)
   CVertex      *pVxStrPrev;                // previous stretch segment
   CVertex      *pVxAncNext;                // next anchor (rotation)
   CVertex      *pVxStrNext;                // next stretch segment
   double        dEpR, dEpS, dEpF, dEpP, dEpK; // previous chain angles
   double        dEnR, dEnS, dEnF, dEnP, dEnK; // next chain angles
   double        dEaR, dEaA;                // distance between anchors
public:
   //---Dragging--------------------------------
   CVertex*      VxDrag(void)               { return(pVxDrag); };
   CVertex*      VxAncPrev(void)            { return(pVxAncPrev); };
   CVertex*      VxStrPrev(void)            { return(pVxStrPrev); };
   CVertex*      VxAncNext(void)            { return(pVxAncNext); };
   CVertex*      VxStrNext(void)            { return(pVxStrNext); };
   void          SetVxDrag(CVertex *pVx)    { pVxDrag = pVx; };
   void          SetVxAncPrev(CVertex *pVx) { pVxAncPrev = pVx; };
   void          SetVxStrPrev(CVertex *pVx) { pVxStrPrev = pVx; };
   void          SetVxAncNext(CVertex *pVx) { pVxAncNext = pVx; };
   void          SetVxStrNext(CVertex *pVx) { pVxStrNext = pVx; };

   double        EpR(void)             { return(dEpR); };
   double        EpS(void)             { return(dEpS); };
   double        EpF(void)             { return(dEpF); };
   double        EpP(void)             { return(dEpP); };
   double        EpK(void)             { return(dEpK); };
   double        EnR(void)             { return(dEnR); };
   double        EnS(void)             { return(dEnS); };
   double        EnF(void)             { return(dEnF); };
   double        EnP(void)             { return(dEnP); };
   double        EnK(void)             { return(dEnK); };
   double        EaR(void)             { return(dEaR); };
   double        EaA(void)             { return(dEaA); };
   void          SetEp(double R, double S, double F, double P, double K) { dEpR = R; dEpS = S; dEpF = F; dEpP = P; dEpK = K; };
   void          SetEn(double R, double S, double F, double P, double K) { dEnR = R; dEnS = S; dEnF = F; dEnP = P; dEnK = K; };
   void          SetEa(double R, double A) { dEaR = R; dEaA = A; };

public:
   CSystem(CApplication *pApp);             // constructor
   ~CSystem();                              // destructor

   int      NextOpticNumber(void);          // returns counting number of next counted optic (top level only)
   void     PrintAllRenderer(HDC hdcPrint, int *piPage); // recursively print all (allowed) renderer windows

   //---User functions--------------------------
   BOOL     UserCloseSystem(void);          // close this system
   CVertex* MenuInsertVx(int iTyp);         // menu command insert
   void     MenuDeleteVx(void);             // menu command delete
   CSystem* MenuInsertSpawn(void);          // insert output coupled system(s)

   //---Vertex chain----------------------------
   //BOOL     UserDeleteVx(CVertex *pVxDel);  // request delete given vx
   void     UserCreateSystem(int iTyp);     // create a default system type
   CVertex* CreateVxAfter(CVertex *pVxAft, int iTyp); // insert a vertex after given one
   void     DeleteVx(CVertex *pVxDel);      // delete vx, maintain chain
   void     SelectAllVx(BOOL tfSel);        // select or de-select all vertices
   void     SelectVxByRect(double dXMin, double dYMin, double dXMax, double dYMax, BOOL tfInvert);
   int      NumSelectedVx(CVertex **pVx);   // number of vertices selected
   CVertex* NextSelectedVx(CVertex *pVx);   // walk through selection
   BOOL     LinkNextVx(CVertex *pVx);       // concatenate in Draft mode
   void     UnlinkVx(CVertex *pVx);         // unlink in Draft mode

   //---Physics!--------------------------------
   void     ApplyEquations(double *pcdVar); // apply equation values to prepare for solving the system
   void     PlaceCanvasVertices(BOOL tfRecursive); // calculate canvas positions for vxs
   void     GraphRendererPoint(int iRVar, int iPt); // set points for graph renderers
   void     RefreshRenderers(BOOL tfRecursive); // update all the renderers
   void     SolveSystemABCD(void);          // solve the given system
   void     SetQSysInit(const CRecQ *pQSag, const CRecQ *pQTan);
   void     SetStableABCD(BOOL tfSag, BOOL tfTan) { tfStableAbcd[SAG] = tfSag; tfStableAbcd[TAN] = tfTan; };
   BOOL     StableABCD(int k)          { return(((k>=0)&&(k<SAGTAN)) ? tfStableAbcd[k] : FALSE); };

   //---Data files------------------------------
   CVertex* FindVxByTag(const char *pszTag); // find Vx by its tag
   CSystem* FindSysByTag(const char *pszTag); // find system by its tag
   void     SaveSystem(HANDLE hFile);       // save top or spawned systems
   BOOL     LoadSystem(const char *pszDataFile, const char *szSysName); // load top or spawned system from buffer
   void     SetFileName(const char *pszPath, const char *pszName); // set the current file name
   void     SetDefaultName(void);           // call for new systems for 'Untitled1' names
   BOOL     GetFullFile(char *pszBuf, size_t len); // retrieve current path and file name
   const char* FileName(void);              // retrieve name only

   //---Property Manager------------------------
   void     PrepareProperties(BOOL tfAct);  // show or hide system properties
   void     UpdateProperties(void);         // update property values
   void     PrepareVxProperties(BOOL tfAct); // prepare selection properties
   void     UpdateVxProperties(void);       // update selection properties

   //---Renderer Chain--------------------------
   CSysWin* UserCreateSysWin1d(void);       // add a 1d renderer to chain
   CSysWin* UserCreateSysWin2d(void);       // add a 2d renderer to chain
   CSysWin* UserCreateSysWin3d(void);       // add a 3d renderer to chain
   CSysWin* UserCreateSysWinGraph(void);    // add a system graph renderer to chain
   CSysWin* UserCreateSysWinVxGraph(BOOL tfSel);  // add VxGraph to renderer chain
   CSysWin* UserCreateSysWinInventory(void); // add inventory renderer to chain
   CSysWin* UserCreateSysWinABCDSolver(void); // add ABCD solver to chain
   void     UserDeleteSysWin(CSysWin *pSWin); // controlled removal of a renderer
   void     AppendSysWin(CSysWin *pSWinApp); // append a (new) CSysWin to the end of the chain
   void     DeleteSysWin(CSysWin *pSWin);   // remove renderer from chain
   HWND     CreateSysWinWindow(const char *pszClassName, LPVOID lpVoid, UINT uStyle=0x0000); // create an MDI client window in parent app
   void     DestroySysWinWindow(HWND hWnd); // destroy MDI client window in parent app
   void     UpdateAllRendererTitle(void);   // update all titles, e.g. on name change

   //---Flags-----------------------------------
   UINT        CheckBit(UINT u)        { return( ((UINT)ddSysProp[CSYSI_PROP_FLAGS]) & u ); };
   void        SetBit(UINT u)          { ddSysProp[CSYSI_PROP_FLAGS] = (double) (((UINT) ddSysProp[CSYSI_PROP_FLAGS]) | u); };
   void        ClearBit(UINT u)        { ddSysProp[CSYSI_PROP_FLAGS] = (double) (((UINT) ddSysProp[CSYSI_PROP_FLAGS]) &~u); };
   void        ToggleBit(UINT u)       { ddSysProp[CSYSI_PROP_FLAGS] = (double) (((UINT) ddSysProp[CSYSI_PROP_FLAGS]) ^ u); };

   //---Equations-------------------------------
   // Additionally, these are indirected to the top-level system
   double   WLen(void);
   void     SetWLen(double d);
   double   MSquared(int iPln);
   void     SetMSquared(int iPln, double d);

   //---Properties------------------------------
   double   PhysicalLength(void)       { return(ddSysProp[CSYSI_PROP_PHYSLEN]); };
   double   OpticalLength(void)        { return(ddSysProp[CSYSI_PROP_OPTLEN ]); };
   double   Rotation(void)             { return(ddSysProp[CSYSI_PROP_ROTATION]); };
   double   StartX(void)               { return(ddSysProp[CSYSI_PROP_STARTX]); };
   double   StartY(void)               { return(ddSysProp[CSYSI_PROP_STARTY]); };
   double   ModeSpacing(void)          { return(ddSysProp[CSYSI_PROP_MODESPACING]); };
   void     SetRotation(double dRot);       // set rotation for this system
   void     SetStartPos(double dX, double Y); // set STARTX and STARTY
   int      InputParam(void)           { return(CheckBit(CSYSF_INPUTPARAM)); };
   void     UserSetInputParam(int iTyp);    // set input type
   BOOL     MSqAsymmetric(void);            // asymmetric MSquared (indirected)
   void     SetMSqAsymmetric(BOOL tf);      // set asymmetric MSquared (indirected)

   //---Access----------------------------------
   CApplication* App(void)             { return(pAppParent); };
   CSystem*    Next(void)              { return(pSysNext); };
   void        SetNext(CSystem *pSys)  { pSysNext = pSys; };
   void        UserSetTag(const char *psz); // update tag and windows' titles
   BOOL        MatchTag(const char *psz){ return( (strncmp(szSysTag, psz, strlen(szSysTag))==0) ? TRUE : FALSE); };
   const char* Tag(void)               { return( (const char*) szSysTag ); };
   int         SysType(void)           { return(iSysType); };
   void        SetSysType(int iTyp)    { iSysType = iTyp; };
   CEquation*  SysEquation(int k)      { return(((k>=0)&&(k<CSYSI_NUM_EQTN)) ? &EqSys[k] : NULL); };
   CVertex*    VxTop(void)             { return(pVxTop); };
   CVertex*    VxSpawn(void)           { return(pVxSpawn); };
   void        SetVxSpawn(CVertex *pVx){ pVxSpawn = pVx; };
   CSystem*    GetParentSystem(void);       // get parent system (or NULL)
   CSystem*    GetTopSystem(void);          // gets topmost system
   static int         NumSysFcn(void)  { return(CSYSI_PROP_FCNMAX+1); };
   static const char* SysFcnString(int iIndx); // return plot function string (CszSysGraphProperties)
   void        FcnValue(int iPltFcn, double *pdSag, double *pdTan); // return given function value(s)
   BOOL        DraftMode(void)         { return(CheckBit(CSYSF_DRAFTMODE) ? TRUE : FALSE); };
   void        UserSetDraftMode(BOOL tfDraft); // set or clear draft mode
   void        SetDraftMode(BOOL tfDft); // set flag, set/clear menu check

   //---Callback--------------------------------
   static BOOL _SysPropItemCallback(void *vData, UINT uID, void *pVoid); // callback from property manager for system items
   BOOL         SysPropItemCallback(void *vData, UINT uID);
   static BOOL _VxPropItemCallback(void *vData, UINT uID, void *pVoid); // callback from property manager for Vx items
   BOOL         VxPropItemCallback(void *vData, UINT uID);

   //---Debug-----------------------------------
   void     DebugPrint(char *psz, int *pInt); // print info about the system

   //---String lists----------------------------
   static const char* CszSysType[CSYS_TYPE_MAX+1]; // saved file header types
   static const char* CszVarFormat[CSYSC_NUM_SAVE]; // format for saving variables in file
   static const char* CszSysEquationName[CSYSI_NUM_EQTN]; // equation names
   static const char* CszSysAuxSaveName[CSYSC_NUM_AUXSAVE]; // auxiliary save
   static const char* CszSysSaveFcnString[CSYSI_PROP_FCNMAX+1]; // plotting function names in files
   static const char* CszSysFcnNames;       // plottable functions for CSYSI_PROP
   static const char* CszSysInputParam;     // names for input parameter specification for (int) ddProp(CSYSI_PROP_INPUTPARM)
   static const char* CszValidNameChar;     // allowed characters in system and vertex names

   static const UINT CuSysPropertiesProp[];
   static const UINT CuSysPropertiesPropInitwR[];
   static const UINT CuSysPropertiesPropInitw0z0[] ;
   static const UINT CuSysPropertiesSpawn[];
   static const UINT CuSysPropDraftHide[];
   static const UINT CuSysPropertiesReso[];
   static const UINT CuVxProperties[];
   static const UINT CuVxPropDraftHide[];
};
#endif//CSYSTEM_H
