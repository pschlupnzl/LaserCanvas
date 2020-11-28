/*********************************************************
* CAbdc.h
* I still haven't quite decided whether to use a CMatrix
* class a MATRIX2x2 typedef, or both.
* The irony  here is that the CAbcd class, which  used to
* incorporate a pair of each of ABCD CMatrix2x2 and CRecQ
* for each plane, is no longer in  use. It seemed clearer
* to calculate  everything explicitly in  CSystem::Solve-
* SystemABCD();  thus, the classes  here are rather  more
* general.
* $PSchlup 2000-2006 $     $Revision 4 $
* Revision History
*   4  2006nov24  Added M parameter
*********************************************************/
#ifndef CABCD_H                             // prevent multiple includes
#define CABCD_H

//===Classes Defined======================================
class CMatrix2x2;                           // 2x2 matrix
class CRecQ;                                // reciprocal q

//===Includes=============================================
#include <stdio.h>                          // standard header
#include <math.h>                           // mathematical functions
#ifndef M_PI
#define M_PI 3.141592653589793
#endif

//===Typedefs=============================================
#ifndef SQR
#define SQR(x) ((x)*(x))
#endif//SQR
#ifndef SQRT
#define SQRT(x) (sqrt(((x)>0.00) ? (x) : (-(x))))
#endif//SQRT

//===Constants============================================
#define SAG                      0          // (index) sagittal index
#define TAN                      1          // (index) tangential index
#define SAGTAN                   2          // (count) for arrays

/**********************************************************
* CMatrix2x2 Class
**********************************************************/
class CMatrix2x2 {
public: ///TODO: Make private
   double A, B, C, D;                       // members
public:
   CMatrix2x2(void);                        // constructor
   CMatrix2x2(double _A, double _B, double _C, double _D); // constructor with initialization
   ~CMatrix2x2();                           // destructor
   void   Eye(void);                        // reset to identity
   void   Set(double _A, double _B, double _C, double _D); // set the values
   double Det(void);                        // returns the determinant
   double Trace(void);                      // returns the matrix trace

   void   PreMult(const CMatrix2x2 *pMx2);
};


/**********************************************************
* CRecQ Class
* It seems easiest, after all, to store the real and ima-
* ginary parts of the Q (=1/q) parameter, since these are
* needed for propagating through an arbitrary ABCD matrix
* and the physical parameters are relatively easily deri-
* ved. The change to the  previous version, then, is only
* that we now store the wavelength also, to form a closed
* class that requires no external information to render a
* given parameter. The wavelength _L is not allowed to be
* zero.
* Units: There are NO scale factors in the equations used
* so the equations are valid either for
*  - All variables in um (or any other unit)
*  - w0 [um]; lambda [nm]; z, R [mm] (the to be used)
**********************************************************/
class CRecQ {
private:
   double _R;                               // 1 / R
   double _v;                               // pi w^2 / lambda
   double _L;                               // lambda
   double _M;                               // M (not squared) parameter
public:

   CRecQ(void);                             // constructor
   ~CRecQ();                                // destructor

   void   SetWLen(double dWLen)        { _L  = (dWLen==0.00) ? 1.00 : dWLen; }; // set the wavelength only
   void   SetMSq(double dMSq)          { if(dMSq < 1.00) dMSq = 1.00; _M = SQRT(dMSq); }; // set M^2 only
   void   ApplyAbcd(const CMatrix2x2 *pMx); // applies ABCD matrix

   int    operator==(const CRecQ &Q);       // equality comparison operator
   int    operator!=(const CRecQ &Q);       // inequality comparison operator

   //---Set functions-----------------
   void   Set(double R, double v, double L, double M2); // set members directly
   void   SetRealImag(double dReal, double dImag, double dWLen, double M2); // set by complex parts
   void   SetRW(double dRz, double dwz, double dWLen, double M2); // set by curvature and mode size
   void   SetW0z0(double dw0, double dz0, double dWLen, double M2); // set by waist and distance to waist

private:
   //---Embedded parameters-----------
   double w2(void);                         // w^2 at plane
   double w(void);                          // mode w at plane
   double w02(void);                        // waist squared
   double w0(void);                         // waist size
   double w(double z);                      // mode size at given position

public:
   //---Beam parameters---------------
   double R(void);                          // R at plane
   double W(void);                          // mode W at plane
   double W0(void);                         // waist
   double R(double z);                      // curvature at given position
   double W(double z);                      // mode size at given position
   double z0(void);                         // relative distane to the waist
   double zR(void);                         // Rayleigh length

   //---Access------------------------
   double Get_R(void)                 { return(_R); };
   double Get_v(void)                 { return(_v); };
   double WLen(void)                  { return(_L); };
   double M2(void)                    { return(SQR(_M)); };
};

#endif//CABCD_H
