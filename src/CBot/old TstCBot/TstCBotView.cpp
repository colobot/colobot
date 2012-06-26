// TstCBotView.cpp : implementation of the CTstCBotView class
//

#include "stdafx.h"
#include "TstCBot.h"

#include "TstCBotDoc.h"
#include "TstCBotView.h"
#include "BotConsoleDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTstCBotView

IMPLEMENT_DYNCREATE(CTstCBotView, CView)

BEGIN_MESSAGE_MAP(CTstCBotView, CView)
    //{{AFX_MSG_MAP(CTstCBotView)
    ON_WM_SIZE()
    ON_COMMAND(ID_CP1, OnCp1)
    ON_COMMAND(ID_EXE, OnExe)
    ON_COMMAND(ID_FILE_SAVE, OnFileSave)
    ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTstCBotView construction/destruction

CTstCBotView::CTstCBotView()
{
    // TODO: add construction code here
    m_pEdit = NULL;
    m_pProg = NULL;
}

CTstCBotView::~CTstCBotView()
{
}

BOOL CTstCBotView::PreCreateWindow(CREATESTRUCT& cs)
{
    // TODO: Modify the Window class or styles here by modifying
    //  the CREATESTRUCT cs

    return CView::PreCreateWindow(cs);
}

void CTstCBotView::OnActivateView( BOOL bActivate, CView* pActivateView, CView* pDeactiveView )
{
    if ( m_pEdit == NULL)
    {
        m_pEdit = new CEdit();
        CRect       rect;
        GetClientRect( rect );

        m_pEdit->Create( WS_VISIBLE|WS_BORDER|WS_TABSTOP|ES_MULTILINE|ES_WANTRETURN|ES_NOHIDESEL|ES_AUTOVSCROLL,
                         rect, this, IDC_EDIT1 );
        m_pEdit->SetTabStops(12);
        LoadEdition("CBotTest.txt");
        m_pEdit->SetFocus();
    }
}


/////////////////////////////////////////////////////////////////////////////
// CTstCBotView drawing

void CTstCBotView::OnDraw(CDC* pDC)
{
    CTstCBotDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);

    // TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CTstCBotView diagnostics

#ifdef _DEBUG
void CTstCBotView::AssertValid() const
{
    CView::AssertValid();
}

void CTstCBotView::Dump(CDumpContext& dc) const
{
    CView::Dump(dc);
}

CTstCBotDoc* CTstCBotView::GetDocument() // non-debug version is inline
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTstCBotDoc)));
    return (CTstCBotDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTstCBotView message handlers

void CTstCBotView::OnSize(UINT nType, int cx, int cy)
{
    CView::OnSize(nType, cx, cy);

    if ( m_pEdit != NULL )
    {
        CRect       rect;
        GetClientRect( rect );
        m_pEdit->MoveWindow( rect );
        m_pEdit->SetFocus();
    }
}

void CTstCBotView::SaveEdition(const char* filename)
{
    CString     program;

    m_pEdit->GetWindowText(program);

    FILE*   pf = fopen(filename, "wb");
    if (pf==NULL) return;

    fputs (program, pf);
    fclose(pf);
}

void CTstCBotView::LoadEdition(const char* filename)
{
    CString     program("{ int x = 10000; while (x > 0) x = x-1; }");

    FILE*   pf = fopen(filename, "r");
    if (pf!=NULL)
    {
        char    buffer[10000];
        program.Empty();

        while (NULL != fgets (buffer, 100000, pf))
        {
            program += buffer;
            program = program.Left(program.GetLength()-1) + "\r\n";
        }

        fclose(pf);
    }

    m_pEdit->SetWindowText(program);
}



// compile le programme
#include <stdio.h>

void CTstCBotView::OnCp1()
{
    CString     program;

    SaveEdition("CBotTest.txt");

    m_pEdit->GetWindowText(program);

    CString         TextError;
    int             code, start, end;

    if ( m_pProg == NULL ) m_pProg = new CBotProgram();

    CTstCBotApp* pApp = (CTstCBotApp*)AfxGetApp();

    if (m_pProg->Compile(program, pApp->m_Liste))
    {
        CString done = "Compilation sans erreur.\nLes fonctions suivantes sont externes:\n";

        for ( int i = 0; i < pApp->m_Liste.RetSize(); i++)
        {
            done += CString(pApp->m_Liste[i]) + "\n";
        }

        AfxMessageBox( done );
    }
    else
    {
        m_pProg->GetError(code, start, end);
        delete m_pProg;
        m_pProg = NULL;

        m_pEdit->SetSel( start, end );
        m_pEdit->SetFocus();                // met en évidence la partie avec problème

        TextError.LoadString( code );
        if (TextError.IsEmpty())
        {
            char    buf[100];
            sprintf(buf, "Erreur numéro %d.", code);
            TextError = buf;
        }
        AfxMessageBox( TextError );
    }

    m_pEdit->SetFocus();
}


//////////////////////////////////////////////////////


void CTstCBotView::OnExe()
{
    CTstCBotApp* pApp = (CTstCBotApp*)AfxGetApp();

    if( m_pProg == NULL)
    {
        AfxMessageBox("Pas de programme compilé !");
        return;
    }

    if( pApp->m_Liste.RetSize() == 0 )
    {
        AfxMessageBox("Aucune fonction marquée \"extern\" !");
        return;
    }



    CBotConsoleDlg dlg;
    dlg.DoModal();              // dialogue pour faire la console

    if ( dlg.m_code>0 )
    {
        CString TextError;

        m_pEdit->SetSel( dlg.m_start, dlg.m_end );
        m_pEdit->SetFocus();                // met en évidence la partie avec problème

        TextError.LoadString( dlg.m_code );
        if (TextError.IsEmpty())
        {
            char    buf[100];
            sprintf(buf, "Erreur numéro %d.", dlg.m_code);
            TextError = buf;
        }
//      AfxMessageBox( TextError );
    }

    m_pEdit->SetFocus();

    return;
}



void CTstCBotView::OnFileSave()
{
    // TODO: Add your command handler code here
    SaveEdition("CBotTest.txt");
}

void CTstCBotView::OnFileSaveAs()
{
    CFileDialog     *pDlg;
    CString         s;

    pDlg = new CFileDialog(FALSE, "TXT", NULL,
                           OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY,
                           "cboxtest|*.txt", this);
    if ( pDlg == NULL )  return;

    if ( pDlg->DoModal() == IDOK )  // choix du fichier ...
    {
        SaveEdition(pDlg->GetPathName());
    }

    delete pDlg;
}

#if 0
void test()
{
    int y,z;

    for (;;);
    for (x = 0; y = 1; z = 3) int q = 6;
    for (int x = 0; int y = 1; int z = 3) int q = 6;
    // pour voir
}
#endif

