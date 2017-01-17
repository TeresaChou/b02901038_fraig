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
class StrashKey
{
public:
   StrashKey(): _key(0) {}
   StrashKey(const size_t a, const size_t b) { _key = (a << 20) + b; }

   size_t operator() () const { return _key; }
   bool operator== (const StrashKey& k) const { return (k._key == _key); }

private:
   size_t   _key;
};
 
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
   opt checkOpt() const;
   // bool isMarked() const { return (_markFlag == _markFlagRef); }
	bool floating() const;
	bool unUsed() const { return _fanout.empty(); }
	unsigned faninLiteral(size_t i) const { return _fanin[i].literal(); }
   virtual StrashKey getStrashKey() const = 0;
	
   // Printing functions
   virtual void printGate() const = 0;		// called by print netlist
   void reportGate() const;
   void reportFanin(int level) const;
   void reportFanout(int level) const;
	void setDFSList_RC(IdList& _dfsList) const;

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

   public:
		bool operator == (const CirGateSP& comp) const { return (_gateSP == comp._gateSP); }
      bool operator < (const CirGateSP& comp) const { return (literal() < comp.literal()); }

   private:
		CirGate* gate() const { return (CirGate*)(_gateSP & ~(size_t)NEG); }
		bool isInv() const { return (_gateSP & NEG); }
		bool isFlt() const { return (_gateSP & FLT)/2; }
		unsigned literal() const {
			if(isFlt())	return _gateSP/8;
			return ( gate()->getGateID()*2 + (isInv()? 1: 0) ); 
		}

		size_t	_gateSP;
	};

   // basic helping functions
	virtual void connectLinks() = 0;
	bool addFanout(CirGateSP p);
	void delFanout(CirGateSP p);
	void clearFanin();
	void changeFanin(CirGateSP from, CirGateSP to);

	// optimizing functions
	void mergeInto(CirGate* host);
   void replaceByConst(CirGate* gate, unsigned sign);
   void replaceByFanin(unsigned number);

   // simulating functions
   void feedInput(unsigned input) { _value = input;   _markFlag = _markFlagRef; }
   unsigned getSimValue();

	static size_t		_markFlagRef;
private:
	// for recursive use
	void reportFaninRC(int level, int tab) const;
	void reportFanoutRC(int level, int tab) const;

protected:
	unsigned				_gateID;
	unsigned				_lineNo;
   unsigned          _value;
	mutable size_t		_markFlag;
	vector<CirGateSP>	_fanin;
	vector<CirGateSP>	_fanout;
};


class AigGate	:public CirGate
{
   friend class CirMgr;

public:
	// when first constructed, inputs are all stored as literal IDs
	// member function connectLink() has to be called afterward to link them as pointers
	AigGate(unsigned id, unsigned line, size_t input1, size_t input2)
		: CirGate(id, line) {
      if(input1 < input2){
         _fanin.push_back(CirGateSP(input1));
         _fanin.push_back(CirGateSP(input2));
      }
      else {
         _fanin.push_back(CirGateSP(input2));
         _fanin.push_back(CirGateSP(input1));
      }
	}
	~AigGate() {}

   string getTypeStr() const { return "AIG"; }
	bool isAig() const { return true; }
   StrashKey getStrashKey() const { return StrashKey(_fanin[0]._gateSP, _fanin[1]._gateSP); } 
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
   friend class CirMgr;

public:
	PIGate(unsigned id, unsigned line): CirGate(id, line) {}
	~PIGate() {}

   string getTypeStr() const { return "PI"; }
   StrashKey getStrashKey() const { return StrashKey(); } 
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
   friend class CirMgr;

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
   StrashKey getStrashKey() const { return StrashKey(); } 
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
   friend class CirMgr;

public:
	ConstGate(): CirGate(0, 0) {}
	~ConstGate() {}

   string getTypeStr() const { return "CONST"; }
	bool isAig() const { return false; }
   StrashKey getStrashKey() const { return StrashKey(); } 
	void connectLinks(){}
	void printGate() const { cout << "CONST0" << endl; }
private:
};

#endif // CIR_GATE_H
