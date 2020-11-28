/*********************************************************
* CABCD.cpp
* Class that performs the ABCD calculations and so on. In
* the current  edition, performs sagittal  and tangential
* calculations concurrently, so  it's like a dual 2x2 ma-
* trix class.
*
* The solving is actually performed by CSystem.
*
* $PSchlup 2000-2006 $     $Revision 0 $
*********************************************************/
#include "CABCD.h"

/*########################################################
 ## CMatrix2x2 Class                                   ##
########################################################*/
/**********************************************************
* Constructors
**********************************************************/
CMatrix2x2::CMatrix2x2(void) {
   Eye();                                   // reset to identity
}
CMatrix2x2::CMatrix2x2(double _A, double _B, double _C, double _D) {
   Set(_A, _B, _C, _D);
}

/**********************************************************
* Destructor
**********************************************************/
CMatrix2x2::~CMatrix2x2() {
   ;//NOP
}

/*********************************************************
* Eye
* Set the matrix elements to the identity matrix
*********************************************************/
void CMatrix2x2::Eye(void) {
   Set(1.00, 0.00, 0.00, 1.00);
}

/*********************************************************
* Set - Set the values
*********************************************************/
void CMatrix2x2::Set(double _A, double _B, double _C, double _D) {
   A = _A; B = _B;
   C = _C; D = _D;
}

/*********************************************************
* Det - Matrix determinant
*********************************************************/
double CMatrix2x2::Det(void) {
   return(A*D - B*C);
}

/*********************************************************
* Trace - Matrix trace
*********************************************************/
double CMatrix2x2::Trace(void) {
   return(A + D);
}

/*********************************************************
* Multiplication
* This is implemented as pre-multiplication. The functio-
* nal version  applies the  multiplication and  keeps the
* result.
*********************************************************/

//---Functional pre-multiplication--------------
void CMatrix2x2::PreMult(const CMatrix2x2* pMx2) {
   Set(                                     // perform pre-multiplication
      pMx2->A * A + pMx2->B * C,
      pMx2->A * B + pMx2->B * D,
      pMx2->C * A + pMx2->D * C,
      pMx2->C * B + pMx2->D * D);
}







/*#########################################################
 ## Class CRecQ                                         ##
#########################################################*/
/**********************************************************
* Constructor
**********************************************************/
CRecQ::CRecQ(void) {
   Set(0.00, 0.00, 1.00, 1.00);
}

/**********************************************************
* Destructor
**********************************************************/
CRecQ::~CRecQ() {
}

/*********************************************************
* Comparison operators
* Compares the members to each other. Only If all three
* are equal are the CRecQ are considered equal.
*********************************************************/
//===Equality=============================================
int CRecQ::operator==(const CRecQ &Q) {
   return( ((_R==Q._R) && (_v==Q._v) && (_L==Q._L) && (_M==Q._M)) );
}
//===Inequality===========================================
int CRecQ::operator!=(const CRecQ &Q) {
   return( ((_R!=Q._R) || (_v!=Q._v) || (_L!=Q._L) && (_M!=Q._M)) );
}



/*********************************************************
* Set formulae
* The q  parameter is  defined in terms of the  wavefront
* curvature R, the mode size w, and the wavelength lambda
* as
*     1     1        -lambda
*    --- = ---  + j ---------
*     q     R         pi w^2
* which, using the CRecQ member definitions
*           1             pi w^2
*     _R = --- ,    _v = --------  (no sign!)
*           R             lambda
* becomes
*                      1
*     Q  =  _R  -  j ----
*                     _v
* Note the sign: _v = - 1/Im{Q} but is itself "positive."
*
* Relative to the waist w0 and its distance z, the curva-
* ture  and mode size at an arbitrary position are  given
* by
*          (      /  pi w0^2  \ ^2 )
* R(z) = z ( 1 + | ----------- |   )
*          (      \  lambda z /    )
*
*               (      / lambda z \ ^2 )
* w(z)^2 = w0^2 ( 1 + | ---------- |   )
*               (      \  pi w0^2 /    )
* These are used when setting the CRecQ members for a gi-
* ven waist and distance.
*
* Embedded Gaussian and M^2:
* - For a given measured spot size, divide by the quality
*   factor M (NOT ^2!) to get the spot size of the embed-
*   ded Gaussian
* - Propagate the embedded Gaussian as usual
* - To get the predicted measured size, multiply the size
*   by factor M again.
* - The waist LOCATION and Rayleigh length are unchanged.
* Since the CRecQ class deals with a scaled waist squared
* _v anyway, we keep the M^2. The M^2 parameter is forced
* to be >= 1.00.
*
* Notes
* - When  solving for a stable  resonator mode, we  don't
*   need to perform the first step  because we're already
*   solving for the embedded Gaussian.
* - For propagation systems, we  divide the input mode or
*   waist size by M before establishing the Q parameter.
* - The MEASURED spot size as a  function of the MEASURED
*   waist follows the same  formula as above, except that
*   the WAVELENGTH is multiplied by M^2.
*
*********************************************************/
void CRecQ::Set(double R, double v, double L, double M2) {
   if(L==0.00) L = 1.00;                    // prevent #DIV/0! errors
   _R  = R;
   _v  = v;
   _L  = L;
   if(M2 < 1.00) M2 = 1.00; _M = SQRT(M2);
}
//===Complex Parts========================================
void CRecQ::SetRealImag(double dReal, double dImag, double dWLen, double M2) {
   Set(dReal,
      (dImag==0.00) ? 0.00 : -1.00 / dImag,
      dWLen,
      M2);
}
//===Curvature and Size===================================
void CRecQ::SetRW(double dRz, double dWz, double dWLen, double M2) {
   if(M2 < 1.00) M2 = 1.00;                 // limit M^2 value
   if(dWLen == 0.00) dWLen = 1.00;          // prevent #DIV/0! errors
   Set(
      (dRz==0.00) ? 0.00 : 1.00 / dRz,
      (dWz==0.00) ? 0.00 : M_PI  * SQR(dWz) / (M2 * dWLen),
      dWLen,
      M2);
}
//===Waist and Dist=======================================
void CRecQ::SetW0z0(double dW0, double dz0, double dWLen, double M2) {
   if(M2 < 1.00) M2 = 1.00;                 // limit M^2 value
   if(dWLen == 0.00) dWLen = 1.00;          // prevent #DIV/0! errors
   SetRW(                                   // use indirect -- it's easier
      (dz0==0.00) ? 0.00 : dz0 * (1.00 + SQR((M_PI*SQR(dW0)) / (M2*dWLen*dz0))),
      (dW0==0.00) ? 0.00 : SQRT( SQR(dW0) * (1.00 + SQR((M2*dWLen*dz0) / (M_PI * SQR(dW0)))) ),
      dWLen,
      M2);
}



/*********************************************************
* Get Functions
* The propagation of the complex parameter q after a dis-
* tance z is given by
*    q' = q + z
* which can be expanded (algebraically) to directly yield
* the _R and _v values at an  arbitrary distance. Equiva-
* lently, we could  propagate Q through an ABCD matrix of
* length z [1 z;0 1].
* We use the alternative method here, and calculate first
* the waist w0 and distance z to it, then use the SQRT(1+
* SQR(..))  formulae listed under Set(). This  seems con-
* ceptually easier, and if there are a lot of repeat cal-
* culations, we could store w0 and z0 separately.
*             [      /  pi w^2  \ ^2 ] ^-1
*  w0^2 = w^2 [ 1 + | ---------- |   ]
*             [      \ lambda R /    ]
*  (Special case _R = 0.00 --> R = inf, so w0 = w.)
*
*        [      / lambda R \ ^2 ] ^-1
*  z = R [ 1 + | ---------- |   ]
*        [      \  pi w^2  /    ]
*  (Special case _R = 0.00 --> z = 0.)
*
*********************************************************/
//===Curvature at plane===================================
double CRecQ::R(void) {                     // a bit confusing: R = 1/_R
   return( (_R==0.00) ? 0.00 : 1.00 / _R );
}
//===Mode squared at plane================================
double CRecQ::w2(void) {                    // _v = pi w^2 / lambda
   return( _v * _L / M_PI );
}
double CRecQ::w(void) {
   return(SQRT(w2()));
}
double CRecQ::W(void) {
   return(_M * w());
}
//===Waist squared========================================
double CRecQ::w02(void) {                  // infinite curvature --> at waist
   return(  w2()/( 1.00 + SQR(_v*_R) )  ); // recall: _R = 1/R
}
//===Waist================================================
double CRecQ::w0(void) {
   return(SQRT(w02()));
}
double CRecQ::W0(void) {
   return(_M * w0());
}
//===Waist position=======================================
///TODO: Is this really supposed to be negative?! Check the calculations
double CRecQ::z0(void) {
   if((_R==0.00)||(_v==0.00)) {             // infinite curvature..
      return( 0.00 );                       //..means we'er at the waist
   } else {
      return(
      -1.00 / (_R * (1.00 + 1.00/SQR(_v*_R))) );
   }
}
//===Curvature============================================
double CRecQ::R(double z) {
///TODO: CHECK calculation here
   if(z==0.00) return(0.00);                // at waist, infinite curvature
   if(_L==0.00) _L = 1.00;                  // prevent #DIV/0! errors, wavelength can't be zero
   return(
      (1.00/z) * SQR(M_PI*w02() / _L) + z
   );
}
//===Mode Size============================================
double CRecQ::w(double z) {
///TODO: CHECK calculation here
   if(z==0.00) return(w0());                // don't calculate if we know we're at the waist
   if(w02()==0.00) return(0.00);            // prevent #DIV/0! errors
   return( SQRT(
      w02() * (1.00 + SQR((z*_L) / (M_PI*w02())))
   ));
}
double CRecQ::W(double z) {
   return(_M * w(z));
}
//===Rayleigh Range=======================================
// The Rayleigh range is the same for the beam as for the
// embedded mode, so here we return just the embedded va-
// lue.
double CRecQ::zR(void) {
   return(
      (M_PI * w02()) / (_L + ((_L==0.00) ? 1.00 : 0.00))
   );
}

/*********************************************************
* ApplyAbcd
* Propagates the current Q through the given ABCD matrix.
* For propagating through an ABCD note that (omitting the
* underscores or R=_R and v=_v)
*  1           1
* ---  =  R - --- i
*  q           v
*
* Now
*  1       C + D(1/q)
* ---  =  ------------
*  q'      A + B(1/q)
*          C + D(R - i/v)
*      =  ----------------
*          A + B(R - i/v)
*          (C + DR) - iD/v    (A + BR) + iB/v
*      =  -----------------  -----------------
*          (A + BR) - iB/v    (A + BR) + iB/v
*          [(A+BR)(C+DR) + DB/v^2] + i[-(A+BR)D/v + (C+DR)B/v]
*      =  -----------------------------------------------------
*                        (A+BR)^2 + (B/v)^2
*          [(A+BR)(C+DR)v^2 + DB] + iv[-(A+BR)D + (C+DR)B]
*      =  -------------------------------------------------
*                        (A+BR)^2 v^2 + B^2
*
* Dividing into real and imaginary parts, we have
*
*           (A+BR)(C+DR)v^2 + DB
*    R' =  ----------------------                (**)
*            (A+BR)^2 v^2 + B^2
*
*    1     v[-(A+BR)D + (C+DR)B]
* - --- = -----------------------
*    v'      (A+BR)^2 v^2 + B^2
*
* or
*              (A+BR)^2 v^2 + B^2
*    v' = - -----------------------   (sign!)
*            v[-(A+BR)D + (C+DR)B]
*          (A+BR)^2 v + B^2/v
*       = --------------------                   (**)
*           (A+BR)D - (C+DR)B
*
* By storing the (A + BR)  and (C + DR) terms, we can re-
* place the value for _R before starting on the _v calcu-
* lation.
*********************************************************/
void CRecQ::ApplyAbcd(const CMatrix2x2 *pMx) {
   double ApBR, CpDR;                       // store intermediate values
   double divisor;

   //---Repeated terms----------------
   ApBR = pMx->A + pMx->B * _R;             // terms
   CpDR = pMx->C + pMx->D * _R;

   //---Real part---------------------
   divisor = SQR(ApBR) * SQR(_v) + SQR(pMx->B);
   if(divisor==0.00) divisor = 1.00;        // prevent #DIV/0! errors
   _R = (ApBR*CpDR*SQR(_v) +  pMx->D*pMx->B) / divisor;

   //---Imag part---------------------
   divisor = ApBR * pMx->D - CpDR * pMx->B;
   if(divisor==0.00) divisor = 1.00;        // prevent #DIV/0! errors (the 1/divisor one)
   if(_v != 0.00) {                         // prevent #DIV/0! error (the one for 1/v)
      _v = (SQR(ApBR)*_v + SQR(pMx->B)/_v) / divisor;
   }
}


