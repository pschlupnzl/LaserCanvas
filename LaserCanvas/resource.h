/*********************************************************
* Resource.h
* This isn't my idea;  Microsoft VisualStudio generates a
* resource.h file with its resource editor.  This ensures
* that menu and command constants are the same everywhere
*
* This file can only include constants - no macros!
*
* $PSchlup 2006 $     $Revision 5 $
*********************************************************/
#ifndef RESOURCE_H                          // prevent multiple includes
#define RESOURCE_H

//===Window classes=======================================
#define CSZ_WNDCLASS_APPLICATION  "LasrCanv5_Class"
#define CSZ_CAPTION_APPLICATION   "LaserCanvas 5"

//---SysWin classes-------------------------
#define CSZ_WNDCLASS_SYSWIN1D         "SysWin1d_Class"
#define CSZ_WNDCLASS_SYSWIN2D         "SysWin2d_Class"
#define CSZ_WNDCLASS_SYSWIN3D         "SysWin3d_Class"
#define CSZ_WNDCLASS_SYSWINGRAPH      "SysWinGraph_Class"
#define CSZ_WNDCLASS_SYSWINVXGRAPH    "SysWinVxGraph_Class"
#define CSZ_WNDCLASS_SYSWININVENTORY  "SysWinInventory_Class"
#define CSZ_WNDCLASS_SYSWINABCDSOLVER "SysWinABCDSolver_Class"

//===Macros===============================================
#define MoveTo(dc,x,y) MoveToEx(dc,x,y,NULL)

//===Menu Constants=======================================
#define CMU_FILE_EXIT        10000
#define CMU_FILE_NEWRESO     10001
#define CMU_FILE_NEWPROP     10002
#define CMU_FILE_OPEN        10003
#define CMU_FILE_SAVE        10004
#define CMU_FILE_SAVEAS      10005
#define CMU_FILE_CLOSESYSTEM 10008
#define CMU_FILE_CLOSEMODEL  10009
#define CMU_FILE_PRINTSETUP  10017
#define CMU_FILE_PRINT       10018
#define CMU_FILE_PRINTALL    10019

#define CMU_COPYDATA         10021

#define CMU_INS_MIRR         10031
#define CMU_INS_LENS         10032
#define CMU_INS_CRYS         10033
#define CMU_INS_BREW         10034
#define CMU_INS_PLAT         10035
#define CMU_INS_PRIP         10036
#define CMU_INS_SCRN         10037
#define CMU_DEL_OPTI         10039
#define CMU_INS_SPAWN        10040

#define CMU_CNV_ZOOMIN       10048
#define CMU_CNV_ZOOMOUT      10049

#define CMU_TOOL_ARROW       10051
#define CMU_TOOL_MEASURE     10052
#define CMU_TOOL_ZOOM        10053
#define CMU_TOOL_PAN         10054
#define CMU_TOOL_ROTATE      10055
#define CMU_TOOL_DRAFT       10056

#define CMU_VIEW_MODESIZE    10071
#define CMU_VIEW_SYSGRAPH    10072
#define CMU_VIEW_VXGRAPH     10073
#define CMU_VIEW_INVENTORY   10074
#define CMU_VIEW_SOLVER      10075
#define CMU_VIEW_RENDER3D    10076
#define CMU_VIEW_PROPEQTN    10077
#define CMU_VIEW_PROPERTIES  10078
#define CMU_VIEW_DOCKPROP    10079

#define CMU_HELP_HELP        10088
#define CMU_HELP_ABOUT       10089

#define CMU_POPUP_COPYDATA   10901          // popup menu commands
#define CMU_BUTTON_SOLVE     10902          // solver solve button
#define CMU_BUTTON_EVAL      10903          // solver evaluate button
#define CMU_BUTTON_RESTORE   10904          // solver restore

#define CMU_WINDOW_TILE      11001
#define CMU_WINDOW_CASCADE   11002
#define CMU_WINDOW_ARGICON   11003
#define CMU_WINDOW_RESTORE   11004
#define CMU_WINDOW_MDICHILD  11010
#define CMI_MDIMENU              4          // index where MDI windows are appended


//===Stringtable Constants================================

//---Window titles------------------------------
#define SZ_TITLE_SYSWIN1D         20000
#define SZ_TITLE_SYSWIN2D         20001
#define SZ_TITLE_SYSWIN3D         20002
#define SZ_TITLE_SYSWINGRAPH      20003
#define SZ_TITLE_SYSWINVXGRAPH    20004
#define SZ_TITLE_SYSWININVENTORY  20005
#define SZ_TITLE_SYSWINABCDSOLVER 20006

//---Load File Errors---------------------------
// These IDs are passed directly  from the function where
// the error occurs to CApplication::LoadErrorMsg.
#define SZERR_LOAD_FILEOPEN  20101         // Could not open file for reading
#define SZERR_LOAD_BUFALLOC  20102         // Failed to allocate buffer for reading
#define SZERR_LOAD_NOTOPSYS  20103         // Failed to locate top system in data file
#define SZERR_LOAD_NOSPAWN   20111         // Failed to locate spawned system
#define SZERR_LOAD_SYSNOOPT  20124         // System contains no optics
#define SZERR_LOAD_RNDNOSYS  20121         // Renderer has no system declared
#define SZERR_LOAD_RNDBADSYS 20122         // Renderer has bad system declaration
#define SZERR_LOAD_RNDSYSFND 20123         // Could not find system for renderer
#define SZERR_LOAD_GRPHBADVX 20131         // Bad vertex declaration in graph
#define SZERR_LOAD_NOGRPHVX  20132         // Could not find vertex in system
#define SZERR_LOAD_GRPHQSYS  20133         // Renderer for vertex points to wrong system
#define SZERR_SOLV_ALLOC     20134         // Error allocating buffers
#define SZERR_SOLV_NOVAR     20135         // Equation contains no variables
#define SZERR_HELP_FILE      20136         // Could not find the help file or application
#define SZERR_HELP_LAUNCH    20137         // Could not launch the help browser
#define SZERR_SYSDRAFTOPTICS 20138         // The system contains optics not allowed in draft mode
#define SZERR_SYSDRAFTCAVITY 20139         // The system cannot be solved as linked

#define SZMSG_DELSPAWN       21001         // This will delete the outcoupled system from the model [OKCANCEL]
#define SZMSG_FILESAVED      21002         // Saved to file %s [status bar]
#define SZMSG_SOLVEEVAL      21003         // solver evaluation answer (with %lg)

//---Property Manager Labels--------------------
//---Property Sheet-----------------------------
#define CPS_VXHEADER          2000
#define CPS_VXTAG             2001
#define CPS_VXRADCURV         2002
#define CPS_VXRADCURVSAG      2003
#define CPS_VXRADCURVTAN      2004
#define CPS_VXFOCALLEN        2005
#define CPS_VXFOCALLENSAG     2006
#define CPS_VXFOCALLENTAN     2007
#define CPS_VXROCFLASTIG      2008
#define CPS_VXFACEANGL        2009
#define CPS_VXLOCKFACEANGL    2010
#define CPS_VXBLOCKREFINDX    2011
#define CPS_VXTHICKNESS       2012
#define CPS_VXFLIPDIR         2013
#define CPS_VXFLIPDIRFLIP     2014
#define CPS_VXDISTNEXT        2015
#define CPS_VXSPOTSIZE        2016
#define CPS_VXCURVATURE       2017
#define CPS_VXABCDSAG         2018
#define CPS_VXABCDTAN         2019
#define CPS_VXASTIG           2020
#define CPS_VXMAX             2020 // for end of auto-mask loop

#define CPS_EQHEADER          2030
#define CPS_EQVARNAME         2031
#define CPS_EQVALUE           2032
#define CPS_EQSLIDER          2033
#define CPS_EQRANGE           2034

#define CPS_SLVHEADER         2040
#define CPS_SLVFCNTOL         2041
#define CPS_SLVMAXITER        2042
#define CPS_SLVDIR            2043

#define CPS_SYSHEADER         2050
#define CPS_SYSHEADRESO       2051
#define CPS_SYSHEADPROP       2052
#define CPS_SYSHEADSPWN       2053
#define CPS_SYSTAG            2054
#define CPS_SYSWAVELEN        2055
#define CPS_SYSMSQUARED       2056
#define CPS_SYSMSQUAREDSAG    2057
#define CPS_SYSMSQUAREDTAN    2058
#define CPS_SYSMSQASYM        2059
#define CPS_SYSROTATION       2060
#define CPS_SYSSTARTX         2061
#define CPS_SYSSTARTY         2062
#define CPS_SYSPHYSLEN        2063
#define CPS_SYSOPTLEN         2064
#define CPS_SYSMODESPACE      2065
#define CPS_SYSINITDATA       2066
#define CPS_SYSISAGSIZE       2067
#define CPS_SYSISAGCURV       2068
#define CPS_SYSISAGWAIST      2069
#define CPS_SYSISAGDSTW       2070
#define CPS_SYSITANSIZE       2071
#define CPS_SYSITANCURV       2072
#define CPS_SYSITANWAIST      2073
#define CPS_SYSITANDSTW       2074
#define CPS_SYSABCDSAG        2075
#define CPS_SYSABCDTAN        2076
#define CPS_SYSSTABILITY      2077

#define CPS_CNVHEADER         2080
#define CPS_CNVXYCENTER       2081
#define CPS_CNVZOOM           2082
#define CPS_CNVMODESIZE       2083
#define CPS_CNVOPTSIZE        2084
#define CPS_CNVGRIDSIZE       2085
#define CPS_CNVSNAPGRID       2086
#define CPS_CNVSHOWDIST       2087
#define CPS_CNVSHOWANNOT      2088
#define CPS_CNVSHOWWAIST      2089
#define CPS_CNVSHOWEQTN       2090
#define CPS_CNVSHOWICONS      2091
#define CPS_CNV3DORG          2092
#define CPS_CNV3DCAM          2093
#define CPS_CNV3DCAMELEV      2094
#define CPS_CNV3DCAMANGL      2095

#define CPS_GRPHEADER         2100
#define CPS_GRPSYSFCN         2101
#define CPS_GRPVXFCN          2102
#define CPS_GRPAXXRNG         2103
#define CPS_GRPAXYRNG         2104
#define CPS_GRPNUMSTP         2105
#define CPS_GRPRUNVAR         2106

#define CPS_CMDHEADER         2190
#define CPS_COMMANDNEW        2191
#define CPS_COMMANDNEWPROP    2192
#define CPS_COMMANDOPEN       2193
#define CPS_COMMANDCLOSE      2194


#define CPL_SYSFCNNAMES       2901
#define CPL_SYSINPPARAM       2902
#define CPL_VXFCNNAMES        2903
#define CPL_VXTYPENAMES       2904
#define CPL_SLVDIR            2905
#define CPL_INVVXCOLS         2906
#define CPL_INVSYSROWS        2907

#endif//RESOURCE_H
