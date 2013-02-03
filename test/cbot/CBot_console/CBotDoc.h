/*
 * CBotDoc.h
 *
 *  Created on: 08-08-2012
 *      Author: michal
 */

#pragma once
#ifndef CBOTDOC_H_
#define CBOTDOC_H_

#include "CClass.h"
#include <string>

class CBotDoc {

public:
	CBotDoc(std::string);
	virtual ~CBotDoc();

//	CEdit*			m_pEdit;			// to memorize the text, and display
	CBotProgram*	m_pProg;			// the compiled program
	std::string		m_DocText;
	CBotStringArray	m_Liste;

// Operations

	bool			Compile();

//	virtual bool OnNewDocument();

	void OnRun();
	void OnChangeEdit1();
	void OnTest();

};


#endif /* CBOTDOC_H_ */
