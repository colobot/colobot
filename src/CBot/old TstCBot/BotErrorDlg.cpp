// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see .

#include "stdafx.h"
#include "TstCBot.h"
#include "BotErrorDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBotErrorDlg dialog


CBotErrorDlg::CBotErrorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBotErrorDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBotErrorDlg)
	m_TextProgram = _T("");
	//}}AFX_DATA_INIT
}


void CBotErrorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBotErrorDlg)
	DDX_Control(pDX, IDC_EDIT1, m_eProgram);
	DDX_Control(pDX, IDC_STATIC1, m_sMessage);
	DDX_Text(pDX, IDC_EDIT1, m_TextProgram);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBotErrorDlg, CDialog)
	//{{AFX_MSG_MAP(CBotErrorDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBotErrorDlg message handlers

BOOL CBotErrorDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_sMessage.SetWindowText(m_TextError);
	m_eProgram.SetFocus();
	m_eProgram.SetSel(m_start, m_end);

	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
