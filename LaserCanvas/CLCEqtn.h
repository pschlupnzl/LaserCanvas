/*****************************************************************************
*  CLCEqtn.h (CxEqtn.h v1.0+)                           CÆSIVM LaserCanvas
*  Equation class implementation for LaserCanvas Rev. 4
* $PSchlup 2004-2006 $     $Revision 6 $
*****************************************************************************/
#ifndef CLCEQTN_H
#define CLCEQTN_H
class CEquation;                            // CEquation object for LaserCanvas

#define CLCEQTN_SZVERSION "CEquation v6.3"    // revision string

#include <windows.h>                        // standard Windows header
#include <stdio.h>                          // for sprintf
#include <string.h>                         // string manipulation
#include <math.h>                           // standard Math library

//---Macros-------------------------------------
#ifndef MAX
#define MAX(a,b)  ((a)>(b)?(a):(b))
#endif/*MAX*/
#ifndef MIN
#define MIN(a,b)  ((a)<(b)?(a):(b))
#endif/*MIN*/
#ifndef ABS
# define ABS(x) (((x)<0)? (-(x)) : (x))
#endif/*ABS*/
#ifndef SIGN
# define SIGN(x) ( ((x)==0.00) ? 0 : (((x)>0) ? +1 : -1) )
#endif//SIGN
#ifndef M_PI
# define M_PI 3.1415926536897932
#endif//M_PI

//---Characters---------------------------------
// ILLEGALCHAR: Characters not ever allowed in the string
#define EQ_ILLEGALCHAR "`~@#$%[]{}?\;:"

// VALIDCHAR: Valid first characters of variable name
#define EQ_VALIDCHAR "abcdefghijklmnopqrstuvwxyz_ABCDEFGHIJKLMNOPQRSTUVWXYZ!"

// VALIDSYMB: Valid variable name symbols
#define EQ_VALIDSYMB "abcdefghijklmnopqrstuvwxyz_ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890"

//---Operators----------------------------------
#define OP_NULL             0x0000
#define OP_PSH                   1          // comma (Push) binary op
#define OP_POP                   2          // remove (comma in non-multi situations)
#define OP_SET                   3          // set (variable = expression)

#define OP_BINARYMIN             4          // first "real" binary operator
//                               | - equal
#define OP_OR                    4          // or ||
#define OP_AND                   5          // and &&

#define OP_RELOPMIN              6          // start of relational operators
#define OP_LTE                   6          // less or equal <=
#define OP_GTE                   7          // greater or equal >=
#define OP_LT                    8          // less than <
#define OP_GT                    9          // greater than >
#define OP_NEQ                  10          // not equal !=
#define OP_EQ                   11          // equal ==
#define OP_RELOPMAX             11          // end of relation operators

#define OP_ADD                  12          // ascending in precedence order
#define OP_SUB                  13
#define OP_DIV                  14
#define OP_MUL                  15
#define OP_POW                  16
//                               | - equal
#define OP_BINARYMAX            16          // last real binary operator

#define OP_UNARY                20          // unary ops have OP_UNARY added
#define OP_ABS                   0
#define OP_SQRT                  1
#define OP_EXP                   2
#define OP_LOG                   3
#define OP_LOG10                 4
#define OP_CEIL                  5
#define OP_FLOOR                 6
#define OP_COS                   7
#define OP_SIN                   8
#define OP_TAN                   9
#define OP_ACOS                 10
#define OP_ASIN                 11
#define OP_ATAN                 12
#define OP_COSH                 13
#define OP_SINH                 14
#define OP_TANH                 15
#define OP_SIND                 16
#define OP_COSD                 17
#define OP_TAND                 18
#define OP_ASIND                19
#define OP_ACOSD                20
#define OP_ATAND                21
#define OP_NOT                  22          // not !
#define OP_SIGN                 23
#define NUM_UNARYOP             24          // number of defined unary ops

#define OP_NARG                 50          // n-arg ops have OP_NARG added
#define OP_NARG_MOD              0
#define OP_NARG_REM              1
#define OP_NARG_ATAN2            2
#define OP_NARG_ATAN2D           3
#define OP_NARG_MAX              4
#define OP_NARG_MIN              5
#define OP_NARG_IF               6
#define NUM_NARGOP               7          // number of define n-arg ops

#define OP_BRACKETOFFSET       100          // added for each nested bracket

//---Constants----------------------------------
#define NUM_CONSTANT 1
const char CEquationConstantStr[]= "pi\0";
const double CEquationConstantVal[] = { M_PI };

//---Units--------------------------------------
// Units differ from constants in that they are
// searched for in place of a BINARY_OP
#define NUM_UNIT 0 //**DEBUG**
const char CEquationUnitsStr[]="in\0\"\0ft\0'\0um\0mm\0cm\0m\0km\0";
const double CEquationUnitsVal[] = {0.0254, 0.0254, 0.3048, 0.3048, 1.000e-6,
      0.001, 0.010, 1.000, 1.000e3};

//---Character strings--------------------------
// Each operator / constant is terminated by a single NULL character. The loops
// count up to NUM_UNARYOP and NUM_CONSTANT, so ensure these values are correct

const char CEquationBinaryOpStr[] =
   "+\0-\0*\0/\0^\0||\0&&\0<=\0>=\0<\0>\0!=\0==\0";

const char CEquationUnaryOpStr[] =
   "abs\0sqrt\0exp\0log\0log10\0ceil\0floor\0cos\0sin\0tan\0"
   "acos\0asin\0atan\0cosh\0sinh\0tanh\0sind\0cosd\0tand\0asind\0"
   "acosd\0atand\0!\0sign\0";

const char CEquationNArgOpStr[] =           // n-arg operator strings
   "mod\0rem\0atan2\0atan2d\0max\0min\0if\0";
const int CEquationNArgOpArgc[NUM_NARGOP] = { // n-arg operator argument counts (-ve: min arg count)
   2,2,2,2,-2,-2,3};



#define OP2STR(o) (\
   (o==OP_PSH)           ? "Push" : \
   (o==OP_POP)           ? "Pop" : \
   (o==OP_SET)           ? "Assign" : \
   (o==OP_ADD)           ? "+" : \
   (o==OP_SUB)           ? "-" : \
   (o==OP_MUL)           ? "*" : \
   (o==OP_DIV)           ? "/" : \
   (o==OP_POW)           ? "^" : \
   (o==OP_OR)            ? "Or" : \
   (o==OP_AND)           ? "And" : \
   (o==OP_LTE)           ? "<=" : \
   (o==OP_GTE)           ? ">=" : \
   (o==OP_LT )           ? "<" : \
   (o==OP_GT )           ? ">" : \
   (o==OP_NEQ)           ? "!=" : \
   (o==OP_EQ )           ? "==" : \
   (o-OP_UNARY)==OP_ABS  ? "Abs" : \
   (o-OP_UNARY)==OP_SQRT ? "Sqrt" : \
   (o-OP_UNARY)==OP_EXP  ? "Exp" : \
   (o-OP_UNARY)==OP_LOG10? "Log10" : \
   (o-OP_UNARY)==OP_LOG  ? "Log" : \
   (o-OP_UNARY)==OP_CEIL ? "Ceil" : \
   (o-OP_UNARY)==OP_FLOOR? "Floor" : \
   (o-OP_UNARY)==OP_COS  ? "Cos" : \
   (o-OP_UNARY)==OP_SIN  ? "Sin" : \
   (o-OP_UNARY)==OP_TAN  ? "Tan" : \
   (o-OP_UNARY)==OP_ACOS ? "ACos" : \
   (o-OP_UNARY)==OP_ASIN ? "ASin" : \
   (o-OP_UNARY)==OP_ATAN ? "ATan" : \
   (o-OP_UNARY)==OP_COSH ? "Cosh" : \
   (o-OP_UNARY)==OP_SINH ? "Sinh" : \
   (o-OP_UNARY)==OP_TANH ? "Tanh" : \
   (o-OP_UNARY)==OP_SIND ? "SinD" : \
   (o-OP_UNARY)==OP_COSD ? "CosD" : \
   (o-OP_UNARY)==OP_TAND ? "TanD" : \
   (o-OP_UNARY)==OP_ASIND? "ASinD" : \
   (o-OP_UNARY)==OP_ACOSD? "ACosD" : \
   (o-OP_UNARY)==OP_ATAND? "ATanD" : \
   (o-OP_UNARY)==OP_NOT  ? "Not" : \
   (o-OP_UNARY)==OP_SIGN ? "Sign" : \
   (o-OP_NARG )==OP_NARG_MOD   ? "Mod" : \
   (o-OP_NARG )==OP_NARG_REM   ? "Rem" : \
   (o-OP_NARG )==OP_NARG_ATAN2 ? "Atan2" : \
   (o-OP_NARG )==OP_NARG_ATAN2D? "Atan2D" : \
   (o-OP_NARG )==OP_NARG_MAX   ? "Max" : \
   (o-OP_NARG )==OP_NARG_MIN   ? "Min" : \
   (o-OP_NARG )==OP_NARG_IF    ? "If" : \
   "*unknown*")

//---Errors-------------------------------------
#define EQERR_NONE                    0     // no error

#define EQERR_PARSE_ALLOCFAIL        -1     // could not allocate memory
#define EQERR_PARSE_NOEQUATION       -2     // there's no equation

#define EQERR_PARSE_NUMBEREXPECTED    1     // looking for number, (, -sign, or unary op
#define EQERR_PARSE_UNKNOWNFUNCVAR    2
#define EQERR_PARSE_BRACKETEXPECTED   3     // expecting ( (after unary operator)
#define EQERR_PARSE_BINARYOPEXPECTED  4     // expecting +-*/^ operator
#define EQERR_PARSE_BRACKETSOPEN      5     // not enough closing brackets
#define EQERR_PARSE_UNOPENEDBRACKET   6     // not enough opening brackets
#define EQERR_PARSE_NOADVANCE         7     // current token failed to advance iThisScan
#define EQERR_PARSE_CONTAINSVAR       8     // contains vars when not permitted
#define EQERR_PARSE_NARGBADCOUNT      9     // wrong number of arguments to function
#define EQERR_PARSE_STACKOVERFLOW    10     // stack overflow, too many ops
#define EQERR_PARSE_ASSIGNNOTVAR     11     // assignment needs variable
#define EQERR_PARSE_ILLEGALCHAR      99     // illegal character

#define EQERR_EVAL_UNKNOWNBINARYOP  101     // Unknown binary operator
#define EQERR_EVAL_UNKNOWNUNARYOP   102     // Unknown unary operator
#define EQERR_EVAL_UNKNOWNNARGOP    103     // Unkown n-arg operator
#define EQERR_EVAL_UNKNOWNVALOP     104     // Unknown Valop type
#define EQERR_EVAL_STACKNOTEMPTY    105     // Stack not empty at end of equation
#define EQERR_EVAL_STACKUNDERFLOW   106     // Stack hasn't enough entries
#define EQERR_EVAL_CONTAINSVAR      108     // contains variables than are not supplied
#define EQERR_EVAL_BADTOKEN         109     // not right type of token
#define EQERR_EVAL_ASSIGNNOTALLOWED 110     // not allowed to change variables
#define EQERR_EVAL_NOEQUATION       199     // there is no equation to evaluate

#define EQERR_MATH_DIV_ZERO         201     // division by zero
#define EQERR_MATH_DOMAIN           202     // domain error (acos, etc) (sometimes cplx)
#define EQERR_MATH_SQRT_NEG         203     // square root of negative (--> cplx)
#define EQERR_MATH_LOG_ZERO         204     // log of zero - always undefined
#define EQERR_MATH_LOG_NEG          205     // log of negative (--> cplx)
#define EQERR_MATH_OVERFLOW         206     // exp(large number) overflow

//---Parse status-------------------------------
#define LOOKFOR_NUMBER     0x01  // number, unary op, parentheses, negative, constant
#define LOOKFOR_BINARYOP   0x02  // binary op only
#define LOOKFOR_BRACKET    0x03  // brackets only (after unary op)

//===Operator stack typedef===============================
template<class T> class TEqStack;

//---Valop types--------------------------------
#define VOTYP_UNDEFINED       0x00          // undefined
#define VOTYP_VAL             0x01          // valop is numeric value
#define VOTYP_OP              0x02          // valop is built-in operator or function
#define VOTYP_REF             0x03          // valop is index into variable array
#define VOTYP_UNIT            0x04          // unit: immediate multiply by value
#define VOTYP_NARGC           0x05          // n-argument count whenever bracket is closed

//---Data Structure-------------------
typedef struct tagVALOP {
   unsigned char   uTyp;                    // type (op, value, variable)
   union {
      double       dVal;                    // value for constants
      unsigned int uOp;                     // operator code
      int          iRef;                    // index into variable array
      int          iUnit;                   // index into unit array
      int          iArgc;                   // number of pushed arguments
   };
   int             iPos;                    // position in source string
} VALOP, *PVALOP;

/*********************************************************
* CEquation declaration
*********************************************************/
class CEquation {
private:
   char  *pszSrcEquation;                   // string of equation source
   int    iEqnLength;                       // number of legitimate ops in valop stack
   VALOP *pvoEquation;                      // array of ops
   int    iError;                           // error that occured
   int    iErrorLocation;                   // location of error (pointer into SrcEquation)

   BOOL   SetSrcEquation(const char *sz);   // allocate memory and set source equation string
   void   FreeSrcEquation(void);            // free previously allocated buffer
   BOOL   AllocEquation(int iNumOps);       // allocate memory for the VALOP stack
   void   FreeEquation(void);               // free previously allocated memory
   int   _ProcessOps(TEqStack<VALOP> *pvosParsEqn, TEqStack<int> *pisOps, TEqStack<int> *pisPos, int iThisOp, int iBrktOff);
public:
   CEquation(void);                         // constructor and initialization
   ~CEquation();                            // destructor
   int    ParseEquation(const char *szEqn, const char *pszVars); // supply a new string and parse it
   int    DoEquation(double dVar[], double *dAns, BOOL tfAllowAssign=FALSE); // calculate equation - returns err code
   double Answer(double dVar[], BOOL tfAllowAssign=FALSE);      // overloaded equation solver, no error info
   void   GetEquationString(char *szBuf, size_t len); // get source string
   int    GetLastError(char *szBuffer, size_t len); // position and description of last error
   void   LastErrorMessage(char *szBuffer, size_t len, const char *szSource); // formatted message string
   //void   LastErrorMessageBox(HWND hWnd, const char *szTitle); // MessageBox describing last error

   BOOL   ParseConstantEquation(const char *szEqtn, double *pdAns); // parse an equation without variables
   int    ParseDoubleEquation(double dVal, const char *pszFmt=NULL); // create an equation for a value
   BOOL   ContainsUnits(void);              // returns TRUE if units within equation
   int    ContainsVariables(void);          // returns EQERR_CONTAINS_VARIABLE if one or more variables are used
   int    ContainsVariable(int iVar);       // returns EQERR_CONTAINS_VARIABLE if given variable is use
   VALOP *GetEquationStack(void) { return(pvoEquation); }; // returns pointer to equation stack (debug only)
   int    GetEquationLength(void) { return(iEqnLength); }; // returns equation length (debug only)

   const char* _GetSrcEqStr(void) { return(pszSrcEquation); }; // returns pointer to internal source equation (debug only)
};

/*********************************************************
*  Stack Template
*********************************************************/
#define EQSTACK_CHUNK                16     // chunks of elements to allocate at a time
template<class T> class TEqStack {
private:
   T   *tStck;                              // array of values
   T    tNul;                               // NULL returned on errors
   int  iLen;                               // number of entries allocated
   int  iTop;                               // pointer to top of stack

   int  Alloc(int iNewLen);                 // allocate more memory
public:
   TEqStack(void);                          // initialize
   ~TEqStack();                             // exit
   int  Push(T t);                          // push a value - return top
   T    Pop(void);                          // pop a value
   T    Peek(void);                         // show top value without popping it
   int  Top(void);                          // get top of stack
   T    PeekBack(int iOffs=-1);             // peek further back (iOffs -ve)
   int  InsertBack(T t, int iOffs);         // insert a value further back (iOffs -ve)
   void Display(const char *pszHead, const char *pszFmt);
};

//---Management---------------------------------
template <class T>
TEqStack<T>::TEqStack(void) {
   tStck = (T*) NULL; iLen = 0;             // nothing allocated yet
   memset(&tNul, 0x00, sizeof(T));          // prepare NULL for errors
   iTop = 0;                                // no elements in stack
   Alloc(EQSTACK_CHUNK);                    // allocate some space
}

template <class T>
TEqStack<T>::~TEqStack() {
   if(tStck) delete(tStck); tStck = (T*) NULL; iLen = 0; // free existing
}

template <class T>
int TEqStack<T>::Alloc(int iNewLen) {
   T *ptStckNew;                            // new stack
   if((iNewLen==iLen) || (iNewLen<iTop)) return(iLen);   // ignore non-changes and too-smalls
   ptStckNew = (T*) malloc(iNewLen * sizeof(T));         // allocate new stack
   if(ptStckNew == (T*) NULL) return(iLen);              // return existing length on alloc failure
   memset(ptStckNew, 0x00, iNewLen * sizeof(T));         // clear stack (not really necessary)
   memcpy(ptStckNew, tStck, iTop*sizeof(T));             // copy as needed
   if(tStck) delete(tStck); tStck = (T*) NULL; iLen = 0; // free existing
   tStck = ptStckNew; iLen = iNewLen;       // point to new stack
   return(iNewLen);
}

//---Functions----------------------------------
template <class T>
int TEqStack<T>::Push(T t) {
   if(iTop >= iLen) Alloc(iLen + EQSTACK_CHUNK); // allocate some more
   if(iTop >= iLen) return(-1);             // didn't work, return with error
   tStck[iTop] = t;                         // save the value and..
   iTop++;                                  //..increment the counter
   return(iTop);
}

template <class T>
int TEqStack<T>::InsertBack(T t, int iOffs) {
   if(iTop >= iLen) Alloc(iLen + EQSTACK_CHUNK); // allocate some more
   if(iTop >= iLen) return(-1);             // didn't work, return with error
   for(int k=iTop; k>iTop+iOffs; k--) tStck[k]=tStck[k-1];
   tStck[iTop+iOffs] = t;
   iTop++;
   return(iTop);
}

template <class T>
T TEqStack<T>::Pop(void) {
   if(iTop <= 0) return(tNul);              //..in case the stack is empty
   iTop--;                                  // decrement counter and..
   return(tStck[iTop]);                     //..return the value
}

template <class T>
T TEqStack<T>::Peek(void) {
   if(iTop <= 0) return(tNul);
   return(tStck[iTop-1]);                   // return value at top of stack
}

template <class T>
int TEqStack<T>::Top(void) {
   return(iTop);                            // stack valid provided Top() > 0
}

template <class T>
T TEqStack<T>::PeekBack(int iOffs) {     // peek further back (iOffs is negative)
   if((iTop+iOffs) < 0) return(tNul);
   return(tStck[iTop+iOffs]);
}

template <class T>
void TEqStack<T>::Display(const char *pszHead, const char *pszFmt) {
   printf("%s", pszHead);
   for(int k=0; k<iTop; k++) {
      printf("  %d:", k);
      printf(pszFmt, tStck[k]);
   }
   printf("\n");
}

#endif/*CLCEQTN_H*/


