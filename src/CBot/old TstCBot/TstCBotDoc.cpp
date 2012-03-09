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

#include "TstCBotDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTstCBotDoc

IMPLEMENT_DYNCREATE(CTstCBotDoc, CDocument)

BEGIN_MESSAGE_MAP(CTstCBotDoc, CDocument)
	//{{AFX_MSG_MAP(CTstCBotDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTstCBotDoc construction/destruction

CTstCBotDoc::CTstCBotDoc()
{
	// TODO: add one-time construction code here

}

CTstCBotDoc::~CTstCBotDoc()
{
}

BOOL CTstCBotDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CTstCBotDoc serialization

void CTstCBotDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CTstCBotDoc diagnostics

#ifdef _DEBUG
void CTstCBotDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CTstCBotDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTstCBotDoc commands

