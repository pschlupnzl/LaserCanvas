/*********************************************************
* CChainLink
* A class of links that  can be inherited to objects that
* form a chain. The function  implementations are defined
* in this header file; there is no .cpp file.
*  Member Functions   Return value
* ------------------ --------------
*    CreateAfter()    New element
*    CreateBefore()   New element
*    End()            Last element in chain
*    Top()            First element in chain
*    InsertAfter(el)  New element
*    InsertBefore(el) New element
*    Next()           Next element in chain
*    Prev()           Previous element in chain
*    SetNext(el)      ---
*    SetPrev(el)      ---
*
* Typically, inherit CChainLink as a public inherited ob-
* ject, then use casting to retrieve the object,
*     class MyClass : public CChainLink {
*        // class definition
*     };
*  pObj = (MyClass*) pObj->Next();
*  This works because the object pointers are identical.
*
* $PSchlup 2004-2006 $     $Revision 0.0 $
*****************************************************************************/
#ifndef CCHANLNK_H
#define CCHANLNK_H

#define printf_debug(s)

class CChainLink;

class CChainLink {
private:
   CChainLink *pPrev;
   CChainLink *pNext;
public:
   CChainLink(void) { pPrev = pNext = NULL; printf_debug("CHainLink\n"); }; // constructor
   virtual ~CChainLink() {                  // destructor
      if(pPrev) pPrev->SetNext(pNext);
      if(pNext) pNext->SetPrev(pPrev);
      printf_debug("~ChainLink\n");
   }

   CChainLink* CreateAfter(void) {          // create new and insert after this
      CChainLink *pclNew;
      pclNew = new CChainLink;
      InsertAfter(pclNew);
      return(pclNew);
   }
   CChainLink* CreateBefore(void) {         // create new and instert before this
      CChainLink *pclNew;
      pclNew = new CChainLink;
      InsertBefore(pclNew);
      return(pclNew);
   }
   void InsertAfter(CChainLink *pcl) {      // insert after this in chain
      if(pcl == NULL) return;
      pcl->SetPrev(this);
      pcl->SetNext(pNext);
      if(pNext) pNext->SetPrev(pcl);
      pNext = pcl;
   }
   void InsertBefore(CChainLink *pcl) {     // insert before this in chain
      if(pcl == NULL) return;
      pcl->SetNext(this);
      pcl->SetPrev(pPrev);
      if(pPrev) pPrev->SetNext(pcl);
      pPrev = pcl;
   }

   CChainLink* Top(void) {                  // return top of chain
      CChainLink *pclRet;
      pclRet = this;
      while(pclRet->Prev()) pclRet = pclRet->Prev();
      return(pclRet);
   }
   CChainLink* End(void) {                  // return end of chain
      CChainLink *pclRet;
      pclRet = this;
      while(pclRet->Next()) pclRet = pclRet->Next();
      return(pclRet);
   }

   void        SetNext(CChainLink *pNxt) { pNext = pNxt; };
   void        SetPrev(CChainLink *pPrv) { pPrev = pPrv; };
   CChainLink* Next(void) { return(pNext); };
   CChainLink* Prev(void) { return(pPrev); };
};
#endif//CCHANLNK_H
