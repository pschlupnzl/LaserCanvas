/*********************************************************
* CMouse.h
* Header file for CMouse class, for LaserCanvas rev. 5
* $PSchlup 2004-2006 $     $Revision 6 $
*********************************************************/
#ifndef CMOUSE_H
#define CMOUSE_H
class CMouse;                               // mouse control object

#include <windows.h>
#include "CMActive.h"                       // active elements

//---Constants----------------------------------
#define CM_DRAGTHRESHOLD             4      // minimum move before a drag is started
#define ACSM_NONE                    0      // no change (e.g. cursor change)
#define ACSM_CHAR                    1      // update of the keys
#define ACSM_MOVE                    2      // simple move
#define ACSM_DOWN                    3      // messages to Active callbacks: Left button down
#define ACSM_LBUP                    4      // left button is released
#define ACSM_LEFT                    5      // left button down and up without drag
#define ACSM_DRAG                    6      // object is being dragged
#define ACSM_DEND                    7      // dragged object is released
#define ACSM_RBDN                    8      // right button is pressed
#define ACSM_DBLK                    9      // left double-click
#define ACSM_MDDN                   10      // middle down
#define ACSM_MDUP                   11      // middle up
#define ACSM_MDCK                   12      // middle click
#define ACSM_MDRG                   13      // middle drag
#define ACSM_MEND                   14      // middle drag end

//---CMouse class-------------------------------
class CMouse {
   CMActive* pActCurrent;                   // currently active Active
   CMActive* pActTop;                       // start of Actives chain
   HWND      hwCapture;                     // window to capture mouse events
   POINT     ptDownLocation;                // location where left button went down
   POINT     ptAuxData;                     // auxiliary data
   BOOL      tfDown;                        // left button currently down
   BOOL      tfMDown;                       // middle down
   BOOL      tfDrag;                        // drag currently happening
   WORD      wKeysDown;                     // keys when the mouse was clicked down
public:
   CMouse(void);                            // constructor and initialization
   ~CMouse(void);                           // destructor

   CMActive* CreateActive(                  // create a new Active with given properties
      int left, int top, int right, int bottom,
      HCURSOR hcMove, HCURSOR hcMoveCtrl, HCURSOR hcDrag, HCURSOR hcDragCtrl,
      void (*CallbackFcn)(int iMsg, int x, int y, int wKeys, void *pVoid, long int lData),
      void *pVoid, long int lData, HMENU hmContextMenu);
   CMActive* CreateActiveIndirect(ACTIVECREATESTRUCT *pacs); // create new Active with structure instead
   void      DeleteActive(CMActive *pActDel); // delete the specified Active
   void      DeleteAllActive(void);         // delete all the actives
   void      PaintAllActiveRect(HDC hDC);   // paints the active rectangles
   void      ClearMouseStatus(void);        // clears Cur.Active, tfDown, tfDrag
   CMActive* ActiveTop(void)           { return(pActTop); };
   CMActive* FindActiveByData(void *pVoid, long int lData); // find first Active with given data
   void      CMSetCapture(HWND hwCapture);  // set the mouse capture window
   void      CMReleaseCapture(void);        // release the capture
   void      GetDownLocation(int *pix, int *piy) { if(pix) *pix=ptDownLocation.x; if(piy) *piy=ptDownLocation.y; };
   WORD      Keys(void)                { return(wKeysDown); };
   BOOL      MouseProc(HWND hWnd, UINT uMsg, int x, int y); // dereferenced version
};


#endif/*CMOUSE_H*/
