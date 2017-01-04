/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include "cirDef.h"
#include "cirMgr.h"
#include "sat.h"

using namespace std;

extern CirMgr *cirMgr;

// TODO: Feel free to define your own classes, variables, or functions.

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
class CirGateSP;
class CirGate
{
public:

	friend class CirMgr;

   CirGate(unsigned id, unsigned line): _gateID(id), _lineNo(line) {}
   virtual ~CirGate() {}

   // Basic access methods
   virtual string getTypeStr() const = 0;
	virtual bool isAig() const = 0;
   unsigned getLineNo() const { return _lineNo; }
	unsigned getGateID() const { return _gateID; }
	
   // Printing functions
   virtual void printGate() const = 0;		// called by print netlist
   void reportGate() const;
   void reportFanin(int level) const;
   void reportFanout(int level) const;
	void DFSPrint(unsigned &order) const;

protected:
	// "signed pointer"
	// store a pointer as size_t
	// if constructed as a pointer, it's a pointer and the last bit is used to show negation
	// if constructed as a size_t, it's the literal ID and marked at the last second bit
	// so that we know it's a floating input
	class CirGateSP
	{
		friend class CirGate;
		friend class AigGate;
		friend class POGate;
		
		#define NEG 0x1
		#define FLT 0x2
		CirGateSP(CirGate* p, size_t sign): _gateSP((size_t)p + sign) {}
		CirGateSP(size_t literal): _gateSP(literal*8 + FLT) {}

		bool operator == (const CirGateSP& comp) const { return (_gateSP == comp._gateSP); }

		CirGate* gate() const { return (CirGate*)(_gateSP & ~(size_t)NEG); }
		bool isInv() const { return (_gateSP & NEG); }
		unsigned literal() const {
			if(isFlt())	return _gateSP/8;
			return ( gate()->getGateID()*2 + (isInv()? 1: 0) ); 
		}
		bool isFlt() const { return (_gateSP & FLT)/2; }

		size_t	_gateSP;
	};

public:
	// other helping functions
	virtual void connectLinks() = 0;
	void clearFanin();
	bool addFanout(CirGateSP p);
	void changeFanin(CirGateSP from, CirGateSP to);
	void delFanout(CirGateSP p);
	void mergeInto(CirGate* host);
	bool floating() const;
	bool unUsed() const { return _fanout.empty(); }
	unsigned faninLiteral(size_t i) const { return _fanin[i].literal(); }

	static size_t		_markFlagRef;
private:
	// for recursive use
	void reportFaninRC(int level, int tab) const;
	void reportFanoutRC(int level, int tab) const;

protected:
	unsigned				_gateID;
	unsigned				_lineNo;
	mutable size_t		_markFlag;
	vector<CirGateSP>	_fanin;
	vector<CirGateSP>	_fanout;
};

class AigGate	:public CirGate
{
public:
	// when first constructed, inputs are all stored as literal IDs
	// member function connectLink() has to be called afterward to link them as pointers
	AigGate(unsigned id, unsigned line, size_t input1, size_t input2)
		: CirGate(id, line) {
		_fanin.push_back(CirGateSP(input1));
		_fanin.push_back(CirGateSP(input2));
	}
	~AigGate() {}

   string getTypeStr() const { return "AIG"; }
	bool isAig() const { return true; }
	// link input nodes with signed pointer
	// if node not found, leave it as literal ID
	void connectLinks(){
		size_t liID = _fanin[0].literal();
		CirGate* temp = cirMgr->getGate(liID/2);
		if(temp){
			_fanin[0] = CirGateSP(temp, liID%2);
			temp->addFanout(CirGateSP(this, liID%2));
		}
		liID = _fanin[1].literal();
		temp = cirMgr->getGate(liID/2);
		if(temp){
			_fanin[1] = CirGateSP(temp, liID%2);
			temp->addFanout(CirGateSP(this, liID%2));
		}
	}
   void printGate() const {
		cout << getTypeStr() << " " << _gateID << " ";
		if(_fanin[0].isFlt()) 
			cout << "*" << (_fanin[0].literal()%2? "!": "") << _fanin[0].literal()/2 << " ";
		else{
			if(_fanin[0].isInv())	cout<<"!";
			cout << _fanin[0].gate()->getGateID() << " ";
		}
		if(_fanin[1].isFlt()) cout << "*" << (_fanin[1].literal()%2? "!": "") << _fanin[1].literal()/2;
		else{
			if(_fanin[1].isInv())	cout<<"!";
			cout << _fanin[1].gate()->getGateID();
		}
		cout << endl;
	}
private:
};


class PIGate :public CirGate
{
public:
	PIGate(unsigned id, unsigned line): CirGate(id, line) {}
	~PIGate() {}

   string getTypeStr() const { return "PI"; }
	bool isAig() const { return false; }
   void printGate() const {
		cout << getTypeStr() << "  " << _gateID;
		if(!cirMgr->getSymb(_gateID).empty())	cout << " (" << cirMgr->getSymb(_gateID) << ")";
		cout << endl;
	}
	void connectLinks() {};
private:
	// PIGate has no fanin but fanouts
};


class POGate :public CirGate
{
public:
	// when first constructed, inputs are all stored as literal IDs
	// member function connectLink() has to be called afterward to link them as pointers
	POGate(unsigned id, unsigned line, size_t input)
		: CirGate(id, line) {
		_fanin.push_back(CirGateSP(input));
	}
	~POGate() {}

   string getTypeStr() const { return "PO"; }
	bool isAig() const { return false; }
	// link input nodes with signed pointer
	// if node not found, leave it as literal ID
	void connectLinks(){
		size_t liID = _fanin[0].literal();
		CirGate* temp = cirMgr->getGate(liID/2);
		if(temp){
			_fanin[0] = CirGateSP(temp, liID%2);
			temp->addFanout(CirGateSP(this, liID%2));
		}
	}
   void printGate() const {
		cout << getTypeStr() << "  " << _gateID << " ";
		if(_fanin[0].isFlt()) cout << "*" << (_fanin[0].literal()%2? "!": "") << _fanin[0].literal()/2;
		else{
			if(_fanin[0].isInv())	cout<<"!";
			cout << _fanin[0].gate()->getGateID();
		}
		if(!cirMgr->getSymb(_gateID).empty())	cout << " (" << cirMgr->getSymb(_gateID) << ")";
		cout << endl;
	}
private:
	// POGate has no fanouts but fanins
};


class ConstGate :public CirGate
{
public:
	ConstGate(): CirGate(0, 0) {}
	~ConstGate() {}

   string getTypeStr() const { return "CONST"; }
	bool isAig() const { return false; }
	void connectLinks(){}
	void printGate() const { cout << "CONST0" << endl; }
private:
};

#endif // CIR_GATE_H
