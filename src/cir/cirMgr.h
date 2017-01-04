/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <map>

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

#include "cirDef.h"

extern CirMgr *cirMgr;

class CirMgr
{
public:
   CirMgr() {}
   ~CirMgr() {} 

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const {
		map<unsigned, CirGate*>::const_iterator it = _gateList.find(gid);
		if(it == _gateList.end())	return 0;
		return it->second;
	}

   // Member functions about circuit construction
   bool readCircuit(const string&);

   // Member functions about circuit optimization
   void sweep();
   void optimize();

   // Member functions about simulation
   void randomSim();
   void fileSim(ifstream&);
   void setSimLog(ofstream *logFile) { _simLog = logFile; }

   // Member functions about fraig
   void strash();
   void printFEC() const;
   void fraig();

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void printFECPairs() const;
   void writeAag(ostream&) const;
   void writeGate(ostream&, CirGate*) const;
   
   // other helping functions
	bool removeGate(unsigned id, bool free = true);
	bool mergeGate(unsigned idfrom, unsigned idto);
	string getSymb(unsigned id) const {
		map<unsigned, string>::const_iterator it = _symbolList.find(id);
		if(it == _symbolList.end())	return "";
		return it->second;
	}
	void setFloatingList(bool AigOnly = false);
	void setUnUsedList(bool AigOnly = false);

private:
	

   ofstream				        *_simLog;
	map<unsigned, CirGate*>		_gateList;
	map<unsigned, string>		_symbolList;
	unsigned							_M, _I, _L, _O, _A;
	IdList							_PIList, _POList, _AigList, _floatingList, _unUsedList;
};

#endif // CIR_MGR_H
