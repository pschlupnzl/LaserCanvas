/*********************************************************
* LasrCanv5
* LaserCanvas revision 5
* There was nothing  really wrong with Revision 4,  I had
* simply  left it for  too long, and  I'm hoping  that by
* starting from scratch again  I will be able to create a
* solid application.
* The features are all available. The things that cost me
* the most time over the years were
*  - CEquation
*  - The constrained drag
*  - CAxes
* Development notice:  The first version of this applica-
* tion dates back to 2000. Although I have been at Otago,
* St Andrews, Colorado State  universities and the ETH in
* Switzerland,  I did not use any work  time at those in-
* stitutions, nor any of their resources, for writing any
* code. As such I do not think that the ownership of this
* code can be contested.
*
* In this revision, I hope to  separate the Windows parts
* some more. Who knows? Maybe I'll be able to port it.
*
* Files in this build:-
* Application
*    CApplication.cpp     CApplication.h
*    CPropMgr.cpp         CPropMgr.h
*    LasrCanv5.cpp        LasrCanv5.h
*                         resource.h
* Interface
*                         CChainLink.h
*    CMActive.cpp         CMActive.h
*    CMouse.cpp           CMouse.h
*    CQuickEdit.cpp       CQuickEdit.h
* Renderers
*    CSysWin.cpp          CSysWin.h
*    CSysWin1d.cpp        CSysWin1d.h
*    CSysWin2d.cpp        CSysWin2d.h
*    CSysWinGraph.cpp     CSysWinGraph.h
*    CSysWinVxGraph.cpp   CSysWinVxGraph.h
*    Renderer2d.cpp       Renderer2d.h
* System
*    CLCEqtn.cpp          CLCEqtn.h
*    CSystem.cpp          CSystem.h
*    CVertex.cpp          CVertex.h
* Toolbars
*    CBasicBar.cpp        CBasicBar.h
*    CButton.cpp          CButton.h
*    CButtonBar.cpp       CButtonBar.h
*    CStatusBar.cpp       CStatusBar.h
*
* Copyright (c) Philip Schlup 2000-2006
* $PSchlup 2006 $     $Revision 5 $
*********************************************************/
#include "LasrCanv5.h"

/*********************************************************
* WinMain - Windows entrypoint function
*********************************************************/
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int iShow) {
UNREFERENCED_PARAMETER(hPrevInst);
UNREFERENCED_PARAMETER(lpCmdLine);
   CApplication *pApp;                      // application object
   int           iRet;                      // return value

   /*char *pcWav = NULL;
   long int lFileLen = 0;
   FILE *pFileWav = NULL;
   pFileWav = fopen("chord.wav","r");
   if(pFileWav) {
      fseek(pFileWav, 0, SEEK_END);
      lFileLen = ftell(pFileWav);
      fseek(pFileWav, 0, SEEK_SET);
      pcWav = (char*) malloc(lFileLen * sizeof(char));
      if(pcWav) {
         fread(pcWav, sizeof(char), lFileLen, pFileWav);
         PlaySound((LPSTR) pcWav, NULL, SND_MEMORY | SND_ASYNC);
      }
      fclose(pFileWav);
      pFileWav = NULL;
   }*/
   // if(pcWav) free(pcWav); pcWav = NULL;     // delete the wave buffer

   /*//===Execution counter=================================
   int iCt;FILE*pFile=fopen("count.txt","rt");if(pFile)if(fscanf(pFile,"%d",
   &iCt)>0){fclose(pFile);pFile=fopen("count.txt","wt");if(pFile)fprintf(pFile,
   "%d",++iCt);fclose(pFile);};*/

   //===Create application object=========================
   pApp = new CApplication(hInst, iShow);   // create the application
   iRet = pApp->MessageLoop();              // run program
   delete(pApp); pApp = NULL;               // delete and invalidate pointer


   return(iRet);
}
