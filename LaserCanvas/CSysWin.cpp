/*********************************************************
* CSysWin
* Window class that renders the CSystem, in an attempt to
* separate the Windows graphics  rendering from the CSys-
* tem classes.
*
* CSysWin is a BASE CLASS for a number of different types
* of system 'renderers,' such as
*  - 2D Layout, the one where optics can be dragged
*  - 1D "Layou" that displays the mode linearly
*  - Vertex graphs
*  - Inventory list
*  - ABCD solver
* The last point is critical: The vertices no longer hold
* pointers to  graph objects; the graphs  are part of the
* system renderers (i.e., CSysWin) chain.
* I can't immediately see how  to make the property sheet
* a CSysWin renderer even though that would be convenient
* from a currency  point of view: It would  be updated at
* the correct times. The PropertySheet cannot be owned by
* a single system, or my many systems concurrently, so we
* have to keep it separate.  The general implementational
* strategies will probably be similar, though.
*
* CSysWin-derived classes will have to provide these fun-
* ctions (cf. messages in Windows!)
* Refresh
*    Refresh the display
* VertexAdded(pVxNew)
*    When a vertex is added.  This may well be ignored by
*    most of the renderers
* DeletingVertex(pVxDel)
*    For example, a vertex graph renderer must remove it-
*    self from the renderer chain and be destroyed
*
* The CSysWin chain is maintained by the CSystem.
*
*
* $PSchlup 2006 $     $Revision 5 $
*********************************************************/
#include "CSysWin.h"                        // class header

// The order here must match CSYSWINI_TYPE_
const char* CSysWin::CszSysWinType[] = {"1d", "2d", "3d", "SystemGraph", "VertexGraph", "Inventory", "Solver"};

CApplication* CSysWin::App(void) { return(pSystem ? pSystem->App() : NULL); };

/*********************************************************
* Constructor
*********************************************************/
CSysWin::CSysWin(CSystem *pSys) { printf("+ CSysWin 0x%08lx created in system <%s> \n", this, (pSys)?pSys->Tag() : "NULL");
   //===Members===========================================
   hwRenderer  = NULL;                      // no window yet
   pSystem     = pSys;                      // relevant system
   pSysWinNext = NULL;                      // next renderer in chain
}


/*********************************************************
* Destructor
*********************************************************/
CSysWin::~CSysWin() {
   //---Destroy window--------------------------
   if(hwRenderer) pSystem->DestroySysWinWindow(hwRenderer); hwRenderer = NULL;
printf("- Deleted CSysWin\n");}


/*########################################################
 ## Data Files                                         ##
########################################################*/
/*********************************************************
* SaveSysWin Header and Footer
* Write a generic renderer header or footer to the file.
*********************************************************/
//===Header===============================================
void CSysWin::SaveSysWinHeader(HANDLE hFile, int iType) {
   char szBuf[256];                         // string to write to buffer
   DWORD dwBytes;                           // number of bytes written
   RECT  rcWin, rcMDI;                      // window, MDI client positions

   //---Preliminaries-----------------------
   if(hFile == NULL) printf("? CSysWin::SaveSysWinHeader@84 Called with hFile=NULL\n");
   if(hFile == NULL) return;                // ignore if no file
   GetWindowRect(hwRenderer, &rcWin);       // read window position on screen
   GetWindowRect(App()->GetMDIClient(), &rcMDI);
   SetRect(&rcWin, rcWin.left-rcMDI.left, rcWin.top-rcMDI.top,
      rcWin.right-rcWin.left, rcWin.bottom-rcWin.top); // rcWin is now x,y,wid,hig

   //---Save data---------------------------
   sprintf(szBuf, "Renderer %s {\r\n", CszSysWinType[iType]);
   WriteFile(hFile, szBuf, strlen(szBuf), &dwBytes, NULL);
   sprintf(szBuf, "   System = %s\r\n", pSystem->Tag());
   WriteFile(hFile, szBuf, strlen(szBuf), &dwBytes, NULL);
   if(hwRenderer) {
      sprintf(szBuf, "   Window = %d, %d, %d, %d\r\n", rcWin.left, rcWin.top, rcWin.right, rcWin.bottom);
      WriteFile(hFile, szBuf, strlen(szBuf), &dwBytes, NULL);
   }
}
//===Footer===============================================
void CSysWin::SaveSysWinFooter(HANDLE hFile) {
   DWORD dwBytes;                           // number of bytes written

   //---Preliminaries-----------------------
   if(hFile == NULL) printf("? CSysWin::SaveSystemFooter@96 Called with hFile=NULL\n");
   if(hFile == NULL) return;                // ignore if no file

   //---Save data---------------------------
   WriteFile(hFile, "}\r\n", 3, &dwBytes, NULL);
}

/*********************************************************
* LoadSysWinHeader
* Load things saved by header. Since this is all NON-CRI-
* TICAL, nothing happens if these items can't be loaded.
*********************************************************/
void CSysWin::LoadSysWinHeader(const char *pszDataFile, char *pszMin, char *pszMax) {
UNREFERENCED_PARAMETER(pszDataFile);
   int   ix, iy, iw, ih;                    // dimensions of window
   char *psz;                               // pointer within search space

   if(pszMin==NULL) return;                 // couldn't load if no pointer
   if(pszMax==NULL) pszMax = pszMin + strlen(pszMin); // to end of file if not given

   //---Window rectangle------------------------
   psz = strstr(pszMin, "Window");          // search for property text within vertex
   if((psz!=NULL) && (psz<pszMax)) {        // if found..
      psz = strchr(psz, '=');               //..find '=' sign
      if((psz!=NULL) && (psz<pszMax)) {     // ensure '=' sign is within vertex
         if(sscanf(psz+1, "%d,%d,%d,%d", &ix, &iy, &iw, &ih) >= 4) {
            if(hwRenderer) MoveWindow(hwRenderer, ix, iy, iw, ih, TRUE);
         }
      }
   }

}
