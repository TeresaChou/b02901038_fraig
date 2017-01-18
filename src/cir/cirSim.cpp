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
#include <string>
#include <sstream>
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
   unsigned * inputs = new unsigned[_I];
   unsigned count = 0, fail = 0;
   while(!_FECReady || fail < 30) {
      for(size_t i=0; i<_I; i++) inputs[i] = rnGen(INT_MAX);
      if(simulate(inputs))   fail = 0;
      else fail++;
      writeSimLog(inputs);
      count++;
   }
   cout << "MAX_FAILS = " << fail << endl
        << count*sizeof(unsigned)*8 << "patterns simulated.";
}

void
CirMgr::fileSim(ifstream& patternFile)
{
   unsigned* inputs = new unsigned[_I];
   char* buffer = new char[_I];
   for(size_t i=0; i<_I; i++) inputs[i] = 0;
	string str;
	stringstream ss(str);
   unsigned line = 0, digit = 0;
   bool done = false, get;
	while(!done) {
      if(!getline(patternFile,str)) done = true;
      else {
         get = true;
         if(str.length() != _I) {
            cout << "Error: Pattern(" << str << ") legnth(" << str.length() << ") "
                 << "does not match the number of inputs(" << _I << ") in the circuit!!" << endl;
            get = false;
         }
         else 
            for(size_t i=0; i<_I; i++)
               if(str[i] != '0' && str[i] != '1') {
                  cout << "Error: Pattern(" << str << ") contains non-0/1 character('"
                       << str[i] << "')." << endl;
                  get = false;
               }
         if(get) {
            line++;
            ss.str(""); ss.clear();
            ss << str;
            for(size_t i=0; i<_I; i++) {
               ss >> buffer[i];
               inputs[i] += ((unsigned)buffer[i]-48) << digit;
            }
            digit++;
         }
      }
      if(digit >= (sizeof(unsigned)*8) || done) {
         simulate(inputs);
         writeSimLog(inputs, digit);
         for(size_t i=0; i<_I; i++) inputs[i] = 0;
         digit = 0;
      }
   }
   cout << line << " patterns simulated." << endl;
   delete inputs;
   delete buffer;
}

bool
CirMgr::simulate(unsigned* inputs)
{
   CirGate::_markFlagRef++;
   CirGate* temp;
   for(size_t i=0; i<_I; i++) {
      temp = getGate(_PIList[i]);
      temp->feedInput(inputs[i]);
  }
  return divideFEC();
}

bool
CirMgr::divideFEC()
{
   bool divided;
   vector<FECGrp*>*  newFECList = new vector<FECGrp*>;
   FECGrp_p oriGrp, newGrp;
   // for some reason I just have to use FECGrp_p instead of FECGrp*
   // otherwise the compiler doesn't let me use them as parameter for function calling
   unsigned simVal, grpInc = 0;
   _FECReady = true;
   for(size_t i = 0, n = _FECList->size(); i<n; i++) {
      oriGrp = (*_FECList)[i];
      size_t number = oriGrp->size();
      // check if the group can be divided
      simVal = (*oriGrp)[0]->getSimValue();
      divided = false;
      for(size_t j = 1; j<number; j++)
         if((*oriGrp)[j]->getSimValue() != simVal) { divided = true; break; }
      if(grpInc > 0) grpInc++; // so it will only equals to 1 for one time
      if(divided) {
         grpInc++;
         HashMap<ID, FECGrp*> newGrps(getHashSize(number));
         for(size_t j = 0; j<number; j++) {
            simVal = (*oriGrp)[j]->getSimValue();
            if(newGrps.query(simVal, newGrp))   newGrp->push_back((*oriGrp)[j]);
            else {
               newGrp = new FECGrp;
               newGrp->push_back((*oriGrp)[j]);
               newGrps.insert(simVal, newGrp);
            }
         }
         for(HashMap<ID, FECGrp*>::iterator it = newGrps.begin(); it != newGrps.end(); it++) {
            if((*it).second->size() > 3) _FECReady = false;
            if((*it).second->size() > 1) newFECList->push_back((*it).second);
            else delete (*it).second;
         }
         delete oriGrp;
      }
      else if(grpInc > 1)  newFECList->push_back(oriGrp);
      if(grpInc == 1)   
         for(size_t k = 0; k<i; k++) {
            oriGrp = (*_FECList)[k];
            newFECList->push_back(oriGrp);
         }
   }
   if(grpInc) {
      delete _FECList;
      _FECList = newFECList;
   }
   else  delete newFECList;
   cout << "Total FEC Group = " << _FECList->size() << endl;
   return (grpInc > 0);
}

bool
CirMgr::writeSimLog(unsigned* inputs, unsigned n)
{
   if(!_simLog)   return false;
   unsigned* outputs = new unsigned[_O];
   for(size_t i=0; i<_O; i++) outputs[i] = getGate(_POList[i])->getSimValue();
   for(size_t i=0; i<n; i++) {
      for(size_t j=0; j<_I; j++) {
         (*_simLog) << (inputs[j] & 1);
         inputs[j] = inputs[j] >> 1;
      }
      (*_simLog) << ' ';
      for(size_t j=0; j<_O; j++) {
         (*_simLog) << (outputs[j] & 1);
         outputs[j] = outputs[j] >> 1;
      }
      (*_simLog) << '\n';
   }
   return true;
}

unsigned
CirGate::getSimValue()
{
   if(_markFlag == _markFlagRef) {
   	return _value;
   }
   if(getTypeStr() == "CONST")  {}
   // for PO
   else if(!isAig()) {
   	_value = _fanin[0].gate()->getSimValue();
   	if(_fanin[0].isInv())	_value = ~_value;
   }
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
