/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void
CirMgr::randomSim()
{
}

// data member: ofstream* _simLog
void
CirMgr::fileSim(ifstream& patternFile)
{
}

void
// something else
// remember to set const and PIs

void
CirMgr::divideFEC()
{
   vector<FECGrp*>*  newFECList = new vector<FECGrp*>
   FECGrp* oriGrp, newGrp;
   unsigned simVal;
   for(size_t i = 0, n = _FECList.size(); i<n; i++) {
      oriGrp = _FECList->pop_back();
      HashMap<unsigned, FECGrp*> newGrps;
      for(size_t j = 0, m = oriGrp->size(); j<m; j++) {
         simVal = (*oriGrp)[j]->getSimValue();
         if(newGrps.query(simVal, newGrp))   newGrp->push_back((*oriGrp)[j]);
         else {
            newGrp = new FECGrp;
            newGrp->push_back((*oriGrp)[j]);
            newGrps.insert(simVal, newGrp);
         }
      }
      for(HashMap<unsigned, FECGrp*>::iterator it = newGrps.begin(); it != newGrps.end(); it++) {
         if(it->second->size() > 1) newFECList.push_back(it->second);
         else delete it->second;
      }
      delete oriGrp;
   }
   delete _FECList;
   _FECList = newFECList;
}

unsigned
CirGate::getSimValue()
{
   if(_markFlag == _markFlagRef) return _value;
   // in case that I forget to set CONST 0
   if(getTypeStr() == "CONST")   _value = 0;
   // for PO
   else if(!isAig()) _value = _fanin[0].gate()->getSimValue();
   // for Aig
   else {
      unsigned input1, input2;
      if(_fanin[0].isFlt())   input1 = 0;
      else {
         input1 = _fanin[0].gate()->getSimValue();
         if(_fanin[0].isInv())   input1 = ~input1;
      }
      if(_fanin[1].isFlt())   input2 = 0;
      else {
         input2 = _fanin[1].gate()->getSimValue();
         if(_fanin[1].isInv())   input2 = ~input2;
      }
      _value = input1 & input2;
   }
    _markFlag = _markFlagRef;
   return _value;
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/
