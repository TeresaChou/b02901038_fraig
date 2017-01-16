/****************************************************************************
  FileName     [ cirOpt.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include <algorithm>
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
// 2. CONST 0 AND X = 0
// 3. X AND X = X
// 4. X AND !X = 0
// remember to update _unUsedList
void
CirMgr::optimize()
{
   CirGate* target;
   for(unsigned i=0, n = _dfsList.size(); i<n; i++) {
      target = getGate(_dfsList[i]);
      if(!target->isAig())   continue;
      opt check = target->checkOpt();
      if(check == X_Y)  continue; 
      if(check == X_1 || check == X_X)  target->replaceByFanin(1);
      else if(check == X_0 || check == X_nX) {
         CirGate* constGate = getGate(0);
         target->replaceByConst(constGate, 0);
      }
      freeGate(_dfsList[i], target);
   }
   _unUsedList.clear();
   setUnUsedList();
	_floatingList.clear();
	setFloatingList();
   _dfsList.clear();
   setDFSList();
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
   if(idfrom == idto) return false;
	CirGate* from = getGate(idfrom);
	CirGate* to = getGate(idto);
	if(!(from && to))	return false;
	from->mergeInto(to);
   cout << "Simplifying: " << idto << " merging " << idfrom << " ..." << endl;
   freeGate(idfrom, from);
	return true;
}

bool
CirMgr::freeGate(unsigned id, CirGate* target)
{
   if(target == 0)   return false;
   removeFromAigList(id);
   _gateList.remove(id);
   delete target;
   return true;
}

bool
CirMgr::removeFromAigList(unsigned id)
{
	for(vector<unsigned>::iterator it = _AigList.begin(); it!=_AigList.end(); it++)
		if(*it == id)	{
         _AigList.erase(it);
         _A--;
         return true;
      }
   return false;
}


