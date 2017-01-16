/****************************************************************************
  FileName     [ cirDef.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic data or var for cir package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_DEF_H
#define CIR_DEF_H

#include <vector>
#include "myHashMap.h"

using namespace std;

// TODO: define your own typedef or enum

typedef vector<unsigned>	IdList;

class CirGate;
class CirMgr;
class SatSolver;

enum opt
{
	X_1, X_0, X_X, X_nX, X_Y
};

class ID
{
public:
	ID(unsigned input = 0): _id(input) {}
	
	size_t operator() () const { return _id; }
	bool operator== (const ID& comp) const { return (_id == comp._id); }
	
private:
	unsigned _id;
};

#endif // CIR_DEF_H
