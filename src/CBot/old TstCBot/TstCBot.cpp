// TstCBot.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "TstCBot.h"

#include "MainFrm.h"
#include "TstCBotDoc.h"
#include "TstCBotView.h"
#include "CMyThread.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


////////////////////////////////////////////////////////////////////
// routine show()
// utilisable depuis le programme écrit en CBot

// exécution
BOOL    rShow( CBotVar* pVar, CBotVar* pResult, int& Exception )
{
    CString     s;

    if ( pVar == NULL )
    {
        Exception = 22; return FALSE;
    }

    while ( pVar != NULL )
    {
        CString ss;
        ss.LoadString( TX_TYPENAMES + pVar->RetType() );
        s += ss + " ";

        ss = pVar->RetName();
        if (ss.IsEmpty()) ss = "<sans nom>";
        s += ss + " = ";

        s += pVar->RetValString();
        s += "\n";
        pVar = pVar->RetNext();
    }

    AfxMessageBox(s, MB_OK|MB_ICONINFORMATION);

//  if ( pResult && pResult->RetType() == CBotTypInt) pResult->SetValInt(123);

    return  TRUE;           // pas d'interruption
}

int cShow( CBotVar* &pVar, CBotString& RetClass)
{
    if ( pVar == NULL ) return 22;
    return CBotTypInt;      // tous paramètres acceptés, un entier en retour
}

int cErr( CBotVar* &pVar, CBotString& RetClass)
{
    pVar = pVar->RetNext(); // avance le pointeur sur l'erreur
    return  6666;
}

////////////////////////////////////////////////////////////////////
// routine print()
// utilisable depuis le programme écrit en CBot

// exécution
BOOL    rPrintLn( CBotVar* pVar, CBotVar* pResult, int& Exception )
{
    CString     s;

    CTstCBotApp* pApp = (CTstCBotApp*)AfxGetApp();
    CEdit*       pEdit = pApp->m_pConsole;

    if (pEdit == NULL) return TRUE;
    pEdit->GetWindowText(s);

    while ( pVar != NULL )
    {
        if ( !s.IsEmpty() ) s += "\r\n";
        s += pVar->RetValString();
        pVar = pVar->RetNext();
    }

    pEdit->SetWindowText(s);
    pEdit->SetSel(s.GetLength(), s.GetLength());
    pEdit->SetFocus();
    return  TRUE;           // pas d'interruption
}

BOOL    rPrint( CBotVar* pVar, CBotVar* pResult, int& Exception )
{
    CString     s;

    CTstCBotApp* pApp = (CTstCBotApp*)AfxGetApp();
    CEdit*       pEdit = pApp->m_pConsole;

    if (pEdit == NULL) return TRUE;
    pEdit->GetWindowText(s);

    while ( pVar != NULL )
    {
        if ( !s.IsEmpty() ) s += " ";
        s += pVar->RetValString();
        pVar = pVar->RetNext();
    }

    pEdit->SetWindowText(s);
    pEdit->SetSel(s.GetLength(), s.GetLength());
    pEdit->SetFocus();
    return  TRUE;           // pas d'interruption
}

int cPrint( CBotVar* &pVar, CBotString& RetClass)
{
    return 0;               // tous paramètres acceptés, un entier en retour
}


//////////////////////////////////////////////////////////////////
// class CPoint pour essayer

// exécution
BOOL    rCPoint( CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception )
{
    CString     s;

    if ( pVar == NULL )return TRUE;         // constructeur sans paramètres est ok

    if ( pVar->RetType() > CBotTypDouble )
    {
        Exception = 6023; return FALSE;
    }

    CBotVar*    pX = pThis->RetItem("x");
    if ( pX == NULL )
    {
        Exception = 6024; return FALSE;
    }

    pX->SetValFloat( pVar->RetValFloat() );
    pVar    = pVar->RetNext();

    if ( pVar == NULL )
    {
        Exception = 6022; return FALSE;
    }

    if ( pVar->RetType() > CBotTypDouble )
    {
        Exception = 6023; return FALSE;
    }

    CBotVar*    pY = pThis->RetItem("y");
    if ( pY == NULL )
    {
        Exception = 6024; return FALSE;
    }

    pY->SetValFloat( pVar->RetValFloat() );
    pVar    = pVar->RetNext();

    if ( pVar != NULL )
    {
        Exception = 6025; return FALSE;
    }

    return  TRUE;           // pas d'interruption
}

int cCPoint( CBotVar* pThis, CBotVar* &pVar, CBotString& RetClass)
{
    // l'objet doit être de la classe CPoint
    if ( !pThis->IsElemOfClass("CPoint") ) return 6021;

    // ok si aucun paramètres !
    if ( pVar == NULL ) return 0;

    // paramètre de type numérique svp
    if ( pVar->RetType() > CBotTypDouble ) return 6023;
    pVar    = pVar->RetNext();

    // il doit y avoir un second paramètre
    if ( pVar == NULL ) return 6022;
    // également de type numérique
    if ( pVar->RetType() > CBotTypDouble )return 6023;
    pVar    = pVar->RetNext();

    // et pas plus de 2 paramètres svp
    if ( pVar != NULL ) return 6025;

    return 0;       // cette fonction retourne void
}

// méthode déterminant l'opposé
BOOL    rOppose( CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception )
{
    CString     s;

    if ( pVar != NULL )                             // pas de paramètre
    {
        Exception = 6025; return FALSE;
    }

    CBotVar*    pvar = pThis->RetItemList();        // demande la chaîne des items

    // tous les paramètres sont des nombres
    while (pvar != NULL)
    {
        pvar->SetValFloat( -pvar->RetValFloat() );
        pvar    = pvar->RetNext();
    }

    pResult->Copy(pThis);
    return  TRUE;                           // pas d'interruption
}

int cOppose( CBotVar* pThis, CBotVar* &pVar, CBotString& RetClass)
{
    // l'objet doit être de la classe CPoint
    if ( !pThis->IsElemOfClass("CPoint") ) return 6021;

    RetClass = "CPoint";                                    // l'objet rendu est de cette class

    // ok si aucun paramètres !
    if ( pVar == NULL ) return CBotTypClass;                // le paramètre retourné est une instance de la classe

    return TX_OVERPARAM;        // ça va pas
}


/////////////////////////////////////////////////////////////////////////////
// CTstCBotApp

BEGIN_MESSAGE_MAP(CTstCBotApp, CWinApp)
    //{{AFX_MSG_MAP(CTstCBotApp)
    ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
        // NOTE - the ClassWizard will add and remove mapping macros here.
        //    DO NOT EDIT what you see in these blocks of generated code!
    //}}AFX_MSG_MAP
    // Standard file based document commands
    ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
    ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTstCBotApp construction

CTstCBotApp::CTstCBotApp()
{
    // TODO: add construction code here,
    // Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CTstCBotApp object

CTstCBotApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CTstCBotApp initialization

BOOL CTstCBotApp::InitInstance()
{
    AfxEnableControlContainer();

    // Standard initialization
    // If you are not using these features and wish to reduce the size
    //  of your final executable, you should remove from the following
    //  the specific initialization routines you do not need.

#ifdef _AFXDLL
    Enable3dControls();         // Call this when using MFC in a shared DLL
#else
    Enable3dControlsStatic();   // Call this when linking to MFC statically
#endif

    // Change the registry key under which our settings are stored.
    // You should modify this string to be something appropriate
    // such as the name of your company or organization.
    SetRegistryKey(_T("Local AppWizard-Generated Applications"));

    LoadStdProfileSettings();  // Load standard INI file options (including MRU)

    // Register the application's document templates.  Document templates
    //  serve as the connection between documents, frame windows and views.

    CSingleDocTemplate* pDocTemplate;
    pDocTemplate = new CSingleDocTemplate(
        IDR_MAINFRAME,
        RUNTIME_CLASS(CTstCBotDoc),
        RUNTIME_CLASS(CMainFrame),       // main SDI frame window
        RUNTIME_CLASS(CTstCBotView));
    AddDocTemplate(pDocTemplate);

    // Parse command line for standard shell commands, DDE, file open
    CCommandLineInfo cmdInfo;
    ParseCommandLine(cmdInfo);

    // Dispatch commands specified on the command line
    if (!ProcessShellCommand(cmdInfo))
        return FALSE;

    // The one and only window has been initialized, so show and update it.
    m_pMainWnd->ShowWindow(SW_SHOW);
    m_pMainWnd->UpdateWindow();



///////////////////////////////////
//  défini la fonction "show()"
// --------------------------------

    CBotProgram::AddFunction("show", rShow, cShow);
    CBotProgram::AddFunction("err", rShow, cErr);
    CBotProgram::AddFunction("print", rPrint, cPrint);
    CBotProgram::AddFunction("println", rPrintLn, cPrint);


///////////////////////////////////
// définie la classe globale CPoint
// --------------------------------

    CBotClass*  m_pClassPoint;

    m_pClassPoint   = new CBotClass("CPoint", NULL);
    // ajoute le composant ".x"
    m_pClassPoint->AddItem("x", CBotTypFloat);
    // ajoute le composant ".y"
    m_pClassPoint->AddItem("y", CBotTypFloat);

    // ajoute le constructeur pour cette classe
    m_pClassPoint->AddFunction("CPoint", rCPoint, cCPoint);
    // ajoute la méthode Opposé
    m_pClassPoint->AddFunction("Opposé", rOppose, cOppose);


//////////////////////////////////////////////////////////////////
// compile un bout de programme pour voir s'il est bien accessible
// depuis un autre "module"

    CBotProgram*    p = new CBotProgram;
    CBotStringArray Liste;
    p->Compile(" public void MonProgram( ) { show (\"mon programme\") ;}", Liste );

    // l'objet n'est pas détruit et plus référencé
    // je sais c'est pas bien


    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
    CAboutDlg();

// Dialog Data
    //{{AFX_DATA(CAboutDlg)
    enum { IDD = IDD_ABOUTBOX };
    //}}AFX_DATA

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CAboutDlg)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:
    //{{AFX_MSG(CAboutDlg)
        // No message handlers
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
    //{{AFX_DATA_INIT(CAboutDlg)
    //}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CAboutDlg)
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
    //{{AFX_MSG_MAP(CAboutDlg)
        // No message handlers
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CTstCBotApp::OnAppAbout()
{
    CAboutDlg aboutDlg;
    aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CTstCBotApp commands
