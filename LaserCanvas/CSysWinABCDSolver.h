/*********************************************************
* CSysWinABCDSolver
* $PSchlup 2006 $     $Revision 0 $
*********************************************************/
#ifndef CSYSWINABCDSOLVER_H                 // prevent multiple includes
#define CSYSWINABCDSOLVER_H

//===Classes Defined======================================
class CSysWinABCDSolver;                    // solver system renderer

//===Included Files=======================================
#include <windows.h>                        // standard Windows include
#include <stdio.h>                          // standard header

#include "CApplication.h"                   // for variable access
#include "CSysWin.h"                        // base class header
#include "CSystem.h"                        // system class
#include "CMouse.h"                         // mouse layer class
#include "CLCEqtn.h"                        // equations

//===Constants============================================
#define CSLV_HUGE_VAL         1e12          // some big number
//---Variables------------------------
#define CSLVI_PROP_VXWSAG        0          // (index) sagittal mode size
#define CSLVI_PROP_VXRSAG        1          // (index) sagittal radius of curvature
#define CSLVI_PROP_VXW0SAG       2          // (index) sagittal waist
#define CSLVI_PROP_VXZ0SAG       3          // (index) sagittal waist distance
#define CSLVI_PROP_VXZRSAG       4          // (index) sagittal Rayleigh length
#define CSLVI_PROP_VXWTAN        5          // (index) tangential mode size
#define CSLVI_PROP_VXRTAN        6          // (index) tangential radius of curvature
#define CSLVI_PROP_VXW0TAN       7          // (index) tangential waist
#define CSLVI_PROP_VXZ0TAN       8          // (index) tangential waist distance
#define CSLVI_PROP_VXZRTAN       9          // (index) tangential Rayleigh length
#define CSLVI_PROP_VXASTIG      10          // (index) astigmatims
//  .   .   .   .   .   .   .  |||   .   .   .   .
#define CSLVI_PROP_VXMAX        10          // (index) highest vertex property index
#define CSLVC_NUM_VXPROP        11          // (count) number of vertex properties
//  .   .   .   .   .   .   .  |||   .   .   .   .
#define CSLVI_PROP_SYSOPTLEN    11          // (index) system optical length
#define CSLVI_PROP_SYSMODESP    12          // (index) cavity mode spacing
#define CSLVI_PROP_SYSSTABSAG   13          // (index) sagittal system stability parameter
#define CSLVI_PROP_SYSSTABTAN   14          // (index) tangential system stability parameter
//  .   .   .   .   .   .   .  |||   .   .   .   .
#define CSLVI_PROP_SYSMAX       14          // (index) highest parameter
#define CSLVI_PROP_APPVAR       15          // (index) start of variable names
#define CSLVC_NUM_SYSPROP        4          // (count) number of system properties

//---Errors---------------------------
#define CSLVC_ERRALLOC           1          // allocation problem
#define CSLVC_ERRPARSE           2          // equation parse error
#define CSLVC_ERRNOVAR           3          // no variables used

//---Properties-----------------------
#define CSLVC_PROP_FCNTOL       0           // (index) solver function tolerance
#define CSLVC_PROP_MAXITER      1           // (index) (int) maximum iterations
#define CSLVC_PROP_SOLVEDIR     2           // (index) (int) solve min or max
#define CSLVC_NUM_PROP          3           // (count) number of properties

#define CSLVI_SOLVE_MIN         0           // (index) solve for minimum
#define CSLVI_SOLVE_MAX         1           // (index) solve for maximum

//---Auxiliary saved parameters-----------------
#define CSLVI_SAVE_EQNSTRING     0          // (index) equation string
#define CSLVC_NUM_SAVE           1          // (count) number of auxiliary saves


/*********************************************************
* CSysWinABCDSolver Class
*********************************************************/
class CSysWinABCDSolver : public CSysWin {
private:
   double    ddProp[CSLVC_NUM_PROP];        // min, max of selected zoom area
   CMouse    Mouse;                         // mouse and its actives
   HWND      hwEditEqn;                     // equation edit field
   HWND      hwButSolve;                    // solver button
   HWND      hwButEval;                     // evaluate button
   HWND      hwButRestore;                  // restore button
   //---Solver------------------------
   double    dVarSolve[CAPP_NUMVAR];        // solver variable values
   double    dVarRestore[CAPP_NUMVAR];      // restore previous values
   int       iNumVarSlv;                    // number of variables USED
   CVertex **pVxArraySlv;                   // vertices used in solver
   int      *piFcnArraySlv;                 // functions used in solver
   double   *pdValArraySlv;                 // values of variables
   CEquation EqSolver;                      // solver equation

public:
   CSysWinABCDSolver(CSystem *pSys);        // constructor
   ~CSysWinABCDSolver();                    // overloaded destructor

   void    Solve(BOOL tfEval);              // try to solve the system
   void    Restore(void);                   // restore previous variables
   void    OnPaint(void);                   // paint the window
   void    OnResize(void);                  // resize function

   static double _EqSolveFcn(double ddVar[], void *pVoid);
   double  EqSolveFcn(double ddVar[]);      // function to be minimized

   //---Overloaded functions--------------------
   int     Type(void)                  { return(CSYSWINI_TYPE_ABCDSOLVER); };
   void    Refresh(void);                   // refresh the renderer
   void    UpdateTitle(int iID);            // update title bar
   void    SaveSysWin(HANDLE hFile);        // save current renderer
   BOOL    LoadSysWin(const char *pszDataFile, char *pszMin, char *pszMax); // load renderer from file
   void    GraphPoint(int iRVar, int iPt);  // update a graphing point
   void    MenuCommand(int iCmd);           // process specific commands
   void    DebugPrint(char *psz, int *pInt); // debug text
   void    Print(HDC hdcPrint);             // print to printer device context
   static LRESULT CALLBACK WndProcSysWinABCDSolver(HWND, UINT, WPARAM, LPARAM); // window procedure

   //---Properties------------------------------
   void    PrepareProperties(BOOL tfAct);   // show / hide properties
   void    UpdateProperties(void);          // update properties
   BOOL    SysWinPropItemCallback(void *vData, UINT uID); // modified from property manager
   static BOOL _SysWinPropItemCallback(void *vData, UINT uID, void *pVoid); // callback from property manager

   //---Access----------------------------------
   double  FcnTol(void)                { return(ddProp[CSLVC_PROP_FCNTOL]); };
   void    UserSetFcnTol(double dTol);      // set tolerance, refresh
   int     MaxIter(void)               { return((int) ddProp[CSLVC_PROP_MAXITER]); };
   void    UserSetMaxIter(int iMax);        // set maximum iterations, refresh
   int     SolveDir(void)              { return((int) ddProp[CSLVC_PROP_SOLVEDIR]); };
   void    UserSetSolveDir(int iDir);       // set solve direction, refresh prop manager

   //---Mouse-----------------------------------
   static void _MouseCallback(int iMsg, int x, int y, int wKeys, void *pData, long int lData);
   void MouseCallback(int iMsg, int x, int y, int wKeys, long int lData);

   //---Tables----------------------------------
   static const char* CszSysWinABCDSolverSaveProp[CSLVC_NUM_PROP]; // save all properties
   static const char* CszSysWinABCDSolverSaveAux[CSLVC_NUM_SAVE]; // auxiliary saved information
   static const char* CszSolveDirNames;     // min/max solver direction
   static const UINT CuProperties[];        // properties revealed by this renderer

   static const char* CszSolverVariables[CSLVI_PROP_SYSMAX+1]; // variable names in solver equation
};

#endif//CSYSWINABCDSOLVER_H
