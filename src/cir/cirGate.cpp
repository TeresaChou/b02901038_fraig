/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <sstream>
#include <stdarg.h>
#include <cassert>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirGate::reportGate()", "CirGate::reportFanin()" and
//       "CirGate::reportFanout()" for cir cmds. Feel free to define
//       your own variables and functions.

extern CirMgr *cirMgr;
size_t CirGate::_markFlagRef = 0;

/**************************************/
/*   class CirGate member functions   */
/**************************************/
void
CirGate::reportGate() const
{
	cout << "==================================================" << endl;
	cout << "= " << getTypeStr() << "(" << getGateID() << ")";
	if(!cirMgr->getSymb(_gateID).empty())	cout << "\"" << cirMgr->getSymb(_gateID) << "\"";
	cout << ", line " << getLineNo() << endl;
	cout << "==================================================" << endl;
}

void
CirGate::reportFanin(int& level) const
{
   assert (level >= 0);
	_markFlagRef++;
	reportFaninRC(level, 1);
}

void
CirGate::reportFaninRC(int level, int tab) const
{
	cout << getTypeStr() << " " << _gateID;
	if(_markFlag == _markFlagRef)	{ cout << " (*)" << endl;	return; }
	cout << endl;
	_markFlag = _markFlagRef;	
	if(level != 0){
		for(size_t i=0; i<_fanin.size(); i++){
			for(int j=0; j<tab; j++)	cout << "  ";		// tab for the following one
			if(_fanin[i].isFlt())	cout << "UNDEF " << _fanin[i].literal()/2;
			else {
				if(_fanin[i].isInv())	cout << "!";
				_fanin[i].gate()->reportFaninRC(level-1, tab+1);
			}
		}
	}
}

void
CirGate::reportFanout(int& level) const
{
   assert (level >= 0);
	_markFlagRef++;
	reportFanoutRC(level, 1);
}

void
CirGate::reportFanoutRC(int level, int tab) const
{
	cout << getTypeStr() << " " << _gateID;
	if(_markFlag == _markFlagRef)	{ cout << " (*)" << endl;	return; }
	cout << endl;
	_markFlag = _markFlagRef;	
	if(level != 0){
		for(size_t i=0; i<_fanout.size(); i++){
			for(int j=0; j<tab; j++)	cout << "  ";		// tab for the following one
			if(_fanout[i].isInv())	cout << "!";
			_fanout[i].gate()->reportFanoutRC(level-1, tab+1);
		}
	}
}

void
CirGate::setDFSList_RC(IdList& _dfsList) const
{
	for(size_t i=0; i<_fanin.size(); i++){
		if(!_fanin[i].isFlt()) _fanin[i].gate()->setDFSList_RC(_dfsList);
	}
	if(_markFlag != _markFlagRef){
      _dfsList.push_back(_gateID);
		_markFlag = _markFlagRef;
	}
}


bool
CirGate::addFanout(CirGateSP p)
{
	for(vector<CirGateSP>::iterator it = _fanout.begin(); it!=_fanout.end(); it++)		
		if(*it == p)	return false;
	_fanout.push_back(p);
   ::sort(_fanout.begin(), _fanout.end());
	return true;
}


void
CirGate::delFanout(CirGateSP p){
	for(vector<CirGateSP>::iterator it = _fanout.begin(); it!=_fanout.end(); it++)
		if(*it == p)	{ _fanout.erase(it); break; }
}

// doesn't change the _fanin list itself
void
CirGate::clearFanin() {
	unsigned sign;
	for(vector<CirGateSP>::iterator it = _fanin.begin(); it!=_fanin.end(); it++) {
      if(it->isFlt())   continue;
		sign = (it->isInv()? 1: 0);
		it->gate()->delFanout(CirGateSP(this, sign));
	}
}

// two funcions to change GateSP list
// if from == to, simply remove the item
void
CirGate::changeFanin(CirGateSP from, CirGateSP to){
	for(vector<CirGateSP>::iterator it = _fanin.begin(); it!=_fanin.end(); it++)
		if(*it == from) {
			if(from == to) _fanin.erase(it);
			else *it = to;
			break;
		}
}

bool
CirGate::floating() const {
	for(size_t i=0; i<_fanin.size(); i++)
		if(_fanin[i].isFlt())	return true;
	return false;
}
