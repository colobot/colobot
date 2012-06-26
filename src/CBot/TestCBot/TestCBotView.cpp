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
// * along with this program. If not, see  http://www.gnu.org/licenses/.

// TestCBotView.cpp : implementation of the CTestCBotView class
//

#include "stdafx.h"
#include "TestCBot.h"

#include "TestCBotDoc.h"
#include "TestCBotView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTestCBotView

IMPLEMENT_DYNCREATE(CTestCBotView, CView)

BEGIN_MESSAGE_MAP(CTestCBotView, CView)
    //{{AFX_MSG_MAP(CTestCBotView)
    ON_WM_SIZE()
    ON_MESSAGE(WM_ACTWINDOW, ActWindow)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTestCBotView construction/destruction

CTestCBotView::CTestCBotView()
{
}

CTestCBotView::~CTestCBotView()
{
}

BOOL CTestCBotView::PreCreateWindow(CREATESTRUCT& cs)
{
    return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CTestCBotView drawing

void CTestCBotView::OnDraw(CDC* pDC)
{
    CTestCBotDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
}

/////////////////////////////////////////////////////////////////////////////
// CTestCBotView diagnostics

#ifdef _DEBUG
void CTestCBotView::AssertValid() const
{
    CView::AssertValid();
}

void CTestCBotView::Dump(CDumpContext& dc) const
{
    CView::Dump(dc);
}

CTestCBotDoc* CTestCBotView::GetDocument() // non-debug version is inline
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTestCBotDoc)));
    return (CTestCBotDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTestCBotView message handlers

void CTestCBotView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
    CTestCBotDoc* pDoc = GetDocument();
//  CTestCBotApp* pApp = (CTestCBotApp*)AfxGetApp();

    if ( pDoc->m_pEdit == NULL)
    {
        pDoc->m_pEdit = new CEdit();
        CRect       rect;
        GetClientRect( rect );

        pDoc->m_pEdit->Create( WS_VISIBLE|WS_BORDER|WS_TABSTOP|ES_MULTILINE|ES_WANTRETURN|ES_NOHIDESEL|ES_AUTOVSCROLL,
                         rect, this, IDC_EDIT1 );
        pDoc->m_pEdit->SetTabStops(12);
        pDoc->m_pEdit->SetWindowText(pDoc->m_DocText);
    }

    if ( !bActivate && !pDoc->Compile() )
    {
//      comment faire pour réactiver l'ancien document
    }

    CView::OnActivateView(bActivate, pActivateView, pDeactiveView);

    if ( bActivate ) pDoc->m_pEdit->SetFocus();
}


void CTestCBotView::OnSize(UINT nType, int cx, int cy)
{
    CView::OnSize(nType, cx, cy);

    CTestCBotDoc* pDoc = GetDocument();
    if ( pDoc->m_pEdit != NULL )
    {
        CRect       rect;
        GetClientRect( rect );
        pDoc->m_pEdit->MoveWindow( rect );
        pDoc->m_pEdit->SetFocus();
    }
}



LONG CTestCBotView::ActWindow(UINT wparam, LONG lparam)
{
//  GetParentFrame()->SetActiveView( this, TRUE );
//  CMDIChildWnd::OnMDIActivate(1, this, this)
    return 0;
}
