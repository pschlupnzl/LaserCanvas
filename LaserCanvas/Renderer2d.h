/*********************************************************
* Render2d.h
* 2d vector icons for canvas and mode size renderers
* $PSchlup 2006 $     $Revision 5 $
*********************************************************/
#ifndef RENDERER2D_H                        // prevent multiple includes
#define RENDERER2D_H

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
void Renderer2dIcon  (HDC hdc, CVertex *pVx, int x, int y, double dSize, double dAbsAng); // switchyard
void Renderer2dMirror(HDC hdc, int x, int y, double dSize, double dAbsAng, double dROC);
void Renderer2dLens  (HDC hdc, int x, int y, double dSize, double dAbsAng, double dFL);
void Renderer2dSource(HDC hdc, int x, int y, double dSize, double dAbsAng);
void Renderer2dScreen(HDC hdc, int x, int y, double dSize, double dAbsAng);
void Renderer2dPlate (HDC hdc, int x, int y, double dSize, double dAbsAng, double dROC);
void Renderer2dPrism (HDC hdc, int x, int y, double dSize, double dAbsAng, double dRefIndx);
void Renderer2dMirrorLock(HDC hdc, int x, int y, double dSize, double dAbsAng);
void Renderer2dSysSpawn(HDC hdc, int x, int y, double dSize, double dAbsAng);
void Renderer2dOpticGraph(HDC hdc, int x, int y, double dSize, double dAbsAng);
void Renderer2dMode(HDC hdc, int xVx, int yVx, double dAbsAng, double dModeSize, double dLenSeg, double w0, double z0, double zR, double zMax, BOOL tfWaist=TRUE);

void Renderer2dPatchPlate(HDC hdc, int x0, int y0, double dAng0, double dROC0, int x1, int y1, double dAng1, double dROC1, double dSize);
void Renderer2dPatchPrism(HDC hdc, int x0, int y0, double dAng0, int x1, int y1, double dAng1, double dSize, double dRefIndx);


#endif//RENDERER2D_H
