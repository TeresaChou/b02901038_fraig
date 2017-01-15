/****************************************************************************
  FileName     [ cirOpt.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed
void
CirMgr::sweep()
{
	bool clear = false;
	while(!clear){
		clear = true;
		for(vector<unsigned>::iterator it=_unUsedList.begin(); it!=_unUsedList.end(); ) {
			if(removeGate(*it)){	
				_unUsedList.erase(it);
				clear = false;
			}
			else it++;
		}
		setUnUsedList(true);
	}
   _unUsedList.clear();
   setUnUsedList();
	_floatingList.clear();
	setFloatingList();
}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
// 1. CONST 1 AND X = X
// 2. CONST 0 AND X
// 3. X AND X
// 4. X AND !X
// remember to update _unUsedList
void
CirMgr::optimize()
{
   CirGate* temp;
      if(input1 < 2) {
         
      }
   }
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/

// for sweeping
// remove unused gates
bool
CirMgr::removeGate(unsigned id)
{
	CirGate* target = getGate(id);
   if(target == 0)   return false;
	if(!target->isAig())	return false;
	target->clearFanin();
   freeGate(id, target);
	cout << "AIG(" << id << ") removed..." << endl;
	return true;
}

bool
CirMgr::mergeGate(unsigned idfrom, unsigned idto)
{
	CirGate* from = getGate(idfrom);
	CirGate* to = getGate(idto);
	if(!(from && to))	return false;
	from->mergeInto(to);
   freeGate(idfrom, from);
	return true;
}

bool
CirMgr::replaceGate(unsigned id, bool con, short number)
{
   CirGate* temp = getGate(id);
   if(temp == 0)  return false;
   temp->replaceBy(con, number);
   freeGate(id, temp);
   return true;
}

bool freeGate(unsigned id, CirGate* target)
{
   removeFromAigList(id);
   _gateList.erase(id);
   delete target;
}

bool
removeFromAigList(unsigned id)
{
	for(vector<unsigned>::iterator it = _AigList.begin(); it!=_AigList.end(); it++)
		if(*it == id)	{
         _AigList.erase(it);
         _A--;
         break;
      }
}

string
CirMgr::getSymb(unsigned id) const
{
   map<unsigned, string>::const_iterator it = _symbolList.find(id);
   if(it == _symbolList.end())	return "";
   return it->second;
}

// run through all the gates to check for gates with floating inputs and gates unused
// store them in two lists
void
CirMgr::setFloatingList(bool AigOnly)
{
	CirGate* temp;
	for(size_t i=0; i<_AigList.size(); i++){
		temp = getGate(_AigList[i]);
		if(temp->floating())	_floatingList.push_back(_AigList[i]);
	}
	if(!AigOnly){
		for(size_t i=0; i<_POList.size(); i++){
			temp = getGate(_POList[i]);
			if(temp->floating())	_floatingList.push_back(_POList[i]);
		}
	}
	::sort(_floatingList.begin(), _floatingList.end());
}

void
CirMgr::setUnUsedList(bool AigOnly)
{
	CirGate* temp;
	if(!AigOnly){
		for(size_t i=0; i<_PIList.size(); i++){
			temp = getGate(_PIList[i]);
			if(temp->unUsed())	_unUsedList.push_back(_PIList[i]);
		}
	}
	for(size_t i=0; i<_AigList.size(); i++){
		temp = getGate(_AigList[i]);
		if(temp->unUsed())	_unUsedList.push_back(_AigList[i]);
	}
	::sort(_unUsedList.begin(), _unUsedList.end());
}
