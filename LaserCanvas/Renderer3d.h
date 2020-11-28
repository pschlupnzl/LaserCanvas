/*********************************************************
* Renderer3d.h
* $PSchlup 2006 $     $Revision 0 $
*********************************************************/
#ifndef RENDERER3D_H                        // prevent multiple includes
#define RENDERER3D_H

//===Includes=============================================
#include <windows.h>                        // standard Windows header
#include <math.h>                           // trigonometric functions
#include "common.h"
#include "CVertex.h"                        // vertex types

//===Macros===============================================
//---Trig functions-----------------------------
// We may want to create lookup tables for trig functions
// for speed reasons. We'll use the math ones for now
// TAN has name conflict with SAG/TAN defined in CAbcd.h
#define SIN(x)              sin(x)
#define COS(x)              cos(x)
#define TRIGTAN(x)          tan(x)

// These maybe-safe versions of the  math functions. Note
// that  atan2(1e-309, 1e-309) can still cause  an excep-
// tion. I don't know how to handle that. I've also tried
// with _matherr, but it's REALLY hard to spot these er-
// rors on the fly.
#define ASIN(x)             ((((x)>=-1.00) && ((x)<=1.00)) ? asin(x) : 0.00)
#define ATAN(x)             ( atan(x) )
#define ATAN2(y,x)          ( ((x)==0.00) ? (((y)>0.00) ? M_PI : -M_PI) : atan2(y, x) )

/*********************************************************
* Functions
*********************************************************/
//===Wireframe Routines===================================
void    Rotate3d   (double pddXYZ[], int N, double dRotX, double dRotY, double dRotZ);
void    Translate3d(double pddXYZ[], int N, double dOffX, double dOffY, double dOffZ);
void    Scale3d    (double pddXYZ[], int N, double dSclX, double dSclY, double dSclZ);
void    Camera3d   (double pddXYZ[], int N, double *xyzLoc, double *xyzLkAt, double dView, RECT *prcWindow);
int     Box3d      (double pddXYZ[], int N);

//===Optics===============================================
void Renderer3dPath  (CVertex *pVx);
void Renderer3dIcon  (CVertex *pVx, double dSize);
void Renderer3dMirror(double dROC);
void Renderer3dFlatMirror(void);
void Renderer3dLens(double dFL);
void Renderer3dScreen(void);
void Renderer3dPrism(double dRefIndx);
void Renderer3dPlate(CVertex *pVx, double dSize);

/*void Renderer3dLens  (HDC hdc, int x, int y, double dSize, double dAbsAng, double dFL);
void Renderer3dSource(HDC hdc, int x, int y, double dSize, double dAbsAng);
void Renderer3dScreen(HDC hdc, int x, int y, double dSize, double dAbsAng);
void Renderer3dPlate (HDC hdc, int x, int y, double dSize, double dAbsAng, double dROC);
void Renderer3dPrism (HDC hdc, int x, int y, double dSize, double dAbsAng, double dRefIndx);
void Renderer3dMode(HDC hdc, int xVx, int yVx, double dAbsAng, double dModeSize, double dLenSeg, double w0, double z0, double zR, double zMax, BOOL tfWaist=TRUE);
*/

//===Mode=================================================
void Renderer3dMode(CVertex *pVx, double dModeSize);
void Renderer3dBaseplate(const CVertex *pVxTop, double dOpticSize);

//===Renderers============================================
void Renderer3dWireframe(HDC hdc, double xyzLoc[], double xyzLkAt[], double dView, RECT *prcWindow);


#endif//RENDERER3D_H
