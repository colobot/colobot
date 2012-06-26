// BotErrorDlg.cpp : implementation file
//

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
