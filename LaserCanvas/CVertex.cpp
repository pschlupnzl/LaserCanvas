/*********************************************************
* CVertex
* Vertex class.
* Hierarchy
* CApplication
*  + CSystem
*     + CVertex
*********************************************************/
#include "CVertex.h"                        // class header

//===Names================================================
const char* CVertex::CszVxTypeName[CVX_NUM_TYPE] = { // names for vertex types (must be unique in substrings!)
   "Mirror"         ,                       // plane or curved mirror
   "ThinLens"       ,                       // thin lens
   "Flat"           ,                       // flat mirror: between prisms
   "ThermalLens"    ,                       // thermal lens within a material
   "Screen"         ,                       // observation screen
   "Source"         ,                       // user input beam source
   "CrystalInput"   ,                       // normal incidence input face
   "CrystalOutput"  ,                       // normal incidence output face
   "BrewsterInput"  ,                       // brewster block input
   "BrewsterOutput" ,                       // brewster block output
   "PrismA"         ,                       // first prism apex
   "PrismB"         ,                       // second prism apex
   "PlateInput"     ,                       // parallel block input face
   "PlateOutput"    ,                       // parallel block output face
   "OutputCoupler"                          // output coupler (start of transmitted system)
};

const char* CVertex::CszVxTagTemplate[CVX_NUM_TYPE] = { // check no name conflict!
   "M"              ,   // mirror
   "L"              ,   // lens
   "FM"             ,   // flat mirror
   "TL"             ,   // thermal lens
   "I"              ,   // screen
   "Src"            ,   // source
   "CI"             ,   // crystal in
   "CO"             ,   // crystal out
   "BI"             ,   // brewster in
   "BO"             ,   // brewster out
   "Pa"             ,   // first prism
   "Pb"             ,   // second prism
   "PI"             ,   // plate in
   "PO"             ,   // plate out
   "OC"                 // output coulper
};

const char* CVertex::CszVxSaveAuxName[CVXE_NUM_SAVE] = { // additional saved parameters
   "Selected"      ,                        // selected
   "Spawn"         ,                        // spawned system
   "LinkedTo"      ,                        // linked vertex
   "LockedAngle"   ,                        // angle is locked
   "Flipped"       ,                        // optic is flipped
   "Astigmatic"                             // ROC/FL is astigmatic
};

const char* CVertex::CszVxSaveFcnString[CVXI_PROP_FCNMAX+1] = { // plot function save names
   "Mode"       ,                           // mode size
   "Curvature"  ,                           // curvature
   "Waist"      ,                           // waist
   "Distance"   ,                           // distance to waist
   "Rayleigh"   ,                           // Rayleigh range
   "Astigmatism"                            // astigmatism
};

// The order of these names must match the CVXI indexing
const char* CVertex::CszVxEquationName[CVXI_NUM_EQTN] = { // names for equations CVXI_EQTN
   "DistanceToNext"  ,                      // distance to next optic
   "RefractiveIndex" ,                      // refractive index
   "FaceAngle"       ,                      // interface / incidence angle
   "ROC"             ,                      // radius of curvature
   "ROC_tan"         ,                      // tangential curvature
   "FL"              ,                      // focal length
   "FL_tan"          ,                      // tangential focal length
   "Thickness"                              // thickness / insertion
};

const UINT CVertex::CuVxSavePropertyMask[CVX_NUM_TYPE] = { // bit-flag masks for vertex type properties
/*Mirror        */ (CVXB_EQTN_DIST2NEXT | CVXB_EQTN_FACEANGLE | CVXB_EQTN_ROC | CVXB_EQTN_ROCTAN),
/*ThinLens      */ (CVXB_EQTN_DIST2NEXT | CVXB_EQTN_FL | CVXB_EQTN_FLTAN),
/*Flat          */ (CVXB_EQTN_DIST2NEXT | CVXB_EQTN_FACEANGLE),
/*ThermalLens   */ (CVXB_EQTN_FL | CVXB_EQTN_FLTAN),
/*Screen        */ (CVXB_EQTN_DIST2NEXT),
/*Source        */ (CVXB_EQTN_DIST2NEXT),
/*CrystalInput  */ (CVXB_EQTN_THICKNESS | CVXB_EQTN_ROC | CVXB_EQTN_ROCTAN | CVXB_EQTN_FACEANGLE | CVXB_EQTN_N),
/*CrystalOutput */ (CVXB_EQTN_DIST2NEXT | CVXB_EQTN_ROC | CVXB_EQTN_ROCTAN),
/*BrewsterInput */ (CVXB_EQTN_THICKNESS | CVXB_EQTN_ROC | CVXB_EQTN_ROCTAN | CVXB_EQTN_N),
/*BrewsterOutput*/ (CVXB_EQTN_DIST2NEXT | CVXB_EQTN_ROC | CVXB_EQTN_ROCTAN),
/*PrismFace1    */ (CVXB_EQTN_DIST2NEXT | CVXB_EQTN_N),
/*PrismFace2    */ (CVXB_EQTN_DIST2NEXT),
/*PlateInput    */ (CVXB_EQTN_THICKNESS | CVXB_EQTN_ROC | CVXB_EQTN_ROCTAN | CVXB_EQTN_FACEANGLE | CVXB_EQTN_N),
/*PlateOutput   */ (CVXB_EQTN_DIST2NEXT | CVXB_EQTN_ROC | CVXB_EQTN_ROCTAN),
/*OutputCoupler */ (CVXB_EQTN_DIST2NEXT | CVXB_EQTN_N | CVXB_EQTN_THICKNESS) // ROC from aux optic link
};

const char* CVertex::CszVxFcnNames = NULL; // plottable properties names, from RC
const char* CVertex::CszVxTypeNames = NULL; // type names for prop manager, from RC

/*########################################################
 ## Class                                              ##
########################################################*/
/*********************************************************
* Constructor
*********************************************************/
///TODO: Check all constructors for all initialization!
CVertex::CVertex(CSystem *pSys, int iTyp) { printf("+Created CVertex\n");
   char  szBuf[512];                       // string table
   char *pszBuf;                           // temporary pointer
   int   iLen;                             // string table entry length
   int k;                                  // property, equation loop counter

   //---Invalidate all members------------------
   memset(szVxTag, 0x00, sizeof(szVxTag));  // vertex tag
   pSysParent  = pSys;                      // parent system
   pVxPrev     = NULL;                      // previous vertex in chain
   pVxNext     = NULL;                      // next vertex in chain
   pVxLink     = NULL;
   iVxType     = iTyp;                      // type of Vx
   pSysSpawned = NULL;                      // spawned system
   uFlags      = 0x0000;                    // no flags set
   for(k=0; k<CVXI_NUM_PROP; k++) ddProp[k] = 0.0000;
   for(k=0; k<CVXI_NUM_EQTN; k++) EEq[k].ParseDoubleEquation(0.0000);

   //---Defaults--------------------------------
   if(pSysParent) sprintf(szVxTag, "%s%d", CszVxTagTemplate[iVxType], pSysParent->NextOpticNumber());
   else           sprintf(szVxTag, "%s%08lx", CszVxTagTemplate[iVxType], this); // see also at LoadVertex!

   //---String table----------------------------
   // Here delimited with '~' (including at end!)
   if(CszVxFcnNames==NULL) {
      LoadString(App()->GetInstance(), CPL_VXFCNNAMES, szBuf, sizeof(szBuf)/sizeof(char));
      for(k=0, iLen=strlen(szBuf); k<iLen; k++) if(szBuf[k]=='~') szBuf[k] = '\0';
      pszBuf = (char*) malloc((iLen+1)*sizeof(char)); // allocate buffer
      memset(pszBuf, 0x00, (iLen+1)*sizeof(char));    // empty the buffer
      memcpy(pszBuf, szBuf, iLen*sizeof(char));       // copy the formatted string
      CszVxFcnNames = (const char*) pszBuf;        // fix pointer
   }
   if(CszVxTypeNames==NULL) {
      LoadString(App()->GetInstance(), CPL_VXTYPENAMES, szBuf, sizeof(szBuf)/sizeof(char));
      for(k=0, iLen=strlen(szBuf); k<iLen; k++) if(szBuf[k]=='~') szBuf[k] = '\0';
      pszBuf = (char*) malloc((iLen+1)*sizeof(char)); // allocate buffer
      memset(pszBuf, 0x00, (iLen+1)*sizeof(char));    // empty the buffer
      memcpy(pszBuf, szBuf, iLen*sizeof(char));       // copy the formatted string
      CszVxTypeNames = (const char*) pszBuf;        // fix pointer
   }
}


/*********************************************************
* Destructor
*********************************************************/
CVertex::~CVertex() {
   //---Spawned systems-------------------------
   if(pSysSpawned != NULL) delete(pSysSpawned); pSysSpawned = NULL;

printf("-CVertex deleted\n");}


/*********************************************************
* App: Access to application object through parent system
*********************************************************/
CApplication* CVertex::App(void) {
   return((pSysParent) ? pSysParent->App() : NULL);
}


/*########################################################
## Data files                                           ##
########################################################*/
/*********************************************************
* SaveVertex
*********************************************************/
void CVertex::SaveVertex(HANDLE hFile) {
   char     szBuf[256];                     // text buffer
   DWORD    dwBytes;                        // bytes written
   UINT     uBit;                           // property bit
   int      iEqn;                           // equation index

   if(hFile == NULL) printf("? CVertex::SaveVertex@36 Called with hFile=NULL\n");
   if(hFile == NULL) return;

   //===Header============================================
   sprintf(szBuf, "%s @ %s {\r\n", CszVxTypeName[iVxType], szVxTag);
   WriteFile(hFile, szBuf, strlen(szBuf), &dwBytes, NULL);

   //===Auxiliaries=======================================
   //---Selection-------------------------------
   if(Selected()) {
      sprintf(szBuf, "   %s\r\n", CszVxSaveAuxName[CVXE_SAVE_SELECTED]);
      WriteFile(hFile, szBuf, strlen(szBuf), &dwBytes, NULL);
   }

   //---Locked----------------------------------
   switch(Type()) {
   case CVX_TYPE_MIRROR:
   case CVX_TYPE_FLATMIRROR:
      if(!LockedAngle()) break;             // skip normal
      sprintf(szBuf, "   %s\r\n", CszVxSaveAuxName[CVXE_SAVE_LOCKANGL]);
      WriteFile(hFile, szBuf, strlen(szBuf), &dwBytes, NULL);
      break;
   }

   //---Flipped---------------------------------
   switch(Type()) {
   case CVX_TYPE_INBREWSTER:
   case CVX_TYPE_PRISM1:
      if(!IsFlipped()) break;               // skip if normal oriented
      sprintf(szBuf, "   %s\r\n", CszVxSaveAuxName[CVXE_SAVE_FLIPPED]);
      WriteFile(hFile, szBuf, strlen(szBuf), &dwBytes, NULL);
      break;
   }

   //---Linked----------------------------------
   switch(Type()) {
   case CVX_TYPE_SCREEN:
      if(pVxLink==NULL) break;            // otherwise drop through...
   case CVX_TYPE_THERMALLENS:
   case CVX_TYPE_INCRYSTAL:
   case CVX_TYPE_OUTCRYSTAL:
   case CVX_TYPE_INBREWSTER:
   case CVX_TYPE_OUTBREWSTER:
   case CVX_TYPE_PRISM1:
   case CVX_TYPE_PRISM2:
   case CVX_TYPE_INPLATE:
   case CVX_TYPE_OUTPLATE:
      sprintf(szBuf, "   %s = %s\r\n", CszVxSaveAuxName[CVXE_SAVE_VXLINK], pVxLink->Tag());
      WriteFile(hFile, szBuf, strlen(szBuf), &dwBytes, NULL);
      break;
   }

   //---Astigmatic ROC / FL---------------------
   switch(Type()) {
   case CVX_TYPE_MIRROR:
   case CVX_TYPE_LENS:
   case CVX_TYPE_THERMALLENS:
   case CVX_TYPE_INCRYSTAL:
   case CVX_TYPE_OUTCRYSTAL:
   case CVX_TYPE_INBREWSTER:
   case CVX_TYPE_OUTBREWSTER:
   case CVX_TYPE_INPLATE:
   case CVX_TYPE_OUTPLATE:
      if(!AstigROCFL()) break;
      sprintf(szBuf, "   %s\r\n", CszVxSaveAuxName[CVXE_SAVE_ROCFLASTIG]);
      WriteFile(hFile, szBuf, strlen(szBuf), &dwBytes, NULL);
      break;

   }

   //===Equations=========================================
   //---Write equations-------------------------
   for(iEqn=0, uBit=0x0001; iEqn<CVXI_NUM_EQTN; iEqn++, uBit=uBit<<1) {
      if((CuVxSavePropertyMask[iVxType] & uBit) != 0) {
         sprintf(szBuf, "   %s = ", CszVxEquationName[iEqn]);
         WriteFile(hFile, szBuf, strlen(szBuf), &dwBytes, NULL);
         EEq[iEqn].GetEquationString(szBuf, sizeof(szBuf));
         WriteFile(hFile, szBuf, strlen(szBuf), &dwBytes, NULL);
         sprintf(szBuf, "\r\n");
         WriteFile(hFile, szBuf, strlen(szBuf), &dwBytes, NULL);
      }
   }

   //---Spawned system--------------------------
   if(pSysSpawned != NULL) {
      sprintf(szBuf, "   %s = %s\r\n", CszVxSaveAuxName[CVXE_SAVE_SPAWN], pSysSpawned->Tag());
      WriteFile(hFile, szBuf, strlen(szBuf), &dwBytes, NULL);
   }

   //===Trailer===========================================
   sprintf(szBuf, "}%s", pVxNext!=NULL ? "\r\n" : "\r\n\r\n");
   WriteFile(hFile, szBuf, strlen(szBuf), &dwBytes, NULL);
}


/*********************************************************
* LoadVertex
* The way this is called: The System creates a new empty
* vertex, then immediately calls LoadVertex. If LoadVer-
* tex returns FALSE, the vertex must be deleted again.
*********************************************************/
BOOL CVertex::LoadVertex(const char *pszDataFile, char **pszSysMin, char **pszSysMax) {
   char  szBuf[256];                        // line buffer for graph and system
   char *pszVxMin, *pszVxMax;               // vertex boundaries
   char *pszEqn;                            // equation pointer
   char *pszProp;                           // property limits
   int   iTyp;                              // loop through vertex types
   int   iEqn;                              // equation loop counter
   UINT  uBit;                              // bit mask
   CSystem  *pSysNew;                       // new spawned system

   if((pszDataFile == NULL)
      || (pszSysMin == NULL)
      || (pszSysMax == NULL)) {
         if(pszDataFile == NULL) printf("? CVertex::LoadVertex@ called with pszDataFile = NULL\n");
         if(pszSysMin   == NULL) printf("? CVertex::LoadVertex@ called with pszSysMin   = NULL\n");
         if(pszSysMax   == NULL) printf("? CVertex::LoadVertex@ called with pszSysMax   = NULL\n");
         return(FALSE);
   }

   //===Find vertex=======================================
   // Logic: pszSysMin points to the current search start
   // (not necessarily the start of the system proper) so
   // we must find  the vertex type before  the next open
   // brace and before pszSysMin.
   //                  | [System]
   //                  | vertex_read {
   //                  |    ...         end search here
   //                  | }            /
   //   pszSysMin -->  | NextVertex {
   //                  |    ...
   //                  | }
   ///TODO: Include an error pointer and message for user
   for(iTyp=0; iTyp<CVX_NUM_TYPE; iTyp++) {
      //---Search for this Vx Type------------------------
      pszVxMax = strchr(*pszSysMin, '{');   // find opening brace
      if((pszVxMax==NULL) || (pszVxMax>*pszSysMax)) return(FALSE); // no brace within system - exit
      pszVxMin = strstr(*pszSysMin, CszVxTypeName[iTyp]);          // look for first occurrence of vx type
      if((pszVxMin==NULL) || (pszVxMin > pszVxMax)) continue;      // type not found within range, search again

      //===Vertex Found===================================
      iVxType = iTyp;                    // store vertex type
      //---Tag--------------------------------------------
      pszVxMin = strchr(pszVxMin, '@');               // look for tag delimiter
      if((pszVxMin!=NULL) && (pszVxMin<pszVxMax)) {   // if occurs before opening brace
         memset(szVxTag, 0x00, sizeof(szVxTag));      // clear tag string buffer
         pszVxMin += strcspn(pszVxMin, CSystem::CszValidNameChar); // skip everything up to start of name
         if(strspn(pszVxMin, CSystem::CszValidNameChar)>CVXC_MAXTAG) strncpy(szVxTag, pszVxMin, CVXC_MAXTAG);
         else                                                        strncpy(szVxTag, pszVxMin, strspn(pszVxMin, CSystem::CszValidNameChar));
      } else {
         if(pSysParent) sprintf(szVxTag, "%s%d", CszVxTagTemplate[iVxType], pSysParent->NextOpticNumber());
         else           sprintf(szVxTag, "%s%08lx", CszVxTagTemplate[iVxType], this); // update name to reflect type
      }

      //---Find boundaries--------------------------------
      pszVxMin = pszVxMax + 1;           // start reading just after '{'
      while((*pszVxMax!='}') && (pszVxMax<*pszSysMax)) pszVxMax++; // move end to '}'

      //===Auxiliaries====================================
      //---Selection--------------------------------------
      pszProp = strstr(pszVxMin, CszVxSaveAuxName[CVXE_SAVE_SELECTED]);
      if((pszProp!=NULL) && (pszProp<pszVxMax)) SetBit(CVXF_SELECTED);

      //---Locked angle-----------------------------------
      pszProp = strstr(pszVxMin, CszVxSaveAuxName[CVXE_SAVE_LOCKANGL]);
      if((pszProp!=NULL) && (pszProp<pszVxMax)) LockAngle(TRUE);

      //---Flipping---------------------------------------
      pszProp = strstr(pszVxMin, CszVxSaveAuxName[CVXE_SAVE_FLIPPED]);
      if((pszProp!=NULL) && (pszProp<pszVxMax)) FlipVertex(TRUE);

      //---Astigmatic ROC / FL----------------------------
      pszProp = strstr(pszVxMin, CszVxSaveAuxName[CVXE_SAVE_ROCFLASTIG]);
      if((pszProp!=NULL) && (pszProp<pszVxMax)) SetAstigROCFL(TRUE);

      //---Linked Vx-----------------------------------
      // This is non-trivial, because  we cannot link the
      // vertices  together until AFTER the whole  system
      // has been loaded.
      // The way we overcome this  is to use pVxLink as a
      // pointer to a temporary string  buffer containing
      // the requested link Vx tag.  The system MUST FREE
      // this string, irrespective of whether the link is
      // available or not. (That's a whole  separate pro-
      // blem.)
      // The good news?  It doesn't matter where the link
      // info appears in the Vx structure.
      // Screens are checked like the others, except that
      // it is not an error if it is not linked UNLESS it
      // is within a block.
      pVxLink = NULL;                       // be absolutely sure we don't point at anything
      switch(Type()) {
      case CVX_TYPE_SCREEN:
      case CVX_TYPE_THERMALLENS:
      case CVX_TYPE_INCRYSTAL:
      case CVX_TYPE_OUTCRYSTAL:
      case CVX_TYPE_INBREWSTER:
      case CVX_TYPE_OUTBREWSTER:
      case CVX_TYPE_PRISM1:
      case CVX_TYPE_PRISM2:
      case CVX_TYPE_INPLATE:
      case CVX_TYPE_OUTPLATE:
         pszProp = strstr(pszVxMin, CszVxSaveAuxName[CVXE_SAVE_VXLINK]);
         if((pszProp!=NULL) && (pszProp<pszVxMax)) {       // search for property within vertex bounds
            pszProp = strchr(pszProp, '=');                // find '=' sign
            if((pszProp!=NULL) && (pszProp<pszVxMax)) {    // ensure '=' within vertex
               sscanf(pszProp+1, "%s", szBuf);             // scan the tag
               pVxLink = (CVertex*) (char*) malloc((strlen(szBuf)+1) * sizeof(char));
               if(pVxLink) strcpy((char*)pVxLink, szBuf);  // copy the requested tag string over
            }//endif('=')
         }//endif(property found)
         break;
      }


      //===Equations======================================
      //---Equations--------------------------------------
      for(iEqn=0, uBit=0x0001; iEqn<CVXI_NUM_EQTN; iEqn++, uBit=uBit<<1) { // loop through properties
         if((CuVxSavePropertyMask[iVxType] & uBit) != 0) {        // if property is part of vx
            pszEqn = strstr(pszVxMin, CszVxEquationName[iEqn]);   // search for property text within vertex
            if((pszEqn!=NULL) && (pszEqn<pszVxMax)) {             // if found..
               pszEqn = strchr(pszEqn, '=');                      //..find '=' sign
               if((pszEqn!=NULL) && (pszEqn<pszVxMax)) {          // ensure '=' sign is within vertex
                  sscanf(pszEqn+1, "%s", szBuf);
                  EEq[iEqn].ParseEquation(szBuf, App()->VarsString()); // parse equations with variables
               }//endif(found '=')
            }//endif(found equation title)
         }//endif(equation part of vertex)
      }//endfor(equations)

      //---Spawned System---------------------------------
      pszProp = strstr(pszVxMin, CszVxSaveAuxName[CVXE_SAVE_SPAWN]); // search for spawn system
      if((pszProp!=NULL) && (pszProp<pszVxMax)) {
         pszProp = strchr(pszProp, '='); // search for '='
         if((pszProp==NULL) || (pszProp>pszVxMax)) break; // exit if not found within boundaries
         //---Read name----------------------
         memset(szBuf, 0x00, sizeof(szBuf));  // clear the buffer
         pszProp += strcspn(pszProp, CSystem::CszValidNameChar);     // scan to start of name
         strncpy(szBuf+1, pszProp, strspn(pszProp, CSystem::CszValidNameChar));  // copy valid name
         szBuf[0] = '['; szBuf[strlen(szBuf)] = ']'; // add boundary brackets
         //---Load System--------------------
         pSysNew = new CSystem(App());
         pSysNew->SetSysType(CSYS_TYPE_SPAWN); // automatically set as spawned system
         if(pSysNew->LoadSystem(pszDataFile, szBuf)) {
            pSysNew->SetVxSpawn(this);   // track system's spawn parent
            pSysSpawned = pSysNew;       // spawn system from vertex
         } else {
            delete(pSysNew);             // delete system if there's a problem
         }
      }

      //---Finish up-----------------------------------
      *pszSysMin = pszVxMax + 1;         // start next search beyond end here
      return(TRUE);                      // return from here, ensure don't read twice

   }
   return(FALSE);                           // some other propblem..?
}


/*########################################################
 ## Properties                                         ##
########################################################*/
/*********************************************************
* FaceAngle
* This function  is nontrivial since it  also affects the
* canvas angle and  angle-to-next properties. The meaning
* of FaceAngle  in the physical  sense may depend  on the
* type of  vertex. The face  angle of Brewster  faces can
* only be retrieved, not set.
* The face angle is  physically restricted  to be between
* -pi/2 and  +pi/2. Since the angle may be defined  as an
* equation, we limit-check the  face angle on output, ra-
* ther than on input.
*********************************************************/
void CVertex::SetFaceAngle(double dAng) {
   switch(Type()) {
   //---Mirrors---------------------------------
   case CVX_TYPE_MIRROR:
   case CVX_TYPE_FLATMIRROR:
      EEq[CVXI_EQTN_FACEANGLE].ParseDoubleEquation(dAng);
      break;
   //---Plate-----------------------------------
   case CVX_TYPE_INPLATE:
      EEq[CVXI_EQTN_FACEANGLE].ParseDoubleEquation(dAng);
      break;
   case CVX_TYPE_OUTPLATE:
      if(pVxLink) pVxLink->SetFaceAngle(dAng);
      break;

   }
}

/*********************************************************
* FaceAngle
* Returns  an incidence or crystal angle. The  definition
* depends on the type of  optic.
*  - MIRROR, FLATMIRROR: Incidence angle
*  - BREWSTER: The external incidence angle, which is the
*              Brewster angle.
*  - CRYSTAL: The crystal CUT angle, which  is the inter-
*             nal angle qi. This may lead to total inter-
*             nal  reflection, so that condition is  pre-
*             vented here.
*  - PLATE: The external incidence angle.
*  - PRISM: The external  incidence  angle, which  is the
*           Brewster angle.
* The values are  calculated from  ddProp[CVXI_PROP_FACE-
* ANGLE], but this split in  definitions was chosen since
* they seem to make most sense when designing a laser.
* The functions
*    Angle2Next()
*    ApplyEquations() ( :VertexABCD)
*    SetCanvasPosition()
* make heavy use of FaceAngle(), so the  definitions here
* must agree with  what those  functions expect. For most
* optic types, the  output face  does not  return a valid
* value, so  these functions  should reference  the input
* face directly. No attention is paid to the FLIPPED bit.
*
* Just to be sure nothing  weird happens, we ACTIVELY re-
* turn zero for anything that cannot be rotated.
*********************************************************/
double CVertex::FaceAngle(void) {
   double dAng;                             // checked value

///TODO: Maybe allow for ring
   if(Prev()==NULL) return(0.00);           // normal incidence at start
   if(Next()==NULL) return(0.00);           // normal incidence at start

   //===Calculate=========================================
   switch(Type()) {
   //---Mirrors---------------------------------
   case CVX_TYPE_MIRROR:
   case CVX_TYPE_FLATMIRROR:
      dAng = ddProp[CVXI_PROP_FACEANGLE] * M_PI / 180.00;
      break;

   //---Crystal---------------------------------
   // Cut angle. Prevents TIR.
   case CVX_TYPE_INCRYSTAL:
      dAng = ddProp[CVXI_PROP_FACEANGLE] * M_PI / 180.00;
      if(RefIndex()*SIN(dAng) > 1.00) dAng = ASIN(1.00/RefIndex());
      if(RefIndex()*SIN(dAng) <-1.00) dAng =-ASIN(1.00/RefIndex());
      break;

   //---Brewster--------------------------------
   // Brewster angle
   case CVX_TYPE_INBREWSTER: dAng = ATAN(RefIndex()); break;

   //---Plate-----------------------------------
   // External incidence angle
   case CVX_TYPE_INPLATE: dAng = ddProp[CVXI_PROP_FACEANGLE] * M_PI / 180.00; break;

   //---Brewster Prism--------------------------
   // FaceAngle is the Brewster angle
   // Can't use RefIndex(), must access ddProp[CVXI_PROP_N] directly
   case CVX_TYPE_PRISM1:
      dAng = ddProp[CVXI_PROP_N]; if(dAng < 1.00) dAng = 1.00;
      dAng = ATAN(dAng);
      break;

   //---Fixed-----------------------------------
   default: dAng = 0.00; break;
   }

   //===Bounded Value=====================================
   if(dAng > M_PI_2) dAng = M_PI_2;
   if(dAng <-M_PI_2) dAng =-M_PI_2;
   return(dAng);
};


/*********************************************************
* Angle2Next
* The canvas angle to the next optic relative to the pre-
* vious is a  somewhat complicated function  that depends
* on the type of vertex. Some vertices are inline only so
* will always return 0. For refractive interfaces, we re-
* ference the input optic to get  the relevant angle; see
* comments at FaceAngle().
* Note that the output face (or  second prism) angles are
* always equal but opposite to the input face angle.
*
* Brewster ext. angle: qB = tan(n)
* Brewster int. angle: qi = asin(sin(qB)/n) = M_PI_2 - qB
* Brewster next angle: qn = qB - qi = 2qB - M_PI_2
*
* Arbitrary external: qe = asin( n sin(qi) )
* Arbitrary internal: qi = asin( sin(qe) / n )
* Arbitrary next:     qn = qe - qi
*
* Prism: Deflection is twice the normal, i.e. 2 qn.
*********************************************************/
double CVertex::Angle2Next(void) {
   double   dAng;                           // returned angle
   CVertex *pVx;                            // loop counter (prisms)

   switch(Type()) {
   //---Mirrors---------------------------------
   case CVX_TYPE_MIRROR:
   case CVX_TYPE_FLATMIRROR:
      if(Prev()==NULL) return(0.00);        ///TODO: Ring cavity?
      return(M_PI - 2.00 * FaceAngle());    // equal input / output angles

   //---Crystal---------------------------------
   // FaceAngle() returns cut angle, TIR-safe
   case CVX_TYPE_INCRYSTAL:
      return( ASIN(RefIndex() * SIN(FaceAngle())) - FaceAngle() );
   case CVX_TYPE_OUTCRYSTAL:
      return((pVxLink==NULL) ? 0.00 : -pVxLink->Angle2Next());

   //---Brewster--------------------------------
   // FaceAngle() return Brewster angle
   case CVX_TYPE_INBREWSTER:
      return( IsFlipped() ?
         -(2.00*FaceAngle() - M_PI_2) :
         +(2.00*FaceAngle() - M_PI_2) );
   case CVX_TYPE_OUTBREWSTER:
      return((pVxLink==NULL) ? 0.00 : -pVxLink->Angle2Next());

   //---Plate-----------------------------------
   // FaceAngle() returns external incidence angle
   case CVX_TYPE_INPLATE:
      return( FaceAngle() - ASIN(SIN(FaceAngle())/RefIndex()) );
   case CVX_TYPE_OUTPLATE:
      return((pVxLink==NULL) ? 0.00 : -pVxLink->Angle2Next());

   //---Brewster Prisms-------------------------
   // Damned prisms. If  there's an odd number  of (flat)
   // mirrors between the prisms, we have to FLIP the se-
   // cond one once more
   case CVX_TYPE_PRISM1:
      return( IsFlipped() ?
         -2.00 * (2.00 * FaceAngle() - M_PI_2) :
         +2.00 * (2.00 * FaceAngle() - M_PI_2) );
   case CVX_TYPE_PRISM2:
      if(pVxLink==NULL) return(0.00);
      dAng = -pVxLink->Angle2Next();
      for(pVx=pVxLink; (pVx) && (pVx!=this); pVx=(CVertex*) pVx->Next()) {
         if(pVx->Type()==CVX_TYPE_FLATMIRROR) dAng = -dAng;
      }
      return(dAng);

   //---Others: Straight------------------------
   default: return(0.00);                   // others are inline
   }
}


/*********************************************************
* Dist2Next
* This used to be a trivial  function. Because refractive
* blocks (crystal, plate, Brewster) are  now defined with
* a thickness parameter and are allowed one centered lens
* or screen, this  function must perform some checks. The
* advantage is  that we can calculate  projection lengths
* here.
* Cases
*  - Input faces
*     - No lens or screen: The projected thickness
*     - Lens or screen: Half projected thickness
*  - Thermal lens: Half projected thickness = same as in-
*    put face (Check pVxLinked to catch bad systems)
*  - Screen:  If linked, half  projected thickness = same
*    as input face
* For the angle  determination, see  also FaceAngle() and
* AbcdVx().
*
* The setter remains trivial, unused parameters can still
* be set.
*********************************************************/
double CVertex::Dist2Next(void) {
   double dRet;                             // returned value
   double qext, qint;                       // external, internal angles, cosine

   dRet = ddProp[CVXI_PROP_DIST2NEXT];      // original trivial version

   switch(Type()) {                         // specific changes
   case CVX_TYPE_INBREWSTER:
      qext = FaceAngle();                   // Brewster angle
      qint = (qext>0.00)                    // internal angle
         ? M_PI_2-qext : -M_PI_2-qext;
      dRet = Thickness() / (COS(qint) + ((COS(qint)==0.00) ? 1.00 : 0.00));
      if((pVxNext) && (pVxNext->Type() != CVX_TYPE_OUTBREWSTER)) dRet /= 2.00;
      break;

   case CVX_TYPE_INPLATE:
      qext = FaceAngle();                   // external incidence angle
      qint = ASIN(SIN(qext) / RefIndex());  // internal angle
      dRet = Thickness() / (COS(qint) + ((COS(qint)==0.00) ? 1.00 : 0.00));
      if((pVxNext) && (pVxNext->Type() != CVX_TYPE_OUTPLATE)) dRet /= 2.00;
      break;

   case CVX_TYPE_INCRYSTAL:
      qint = FaceAngle();                   // internal angle
      dRet = Thickness() / (COS(qint) + ((COS(qint)==0.00) ? 1.00 : 0.00));
      if((pVxNext) && (pVxNext->Type() != CVX_TYPE_OUTCRYSTAL)) dRet /= 2.00;
      break;

   case CVX_TYPE_THERMALLENS:               // lens (pVxLink check for bad system)
      if(pVxLink) dRet = pVxLink->Dist2Next();
      break;

   case CVX_TYPE_SCREEN:                    // screen check if inside
      if(pVxLink) switch(pVxLink->Type()) {
      case CVX_TYPE_INCRYSTAL:
      case CVX_TYPE_INBREWSTER:
      case CVX_TYPE_INPLATE:
         dRet = pVxLink->Dist2Next();       // inside material: D/2
         break;
      }
   }
   return(dRet);                            // return processed value
}

//===SetDist2Next=========================================
void   CVertex::SetDist2Next(double d) {
   EEq[CVXI_EQTN_DIST2NEXT].ParseDoubleEquation(d);
}



/*********************************************************
* Trivial Functions
* The user-settable equation properties are rendered into
* the ddProp fields, e.g. when ApplyVxABCD is called with
* the current variable values.
*********************************************************/
//===General==============================================
double CVertex::X(void)                   { return(ddProp[CVXI_PROP_CANVAS_X]); };
double CVertex::Y(void)                   { return(ddProp[CVXI_PROP_CANVAS_Y]); };
double CVertex::OpticCanvasAngle(void)    { return(ddProp[CVXI_PROP_CANVAS_ANGLE]); };
double CVertex::SegmentCanvasAngle(void)  { return(ddProp[CVXI_PROP_SEGMENT_ANGLE]); };

//===Mirror===============================================
double CVertex::ROC(int iPlane) {
   if(AstigROCFL())
      return(ddProp[(iPlane==SAG) ? CVXI_PROP_ROC : CVXI_PROP_ROCTAN]);
   else
      return(ddProp[CVXI_PROP_ROC]);
};
void CVertex::SetROC(double dSag, double dTan) {
   EEq[CVXI_EQTN_ROC   ].ParseDoubleEquation(dSag);
   EEq[CVXI_EQTN_ROCTAN].ParseDoubleEquation(dTan);
};

//===Lens=================================================
double CVertex::FL(int iPlane) {
   if(AstigROCFL())
      return(ddProp[(iPlane==SAG) ? CVXI_PROP_FL : CVXI_PROP_FLTAN]);
   else
      return(ddProp[CVXI_PROP_FL]);
}
void CVertex::SetFL(double dSag, double dTan) {
   EEq[CVXI_EQTN_FL   ].ParseDoubleEquation(dSag);
   EEq[CVXI_EQTN_FLTAN].ParseDoubleEquation(dTan);
};

//===Output Coupler=======================================
double CVertex::Thickness(void)           { return(ddProp[CVXI_PROP_THICKNESS]); };
void   CVertex::SetThickness(double d)    { EEq[CVXI_EQTN_THICKNESS].ParseDoubleEquation(d); };

/*********************************************************
* Equation Setter
* Returns TRUE if equation is ok
* Called from
* <- CSystem::VxPropItemCallback
*    <- QEditCallback
*       <- Prop manager editing of single selection
*********************************************************/
BOOL CVertex::ParseVxEq(int k, const char *pszSrc) {
   if((k<0) || (k>=CVXI_NUM_EQTN)) return(TRUE); // accept if no equation
   return( EEq[k].ParseEquation(pszSrc, App()->VarsString()) );
}


/*********************************************************
* Refractive Index
* We need to be a little crafty here because a refractive
* block is modeled as TWO vertices. This function returns
* the LOCAL refractive index (i.e., of the NEXT segment),
* as used by SpaceABCD(). Calculations  involving refrac-
* tion in a block must themselves go to the linked vertex
* to get the internal refractive index.
* The unphysical condition of  n < 1.00 is already caught
* here, but no warning is issued.
* As with FaceAngle(), we actively return 1 for any items
* that cannot be within a refractive  block. Items within
* a refractive  block (thermal lens, some  screens), this
* function returns the relevant refractive index from the
* input face of the block.
* Prisms return 1.00. For the angle calculations, access
* ddProp[CVXI_PROP_N] directly.
*********************************************************/
double CVertex::RefIndex(void) {
   double dRet;                             // return value
   switch(Type()) {
   case CVX_TYPE_THERMALLENS:
   case CVX_TYPE_SCREEN:
      dRet = (pVxLink==NULL) ? 1.00 : pVxLink->RefIndex();
      break;
   case CVX_TYPE_INCRYSTAL:
   case CVX_TYPE_INBREWSTER:
   case CVX_TYPE_INPLATE:
      dRet = ddProp[CVXI_PROP_N];
      break;
   default: return(1.00);                   // all others must be 1.00
   }
   if(dRet < 1.00) dRet = 1.00;             // physical constraint!
   return(dRet);                            // return legal refractive index
};

/*********************************************************
* SetRefIndex
* Sets the  refractive index  for the  allowed types. See
* the disambiguation tree in RefIndex and comments there.
*********************************************************/
void CVertex::SetRefIndex(double d) {
   switch(Type()) {
   case CVX_TYPE_THERMALLENS:
   case CVX_TYPE_SCREEN:
      if(pVxLink) pVxLink->SetRefIndex(d);
      break;
   case CVX_TYPE_INCRYSTAL:
   case CVX_TYPE_INBREWSTER:
   case CVX_TYPE_PRISM1:
   case CVX_TYPE_INPLATE:
   case CVX_TYPE_OUTCOUPLER:
      EEq[CVXI_EQTN_N].ParseDoubleEquation(d);
      break;
   }
};

/*********************************************************
* SetCanvasPosition
* Sets the canvas X, Y, and angle properties.  The canvas
* angle depends on a  number of parameters of the vertex.
* The angle dA supplied (by CSystem::PositionCanvasVerti-
* ces) is the angle of the  PREVIOUS segment. The X and Y
* values might need tweaking too, for  example to account
* for prism insertion. The  first optic needs to be flip-
* ped by 180 degrees, since it's usually an end mirror,
* unless it's a propagation system.
*
* The angle that FaceAngle() returns  depends on the type
* of  optic. See notes at  FaceAngle() and  Angle2Next().
* Although it might be a little  slow, we derive the can-
* vas angle using both of these functions.
* Note that  the output object is always parallel  to the
* input object but rotated  by 180 degrees. Since we must
* assume that the input  object has already  been placed,
* we simply pull  the canvas angle from  its CANVAS_ANGLE
* property.
*
* Why ddProp[ANGLE] = <<->> dA ?? Because of reverse y in
* window?
*
* Called from
*  <- CSystem::PlaceCanvasVertices
*  <- CSWin2d::MouseCallback in DRAFT mode
*********************************************************/
void CVertex::SetCanvasPosition(double dX, double dY, double dA) {

   //===Segment===========================================
   ddProp[CVXI_PROP_SEGMENT_ANGLE] = dA + Angle2Next(); // angle of NEXT segment

   //===Optic=============================================
   //---Defaults--------------------------------
   ddProp[CVXI_PROP_CANVAS_X] = dX;
   ddProp[CVXI_PROP_CANVAS_Y] = dY;
   ddProp[CVXI_PROP_CANVAS_ANGLE] = -dA;

   switch(Type()) {
   //---Mirrors---------------------------------
   case CVX_TYPE_MIRROR:
   case CVX_TYPE_FLATMIRROR:
      ddProp[CVXI_PROP_CANVAS_ANGLE] = -dA + FaceAngle();
      break;

   //---Crystal---------------------------------
   // FaceAngle() returns crystal cut angle
   case CVX_TYPE_INCRYSTAL:
      ddProp[CVXI_PROP_CANVAS_ANGLE] = -dA - Angle2Next() - FaceAngle();
      break;
   case CVX_TYPE_OUTCRYSTAL:
      ddProp[CVXI_PROP_CANVAS_ANGLE] =
         (pVxLink==NULL) ? 0.00 : pVxLink->ddProp[CVXI_PROP_CANVAS_ANGLE];
      break;

   //---Brewster--------------------------------
   // FaceAngle() returns the Brewster angle (= incidence angle)
   case CVX_TYPE_INBREWSTER:
      ddProp[CVXI_PROP_CANVAS_ANGLE] = -dA + (IsFlipped() ? FaceAngle() : -FaceAngle());
      break;
   case CVX_TYPE_OUTBREWSTER:
      ddProp[CVXI_PROP_CANVAS_ANGLE] =
         (pVxLink==NULL) ? 0.00 : pVxLink->ddProp[CVXI_PROP_CANVAS_ANGLE];
      break;

   //---Plate-----------------------------------
   // FaceAngle() returns the external incidence angle
   case CVX_TYPE_INPLATE:
      ddProp[CVXI_PROP_CANVAS_ANGLE] = -dA - FaceAngle();
      break;
   case CVX_TYPE_OUTPLATE:
      ddProp[CVXI_PROP_CANVAS_ANGLE] =
         (pVxLink==NULL) ? 0.00 : pVxLink->ddProp[CVXI_PROP_CANVAS_ANGLE];
      break;

   //---Prism pair------------------------------
   // FaceAngle() returns the Brewster angle (=incidence angle)
   case CVX_TYPE_PRISM1:
      ddProp[CVXI_PROP_CANVAS_ANGLE] = -dA + -0.50*Angle2Next() + (IsFlipped() ? M_PI : 0.00);
      break;
   case CVX_TYPE_PRISM2:
      if(pVxLink==NULL) break;              // catch bad linkage
      ddProp[CVXI_PROP_CANVAS_ANGLE] = pVxLink->ddProp[CVXI_PROP_CANVAS_ANGLE]
         + pVxLink->ddProp[CVXI_PROP_SEGMENT_ANGLE];
      for(CVertex* pVx=pVxLink; (pVx) && (pVx!=this); pVx=(CVertex*) pVx->Next()) {
         if(pVx->Type()==CVX_TYPE_FLATMIRROR) ddProp[CVXI_PROP_CANVAS_ANGLE] = -ddProp[CVXI_PROP_CANVAS_ANGLE] + M_PI;
      }
      ddProp[CVXI_PROP_CANVAS_ANGLE] += -dA  + M_PI;

      break;

   //---Output coupler--------------------------
   case CVX_TYPE_OUTCOUPLER:
      ddProp[CVXI_PROP_CANVAS_ANGLE] = -dA
         + ((pSysParent->VxSpawn()) ? pSysParent->VxSpawn()->FaceAngle() : 0.00);
      break;

   }

   //===Reverse first optic===============================
    if(Prev()==NULL) switch(pSysParent->SysType()) {
       case CSYS_TYPE_RESO:
         ddProp[CVXI_PROP_CANVAS_ANGLE] += M_PI;
         break;
    }
}


/*########################################################
 ## ABCD                                                ##
########################################################*/
/*********************************************************
* ApplyEquations
* Applies the current  variable values  and evaluates the
* vertex equations to get parameter values
*********************************************************/
void CVertex::ApplyEquations(double *pcdVar) {
   int iEqn;                                // equation loop counter

///TODO: Calculate only those equations that are NEEDED
   //---Equations-------------------------------
   for(iEqn=0; iEqn<CVXI_NUM_EQTN; iEqn++) {
      ddProp[CVXI_PROP_EQTN+iEqn] = EEq[iEqn].Answer(pcdVar);
   }

   //---Limits----------------------------------
   if(ddProp[CVXI_PROP_DIST2NEXT] < 0.00) ddProp[CVXI_PROP_DIST2NEXT] = 0.00;
   if(ddProp[CVXI_PROP_THICKNESS] < 0.00) ddProp[CVXI_PROP_THICKNESS] = 0.00;

   //---Astigmatism-----------------------------
   // The ROC(.)  and FL(.) functions return  the correct
   // values,  but because  CVertex allows access  to its
   // properties  with Prop(.), we  must ensure that  the
   // correct value is stored here.
   if(!AstigROCFL()) {
      ddProp[CVXI_PROP_ROCTAN] = ddProp[CVXI_PROP_ROC];
      ddProp[CVXI_PROP_FLTAN ] = ddProp[CVXI_PROP_FL ];
   }

   //---Specifics-------------------------------
   switch(Type()) {
   case CVX_TYPE_PRISM2:
      if(pVxLink) ddProp[CVXI_PROP_N] = pVxLink->ddProp[CVXI_PROP_N];
      break;
   case CVX_TYPE_OUTCOUPLER:
      ddProp[CVXI_PROP_ROC   ] = (pSysParent->VxSpawn()) ? pSysParent->VxSpawn()->ROC(SAG) : 0.00;
      ddProp[CVXI_PROP_ROCTAN] = (pSysParent->VxSpawn()) ? pSysParent->VxSpawn()->ROC(TAN) : 0.00;
      break;
   }
}


/*********************************************************
* VxABCDs
*
* The matrix value for interfaces depend on the direction
* in which the beam is travelling; we need to reverse the
* "angles" when coming back the other way. (CHECK THIS!)
* The reasoning for not storing these matrices is that on
* average they will only be  calculated once per variable
* value anyway, when the system is solved, since the RecQ
* values ARE saved. Short of  dynamic allocation, storing
* four matrices per optic is also unnecessarily wasteful,
* so we just calculate the values directly  here whenever
* they are needed.
*
* If either matrix pointer is NULL, the relevant calcula-
* tion is skipped.
*
* Matrices from  AE Siegman, "Lasers," University Science
* Books, Calif. U.S.A. (1986)
*  Space      [ 1   L / n ]
*             [ 0     1   ]
*  Lens
*             [    1     0 ]
*             [ -1 / f   1 ]
*  Mirror; R > 0 for concave
*             [    1     0 ]
*             [ -2 / R   1 ]
*     Arbitrary incidence:
*        Tangential: R --> R cos(theta)
*        Sagittal:   R --> R / cos(theta)
*  Curved dielectric, n1 -> n2, R > 0 concave
*             [     1       0 ]
*             [ (n2-n1)/R   1 ]
*     Arbitrary theta, tangential
*        n1 sin(theta1) = n2 sin(theta2)
*               n2 cos(theta2) - n1 cos(theta1)
*        Dne = ---------------------------------
*                   cos(theta1) cos(theta2)
*             [ cos(theta2)/cos(theta1)           0              ]
*             [       Dne / R            cos(theta1)/cos(theta2) ]
*     Arbitrary theta, sagittal
*        Dne = n2 cos(theta2) - n1 cos(theta1)
*             [    1      0 ]
*             [ Dne / R   1 ]
*  Duct
*      n(x) = n0 - (1/2)(n2 x^2)
*  gamma = sqrt(n2 / n0)
*             [     cos(gamma z)          sin(gamma z) / (n0 gamma) ]
*             [-(n0 gamma) sin(gamma z)        cos(gamma z)         ]
*
* Note on  refractive media: When calculating  the cavity
* round-trip matrix, it's not enough to calculate M1, M2,
* M3 for the block  input, propagation, and output matri-
* ces and use M3 M2 M1 on the way  up and M1 M2 M3 on the
* way down through  the cavity, because the  order of n1,
* n2 changes when you go through the interface backwards.
*
* Sign conventions: See the LaserCanvas5 doc.
*  - Mirrors:    ROC +ve ==> concave (focusing)
*  - Lenses:     FL  +ve ==> focusing (convex)
*  - Interfaces: ROC +ve ==> concave (defocusing)
* It's a bit silly that they're all different. Anyway.
* From the matrix element  equivalence between a focusing
* lens with f = |f| and a plano-convex  zero-length block
* with R = -|R|, we have
*       -1        (n2 - n1)
*      -----  =  -----------
*       |f|         -|R|
* or (n2 - n1) = |R|/|f| > 0. This is only satisfied when
* n2 = n (internal) and n1 = 1 (external) IRRESPECTIVE of
* whether the interface is input / output, or whether the
* cavity is being spanned forwards or backwards.
*
* Angles
* At an angled interface, the tangential mode must expand
* going in and shrink coming out. The external angle qEXT
* is always larger than the internal qINT, which means in
* each of these cases the A element is given by
*
*  - Input  -->--  cos(qInt) / cos(qExt) >= 1
*  - Output -->--  cos(qExt) / cos(qInt) <= 1
*  - Output --<--  cos(qInt) / cos(qExt) >= 1
*  - Input  --<--  cos(qExt) / cos(qInt) <= 1
*********************************************************/
void CVertex::AbcdVx(CMatrix2x2 *pMxSag, CMatrix2x2 *pMxTan, BOOL tfForwards) {
   double n1, n2, q1, q2;                   // angled plate calculations
   double nint, qext, qint;                 // refractive index, internal / external angles
   double Dne, denom;                       // equivalent refractive index difference
   CMatrix2x2 mxTemp;                       // temporary matrix

   switch(Type()) {

   //===Mirror============================================
   case CVX_TYPE_MIRROR:
      if(pMxSag) pMxSag->Set(
         1.00,                                                    0.00,
         ROC(SAG)==0.00?0.00 : -(2.00*COS(FaceAngle()))/ROC(SAG), 1.00);
      if(pMxTan) pMxTan->Set(
         1.00,                                                    0.00,
         ROC(TAN)==0.00?0.00 : -2.00/(COS(FaceAngle())*ROC(TAN)), 1.00);
      break;


   //===Lens==============================================
   case CVX_TYPE_LENS:
   case CVX_TYPE_THERMALLENS:
      if(pMxSag) pMxSag->Set(
         1.00,                               0.00,
         FL(SAG)==0.00?0.00 : -1.00/FL(SAG), 1.00);
      if(pMxTan) pMxTan->Set(
         1.00,                               0.00,
         FL(TAN)==0.00?0.00 : -1.00/FL(TAN), 1.00);
      break;


   //===Interfaces========================================
   case CVX_TYPE_INPLATE:
   case CVX_TYPE_OUTPLATE:
   case CVX_TYPE_INBREWSTER:
   case CVX_TYPE_OUTBREWSTER:
   case CVX_TYPE_INCRYSTAL:
   case CVX_TYPE_OUTCRYSTAL:
      switch(Type()) {
      //===Brewster Interfaces============================
      // FaceAngle(): Returns (external) Brewster angle
      // These calculations  are slightly  faster because
      // Snell's law simplifies to  q2 = pi/2 - q1.  Note
      // convention  that q1 and q2 have same sign, so we
      // must treat negative external angles differently.
      case CVX_TYPE_INBREWSTER:
         nint = RefIndex();                 // internal refractive index
         qext = FaceAngle();                // Brewster angle
         qint = (qext>0.00)                 // internal angle
            ? M_PI_2-qext : -M_PI_2-qext;
         break;
      case CVX_TYPE_OUTBREWSTER:
         if(pVxLink==NULL) {                // catch broken link
            nint = 1.00; qext = 0.00;       // default values
         } else {
            nint = pVxLink->RefIndex();     // internal refractive index
            qext = pVxLink->FaceAngle();    // external incidence angle
            qint = (qext>0.00)              // internal angle
               ? M_PI_2-qext : -M_PI_2-qext;
         }
         break;

      //==="External" Interfaces==========================
      // FaceAngle(): Returns external incidence angle
      case CVX_TYPE_INPLATE:
         nint = RefIndex();                 // internal refractive index
         qext = FaceAngle();                // external incidence angle
         qint = ASIN(SIN(qext) / nint);     // internal angle
         break;
      case CVX_TYPE_OUTPLATE:
         if(pVxLink==NULL) {                // catch broken link
            nint = 1.00; qext = 0.00; qint = 0.00; // default values
         } else {
            nint = pVxLink->RefIndex();     // internal refractive index
            qext = pVxLink->FaceAngle();    // external incidence angle
            qint = ASIN(SIN(qext) / nint);  // internal angle
         }
         break;

      //==="Internal" Interfaces==========================
      // FaceAngle(): Returns the internal cut angle, TIR
      // safe, but we check it anyway.
      case CVX_TYPE_INCRYSTAL:
         nint = RefIndex();                 // internal refractive index
         qint = FaceAngle();                // internal angle
         qext =                             // external angle (caution TIR)
            (SIN(qint)*nint) >= 1.00 ? M_PI_2 :
            (SIN(qint)*nint) <=-1.00 ?-M_PI_2 :
            ASIN(SIN(qint)*nint);
         break;
      case CVX_TYPE_OUTCRYSTAL:
         if(pVxLink==NULL) {                // catch broken link
            nint = 1.00; qext = 0.00; qint = 0.00; // default values
         } else {
            nint = pVxLink->RefIndex();     // internal refractive index
            qint = pVxLink->FaceAngle();    // internal angle
            qext =                          // external angle (caution TIR)
               (SIN(qint)*nint) >= 1.00 ? M_PI_2 :
               (SIN(qint)*nint) <=-1.00 ?-M_PI_2 :
               ASIN(SIN(qint)*nint);
         }
         break;
      }

      //===Interfaces=====================================
      // Curvature and refractive indices (see notes)
      //  - Always R < 0 for focusing
      //  - Always n2 = n; n1 = 1.00
      // Angles
      //  - Input  -->--  q2 = qInt, q1 = qExt
      //  - Output -->--  q2 = qExt, q1 = qInt
      //  - Output --<--  q2 = qInt, q1 = qExt
      //  - Input  --<--  q2 = qExt, q1 = qInt
      switch(Type()) {
      case CVX_TYPE_INBREWSTER:
      case CVX_TYPE_INPLATE:
      case CVX_TYPE_INCRYSTAL:
         n1 = 1.00;
         q1 = (tfForwards) ? qext : qint;
         n2 = nint;
         q2 = (tfForwards) ? qint : qext;
         break;
      case CVX_TYPE_OUTBREWSTER:
      case CVX_TYPE_OUTPLATE:
      case CVX_TYPE_OUTCRYSTAL:
         n1 = 1.00;
         q1 = (tfForwards) ? qint : qext;
         n2 = nint;
         q2 = (tfForwards) ? qext : qint;
         break;
      }

      //---Quantities---------------------------
      // For the (unlikely) case that the interface is at
      // normal incidence, we can set the matrix elements
      // without going through the whole calculation.
      // We must always handle ROC = 0 separately because
      // it is short-hand for ROC = \infty, and so should
      // give a zero C element.
      if(qext==0.00) {                      // normal incidence
         if(pMxSag) pMxSag->Set(
            1.00,                                     0.00,
            (ROC(SAG)==0.00)?0.00 : (n2-n1)/ROC(SAG), 1.00);
         if(pMxTan) pMxTan->Set(
            1.00,                                     0.00,
            (ROC(TAN)==0.00)?0.00 : (n2-n1)/ROC(TAN), 1.00);
      } else {                              // arbitrary incidence
         ///TODO: Can we simplify this for Brewster angles?
         Dne = nint*COS(qint) - 1.00*COS(qext); // effective ref index difference
         denom = COS(q1) * COS(q2);         // denominator
         if(denom==0.00) denom = 1.00;      // prevent #DIV/0! errors

         if(pMxSag) pMxSag->Set(
            1.00,                                 0.00,
            (ROC(SAG)==0.00)?0.00 : Dne/ROC(SAG), 1.00);
         if(pMxTan) pMxTan->Set(
            (COS(q1)==0.00)?1.00 : COS(q2)/COS(q1),       0.00,
            (ROC(TAN)==0.00)?0.00 : Dne/(denom*ROC(TAN)), (COS(q2)==0.00)?1.00 : COS(q1)/COS(q2));
      }
      break;

   //===Output Coupler====================================
   // Characterized by  curvature (from  spawning vx) and
   // propagation  through material n length d.  The ABCD
   // is thus a combination of INPLATE and propagation.
   case CVX_TYPE_OUTCOUPLER:
      if(pSysParent->VxSpawn()==NULL) break; // prevent errors on bad link

      //---Quantities---------------------------
      nint = ddProp[CVXI_PROP_N];           // internal refractive index
      qext = pSysParent->VxSpawn()->FaceAngle(); // external incidence angle from linked Vx
      qint = ASIN(SIN(qext) / nint);        // internal angle

      //---Curved Face--------------------------
      // equivalent to curved input face, forwards
      n1 = 1.00;
      q1 = qext;
      n2 = nint;
      q2 = qint;

      if(qext==0.00) {                      // normal incidence
         if(pMxSag) pMxSag->Set(
            1.00,
               0.00,
            (pSysParent->VxSpawn()->ROC(SAG)==0.00)?0.00 : (n2-n1)/pSysParent->VxSpawn()->ROC(SAG),
               1.00);
         if(pMxTan) pMxTan->Set(
            1.00,
               0.00,
            (pSysParent->VxSpawn()->ROC(TAN)==0.00)?0.00 : (n2-n1)/pSysParent->VxSpawn()->ROC(TAN),
               1.00);
      } else {                              // arbitrary incidence
         Dne = nint*COS(qint) - 1.00*COS(qext); // effective ref index difference
         denom = COS(q1) * COS(q2);         // denominator
         if(denom==0.00) denom = 1.00;      // prevent #DIV/0! errors

         if(pMxSag) pMxSag->Set(
            1.00,
               0.00,
            (pSysParent->VxSpawn()->ROC(SAG)==0.00)?0.00 : Dne/pSysParent->VxSpawn()->ROC(SAG),
               1.00);
         if(pMxTan) pMxTan->Set(
            (COS(q1)==0.00)?1.00 : COS(q2)/COS(q1),
              0.00,
            (pSysParent->VxSpawn()->ROC(TAN)==0.00)?0.00 : Dne/(denom*pSysParent->VxSpawn()->ROC(TAN)),
               (COS(q2)==0.00)?1.00 : COS(q1)/COS(q2));
      }

      //---Propagation--------------------------
      // Here we must also account for the longer dis-
      // tance traveled due to refraction
      if(nint <= 0.00) nint = 1.00;         // prevent #DIV/0! errors
      mxTemp.Set(
         1.00,
            (1.00/nint) * Thickness() / (COS(qint) + ((COS(qint)==0.00)?1.00:0.00)),
         0.00,
            1.00);
      if(pMxSag) pMxSag->PreMult(&mxTemp);
      if(pMxTan) pMxTan->PreMult(&mxTemp);

      //---Back face----------------------------
      // This is like output face, forwards, with flat ROC
      // means there's no difference in the sagittal plane
      n1 = 1.00;
      q1 = qint;
      n2 = nint;
      q2 = qext;
      if(qext!=0.00) {                      // arbitrary incidence
         Dne = nint*COS(qint) - 1.00*COS(qext); // effective ref index difference
         denom = COS(q1) * COS(q2);         // denominator
         if(denom==0.00) denom = 1.00;      // prevent #DIV/0! errors
         mxTemp.Set(
            (COS(q1)==0.00)?1.00 : COS(q2)/COS(q1),  0.00,
            0.00,                                   (COS(q2)==0.00)?1.00 : COS(q1)/COS(q2));
         if(pMxTan) pMxTan->PreMult(&mxTemp);
      }
      break;

   //===Identity Elements=================================
   // These elements have no net effect  on the beam. The
   // output coupler  is managed separately when the sys-
   // tem mode is launched by the parent system.
   case CVX_TYPE_FLATMIRROR:
   case CVX_TYPE_SCREEN:
   case CVX_TYPE_PRISM1:
   case CVX_TYPE_PRISM2:
      if(pMxSag) pMxSag->Eye();
      if(pMxTan) pMxTan->Eye();
      break;

   //===Null elements=====================================
   // The source shouldn't be used except for the property manager
   case CVX_TYPE_SOURCE:
      if(pMxSag) pMxSag->Set(1.00, 0.00, 0.00, 1.00);
      if(pMxTan) pMxTan->Set(1.00, 0.00, 0.00, 1.00);
      break;


   //===Exceptions========================================
   default:
      if(pMxSag) pMxSag->Eye();
      if(pMxTan) pMxTan->Eye();
      printf("CVertex::VxAbcd@915: WARNING! %s not handled",
         (Type()==CVX_TYPE_MIRROR     ) ? "CVX_TYPE_MIRROR     " :
         (Type()==CVX_TYPE_LENS       ) ? "CVX_TYPE_LENS       " :
         (Type()==CVX_TYPE_FLATMIRROR ) ? "CVX_TYPE_FLATMIRROR " :
         (Type()==CVX_TYPE_THERMALLENS) ? "CVX_TYPE_THERMALLENS" :
         (Type()==CVX_TYPE_SCREEN     ) ? "CVX_TYPE_SCREEN     " :
         (Type()==CVX_TYPE_SOURCE     ) ? "CVX_TYPE_SOURCE     " :
         (Type()==CVX_TYPE_INCRYSTAL  ) ? "CVX_TYPE_INCRYSTAL  " :
         (Type()==CVX_TYPE_OUTCRYSTAL ) ? "CVX_TYPE_OUTCRYSTAL " :
         (Type()==CVX_TYPE_INBREWSTER ) ? "CVX_TYPE_INBREWSTER " :
         (Type()==CVX_TYPE_OUTBREWSTER) ? "CVX_TYPE_OUTBREWSTER" :
         (Type()==CVX_TYPE_PRISM1     ) ? "CVX_TYPE_PRISM1     " :
         (Type()==CVX_TYPE_PRISM2     ) ? "CVX_TYPE_PRISM2     " :
         (Type()==CVX_TYPE_INPLATE    ) ? "CVX_TYPE_INPLATE    " :
         (Type()==CVX_TYPE_OUTPLATE   ) ? "CVX_TYPE_OUTPLATE   " :
         (Type()==CVX_TYPE_OUTCOUPLER ) ? "CVX_TYPE_OUTCOUPLER " : "UNKNOWN_CVX_TYPE");

   }
}

/*********************************************************
* AbcdSp
* Fills in the matrices associated with the space follow-
* ing the vertex. If either pointer is NULL, the relevant
* calculation is skipped. The tfForwards flag is not used
* but is  kept to match the signature of the  vertex ABCD
* function.
*********************************************************/
void CVertex::AbcdSp(CMatrix2x2 *pMxSag, CMatrix2x2 *pMxTan) {
   // RefIndex() returns the LOCAL refractive index, i.e.
   // of the following segment. This is usually ok except
   // in the case  of the prisms, which don't  have a re-
   // fractive index space. However, RefIndex() retruns 1
   // for prisms, so we can use the function directly.
   if(pMxSag) pMxSag->Set(1.00, Dist2Next() / RefIndex(), 0.00, 1.00);
   if(pMxTan) pMxTan->Set(1.00, Dist2Next() / RefIndex(), 0.00, 1.00);
}


/*********************************************************
* SetQVx
* Sets the  vertex 1/q parameter. The parameter  is AFTER
* the  given optic, so applies  to the space  immediately
* following the vertex.
* Called from
*   <-- CSystem::SolveSystemABCD : Apply
*********************************************************/
void CVertex::SetQVx(const CRecQ *pcQSag, const CRecQ *pcQTan) {
   CRecQ *pQ;                               // suppress "Non-const function CRecQ::GetR() called for const object"
   //---Set Q Value-----------------------------
   if(pcQSag) { pQ = (CRecQ*) pcQSag; QVx[SAG].Set(pQ->Get_R(), pQ->Get_v(), pQ->WLen(), pQ->M2()); };
   if(pcQTan) { pQ = (CRecQ*) pcQTan; QVx[TAN].Set(pQ->Get_R(), pQ->Get_v(), pQ->WLen(), pQ->M2()); };

   //---Calculate Parameters--------------------
   ddProp[ CVXI_PROP_MODESAG  ] = QVx[SAG].W();
   ddProp[ CVXI_PROP_CURVSAG  ] = QVx[SAG].R();
   ddProp[ CVXI_PROP_WAISTSAG ] = QVx[SAG].W0();
   ddProp[ CVXI_PROP_WDISTSAG ] = QVx[SAG].z0();
   ddProp[ CVXI_PROP_ZRSAG    ] = QVx[SAG].zR();
   ddProp[ CVXI_PROP_MODETAN  ] = QVx[TAN].W();
   ddProp[ CVXI_PROP_CURVTAN  ] = QVx[TAN].R();
   ddProp[ CVXI_PROP_WAISTTAN ] = QVx[TAN].W0();
   ddProp[ CVXI_PROP_WDISTTAN ] = QVx[TAN].z0();
   ddProp[ CVXI_PROP_ZRTAN    ] = QVx[TAN].zR();
   ddProp[ CVXI_PROP_ASTIG    ] = ddProp[CVXI_PROP_WDISTSAG] - ddProp[CVXI_PROP_WDISTTAN];
}


/*********************************************************
* SymmetricABCD
* Returns TRUE  if the mode for the next segment  is sym-
* metric in sagittal and tangential directions
*********************************************************/
BOOL CVertex::SymmetricQVx(void) {
   return( (QVx[SAG] == QVx[TAN]) ? TRUE : FALSE );
}


/*********************************************************
* VxFcnString
* Returns the string corresponding to the specified plot-
* ting function index.
* Called from
*  <- renderer::OnPaint
*********************************************************/
const char* CVertex::VxFcnString(int iIndx) {
   char *psz;                               // string pointer loop counter
   if(CszVxFcnNames==NULL) return(NULL);    // ignore if not loaded
   if((iIndx<0) || (iIndx>CVXI_PROP_FCNMAX)) return(NULL);
   for(psz=(char*)CszVxFcnNames; iIndx>0; iIndx--, psz+=strlen(psz)+1);
   return((const char*)psz);
}

/*********************************************************
* TypeString
* Returns the string representing this type.
* The first entry  in the list is the  multiple-selection
* heading, so that CSystem can access it directly without
* having to call anything.
* Called from:
* <- CSystem::PrepareVxProperties
*********************************************************/
const char* CVertex::TypeString(void) {
   char *psz;                               // string pointer loop counter
   int   k;                                 // loop counter
   if(CszVxTypeNames==NULL) return(NULL);   // ignore if not loaded
   for(psz=(char*)CszVxTypeNames, k=-1; k<Type(); k++, psz+=strlen(psz)+1);
   return((const char*)psz);
}

/*********************************************************
* FcnValue
* Returns  the relevant  value(s) for the given  plotting
* function. Before calling  this, the Application has ap-
* plied the equations  and the system  has solved for the
* ABCD.
* The iPltFcn value is one of the CVXI_PROP constants. In
* the SysWin renderer its value is also an index into the
* dropdown or popup list, hence they are listed first. In
* the case of sagittal/tangential type value pairs, the
* "sagittal" iPltFcn value represents the pair. For exam-
* ple iPltFcn = CVXI_PROP_MODESAG returns both the sagit-
* tal and tangential mode.
* Called from
*  <- CSysWinVxGraph::GraphPoint
*********************************************************/
void CVertex::FcnValue(int iPltFcn, double *pdSag, double *pdTan) {

   switch(iPltFcn) {
   case CVXI_PROP_MODESAG:
      if(pdSag) *pdSag = pSysParent->StableABCD(SAG) ? QVx[SAG].W() : 0.00;
      if(pdTan) *pdTan = pSysParent->StableABCD(TAN) ? QVx[TAN].W() : 0.00;
      break;
   case CVXI_PROP_CURVSAG:
      if(pdSag) *pdSag = pSysParent->StableABCD(SAG) ? QVx[SAG].R() : 0.00;
      if(pdTan) *pdTan = pSysParent->StableABCD(TAN) ? QVx[TAN].R() : 0.00;
      break;
   case CVXI_PROP_WAISTSAG:
      if(pdSag) *pdSag = pSysParent->StableABCD(SAG) ? QVx[SAG].W0() : 0.00;
      if(pdTan) *pdTan = pSysParent->StableABCD(TAN) ? QVx[TAN].W0() : 0.00;
      break;
   case CVXI_PROP_WDISTSAG:
      if(pdSag) *pdSag = pSysParent->StableABCD(SAG) ? QVx[SAG].z0() : 0.00;
      if(pdTan) *pdTan = pSysParent->StableABCD(TAN) ? QVx[TAN].z0() : 0.00;
      break;
   case CVXI_PROP_ZRSAG:
      if(pdSag) *pdSag = pSysParent->StableABCD(SAG) ? QVx[SAG].zR() : 0.00;
      if(pdTan) *pdTan = pSysParent->StableABCD(TAN) ? QVx[TAN].zR() : 0.00;
      break;
   case CVXI_PROP_ASTIG:
      if(pdSag) *pdSag = ((pSysParent->StableABCD(SAG)) && (pSysParent->StableABCD(TAN)))
         ? (QVx[SAG].z0()-QVx[TAN].z0()) : 0.00;
      break;
   }
}

/*########################################################
## Debug Code                                           ##
########################################################*/
/*********************************************************
* DebugPrint
* Prints info about the vertex
*********************************************************/
void CVertex::DebugPrint(char *psz, int *pInt) {
   for(int k=0; k<*pInt; k++) sprintf(psz+strlen(psz), "   ");
   sprintf(psz+strlen(psz), "%s <%s> (%s <- | -> %s) $ %s",
      (Type() == CVX_TYPE_MIRROR       ) ? "MIRROR" :
      (Type() == CVX_TYPE_LENS         ) ? "LENS" :
      (Type() == CVX_TYPE_FLATMIRROR   ) ? "FLATMIRROR" :
      (Type() == CVX_TYPE_THERMALLENS  ) ? "THERMALLENS" :
      (Type() == CVX_TYPE_SCREEN       ) ? "SCREEN" :
      (Type() == CVX_TYPE_SOURCE       ) ? "SOURCE" :
      (Type() == CVX_TYPE_INCRYSTAL    ) ? "INCRYSTAL" :
      (Type() == CVX_TYPE_OUTCRYSTAL   ) ? "OUTCRYSTAL" :
      (Type() == CVX_TYPE_INBREWSTER   ) ? "INBREWSTER" :
      (Type() == CVX_TYPE_OUTBREWSTER  ) ? "OUTBREWSTER" :
      (Type() == CVX_TYPE_PRISM1       ) ? "PRISM1" :
      (Type() == CVX_TYPE_PRISM2       ) ? "PRISM2" :
      (Type() == CVX_TYPE_INPLATE      ) ? "INPLATE" :
      (Type() == CVX_TYPE_OUTPLATE     ) ? "OUTPLATE" :
      (Type() == CVX_TYPE_OUTCOUPLER   ) ? "OUTCOUPLER" : "Unknown",
      szVxTag,
      pVxPrev ? pVxPrev->Tag() : "---",
      pVxNext ? pVxNext->Tag() : "---",
      pVxLink ? pVxLink->Tag() : "---");
   if(pSysSpawned) {
      sprintf(psz+strlen(psz), "Spawn<%s> {\n", pSysSpawned->Tag());
      (*pInt)++;
      pSysSpawned->DebugPrint(psz, pInt);
      for(int k=0; k<*pInt; k++) sprintf(psz+strlen(psz), "   ");
      sprintf(psz+strlen(psz), "}");
      (*pInt)--;
   }
   sprintf(psz+strlen(psz), "\n");
}


/*########################################################
 ## Move Functions
########################################################*/
/*********************************************************
* CanStretch
* Returns TRUE if, based on the current selection, the vx
* segment following can act as the stretch part in a move
*********************************************************/
BOOL CVertex::CanStretch(BOOL tfFwd) {
   //---Selection-------------------------------
   if(tfFwd) {
      if((Next()==NULL) || (Next()->Selected())) return(FALSE);
   } else {
      if(Selected()) return(FALSE);
   }

   //---Types-----------------------------------
   switch(Type()) {
   case CVX_TYPE_SCREEN:
   case CVX_TYPE_THERMALLENS:               // intra-block optics move with
      if(pVxLink==NULL) return(TRUE);       // shouldn't happen
      switch(pVxLink->Type()) {
      case CVX_TYPE_INBREWSTER:
      case CVX_TYPE_INCRYSTAL:
      case CVX_TYPE_INPLATE:
         return(FALSE);
      default:
         return(TRUE);
      }
   case CVX_TYPE_INBREWSTER:
   case CVX_TYPE_INCRYSTAL:
   case CVX_TYPE_INPLATE:
      return(FALSE);                        // blocks move with
   default:
      return(TRUE);
   }
}

/*********************************************************
* CanAnchor
* Returns TRUE if, based on the current selection and vx
* type, this vertex can act as an anchor in a move.
* Only really applies to mirrors.
* This function is called both when the equivalent chains
* are being established (pSysParent->VxDrag==NULL) and on
* subsequent dragging calls (non-NULL VxDrag).
* Called from
* <- InitMoveVxTo
* <- MoveVxTo
*********************************************************/
BOOL CVertex::CanAnchor(void) {
   switch(Type()) {
   case CVX_TYPE_MIRROR:
   case CVX_TYPE_FLATMIRROR:
      if((pSysParent->VxDrag()==NULL))      // InitMoveVxTo: selection continues
         return((Selected() || LockedAngle()) ? FALSE : TRUE);
      return(LockedAngle() ? FALSE : TRUE); // fixed angles can't anchor

   case CVX_TYPE_SCREEN:
      if(Next()==NULL) return(Selected() ? FALSE : TRUE); // screen at end of system
      return(FALSE);                        // intra-system screen never anchors

   case CVX_TYPE_SOURCE:
   case CVX_TYPE_OUTCOUPLER:
      return(Selected() ? FALSE : TRUE);

   default:
      return(FALSE);
   }
}

/*********************************************************
* InitMoveVxTo
* Initialize the equivalent chains.
* Another reason for  having a separate function for this
* is to help with debugging.
*********************************************************/
void CVertex::InitMoveVxTo(void) {
   CVertex *pVx;                            // loop counter
   double   dEpX, dEpY;                     // equivalent chain cartesian
   double   dEnX, dEnY;                     // equivalent chain cartesian
   double   dEpR = 0.00, dEpA = 0.00, dEpS = 0.00, dEpF = 0.00, dEpP = 0.00, dEpK = 0.00;   // polar coordinates, angles
   double   dEnR = 0.00, dEnA = 0.00, dEnS = 0.00, dEnF = 0.00, dEnP = 0.00, dEnK = 0.00;   // polar coordinates, angles

   if(!Selected()) return;                  // ignore bad selection (causes chain problems)
   // Defaults

   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   // Chains
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   //===Backwards Chain===================================
   dEpX = dEpY = 0.00;                      // increment equivalent chain
   pVx = Prev();                            // start looking at previous
   while((pVx!=NULL) && ((pSysParent->VxStrPrev()==NULL) || (pSysParent->VxAncPrev()==NULL))) {
      //---Stretch------------------------------
      if((pSysParent->VxStrPrev()==NULL) && (pVx->CanStretch(FALSE))) { // either stretch..
         pSysParent->SetVxStrPrev(pVx);
      } else {                              //..or add to equivalent (fixed) chain
         dEpX += pVx->Dist2Next() * COS(pVx->SegmentCanvasAngle());
         dEpY += pVx->Dist2Next() * SIN(pVx->SegmentCanvasAngle());
      }
      //---Anchor-------------------------------
      if(pVx->CanAnchor()) {
         pSysParent->SetVxAncPrev(pVx);
         break;
      }
      pVx = pVx->Prev();                    // continue looking
   }

   //===Forwards Chain====================================
   dEnX = dEnY = 0.00;                      // increment equivalent chain
   pVx = this;
   while((pVx!=NULL) && ((pSysParent->VxStrNext()==NULL) || (pSysParent->VxAncNext()==NULL))) {
      //---Anchor-------------------------------
      if(pVx->CanAnchor()) {
         pSysParent->SetVxAncNext(pVx);
         break;
      }
      //---Stretch------------------------------
      if((pSysParent->VxStrNext()==NULL) && (pVx->CanStretch(TRUE))) { // either stretch..
         pSysParent->SetVxStrNext(pVx);
      } else {                              //..or add to equivalent (fixed) chain
         dEnX += pVx->Dist2Next() * COS(pVx->SegmentCanvasAngle());
         dEnY += pVx->Dist2Next() * SIN(pVx->SegmentCanvasAngle());
      }
      pVx = pVx->Next();                    // continue looking
   }

   //===Check paths=======================================
   if(pSysParent->VxAncPrev() && (pSysParent->VxStrPrev()==NULL)) return;
   if(pSysParent->VxAncNext() && (pSysParent->VxStrNext()==NULL)) return;

   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   // Store Paths
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   pSysParent->SetVxDrag(this);             // I'm being dragged (others cleared at end of drag)
   //===Full System=======================================
   // If the whole system is selected, we have no anchors
   // and use the EqcPrev R, S values to store the offset
   // from the mouse to the first vertex
   if((pSysParent->VxAncPrev()==NULL) && (pSysParent->VxAncNext()==NULL)) {
      pSysParent->SetEp(
         X() - pSysParent->VxTop()->X(),
         Y() - pSysParent->VxTop()->Y(),
         0.00, 0.00, 0.00);
      return;                               // that's all for this kind of drag
   }

   //===Next Only=========================================
   // Uses some Ep variables for info about top vertex
   if(pSysParent->VxAncNext()) {
      dEnA = pSysParent->VxStrNext()->SegmentCanvasAngle();     // (temp) next stretch canvas angle
      dEnR = SQRT(SQR(dEnX) + SQR(dEnY));                       // next equivalent length
      dEnS = M_PI - dEnA + ATAN2(dEnY, dEnX);                   // next angle equiv-->stretch
      dEnF = pSysParent->VxAncNext()->Prev()->SegmentCanvasAngle() - dEnA;  // angle next anchor incoming-->stretch
      dEnP = pSysParent->VxAncNext()->SegmentCanvasAngle();     // angle CANVAS next anchor segment

      dEpR = SQRT(SQR(dEpX) + SQR(dEpY));                       // prev chain length to top
      dEpS = ATAN2(dEpY, dEpX) - dEnA;                          // angle prev chain-->stretch
      dEpF = pSysParent->VxTop()->SegmentCanvasAngle() - dEnA;  // angle top segment-->stretch
   }

   //===Prev Only=========================================
   if(pSysParent->VxAncPrev()) {
      dEpA = pSysParent->VxStrPrev()->SegmentCanvasAngle();     // (temp) previous stretch canvas angle
      dEpR = SQRT(SQR(dEpX) + SQR(dEpY));                       // length of equivalent
      dEpS = M_PI - dEpA + ATAN2(dEpY, dEpX);                   // angle between equivalent and stretch segment
      dEpF = pSysParent->VxAncPrev()->SegmentCanvasAngle() - dEpA; // angle between equivalent and segment
      dEpP = (pSysParent->VxAncPrev()->Prev()==NULL) ? 0.00 :
            pSysParent->VxAncPrev()->Prev()->SegmentCanvasAngle();   // incoming segment CANVAS angle
   }

   //===Double Anchor=====================================
   // Modifies the values set above as necessary
   if(pSysParent->VxAncPrev() && pSysParent->VxAncNext()) {
      if(pSysParent->VxDrag()->CanAnchor()) {
         dEpK = dEpA - Prev()->SegmentCanvasAngle();
         dEnK = dEnA - SegmentCanvasAngle();

      //===Constrained====================================
      } else {
         dEnR = SQRT(SQR(dEpX + dEnX) + SQR(dEpY + dEnY));
         dEpK = M_PI - dEpA + ((dEnR<=0.00) ? 0.00 : (ATAN2(dEpY + dEnY, dEpX + dEnX)));
         dEnK = pSysParent->VxStrNext()->SegmentCanvasAngle()
                  - pSysParent->VxStrPrev()->SegmentCanvasAngle();
         pSysParent->SetEa(
            SQRT(SQR(pSysParent->VxAncNext()->X() - pSysParent->VxAncPrev()->X())
               + SQR(pSysParent->VxAncNext()->Y() - pSysParent->VxAncPrev()->Y())),
            ATAN2(                      // angle to next segment
               pSysParent->VxAncNext()->Y() - pSysParent->VxAncPrev()->Y(),
               pSysParent->VxAncNext()->X() - pSysParent->VxAncPrev()->X()) );
      }
   }


   //===Set===============================================
   pSysParent->SetEp(dEpR, dEpS, dEpF, dEpP, dEpK);
   pSysParent->SetEn(dEnR, dEnS, dEnF, dEnP, dEnK);
}

/*********************************************************
* MoveVxTo
* Having determined the equivalent chains and anchor ver-
* tices in InitMoveTo, we here calculate the lengths that
* the stretch segments need to become and the angles that
* the anchor segments should absorb.
* Note this is more difficult  in full-polar coordinates,
* unlike rev 4 that used cartesian vertex placements fol-
* lowed by Canvas2Polar. The  advantage here is that only
* those equations genuinely needed (anchor angle, stretch
* length) are modified, keeping rest intact.
*
* We differentiate between the moves
*  - Whole cavity: Trivial offset, separate case in Init
*  - Top of cavity: Includes top optic but no prev anchor
*  - End of cavity: No next anchor
*  - Constrained: Includes both previous and next
* Called from
* <- Renderer2d
*    (who is responsible for calling App()->ScanAll())
*********************************************************/
void CVertex::MoveVxTo(double dX, double dY) {
   double dEpA, dEpL;                       // final prev stretch segment canvas angle, length
   double dEnA, dEnL;                       // final next stretch segment canvas angle, length
   double dA;                               // temp angle for pi wrapping
   double dMouseR,  dMouseA;                // polar coordinates for mouse
   double a, b, c, A, B, C;                 // variables in equation for code below
   double dDet;                             // square root in stretch equation

   if(pSysParent->VxDrag()==NULL) return;   // ignore problems initializing above

   //---Clear values----------------------------
   dEpL = dEnL = dEpA = dEnA = 0.00;

   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   // Full System
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   if((pSysParent->VxAncPrev()==NULL) && (pSysParent->VxAncNext()==NULL)) {
      pSysParent->SetStartPos(
         dX - pSysParent->EpR(),
         dY - pSysParent->EpS());
      return;
   }

   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   // Next Anchor
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   //===End Only==========================================
   // Implicitly, this means the top vertex is included
   if(pSysParent->VxAncNext()!=NULL) {
      //---Mouse coordinates--------------------
      dMouseR = SQRT(SQR(dX-pSysParent->VxAncNext()->X()) + SQR(dY-pSysParent->VxAncNext()->Y()));
      dMouseA = (dX - pSysParent->VxAncNext()->X() == 0.00) ?
         (dY > pSysParent->VxAncNext()->Y()) ? M_PI_2 : -M_PI_2 :
         ATAN2(dY-pSysParent->VxAncNext()->Y(), dX-pSysParent->VxAncNext()->X())
         + M_PI;

      //---Next stretch-------------------------
      a = dMouseR;                               // distance to mouse
      b = pSysParent->EnR();                     // length of fixed
      A = pSysParent->EnS();                     // angle fixed to stretch

      dDet = SQR(b*COS(A)) - (SQR(b) - SQR(a));  // determinant in quadratic equation
      if(dDet < 0.00) return;                    // can't move here, ignore
      c = b*COS(A) + SQRT(dDet);                 // length of stretch
      B = (a==0.00) ? 0.00 : ASIN(b * SIN(A) / a); // angle mouse to stretch

      dEnL = c;                                  // final prev stretch length (if double-constrained ok)
      dEnA = dMouseA + B;
   }

   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   // Previous Anchor
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   if(pSysParent->VxAncPrev()) {
      //---Mouse coordinates--------------------
      dMouseR = SQRT(SQR(dX-pSysParent->VxAncPrev()->X()) + SQR(dY-pSysParent->VxAncPrev()->Y()));
      dMouseA = (dX - pSysParent->VxAncPrev()->X() == 0.00) ?
         (dY > pSysParent->VxAncPrev()->Y()) ? M_PI_2 : -M_PI_2 :
         ATAN2(dY-pSysParent->VxAncPrev()->Y(), dX-pSysParent->VxAncPrev()->X());

      //---Previous stretch---------------------
      a = dMouseR;                               // distance to mouse
      b = pSysParent->EpR();                     // length of fixed
      A = pSysParent->EpS();                     // angle fixed to stretch

      dDet = SQR(b*COS(A)) - (SQR(b) - SQR(a));  // determinant in quadratic equation
      if(dDet < 0.00) return;                    // can't move here, ignore
      c = b*COS(A) + SQRT(dDet);                 // length of stretch
      B = (a==0.00) ? 0.00 : ASIN(b * SIN(A) / a); // angle mouse to stretch

      dEpL = c;                                  // final prev stretch length (if double-constrained ok)
      dEpA = dMouseA + B;                        // final prev stretch CANVAS angle
   }

   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   // Double Anchor
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   if(pSysParent->VxAncPrev() && pSysParent->VxAncNext()) {
      //===Move Anchor====================================
      // no further processing in this case

      //===Constrained====================================
      if(!pSysParent->VxDrag()->CanAnchor()) {
         //---Total Prev------------------------
         // This is the single-prime triangle (a, B re-used below)
         b = pSysParent->EnR();             // combined fixed length
         c = dEpL;                          // previous stretch length
         A = pSysParent->EpK();             // angle prev stretch-->combined fixed
         a = SQRT( SQR(b) + SQR(c) - 2.00*b*c*COS(A) );
         if(a == 0.00) return;              // prevent #DIV/0! errors
         B = ASIN( b * SIN(A) / a);         // angle stretch to total prev
//printf("dEnL=%lg, a'=%lg, b'=%lg, c'=%lg, A'=%lg, B'=%lg -- ", dEnL, a, b, c, A*180/M_PI, B*180/M_PI);
         //---Next Stretch----------------------
         // This is the double-prime triangle
         b = a;                             // total previous from above
         a = pSysParent->EaR();             // distance between anchors
         A = -B - pSysParent->EnK() + M_PI; // from angle between them both

         dDet = SQR(b*COS(A)) - (SQR(b) - SQR(a));  // determinant in quadratic equation
         if(dDet < 0.00) return;            // can't move here, ignore
         c = b*COS(A) + SQRT(dDet);         // length of stretch
         C = (a==0.00) ? 0.00 : -ASIN(c * SIN(A) / a); // angle mouse to stretch

         dEnL = c;                          // length of next stretch segment
//printf("dEpA=%lg, A''=%lg, C''=%lg, a''=%lg, b''=%lg, c''=dEnL=%lg\n",dEpA,A*180/M_PI,C*180/M_PI,a, b, c);

         //---Retrieve angles-------------------
         dEpA = pSysParent->EaA() + C + B;  // C'', B'
         dEnA = (dEpA + pSysParent->EnK());

         //---Check validity--------------------
         /*double dX, dY;
         dX = dY = 0.00;
         dX += dEpL * COS(dEpA);
         dY += dEpL * SIN(dEpA);
         dX += pSysParent->EpR() * COS(dEpA + pSysParent->EpK() - M_PI);
         dY += pSysParent->EpR() * SIN(dEpA + pSysParent->EpK() - M_PI);
         dX += dEnL * COS(dEnA);
         dY += dEnL * SIN(dEnA);
         dA = ATAN2(dY, dX) - pSysParent->EaA();
         while(dA < -M_PI) dA += 2*M_PI;
         while(dA >  M_PI) dA -= 2*M_PI;
         if(fabs(dA) > 0.02) { printf("*"); return; } // */
      }
   }

   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   // Apply
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   if((dEnL <= 0.00) && (pSysParent->VxAncNext())) return;
   if((dEpL <= 0.00) && (pSysParent->VxAncPrev())) return; // lengths must be positive

   //===Previous==========================================
   if(pSysParent->VxAncPrev()) {
      pSysParent->VxStrPrev()->SetDist2Next(dEpL);    // length of stretch segment
      if(pSysParent->VxAncPrev()->Prev()==NULL) {
         dA = dEpA + pSysParent->EpF();               // canvas angle of first segment
         pSysParent->SetRotation(dA * 180.00/M_PI);   // rotate whole system
      } else {
         dA = pSysParent->EpP() - pSysParent->EpF() - dEpA + M_PI; // angle relative to incoming
         while(dA > M_PI) dA -= 2.00*M_PI;
         while(dA <-M_PI) dA += 2.00*M_PI;
         pSysParent->VxAncPrev()->SetFaceAngle(dA/2.00 * 180.00/M_PI);
      }
   }

   //===Next==============================================
   if(pSysParent->VxAncNext()) {
      //---Stretch------------------------------
      pSysParent->VxStrNext()->SetDist2Next(dEnL);    // length of stretch segment

      //---Apply Top (next only)----------------
      if(pSysParent->VxAncPrev()==NULL) {
         pSysParent->SetRotation( (dEnA + pSysParent->EpF()) * 180.00/M_PI );
         pSysParent->SetStartPos(
            dX - pSysParent->EpR() * COS(dEnA + pSysParent->EpS()),
            dY - pSysParent->EpR() * SIN(dEnA + pSysParent->EpS()));
      }

      //---Apply Continued----------------------
      if(pSysParent->VxAncNext()->Next()) {
         dA = pSysParent->EnP() - pSysParent->EnF() - dEnA + M_PI; // angle relative to incoming
         while(dA > M_PI) dA -= 2.00*M_PI;
         while(dA <-M_PI) dA += 2.00*M_PI;
         pSysParent->VxAncNext()->SetFaceAngle(-dA/2.00 * 180.00/M_PI);
      }

   }

   //===Double Anchor=====================================
   if(pSysParent->VxAncPrev() && pSysParent->VxAncNext() && pSysParent->VxDrag()->CanAnchor()) {
      //---Anchor Drag-----------------------
      dA = pSysParent->EnK() - pSysParent->EpK() + dEnA - dEpA + M_PI;
      while(dA > M_PI) dA -= 2.00*M_PI;
      while(dA <-M_PI) dA += 2.00*M_PI;
      pSysParent->VxDrag()->SetFaceAngle(-dA/2.00 * 180.00/M_PI);
   }

}


/*********************************************************
* EndMoveVxTo
* Clear the chains defined by InitMoveTo and complete the
* vertex move.
*********************************************************/
void CVertex::EndMoveVxTo(void) {
   //---Render system---------------------------
   App()->ScanAll();                        // update everything

   //---Clear pointers--------------------------
   pSysParent->SetVxDrag(NULL);             // dragging vertex
   pSysParent->SetVxAncPrev(NULL);          // previous anchor (rotation)
   pSysParent->SetVxStrPrev(NULL);          // previous stretch segment
   pSysParent->SetVxAncNext(NULL);          // next anchor (rotation)
   pSysParent->SetVxStrNext(NULL);          // next stretch segment
}


/*********************************************************
* MoveVxToDraft
* Vertex move in system draft mode.
* Note *: Vertices that have been linked are  locked into
* place and  cannot be moved. This is an interface design
* issue to make the linking more obvious to the user, but
* not a fundamental one (and I'm trying not having it).
* Called from
*  <- CSWin2d::OnMouseMove (Draft Mode)
*  <- CSystem::MenuInsertVx
*********************************************************/
void CVertex::MoveVxToDraft(double dX, double dY) {
   CVertex *pVxMove;                        // first vertex to move
   CVertex *pVx;                            // manual move also trailing objects
   double   dAng;                           // angle on canvas

   //---Lock linked-----------------------------
   //if(CheckBit(CVXF_DRAFTLINK)) return;     // lock down when linked (see note)

   //---Find top of blocks----------------------
   switch(Type()) {
   case CVX_TYPE_SCREEN:
   case CVX_TYPE_THERMALLENS:
   case CVX_TYPE_OUTBREWSTER:
   case CVX_TYPE_OUTCRYSTAL:
   case CVX_TYPE_OUTPLATE:
      if(VxLinked()==NULL) break;           // ignore bad linking
      dAng = VxLinked()->Angle2Next();      // here, this is the Canvas angle of the optic
      for(pVx=this; (pVx) && (pVx!=VxLinked()); ) {
         pVx = (CVertex*) pVx->Prev();
         dX -= pVx->Dist2Next() * COS(dAng);
         dY -= pVx->Dist2Next() * SIN(dAng);
      }
      pVxMove = VxLinked();
      break;
   default:
      pVxMove = this;
   }


   //---This vertex-----------------------------
   pVxMove->SetCanvasPosition(dX, dY, 0.00); // move this vertex

   //---Remaining block-------------------------
   switch(pVxMove->Type()) {
   case CVX_TYPE_INBREWSTER:
   case CVX_TYPE_INCRYSTAL:
   case CVX_TYPE_INPLATE:
      dAng = pVxMove->Angle2Next();         // angle on canvas
      for(pVx=pVxMove->Next(); (pVx!=NULL) && (pVx->VxLinked()==pVxMove); pVx=(CVertex*) pVx->Next()) {
         dX += pVx->Prev()->Dist2Next() * COS(dAng);
         dY += pVx->Prev()->Dist2Next() * SIN(dAng);
         pVx->SetCanvasPosition(dX, dY, 0.00);
      }
      break;
   }
}

