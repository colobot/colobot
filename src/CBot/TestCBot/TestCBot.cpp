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
// * along with this program. If not, see  http://www.gnu.org/licenses/.// TestCBot.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "TestCBot.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "TestCBotDoc.h"
#include "TestCBotView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTestCBotApp

BEGIN_MESSAGE_MAP(CTestCBotApp, CWinApp)
    //{{AFX_MSG_MAP(CTestCBotApp)
    ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
    //}}AFX_MSG_MAP
    // Standard file based document commands
    ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
    ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTestCBotApp construction

CTestCBotApp::CTestCBotApp()
{
    m_pConsole   = NULL;
    m_LastActive = NULL;
    m_pClassPoint= NULL;
}


/////////////////////////////////////////////////////////////////////////////
// The one and only CTestCBotApp object

CTestCBotApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CTestCBotApp initialization

#include "Routines.cpp"


static char BASED_CODE szSection[]    = "Recent File List";
static char BASED_CODE szFilename[]   = "File1";


#include "../ClassFILE.cpp"

// routine pour mettre à jour l'instance de la classe Bot courante
void rMajObject( CBotVar* pThis, void* pUser )
{
    if (!pThis->IsElemOfClass("object"))
        return ;
    CBotVar*    pPos = pThis->GivItem("position");
    CBotVar*    pX = pPos->GivItem("x");
    CBotVar*    pY = pPos->GivItem("y");
    CBotVar*    pZ = pPos->GivItem("z");
//  CBotVar*    pPt = pThis->GivItem("transport");

    CBotString p = pX->GivValString();

//  pX->SetValFloat( pUser == (void*)1 ? (float)12.5 : (float)44.4 );
    pZ->SetValFloat( (float)0 );
    pY->SetValFloat( (float)-3.33 );
    pX->SetValFloat( pX->GivValFloat() + 10 ) ;

//  pX = pThis->GivItem( "xx" );
//  pX->SetValFloat( (float)22 );

    // crée une instance sur une classe object
//  CBotVar* pAutre = CBotVar::Create("autre", CBotTypClass, "object");
//  pAutre->SetUserPtr( (void*)3 );
//  pPt->SetPointer( pAutre );
//  pPt->SetPointer( NULL );
//  delete pAutre;
}


BOOL CTestCBotApp::InitInstance()
{
//////////////////////////////////////////////
//  défini les mots clefs supplémentaires
// -------------------------------------------

    CBotProgram::Init();

//////////////////////////////////////////////
//  défini les fonctions "show()" et "print()"
// -------------------------------------------

    CBotProgram::AddFunction("show", rShow, cShow);
    CBotProgram::AddFunction("print", rPrint, cPrint);
    CBotProgram::AddFunction("println", rPrintLn, cPrint);


///////////////////////////////////
// définie la classe globale CPoint
// --------------------------------

    m_pClassPoint   = new CBotClass("CPoint", NULL);
    // ajoute le composant ".x"
    m_pClassPoint->AddItem("x", CBotTypFloat);
    // ajoute le composant ".y"
    m_pClassPoint->AddItem("y", CBotTypFloat);

    // ajoute le constructeur pour cette classe
    m_pClassPoint->AddFunction("CPoint", rCPoint, cCPoint);

    m_pClassPointIntr   = new CBotClass("point", NULL, TRUE);
    // ajoute le composant ".x"
    m_pClassPointIntr->AddItem("x", CBotTypFloat);
    // ajoute le composant ".y"
    m_pClassPointIntr->AddItem("y", CBotTypFloat);
    // ajoute le composant ".z"
    m_pClassPointIntr->AddItem("z", CBotTypFloat);

    // ajoute le constructeur pour cette classe
    m_pClassPointIntr->AddFunction("point", rCPoint, cCPoint);

    // défini la classe "object"
    CBotClass*  pClassObject = new CBotClass( "object", NULL ) ;
    pClassObject->AddItem( "xx", CBotTypFloat );
    pClassObject->AddItem( "position", CBotTypResult( CBotTypIntrinsic, "point" ) );
    pClassObject->AddItem( "transport", CBotTypResult( CBotTypPointer, "object" ) );
    pClassObject->AddUpdateFunc( rMajObject );

    InitClassFILE();

    AfxEnableControlContainer();

    // Standard initialization

#ifdef _AFXDLL
    Enable3dControls();         // Call this when using MFC in a shared DLL
#else
    Enable3dControlsStatic();   // Call this when linking to MFC statically
#endif

    // Change the registry key under which our settings are stored.
    SetRegistryKey(_T("Local AppWizard-Generated Applications"));

    LoadStdProfileSettings();  // Load standard INI file options (including MRU)

    // Register document templates

    CMultiDocTemplate* pDocTemplate;
    pDocTemplate = new CMultiDocTemplate(
        IDR_TESTCBTYPE,
        RUNTIME_CLASS(CTestCBotDoc),
        RUNTIME_CLASS(CChildFrame), // custom MDI child frame
        RUNTIME_CLASS(CTestCBotView));
    AddDocTemplate(pDocTemplate);

    // create main MDI Frame window
    CMainFrame* pMainFrame = new CMainFrame;
    if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
        return FALSE;
    m_pMainWnd = pMainFrame;

    // Parse command line for standard shell commands, DDE, file open
    CCommandLineInfo cmdInfo;
    ParseCommandLine(cmdInfo);

    if (m_lpCmdLine[0] == 0)
    {
        CString Filename = GetLocalProfileString(szSection, szFilename);
        if (Filename.IsEmpty()) Filename = "TstCbot.txt";
        else OpenDocumentFile(Filename);
    }
    else
        // Dispatch commands specified on the command line
        if (!ProcessShellCommand(cmdInfo))
            return FALSE;
    pMainFrame->ShowWindow(m_nCmdShow);
    pMainFrame->UpdateWindow();


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
void CTestCBotApp::OnAppAbout()
{
    CAboutDlg aboutDlg;
    aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CTestCBotApp commands

int CTestCBotApp::ExitInstance()
{
    delete  m_pFuncFile;

    CBotProgram::Free();
    return CWinApp::ExitInstance();
}
