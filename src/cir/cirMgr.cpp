/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include <sstream>
#include <algorithm>
#include "cirMgr.h"
#include "cirGate.h"
#include "cirDef.h"
#include "util.h"

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;

enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

static bool
parseError(CirParseError err)
{
   switch (err) {
      case EXTRA_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Extra space character is detected!!" << endl;
         break;
      case MISSING_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing space character!!" << endl;
         break;
      case ILLEGAL_WSPACE: // for non-space white space character
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal white space char(" << errInt
              << ") is detected!!" << endl;
         break;
      case ILLEGAL_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
              << errMsg << "!!" << endl;
         break;
      case ILLEGAL_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
              << errMsg << "\"!!" << endl;
         break;
      case ILLEGAL_SYMBOL_TYPE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal symbol type (" << errMsg << ")!!" << endl;
         break;
      case ILLEGAL_SYMBOL_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Symbolic name contains un-printable char(" << errInt
              << ")!!" << endl;
         break;
      case MISSING_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing " << errMsg << "!!" << endl;
         break;
      case MISSING_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
              << errMsg << "\"!!" << endl;
         break;
      case MISSING_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": A new line is expected here!!" << endl;
         break;
      case MISSING_DEF:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
              << " definition!!" << endl;
         break;
      case CANNOT_INVERTED:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": " << errMsg << " " << errInt << "(" << errInt/2
              << ") cannot be inverted!!" << endl;
         break;
      case MAX_LIT_ID:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
              << endl;
         break;
      case REDEF_GATE:
         cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
              << "\" is redefined, previously defined as "
              << errGate->getTypeStr() << " in line " << errGate->getLineNo()
              << "!!" << endl;
         break;
      case REDEF_SYMBOLIC_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
              << errMsg << errInt << "\" is redefined!!" << endl;
         break;
      case REDEF_CONST:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Cannot redefine const (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_SMALL:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too small (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_BIG:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too big (" << errInt << ")!!" << endl;
         break;
      default: break;
   }
   return false;
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool
CirMgr::readCircuit(const string& fileName)
{
	// open the file
	ifstream input(fileName.c_str());
	if(!input){
		cerr << "Failed to open file " << fileName <<endl;	return false;
	}
	// header
	string str;
	getline(input,str);
	stringstream ss(str);
	ss.ignore(4);
	ss >> _M;	ss.ignore();
	ss >> _I;	ss.ignore();
	ss >> _L;	ss.ignore();
	ss >> _O;	ss.ignore();
	ss >> _A;
	// all the gate are store in map _gateList
	// every type of gate has its own IDList
	// Const 0
	CirGate* c0 = new ConstGate;
	_gateList[0] = c0;
	// PIs
	unsigned line = 1;
	int content;
	for(unsigned i=0; i<_I; i++){
		getline(input,str);	line++;
		ss.str("");	ss.clear();	// I found that this step is neccessary to reset a stringstream
		ss << str;					// only ss.str(str) would not work
		ss >> content;
		CirGate* pi = new PIGate(content/2, line);
		_gateList[content/2] = pi;
		_PIList.push_back(content/2);
	}
	// POs
	for(unsigned i=1; i<=_O; i++){
		getline(input,str);	line++;
		ss.str("");	ss.clear();
		ss << str;
		ss >> content;
		CirGate* po = new POGate(_M+i, line, content);
		_gateList[_M+i] = po;
		_POList.push_back(_M+i);
	}
	// AIGs
	size_t input1, input2;
	for(unsigned i=0; i<_A; i++){
		getline(input,str);	line++;
		ss.str("");	ss.clear();
		ss << str;
		ss >> content;	ss.ignore();
		ss >> input1;	ss.ignore();
		ss >> input2;
		CirGate* aig = new AigGate(content/2, line, input1, input2);
		_gateList[content/2] = aig;
		_AigList.push_back(content/2);
	}
	// when gates are constructed, inputs are stored as literal ID
	// now link them with pointers
	for(unsigned i=0; i<_A; i++) { _gateList[_AigList[i]]->connectLinks(); }
	for(unsigned i=1; i<=_O; i++) {	_gateList[_M+i]->connectLinks();	}
	// set the _floatingList and the _unUsedList
	setFloatingList();
	setUnUsedList();
	// symbols
	// stored in map _symbolList
	char head;
	string name;
	while(getline(input, str)){
		ss.str("");	ss.clear();
		ss << str;
		ss >> head >> content;	ss.ignore();
		ss >> name;
		switch(head){
		case 'i':
			_symbolList[_PIList[content]] = name;	break;
		case 'o':
			_symbolList[_POList[content]] = name;	break;
		}
	}
	input.close();
   return true;
}

/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void
CirMgr::printSummary() const
{
	cout << endl;
	cout << "Circuit Statistics" << endl;
	cout << "==================" << endl;
	cout << "  PI  " << setw(10) << right << _I << endl;
	cout << "  PO  " << setw(10) << right << _O << endl;
	cout << "  AIG " << setw(10) << right << _A << endl;
	cout << "------------------" << endl;
	cout << "  Total" << setw(9) << right << _I+_O+_A << endl;
}

void
CirMgr::printNetlist() const		//TODO: check
{
/*
   cout << endl;
   for (unsigned i = 0, n = _dfsList.size(); i < n; ++i) {
      cout << "[" << i << "] ";
      _dfsList[i]->printGate();
   }
*/
	// PO post order
	unsigned order = 0;
	CirGate::_markFlagRef++;
	cout << endl;
	for(size_t i=0; i<_POList.size(); i++){
		getGate(_POList[i])->DFSPrint(order);	// CirGate member function
	}
}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
   for(size_t i=0; i<_PIList.size(); i++) { cout << " " << _PIList[i]; }
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   for(size_t i=0; i<_POList.size(); i++) { cout << " " << _POList[i]; }
   cout << endl;
}

void
CirMgr::printFloatGates() const
{
	if(!_floatingList.empty()){
		cout << "Gates with floating fanin(s):";
		for(size_t i=0; i<_floatingList.size(); i++)	cout << " " << _floatingList[i];
		cout << endl;
	}
	if(!_unUsedList.empty()){
		cout << "Gates defined but not used  :";
		for(size_t i=0; i<_unUsedList.size(); i++)	cout << " " << _unUsedList[i];
		cout << endl;
	}
}

void
CirMgr::printFECPairs() const
{
}

void
CirMgr::writeAag(ostream& outfile) const
{
	// make a DFS list of Aig gates
	IdList DFSAigList;
	for(size_t i=0; i<_AigList.size(); i++)
		if(!getGate(_AigList[i])->unUsed())	DFSAigList.push_back(_AigList[i]);
	// header
	outfile << "aag " << _M << " " << _I << " " << _L << " " << _O 
			  <<	" " << DFSAigList.size() << endl;
	// PIs POs
	for(size_t i=0; i<_PIList.size(); i++)	outfile << _PIList[i]*2 << endl;
	for(size_t i=0; i<_POList.size(); i++) outfile << getGate(_POList[i])->faninLiteral(0) << endl;
	// Aigs
	CirGate* temp;
	for(size_t i=0; i<DFSAigList.size(); i++){
		temp = getGate(DFSAigList[i]);
		outfile << temp->getGateID()*2;		outfile << " ";
		outfile << temp->faninLiteral(0);	outfile << " ";
		outfile << temp->faninLiteral(1);	outfile << endl;
	}
	// symbols
	string symbol;
	for(size_t i=0; i<_PIList.size(); i++){
		symbol = getSymb(_PIList[i]);
		if(!symbol.empty()) outfile << "i" << i << " " << symbol << endl;
	}
	for(size_t i=0; i<_POList.size(); i++){
		symbol = getSymb(_POList[i]);
		if(!symbol.empty()) outfile << "o" << i << " " << symbol << endl;
	}
	outfile << "c" << endl;
	outfile << "AAG output by Yun (Teresa) Chou" << endl;
}

void
CirMgr::writeGate(ostream& outfile, CirGate *g) const
{
}

