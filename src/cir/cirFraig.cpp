/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashMap.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed
void
CirMgr::strash()
{
   HashMap<StrashKey, CirGate_p>  strashMap(getHashSize(_A));
   StrashKey key;
   CirGate_p temp, host;
   for(size_t i=0, n = _dfsList.size(); i<n; i++) {
      temp = getGate(_dfsList[i]);
      if(!temp->isAig())   continue;
      key = temp->getStrashKey();
      if(strashMap.query(key, host)) {
         cout << "Strashing: ";
         mergeGate(temp, host);
      }
      else  strashMap.insert(key, temp);
   }
   _floatingList.clear();
   setFloatingList();
}

void
CirMgr::fraig()
{
   SatSolver sat;
   sat.initialize();
   
   genProofModel(sat);

   unsigned newInput = 0;
   FECGrp_p group;
   unsigned* sampleInputs = new unsigned[_I];
   for(size_t i=0; i<_I; i++) sampleInputs[i] = 0;
   while(!_FECList->empty()) {
      group = _FECList->back();
      for(size_t j = 0, m = group->size(); j<m-1; j++)
         for(size_t k = j+1; k<m; k++) {
            if(!proveSat(sat, (*group)[j], (*group)[k], sampleInputs)) {
               mergeGate((*group)[j], (*group)[k]);   break;
            }
            else newInput++;
         }
      delete group;
      _FECList->pop_back();
      if(newInput > 20) {
         simulate(sampleInputs);
         cout << newInput << " patterns simulated." << endl;
         for(size_t i=0; i<_I; i++) sampleInputs[i] = 0;
         newInput = 0;
      }
   }
   delete _FECList;
   _FECList = new vector<FECGrp*>;
   strash();
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/

bool
CirMgr::mergeGate(CirGate* from, CirGate* to)
{
   if(from == to) return false;
	from->mergeInto(to);
   cout << to->getGateID() << " merging " << from->getGateID() << " ..." << endl;
   freeGate(from->getGateID(), from);
	return true;
}

void
CirMgr::genProofModel(SatSolver& sat)
{
   CirGate* temp;
   temp = getGate(0);
   Var c0 = sat.newVar();
   temp->setVar(c0);
   for(size_t i=0; i<_I; i++) {
      temp = getGate(_PIList[i]);
      temp->setVar(sat.newVar());
   }
   for(size_t i=0; i<_A; i++) {
      temp = getGate(_AigList[i]);
      temp->setVar(sat.newVar());
   }
   
   for(size_t i=0; i<_A; i++) {
      temp = getGate(_AigList[i]);
      temp->addClause(sat, c0);
   }
}

void
CirGate::addClause(SatSolver& sat, Var& c0)
{
   Var input1, input2;
   bool inv1, inv2;
   if(_fanin[0].isFlt()) {
      inv1 = false;
      input1 = c0;
   }
   else {
      inv1 = _fanin[0].isInv();
      input1 = _fanin[0].gate()->getVar();
   }
   if(_fanin[1].isFlt()) {
      inv2 = false;
      input2 = c0;
   }
   else {
      inv2 = _fanin[1].isInv();
      input2 = _fanin[1].gate()->getVar();
   }
   sat.addAigCNF(_var, input1, inv1, input2, inv2);
}

bool
CirMgr::proveSat(SatSolver& sat, CirGate* gateA, CirGate* gateB, unsigned* sample)
{
   bool result;
   Var topVar = sat.newVar();
   Var c0 = getGate(0)->getVar();
   if(gateA->getSimValue() == gateB->getSimValue())
      sat.addXorCNF(topVar, gateA->getVar(), true, gateB->getVar(), false);
   else 
      sat.addXorCNF(topVar, gateA->getVar(), true, gateB->getVar(), false);
   
   sat.assumeRelease();
   sat.assumeProperty(c0, false);
   sat.assumeProperty(topVar, true);
   result = sat.assumpSolve();

   cout << "Updating by "<< (result? "SAT": "UNSAT")
        << "  Total FEC group = " << _FECList->size() << endl;
   if(result) {
      CirGate* temp;
      for(size_t i=0; i<_I; i++) {
         temp = getGate(_PIList[i]);
         sample[i] = sample[i] << 1;
         sample[i] += (unsigned)sat.getValue(temp->getVar());
      }
   }
   return result;
}
