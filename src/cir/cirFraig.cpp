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
   HashMap<StrashKey, CirGate*>  strashMap(getHashSize(_A));
   StrashKey key;
   CirGate* temp, host;
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
CirMgr::fraig()
{
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/
