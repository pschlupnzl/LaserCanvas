/*********************************************************
* CVertex.h
* $PSchlup 2006 $     $Revision 5 $
*********************************************************/
#ifndef CVERTEX_H                           // prevent multiple includes
#define CVERTEX_H
//===Classes defined======================================
class CVertex;                              // vertex class

//===Includes=============================================
#include <windows.h>                        // standard Windows include
#include <stdio.h>                          // standard header

#include "CSystem.h"                        // system class
#include "CSysWin.h"                        // system renderers base class
#include "CLCEqtn.h"                        // equation objects
#include "CAbcd.h"                          // ABCD / Matrix2x2 classes

//===Constants============================================
#define CVXC_MAXTAG             31          // length of tag string

//---Auxiliary names for saving-----------------
#define CVXE_SAVE_SELECTED       0          // (index) selected
#define CVXE_SAVE_SPAWN          1          // (index) spawned system
#define CVXE_SAVE_VXLINK         2          // (index) linked vertex
#define CVXE_SAVE_LOCKANGL       3          // (index) angle is locked
#define CVXE_SAVE_FLIPPED        4          // (index) optic is flipped
#define CVXE_SAVE_ROCFLASTIG     5          // (index) optic is astigmatic
#define CVXE_NUM_SAVE            6          // (count) number of auxiliary names

//---Vertex types-------------------------------
// User-level objects
//    Mirror
//    Lens
//    Block
//    Brewster
//    Prism pair
//    Screen
//    Source
///TODO: Feature Request: Thick Lenses, Lenses at Arbitrary Angles
///TODO: Feature Request: Arbitrary curvature for crystal face
///TODO: Feature Request: Prism insertion (what does that do to the ABCD?)

// Special Types and Their Uses
//  - FlatMirror: For within a prism pair
//  - ThermalLens: For inside block / Brewster block.
//        pVxAux points to the input face
//  - OutputCoupler: The transmissive part of the
//        optic, so thickness and adopt curvature

// Generally, the  paradigm of version 5 is to  have more
// unique types rather than fewer  with flags. This makes
// the code a little  longer, but in many  cases the pro-
// cessing is automated anyway.

#define CVX_TYPE_MIRROR          0          // (counter) plane or curved mirror
#define CVX_TYPE_LENS            1          // (counter) thin lens
#define CVX_TYPE_FLATMIRROR      2          // (counter) flat mirror: between prisms
#define CVX_TYPE_THERMALLENS     3          // (counter) thermal lens within a material
#define CVX_TYPE_SCREEN          4          // (counter) observation screen
#define CVX_TYPE_SOURCE          5          // (counter) user input beam source
#define CVX_TYPE_INCRYSTAL       6          // (counter) normal incidence input face
#define CVX_TYPE_OUTCRYSTAL      7          // (counter) normal incidence output face
#define CVX_TYPE_INBREWSTER      8          // (counter) brewster block input
#define CVX_TYPE_OUTBREWSTER     9          // (counter) brewster block output
#define CVX_TYPE_PRISM1         10          // (counter) first prism input face
#define CVX_TYPE_PRISM2         11          // (counter) first prism output face
#define CVX_TYPE_INPLATE        12          // (counter) parallel plate input face
#define CVX_TYPE_OUTPLATE       13          // (counter) parallel plate output face
#define CVX_TYPE_OUTCOUPLER     14          // (counter) output coupler
#define CVX_NUM_TYPE            15          // (count) number of types

//---Properties---------------------------------
// Some of these properties could be piggy-backed, but it
// would make some of the other code more specific. Since
// memory is pretty cheap these days let's just keep each
// property separate
// Canvas properties:  Since the  model system  exists in
// real space, the X, Y, and angle  properties are "real"
// and not just a 2d-renderer property.
// Plottable  functions are listed  FIRST! The  names are
// listed in the RC file. For  plotting, the sagittal va-
// lues also represent the sagittal/tangential pair.
///TODO: Check if we really need CANVAS_X and CANVAS_Y
#define CVXI_PROP_MODESAG        0          // (index) mode size sagittal
#define CVXI_PROP_CURVSAG        1          // (index) curvature sagittal
#define CVXI_PROP_WAISTSAG       2          // (index) sagittal waist size in segment
#define CVXI_PROP_WDISTSAG       3          // (index) distance to sagittal waist
#define CVXI_PROP_ZRSAG          4          // (index) sagittal Rayleigh range
#define CVXI_PROP_ASTIG          5          // (index) astigmatism
//   .   .   .   .   .   .   .  |||   .   .   .   .   .
#define CVXI_PROP_FCNMAX         5          // (index) highest allowed plotting function
#define CVXI_PROP_MODETAN        6          // (index) mode size tangential
#define CVXI_PROP_CURVTAN        7          // (index) curvature tangential
#define CVXI_PROP_WAISTTAN       8          // (index) tangential waist size in segment
#define CVXI_PROP_WDISTTAN       9          // (index) distance to tangential waist
#define CVXI_PROP_ZRTAN         10          // (index) tangential Rayleigh range
//   .   .   .   .   .   .   .  \|/   .   .   .   .   .
#define CVXI_PROP_EQTN          11          // (index) start of equation-mapping properties
//   .   .   .   .   .   .   .  |||   .   .   .   .   .
#define CVXI_PROP_DIST2NEXT     11          // (index) (CVXI_EQTN_DIST2NEXT)
#define CVXI_PROP_N             12          // (index) (CVXI_EQTN_N)
#define CVXI_PROP_FACEANGLE     13          // (index) (CVXI_EQTN_FACEANGLE)
#define CVXI_PROP_ROC           14          // (index) (CVXI_EQTN_ROC)
#define CVXI_PROP_ROCTAN        15          // (index) (CVXI_EQTN_ROCTAN)
#define CVXI_PROP_FL            16          // (index) (CVXI_EQTN_FL)
#define CVXI_PROP_FLTAN         17          // (index) (CVXI_EQTN_FLTAN)
#define CVXI_PROP_THICKNESS     18          // (index) thickness of output coupler / prism insertion
//   .   .   .   .   .   .   .  \|/   .   .   .   .   .
#define CVXI_PROP_CANVAS_X      19          // (index) optic canvas X position
#define CVXI_PROP_CANVAS_Y      20          // (index) optic canvas Y position
#define CVXI_PROP_CANVAS_ANGLE  21          // (index) optic canvas angle
#define CVXI_PROP_SEGMENT_ANGLE 22          // (index) segment canvas angle
#define CVXI_NUM_PROP           23          // (count) number of properties

//---User Properties----------------------------
// These represent user-settable properties that are ren-
// dered as CLCEqtn objects and saved in data files
// When the system  is rendered, e.g. by ApplySystemABCD,
// the equation values are copied to the property buffers
// and, if necessary, to the graphing renderer arrays. In
// this way, the equations  are only solved once, at spe-
// cific times, and subsequent  calls to retrieve the va-
// lues will return the same values.
// ***** See also pszPropertyName[] in CVertex.cpp *****
#define CVXI_EQTN_DIST2NEXT      0          // (index) distance to next optic
#define CVXI_EQTN_N              1          // (index) refractive index
#define CVXI_EQTN_FACEANGLE      2          // (index) interface / incidence angle
#define CVXI_EQTN_ROC            3          // (index) radius of curvature
#define CVXI_EQTN_ROCTAN         4          // (index) tangential ROC
#define CVXI_EQTN_FL             5          // (index) focal length
#define CVXI_EQTN_FLTAN          6          // (index) tangential focal length
#define CVXI_EQTN_THICKNESS      7          // (index) thickness / insertion
#define CVXI_NUM_EQTN            8          // (count) number of equations

//---Property Masks-----------------------------
// For each type of vertex, only some properties are used
// The sequence of CVXI indices and CVXB bits must match,
// then we can  combine the relevant bit  fields for each
// vertex type
// The masks include both equations and static properties
#define CVXB_EQTN_DIST2NEXT      0x00000001 // (bit) distance to next optic
#define CVXB_EQTN_N              0x00000002 // (bit) refractive index
#define CVXB_EQTN_FACEANGLE      0x00000004 // (bit) face angle
#define CVXB_EQTN_ROC            0x00000008 // (bit) radius of curvature
#define CVXB_EQTN_ROCTAN         0x00000010 // (bit) tangential ROC
#define CVXB_EQTN_FL             0x00000020 // (bit) focal length
#define CVXB_EQTN_FLTAN          0x00000040 // (bit) tangential focal length
#define CVXB_EQTN_THICKNESS      0x00000080 // (bit) thickness / insertion

//---Flags--------------------------------------
#define CVXF_SELECTED       0x0001          // (bit) vertex selected
#define CVXF_FLIPPED        0x0002          // (bit) vertex is flipped
#define CVXF_LOCKANGL       0x0004          // (bit) lock face angle (no anchor)
#define CVXF_ROCFLASTIG     0x0008          // (bit) astigmatic ROC/FL
#define CVXF_DRAFTLINK      0x8000          // (bit) link valid in draft mode

//===Class================================================
class CVertex {
private:
   char       szVxTag[CVXC_MAXTAG+1];       // tag / user tag
   CSystem   *pSysParent;                   // parent system
   CVertex   *pVxPrev;                      // previous vertex in chain
   CVertex   *pVxNext;                      // next vertex in chain
   CVertex   *pVxLink;                      // linked vertex for blocks etc
   CSystem   *pSysSpawned;                  // spawned system
   int        iVxType;                      // type of vertex
public:///TODO: make private
   double     ddProp[CVXI_NUM_PROP];        // valued properties
private:
   CEquation  EEq[CVXI_NUM_EQTN];           // vertex equationed properties
   UINT       uFlags;                       // vertex flags
public:///TODO: make private
   CRecQ      QVx[SAGTAN];                  // Q starting into next space

public:
   CVertex(CSystem *pSys, int iTyp);        // constructor
   ~CVertex();                              // destructor

   //---Moving functions------------------------
   BOOL      CanStretch(BOOL tfFwd);        // query if segment can stretch
   BOOL      CanAnchor(void);               // query if vertex can act as anchor
   void      InitMoveVxTo(void);            // prepare vertex chains
   void      MoveVxTo(double dX, double dY); // constrained move vx to position
   void      InitMoveVxToOld(void);            // prepare vertex chains
   void      MoveVxToOld(double dX, double dY); // constrained move vx to position
   void      EndMoveVxTo(void);             // finish the move
   void      MoveVxToDraft(double dX, double dY); // Vx move in draft mode


   //---Data files------------------------------
   void      SaveVertex(HANDLE hFile);       // save vertex properties
   BOOL      LoadVertex(const char *pszDataFile, char **pszMin, char **pszMax); // load a vertex from file

   //---Properties------------------------------
   void      SetCanvasPosition(double dX, double dY, double dA); // nontrivial absolute angle function
   double    X(void);
   double    Y(void);
   double    OpticCanvasAngle(void);
   double    SegmentCanvasAngle(void);
   double    Angle2Next(void);              // nontrivial relative angle function

   double    Dist2Next(void);
   void      SetDist2Next(double d);
   double    FaceAngle(void);
   void      SetFaceAngle(double d);        // nontrivial face angle function

   //---General-------------
   const CEquation* Eq(int k)          { return(((k<0) || (k>=CVXI_NUM_EQTN)) ? NULL : &EEq[k]); };
   BOOL      ParseVxEq(int k, const char *pszSrc); // parse a given equation
   double   _Prop(int k)               { return(((k<0) || (k>CVXI_NUM_PROP)) ? 0.00 : ddProp[k]); };

   double    RefIndex(void);                // returns physical (n>=1) refractive index
   void      SetRefIndex(double d);         // sets the refractive index

   //---Mirror--------------
   double    ROC(int iPlane);
   void      SetROC(double dSag, double dTan);
   //---Lens----------------
   double    FL(int iPlane);
   void      SetFL(double dSag, double dTan);

   //---Output coupler------
   double    Thickness(void);
   void      SetThickness(double d);

   //---Access----------------------------------
   CApplication* App(void);                 // acces through the system
   void        SetTag(const char *psz) { strncpy(szVxTag, psz, CVXC_MAXTAG); }; // maybe also check uniqueness in system?
   BOOL        MatchTag(const char *psz){ return( (strncmp(szVxTag, psz, strlen(szVxTag))==0) ? TRUE : FALSE); };
   const char* Tag(void)               { return( (const char*) szVxTag ); };
   int         Type(void)              { return( iVxType ); };
   void        SetType(int iTyp)       { iVxType = iTyp; }; // change type (rarely needs to be used!)
   const char* TypeString(void);            // string for this vx type
   CVertex*    Prev(void)              { return(pVxPrev); };
   CVertex*    Next(void)              { return(pVxNext); };
   CVertex*    VxLinked(void)          { return(pVxLink); };
   void        SetPrev(CVertex *pVx)   { pVxPrev = pVx; };
   void        SetNext(CVertex *pVx)   { pVxNext = pVx; };
   void        SetLinked(CVertex *pVx) { pVxLink = pVx; };
   CSystem*    SysParent(void)         { return(pSysParent); };
   CSystem*    SysSpawned(void)        { return(pSysSpawned); };
   void        SetSysSpawned(CSystem *pSys) { pSysSpawned = pSys; };
   static int         NumVxFcn(void)   { return(CVXI_PROP_FCNMAX+1); };
   static const char* VxFcnString(int iIndx);      // returns the string for the given plotting function
   void        FcnValue(int iPltFcn, double *pSag, double *pTan); // retrieve a value


   //---ABCD------------------------------------
   void        ApplyEquations(double *pcdVar); // calculate the vertex and space ABCDs
   void        AbcdVx(CMatrix2x2 *pMxSag, CMatrix2x2 *pMxTan, BOOL tfForwards);
   void        AbcdSp(CMatrix2x2 *pMxSag, CMatrix2x2 *pMxTan);
   CRecQ*      Q(int k)                { return(((k>=0) && (k<SAGTAN)) ? &QVx[k] : NULL); };
   void        SetQVx(const CRecQ *pcQSag, const CRecQ *pcQTan); // set the vertex Q AND the dependent parameters
   BOOL        SymmetricQVx(void);          // returns TRUE if the segment Q is symmetric in sagittal/tangential

   //---Debug-----------------------------------
   void     DebugPrint(char *psz, int *pInt); // debug print info about vertex

   //---Flags-----------------------------------
   UINT     CheckBit(UINT u)           { return(uFlags & u); };
   void     SetBit(UINT u)             { uFlags |= u; };
   void     ClearBit(UINT u)           { uFlags &=~u; };
   void     ToggleBit(UINT u)          { uFlags ^= u; };
   BOOL     Selected(void)             { return(CheckBit(CVXF_SELECTED) ? TRUE : FALSE); };
   void     Select(BOOL tfSel)         { if(tfSel) SetBit(CVXF_SELECTED); else ClearBit(CVXF_SELECTED); };
   BOOL     IsFlipped(void)            { return( CheckBit(CVXF_FLIPPED) ? TRUE : FALSE); }; // check if optic is flipped
   void     FlipVertex(BOOL tfFlip)    { if(tfFlip) SetBit(CVXF_FLIPPED); else ClearBit(CVXF_FLIPPED); }; // flip flippable optics
   BOOL     LockedAngle(void)          { return( CheckBit(CVXF_LOCKANGL) ? TRUE : FALSE); }; // check if angle is locked
   void     LockAngle(BOOL tf)         { if(tf) SetBit(CVXF_LOCKANGL); else ClearBit(CVXF_LOCKANGL); }; // lock / unlock
   BOOL     AstigROCFL(void)      { return( CheckBit(CVXF_ROCFLASTIG) ? TRUE : FALSE); }; // check if optic is astigmatic
   void     SetAstigROCFL(BOOL tf){ if(tf) SetBit(CVXF_ROCFLASTIG); else ClearBit(CVXF_ROCFLASTIG); }; // set / clear astigmatism

   //---Tables----------------------------------
   static const char *CszVxTypeName[CVX_NUM_TYPE];         // names for vertices
   static const char *CszVxTagTemplate[CVX_NUM_TYPE];      // template for auto names
   static const char *CszVxSaveAuxName[];                  // additional saved parameters
   static const char *CszVxEquationName[CVXI_NUM_EQTN];    // names for equations CVXI_EQTN
   static const UINT CuVxSavePropertyMask[CVX_NUM_TYPE];   // bit-flag masks for vertex type properties
   static const char *CszVxSaveFcnString[CVXI_PROP_FCNMAX+1]; // plot function save names
   static const char *CszVxFcnNames;        // plottable function names
   static const char *CszVxTypeNames;       // type names for property manager

};
#endif//CVERTEX_H
