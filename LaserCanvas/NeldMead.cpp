/*********************************************************
*  Nelder-Mead Simplex Search
* ----------------------------
* Minimises *func by the Nelder-Mead search method, star-
* ting from an initial guess. Returns the optimised para-
* meters, minimised function value, and number of itera-
* tions. The algorithm is  inspired by that in "Numerical
* Recipes in C," Cambridge University Press, 1998.
*
* Arguments:
*   *func        Pointer  to minimization  function.  The
*                function must have the declaration
*                   double Fcn(double[], void*);
*                taking a double array (the parameters to
*                minimise and an application-defined void
*                pointer.
*    xInitial[]  Array of initial parameters.
*    nDim        Number of dimensions.
*   *nMaxIter    Maximum number of loop iterations.
*    fXTol       Solving parameter tolerance (not used)
*    fYTol       Solving function value tolerance
*    aux         Application-defined  pointer  passed  to
*                the minimization function.
*
* Returns:
*    Ret value   Minimised function value
*    xInitial[]  The array contents are replaced with the
*                minimising parameters
*   *nMaxIter    Contains the  number of  "unused" itera-
*                tions. If this value is 0, the algorithm
*                exceeded the  maximum allowed  number of
*                iterations. If  it is > 0, the algorithm
*                terminated through either fXTol or fYTol
*                criteria.
*
* $PSchlup 2001-2006 $     $Revision 2 $
* Revision History
*   2  2006oct    Modified for double precision
*   1  2001apr    Initial development (St Andrews)
*********************************************************/

//===Preprocessor=========================================
#include <malloc.h>				// for malloc();
#define ALPHA (-1)				// reflection coefficient
#define BETA 2						// expansion coefficient
#define GAMMA 0.5					// contraction coefficient

//===Function Prototype===================================
//double SimplexMinimise(double(*)(double [],void*),double[],int,int*,double,double,void*);

/*********************************************************
* SimplexMinimize
* Minimize function *func using the Nelder--Mead Simplex
* search method.
*********************************************************/
double SimplexMinimise(
   double (*func)(double [],void*),         // function to be minimised
   double   xInitial[],                     // initial search parameters
   int      nDim,                           // number of dimensions
   int     *nMaxIter,                       // maximum iterations.  On return, stores "spare" iterations
   double   fXTol,                          // parameter termination criterion
   double   fYTol,                          // function termination criterion
   void    *aux                             // auxiliary data for *func
) {
   //===Variables=========================================
   int      i, j;                           // loop counters
   int      nIter;                          // number of iterations
   int      iWorst, iNextWorst, iBest;      // "sorted" indices
   double **x;                              // simpex vertices' coordinates
   double  *y;                              // function value at vertices
   double  *xC;                             // centroid coordinates of simplex
   double  *xTry;                           // try coordinates
   double   yTry;                           // try function value
   double   xMin, xMax;                     // used during parameter tolerance check

   //===Allocate Memory===================================
   // fVertex: Number of ROWS: nDim+1.  Number of COLUMNS: nDim.
   x     = (double**) malloc( ((nDim+1)        * sizeof(double*)) );
   x[0]  = (double*)  malloc( ((nDim+1) * nDim * sizeof(double)) );
   for(i = 1; i < nDim+1; i++) x[i] = x[i-1] + nDim;

   y    = (double*) malloc((nDim + 1) * sizeof(double));
   xC   = (double*) malloc( nDim      * sizeof(double));
   xTry = (double*) malloc( nDim      * sizeof(double));

   //===Create Simplex====================================
   for(i = 0; i < nDim + 1; i++) {
      for(j = 0; j < nDim; j++) {
         x[i][j] = xInitial[j];
         // Move vertex along coordinate axis to form the
         // simplex. The  last vertex (i = nDim+1) is not
         // shifted, since i != j. This simplex is parti-
         // cularly unsophisticated, possibly wasteful.
         if(i == j) x[i][j] = x[i][j] + 1;
      }
      y[i] = func(x[i], aux);               // calculate function at vertex
   }

   //===Minimization Loop=================================
   for(nIter = 0; nIter < *nMaxIter; nIter++) {
      //---Order smallest to largest------------
      iWorst = (y[0] > y[1]) ? 0 : 1;       // assign default values
      iNextWorst = 1 - iWorst;
      iBest = 0;
      for(i = 0; i < nDim+1; i++) {
         if(y[i] < y[iBest]) iBest = i;     // iBest: smallest function value
         if(y[i] > y[iWorst]) {             // iWorst: largest function value
            iNextWorst = iWorst;            // iNextWorst: second-largest function value
            iWorst = i;
         } else if(y[i] > y[iNextWorst] && i != iWorst) {
            iNextWorst = i;
         }
      }
      //---Check Tolerances---------------------
      for(j = 0; j < nDim; j++) {           // check tolerance in each dimension
         xMin = xMax = x[0][j];
         for(i = 1; i < nDim+1; i++) {      // get min and max over all points
            if(x[i][j] < xMin) xMin = x[i][j];
            if(x[i][j] > xMax) xMax = x[i][j];
         }
         if((xMax - xMin) > fXTol) {        // at the first over tolerance,..
            j = -1;                         //..set a flag and..
            break;                          //..exit the checking loop
         }
      }
      // Check tolerances.  j=-1 ==> fXtol exceeded.  The
      // rest is written as a dual-end check to avoid ne-
      // cessity of including <math.h>.
      if((j>-1) || (((y[iWorst] - y[iBest]) < fYTol) && ((y[iBest] - y[iWorst]) > (-fYTol))) ) {
         // if we're done, swap best point into index 0
         for(j=0; j<nDim; j++) {
            xTry[0] = x[0][j]; x[0][j] = x[iBest][j]; x[iBest][j] = xTry[0];
         }
         yTry = y[0]; y[0] = y[iBest]; y[iBest] = yTry;
         break;
      }

      //---Get face centroid--------------------
      for(j=0; j<nDim; j++) {               // find centroid in each dimension
         xC[j] = 0.0;
         for(i=0; i<nDim+1; i++) {          // average over all vertices
            if(i!=iWorst) xC[j] += x[i][j]; // exclude worst point
         }
         xC[j] /= nDim;
      }

      //---Try a reflection-----------
      for(j=0; j < nDim; j++) xTry[j] = xC[j] + ALPHA * (x[iWorst][j] - xC[j]);
      yTry = func(xTry, aux);
      if(yTry < y[iNextWorst]) {            // better than what we alread had -- replace worst
         for(j=0; j < nDim; j++) x[iWorst][j] = xTry[j];
         y[iWorst] = yTry;
         if(yTry < y[iBest]) {              // if also better than best, try expansion
            //--Try expansion---------
            for(j=0; j<nDim; j++) xTry[j] = xC[j] + BETA * (x[iWorst][j] - xC[j]);
            yTry = func(xTry, aux);
            if(yTry < y[iBest]) {           // better than best again, keep it
               for(j=0; j<nDim; j++) x[iWorst][j] = xTry[j];
            }
         }
      } else {                              // keep reflection if it's better than the worst we had
         if(yTry < y[iWorst]) {
            for(j=0; j<nDim; j++) x[iWorst][j] = xTry[j];
            y[iWorst] = yTry;
         }
         //---Try contraction---------
         for(j=0; j<nDim; j++) xTry[j] = xC[j] + GAMMA * (x[iWorst][j] - xC[j]);
         yTry = func(xTry, aux);
         if(yTry < y[iWorst]) {             // better than what we had, so keep it
            for(j=0; j<nDim; j++) x[iWorst][j] = xTry[j];
            y[iWorst] = yTry;
         } else {
            //---Failing all those, shrink---
            for(i=0; i<nDim+1; i++) {
               if(i != iBest) {
                  for(j=0; j<nDim; j++) x[i][j] = (x[i][j] + x[iBest][j]) / 2.0;
                  y[i] = func(x[i], aux);
               }
            }
         }
      }
   }
//   if(nIter >= MAX_ITERATIONS) fprintf(stderr, "Warning: Maximum number of iterations exceeded.\n");

   //===Prepare for Output================================
   *nMaxIter = *nMaxIter - nIter;           // return number of "spare" iterations
   for(j=0;j<nDim;j++) xInitial[j] = x[0][j]; // return best parameters
   yTry = y[0];                             // return best function value

   //---Free Memory-----------------------------
   free(*x);
   free(x);
   free(y);
   free(xC);
   free(xTry);

   return(yTry);                           // return best function value
}

