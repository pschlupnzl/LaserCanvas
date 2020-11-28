/*********************************************************
* Renderer3d.cpp
* Definition and execution of 3d wireframe and OpenGL ob-
* jects.
* $PSchlup 2006 $     $Revision 0 $
* Revision History
*   0  2006nov25  Original wireframe version
*********************************************************/
#include "Renderer3d.h"

/*********************************************************
* Vertex buffers
* Patch1 and Patch2 are multi-vertex patches, such as the
* back face of mirrors or top/bottom of prisms. The Patch
* array is assumed to be in squares
*********************************************************/
#define C3DI_NUM_PATCH         400          // number of patches in buffer
static double SxyzPatch1[3*16];
static double SxyzPatch2[3*16];
static double SxyzPatch[3*4*C3DI_NUM_PATCH];
static int SiN1 = 0;
static int SiN2 = 0;
static int SiN  = 0;

/*########################################################
 ## 3D Routines
########################################################*/
// The coordinate system is the same as that used by POV-
// Ray, taken via Matlab where all the testing was done.
/*********************************************************
* Rotate
* Rotate vertices pddXYZ about X, -Y, and Z axes (in that
* order), which are the same rotations as POVRay does.
*  X rotation         -Y rotation        Z rotation
* [ 1   0     0   ]  [ cosY  0 -sinY ]  [ cosZ  sinZ  0 ]
* [ 0  cosX  sinX ]  [  0    1   0   ]  [-sinZ  cosZ  0 ]
* [ 0 -sinX  cosX ]  [ sinY  0  cosY ]  [  0     0    1 ]
*
*********************************************************/
void Rotate3d(double pddXYZ[], int N, double dRotX, double dRotY, double dRotZ) {
   double dX, dY, dZ;                       // temp storage for matrix multiplication
   double dSin, dCos;                       // sine, cosine of angles
   int    k;                                // loop counter
   if(pddXYZ==NULL) return;                 // check for acces violation

   for(k=0; k<N; k++) {
      //---X--------------------------
      dSin = SIN(dRotX * M_PI / 180.00);
      dCos = COS(dRotX * M_PI / 180.00);
      dX = pddXYZ[3*k+0]; dY = pddXYZ[3*k+1]; dZ = pddXYZ[3*k+2];
      pddXYZ[3*k+0] =      dX + 0.00    + 0.00;
      pddXYZ[3*k+1] = 0.00    + dCos*dY + dSin*dZ;
      pddXYZ[3*k+2] = 0.00    +-dSin*dY + dCos*dZ;

      //---Y--------------------------
      dSin = SIN(dRotY * M_PI / 180.00);
      dCos = COS(dRotY * M_PI / 180.00);
      dX = pddXYZ[3*k+0]; dY = pddXYZ[3*k+1]; dZ = pddXYZ[3*k+2];
      pddXYZ[3*k+0] = dCos*dX + 0.00    +-dSin*dZ;
      pddXYZ[3*k+1] = 0.00    +      dY + 0.00;
      pddXYZ[3*k+2] = dSin*dX + 0.00    + dCos*dZ;

      //---Z--------------------------
      dSin = SIN(dRotZ * M_PI / 180.00);
      dCos = COS(dRotZ * M_PI / 180.00);
      dX = pddXYZ[3*k+0]; dY = pddXYZ[3*k+1]; dZ = pddXYZ[3*k+2];
      pddXYZ[3*k+0] = dCos*dX + dSin*dY + 0.00;
      pddXYZ[3*k+1] =-dSin*dX + dCos*dY + 0.00;
      pddXYZ[3*k+2] = 0.00    + 0.00    +      dZ;
   }
}

/*********************************************************
*  Translate
*  Translate vertices along axes
*********************************************************/
void Translate3d(double pddXYZ[], int N, double dOffX, double dOffY, double dOffZ) {
   int k;                                   // loop counter
   if(pddXYZ==NULL) return;                 // check access violation
   for(k=0; k<N; k++) {
      pddXYZ[3*k+0] += dOffX;
      pddXYZ[3*k+1] += dOffY;
      pddXYZ[3*k+2] += dOffZ;
   }
}

/*********************************************************
*  Scale
*  Scale vertices away from the origin
*********************************************************/
void Scale3d(double pddXYZ[], int N, double dSclX, double dSclY, double dSclZ) {
   int k;                                   // loop counter
   if(pddXYZ==NULL) return;                 // check access violation
   for(k=0; k<N; k++) {
      pddXYZ[3*k+0] *= dSclX;
      pddXYZ[3*k+1] *= dSclY;
      pddXYZ[3*k+2] *= dSclZ;
   }
}


/*********************************************************
*  Camera
*  Paint xyz lines using projection specified by the
*  location, look_at, and viewing angle as per POVRay
*  Revision <<0.1 - gives out [x,y] or [x,y,z]--NOPE>>
*           0.0 - original from POVMAT
*********************************************************/
void Camera3d(double pddXYZ[], int N, double *xyzLoc, double *xyzLkAt, double dView, RECT *prcWindow) {
   double DX, DY, DZ;                       // offset from camera to look-at
   double RXZ, R;                           // distances in XY plane, total
   double dfXZ, dthY;                       // rotation angles
   double dZoom;                            // scaling factor for image
   int k;                                   // loop counter

   //---Preliminaries---------------------------
   if(pddXYZ==NULL) return;                 // check access violation
   if((xyzLoc==NULL) || (xyzLkAt==NULL)) return; // check access violation
   if((xyzLoc[0]==xyzLkAt[0]) && (xyzLoc[1]==xyzLkAt[1]) && (xyzLoc[2]==xyzLkAt[2])) return; // degenerate camera

   //---Angles----------------------------------
   DX = xyzLoc[0] - xyzLkAt[0];
   DY = xyzLoc[1] - xyzLkAt[1];
   DZ = xyzLoc[2] - xyzLkAt[2];

   RXZ = SQRT(SQR(DX) + SQR(DZ));           // distance in X-Z plane
   R   = SQRT(SQR(DX) + SQR(DY) + SQR(DZ)); // total distance

   dfXZ = 180/M_PI * ATAN2( DZ, DX )+90;    // rotation angle in X-Z plane
   dthY =-180/M_PI * ATAN2( DY, RXZ );      // rotation angle up to Y axis

   //---Manipulate------------------------------
   Translate3d(pddXYZ, N, -xyzLkAt[0], -xyzLkAt[1], -xyzLkAt[2]);
   Rotate3d   (pddXYZ, N, 0.00, dfXZ, 0.00);     // rotate around about Y axis
   Rotate3d   (pddXYZ, N, dthY, 0.00, 0.00);     // rotate up about X axis
   Translate3d(pddXYZ, N, 0.00, 0.00, R);        // translate away from the camera

   //---Project---------------------------------
   dZoom = (double) (prcWindow->right-prcWindow->left) / (2*R*TRIGTAN(dView*M_PI/180.00 / 2));
   for(k=0; k<N; k++) {
      if(pddXYZ[3*k+2]==0.00) pddXYZ[3*k+2] = 1.00; // prevent #DIV/0! errors
      pddXYZ[3*k+0] = dZoom * R * pddXYZ[3*k+0] / pddXYZ[3*k+2];
      pddXYZ[3*k+1] = dZoom * R * pddXYZ[3*k+1] / pddXYZ[3*k+2];
   }
}

/*********************************************************
* LineUnitCube
*********************************************************/
int Box3d(double pddXYZ[], int N) {
   static double SdUnitCube[17*3] = {
      0.00, 0.00, 0.00,
      1.00, 0.00, 0.00,
      1.00, 1.00, 0.00,
      1.00, 0.00, 0.00,
      1.00, 0.00, 1.00,
      1.00, 1.00, 1.00,
      1.00, 0.00, 1.00,
      0.00, 0.00, 1.00,
      0.00, 1.00, 1.00,
      0.00, 0.00, 1.00,
      0.00, 0.00, 0.00,
      0.00, 1.00, 0.00,
      1.00, 1.00, 0.00,
      1.00, 1.00, 1.00,
      0.00, 1.00, 1.00,
      0.00, 1.00, 1.00,
      0.00, 1.00, 0.00};
   if(pddXYZ==NULL) return(0);              // check access violation
   if(N > 17) N = 17;                       // limit number of values copied
   memcpy(pddXYZ, SdUnitCube, N * 3 * sizeof(double));
   return(N);
}


/*########################################################
 ## Optics
########################################################*/
/*********************************************************
* Renderer3dPath
* Draws a simple straight line from this Vx to the next.
*********************************************************/
void Renderer3dPath(CVertex *pVx) {
   if((pVx==NULL) || (pVx->Next()==NULL)) return;
   SiN1 = 2;
   SxyzPatch1[3*0+0] = pVx->Y();         SxyzPatch1[3*0+1] = 0.00; SxyzPatch1[3*0+2] = pVx->X();
   SxyzPatch1[3*1+0] = pVx->Next()->Y(); SxyzPatch1[3*1+1] = 0.00; SxyzPatch1[3*1+2] = pVx->Next()->X();
}


/*********************************************************
* Renderer3dIcon
* Switchyard function.
* Unlike  the 2d Canvas renderer, the 3d renderer  relies
* more heavily on direct accessing of the CVertex proper-
* ties.
*********************************************************/
void Renderer3dIcon(CVertex *pVx, double dSize) {
   if(pVx==NULL) return;
   //---Obtain Optic----------------------------
   switch(pVx->Type()) {
   case CVX_TYPE_MIRROR:      Renderer3dMirror(pVx->ROC(TAN)); break;
   case CVX_TYPE_FLATMIRROR:  Renderer3dFlatMirror(); break;
   case CVX_TYPE_LENS:        Renderer3dLens(pVx->FL(TAN)); break;
   case CVX_TYPE_THERMALLENS: Renderer3dLens(pVx->FL(TAN)); break;
   case CVX_TYPE_SOURCE:      Renderer3dScreen(); break;
   case CVX_TYPE_SCREEN:      Renderer3dScreen(); break;
   case CVX_TYPE_PRISM1:      Renderer3dPrism(pVx->_Prop(CVXI_PROP_N)); break;
   case CVX_TYPE_PRISM2:      Renderer3dPrism((pVx->VxLinked()) ? pVx->VxLinked()->_Prop(CVXI_PROP_N) : 1.00); break;
   case CVX_TYPE_INPLATE:     Renderer3dPlate(pVx, dSize); break;
   case CVX_TYPE_INBREWSTER:  Renderer3dPlate(pVx, dSize); break;
   case CVX_TYPE_INCRYSTAL:   Renderer3dPlate(pVx, dSize); break;
   case CVX_TYPE_OUTCOUPLER:  Renderer3dMirror(pVx->ROC(TAN)); break;
   }

   //---Finishing-------------------------------
   switch(pVx->Type()) {
   case CVX_TYPE_MIRROR:
   case CVX_TYPE_FLATMIRROR:
   case CVX_TYPE_LENS:
   case CVX_TYPE_THERMALLENS:
   case CVX_TYPE_SCREEN:
   case CVX_TYPE_SOURCE:
   case CVX_TYPE_PRISM1:
   case CVX_TYPE_PRISM2:
   case CVX_TYPE_OUTCOUPLER:
      Scale3d    (SxyzPatch1, SiN1 , dSize/2.00, dSize/2.00, dSize/2.00);
      Scale3d    (SxyzPatch2, SiN2 , dSize/2.00, dSize/2.00, dSize/2.00);
      Scale3d    (SxyzPatch , SiN*4, dSize/2.00, dSize/2.00, dSize/2.00);
      Rotate3d   (SxyzPatch1, SiN1 , 0.00, pVx->OpticCanvasAngle()*180.00/M_PI, 0.00);
      Rotate3d   (SxyzPatch2, SiN2 , 0.00, pVx->OpticCanvasAngle()*180.00/M_PI, 0.00);
      Rotate3d   (SxyzPatch , SiN*4, 0.00, pVx->OpticCanvasAngle()*180.00/M_PI, 0.00);
      Translate3d(SxyzPatch1, SiN1 , pVx->Y(), 0.00, pVx->X());
      Translate3d(SxyzPatch2, SiN2 , pVx->Y(), 0.00, pVx->X());
      Translate3d(SxyzPatch , SiN*4, pVx->Y(), 0.00, pVx->X());
   }
}

/*********************************************************
* Mirror
*********************************************************/
static double SxyzMirror1[] = {
   +0.3182, +0.3182, +0.0000,
   +0.0000, +0.4500, +0.0000,
   -0.3182, +0.3182, +0.0000,
   -0.4500, +0.0000, +0.0000,
   -0.3182, -0.3182, +0.0000,
   -0.0000, -0.4500, +0.0000,
   +0.3182, -0.3182, +0.0000,
   +0.4500, -0.0000, +0.0000};
static double SxyzMirror2[] = {
   +0.9239, -0.3827, +0.2500,
   +0.7071, -0.7071, +0.2500,
   +0.3827, -0.9239, +0.2500,
   +0.0000, -1.0000, +0.2500,
   -0.3827, -0.9239, +0.2500,
   -0.7071, -0.7071, +0.2500,
   -0.9239, -0.3827, +0.2500,
   -1.0000, -0.0000, +0.2500,
   -0.9239, +0.3827, +0.2500,
   -0.7071, +0.7071, +0.2500,
   -0.3827, +0.9239, +0.2500,
   -0.0000, +1.0000, +0.2500,
   +0.3827, +0.9239, +0.2500,
   +0.7071, +0.7071, +0.2500,
   +0.9239, +0.3827, +0.2500,
   +1.0000, +0.0000, +0.2500};
static double SxyzMirror[] = {
   +0.4500, +0.0000, +0.0000,  +0.3182, -0.3182, +0.0000,  +0.5303, -0.5303, -0.5000,  +0.7500, +0.0000, -0.5000,
   +0.3182, -0.3182, +0.0000,  +0.0000, -0.4500, +0.0000,  +0.0000, -0.7500, -0.5000,  +0.5303, -0.5303, -0.5000,
   +0.0000, -0.4500, +0.0000,  -0.3182, -0.3182, +0.0000,  -0.5303, -0.5303, -0.5000,  +0.0000, -0.7500, -0.5000,
   -0.3182, -0.3182, +0.0000,  -0.4500, -0.0000, +0.0000,  -0.7500, -0.0000, -0.5000,  -0.5303, -0.5303, -0.5000,
   -0.4500, -0.0000, +0.0000,  -0.3182, +0.3182, +0.0000,  -0.5303, +0.5303, -0.5000,  -0.7500, -0.0000, -0.5000,
   -0.3182, +0.3182, +0.0000,  -0.0000, +0.4500, +0.0000,  -0.0000, +0.7500, -0.5000,  -0.5303, +0.5303, -0.5000,
   -0.0000, +0.4500, +0.0000,  +0.3182, +0.3182, +0.0000,  +0.5303, +0.5303, -0.5000,  -0.0000, +0.7500, -0.5000,
   +0.3182, +0.3182, +0.0000,  +0.4500, +0.0000, +0.0000,  +0.7500, +0.0000, -0.5000,  +0.5303, +0.5303, -0.5000,
   +0.7500, +0.0000, -0.5000,  +0.6402, -0.2652, -0.5000,  +0.9239, -0.3827, -1.0000,  +1.0000, +0.0000, -1.0000,
   +0.5303, -0.5303, -0.5000,  +0.2652, -0.6402, -0.5000,  +0.3827, -0.9239, -1.0000,  +0.7071, -0.7071, -1.0000,
   +0.0000, -0.7500, -0.5000,  -0.2652, -0.6402, -0.5000,  -0.3827, -0.9239, -1.0000,  +0.0000, -1.0000, -1.0000,
   -0.5303, -0.5303, -0.5000,  -0.6402, -0.2652, -0.5000,  -0.9239, -0.3827, -1.0000,  -0.7071, -0.7071, -1.0000,
   -0.7500, -0.0000, -0.5000,  -0.6402, +0.2652, -0.5000,  -0.9239, +0.3827, -1.0000,  -1.0000, -0.0000, -1.0000,
   -0.5303, +0.5303, -0.5000,  -0.2652, +0.6402, -0.5000,  -0.3827, +0.9239, -1.0000,  -0.7071, +0.7071, -1.0000,
   -0.0000, +0.7500, -0.5000,  +0.2652, +0.6402, -0.5000,  +0.3827, +0.9239, -1.0000,  -0.0000, +1.0000, -1.0000,
   +0.5303, +0.5303, -0.5000,  +0.6402, +0.2652, -0.5000,  +0.9239, +0.3827, -1.0000,  +0.7071, +0.7071, -1.0000,
   +0.6402, -0.2652, -0.5000,  +0.5303, -0.5303, -0.5000,  +0.7071, -0.7071, -1.0000,  +0.9239, -0.3827, -1.0000,
   +0.2652, -0.6402, -0.5000,  +0.0000, -0.7500, -0.5000,  +0.0000, -1.0000, -1.0000,  +0.3827, -0.9239, -1.0000,
   -0.2652, -0.6402, -0.5000,  -0.5303, -0.5303, -0.5000,  -0.7071, -0.7071, -1.0000,  -0.3827, -0.9239, -1.0000,
   -0.6402, -0.2652, -0.5000,  -0.7500, -0.0000, -0.5000,  -1.0000, -0.0000, -1.0000,  -0.9239, -0.3827, -1.0000,
   -0.6402, +0.2652, -0.5000,  -0.5303, +0.5303, -0.5000,  -0.7071, +0.7071, -1.0000,  -0.9239, +0.3827, -1.0000,
   -0.2652, +0.6402, -0.5000,  -0.0000, +0.7500, -0.5000,  -0.0000, +1.0000, -1.0000,  -0.3827, +0.9239, -1.0000,
   +0.2652, +0.6402, -0.5000,  +0.5303, +0.5303, -0.5000,  +0.7071, +0.7071, -1.0000,  +0.3827, +0.9239, -1.0000,
   +0.6402, +0.2652, -0.5000,  +0.7500, +0.0000, -0.5000,  +1.0000, +0.0000, -1.0000,  +0.9239, +0.3827, -1.0000,
   +1.0000, +0.0000, -1.0000,  +0.9239, -0.3827, -1.0000,  +0.9239, -0.3827, +0.2500,  +1.0000, +0.0000, +0.2500,
   +0.7071, -0.7071, -1.0000,  +0.3827, -0.9239, -1.0000,  +0.3827, -0.9239, +0.2500,  +0.7071, -0.7071, +0.2500,
   +0.0000, -1.0000, -1.0000,  -0.3827, -0.9239, -1.0000,  -0.3827, -0.9239, +0.2500,  +0.0000, -1.0000, +0.2500,
   -0.7071, -0.7071, -1.0000,  -0.9239, -0.3827, -1.0000,  -0.9239, -0.3827, +0.2500,  -0.7071, -0.7071, +0.2500,
   -1.0000, -0.0000, -1.0000,  -0.9239, +0.3827, -1.0000,  -0.9239, +0.3827, +0.2500,  -1.0000, -0.0000, +0.2500,
   -0.7071, +0.7071, -1.0000,  -0.3827, +0.9239, -1.0000,  -0.3827, +0.9239, +0.2500,  -0.7071, +0.7071, +0.2500,
   -0.0000, +1.0000, -1.0000,  +0.3827, +0.9239, -1.0000,  +0.3827, +0.9239, +0.2500,  -0.0000, +1.0000, +0.2500,
   +0.7071, +0.7071, -1.0000,  +0.9239, +0.3827, -1.0000,  +0.9239, +0.3827, +0.2500,  +0.7071, +0.7071, +0.2500,
   +0.9239, -0.3827, -1.0000,  +0.7071, -0.7071, -1.0000,  +0.7071, -0.7071, +0.2500,  +0.9239, -0.3827, +0.2500,
   +0.3827, -0.9239, -1.0000,  +0.0000, -1.0000, -1.0000,  +0.0000, -1.0000, +0.2500,  +0.3827, -0.9239, +0.2500,
   -0.3827, -0.9239, -1.0000,  -0.7071, -0.7071, -1.0000,  -0.7071, -0.7071, +0.2500,  -0.3827, -0.9239, +0.2500,
   -0.9239, -0.3827, -1.0000,  -1.0000, -0.0000, -1.0000,  -1.0000, -0.0000, +0.2500,  -0.9239, -0.3827, +0.2500,
   -0.9239, +0.3827, -1.0000,  -0.7071, +0.7071, -1.0000,  -0.7071, +0.7071, +0.2500,  -0.9239, +0.3827, +0.2500,
   -0.3827, +0.9239, -1.0000,  -0.0000, +1.0000, -1.0000,  -0.0000, +1.0000, +0.2500,  -0.3827, +0.9239, +0.2500,
   +0.3827, +0.9239, -1.0000,  +0.7071, +0.7071, -1.0000,  +0.7071, +0.7071, +0.2500,  +0.3827, +0.9239, +0.2500,
   +0.9239, +0.3827, -1.0000,  +1.0000, +0.0000, -1.0000,  +1.0000, +0.0000, +0.2500,  +0.9239, +0.3827, +0.2500};
/*********************************************************
* Renderer3dMirror
* The data above goes to -0.10; the back plane is at
* +0.25.
*********************************************************/
void Renderer3dMirror(double dROC) {
   int iPtch, k;                            // patch loop counter
   if(dROC != 0.00) dROC = 20.00 / dROC;
   if(dROC > 0.30) dROC = 0.30;
   if(dROC <-0.30) dROC =-0.30;

   //---Flat------------------------------------
   if(dROC==0.00) {
      memcpy(SxyzPatch1, SxyzMirror2,        (SiN1=16)*3*sizeof(double));
      memcpy(SxyzPatch2, SxyzMirror2,        (SiN2=16)*3*sizeof(double));
      memcpy(SxyzPatch, &SxyzMirror[24*4*3], (SiN =16)*4*3*sizeof(double));
      Scale3d(SxyzPatch1, SiN1, 1.00, -1.00, 0.00);

   //---Curved----------------------------------
   } else {
      memcpy(SxyzPatch1, SxyzMirror1, (SiN1= 8)*3*sizeof(double));
      memcpy(SxyzPatch2, SxyzMirror2, (SiN2=16)*3*sizeof(double));
      memcpy(SxyzPatch,  SxyzMirror,  (SiN =40)*4*3*sizeof(double));
   }

   //---Mate edge-------------------------------
   if(dROC < 0.00) Translate3d(SxyzPatch2, SiN2, 0.00, 0.00, -dROC);
   for(iPtch=0; iPtch<SiN; iPtch++) {
      //---Back plane---
      if(dROC < 0.00) for(k=0; k<4; k++) {
         if(SxyzPatch[3*4*iPtch+3*k+2] > 0.00) SxyzPatch[3*4*iPtch+3*k+2] += -dROC;
      }
      //---Front curve---
      for(k=0; k<4; k++) {
         if(SxyzPatch[3*4*iPtch+3*k+2] < 0.00) SxyzPatch[3*4*iPtch+3*k+2] *= dROC;
      }
   }
}

/*********************************************************
* FlatMirror
* A flat mirror is  displayed as a square  optic. This is
* mainly so that I have an extra debugging step.
*********************************************************/
static double SxyzFlatMirror1[] = {
   -1.00,+1.00,+0.00,
   -1.00,-1.00,+0.00,
   +1.00,-1.00,+0.00,
   +1.00,+1.00,+0.00};
static double SxyzFlatMirror2[] = {
   -1.00,-1.00,+0.25,
   -1.00,+1.00,+0.25,
   +1.00,+1.00,+0.25,
   +1.00,-1.00,+0.25};
static double SxyzFlatMirror[] = {
   -1.00,-1.00,+0.25, -1.00,-1.00,+0.00, -1.00,+1.00,+0.00, -1.00,+1.00,+0.25,
   -1.00,+1.00,+0.25, -1.00,+1.00,+0.00, +1.00,+1.00,+0.00, +1.00,+1.00,+0.25,
   +1.00,+1.00,+0.25, +1.00,+1.00,+0.00, +1.00,-1.00,+0.00, +1.00,-1.00,+0.25,
   +1.00,-1.00,+0.25, +1.00,-1.00,+0.00, -1.00,-1.00,+0.00, -1.00,-1.00,+0.25};
void Renderer3dFlatMirror(void) {
   memcpy(SxyzPatch1, SxyzFlatMirror1, (SiN1=4)*3*sizeof(double));
   memcpy(SxyzPatch2, SxyzFlatMirror2, (SiN2=4)*3*sizeof(double));
   memcpy(SxyzPatch,  SxyzFlatMirror,  (SiN =4)*4*3*sizeof(double));
}


/*********************************************************
* Lens
*********************************************************/
static double SxyzLensInner[] = {
   +0.3182, +0.3182, +0.0000,
   +0.0000, +0.4500, +0.0000,
   -0.3182, +0.3182, +0.0000,
   -0.4500, +0.0000, +0.0000,
   -0.3182, -0.3182, +0.0000,
   -0.0000, -0.4500, +0.0000,
   +0.3182, -0.3182, +0.0000,
   +0.4500, -0.0000, +0.0000};
static double SxyzLensFlat[] = {
   +0.9239, -0.3827, +0.2500,
   +0.7071, -0.7071, +0.2500,
   +0.3827, -0.9239, +0.2500,
   +0.0000, -1.0000, +0.2500,
   -0.3827, -0.9239, +0.2500,
   -0.7071, -0.7071, +0.2500,
   -0.9239, -0.3827, +0.2500,
   -1.0000, -0.0000, +0.2500,
   -0.9239, +0.3827, +0.2500,
   -0.7071, +0.7071, +0.2500,
   -0.3827, +0.9239, +0.2500,
   -0.0000, +1.0000, +0.2500,
   +0.3827, +0.9239, +0.2500,
   +0.7071, +0.7071, +0.2500,
   +0.9239, +0.3827, +0.2500,
   +1.0000, +0.0000, +0.2500};
static double SxyzLens[] = {
   +0.4500, +0.0000, +0.0000,  +0.3182, -0.3182, +0.0000,  +0.5303, -0.5303, -0.5000,  +0.7500, +0.0000, -0.5000,
   +0.3182, -0.3182, +0.0000,  +0.0000, -0.4500, +0.0000,  +0.0000, -0.7500, -0.5000,  +0.5303, -0.5303, -0.5000,
   +0.0000, -0.4500, +0.0000,  -0.3182, -0.3182, +0.0000,  -0.5303, -0.5303, -0.5000,  +0.0000, -0.7500, -0.5000,
   -0.3182, -0.3182, +0.0000,  -0.4500, -0.0000, +0.0000,  -0.7500, -0.0000, -0.5000,  -0.5303, -0.5303, -0.5000,
   -0.4500, -0.0000, +0.0000,  -0.3182, +0.3182, +0.0000,  -0.5303, +0.5303, -0.5000,  -0.7500, -0.0000, -0.5000,
   -0.3182, +0.3182, +0.0000,  -0.0000, +0.4500, +0.0000,  -0.0000, +0.7500, -0.5000,  -0.5303, +0.5303, -0.5000,
   -0.0000, +0.4500, +0.0000,  +0.3182, +0.3182, +0.0000,  +0.5303, +0.5303, -0.5000,  -0.0000, +0.7500, -0.5000,
   +0.3182, +0.3182, +0.0000,  +0.4500, +0.0000, +0.0000,  +0.7500, +0.0000, -0.5000,  +0.5303, +0.5303, -0.5000,
   +0.7500, +0.0000, -0.5000,  +0.6402, -0.2652, -0.5000,  +0.9239, -0.3827, -1.0000,  +1.0000, +0.0000, -1.0000,
   +0.5303, -0.5303, -0.5000,  +0.2652, -0.6402, -0.5000,  +0.3827, -0.9239, -1.0000,  +0.7071, -0.7071, -1.0000,
   +0.0000, -0.7500, -0.5000,  -0.2652, -0.6402, -0.5000,  -0.3827, -0.9239, -1.0000,  +0.0000, -1.0000, -1.0000,
   -0.5303, -0.5303, -0.5000,  -0.6402, -0.2652, -0.5000,  -0.9239, -0.3827, -1.0000,  -0.7071, -0.7071, -1.0000,
   -0.7500, -0.0000, -0.5000,  -0.6402, +0.2652, -0.5000,  -0.9239, +0.3827, -1.0000,  -1.0000, -0.0000, -1.0000,
   -0.5303, +0.5303, -0.5000,  -0.2652, +0.6402, -0.5000,  -0.3827, +0.9239, -1.0000,  -0.7071, +0.7071, -1.0000,
   -0.0000, +0.7500, -0.5000,  +0.2652, +0.6402, -0.5000,  +0.3827, +0.9239, -1.0000,  -0.0000, +1.0000, -1.0000,
   +0.5303, +0.5303, -0.5000,  +0.6402, +0.2652, -0.5000,  +0.9239, +0.3827, -1.0000,  +0.7071, +0.7071, -1.0000,
   +0.6402, -0.2652, -0.5000,  +0.5303, -0.5303, -0.5000,  +0.7071, -0.7071, -1.0000,  +0.9239, -0.3827, -1.0000,
   +0.2652, -0.6402, -0.5000,  +0.0000, -0.7500, -0.5000,  +0.0000, -1.0000, -1.0000,  +0.3827, -0.9239, -1.0000,
   -0.2652, -0.6402, -0.5000,  -0.5303, -0.5303, -0.5000,  -0.7071, -0.7071, -1.0000,  -0.3827, -0.9239, -1.0000,
   -0.6402, -0.2652, -0.5000,  -0.7500, -0.0000, -0.5000,  -1.0000, -0.0000, -1.0000,  -0.9239, -0.3827, -1.0000,
   -0.6402, +0.2652, -0.5000,  -0.5303, +0.5303, -0.5000,  -0.7071, +0.7071, -1.0000,  -0.9239, +0.3827, -1.0000,
   -0.2652, +0.6402, -0.5000,  -0.0000, +0.7500, -0.5000,  -0.0000, +1.0000, -1.0000,  -0.3827, +0.9239, -1.0000,
   +0.2652, +0.6402, -0.5000,  +0.5303, +0.5303, -0.5000,  +0.7071, +0.7071, -1.0000,  +0.3827, +0.9239, -1.0000,
   +0.6402, +0.2652, -0.5000,  +0.7500, +0.0000, -0.5000,  +1.0000, +0.0000, -1.0000,  +0.9239, +0.3827, -1.0000};
static double SxyzLensEdge[] = {
   +1.0000, +0.0000, -1.0000,  +0.9239, -0.3827, -1.0000,  +0.9239, -0.3827, +1.0000,  +1.0000, +0.0000, +1.0000,
   +0.7071, -0.7071, -1.0000,  +0.3827, -0.9239, -1.0000,  +0.3827, -0.9239, +1.0000,  +0.7071, -0.7071, +1.0000,
   +0.0000, -1.0000, -1.0000,  -0.3827, -0.9239, -1.0000,  -0.3827, -0.9239, +1.0000,  +0.0000, -1.0000, +1.0000,
   -0.7071, -0.7071, -1.0000,  -0.9239, -0.3827, -1.0000,  -0.9239, -0.3827, +1.0000,  -0.7071, -0.7071, +1.0000,
   -1.0000, -0.0000, -1.0000,  -0.9239, +0.3827, -1.0000,  -0.9239, +0.3827, +1.0000,  -1.0000, -0.0000, +1.0000,
   -0.7071, +0.7071, -1.0000,  -0.3827, +0.9239, -1.0000,  -0.3827, +0.9239, +1.0000,  -0.7071, +0.7071, +1.0000,
   -0.0000, +1.0000, -1.0000,  +0.3827, +0.9239, -1.0000,  +0.3827, +0.9239, +1.0000,  -0.0000, +1.0000, +1.0000,
   +0.7071, +0.7071, -1.0000,  +0.9239, +0.3827, -1.0000,  +0.9239, +0.3827, +1.0000,  +0.7071, +0.7071, +1.0000,
   +0.9239, -0.3827, -1.0000,  +0.7071, -0.7071, -1.0000,  +0.7071, -0.7071, +1.0000,  +0.9239, -0.3827, +1.0000,
   +0.3827, -0.9239, -1.0000,  +0.0000, -1.0000, -1.0000,  +0.0000, -1.0000, +1.0000,  +0.3827, -0.9239, +1.0000,
   -0.3827, -0.9239, -1.0000,  -0.7071, -0.7071, -1.0000,  -0.7071, -0.7071, +1.0000,  -0.3827, -0.9239, +1.0000,
   -0.9239, -0.3827, -1.0000,  -1.0000, -0.0000, -1.0000,  -1.0000, -0.0000, +1.0000,  -0.9239, -0.3827, +1.0000,
   -0.9239, +0.3827, -1.0000,  -0.7071, +0.7071, -1.0000,  -0.7071, +0.7071, +1.0000,  -0.9239, +0.3827, +1.0000,
   -0.3827, +0.9239, -1.0000,  -0.0000, +1.0000, -1.0000,  -0.0000, +1.0000, +1.0000,  -0.3827, +0.9239, +1.0000,
   +0.3827, +0.9239, -1.0000,  +0.7071, +0.7071, -1.0000,  +0.7071, +0.7071, +1.0000,  +0.3827, +0.9239, +1.0000,
   +0.9239, +0.3827, -1.0000,  +1.0000, +0.0000, -1.0000,  +1.0000, +0.0000, +1.0000,  +0.9239, +0.3827, +1.0000};
/*********************************************************
* Renderer3dLens
* The lens is assembled out of parts that are similar to
* the curved mirror data.
* LensEdge source data: -1 to +1
*********************************************************/
void Renderer3dLens(double dFL) {
   int iPtch, k;                            // patch, vertex loop counter
   double dThk;                             // edge thickness / mating distance

   //---Preliminaries---------------------------
   if(dFL != 0.00) dFL = 20.00 / dFL;
   if(dFL > 0.30) dFL = 0.30;
   if(dFL <-0.30) dFL =-0.30;

   //===Null Lens=========================================
   if(dFL==0.00) {
      memcpy(SxyzPatch1, SxyzLensFlat, (SiN1=16)*3*sizeof(double));
      memcpy(SxyzPatch2, SxyzLensFlat, (SiN2=16)*3*sizeof(double));
      Scale3d(SxyzPatch1, SiN1, 1.00,  1.00, 0.00);
      Scale3d(SxyzPatch2, SiN2, 1.00, -1.00, 0.00);
      return;
   }

   //===Focusing Lenses====================================
   memcpy(SxyzPatch1, SxyzLensInner, (SiN1=8)*3*sizeof(double)); // inner front
   memcpy(SxyzPatch2, SxyzLensInner, (SiN2=8)*3*sizeof(double)); // inner back
   memcpy( SxyzPatch,         SxyzLens,     24*4*3*sizeof(double)); // front rings
   memcpy(&SxyzPatch[24*4*3], SxyzLens,     24*4*3*sizeof(double)); // back rings
   SiN = 48;

   //---Flip back facets------------------------
   Scale3d(SxyzPatch2, SiN2, 1.00, -1.00, 1.00); // flip back
   Scale3d(&SxyzPatch[24*4*3], 24*4, 1.00, -1.00, 1.00); // flip back

   //---Scale for focal length------------------
   // note opposite sign (compared to mirror)
   Scale3d(&SxyzPatch[00*4*3], 24*4, 1.00, 1.00,-dFL);
   Scale3d(&SxyzPatch[24*4*3], 24*4, 1.00, 1.00,+dFL);

   //---Edge Thickness--------------------------
   if(dFL > 0.00) dThk = (dFL < 0.08) ? 0.06 : 0.00;
   else dThk = (-dFL) + 0.06;

   //---Adjust faces----------------------------
   Translate3d(&SxyzPatch[00*4*3], 24*4, 0.00, 0.00, -dThk-dFL);
   Translate3d(&SxyzPatch[24*4*3], 24*4, 0.00, 0.00, +dThk+dFL);
   Translate3d(SxyzPatch1, SiN1, 0.00, 0.00, -dThk-dFL);
   Translate3d(SxyzPatch2, SiN2, 0.00, 0.00, +dThk+dFL);

   //---Mate edge-------------------------------
   if(dThk > 0.00) {
      memcpy(&SxyzPatch[SiN*4*3], SxyzLensEdge, 16*4*3*sizeof(double)); // edge
      Scale3d(&SxyzPatch[SiN*4*3], 16*4, 1.00, 1.00, dThk);
      SiN += 16;
   }
}

/*********************************************************
* Screen
*********************************************************/
static double SxyzScreen1[] = {
   -1.00,+1.00,0.00,
   -1.00,-1.00,0.00,
   +1.00,-1.00,0.00,
   +1.00,+1.00,0.00};
static double SxyzScreen2[] = {
   -1.00,-1.00,0.00,
   -1.00,+1.00,0.00,
   +1.00,+1.00,0.00,
   +1.00,-1.00,0.00};
void Renderer3dScreen(void) {
   memcpy(SxyzPatch1, SxyzScreen1, (SiN1=4)*3*sizeof(double));
   memcpy(SxyzPatch2, SxyzScreen2, (SiN2=4)*3*sizeof(double));
}


/*********************************************************
* Prism
* See also equivalent code in Renderer2d
*********************************************************/
void Renderer3dPrism(double dRefIndx) {
   if(dRefIndx<1.00) dRefIndx = 1.00;       // ensure physical refractive index
   double dApexHalfAng = M_PI_2 - ATAN(dRefIndx); // calculate apex half-angle
   double dSin, dCos;                       // sine and cosine of half-angle

   dSin = SIN(dApexHalfAng);
   dCos = COS(dApexHalfAng);

   //---Top facet-------------------------------
   SxyzPatch1[0*3+0] = 0.00;  SxyzPatch1[0*3+1] = +0.50; SxyzPatch1[0*3+2] = 0.00;
   SxyzPatch1[1*3+0] = +dCos; SxyzPatch1[1*3+1] = +0.50; SxyzPatch1[1*3+2] = -dSin;
   SxyzPatch1[2*3+0] = +dCos; SxyzPatch1[2*3+1] = +0.50; SxyzPatch1[2*3+2] = +dSin;
   SiN1 = 3;

   //---Bottom facet----------------------------
   SxyzPatch2[0*3+0] = 0.00;  SxyzPatch2[0*3+1] = -0.50; SxyzPatch2[0*3+2] = 0.00;
   SxyzPatch2[1*3+0] = +dCos; SxyzPatch2[1*3+1] = -0.50; SxyzPatch2[1*3+2] = +dSin;
   SxyzPatch2[2*3+0] = +dCos; SxyzPatch2[2*3+1] = -0.50; SxyzPatch2[2*3+2] = -dSin;
   SiN2 = 3;

   //---Walls-----------------------------------
   SxyzPatch[0*4*3+0*3+0] = 0.000; SxyzPatch[0*4*3+0*3+1] = -0.50; SxyzPatch[0*4*3+0*3+2] = 0.000;
   SxyzPatch[0*4*3+1*3+0] = 0.000; SxyzPatch[0*4*3+1*3+1] = +0.50; SxyzPatch[0*4*3+1*3+2] = 0.000;
   SxyzPatch[0*4*3+2*3+0] = +dCos; SxyzPatch[0*4*3+2*3+1] = +0.50; SxyzPatch[0*4*3+2*3+2] = +dSin;
   SxyzPatch[0*4*3+3*3+0] = +dCos; SxyzPatch[0*4*3+3*3+1] = -0.50; SxyzPatch[0*4*3+3*3+2] = +dSin;

   SxyzPatch[1*4*3+0*3+0] = +dCos; SxyzPatch[1*4*3+0*3+1] = -0.50; SxyzPatch[1*4*3+0*3+2] = +dSin;
   SxyzPatch[1*4*3+1*3+0] = +dCos; SxyzPatch[1*4*3+1*3+1] = +0.50; SxyzPatch[1*4*3+1*3+2] = +dSin;
   SxyzPatch[1*4*3+2*3+0] = +dCos; SxyzPatch[1*4*3+2*3+1] = +0.50; SxyzPatch[1*4*3+2*3+2] = -dSin;
   SxyzPatch[1*4*3+3*3+0] = +dCos; SxyzPatch[1*4*3+3*3+1] = -0.50; SxyzPatch[1*4*3+3*3+2] = -dSin;

   SxyzPatch[2*4*3+0*3+0] = +dCos; SxyzPatch[2*4*3+0*3+1] = -0.50; SxyzPatch[2*4*3+0*3+2] = -dSin;
   SxyzPatch[2*4*3+1*3+0] = +dCos; SxyzPatch[2*4*3+1*3+1] = +0.50; SxyzPatch[2*4*3+1*3+2] = -dSin;
   SxyzPatch[2*4*3+2*3+0] = 0.000; SxyzPatch[2*4*3+2*3+1] = +0.50; SxyzPatch[2*4*3+2*3+2] = 0.000;
   SxyzPatch[2*4*3+3*3+0] = 0.000; SxyzPatch[2*4*3+3*3+1] = -0.50; SxyzPatch[2*4*3+3*3+2] = 0.000;
   SiN = 3;
}


/*********************************************************
* Renderer3dPlate
* This should only be called for the input side.
* The calculations here are straight-forward because the
* OpticCanvasAngle returns the angle of the face.
* Note similarity of this code to that of lens
*********************************************************/
#define BLOCKSIZE 0.6182818          // relative width of blocks to optics
void Renderer3dPlate(CVertex *pVx, double dSize) {
   int iPtch, k;                            // patch, vertex loop counter
   double dROC1, dROC2;                     // face curvatures

   if((pVx==NULL) || (pVx->VxLinked()==NULL)) return; // ignore bad references

   //===Preliminaries=====================================
   dSize *= BLOCKSIZE * 0.50;               // scale down size
   SiN = 0;                                 // no patches yet

   dROC1 = pVx->ROC(TAN);
   if(dROC1 != 0.00) dROC1 = 20.00 / dROC1;
   if(dROC1 > 0.30) dROC1 = 0.30;
   if(dROC1 <-0.30) dROC1 =-0.30;

   dROC2 = pVx->VxLinked()->ROC(TAN);
   if(dROC2 != 0.00) dROC2 = 20.00 / dROC2;
   if(dROC2 > 0.30) dROC2 = 0.30;
   if(dROC2 <-0.30) dROC2 =-0.30;

   //===Edges=============================================
   // We here use SxyzPatch1 and SxyzPatch2 for temporary storage
   for(k=0; k<16; k++) {
      SxyzPatch1[3*k+0] = SxyzPatch2[3*k+0] = COS(2*M_PI*k/16);
      SxyzPatch1[3*k+1] = SxyzPatch2[3*k+1] = SIN(2*M_PI*k/16);
      SxyzPatch1[3*k+2] =-dROC1;
      SxyzPatch2[3*k+2] = dROC2;
   }
   SiN1 = SiN2 = 16;
   Scale3d    (SxyzPatch1, SiN1, dSize, dSize, dSize);
   Scale3d    (SxyzPatch2, SiN2, dSize, dSize, dSize);
   Rotate3d   (SxyzPatch1, SiN1, 0.00, pVx->OpticCanvasAngle()*180.00/M_PI, 0.00);
   Rotate3d   (SxyzPatch2, SiN2, 0.00, pVx->VxLinked()->OpticCanvasAngle()*180.00/M_PI, 0.00);
   Translate3d(SxyzPatch1, SiN1, pVx->Y(), 0.00, pVx->X());
   Translate3d(SxyzPatch2, SiN2, pVx->VxLinked()->Y(), 0.00, pVx->VxLinked()->X());

   for(iPtch=0; iPtch<16; iPtch++) {
      SxyzPatch[iPtch*4*3+3*0+0] = SxyzPatch1[3*((iPtch+0)%16)+0];
      SxyzPatch[iPtch*4*3+3*0+1] = SxyzPatch1[3*((iPtch+0)%16)+1];
      SxyzPatch[iPtch*4*3+3*0+2] = SxyzPatch1[3*((iPtch+0)%16)+2];

      SxyzPatch[iPtch*4*3+3*1+0] = SxyzPatch2[3*((iPtch+0)%16)+0];
      SxyzPatch[iPtch*4*3+3*1+1] = SxyzPatch2[3*((iPtch+0)%16)+1];
      SxyzPatch[iPtch*4*3+3*1+2] = SxyzPatch2[3*((iPtch+0)%16)+2];

      SxyzPatch[iPtch*4*3+3*2+0] = SxyzPatch2[3*((iPtch+1)%16)+0];
      SxyzPatch[iPtch*4*3+3*2+1] = SxyzPatch2[3*((iPtch+1)%16)+1];
      SxyzPatch[iPtch*4*3+3*2+2] = SxyzPatch2[3*((iPtch+1)%16)+2];

      SxyzPatch[iPtch*4*3+3*3+0] = SxyzPatch1[3*((iPtch+1)%16)+0];
      SxyzPatch[iPtch*4*3+3*3+1] = SxyzPatch1[3*((iPtch+1)%16)+1];
      SxyzPatch[iPtch*4*3+3*3+2] = SxyzPatch1[3*((iPtch+1)%16)+2];
   }
   SiN = 16;
   Scale3d(SxyzPatch2, SiN2, 1.00,-1.00, 1.00);

   //===Input Face========================================
   if(dROC1 != 0.00) {
      memcpy     (SxyzPatch1, SxyzLensInner, (SiN1=8)*3*sizeof(double)); // inner front
      Scale3d    (SxyzPatch1, SiN1, dSize, dSize, 0.00);
      if(dROC1<0.00) Translate3d(SxyzPatch1, SiN1, 0.00, 0.00, -dROC1);
      Rotate3d   (SxyzPatch1, SiN1, 0.00, pVx->OpticCanvasAngle()*180.00/M_PI, 0.00);
      Translate3d(SxyzPatch1, SiN1, pVx->Y(), 0.00, pVx->X());

      memcpy     (&SxyzPatch[SiN*4*3], SxyzLens, 24*4*3*sizeof(double)); // front rings
      Scale3d    (&SxyzPatch[SiN*4*3], 24*4, dSize, dSize, dSize*dROC1);
      if(dROC1<0.00) Translate3d(&SxyzPatch[SiN*4*3], 24*4, 0.00, 0.00, -dROC1);
      Rotate3d   (&SxyzPatch[SiN*4*3], 24*4, 0.00, pVx->OpticCanvasAngle()*180.00/M_PI, 0.00);
      Translate3d(&SxyzPatch[SiN*4*3], 24*4, pVx->Y(), 0.00, pVx->X());
      SiN += 24;
   }

   //===Output Face=======================================
   if(dROC2!=0.00) {
      memcpy     (SxyzPatch2, SxyzLensInner, (SiN2=8)*3*sizeof(double)); // inner front
      Scale3d    (SxyzPatch2, SiN2, dSize,-dSize, 0.00);
      if(dROC2<0.00) Translate3d(SxyzPatch2, SiN2, 0.00, 0.00, dROC2);
      Rotate3d   (SxyzPatch2, SiN2, 0.00, pVx->VxLinked()->OpticCanvasAngle()*180.00/M_PI, 0.00);
      Translate3d(SxyzPatch2, SiN2, pVx->VxLinked()->Y(), 0.00, pVx->VxLinked()->X());

      memcpy     (&SxyzPatch[SiN*4*3], SxyzLens, 24*4*3*sizeof(double)); // back rings
      Scale3d    (&SxyzPatch[SiN*4*3], 24*4, dSize,-dSize,-dSize*dROC2);
      if(dROC2<0.00) Translate3d(&SxyzPatch[SiN*4*3], 24*4, 0.00, 0.00, dROC2);
      Rotate3d   (&SxyzPatch[SiN*4*3], 24*4, 0.00, pVx->OpticCanvasAngle()*180.00/M_PI, 0.00);
      Translate3d(&SxyzPatch[SiN*4*3], 24*4, pVx->VxLinked()->Y(), 0.00, pVx->VxLinked()->X());
      SiN += 24;
   }
}




/*########################################################
 ## Mode
########################################################*/
/*********************************************************
* Baseplate
* Creates a simple square underneath the optics.
*********************************************************/
void Renderer3dBaseplate(const CVertex *pVxTop, double dOpticScale) {
   CVertex *pVx;                            // loop counter
   double dXMin, dXMax, dYMin, dYMax;       // extreme range
   //---Establish dimensions--------------------
   pVx = (CVertex*) pVxTop;
   dXMin = dXMax = pVx->X();
   dYMin = dYMax = pVx->Y();
   for(; pVx; pVx=(CVertex*)pVx->Next()) {
      if(pVx->X() < dXMin) dXMin = pVx->X();
      if(pVx->X() > dXMax) dXMax = pVx->X();
      if(pVx->Y() < dYMin) dYMin = pVx->Y();
      if(pVx->Y() > dYMax) dYMax = pVx->Y();
   }
   dXMin -= dOpticScale;
   dXMax += dOpticScale;
   dYMin -= dOpticScale;
   dYMax += dOpticScale;

   //---Object----------------------------------
   SxyzPatch1[3*0+0] = dYMin; SxyzPatch1[3*0+1] = dOpticScale/2.00; SxyzPatch1[3*0+2] = dXMin;
   SxyzPatch1[3*1+0] = dYMin; SxyzPatch1[3*1+1] = dOpticScale/2.00; SxyzPatch1[3*1+2] = dXMax;
   SxyzPatch1[3*2+0] = dYMax; SxyzPatch1[3*2+1] = dOpticScale/2.00; SxyzPatch1[3*2+2] = dXMax;
   SxyzPatch1[3*3+0] = dYMax; SxyzPatch1[3*3+1] = dOpticScale/2.00; SxyzPatch1[3*3+2] = dXMin;
   SiN1 = 4;
}

/*********************************************************
* Renderer3dMode
*********************************************************/
#define SiNT 11
#define Nr   12
static double SdZT[SiNT] = {-3.0000, -1.1665, -0.6933, -0.4113, -0.1945, +0.0000, +0.1945, +0.4113, +0.6933, +1.1665, +3.0000};
static double CosNr[Nr] = {+1.0000, +0.8660, +0.5000, +0.0000, -0.5000, -0.8660, -1.0000, -0.8660, -0.5000, -0.0000, +0.5000, +0.8660};
static double SinNr[Nr] = {+0.0000, +0.5000, +0.8660, +1.0000, +0.8660, +0.5000, +0.0000, -0.5000, -0.8660, -1.0000, -0.8660, -0.5000};

/*********************************************************
* Renderer3dMode
* Because of how the  choice of rims is  decided, using a
* method  similar to that in the 2d renderer, we  have to
* set the coordinates dynamically.
*********************************************************/
void Renderer3dMode(CVertex *pVx, double dSize) {
   int kx, ky, kz, kr;                      // table, beam axis, radial loop counters
   int iNz;                                 // number of points along Z table

   if((pVx==NULL) || (pVx->Next()==NULL)) return; // ignore missing segment

   //===Cylinder prototype================================
   /*double dSegLen = pVx->Dist2Next();
   for(kr=0; kr<Nr; kr++) {
      SxyzPatch[3*4*kr+0*3+0] = CosNr[(kr+0)%Nr];
      SxyzPatch[3*4*kr+0*3+1] = SinNr[(kr+0)%Nr];
      SxyzPatch[3*4*kr+0*3+2] = 0.00;

      SxyzPatch[3*4*kr+1*3+0] = CosNr[(kr+0)%Nr];
      SxyzPatch[3*4*kr+1*3+1] = SinNr[(kr+0)%Nr];
      SxyzPatch[3*4*kr+1*3+2] = 1.00;

      SxyzPatch[3*4*kr+2*3+0] = CosNr[(kr+1)%Nr];
      SxyzPatch[3*4*kr+2*3+1] = SinNr[(kr+1)%Nr];
      SxyzPatch[3*4*kr+2*3+2] = 1.00;

      SxyzPatch[3*4*kr+3*3+0] = CosNr[(kr+1)%Nr];
      SxyzPatch[3*4*kr+3*3+1] = SinNr[(kr+1)%Nr];
      SxyzPatch[3*4*kr+3*3+2] = 0.00;
   }
   SiN = Nr;
   Scale3d    (SxyzPatch, SiN*4, dSize, dSize, dSegLen);
   Rotate3d   (SxyzPatch, SiN*4, 0.00, -pVx->SegmentCanvasAngle()*180.00/M_PI, 0.00);
   Translate3d(SxyzPatch, SiN*4, pVx->Y(), 0.00, pVx->X());// */

   //===Preliminaries=====================================
   // It's perhaps a bit wasteful to use all of these va-
   // riables, but it made it easier to port the code be-
   // low from Matlab.
   // Here, the X plane is tangential, the Y sagittal
   double dW0X, dW0Y, dZ0X, dZ0Y;           // waist and waist positions
   double dM2LX, dM2LY;                     // system M^2 * lambda
   double dZMax;                            // segment limits
   double dZRX, dZRY;                       // Rayleigh lengths

   //---Inputs Parameters-----------------------
   dW0X   = pVx->Q(TAN)->W0();   dZ0X = pVx->Q(TAN)->z0();
   dW0Y   = pVx->Q(SAG)->W0();   dZ0Y = pVx->Q(SAG)->z0();
   dM2LX  = pVx->SysParent()->WLen() * pVx->SysParent()->MSquared(TAN);
   dM2LY  = pVx->SysParent()->WLen() * pVx->SysParent()->MSquared(SAG);
   dZMax  = pVx->Dist2Next();

   dM2LX /= pVx->RefIndex();
   dM2LY /= pVx->RefIndex();
   dZ0X   *= pVx->RefIndex();
   dZ0Y   *= pVx->RefIndex();

   //---Derived Values--------------------------
   if(dM2LX <= 1.00) dM2LX = 1.00;          // prevent #DIV/0! errors
   if(dM2LY <= 1.00) dM2LY = 1.00;          // prevent #DIV/0! errors
   dZRX = M_PI * SQR(dW0X) / dM2LX;         // Rayleigh lengths
   dZRY = M_PI * SQR(dW0Y) / dM2LY;
   if(dZRX==0.00) dZRX = 1.00;              // prevent #DIV/0! errors
   if(dZRY==0.00) dZRY = 1.00;              // prevent #DIV/0! errors

   //===Assemble Z Table==================================
   // Strategy
   //  - First point ZMin, last point ZMax, and waists(?)
   //    are always mapped
   //  - Walk through the table to whose waist we're clo-
   //    sest
   //  - Swap to other table if we're closer to its waist
   // Thus we have at most both  full tables plus the two
   // ends.
   // It looks a little weird with  wireframe, but should
   // be ok once we use filled patches.
   static double ddX[2*SiNT+2];             // mode X size at position
   static double ddY[2*SiNT+2];             // mode Y size at position
   static double ddZ[2*SiNT+2];             // position Z point
   double zX, zY;                           // temporary table values
   int iTab;                                // table to use (0=x, 1=y)

   iTab = 0;                                // start using x table
   kz   = 0;                                // start filling at top

   //---First point-----------------------------
   ddZ[kz] = 0.00; kz++;                    // starting optic

   //---Fill table------------------------------
   kx = ky = 0;                             // start at beginning of table
   while((ddZ[kz-1] < dZMax) & (kz <= 2*SiNT)) {
      while((kx<SiNT) && (SdZT[kx] <= (ddZ[kz-1]-dZ0X)/dZRX+1e-6))  kx++;
      while((ky<SiNT) && (SdZT[ky] <= (ddZ[kz-1]-dZ0Y)/dZRY+1e-6))  ky++;
      if((kx >= SiNT) && (ky >= SiNT)) break; // skip once both tables are finished
      zX = SdZT[kx] * dZRX + dZ0X;
      zY = SdZT[ky] * dZRY + dZ0Y;
      //if((iTab==0) & (fabs(zY-dZ0Y)/dZRY < fabs(zX-dZ0X)/dZRX)) iTab = 1;
      //if((iTab==1) & (fabs(zY-dZ0Y)/dZRY > fabs(zX-dZ0X)/dZRX)) iTab = 0;
      iTab = (zX < zY) ? 0 : 1;
      if(kx >= SiNT) iTab = 1;
      if(ky >= SiNT) iTab = 0;
      if(iTab==0) ddZ[kz] = zX; else ddZ[kz] = zY;
      kz++;
   }
   if((kz>1) && (ddZ[kz-1]>dZMax)) kz--;    // strip last if it's too far
   ddZ[kz] = dZMax; kz++;
   iNz = kz;

   //===Create Patches====================================
   //---Modes-----------------------------------
   for(kz=0; kz<iNz; kz++) {
      ddX[kz] = dW0X * SQRT(1.00 + SQR((ddZ[kz]-dZ0X)/dZRX));
      ddY[kz] = dW0Y * SQRT(1.00 + SQR((ddZ[kz]-dZ0Y)/dZRY));
   }

   //---Radial patches--------------------------
   SiN = 0;                                 // count created patches
   for(kz=0; (kz<iNz-1) && (SiN<C3DI_NUM_PATCH); kz++) {
      for(kr=0; (kr<Nr) && (SiN<C3DI_NUM_PATCH); kr++) {
         SxyzPatch[3*4*SiN+0*3+0] = ddX[kz+0] * CosNr[(kr+0)%Nr];
         SxyzPatch[3*4*SiN+0*3+1] = ddY[kz+0] * SinNr[(kr+0)%Nr];
         SxyzPatch[3*4*SiN+0*3+2] = ddZ[kz+0];

         SxyzPatch[3*4*SiN+1*3+0] = ddX[kz+1] * CosNr[(kr+0)%Nr];
         SxyzPatch[3*4*SiN+1*3+1] = ddY[kz+1] * SinNr[(kr+0)%Nr];
         SxyzPatch[3*4*SiN+1*3+2] = ddZ[kz+1];

         SxyzPatch[3*4*SiN+2*3+0] = ddX[kz+1] * CosNr[(kr+1)%Nr];
         SxyzPatch[3*4*SiN+2*3+1] = ddY[kz+1] * SinNr[(kr+1)%Nr];
         SxyzPatch[3*4*SiN+2*3+2] = ddZ[kz+1];

         SxyzPatch[3*4*SiN+3*3+0] = ddX[kz+0] * CosNr[(kr+1)%Nr];
         SxyzPatch[3*4*SiN+3*3+1] = ddY[kz+0] * SinNr[(kr+1)%Nr];
         SxyzPatch[3*4*SiN+3*3+2] = ddZ[kz+0];
         SiN++;
      }
   }

   //---Rescale---------------------------------
   Scale3d    (SxyzPatch, SiN*4, dSize/1000.00, dSize/1000.00, 1.00);
   Rotate3d   (SxyzPatch, SiN*4, 0.00, -pVx->SegmentCanvasAngle()*180.00/M_PI, 0.00);
   Translate3d(SxyzPatch, SiN*4, pVx->Y(), 0.00, pVx->X());
}




/*########################################################
 ## Renderers
########################################################*/
/*********************************************************
* Renderer3dWireframe
* Display the current buffer as a wireframe
*********************************************************/
#define fnVisible(xyz) (((xyz)[3*1+0]-(xyz)[3*0+0])*((xyz)[3*2+1]-(xyz)[3*1+1]) > ((xyz)[3*1+1]-(xyz)[3*0+1])*((xyz)[3*2+0]-(xyz)[3*1+0]))
//#define fnVisible(xyz) (1)
void Renderer3dWireframe(HDC hdc, double xyzLoc[], double xyzLkAt[], double dView, RECT *prcWindow) {
   int iPtch, k;                            // patch, vertex loop counters
   int x0, y0;                              // window centers
   double dClipZ = 20.00;                   // clipping Z plane
   BOOL tfMove;                             // move instead of line

   Camera3d(SxyzPatch1, SiN1,  xyzLoc, xyzLkAt, dView, prcWindow);
   Camera3d(SxyzPatch2, SiN2,  xyzLoc, xyzLkAt, dView, prcWindow);
   Camera3d(SxyzPatch,  SiN*4, xyzLoc, xyzLkAt, dView, prcWindow);

   x0 = (prcWindow->left + prcWindow->right) / 2;
   y0 = (prcWindow->bottom + prcWindow->top) / 2;

   //---First Patch-----------------------------
   if(SiN1==2) {
      if((SxyzPatch1[3*0+2] > dClipZ) && (SxyzPatch1[3*1+2] > dClipZ)) {
         MoveTo(hdc, x0+SxyzPatch1[3*0+0], y0+SxyzPatch1[3*0+1]);
         LineTo(hdc, x0+SxyzPatch1[3*1+0], y0+SxyzPatch1[3*1+1]);
      }
   } else if((SiN1>2) && fnVisible(SxyzPatch1)) {
      for(k=0, tfMove=TRUE; k<=SiN1; k++) {
         if(SxyzPatch1[3*(k%SiN1)+2] < dClipZ) tfMove = TRUE;
         else {
            if(tfMove) MoveTo(hdc, x0+SxyzPatch1[3*(k%SiN1)+0], y0+SxyzPatch1[3*(k%SiN1)+1]);
            else       LineTo(hdc, x0+SxyzPatch1[3*(k%SiN1)+0], y0+SxyzPatch1[3*(k%SiN1)+1]);
            tfMove = FALSE;
         }
      }
   }

   //---Second Patch----------------------------
   if((SiN2>2) && fnVisible(SxyzPatch2)) {
      for(k=0, tfMove=TRUE; k<=SiN2; k++) {
         if(SxyzPatch2[3*(k%SiN2)+2] < dClipZ) tfMove = TRUE;
         else {
            if(tfMove) MoveTo(hdc, x0+SxyzPatch2[3*(k%SiN2)+0], y0+SxyzPatch2[3*(k%SiN2)+1]);
            else       LineTo(hdc, x0+SxyzPatch2[3*(k%SiN2)+0], y0+SxyzPatch2[3*(k%SiN2)+1]);
            tfMove = FALSE;
         }
      }
   }

   //---Square Patches--------------------------
   for(iPtch=0; iPtch<SiN; iPtch++) {
      if(fnVisible(&SxyzPatch[3*4*iPtch])) {
         for(k=0, tfMove=TRUE; k<=4; k++) {
            if(SxyzPatch[3*4*iPtch+3*(k%4)+2] < dClipZ) tfMove = TRUE;
            else {
               if(tfMove) MoveTo(hdc, x0+SxyzPatch[3*4*iPtch+3*(k%4)+0], y0+SxyzPatch[3*4*iPtch+3*(k%4)+1]);
               else       LineTo(hdc, x0+SxyzPatch[3*4*iPtch+3*(k%4)+0], y0+SxyzPatch[3*4*iPtch+3*(k%4)+1]);
               tfMove = FALSE;
            }
         }
      }
   }

   //---Reset counters--------------------------
   SiN1 = SiN2 = SiN = 0;                   // flush counters once painted
}
