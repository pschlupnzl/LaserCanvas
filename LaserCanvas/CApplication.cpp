/*********************************************************
* CApplication
* Application class. There  will be only one CApplication
* object per running instance.
* CSystem hooks: At this point, I envisage that the CSys-
* tem objects will not be chained  in the application ob-
* ject, that we will instead  track them with the visible
* MDI child windows.
*
* $PSchlup 2006 $     $Revision 5 $
*********************************************************/
#include "CApplication.h"                   // class header

const char *CApplication::CpszVar = "x\0y\0z\0"; // null-terminated string of strings

/*********************************************************
* Constructor
*********************************************************/
CApplication::CApplication(HINSTANCE hInst, int iShow) {printf("_CApp____________________________________________\n");
   WNDCLASS wc;                             // window registration structure
   CLIENTCREATESTRUCT ccs;                  // structure for MDI client
   //OBSOLETE char szBuf[256];                         // temporary string buffer
   int k;                                   // loop counter

   //===Members===========================================
   hInstance        = hInst;                // application instance
   hwApp            = NULL;                 // no window yet
   pPropMgr         = NULL;                 // no property manager
   pStatusBar       = NULL;                 // no status bar
   pButtonBar       = NULL;                 // no button bar
   pSysTop          = NULL;                 // top of system chain
   hwMDIClient      = NULL;                 // no mdi client yet
   uFlags           = 0;                    // default flags
   for(k=0; k<CAPP_NUMVAR; k++) {
      dVar[k]       = 0.00;
      dVarMin[k]    = 0.00;
      dVarMax[k]    = 1.00;
      iVarHooks[k]  = 0;
      iVarPoints[k] = 20;
   }
   iCurrentVariable = 0;

   //===Register Window Classes===========================
   // The class names are defined in resource.h
   //---Application window----------------------
   wc.style         = CS_HREDRAW | CS_VREDRAW;
   wc.lpszClassName = CSZ_WNDCLASS_APPLICATION;
   wc.lpszMenuName  = "LASRCANV_MENU";
   wc.hInstance     = hInst;
   wc.hbrBackground = (HBRUSH) NULL;
   wc.hIcon         = LoadIcon(hInst, "ICON_LASRCANV5");
   wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
   wc.cbClsExtra    = 0;
   wc.cbWndExtra    = sizeof(CApplication*);
   wc.lpfnWndProc   = CApplication::WndProcMain;
   RegisterClass(&wc);                      // register main window class

   //---Renderer: 2d----------------------------
   wc.style         = CS_HREDRAW | CS_VREDRAW;
   wc.lpszClassName = CSZ_WNDCLASS_SYSWIN2D;
   wc.lpszMenuName  = NULL;
   wc.hInstance     = hInst;
   wc.hbrBackground = (HBRUSH) NULL;
   wc.hIcon         = LoadIcon(hInst, "ICON_WND_RESO");
   wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
   wc.cbClsExtra    = 0;
   wc.cbWndExtra    = sizeof(CSysWin2d*);
   wc.lpfnWndProc   = CSysWin2d::_WndProcSysWin2d;
   RegisterClass(&wc);

   //---Renderer: 1d----------------------------
   wc.style         = CS_HREDRAW | CS_VREDRAW;
   wc.lpszClassName = CSZ_WNDCLASS_SYSWIN1D;
   wc.lpszMenuName  = NULL;
   wc.hInstance     = hInst;
   wc.hbrBackground = (HBRUSH) NULL;
   wc.hIcon         = LoadIcon(hInst, "ICON_WND_RESO");
   wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
   wc.cbClsExtra    = 0;
   wc.cbWndExtra    = sizeof(CSysWin1d*);
   wc.lpfnWndProc   = CSysWin1d::WndProcSysWin1d;
   RegisterClass(&wc);

   //---Renderer: 3d---------------------------
   wc.style         = CS_HREDRAW | CS_VREDRAW;
   wc.lpszClassName = CSZ_WNDCLASS_SYSWIN3D;
   wc.lpszMenuName  = NULL;
   wc.hInstance     = hInst;
   wc.hbrBackground = (HBRUSH) NULL;
   wc.hIcon         = LoadIcon(hInst, "ICON_WND_RN3D");
   wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
   wc.cbClsExtra    = 0;
   wc.cbWndExtra    = sizeof(CSysWin3d*);
   wc.lpfnWndProc   = CSysWin3d::_WndProcSysWin3d;
   RegisterClass(&wc);

   //---Renderer: System Graph-----------------
   wc.style         = CS_HREDRAW | CS_VREDRAW;
   wc.lpszClassName = CSZ_WNDCLASS_SYSWINGRAPH;
   wc.lpszMenuName  = NULL;
   wc.hInstance     = hInst;
   wc.hbrBackground = (HBRUSH) NULL;
   wc.hIcon         = LoadIcon(hInst, "ICON_WND_GRAPH");
   wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
   wc.cbClsExtra    = 0;
   wc.cbWndExtra    = sizeof(CSysWinGraph*);
   wc.lpfnWndProc   = CSysWinGraph::WndProcSysWinGraph;
   RegisterClass(&wc);

   //---Renderer: Vertex Graph-----------------
   wc.style         = CS_HREDRAW | CS_VREDRAW;
   wc.lpszClassName = CSZ_WNDCLASS_SYSWINVXGRAPH;
   wc.lpszMenuName  = NULL;
   wc.hInstance     = hInst;
   wc.hbrBackground = (HBRUSH) NULL;
   wc.hIcon         = LoadIcon(hInst, "ICON_WND_GRAPH");
   wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
   wc.cbClsExtra    = 0;
   wc.cbWndExtra    = sizeof(CSysWinVxGraph*);
   wc.lpfnWndProc   = CSysWinVxGraph::WndProcSysWinVxGraph;
   RegisterClass(&wc);

   //---Renderer: Inventory---------------------
   wc.style         = CS_HREDRAW | CS_VREDRAW;
   wc.lpszClassName = CSZ_WNDCLASS_SYSWININVENTORY;
   wc.lpszMenuName  = NULL;
   wc.hInstance     = hInst;
   wc.hbrBackground = (HBRUSH) NULL;
   wc.hIcon         = LoadIcon(hInst, "ICON_WND_DATA");
   wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
   wc.cbClsExtra    = 0;
   wc.cbWndExtra    = sizeof(CSysWinInventory*);
   wc.lpfnWndProc   = CSysWinInventory::WndProcSysWinInventory;
   RegisterClass(&wc);

   //---Renderer: Solver------------------------
   wc.style         = CS_HREDRAW | CS_VREDRAW;
   wc.lpszClassName = CSZ_WNDCLASS_SYSWINABCDSOLVER;
   wc.lpszMenuName  = NULL;
   wc.hInstance     = hInst;
   wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
   wc.hIcon         = LoadIcon(hInst, "ICON_WND_EQTN");
   wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
   wc.cbClsExtra    = 0;
   wc.cbWndExtra    = sizeof(CSysWinABCDSolver*);
   wc.lpfnWndProc   = CSysWinABCDSolver::WndProcSysWinABCDSolver;
   RegisterClass(&wc);

   //===Global Graphics Objects===========================
   //---Invalidate Pointers---------------------
   for(k=0; k<CAPPC_NUMRGB;   k++) rgbColor[k]    = RGB(0x00, 0x00, 0x00);
   for(k=0; k<CAPPC_NUMPEN;   k++) hpAppPen[k]    = NULL;
   for(k=0; k<CAPPC_NUMBRUSH; k++) hbrAppBrush[k] = NULL;
   for(k=0; k<CAPPC_NUMFONT;  k++) hfAppFont[k]   = NULL;
   for(k=0; k<CAPPC_NUM_CUR;  k++) hcAppCursor[k] = NULL;

   //---Colors----------------------------------
   rgbColor[ CAPPI_RGBBLACK   ] = RGB(0x00, 0x00, 0x00);
   rgbColor[ CAPPI_RGBWINDOW  ] = GetSysColor(COLOR_WINDOW);
   rgbColor[ CAPPI_RGB3DLITE  ] = GetSysColor(COLOR_3DHIGHLIGHT);
   rgbColor[ CAPPI_RGB3DDARK  ] = GetSysColor(COLOR_3DSHADOW);
   rgbColor[ CAPPI_RGB3DFACE  ] = GetSysColor(COLOR_BTNFACE);
   rgbColor[ CAPPI_RGB3DTEXT  ] = GetSysColor(COLOR_BTNTEXT);
   rgbColor[ CAPPI_RGB3DGRAY  ] = GetSysColor(COLOR_GRAYTEXT);
   rgbColor[ CAPPI_RGBSAG     ] = RGB(0xCC, 0x00, 0x33);
   rgbColor[ CAPPI_RGBTAN     ] = RGB(0x33, 0x00, 0xFF);
   rgbColor[ CAPPI_RGBSAGTAN  ] = RGB(0x99, 0x00, 0xCC);
   rgbColor[ CAPPI_RGBEQTN    ] = RGB(0x00, 0x66, 0x33);
   rgbColor[ CAPPI_RGBOPTIC   ] = RGB(0x00, 0x00, 0x00);
   rgbColor[ CAPPI_RGBSEGMENT ] = RGB(0x88, 0x88, 0x88);
   rgbColor[ CAPPI_RGBCMD     ] = RGB(0x00, 0x00, 0x99);
   rgbColor[ CAPPI_RGBAXES    ] = RGB(0x33, 0x33, 0x33);
   rgbColor[ CAPPI_RGB3DMARKER] = RGB(0xFF, 0xFF, 0xCC);
   rgbColor[ CAPPI_RGBREFINDEX] = RGB(0xDD, 0xDD, 0xDD);
   rgbColor[ CAPPI_RGBGRID    ] = RGB(0xCC, 0xCC, 0xCC);
   rgbColor[ CAPPI_RGBDRAFT   ] = RGB(0x33, 0x33, 0xFF);

   //---Pens------------------------------------
///TODO: Check these with non-standard background colors
   hpAppPen[ CAPPI_PEN              ] = CreatePen(PS_SOLID, 1, RGB(0x00, 0x00, 0x00));
   hpAppPen[ CAPPI_PEN3DLITE        ] = CreatePen(PS_SOLID, 1, rgbColor[CAPPI_RGB3DLITE ]);
   hpAppPen[ CAPPI_PEN3DDARK        ] = CreatePen(PS_SOLID, 1, rgbColor[CAPPI_RGB3DDARK ]);
   hpAppPen[ CAPPI_PEN3DFACE        ] = CreatePen(PS_SOLID, 1, rgbColor[CAPPI_RGB3DFACE ]);
   hpAppPen[ CAPPI_PEN3DTEXT        ] = CreatePen(PS_SOLID, 1, rgbColor[CAPPI_RGB3DTEXT ]);
   hpAppPen[ CAPPI_PENCMD           ] = CreatePen(PS_SOLID, 1, rgbColor[CAPPI_RGBCMD    ]);
   hpAppPen[ CAPPI_PENSAG           ] = CreatePen(PS_SOLID, 1, rgbColor[CAPPI_RGBSAG    ]);
   hpAppPen[ CAPPI_PENTAN           ] = CreatePen(PS_SOLID, 1, rgbColor[CAPPI_RGBTAN    ]);
   hpAppPen[ CAPPI_PENSAGTAN        ] = CreatePen(PS_SOLID, 1, rgbColor[CAPPI_RGBSAGTAN ]);
   hpAppPen[ CAPPI_PENOPTIC         ] = CreatePen(PS_SOLID, 1, rgbColor[CAPPI_RGBOPTIC  ]);
   hpAppPen[ CAPPI_PENSEGMENT       ] = CreatePen(PS_SOLID, 1, rgbColor[CAPPI_RGBSEGMENT]);
   hpAppPen[ CAPPI_PENOPTICSEL      ] = CreatePen(PS_SOLID, 2, rgbColor[CAPPI_RGBOPTIC  ]);
   hpAppPen[ CAPPI_PENSEGMENTSEL    ] = CreatePen(PS_SOLID, 3, rgbColor[CAPPI_RGBSEGMENT]);
   hpAppPen[ CAPPI_PENSEGMENTOUT    ] = CreatePen(PS_SOLID, 1, rgbColor[CAPPI_RGBOPTIC  ]);
   hpAppPen[ CAPPI_PENAXES          ] = CreatePen(PS_SOLID, 1, rgbColor[CAPPI_RGBAXES   ]);
   hpAppPen[ CAPPI_PENGRID          ] = CreatePen(PS_SOLID, 1, rgbColor[CAPPI_RGBGRID   ]);
   hpAppPen[ CAPPI_PENDEBUG         ] = CreatePen(PS_SOLID, 3, RGB(0xFF, 0x00, 0x00));
   hpAppPen[ CAPPI_PENDRAFTOPTIC    ] = CreatePen(PS_SOLID, 1, rgbColor[CAPPI_RGBDRAFT  ]);
   hpAppPen[ CAPPI_PENDRAFTOPTICSEL ] = CreatePen(PS_SOLID, 2, rgbColor[CAPPI_RGBDRAFT  ]);


   //---Brushes---------------------------------
   hbrAppBrush[ CAPPI_BRUSHWINDOW   ] = CreateSolidBrush(rgbColor[CAPPI_RGBWINDOW]);
   hbrAppBrush[ CAPPI_BRUSH3DFACE   ] = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
   hbrAppBrush[ CAPPI_BRUSH3DMARKER ] = CreateSolidBrush(rgbColor[CAPPI_RGB3DMARKER]);
   hbrAppBrush[ CAPPI_BRUSHREFINDEX ] = CreateSolidBrush(rgbColor[CAPPI_RGBREFINDEX]);
   hbrAppBrush[ CAPPI_BRUSHDASH     ] = CreatePatternBrush(LoadBitmap(hInstance, "BITMAP_BRUSHDASH" ));
   hbrAppBrush[ CAPPI_BRUSHHATCH    ] = CreatePatternBrush(LoadBitmap(hInstance, "BITMAP_BRUSHHATCH"));

   //---Fonts-----------------------------------
   hfAppFont[ CAPPI_FONT        ] = CreateFont(-12, 0,0,0,0,0,0,0,0,0,0,0,0, "Verdana");
   hfAppFont[ CAPPI_FONTPROPMGR ] = CreateFont(-12, 0,0,0,0,0,0,0,0,0,0,0,0, "Tahoma");
   hfAppFont[ CAPPI_FONTPRINT   ] = NULL;

   //---Cursors---------------------------------
   hcAppCursor[ CAPPI_CUR_ARROW         ] = LoadCursor(NULL, IDC_ARROW);
   //hcAppCursor[ CAPPI_CUR_ARROW         ] = LoadCursor(hInstance, "CUR_ARROW"        );
   hcAppCursor[ CAPPI_CUR_DELETE        ] = LoadCursor(hInstance, "CUR_DELETE"       );
   hcAppCursor[ CAPPI_CUR_MEAS_LOCK     ] = LoadCursor(hInstance, "CUR_MEAS_LOCK"    );
   hcAppCursor[ CAPPI_CUR_MEAS          ] = LoadCursor(hInstance, "CUR_MEAS"         );
   hcAppCursor[ CAPPI_CUR_MOVE_ARROW    ] = LoadCursor(hInstance, "CUR_MOVE_ARROW"   );
   hcAppCursor[ CAPPI_CUR_MOVE          ] = LoadCursor(hInstance, "CUR_MOVE"         );
   hcAppCursor[ CAPPI_CUR_NODRAG        ] = LoadCursor(hInstance, "CUR_NODRAG"       );
   hcAppCursor[ CAPPI_CUR_PAN_ARROW     ] = LoadCursor(hInstance, "CUR_PAN_ARROW"    );
   hcAppCursor[ CAPPI_CUR_PAN           ] = LoadCursor(hInstance, "CUR_PAN"          );
   hcAppCursor[ CAPPI_CUR_ROTATE_ARROW  ] = LoadCursor(hInstance, "CUR_ROTATE_ARROW" );
   hcAppCursor[ CAPPI_CUR_ROTATE        ] = LoadCursor(hInstance, "CUR_ROTATE"       );
   hcAppCursor[ CAPPI_CUR_STRETCH_ARROW ] = LoadCursor(hInstance, "CUR_STRETCH_ARROW");
   hcAppCursor[ CAPPI_CUR_STRETCH       ] = LoadCursor(hInstance, "CUR_STRETCH"      );
   hcAppCursor[ CAPPI_CUR_ZOOMIN        ] = LoadCursor(hInstance, "CUR_ZOOMIN"       );
   hcAppCursor[ CAPPI_CUR_ZOOMOUT       ] = LoadCursor(hInstance, "CUR_ZOOMOUT"      );

   //---Bitmaps---------------------------------
   // none so far

   //===Prepare Application===============================
   //---Application window----------------------
///TODO: Load application title from string table
   hwApp = CreateWindow(CSZ_WNDCLASS_APPLICATION, CSZ_CAPTION_APPLICATION,
      WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
      HWND_DESKTOP, (HMENU) NULL, hInstance, (LPVOID) this);

   //---MDI Client window-----------------------
   ccs.hWindowMenu = GetSubMenu(GetMenu(hwApp), CMI_MDIMENU);
   ccs.idFirstChild = CMU_WINDOW_MDICHILD;
   hwMDIClient = CreateWindow("MDICLIENT", (LPSTR) NULL,
      WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_HSCROLL | WS_VSCROLL | WS_VISIBLE,
      0, 0, 0, 0, hwApp, (HMENU) NULL, hInstance, (LPVOID) &ccs);
   EnableMenus(-1);                         // set menu states

   //===Bars==============================================
   //---Status Bar------------------------------
   pStatusBar = new CStatusBar(hwApp, 2);   // create status bar
   double ddWidths[2] = {0, 160};
   pStatusBar->SetRegionWidths(ddWidths, 2);
   pStatusBar->SetStandardText("");
   pStatusBar->SetStandardText("(---, ---)", 1);
   pStatusBar->ShowBarWindow(TRUE);

   //---Button Bar------------------------------
   pButtonBar = new CButtonBar(hwApp);      // create button bar
   pButtonBar->ClearBarStyleBit(CBBS_BORDER);
   pButtonBar->SetBarStyleBit(CBBS_BORDERRAISED);
   pButtonBar->AppendButton("BMP_BUT_NEWRESONATOR", CMU_FILE_NEWRESO);
   pButtonBar->AppendButton("BMP_BUT_NEWPROP"     , CMU_FILE_NEWPROP);
   pButtonBar->AppendButton("BMP_BUT_OPEN"        , CMU_FILE_OPEN);
   pButtonBar->AppendButton("BMP_BUT_SAVE"        , CMU_FILE_SAVE);
   pButtonBar->AppendButton("BMP_BUT_PRINT"       , CMU_FILE_PRINT);
   pButtonBar->AppendButton(NULL, NULL);
   pButtonBar->AppendButton("BMP_BUT_SHOWINFO"    , CMU_VIEW_PROPERTIES);
   pButtonBar->AppendButton("BMP_BUT_SHOWEQTN"    , CMU_VIEW_PROPEQTN);
   pButtonBar->AppendButton(NULL, NULL);
   pButtonBar->AppendButton("BMP_BUT_MIRROR"      , CMU_INS_MIRR);
   pButtonBar->AppendButton("BMP_BUT_LENS"        , CMU_INS_LENS);
   pButtonBar->AppendButton("BMP_BUT_BLOCK"       , CMU_INS_PLAT);
   pButtonBar->AppendButton("BMP_BUT_BREWSTER"    , CMU_INS_BREW);
   pButtonBar->AppendButton("BMP_BUT_CRYSTAL"     , CMU_INS_CRYS);
   pButtonBar->AppendButton("BMP_BUT_PRISM"       , CMU_INS_PRIP);
   pButtonBar->AppendButton("BMP_BUT_SCREEN"      , CMU_INS_SCRN);
   pButtonBar->AppendButton("BMP_BUT_DELETE"      , CMU_DEL_OPTI);
   pButtonBar->AppendButton(NULL, NULL);
   pButtonBar->AppendButton("BMP_BUT_1D"          , CMU_VIEW_MODESIZE);
   pButtonBar->AppendButton("BMP_BUT_INVENTORY"   , CMU_VIEW_INVENTORY);
   pButtonBar->AppendButton("BMP_BUT_SOLVER"      , CMU_VIEW_SOLVER);
   pButtonBar->AppendButton("BMP_BUT_RENDER3D"    , CMU_VIEW_RENDER3D);
   pButtonBar->AppendButton("BMP_BUT_GRAPH"       , CMU_VIEW_SYSGRAPH);
   pButtonBar->AppendButton("BMP_BUT_GRAPHVX"     , CMU_VIEW_VXGRAPH);
   pButtonBar->AppendButton("BMP_BUT_SPAWNSYS"    , CMU_INS_SPAWN);
   pButtonBar->AppendButton(NULL, NULL);
   pButtonBar->AppendButton("BMP_BUT_DRAFT"       , CMU_TOOL_DRAFT);
   pButtonBar->AppendButton("BMP_BUT_TOOLARROW"   , CMU_TOOL_ARROW);
   pButtonBar->AppendButton("BMP_BUT_TOOLMEASURE" , CMU_TOOL_MEASURE);
   pButtonBar->AppendButton("BMP_BUT_TOOLZOOM"    , CMU_TOOL_ZOOM);
   pButtonBar->AppendButton("BMP_BUT_TOOLPAN"     , CMU_TOOL_PAN);
   pButtonBar->AppendButton("BMP_BUT_TOOLROTATE"  , CMU_TOOL_ROTATE);
   pButtonBar->ShowBarWindow(TRUE);

   //===Property Manager==================================
   //---Property Manager------------------------
   pPropMgr = new CPropMgr(this);           // instantiate property manager
   UserDockProperties(TRUE);                // create docked window
   //---Items-----------------------------------
   // Visible items (items are  created HIDDEN!) are dis-
   // played in the order in which they are created, so a
   // combination of visibility and creation sequence de-
   // termines the look and layout of the list.
   pPropMgr->NewResourceItem( CPS_VXHEADER      , CPIT_HEADING  , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_VXTAG         , CPIT_TEXT     , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_VXRADCURV     , CPIT_EQUATION , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_VXRADCURVSAG  , CPIT_EQUATION , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_VXRADCURVTAN  , CPIT_EQUATION , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_VXFOCALLEN    , CPIT_EQUATION , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_VXFOCALLENSAG , CPIT_EQUATION , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_VXFOCALLENTAN , CPIT_EQUATION , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_VXFACEANGL    , CPIT_EQUATION , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_VXBLOCKREFINDX, CPIT_EQUATION , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_VXTHICKNESS   , CPIT_EQUATION , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_VXFLIPDIR     , CPIT_TOGGLE   , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_VXDISTNEXT    , CPIT_EQUATION , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_VXROCFLASTIG  , CPIT_CHECKBOX , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_VXLOCKFACEANGL, CPIT_CHECKBOX , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_VXSPOTSIZE    , CPIT_SAGTAN   , CPIF_READONLY);
   pPropMgr->NewResourceItem( CPS_VXCURVATURE   , CPIT_SAGTAN   , CPIF_READONLY);
   pPropMgr->NewResourceItem( CPS_VXASTIG       , CPIT_VALUE    , CPIF_READONLY);
   pPropMgr->NewResourceItem( CPS_VXABCDSAG     , CPIT_QUADVAL  , CPIF_READONLY);
   pPropMgr->NewResourceItem( CPS_VXABCDTAN     , CPIT_QUADVAL  , CPIF_READONLY);

   pPropMgr->NewResourceItem( CPS_GRPHEADER     , CPIT_HEADING  , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_GRPSYSFCN     , CPIT_RADIOLIST, CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_GRPVXFCN      , CPIT_RADIOLIST, CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_GRPRUNVAR     , CPIT_DROPLIST , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_GRPAXXRNG     , CPIT_DBLVAL   , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_GRPAXYRNG     , CPIT_DBLVAL   , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_GRPNUMSTP     , CPIT_VALUE    , CPIF_NONE);

   pPropMgr->NewResourceItem( CPS_SLVHEADER     , CPIT_HEADING  , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_SLVMAXITER    , CPIT_VALUE    , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_SLVFCNTOL     , CPIT_VALUE    , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_SLVDIR        , CPIT_RADIOLIST, CPIF_NONE);

   pPropMgr->NewResourceItem( CPS_SYSHEADER     , CPIT_HEADING  , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_SYSTAG        , CPIT_TEXT     , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_SYSWAVELEN    , CPIT_EQUATION , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_SYSMSQUARED   , CPIT_EQUATION , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_SYSMSQUAREDSAG, CPIT_EQUATION , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_SYSMSQUAREDTAN, CPIT_EQUATION , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_SYSMSQASYM    , CPIT_CHECKBOX , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_SYSROTATION   , CPIT_EQUATION , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_SYSSTARTX     , CPIT_EQUATION , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_SYSSTARTY     , CPIT_EQUATION , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_SYSPHYSLEN    , CPIT_VALUE    , CPIF_READONLY);
   pPropMgr->NewResourceItem( CPS_SYSOPTLEN     , CPIT_VALUE    , CPIF_READONLY);
   pPropMgr->NewResourceItem( CPS_SYSMODESPACE  , CPIT_VALUE    , CPIF_READONLY);
   pPropMgr->NewResourceItem( CPS_SYSINITDATA   , CPIT_RADIOLIST, CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_SYSISAGSIZE   , CPIT_EQUATION , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_SYSITANSIZE   , CPIT_EQUATION , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_SYSITANCURV   , CPIT_EQUATION , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_SYSISAGCURV   , CPIT_EQUATION , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_SYSISAGWAIST  , CPIT_EQUATION , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_SYSITANWAIST  , CPIT_EQUATION , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_SYSISAGDSTW   , CPIT_EQUATION , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_SYSITANDSTW   , CPIT_EQUATION , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_SYSITANWAIST  , CPIT_EQUATION , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_SYSITANDSTW   , CPIT_EQUATION , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_SYSSTABILITY  , CPIT_SAGTAN   , CPIF_READONLY);
   pPropMgr->NewResourceItem( CPS_SYSABCDSAG    , CPIT_QUADVAL  , CPIF_READONLY);
   pPropMgr->NewResourceItem( CPS_SYSABCDTAN    , CPIT_QUADVAL  , CPIF_READONLY);

   pPropMgr->NewResourceItem( CPS_EQHEADER      , CPIT_HEADING  , CPIF_COLLAPSED);
   pPropMgr->NewResourceItem( CPS_EQVARNAME     , CPIT_DROPLIST , CPIF_COLLAPSED, CApplication::_PropItemCallback, (void*)this);
   pPropMgr->NewResourceItem( CPS_EQVALUE       , CPIT_VALUE    , CPIF_COLLAPSED, CApplication::_PropItemCallback, (void*)this);
   pPropMgr->NewResourceItem( CPS_EQSLIDER      , CPIT_SLIDER   , CPIF_COLLAPSED, CApplication::_PropItemCallback, (void*)this);
   pPropMgr->NewResourceItem( CPS_EQRANGE       , CPIT_DBLVAL   , CPIF_COLLAPSED, CApplication::_PropItemCallback, (void*)this);

   pPropMgr->NewResourceItem( CPS_CNVHEADER     , CPIT_HEADING  , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_CNVZOOM       , CPIT_VALUE    , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_CNVMODESIZE   , CPIT_VALUE    , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_CNVOPTSIZE    , CPIT_VALUE    , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_CNVGRIDSIZE   , CPIT_VALUE    , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_CNV3DORG      , CPIT_DBLVAL   , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_CNV3DCAM      , CPIT_DBLVAL   , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_CNV3DCAMELEV  , CPIT_VALUE    , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_CNV3DCAMANGL  , CPIT_VALUE    , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_CNVSHOWICONS  , CPIT_CHECKBOX , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_CNVSNAPGRID   , CPIT_CHECKBOX , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_CNVSHOWDIST   , CPIT_CHECKBOX , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_CNVSHOWANNOT  , CPIT_CHECKBOX , CPIF_NONE);
   pPropMgr->NewResourceItem( CPS_CNVSHOWWAIST  , CPIT_CHECKBOX , CPIF_NONE);

   pPropMgr->NewResourceItem( CPS_CMDHEADER     , CPIT_HEADING  , CPIF_NONE, CApplication::_PropItemCallback, (void*)this);
   pPropMgr->NewResourceItem( CPS_COMMANDNEW    , CPIT_COMMAND  , CPIF_NONE, CApplication::_PropItemCallback, (void*)this);
   pPropMgr->NewResourceItem( CPS_COMMANDNEWPROP, CPIT_COMMAND  , CPIF_NONE, CApplication::_PropItemCallback, (void*)this);
   pPropMgr->NewResourceItem( CPS_COMMANDOPEN   , CPIT_COMMAND  , CPIF_NONE, CApplication::_PropItemCallback, (void*)this);
   pPropMgr->NewResourceItem( CPS_COMMANDCLOSE  , CPIT_COMMAND  , CPIF_NONE, CApplication::_PropItemCallback, (void*)this);

   //---Customize-------------------------------
   char szBuf[256];
   LoadString(GetInstance(), CPS_VXFLIPDIRFLIP, szBuf, sizeof(szBuf)/sizeof(char));
   pPropMgr->FindItemByID( CPS_VXFLIPDIR     )->SetItemToggle(szBuf);
   pPropMgr->FindItemByID( CPS_GRPRUNVAR     )->SetItemRadioList(0, CpszVar);

   pPropMgr->FindItemByID( CPS_CMDHEADER     )->ClearBit(CPIF_HIDDEN);
   pPropMgr->FindItemByID( CPS_COMMANDNEW    )->ClearBit(CPIF_HIDDEN);
   pPropMgr->FindItemByID( CPS_COMMANDNEWPROP)->ClearBit(CPIF_HIDDEN);
   pPropMgr->FindItemByID( CPS_COMMANDOPEN   )->ClearBit(CPIF_HIDDEN);
   pPropMgr->FindItemByID( CPS_COMMANDCLOSE  )->ClearBit(CPIF_HIDDEN);

   //---Update----------------------------------
   EnableMenus(-1);                         // set default button states
   UpdateProperties();                      // update my properties
   pPropMgr->OnResize();                    // position items, update actives


   //===Finalize==========================================
   ShowWindow(hwApp, iShow);                // show the window
}


/*********************************************************
* Destructor
*********************************************************/
CApplication::~CApplication() {
   int k;                                   // loop counter
   //---Destroy all systems---------------------
///TODO: Add "Save Changes?"-method of closing systems

   //---Destroy others--------------------------
   if(pButtonBar) delete(pButtonBar); pButtonBar = NULL;
   if(pStatusBar) delete(pStatusBar); pStatusBar = NULL;
   if(pPropMgr  ) delete(pPropMgr  ); pPropMgr   = NULL;

   //---Close window----------------------------
   if(hwMDIClient) DestroyWindow(hwMDIClient); hwMDIClient = NULL;
   if(hwApp      ) DestroyWindow(hwApp      ); hwApp       = NULL;

   //---Global GDI------------------------------
   for(k=CAPPC_NUM_BMP -1; k>=0; k--) { if(hbmAppBitmap[k]) DeleteObject(hbmAppBitmap[k]); hbmAppBitmap[k] = NULL; }
   for(k=CAPPC_NUM_CUR -1; k>=0; k--) { if(hcAppCursor[k] ) DeleteObject(hcAppCursor[k] ); hcAppCursor[k]  = NULL; }
   for(k=CAPPC_NUMFONT -1; k>=0; k--) { if(hfAppFont[k]   ) DeleteObject(hfAppFont[k]   ); hfAppFont[k]    = NULL; }
   for(k=CAPPC_NUMBRUSH-1; k>=0; k--) { if(hbrAppBrush[k] ) DeleteObject(hbrAppBrush[k] ); hbrAppBrush[k]  = NULL; }
   for(k=CAPPC_NUMPEN  -1; k>=0; k--) { if(hpAppPen[k]    ) DeleteObject(hpAppPen[k]    ); hpAppPen[k]     = NULL; }
   // colorrefs don't need to be freed

printf("~CApp~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");};


/*********************************************************
* MessageLoop
* It is assumed that by the time we get here the relevant
* windows (specifically, hwMDIClient) have been created.
*********************************************************/
int CApplication::MessageLoop(void) {
   MSG    msg;                              // message structure
   HACCEL hAccel;                           // accelerator table

   hAccel = LoadAccelerators(hInstance, "LASRCANV_ACCEL");

   while( GetMessage(&msg, NULL, 0, 0) ) {
      if(TranslateMDISysAccel(hwMDIClient, &msg)) continue;
      if(TranslateAccelerator(hwApp, hAccel, &msg)) continue;
      TranslateMessage(&msg);
      DispatchMessage(&msg);
   }
   return((int) msg.wParam);                // loop exits with PostQuitMessage
}


/*********************************************************
* WndProcMain
* Window procedure for main window. This will be the win-
* dow that handles all the window messages
* Since this is an MDI frame window, it's a bit different
* (see the MSDN help). We  pass unhandled messages to the
* DefFrameProc, plus also the following, even if they are
* handled:
*    WM_COMMAND
*    WM_MENUCHAR
*    WM_SETFOCUS
*    WM_SIZE
* (I'm not passing WM_SIZE since I'm managing the MDI po-
* sition manually.)
* This callback function must be declared static.
*********************************************************/
LRESULT CALLBACK CApplication::WndProcMain(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
   CApplication *pApp;                      // application object stored on window

   pApp = (CApplication*) GetWindowLong(hWnd, 0); // not valid on WM_CREATE
   switch(uMsg) {
   case WM_CREATE:
      pApp = (CApplication*) (((LPCREATESTRUCT) lParam)->lpCreateParams);
      SetWindowLong(hWnd, 0, (LONG) pApp);  // save object pointer to window
      break;
   case WM_CLOSE:
      if(pApp->UserClose()) PostQuitMessage(0); // check ok to close, then exit
      break;

   case WM_SIZE:       pApp->OnResize(); break;
   case WM_COMMAND:    pApp->ProcessCommand(LOWORD(wParam)); break;
   case WM_MENUSELECT: pApp->MenuItemSelect(LOWORD(wParam)); break;
   default:
      if((pApp!=NULL) && (pApp->hwMDIClient!=NULL)) {
         return( DefFrameProc(hWnd, pApp->hwMDIClient, uMsg, wParam, lParam) );
      } else {
         return( DefWindowProc(hWnd, uMsg, wParam, lParam) );
      }
   }
   return(0L);
}

/*********************************************************
* MenuPrint
* Called in response to the  Print menu. Handles the com-
* mon dialog, as well as preparing for printing. The main
* font is  temporarily replaced by one more  suitable for
* the printer resolution.
*********************************************************/
void CApplication::MenuPrint(BOOL tfAll) {
   PRINTDLG pd;                             // dialog structure
   DOCINFO  di;                             // structure for StartDoc
   int      iRes;                           // horizontal resolution pixels per inch
   int      iPage;                          // page counter (accessed by CSystem::PrintAllRenderer)

   //===Prepare===========================================
   if(GetCurrentRenderer()==NULL) return;   // ignore if no renderer
   if(GetCurrentSystem()==NULL) return;     // ignore if no system

   //---Dialog----------------------------------
   memset(&pd, 0x00, sizeof(pd));           // clear whole structure
   pd.lStructSize = sizeof(pd);
   pd.hwndOwner   = hwApp;
   pd.Flags       = PD_RETURNDC | PD_NOPAGENUMS | PD_NOSELECTION
                     | PD_USEDEVMODECOPIESANDCOLLATE;
   pd.nCopies     = 1;
   if(PrintDlg(&pd) ==0) return;            // return on cancel

   //---Graphics Objects------------------------
   // We want a 10-point font, so 10/72 * resolution
   iRes = GetDeviceCaps(pd.hDC, LOGPIXELSX); // resolution (pixels / inch)
   hfAppFont[CAPPI_FONTPRINT] = CreateFont(-10*iRes/72,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Verdana");

   //---Document Structure----------------------
   di.cbSize       = sizeof(di);
   di.lpszDocName  = GetCurrentSystem()->FileName();
   di.lpszOutput   = NULL;
   di.lpszDatatype = NULL;
   di.fwType       = 0;
   StartDoc(pd.hDC, &di);

   //===Print Collated====================================
   iPage = 1;                               // start counting first page
   //---All pages-------------------------------
   if(tfAll) {
      GetCurrentSystem()->GetTopSystem()->PrintAllRenderer(pd.hDC, &iPage);

   //---Contents--------------------------------
   } else {
      if(GetCurrentRenderer()) GetCurrentRenderer()->Print(pd.hDC);
      PrintHeaderFooter(pd.hDC, GetCurrentRenderer()->System()->FileName(), &iPage);
      EndPage(pd.hDC);
   }

   //===Finalize==========================================
   //---Document Structure----------------------
   EndDoc(pd.hDC);

   //---Graphics Objects------------------------
   DeleteDC(pd.hDC);
   DeleteObject(hfAppFont[CAPPI_FONTPRINT]); hfAppFont[CAPPI_FONTPRINT] = NULL;
}


/*********************************************************
* PrintHeaderFooter
* Called from
* <- MenuPrint (single document)
* <- CSystem::PrintAllRenderer
*    <- MenuPrint (all documents)
*********************************************************/
void CApplication::PrintHeaderFooter(HDC hdcPrint, const char *pszFileName, int *piPage) {
   char   szBuf[256];                       // formatted text
   HFONT  hfOld;                            // restore font
   int    iAlgnOld;                         // restore alignment

   iAlgnOld = SetTextAlign(hdcPrint, TA_CENTER);
   hfOld    = (HFONT) SelectObject(hdcPrint, Font(CAPPI_FONTPRINT));

   //---Header----------------------------------
   TextOut(hdcPrint,
      GetDeviceCaps(hdcPrint, HORZRES)/2,
      1.1*GetDeviceCaps(hdcPrint, PHYSICALOFFSETY),
      pszFileName, strlen(pszFileName));

   //---Footer----------------------------------
   if(piPage) {
      sprintf(szBuf, "--%d--", *piPage);
      TextOut(hdcPrint,
         GetDeviceCaps(hdcPrint, HORZRES)/2,
         GetDeviceCaps(hdcPrint, VERTRES) - 1.1*GetDeviceCaps(hdcPrint, PHYSICALOFFSETY),
         szBuf, strlen(szBuf));
   }

   //---Finish----------------------------------
   SetTextAlign(hdcPrint, iAlgnOld);
   SelectObject(hdcPrint, hfOld);

}

/*********************************************************
* ProcessCommand
* This is called on WM_COMMAND but could conceivably also
* be called directly. (It's  probably better to use Send-
* Message, though.)
* Handles menu (and non-menu?) commands, defined as CMU_-
* constants in resource.h
*********************************************************/
///TODO: Complete functionality
///TODO: Menus in languages
void CApplication::ProcessCommand(int iCmd) {
   char szWndTitle[64];                     // window title
   char *pszWnd;                            // pointer to window name
   MENUITEMINFO mii;                        // info for window call
   CSystem *pSys;                           // new system object
   HWND     hwAct;                          // child window to activate

   //===MDI Child activation==============================
   // This is not very elegant but I don't care just now.
   // Note: If we pull this out of an MDI and have to ma-
   // nage our  own windows, we  may as well use  the dw-
   // ItemData!
   if(iCmd >= CMU_WINDOW_MDICHILD) {
      memset(&mii, 0x00, sizeof(mii));      // clear the structure
      mii.cbSize     = sizeof(mii);         // structure size
      mii.fMask      = MIIM_TYPE;           // get menu type info
      mii.fType      = MFT_STRING;          // get the menu item string
      mii.wID        = iCmd;                // item ID
      mii.dwTypeData = szWndTitle;          // buffer to fill
      mii.cch        = sizeof(szWndTitle) / sizeof(char); // length of buffer
      GetMenuItemInfo(GetMenu(hwApp), iCmd, FALSE, &mii);
      pszWnd = strchr(szWndTitle, ' ');     // szWndTitle has form "&1 Canvas Untitled1"
      if(pszWnd==NULL) return;              // problem if we can't find space
      pszWnd++;                             // skip the space, too
      do {                                  // find window type
         if((hwAct=FindWindowEx(hwMDIClient, NULL, CSZ_WNDCLASS_SYSWIN2D         , pszWnd))!=NULL) break;
         if((hwAct=FindWindowEx(hwMDIClient, NULL, CSZ_WNDCLASS_SYSWIN1D         , pszWnd))!=NULL) break;
         if((hwAct=FindWindowEx(hwMDIClient, NULL, CSZ_WNDCLASS_SYSWIN3D         , pszWnd))!=NULL) break;
         if((hwAct=FindWindowEx(hwMDIClient, NULL, CSZ_WNDCLASS_SYSWINGRAPH      , pszWnd))!=NULL) break;
         if((hwAct=FindWindowEx(hwMDIClient, NULL, CSZ_WNDCLASS_SYSWINVXGRAPH    , pszWnd))!=NULL) break;
         if((hwAct=FindWindowEx(hwMDIClient, NULL, CSZ_WNDCLASS_SYSWININVENTORY  , pszWnd))!=NULL) break;
         if((hwAct=FindWindowEx(hwMDIClient, NULL, CSZ_WNDCLASS_SYSWINABCDSOLVER , pszWnd))!=NULL) break;
      } while(0);                           // no repeats
      if(hwAct) ActivateSysWinWindow(hwAct); // activate it, if found
      return;                               // and, we're done
   }

   switch(iCmd) {
   //===File Menu=========================================
   case CMU_FILE_NEWRESO:
   case CMU_FILE_NEWPROP:
      pSys = UserNewSystem();               // create a new system
      pSys->SetDefaultName();               // set default name for top level system
      pSys->UserCreateSystem(
         (iCmd==CMU_FILE_NEWRESO) ? CSYS_TYPE_RESO :
         (iCmd==CMU_FILE_NEWPROP) ? CSYS_TYPE_PROP :
         CSYS_TYPE_RESO);                   // insert resonator optics
      pSys->UserCreateSysWin2d();           // create 2d renderer
      pPropMgr->OnPaint(TRUE);              // update text
      break;
   case CMU_FILE_CLOSEMODEL:  UserCloseSystem(NULL); break; // close top level system
   case CMU_FILE_CLOSESYSTEM: UserCloseSystem(GetCurrentSystem()); break; // close current
   case CMU_FILE_OPEN:        UserLoadSystem();      break;
   case CMU_FILE_SAVE:        UserSaveSystem(FALSE); break;
   case CMU_FILE_SAVEAS:      UserSaveSystem(TRUE);  break;
   case CMU_FILE_PRINTSETUP: break;
   case CMU_FILE_PRINT:       MenuPrint(FALSE); break;
   case CMU_FILE_PRINTALL:    MenuPrint(TRUE);  break;
   case CMU_FILE_EXIT:        SendMessage(hwApp, WM_CLOSE, 0, 0L); break;

   //===System Menus======================================
   case CMU_INS_MIRR:  if(GetCurrentSystem()) GetCurrentSystem()->MenuInsertVx(CVX_TYPE_MIRROR);     break;
   case CMU_INS_LENS:  if(GetCurrentSystem()) GetCurrentSystem()->MenuInsertVx(CVX_TYPE_LENS);       break;
   case CMU_INS_CRYS:  if(GetCurrentSystem()) GetCurrentSystem()->MenuInsertVx(CVX_TYPE_INCRYSTAL);  break;
   case CMU_INS_BREW:  if(GetCurrentSystem()) GetCurrentSystem()->MenuInsertVx(CVX_TYPE_INBREWSTER); break;
   case CMU_INS_PLAT:  if(GetCurrentSystem()) GetCurrentSystem()->MenuInsertVx(CVX_TYPE_INPLATE);    break;
   case CMU_INS_PRIP:  if(GetCurrentSystem()) GetCurrentSystem()->MenuInsertVx(CVX_TYPE_PRISM1);     break;
   case CMU_INS_SCRN:  if(GetCurrentSystem()) GetCurrentSystem()->MenuInsertVx(CVX_TYPE_SCREEN);     break;
   case CMU_DEL_OPTI:  if(GetCurrentSystem()) GetCurrentSystem()->MenuDeleteVx(); break;
   case CMU_INS_SPAWN: if(GetCurrentSystem()) GetCurrentSystem()->MenuInsertSpawn(); break;

   //===Renderer Menus====================================
   case CMU_COPYDATA:
   case CMU_TOOL_ARROW:
   case CMU_TOOL_MEASURE:
   case CMU_TOOL_ZOOM:
   case CMU_TOOL_PAN:
   case CMU_TOOL_ROTATE:
   case CMU_CNV_ZOOMIN:
   case CMU_CNV_ZOOMOUT:
   case CMU_TOOL_DRAFT:
       if(GetCurrentRenderer()!=NULL) GetCurrentRenderer()->MenuCommand(iCmd);
       break;

   //===View Menu=========================================
   case CMU_VIEW_MODESIZE:  if(GetCurrentSystem()!=NULL) ActivateSysWinWindow( (GetCurrentSystem()->UserCreateSysWin1d()         ) ->Window()); ScanAll(); break;
   case CMU_VIEW_SYSGRAPH:  if(GetCurrentSystem()!=NULL) ActivateSysWinWindow( (GetCurrentSystem()->UserCreateSysWinGraph()      ) ->Window()); ScanAll(); break;
   case CMU_VIEW_VXGRAPH:   if(GetCurrentSystem()!=NULL) ActivateSysWinWindow( (GetCurrentSystem()->UserCreateSysWinVxGraph(TRUE)) ->Window()); ScanAll(); break;
   case CMU_VIEW_INVENTORY: if(GetCurrentSystem()!=NULL) ActivateSysWinWindow( (GetCurrentSystem()->UserCreateSysWinInventory()  ) ->Window()); ScanAll(); break;
   case CMU_VIEW_SOLVER:    if(GetCurrentSystem()!=NULL) ActivateSysWinWindow( (GetCurrentSystem()->UserCreateSysWinABCDSolver() ) ->Window()); ScanAll(); break;
   case CMU_VIEW_RENDER3D:  if(GetCurrentSystem()!=NULL) ActivateSysWinWindow( (GetCurrentSystem()->UserCreateSysWin3d()         ) ->Window()); ScanAll(); break;
   case CMU_VIEW_PROPEQTN:   UserViewPropEqtn((GetMenuState(GetMenu(hwApp), CMU_VIEW_PROPEQTN, MF_BYCOMMAND) & MF_CHECKED) ? FALSE : TRUE); break;
   case CMU_VIEW_PROPERTIES: UserViewProperties((GetMenuState(GetMenu(hwApp), CMU_VIEW_PROPERTIES, MF_BYCOMMAND) & MF_CHECKED) ? FALSE : TRUE); break;
   case CMU_VIEW_DOCKPROP:   UserDockProperties((GetMenuState(GetMenu(hwApp), CMU_VIEW_DOCKPROP, MF_BYCOMMAND) & MF_CHECKED) ? FALSE : TRUE); break;


   //===Window Menu=======================================
   case CMU_WINDOW_TILE:    if(hwMDIClient) SendMessage(hwMDIClient, WM_MDITILE, MDITILE_HORIZONTAL, 0L ); break;
   case CMU_WINDOW_CASCADE: if(hwMDIClient) SendMessage(hwMDIClient, WM_MDICASCADE, 0, 0L); break;
   case CMU_WINDOW_ARGICON: if(hwMDIClient) SendMessage(hwMDIClient, WM_MDIICONARRANGE, 0, 0L); break;
   case CMU_WINDOW_RESTORE: if((hwMDIClient) && GetCurrentRenderer()) SendMessage(hwMDIClient, WM_MDIRESTORE, (WPARAM) GetCurrentRenderer()->Window(), 0L); break;

   //===Help Menu=========================================
   case CMU_HELP_HELP: Help("lc5.chm"); break;
   case CMU_HELP_ABOUT:
      if((GetAsyncKeyState(VK_SHIFT)&0x8000) && (GetAsyncKeyState(VK_CONTROL)&0x8000))
         AboutProc(NULL, NULL, (WPARAM) hwApp, (LPARAM) hInstance);
      else About();
      break;
   }
}


/*********************************************************
* MenuItemSelect
* Called on WM_MENUSELECT to display the RC string in the
* status bar.
*
* WM_MENUSELECT
* uItem = (UINT) LOWORD(wParam);   // menu item or submenu index
* fuFlags = (UINT) HIWORD(wParam); // menu flags
* hmenu = (HMENU) lParam;          // handle to menu clicked
*********************************************************/
void CApplication::MenuItemSelect(UINT uID) {
   char szBuf[256];                         // string from rc
   if(pStatusBar==NULL) return;             // ignore if no status bar
   if(uID==0x0000) {
      pStatusBar->SetStandardText(NULL);
   } else {
      if( LoadString(hInstance, uID, szBuf, sizeof(szBuf) / sizeof(char)) > 0) {
         pStatusBar->SetStandardText(szBuf);
      }
   }
}

/*********************************************************
* EnableMenus
* For the check/disable call (-1) all menu items that de-
* pend on a specific renderer, as well as Save/Close, are
* disabled. They are then enabled when specific renderers
* are activated.
* Called from
* <- CApp::WM_INITMENU (-2)
* <- CApp::Constructor (-1)
* <- Renderer::PrepareProperties (deactivation: -1)
* <- Renderer::PrepareProperties (activation: renderer type)
*********************************************************/
void CApplication::EnableMenus(int iRenderer) {
   BOOL tfEn;                               // TRUE if there's a current system

   switch(iRenderer) {
   case -1:                                 // disable any that are activated
      EnableMenuItem(GetMenu(hwApp), CMU_FILE_SAVE       , MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(GetMenu(hwApp), CMU_FILE_SAVEAS     , MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(GetMenu(hwApp), CMU_FILE_CLOSEMODEL , MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(GetMenu(hwApp), CMU_FILE_CLOSESYSTEM, MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(GetMenu(hwApp), CMU_FILE_PRINT      , MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(GetMenu(hwApp), CMU_FILE_PRINTALL   , MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(GetMenu(hwApp), CMU_COPYDATA        , MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(GetMenu(hwApp), CMU_INS_MIRR        , MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(GetMenu(hwApp), CMU_INS_LENS        , MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(GetMenu(hwApp), CMU_INS_CRYS        , MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(GetMenu(hwApp), CMU_INS_BREW        , MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(GetMenu(hwApp), CMU_INS_PLAT        , MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(GetMenu(hwApp), CMU_INS_PRIP        , MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(GetMenu(hwApp), CMU_INS_SCRN        , MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(GetMenu(hwApp), CMU_DEL_OPTI        , MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(GetMenu(hwApp), CMU_INS_SPAWN       , MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(GetMenu(hwApp), CMU_TOOL_ARROW      , MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(GetMenu(hwApp), CMU_TOOL_MEASURE    , MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(GetMenu(hwApp), CMU_TOOL_ZOOM       , MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(GetMenu(hwApp), CMU_TOOL_PAN        , MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(GetMenu(hwApp), CMU_TOOL_ROTATE     , MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(GetMenu(hwApp), CMU_TOOL_DRAFT      , MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(GetMenu(hwApp), CMU_CNV_ZOOMIN      , MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(GetMenu(hwApp), CMU_CNV_ZOOMOUT     , MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(GetMenu(hwApp), CMU_VIEW_MODESIZE   , MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(GetMenu(hwApp), CMU_VIEW_SYSGRAPH   , MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(GetMenu(hwApp), CMU_VIEW_VXGRAPH    , MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(GetMenu(hwApp), CMU_VIEW_INVENTORY  , MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(GetMenu(hwApp), CMU_VIEW_SOLVER     , MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(GetMenu(hwApp), CMU_VIEW_RENDER3D   , MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(GetMenu(hwApp), CMU_VIEW_PROPEQTN   , MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(GetMenu(hwApp), CMU_WINDOW_TILE     , MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(GetMenu(hwApp), CMU_WINDOW_CASCADE  , MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(GetMenu(hwApp), CMU_WINDOW_ARGICON  , MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(GetMenu(hwApp), CMU_WINDOW_RESTORE  , MF_BYCOMMAND | MF_GRAYED);
      if(pButtonBar) {
         pButtonBar->FindButtonByData(CMU_FILE_SAVE       )->EnableButton(FALSE);
         pButtonBar->FindButtonByData(CMU_FILE_PRINT      )->EnableButton(FALSE);
         pButtonBar->FindButtonByData(CMU_INS_MIRR        )->EnableButton(FALSE);
         pButtonBar->FindButtonByData(CMU_INS_LENS        )->EnableButton(FALSE);
         pButtonBar->FindButtonByData(CMU_INS_PLAT        )->EnableButton(FALSE);
         pButtonBar->FindButtonByData(CMU_INS_BREW        )->EnableButton(FALSE);
         pButtonBar->FindButtonByData(CMU_INS_CRYS        )->EnableButton(FALSE);
         pButtonBar->FindButtonByData(CMU_INS_PRIP        )->EnableButton(FALSE);
         pButtonBar->FindButtonByData(CMU_INS_SCRN        )->EnableButton(FALSE);
         pButtonBar->FindButtonByData(CMU_DEL_OPTI        )->EnableButton(FALSE);
         pButtonBar->FindButtonByData(CMU_INS_SPAWN       )->EnableButton(FALSE);
         pButtonBar->FindButtonByData(CMU_TOOL_ARROW      )->EnableButton(FALSE);
         pButtonBar->FindButtonByData(CMU_TOOL_MEASURE    )->EnableButton(FALSE);
         pButtonBar->FindButtonByData(CMU_TOOL_ZOOM       )->EnableButton(FALSE);
         pButtonBar->FindButtonByData(CMU_TOOL_PAN        )->EnableButton(FALSE);
         pButtonBar->FindButtonByData(CMU_TOOL_ROTATE     )->EnableButton(FALSE);
         pButtonBar->FindButtonByData(CMU_TOOL_DRAFT      )->EnableButton(FALSE);
         pButtonBar->FindButtonByData(CMU_VIEW_MODESIZE   )->EnableButton(FALSE);
         pButtonBar->FindButtonByData(CMU_VIEW_SYSGRAPH   )->EnableButton(FALSE);
         pButtonBar->FindButtonByData(CMU_VIEW_VXGRAPH    )->EnableButton(FALSE);
         pButtonBar->FindButtonByData(CMU_VIEW_INVENTORY  )->EnableButton(FALSE);
         pButtonBar->FindButtonByData(CMU_VIEW_SOLVER     )->EnableButton(FALSE);
         pButtonBar->FindButtonByData(CMU_VIEW_RENDER3D   )->EnableButton(FALSE);
         pButtonBar->FindButtonByData(CMU_VIEW_PROPEQTN   )->EnableButton(FALSE);
      }
      break;

   case CSYSWINI_TYPE_2D:
      if(GetCurrentSystem()==NULL) break;   // this shouldn't happen
      //---Selection-dependent------------------
      //---Grayed in Draft Mode-------
      tfEn = (GetCurrentSystem()->NumSelectedVx(NULL)>0) && (!GetCurrentSystem()->DraftMode());
      EnableMenuItem(GetMenu(hwApp), CMU_INS_PRIP        , MF_BYCOMMAND | (tfEn ? MF_ENABLED : MF_GRAYED));
      if(pButtonBar) {
         pButtonBar->FindButtonByData(CMU_INS_PRIP        )->EnableButton(tfEn ? TRUE : FALSE);
      }

      //---Enabled in Draft Mode------
      tfEn = (GetCurrentSystem()->NumSelectedVx(NULL)>0) || (GetCurrentSystem()->DraftMode());
      EnableMenuItem(GetMenu(hwApp), CMU_INS_MIRR        , MF_BYCOMMAND | (tfEn ? MF_ENABLED : MF_GRAYED));
      EnableMenuItem(GetMenu(hwApp), CMU_INS_LENS        , MF_BYCOMMAND | (tfEn ? MF_ENABLED : MF_GRAYED));
      EnableMenuItem(GetMenu(hwApp), CMU_INS_CRYS        , MF_BYCOMMAND | (tfEn ? MF_ENABLED : MF_GRAYED));
      EnableMenuItem(GetMenu(hwApp), CMU_INS_BREW        , MF_BYCOMMAND | (tfEn ? MF_ENABLED : MF_GRAYED));
      EnableMenuItem(GetMenu(hwApp), CMU_INS_PLAT        , MF_BYCOMMAND | (tfEn ? MF_ENABLED : MF_GRAYED));
      EnableMenuItem(GetMenu(hwApp), CMU_INS_SCRN        , MF_BYCOMMAND | (tfEn ? MF_ENABLED : MF_GRAYED));
      EnableMenuItem(GetMenu(hwApp), CMU_DEL_OPTI        , MF_BYCOMMAND | (tfEn ? MF_ENABLED : MF_GRAYED));
      if(pButtonBar) {
         pButtonBar->FindButtonByData(CMU_INS_MIRR        )->EnableButton(tfEn ? TRUE : FALSE);
         pButtonBar->FindButtonByData(CMU_INS_LENS        )->EnableButton(tfEn ? TRUE : FALSE);
         pButtonBar->FindButtonByData(CMU_INS_PLAT        )->EnableButton(tfEn ? TRUE : FALSE);
         pButtonBar->FindButtonByData(CMU_INS_BREW        )->EnableButton(tfEn ? TRUE : FALSE);
         pButtonBar->FindButtonByData(CMU_INS_CRYS        )->EnableButton(tfEn ? TRUE : FALSE);
         pButtonBar->FindButtonByData(CMU_INS_SCRN        )->EnableButton(tfEn ? TRUE : FALSE);
         pButtonBar->FindButtonByData(CMU_DEL_OPTI        )->EnableButton(tfEn ? TRUE : FALSE);
      }

      //---Tools--------------------------------
      //---Grayed in Draft Mode-------
      tfEn = (!GetCurrentSystem()->DraftMode());
      EnableMenuItem(GetMenu(hwApp), CMU_TOOL_ROTATE     , MF_BYCOMMAND | (tfEn ? MF_ENABLED : MF_GRAYED));
      EnableMenuItem(GetMenu(hwApp), CMU_TOOL_MEASURE    , MF_BYCOMMAND | (tfEn ? MF_ENABLED : MF_GRAYED));
      if(pButtonBar) {
         pButtonBar->FindButtonByData(CMU_TOOL_MEASURE    )->EnableButton(tfEn ? TRUE : FALSE);
         pButtonBar->FindButtonByData(CMU_TOOL_ROTATE     )->EnableButton(tfEn ? TRUE : FALSE);
      }

      //---Enabled in any mode--------
      EnableMenuItem(GetMenu(hwApp), CMU_FILE_PRINT      , MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(GetMenu(hwApp), CMU_TOOL_ARROW      , MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(GetMenu(hwApp), CMU_TOOL_ZOOM       , MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(GetMenu(hwApp), CMU_TOOL_PAN        , MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(GetMenu(hwApp), CMU_CNV_ZOOMIN      , MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(GetMenu(hwApp), CMU_CNV_ZOOMOUT     , MF_BYCOMMAND | MF_ENABLED);
      if(pButtonBar) {
         pButtonBar->FindButtonByData(CMU_FILE_PRINT      )->EnableButton(TRUE);
         pButtonBar->FindButtonByData(CMU_TOOL_ARROW      )->EnableButton(TRUE);
         pButtonBar->FindButtonByData(CMU_TOOL_ZOOM       )->EnableButton(TRUE);
         pButtonBar->FindButtonByData(CMU_TOOL_PAN        )->EnableButton(TRUE);
      }

      //---Draft mode-----------------
      tfEn = (GetCurrentSystem()->SysType()==CSYS_TYPE_RESO) ? TRUE : FALSE;
      EnableMenuItem(GetMenu(hwApp), CMU_TOOL_DRAFT      , MF_BYCOMMAND | (tfEn ? MF_ENABLED : MF_GRAYED));
      if(pButtonBar) {
         pButtonBar->FindButtonByData(CMU_TOOL_DRAFT      )->EnableButton(tfEn ? TRUE : FALSE);
      }

      //---Spawn----------------------
      // allow only one level of recursion
      tfEn = (GetCurrentSystem()->SysType()==CSYS_TYPE_RESO) && (GetCurrentSystem()->VxSpawn()==NULL) && !GetCurrentSystem()->DraftMode();
      EnableMenuItem(GetMenu(hwApp), CMU_INS_SPAWN       , MF_BYCOMMAND | (tfEn ? MF_ENABLED : MF_GRAYED));
      if(pButtonBar) {
         pButtonBar->FindButtonByData(CMU_INS_SPAWN       )->EnableButton(tfEn ? TRUE : FALSE);
      }
      break;

   case CSYSWINI_TYPE_3D:
      EnableMenuItem(GetMenu(hwApp), CMU_TOOL_ZOOM       , MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(GetMenu(hwApp), CMU_TOOL_PAN        , MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(GetMenu(hwApp), CMU_TOOL_ROTATE     , MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(GetMenu(hwApp), CMU_FILE_PRINT      , MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(GetMenu(hwApp), CMU_CNV_ZOOMIN      , MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(GetMenu(hwApp), CMU_CNV_ZOOMOUT     , MF_BYCOMMAND | MF_ENABLED);
      if(pButtonBar) {
         pButtonBar->FindButtonByData(CMU_FILE_PRINT       )->EnableButton(TRUE);
         pButtonBar->FindButtonByData(CMU_TOOL_ZOOM       )->EnableButton(TRUE);
         pButtonBar->FindButtonByData(CMU_TOOL_PAN        )->EnableButton(TRUE);
         pButtonBar->FindButtonByData(CMU_TOOL_ROTATE     )->EnableButton(TRUE);
      }
      break;

   case CSYSWINI_TYPE_GRAPH:
   case CSYSWINI_TYPE_VXGRAPH:
      EnableMenuItem(GetMenu(hwApp), CMU_FILE_PRINT      , MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(GetMenu(hwApp), CMU_COPYDATA        , MF_BYCOMMAND | MF_ENABLED);
      if(pButtonBar) {
         pButtonBar->FindButtonByData(CMU_FILE_PRINT       )->EnableButton(TRUE);
      }
      break;

   case CSYSWINI_TYPE_1D:
   case CSYSWINI_TYPE_INVENTORY:
      EnableMenuItem(GetMenu(hwApp), CMU_FILE_PRINT      , MF_BYCOMMAND | MF_ENABLED);
      if(pButtonBar) {
         pButtonBar->FindButtonByData(CMU_FILE_PRINT       )->EnableButton(TRUE);
      }
      break;

   case CSYSWINI_TYPE_ABCDSOLVER:
      break;
   }

   //---Enabled for any renderer----------------
   if(iRenderer >= 0) {
      tfEn = ((GetCurrentSystem()) && (GetCurrentSystem()->NumSelectedVx(NULL)>0)) ? TRUE : FALSE;
      EnableMenuItem(GetMenu(hwApp), CMU_FILE_SAVE       , MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(GetMenu(hwApp), CMU_FILE_SAVEAS     , MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(GetMenu(hwApp), CMU_FILE_CLOSEMODEL , MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(GetMenu(hwApp), CMU_FILE_PRINTALL   , MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(GetMenu(hwApp), CMU_VIEW_PROPEQTN   , MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(GetMenu(hwApp), CMU_VIEW_MODESIZE   , MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(GetMenu(hwApp), CMU_VIEW_SYSGRAPH   , MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(GetMenu(hwApp), CMU_VIEW_VXGRAPH    , MF_BYCOMMAND | (tfEn ? MF_ENABLED : MF_GRAYED));
      EnableMenuItem(GetMenu(hwApp), CMU_VIEW_INVENTORY  , MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(GetMenu(hwApp), CMU_VIEW_SOLVER     , MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(GetMenu(hwApp), CMU_VIEW_RENDER3D   , MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(GetMenu(hwApp), CMU_WINDOW_TILE     , MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(GetMenu(hwApp), CMU_WINDOW_CASCADE  , MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(GetMenu(hwApp), CMU_WINDOW_ARGICON  , MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(GetMenu(hwApp), CMU_WINDOW_RESTORE  , MF_BYCOMMAND | MF_ENABLED);
      if(pButtonBar) {
         pButtonBar->FindButtonByData(CMU_FILE_SAVE       )->EnableButton(TRUE);
         pButtonBar->FindButtonByData(CMU_VIEW_MODESIZE   )->EnableButton(TRUE);
         pButtonBar->FindButtonByData(CMU_VIEW_SYSGRAPH   )->EnableButton(TRUE);
         pButtonBar->FindButtonByData(CMU_VIEW_VXGRAPH    )->EnableButton(tfEn ? TRUE : FALSE);
         pButtonBar->FindButtonByData(CMU_VIEW_PROPEQTN   )->EnableButton(TRUE);
         pButtonBar->FindButtonByData(CMU_VIEW_INVENTORY  )->EnableButton(TRUE);
         pButtonBar->FindButtonByData(CMU_VIEW_SOLVER     )->EnableButton(TRUE);
         pButtonBar->FindButtonByData(CMU_VIEW_RENDER3D   )->EnableButton(TRUE);
      }
   }
   if(pButtonBar) pButtonBar->InvalidateBarRect(NULL); // re-paint the bar
}

/*********************************************************
* CheckMenu
* Sets or clears menu check marks.
* Called from, e.g.
* <- CSysWin2d::MenuCommand
*    <- App::ProcessCommand
*       <- Tool menu command
*********************************************************/
void CApplication::CheckMenu(int iCmd, BOOL tfChecked) {
   CheckMenuItem(GetMenu(hwApp), iCmd, MF_BYCOMMAND | (tfChecked ? MF_CHECKED : MF_UNCHECKED));
   if((pButtonBar) && (pButtonBar->FindButtonByData(iCmd))) {
      pButtonBar->FindButtonByData(iCmd)->ToggleButton(tfChecked ? TRUE : FALSE);
      InvalidateRect(pButtonBar->GetBarWindow(), NULL, TRUE);
   }
}

/*********************************************************
* Help
* Since there seems to be some issue getting HtmlHel() to
* work with Borland, we launch hh.exe manually here.
*********************************************************/
void CApplication::Help(const char *szHelpFile) {
   const char szHelpApp[] = "hh.exe";       // help application name
   char szPath[512];                        // path to application
   char szWinPath[512];                     // System directory
   char szAppName[512];                     // application to launch
   char szCmd[1024];                        // command line
   char *psz;                               // string manipulation pointer

   //---Get paths-------------------------------
   GetWindowsDirectory(szWinPath, sizeof(szWinPath)/sizeof(char));

   GetModuleFileName(NULL, szPath, sizeof(szPath)/sizeof(char));
   psz = szPath + strlen(szPath) - 1;
   while((psz>=szPath) && (*psz != '\\')) *(psz--)='\0';
   if(psz<=szPath) {
      LoadString(GetInstance(), SZERR_HELP_FILE, szPath, sizeof(szPath)/sizeof(char));
      MessageBox(hwApp, szPath, "Error", MB_OK | MB_ICONEXCLAMATION);
      return;
   }
   sprintf(szAppName, "%s\\%s ", szWinPath, szHelpApp);
   sprintf(szCmd, "\"%s\\%s\" ", szWinPath, szHelpApp);
   sprintf(szCmd+strlen(szCmd), "\"%s%s\"", szPath, szHelpFile);

   //---Get path to HH.exe----------------------
   STARTUPINFO si;
   PROCESS_INFORMATION pi;
   memset(&si, 0x00, sizeof(si));
   si.cb = sizeof(si);

   memset(&pi, 0x00, sizeof(pi));
   if( CreateProcess(szAppName,
      szCmd, NULL, NULL, FALSE, 0,
      szPath, NULL, &si, &pi) == 0) {
      LoadString(GetInstance(), SZERR_HELP_LAUNCH, szPath, sizeof(szPath)/sizeof(char));
      MessageBox(hwApp, szPath, "Error", MB_OK | MB_ICONEXCLAMATION);
   }
}


/*********************************************************
* About
* Display the About dialog box
*********************************************************/
void CApplication::About(void) {
   MessageBox(hwApp, "\
LaserCanvas 5\n\
\n\
Interactive ABCD solver for design and teaching.\n\
\n\
(c) 2000-2007 Philip Schlup, PhD <pschlup@hotmail.com>\n\
Written In:\n\
 - Borland C++BuilderX\n\
 - Borland Resource Workshop 1.02\n\
 - Borland C/C++ v. 4.5 1.02\n\
 - Mathworks Matlab 1.02\n\
 - Apple XCode 1.02\n\
 - Microsoft HTML Help Workshop v. 4.74\n\
\n\
If you wish to cite the use of this program, please use\n\
\"LaserCanvas cavity modeling software, available from P.\n\
Schlup, Colorado State University, Fort Collins, CO 80523.\"\n\
\n\
My thanks to family and friends for unwaivering support!\
", "About", MB_OK | MB_ICONINFORMATION);
}

/*********************************************************
* OnResize
* Called on WM_SIZE messages  when the window is resized.
* Each of  the OnParentResize  functions modify  the sup-
* plied rectangle to what  space is remaining in the app-
* plication window; in this way, we can daisy chain child
* window calls and extend it later if necessary.
* Under normal conditions the property manager object has
* been instantiated,  so we call its  OnParentResize fun-
* ction that also positions the workspace window.
*********************************************************/
void CApplication::OnResize(void) {
   RECT rcClient;                           // client window rectangle
   GetClientRect(hwApp, &rcClient);         // read client coordinates
   if(pStatusBar)  pStatusBar->OnParentResize(&rcClient); // position bar, shrink remaining
   if(pButtonBar)  pButtonBar->OnParentResize(&rcClient); // position bar, shrink remaining
   if(pPropMgr)    pPropMgr->OnParentResize(&rcClient); // position manager, shrink remaining
   if(hwMDIClient) MoveWindow(hwMDIClient, rcClient.left, rcClient.top,
      rcClient.right-rcClient.left, rcClient.bottom-rcClient.top, TRUE);
}


/*########################################################
 ## User Functions                                     ##
########################################################*/
// User functions  are wrappers  that are  called in res-
// ponse to menu commands. They maintain the GUI elements
// such as menu check  marks and  toolbar states and call
// the non-User  function(s)s that perform  the necessary
// tasks. This makes them very  useful for direct calling
// for 'simulated' menu commands.
/*********************************************************
* UserNewSystem
* Use this instead of
*    new CSystem()
* to ensure the application's system chain is maintained.
*********************************************************/
CSystem* CApplication::UserNewSystem(void) {
   CSystem *pSys;                           // system loop counter
   CSystem *pSysNew;                        // newly created system
   //---Create system-----------------
   pSysNew = new CSystem(this);             // create a new system
   //---Maintain chain----------------
   if(pSysTop==NULL) {                      // first system..
      pSysTop = pSysNew;                    //..set to top of chain
   } else {
      for(pSys=pSysTop; pSys->Next()!=NULL; pSys=pSys->Next()); // find last element..
      pSys->SetNext(pSysNew);               //..and add this to end
   }
   return(pSysNew);
}

/*********************************************************
* UserDeleteSystem
* Use this instead of
*    delete pSysDel
* to ensure the application's system chain is maintained.
*********************************************************/
void CApplication::UserDeleteSystem(CSystem *pSysDel) {
   CSystem *pSys;                           // system loop counter
   //---Maintain chain----------------
   if(pSysTop==pSysDel) {                   // if it's at the top..
      pSysTop = pSysDel->Next();            //..set top to next in list
   } else {
      for(pSys=pSysTop; (pSys!=NULL)&&(pSys->Next()!=NULL); pSys=pSys->Next()) { // found in chain..
         if(pSys->Next()==pSysDel) pSys->SetNext(pSysDel->Next()); //..jump chain pointer
      }
   }

   //---Delete system-----------------
   delete(pSysDel);
}

/*********************************************************
* UserClose
* This is called when the user  selects to close the main
* application window. We should close all the open system
* objects, querying the user  if changes should be saved,
* and return FALSE if the user Cancels at any time.
*********************************************************/
BOOL CApplication::UserClose(void) {
   CSystem *pSys;
///TODO: User-Close all systems
   //---Close all systems-----------------------
   while((pSys=GetCurrentSystem()) != NULL) {
      while(pSys->GetParentSystem()) pSys = pSys->GetParentSystem(); // go to top systems only
      if( !pSys->UserCloseSystem() ) return(FALSE); // user canceled
      UserDeleteSystem(pSys);
   }
   return(TRUE);
}



/*********************************************************
* Property Manager User Functions
*********************************************************/
//===UserViewPropEqtn=====================================
void CApplication::UserViewPropEqtn(BOOL tfEqtn) {
   if(pPropMgr==NULL) return;               // ignore if no property manager
   if(tfEqtn) pPropMgr->SetBit(CPMF_EQTNSRC);
   else       pPropMgr->ClearBit(CPMF_EQTNSRC);
   CheckMenu( CMU_VIEW_PROPEQTN, (pPropMgr->CheckBit(CPMF_EQTNSRC)) ? TRUE : FALSE);
   InvalidateRect(pPropMgr->Window(), NULL, TRUE);
}

//===UserViewProperties===================================
// Also  called by the property manager to hide itself.
void CApplication::UserViewProperties(BOOL tfView) {
   if(pPropMgr==NULL) return;               // ignore if no property manager
   if(tfView) {
      pPropMgr->CreateMgrWindow();
      CheckMenuItem(GetMenu(hwApp), CMU_VIEW_PROPERTIES, MF_BYCOMMAND | MF_CHECKED);
      EnableMenuItem(GetMenu(hwApp), CMU_VIEW_DOCKPROP, MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(GetMenu(hwApp), CMU_VIEW_PROPEQTN, MF_BYCOMMAND | MF_ENABLED);
   } else {
      pPropMgr->DestroyMgrWindow();
      CheckMenuItem(GetMenu(hwApp), CMU_VIEW_PROPERTIES, MF_BYCOMMAND | MF_UNCHECKED);
      EnableMenuItem(GetMenu(hwApp), CMU_VIEW_DOCKPROP, MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(GetMenu(hwApp), CMU_VIEW_PROPEQTN, MF_BYCOMMAND | MF_GRAYED);
   }
   OnResize();                              // refresh window positions
}

//===UserDockProperties===================================
// note: menu item now reads "float properties" -- inverse of dock flag!
void CApplication::UserDockProperties(BOOL tfDock) {
   if(pPropMgr==NULL) return;               // ignore if no property manager
   if(tfDock) {
      pPropMgr->SetBit(CPMF_DOCKED);
      CheckMenuItem(GetMenu(hwApp), CMU_VIEW_DOCKPROP, MF_BYCOMMAND | MF_CHECKED);
   } else {
      pPropMgr->ClearBit(CPMF_DOCKED);
      CheckMenuItem(GetMenu(hwApp), CMU_VIEW_DOCKPROP, MF_BYCOMMAND | MF_UNCHECKED);
   }
   UserViewProperties(TRUE);
}


/*********************************************************
* UserCloseSystem
* Close a system, e.g. by requesting to close the 2d ren-
* derer window. Returns FALSE ONLY if the user canceled.
*********************************************************/
BOOL CApplication::UserCloseSystem(CSystem *pSysClose) {
   CSystem *pSys;                           // system loop counter

   if(GetCurrentSystem()==NULL) return(TRUE); // ignore if no system
   if(pSysClose) pSys = pSysClose;            // close specified system..
   else pSys = GetCurrentSystem()->GetTopSystem(); //..or find top of current chain
   if( !pSys->UserCloseSystem() ) return(FALSE); // user canceled close
   UserDeleteSystem(pSys);                  // delete if it's ok to do so
   return(TRUE);                            // system was deleted
}


/*########################################################
 ## Variables                                          ##
########################################################*/
//===UserSetVar===========================================
void CApplication::SetVar(int iIndx, double dVal) {
   if((iIndx<0) || (iIndx>=CAPP_NUMVAR)) return; // ignore out of range
   if(dVal < dVarMin[iIndx]) SetVarRange(iIndx, dVal, VarMax(iIndx));
   if(dVal > dVarMax[iIndx]) SetVarRange(iIndx, VarMin(iIndx), dVal);
   dVar[iIndx] = dVal;
}
//===UserSetVarRange======================================
void CApplication::SetVarRange(int iIndx, double dMin, double dMax) {
   if((iIndx<0) || (iIndx>=CAPP_NUMVAR)) return; // ignore out of range
   if(dMin>dMax) {                          // keep ascending
      if(dMin==VarMin(iIndx)) dMax = dMin; else dMin = dMax;
   }
   if(dMin > dVar[iIndx]) dVar[iIndx] = dMin;
   if(dMax < dVar[iIndx]) dVar[iIndx] = dMax;
   dVarMin[iIndx] = dMin;
   dVarMax[iIndx] = dMax;
}
//===VarsString===========================================
const char* CApplication::VarsString(void) { return(CpszVar); };

//===Variable string======================================
const char* CApplication::VarString(int iIndx) {
   char *psz;                               // pointer to variable strings string
   if((iIndx<0) || (iIndx>=CAPP_NUMVAR)) return(NULL); // ignore out of range
   for(psz=(char*)CpszVar; iIndx>0; iIndx--, psz+=strlen(psz) + 1); // loop into string list
   return((const char*) psz);               // return the variable
}


/*########################################################
 ## Update Chain                                       ##
########################################################*/
// These functions are called whenever  something changes
// in the  global-wide settings. System can  manage their
// internal changes by  themselves by calling the equiva-
// lent CSystem:: functions.
/*********************************************************
* ScanAll
* Scans each variable that has a hook on it, AND finishes
* off the system with a clean render-to-canvas call. This
* is probably the most common function to call.
*********************************************************/
void CApplication::ScanAll(void) {
   int    iRVar;                            // running variable loop counter
   int    iPt;                              // point loop counter
   double dVarValue;                        // default value

   //---Run the variables-----------------------
   for(iRVar=0; iRVar<NumVars(); iRVar++) {
      if(iVarHooks[iRVar] > 0) {
         dVarValue = dVar[iRVar];           // store current value
         for(iPt=0; iPt<iVarPoints[iRVar]; iPt++) {
            dVar[iRVar] = dVarMin[iRVar]
               + (dVarMax[iRVar] - dVarMin[iRVar]) * iPt / (iVarPoints[iRVar]-1 + ((iVarPoints[iRVar]==1) ? 1 : 0));
            ApplyAllEquations(dVar);        // apply the equations
            SolveAllSystemABCD();           // solve all the systems
            GraphAllRendererPoint(iRVar, iPt); // graph this point
         }
         dVar[iRVar] = dVarValue;            // restore previous value
      }//if
   }//for(iRVar)

   //---Clean render----------------------------
   ApplyAllEquations(dVar);                 // apply default values in equations
   SolveAllSystemABCD();                    // solve all the systems
   PlaceAllCanvasVertices();                // render to canvas positions
   GraphAllRendererPoint(-1, -1);           // call with "normal" values
   RefreshAllRenderers();                   // update the renderer displays

   //---Update properties-----------------------
   if(GetCurrentRenderer()) GetCurrentRenderer()->UpdateProperties();
   if(GetCurrentSystem()) {
      GetCurrentSystem()->UpdateProperties();
      GetCurrentSystem()->UpdateVxProperties();
   }

   //---Refresh PropManager---------------------
   if(pPropMgr) pPropMgr->OnPaint(TRUE);    // update text only
}

/*********************************************************
* ApplyAllEquations
*********************************************************/
void CApplication::ApplyAllEquations(double *pcdVar) {
   for(CSystem* pSys=pSysTop; pSys; pSys=pSys->Next())
      pSys->ApplyEquations(pcdVar);
}

/*********************************************************
* SolveAllSystemABCD
*********************************************************/
void CApplication::SolveAllSystemABCD(void) {
   for(CSystem* pSys=pSysTop; pSys; pSys=pSys->Next())
      pSys->SolveSystemABCD();
}

/*********************************************************
* GraphAllRendererPoint
*********************************************************/
void CApplication::GraphAllRendererPoint(int iRVar, int iPt) {
   for(CSystem* pSys=pSysTop; pSys; pSys=pSys->Next())
      pSys->GraphRendererPoint(iRVar, iPt);
}

/*********************************************************
* PlaceAllCanvasVertices
*********************************************************/
void CApplication::PlaceAllCanvasVertices(void) {
   for(CSystem* pSys=pSysTop; pSys; pSys=pSys->Next())
      pSys->PlaceCanvasVertices(TRUE);      // place vertices, recursive through systems
}

/*********************************************************
* RefreshAllRenderers
*********************************************************/
void CApplication::RefreshAllRenderers(void) {
   for(CSystem* pSys=pSysTop; pSys; pSys=pSys->Next())
      pSys->RefreshRenderers(TRUE);         // refresh renderers, recursive through systems
}

/*########################################################
 ## File Functions                                     ##
########################################################*/
/*********************************************************
* UserSaveSystem
* Request to save a system. This function manages the di-
* alog boxes, file handles, and the chain of spawned sys-
* tems and graphs
* Note for posterity: This was the part of Rev 4 that did
* not work, so it's the first to be written in Rev 5
*********************************************************/
void CApplication::UserSaveSystem(BOOL tfSaveAs) {
   char         szFull[256];                // full path / name of file to open
   char         szFile[256];                // name of file
   char         szBuf[512];                 // formatted string for status bar
   OPENFILENAME ofn;                        // dialog box structure
   HANDLE       hFile;                      // file handle
   DWORD        dwBytes;                    // number of bytes written
   CSystem     *pSys;                       // system loop counter

   //===Find parent of spawned systems====================
   if(GetCurrentSystem() == NULL) return;   // ignore if no system open
   pSys = GetCurrentSystem()->GetTopSystem(); // get top of system chain

   //===Dialog============================================
   if(pSys->GetFullFile(NULL,0)==FALSE) tfSaveAs = TRUE; // always dialog if not already set
   if(tfSaveAs) {
      //---File name dialog---------------------
      memset(szFull, 0x00, sizeof(szFull));
      memset(szFile, 0x00, sizeof(szFile));
      memset(&ofn,   0x00, sizeof(ofn));
      ofn.lStructSize   = sizeof(ofn);
      ofn.lpstrFile     = szFull;
      ofn.nMaxFile      = sizeof(szFull) / sizeof(char);
      ofn.lpstrFileTitle= szFile;
      ofn.nMaxFileTitle = sizeof(szFile) / sizeof(char);
      ofn.lpstrFilter   = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0(*.*)\0";
      ofn.Flags         = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
      ofn.lpstrDefExt   = "txt";               // default extension
      if( !GetSaveFileName(&ofn) ) return;
   } else {
      pSys->GetFullFile(szFull, sizeof(szFull)/sizeof(char));
   }

   //===File==============================================
   //---Open file-------------------------------
   hFile = CreateFile(szFull, GENERIC_WRITE, 0, (LPSECURITY_ATTRIBUTES) NULL,
      CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);
   if(hFile==NULL) {
      printf("! CSystem::UserSaveSystem@167 Could not open file for writing\n");
      return;
   }

   //---Write Systems---------------------------
   pSys->SaveSystem(hFile);                 // save top system, rest recursively
   if(tfSaveAs) {
      pSys->SetFileName(szFull, szFile);    // set new name if necessary
      pSys->UpdateAllRendererTitle();       // update all titles in case name changed
   }

   //---Finalize--------------------------------
   if(pStatusBar) {                         // misuse part buffer
      LoadString(hInstance, SZMSG_FILESAVED, szFile, sizeof(szFile)/sizeof(char));
      sprintf(szBuf, szFile, szFull);
      pStatusBar->SetPriorityText(szBuf, 4500);
   }
   CloseHandle(hFile);
}


/*********************************************************
* UserLoadSystem
* Request to load a system from a file. This function ma-
* nages the dialog boxes and file handles. If the file is
* loaded without errors, a new system is created and fil-
* led (recursively) by CSystem::LoadSystem.
*
* The entire file is loaded  into a memory buffer so that
* block-delimiting pointers can be used.  Since the files
* are not expected to be too  big, we use malloc; if this
* turns out to fail too often, we  can always use Global-
* Alloc().
*********************************************************/
void CApplication::UserLoadSystem(void) {
   char         szFull[256];                // path and name of file to open
   char         szFile[256];                // name of file
   OPENFILENAME ofn;                        // dialog box structure
   char        *pszDataFile;                // contents of file
   HANDLE       hFile;                      // file handle
   DWORD        dwBytes;                    // number of bytes written
   int          iFileSize;                  // size of the file
   CSystem     *pSysNew;                    // newly created system

   //===File==============================================
   //---File name dialog------------------------
   memset(szFull, 0x00, sizeof(szFull));    // clear file name
   memset(szFile, 0x00, sizeof(szFile));
   memset(&ofn,   0x00, sizeof(ofn));       // clear struct
   ofn.lStructSize   = sizeof(ofn);
   ofn.hwndOwner     = hwApp;
   ofn.lpstrFile     = szFull;
   ofn.nMaxFile      = sizeof(szFull) / sizeof(char);
   ofn.lpstrFileTitle= szFile;
   ofn.nMaxFileTitle = sizeof(szFile) / sizeof(char);
   ofn.lpstrFilter   = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0(*.*)\0";
   ofn.Flags         = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
   if( !GetOpenFileName(&ofn) ) return;     // dialog box, return on Cancel

   //---Open file-------------------------------
   hFile = CreateFile(szFull, GENERIC_READ, 0, (LPSECURITY_ATTRIBUTES) NULL,
      OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);
   if(hFile==NULL) {
      LoadErrorMsg(SZERR_LOAD_FILEOPEN, NULL, szFull);
      return;
   }

   //---Copy to memory buffer-------------------
   iFileSize = GetFileSize(hFile, NULL);
   pszDataFile = (char*) malloc(iFileSize+1); // allocate buffer
   memset(pszDataFile, 0x00, iFileSize+1);  // clear the buffer, ensure terminating 0x00
   if(pszDataFile == NULL) {
      LoadErrorMsg(SZERR_LOAD_BUFALLOC, NULL, NULL);
      CloseHandle(hFile);                   // close the file
      return;
   }
   ReadFile(hFile, pszDataFile, iFileSize, &dwBytes, NULL);
   CloseHandle(hFile);                      // close the file

   //===Load Systems======================================
   pSysNew = UserNewSystem();               // create the new, empty system

   pSysNew->SetFileName(szFull, szFile);    // set the file name
   if( pSysNew->LoadSystem(pszDataFile, NULL) == FALSE) { // load top level system
      UserDeleteSystem(pSysNew); pSysNew = NULL; // if system couldn't be loaded, delete it
   }

   //===Finalize==========================================
   if(pszDataFile) free(pszDataFile); pszDataFile = NULL; // free buffer
   UpdateProperties();                      // update variable properties
   ScanAll();                               // activate everything
}


/*********************************************************
* LoadErrorMsg
* Displays an error message related to finding parts of a
* data file. If pszDataFile and pszErr are both non-zero,
* this function  counts the row number  to that point and
* displays  the line containing  the error in  the dialog
* box.
* If pszDataFile  is NULL and pszErr is  non-null, pszErr
* is appended as-is before the message. pszErr might con-
* tain the name of the system that could not be found.
*********************************************************/
void CApplication::LoadErrorMsg(int iErr, const char *pszDataFile, const char *pszErr) {
   char  szErrorMessage[1024];              // buffer for error message
   char  szErrorTable[256];                 // part of error loaded from stringtable
   char *pszMin, *pszMax;                   // string delimiters
   int   iRow;                              // row loop counter

   //===Preliminaries=====================================
   LoadString(hInstance, iErr, szErrorTable, sizeof(szErrorTable)/sizeof(char));
   sprintf(szErrorMessage, "Error %d", iErr); // header text

   //===Location==========================================
   if(pszDataFile != NULL) {
      //---Count row----------------------------
      pszMin = (char*) pszDataFile;         // scan from beginning of file
      pszMax = (char*) pszDataFile + strlen(pszDataFile); // pointer to end of file
      iRow = 0;                             // count rows, starting at 0
      while((pszMin!=NULL) && (pszMin<pszMax) && (pszMin<pszErr)) { // count rows up to error
         pszMin = strchr(pszMin, '\r');     // advance to next end of row
         pszMin++;                          // skip over end-of-row
         iRow++;                            // count rows
      }
      //---Extract row--------------------------
      pszMax = pszMin;                      // points to end of line
      if(pszMin>pszDataFile) pszMin -= 1;   // jump back over end of row (now before \n)
      do pszMin--; while((pszMin>=pszDataFile) && (*pszMin!='\r')); // scan backwards
      while((*pszMin=='\n') || (*pszMin=='\r')) pszMin++; // remove unnecessary ends of line
      while((*pszMax=='\n') || (*pszMax=='\r')) pszMax--; // remove unnecessary ends of line

      //---Format message-----------------------
      sprintf(szErrorMessage+strlen(szErrorMessage),
         " in line %d\n   <%.*s>\n", iRow, pszMax-pszMin+1, pszMin);

   //===Extra String Only=================================
   } else {//if(pszErr != NULL) {
      sprintf(szErrorMessage+strlen(szErrorMessage),
         "\n   %s\n", pszErr);
   }

   //===Error Text========================================
   sprintf(szErrorMessage+strlen(szErrorMessage),
      "\n%s", szErrorTable);

   //===Message Box=======================================
   MessageBox(hwApp, szErrorMessage, "Load Error", MB_OK | MB_ICONEXCLAMATION);
}



/*########################################################
 ## MDI Child Windows                                  ##
########################################################*/
/*********************************************************
* CreateSysWinWindow
* Creates a new MDI child window  and returns the handle.
* The window title is NOT set.
*********************************************************/
HWND CApplication::CreateSysWinWindow(const char *pszClassName, LPVOID lpVoid, UINT uStyle) {
   HWND hWnd;                               // new window handle
   MDICREATESTRUCT mcs;                     // structure for MDI window

   if(hwMDIClient == NULL) return(NULL);    // ignore if no MDI client

   mcs.szClass = pszClassName;
   mcs.szTitle = NULL;
   mcs.hOwner  = GetInstance();             // INSTANCE handle -- that's pretty f*ing obscure.
   mcs.x       = CW_USEDEFAULT;
   mcs.y       = CW_USEDEFAULT;
   mcs.cx      = 500;
   mcs.cy      = 200;
   mcs.style   = 0 | uStyle;
   mcs.lParam  = (LPARAM) lpVoid;
   hWnd = (HWND) SendMessage(hwMDIClient, WM_MDICREATE, 0, (LPARAM) &mcs);

   return(hWnd);                            // return new window
}

/*********************************************************
* DestroySysWinWindow
* Destroys one of the MDI child windows
*********************************************************/
void CApplication::DestroySysWinWindow(HWND hWnd) {
   if(hwMDIClient==NULL) return;            // ignore if no MDI client
   SendMessage(hwMDIClient, WM_MDIDESTROY, (WPARAM) hWnd, 0L);
}


/*********************************************************
* ActivateSysWinWindow
* Activate the given MDI child  window. This is used when
* a given renderer already exists for the system and must
* be activated in response to a menu command.
* Called from
*  <- CApplication::ProcessCommand (Menu)
*********************************************************/
void CApplication::ActivateSysWinWindow(HWND hWnd) {
   if(hwMDIClient==NULL) return;            // ignore if no MDI client
   if(hWnd==NULL) return;                   // ignore if no window supplied
   SendMessage(hwMDIClient, WM_MDIACTIVATE, (WPARAM) hWnd, 0L);
}


/*********************************************************
* GetCurrentRenderer
* Returns the current renderer from the currently active
* MDI window.
* Called from
*   <-- CApp:: Updating properties on global change
*********************************************************/
CSysWin* CApplication::GetCurrentRenderer(void) {
   HWND     hwActive;                       // active window
   CSysWin *pSWin;                          // active renderer object

   if(hwMDIClient==NULL) return(NULL);      // ignore if no MDI client
   hwActive = (HWND) SendMessage(hwMDIClient, WM_MDIGETACTIVE, 0, 0L);
   if(hwActive == NULL) return(NULL);       // no active window, so no system
   pSWin = (CSysWin*) GetWindowLong(hwActive, 0); // get object from window
   return(pSWin);
}


/*********************************************************
* GetCurrentSystem
* Establish the currently active  system by extracting it
* from the currently active MDI child window. If no child
* windows exist, this function returns NULL.
*********************************************************/
CSystem* CApplication::GetCurrentSystem(void) {
   CSysWin *pSWin;                          // active renderer object

   pSWin = GetCurrentRenderer();            // establish current rendeder window
   if(pSWin == NULL) return(NULL);          // return if no window active
   return(pSWin->System());                 // return renderer's system
}


/*########################################################
 ## Property Manager                                   ##
########################################################*/
/*********************************************************
* PrepareProperties
* Shows or hides  an array of item IDs. The  last item in
* the list must be 0 to indicate the end of the list. For
* de-selection, the callback is  also set to NULL to pre-
* vent the possibility of stale function pointers.
* Called from
* <- Renderer::PrepareProperties
*    <- Renderer::WM_MDIACTIVATE
*********************************************************/
void CApplication::PrepareProperties(const UINT *puPropList, BOOL tfShow, PROPITEMCALLBACK lpfnCallback, void *pVoid) {
   UINT     *puProp;                        // property loop counter
   if(pPropMgr==NULL) return;               // ignore if not available
   //---Process list----------------------------
   for(puProp=(UINT*) puPropList; (*puProp); puProp++) {
      if(tfShow) {
         pPropMgr->FindItemByID(*puProp)->ClearBit(CPIF_HIDDEN);
         pPropMgr->FindItemByID(*puProp)->SetItemCallback(lpfnCallback, pVoid);
      } else {
         pPropMgr->FindItemByID(*puProp)->SetBit(CPIF_HIDDEN);
         pPropMgr->FindItemByID(*puProp)->SetItemCallback(NULL, NULL);
      }
   }
   //---Retrieve callbacks----------------------
   if(tfShow) {
      pPropMgr->FindItemByID( CPS_EQVARNAME     )->SetItemCallback(CApplication::_PropItemCallback, (void*)this);
      pPropMgr->FindItemByID( CPS_EQVALUE       )->SetItemCallback(CApplication::_PropItemCallback, (void*)this);
      pPropMgr->FindItemByID( CPS_EQSLIDER      )->SetItemCallback(CApplication::_PropItemCallback, (void*)this);
      pPropMgr->FindItemByID( CPS_EQRANGE       )->SetItemCallback(CApplication::_PropItemCallback, (void*)this);
      pPropMgr->FindItemByID( CPS_CMDHEADER     )->SetItemCallback(CApplication::_PropItemCallback, (void*)this);
      pPropMgr->FindItemByID( CPS_COMMANDNEW    )->SetItemCallback(CApplication::_PropItemCallback, (void*)this);
      pPropMgr->FindItemByID( CPS_COMMANDNEWPROP)->SetItemCallback(CApplication::_PropItemCallback, (void*)this);
      pPropMgr->FindItemByID( CPS_COMMANDOPEN   )->SetItemCallback(CApplication::_PropItemCallback, (void*)this);
      pPropMgr->FindItemByID( CPS_COMMANDCLOSE  )->SetItemCallback(CApplication::_PropItemCallback, (void*)this);
   }

   //---Update----------------------------------
   if(pPropMgr->Window()) {                 // don't refresh unless window visible
      pPropMgr->OnResize();                 // position items and resize actives
      InvalidateRect(pPropMgr->Window(), NULL, TRUE); // repaint everything
   }
}

/*********************************************************
* UpdateVariableProperties
* Shows or hides the property items associated with equa-
* tions. These are encapsulated  into a separate function
* rather than  having each renderer that  allows variable
* editing  list them  specifically, also because  they're
* application items, not renderer items. Additional bene-
* fit: We  don't remove the callbacks here, so  the items
* are ready to go as soon as they are revealed again.
* Called from
*   <- Renderers
*********************************************************/
static UINT CuVarProperties[] = {
   CPS_EQHEADER      ,
   CPS_EQVARNAME     ,
   CPS_EQVALUE       ,
   CPS_EQSLIDER      ,
   CPS_EQRANGE       ,
   0};
void CApplication::PrepareVariableProperties(BOOL tfShow) {
   if(pPropMgr==NULL) return;               // ignore if no property manager
   UINT     *puProp;                        // property loop counter
   if(pPropMgr==NULL) return;               // ignore if not available
   //---Process list----------------------------
   for(puProp=CuVarProperties; (*puProp); puProp++) {
      if(tfShow) pPropMgr->FindItemByID(*puProp)->ClearBit(CPIF_HIDDEN);
      else       pPropMgr->FindItemByID(*puProp)->SetBit(CPIF_HIDDEN);
   }
}
/*********************************************************
* UpdateProperties
* Updates the properties in the property manager.
* Note: None of the UpdateProperties functions causes the
* property manager to refresh; this must be called expli-
* citly at the time when the parameters are changed.
* Called from <- Renderers
*********************************************************/
void CApplication::UpdateProperties(void) {
   CPropMgr *pMgr = pPropMgr;               // assign to make code readable
   if(pMgr==NULL) return;                   // ignore if no manager

   pMgr->FindItemByID(CPS_EQVARNAME)->SetItemDropList(iCurrentVariable, (char*) VarsString());
   pMgr->FindItemByID(CPS_EQVALUE  )->SetItemValue(Var(iCurrentVariable));
   pMgr->FindItemByID(CPS_EQSLIDER )->SetItemSlider(Var(iCurrentVariable), VarMin(iCurrentVariable), VarMax(iCurrentVariable));
   pMgr->FindItemByID(CPS_EQRANGE  )->SetItemDblValue(VarMin(iCurrentVariable), VarMax(iCurrentVariable));
}


/*********************************************************
* PropItemCallback
* The callback function must be declared static.
*********************************************************/
BOOL CApplication::_PropItemCallback(void *vData, UINT uID, void *pVoid) {
   return(((CApplication*)pVoid)->PropItemCallback(vData, uID)); // direct to member
}
//========================================================
BOOL CApplication::PropItemCallback(void *vData, UINT uID) {
   //---Values------------------------
   switch(uID) {
   case CPS_EQVARNAME:
      iCurrentVariable = *(int*)vData;
      if(iCurrentVariable < 0) iCurrentVariable = 0;
      if(iCurrentVariable > CAPP_NUMVAR) iCurrentVariable = CAPP_NUMVAR;
      UpdateProperties();                   // update variable properties
      pPropMgr->OnPaint(TRUE);              // repaint only these values
      return(TRUE);
///TODO: Update only my items

   case CPS_EQVALUE:
   case CPS_EQSLIDER:
      SetVar(iCurrentVariable, *(double*)vData);
      break;
   case CPS_EQRANGE:
      SetVarRange(iCurrentVariable, ((double*)vData)[0], ((double*)vData)[1]);
      break;
   //---Commands----------------------
   case CPS_COMMANDNEW:     SendMessage(hwApp, WM_COMMAND, CMU_FILE_NEWRESO   , 0L); return(TRUE);
   case CPS_COMMANDNEWPROP: SendMessage(hwApp, WM_COMMAND, CMU_FILE_NEWPROP   , 0L); return(TRUE);
   case CPS_COMMANDOPEN:    SendMessage(hwApp, WM_COMMAND, CMU_FILE_OPEN      , 0L); return(TRUE);
   case CPS_COMMANDCLOSE:   SendMessage(hwApp, WM_COMMAND, CMU_FILE_CLOSEMODEL, 0L); return(TRUE);

   default: return(TRUE);
   }

   //---Update------------------------
   UpdateProperties();                      // update variable properties
   ScanAll();                               // scan through all systems
   return(TRUE);
}


/*########################################################
 ## Status Bar                                         ##
########################################################*/
/*********************************************************
* SetStatusBarInfo
* Set the info window on the status bar.
* Called from
*  <- Renderer::mouse movement
*********************************************************/
void CApplication::SetStatusBarInfo(double *pdX, double *pdY) {
   char szBuf[256];                         // formatted text
   if(pStatusBar==NULL) return;             // ignore if no bar
   sprintf(szBuf, "(");
   if(pdX) sprintf(szBuf+strlen(szBuf), "%.3lg", *pdX);
   else    sprintf(szBuf+strlen(szBuf), "---");
   sprintf(szBuf+strlen(szBuf), ", ");
   if(pdY) sprintf(szBuf+strlen(szBuf), "%.3lg", *pdY);
   else    sprintf(szBuf+strlen(szBuf), "---");
   sprintf(szBuf+strlen(szBuf), ")");
   pStatusBar->SetStandardText(szBuf, 1);
}


/*########################################################
 ## About Dialog                                       ##
########################################################*/
/*********************************************************
* Custom help dialog  with 3d wireframe  mirror mount and
* scrolling credits. Taken from Rev 4, but text changed.
*  Initial call: HelpFcn(NULL, NULL, hWnd, hInstance)
*********************************************************/
LRESULT CALLBACK CApplication::AboutProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
const int iX[] = {
 +3,  +3,  +0, -13, -25, -32, -35, -32, -25, -13,  -0, +13, +25, +32, +35, +50, +50, +47, -47, -50, -50, -47,  +3, -99,  +3,  +3,  +0, -13, -25, -32, -35, -32, -25, -13,  -0, +13, +25, +32, +35, +50, +50, +47, -47, -50, -50, -47,  +3, -99,  +3,  +3,
-99,  +3,  +3, -99,  +0,  +0, -99, -13, -13, -99, -25, -25, -99, -32, -32, -99, -35, -35, -99, -32, -32, -99, -25, -25, -99, -13, -13, -99,  -0,  -0, -99, +13, +13, -99, +25, +25, -99, +32, +32, -99, +35, +35, -99, +50, +50, -99, +50, +50, -99, +47,
+47, -99, -47, -47, -99, -50, -50, -99, -50, -50, -99, -47, -47, -99,  +3,  +3, -99, -47, -37, -33, -33, +50, +54, +54, +50, -47, -51, -51, -47, -99, -47, -37, -33, -33, +50, +54, +54, +50, -47, -51, -51, -47, -99, -47, -47, -99, -37, -37, -99, -33,
-33, -99, -33, -33, -99, +50, +50, -99, +54, +54, -99, +54, +54, -99, +50, +50, -99, -47, -47, -99, -51, -51, -99, -51, -51, -99, -47, -47, -99, -35, -35, -36, -36, -35, -36, -36, -36, -40, -40, -36, -40, -40, -40, -45, -45, -40, -45, -45, -45, -48,
-48, -45, -48, -48, -48, -48, -48, -48, -48, -48, -48, -45, -45, -48, -45, -45, -45, -40, -40, -45, -40, -40, -40, -36, -36, -40, -36, -36, -36, -35, -35, -36, -35, -35, -35, -36, -36, -35, -36, -99, +49, +49, +47, +47, +49, +47, +47, +47, +43, +43,
+47, +43, +43, +43, +38, +38, +43, +38, +38, +38, +35, +35, +38, +35, +35, +35, +35, +35, +35, +35, +35, +35, +38, +38, +35, +38, +38, +38, +43, +43, +38, +43, +43, +43, +47, +47, +43, +47, +47, +47, +49, +49, +47, +49, +49, +49, +47, +47, +49, +47,
-99, +20, +20, +17, +17, +20, +17, +17, +17, +10, +10, +17, +10, +10, +10,  +0,  +0, +10,  +0,  +0,  +0, -10, -10,  +0, -10, -10, -10, -17, -17, -10, -17, -17, -17, -20, -20, -17, -20, -20, -20, -17, -17, -20, -17, -17, -17, -10, -10, -17, -10, -10,
-10,  -0,  -0, -10,  -0,  -0,  -0, +10, +10,  -0, +10, +10, +10, +17, +17, +10, +17, +17, +17, +20, +20, +17, +20, +20, +20, +17, +17, +20, +17
};
const int iY[] = {
+50, +35, +35, +32, +25, +13,  +0, -13, -25, -32, -35, -32, -25, -13,  -0,  +0, -47, -50, -50, -47, +47, +50, +50, -99, +50, +35, +35, +32, +25, +13,  +0, -13, -25, -32, -35, -32, -25, -13,  -0,  +0, -47, -50, -50, -47, +47, +50, +50, -99, +50, +50,
-99, +35, +35, -99, +35, +35, -99, +32, +32, -99, +25, +25, -99, +13, +13, -99,  +0,  +0, -99, -13, -13, -99, -25, -25, -99, -32, -32, -99, -35, -35, -99, -32, -32, -99, -25, -25, -99, -13, -13, -99,  -0,  -0, -99,  +0,  +0, -99, -47, -47, -99, -50,
-50, -99, -50, -50, -99, -47, -47, -99, +47, +47, -99, +50, +50, -99, +50, +50, -99, +54, +54, +50, -33, -33, -37, -47, -51, -51, -47, +50, +54, -99, +54, +54, +50, -33, -33, -37, -47, -51, -51, -47, +50, +54, -99, +54, +54, -99, +54, +54, -99, +50,
+50, -99, -33, -33, -99, -33, -33, -99, -37, -37, -99, -47, -47, -99, -51, -51, -99, -51, -51, -99, -47, -47, -99, +50, +50, -99, +54, +54, -99, +42, +42, +46, +46, +42, +46, +46, +46, +48, +48, +46, +48, +48, +48, +48, +48, +48, +48, +48, +48, +44,
+44, +48, +44, +44, +44, +39, +39, +44, +39, +39, +39, +35, +35, +39, +35, +35, +35, +35, +35, +35, +35, +35, +35, +37, +37, +35, +37, +37, +37, +42, +42, +37, +42, +42, +42, +46, +46, +42, +46, -99, -42, -42, -37, -37, -42, -37, -37, -37, -35, -35,
-37, -35, -35, -35, -35, -35, -35, -35, -35, -35, -39, -39, -35, -39, -39, -39, -44, -44, -39, -44, -44, -44, -48, -48, -44, -48, -48, -48, -48, -48, -48, -48, -48, -48, -46, -46, -48, -46, -46, -46, -42, -42, -46, -42, -42, -42, -37, -37, -42, -37,
-99, -51, -111, -111, -51, -51, -51, -51, -111, -111, -51, -51, -51, -51, -111, -111, -51, -51, -51, -51, -111, -111, -51, -51, -51, -51, -111, -111, -51, -51, -51, -51, -111, -111, -51, -51, -51, -51, -111, -111, -51, -51, -51, -51, -111, -111, -51,
-51, -51, -51, -111, -111, -51, -51, -51, -51, -111, -111, -51, -51, -51, -51, -111, -111, -51, -51, -51, -51, -111, -111, -51, -51, -51, -51, -111, -111, -51, -51, -51
};
const int iZ[] = {
+20, +20, +20, +20, +20, +20, +20, +20, +20, +20, +20, +20, +20, +20, +20, +20, +20, +20, +20, +20, +20, +20, +20, -99, +45, +45, +45, +45, +45, +45, +45, +45, +45, +45, +45, +45, +45, +45, +45, +45, +45, +45, +45, +45, +45, +45, +45, -99, +20, +45,
-99, +20, +45, -99, +20, +45, -99, +20, +45, -99, +20, +45, -99, +20, +45, -99, +20, +45, -99, +20, +45, -99, +20, +45, -99, +20, +45, -99, +20, +45, -99, +20, +45, -99, +20, +45, -99, +20, +45, -99, +20, +45, -99, +20, +45, -99, +20, +45, -99, +20,
+45, -99, +20, +45, -99, +20, +45, -99, +20, +45, -99, +20, +45, -99, +20, +45, -99, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -99, +15, +15, +15, +15, +15, +15, +15, +15, +15, +15, +15, +15, -99, -10, +15, -99, -10, +15, -99, -10,
+15, -99, -10, +15, -99, -10, +15, -99, -10, +15, -99, -10, +15, -99, -10, +15, -99, -10, +15, -99, -10, +15, -99, -10, +15, -99, -10, +15, -99, -25, +25, +25, -25, -25, -25, -25, +25, +25, -25, -25, -25, -25, +25, +25, -25, -25, -25, -25, +25, +25,
-25, -25, -25, -25, +25, +25, -25, -25, -25, -25, +25, +25, -25, -25, -25, -25, +25, +25, -25, -25, -25, -25, +25, +25, -25, -25, -25, -25, +25, +25, -25, -25, -25, -25, +25, +25, -25, -25, -25, -99, -25, +25, +25, -25, -25, -25, -25, +25, +25, -25,
-25, -25, -25, +25, +25, -25, -25, -25, -25, +25, +25, -25, -25, -25, -25, +25, +25, -25, -25, -25, -25, +25, +25, -25, -25, -25, -25, +25, +25, -25, -25, -25, -25, +25, +25, -25, -25, -25, -25, +25, +25, -25, -25, -25, -25, +25, +25, -25, -25, -25,
-99,  +3,  +3, +13, +13,  +3, +13, +13, +13, +20, +20, +13, +20, +20, +20, +23, +23, +20, +23, +23, +23, +20, +20, +23, +20, +20, +20, +13, +13, +20, +13, +13, +13,  +3,  +3, +13,  +3,  +3,  +3,  -7,  -7,  +3,  -7,  -7,  -7, -14, -14,  -7, -14, -14,
-14, -17, -17, -14, -17, -17, -17, -14, -14, -17, -14, -14, -14,  -7,  -7, -14,  -7,  -7,  -7,  +3,  +3,  -7,  +3,  +3,  +3, +13, +13,  +3, +13
};
const int iN = 379;

const char szCred[] = "\
LaserCanvas v5.0$\
Copyright (c) 2000-2007$\
Philip Schlup, PhD$\
<pschlup@hotmail.com>$\
$\
Alumni$\
University of St Andrews$\
University of Otago$\
ETH Zurich$\
Colorado State University$\
$\
Developed using technologies$\
Borland C++BuilderX$\
Borland Resource Workshop 1.02$\
Borland C/C++ v. 4.5$\
MathWorks Matlab$\
Apple XCode (Mac Test)$\
Microsoft HTML Help Workshop$\
$\
$\
$\
$\
$\
Thanks To$\
Mum  Dad$\
Esthi  Kevin$\
Tiwa  Tiwa (II)  Trini$\
Grosi  Dodo$\
and all the Whanau$\
Andrew  Kathryn  McTaggley Jr$\
Aynia  Cam  Joycey$\
ChrisR  ChrisE$\
Glenn  Lindsay  Jarod  Ryan$\
Malcolm  Cameron$\
Dave  Alison$\
Scott  Jess  Kendall$\
Jenny  Anne$\
Iain  Robyn$\
Dianne  Lau  Colleen$\
Kim  Brad  Oma$\
Julie  John  Carrie  Pete$\
Olivia  Timo  Eileen$\
Petrissa  Annalisa  Arne$\
Flo  Wouter  Christian$\
Rosmarie  Jens  Ursi$\
Wädi  Marcel  Hari  Peter$\
André  Maciek  Reinhard$\
Adri  Theres  Kiki$\
Omid  Jesse$\
David  Klaus$\
Randy  Malini  Maya  Alisha$\
la bella donna bionda$\
nella bistro ogni mattini$\
and all the gang$\
$\
For all your support$\
during long late hours$\
my deepest thanks$\
$\
20563.200612031407T$";

   int         k;                           // loop variable
   double      dRot, dElv;                  // rotation angles
   double      csX, snX, csY, snY, dZ;      // trigs, persp. dist
   float       fRot, cs, sn;                // rotation angle, trigs
   WNDCLASS    wc;                          // window class registration struct
   HINSTANCE   hInst;                       // application instance passed to function
   HDC         hdcMem;                      // memory DC
   RECT        rc;                          // client rectangle of window
   SIZE        sz;
   HBITMAP     hbmMem, hbmOld;              // memory bitmap
   char       *psz;                         // pointer into credits string
   //---Initial call----------------------------
   if(hWnd == NULL) {
      hInst = (HINSTANCE) lParam;           // extract instance from argument
      wc.style         = 0;
      wc.lpfnWndProc   = (WNDPROC) AboutProc;
      wc.cbClsExtra    = 0;
      wc.cbWndExtra    = 3*sizeof(long);
      wc.hInstance     = hInst;
      wc.hIcon         = LoadIcon(hInst, "ICON_LASRCANV5");
      wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
      wc.hbrBackground = (HBRUSH) NULL;
      wc.lpszMenuName  = (LPCSTR) NULL;
      wc.lpszClassName = "LC_AboutProc_Class";
      if(!RegisterClass(&wc)) return(-1);
      hWnd = CreateWindow("LC_AboutProc_Class","About",
            WS_BORDER | WS_VISIBLE,
            (GetSystemMetrics(SM_CXSCREEN)-350)>>1,
            (GetSystemMetrics(SM_CYSCREEN)-200)>>1,
            350, 200,
            (HWND) HWND_DESKTOP, (HMENU) NULL, (HINSTANCE) lParam, (LPVOID) NULL);
      if(hWnd == NULL) return(-2);
      SetWindowLong(hWnd, 0, (long) hInst);
      SetWindowLong(hWnd, 1, (long) SetTimer(hWnd, NULL, 20, NULL) );
      SetWindowLong(hWnd, 2, (long) 0);
      ShowWindow(hWnd, SW_SHOW);
      return(0L);
   //---Callback call---------------------------
#define MAPX(x,y,z) ( 50 + 600.00*(      csY*(x) +               snY*(z)) / dZ )
#define MAPY(x,y,z) ( 40 - 600.00*( -snX*snY*(x) + csX*(y) + snX*csY*(z)) / dZ )
#define MAPZ(x,y,z,L) (              ( -csX*snY*(x) - snX*(y) + csX*csY*(z)) + L )

   } else {
      switch(uMsg) {
      case WM_TIMER:
         SetWindowLong(hWnd, 2, GetWindowLong(hWnd, 2) + 2);
         SendMessage(hWnd, WM_NCPAINT, 1, 0L);
         break;

      case WM_NCPAINT:
         HDC hdc;
         HDC *pDC;
         RECT rc;
         HPEN hpGold, hpBlue;
         //---Prelims---
         GetWindowRect(hWnd, &rc);
         hpGold = CreatePen(PS_SOLID, 2, RGB(0xFF, 0xCC, 0x00));
         hpBlue = CreatePen(PS_SOLID, 1, RGB(0x00, 0x00, 0x99));
         rc.right -= rc.left; rc.bottom -= rc.top;
         rc.left  -= rc.left; rc.top    -= rc.top;

         hdc = GetWindowDC(hWnd);
         SaveDC(hdc);

         hdcMem = CreateCompatibleDC(hdc);
         hbmMem = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
         hbmOld = (HBITMAP) SelectObject(hdcMem, hbmMem);
         pDC = &hdcMem;

         SelectObject(*pDC, GetStockObject(BLACK_PEN));
         SetTextAlign(*pDC, TA_CENTER);

         //---Frame and fill---
         //FillRect(*pDC, &rc, (HBRUSH) GetStockObject(WHITE_BRUSH));
         FillRect(*pDC, &rc, (HBRUSH) GetStockObject(WHITE_BRUSH));
         RoundRect(*pDC, rc.left, rc.top, rc.right, rc.bottom, 16, 16);
         MoveToEx(*pDC, rc.right-13,rc.top+4,NULL); LineTo(*pDC,rc.right-6,rc.top+11);
         MoveToEx(*pDC, rc.right-13,rc.top+10,NULL); LineTo(*pDC,rc.right-6,rc.top+3);
         MoveToEx(*pDC, rc.right-14,rc.top+4,NULL); LineTo(*pDC,rc.right-7,rc.top+11);
         MoveToEx(*pDC, rc.right-14,rc.top+10,NULL); LineTo(*pDC,rc.right-7,rc.top+3);

         //---Mount---
         dRot = (double)GetWindowLong(hWnd, 2) / 150.0000;
         dElv = 0.25 + 0.2*cos(dRot*0.3213);
         csX = cos(dElv); snX = sin(dElv);
         csY = cos(dRot); snY = sin(dRot);
         MoveToEx(*pDC, 0, 0, NULL);
         dZ = MAPZ(-iX[0], iY[0], iZ[0], 1200.00+50*cos(dElv*dRot));
         MoveToEx(*pDC, MAPX(-iX[0], iY[0], iZ[0]), MAPY(-iX[0], iY[0], iZ[0]), NULL);
         for(k=1; k<iN; k++) {
            if(iX[k] < -90) {
               k++;
               dZ = MAPZ(-iX[k], iY[k], iZ[k], 1200.00+50*cos(dElv*dRot));
               MoveToEx(*pDC, MAPX(-iX[k], iY[k], iZ[k]), MAPY(-iX[k], iY[k], iZ[k]), NULL);
            } else {
               dZ = MAPZ(-iX[k], iY[k], iZ[k], 1200.00+50*cos(dElv*dRot));
               LineTo(*pDC, MAPX(-iX[k], iY[k], iZ[k]), MAPY(-iX[k], iY[k], iZ[k]));
            }
         }

         //---Credits---
         SetTextColor(*pDC, RGB(0x00, 0x00, 0x66));
         TextOut(*pDC, 225, 8, "LaserCanvas 5", 13);
         SelectObject(*pDC, hpGold);
         MoveToEx(*pDC, 99,  32, NULL); LineTo(*pDC, 339,  32);
         SelectObject(*pDC, hpBlue);
         MoveToEx(*pDC,100,  30, NULL); LineTo(*pDC, 340,  30);
         MoveToEx(*pDC,100,  33, NULL); LineTo(*pDC, 340,  33);
         k = GetWindowLong(hWnd, 2)/5; // scroll speed
         while(k > 70*18) k-=70*18; // scroll credits: LINES HERE
         k = 180 - k;
         psz = (char*) szCred;
         while(strchr(psz, '$')) {
            if((k > 40) && (k<170)) {
               if(k<72) {
                  SetTextColor(*pDC, RGB((40-k)<<3, (40-k)<<3, (40-k)<<3));
               } else if(k>138) {
                  SetTextColor(*pDC, RGB((k-170)<<3, (k-170)<<3, (k-170)<<3));
               } else {
                  SetTextColor(*pDC, RGB(0x00, 0x00, 0x00));
               }
               TextOut(*pDC, 225, k, psz, strchr(psz, '$')-psz);
            }
            k += 18;
            psz = strchr(psz, '$')+1;
         }

         //---Finish---
         BitBlt(hdc, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, hdcMem, 0, 0, SRCCOPY);

         SelectObject(hdcMem, hbmOld);
         DeleteObject(hbmMem);
         DeleteDC(hdcMem);
         RestoreDC(hdc, -1);
         ReleaseDC(hWnd, hdc);
         DeleteObject(hpBlue);
         DeleteObject(hpGold);
         break;

      case WM_CHAR:
      case WM_NCLBUTTONDOWN:
      case WM_LBUTTONDOWN:
         hInst = (HINSTANCE) GetWindowLong(hWnd, 0); // retrieve instance handle
         KillTimer(hWnd, (UINT) GetWindowLong(hWnd, 1)); // stop timer
         DestroyWindow(hWnd);
         break;
      default:
         return(DefWindowProc(hWnd, uMsg, wParam, lParam));
      }
      return(0L);
   }
}
