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
// * along with this program. If not, see  http://www.gnu.org/licenses/.// TestCBotDoc.cpp : implementation of the CTestCBotDoc class
//

#include "stdafx.h"
#include "TestCBot.h"

#include "TestCBotDoc.h"
#include "TestCBotView.h"
#include "CBotConsoleDlg.h"
#include "PerformDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTestCBotDoc

IMPLEMENT_DYNCREATE(CTestCBotDoc, CDocument)

BEGIN_MESSAGE_MAP(CTestCBotDoc, CDocument)
    //{{AFX_MSG_MAP(CTestCBotDoc)
    ON_COMMAND(ID_RUN, OnRun)
    ON_EN_CHANGE(IDC_EDIT1, OnChangeEdit1)
    ON_COMMAND(ID_TEST, OnTest)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTestCBotDoc construction/destruction

static BOOL test = FALSE;


CTestCBotDoc::CTestCBotDoc()
{
    m_pEdit     = NULL;
    m_pProg     = NULL;
    m_bModified = FALSE;
}

CTestCBotDoc::~CTestCBotDoc()
{
    delete  m_pEdit;
    delete  m_pProg;
}

BOOL CTestCBotDoc::OnNewDocument()
{
    if (!CDocument::OnNewDocument())
        return FALSE;

    return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CTestCBotDoc serialization

void CTestCBotDoc::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        m_pEdit->GetWindowText(m_DocText);
        int     w = m_DocText.GetLength();
        ar.Write((LPCTSTR)m_DocText, w);
    }
    else
    {
        int     r;
        char    buf[10001];

        r = ar.Read(buf, 10000);
        buf[r] = 0;
        m_DocText = buf;

        if ( m_pProg == NULL ) m_pProg = new CBotProgram();

        if (!m_pProg->Compile(m_DocText, m_Liste, NULL))
        {
            delete m_pProg;
            m_pProg = NULL;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// CTestCBotDoc diagnostics

#ifdef _DEBUG
void CTestCBotDoc::AssertValid() const
{
    CDocument::AssertValid();
}

void CTestCBotDoc::Dump(CDumpContext& dc) const
{
    CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTestCBotDoc commands

void CTestCBotDoc::OnRun()
{
    OnFileSave();

    m_pEdit->GetWindowText(m_DocText);

    CString         TextError;
    int             code, start, end;

    if ( m_pProg == NULL ) m_pProg = new CBotProgram();

    CTestCBotApp* pApp = (CTestCBotApp*)AfxGetApp();

    if (!m_pProg->Compile(m_DocText, m_Liste, NULL))
    {
        m_pProg->GetError(code, start, end);
        delete m_pProg;
        m_pProg = NULL;

        m_pEdit->SetSel( start, end );
        m_pEdit->SetFocus();                // met en évidence la partie avec problème

        TextError = CBotProgram::GivErrorText( code );
        AfxMessageBox( TextError );

        m_pEdit->SetFocus();
        return;
    }

    if( m_Liste.GivSize() == 0 )
    {
        AfxMessageBox("Aucune fonction marquée \"extern\" !");
        return;
    }

    for ( int i = 0; i < m_Liste.GivSize(); i++ )
    {
        int start, stop;
        m_pProg->GetPosition(m_Liste[i], start, stop, GetPosNom, GetPosParam);
        m_Liste[i] = m_DocText.Mid( start, stop-start );
    }

    CBotConsoleDlg dlg;
    dlg.m_pListe = &m_Liste;
    dlg.m_pEditx = m_pEdit;

    dlg.DoModal();              // dialogue pour faire la console

    if ( dlg.m_code>0 )
    {
        CString TextError;

        TextError = m_pProg->GivErrorText( dlg.m_code );

        m_pEdit->SetSel( dlg.m_start, dlg.m_end );
        m_pEdit->SetFocus();                // met en évidence la partie avec problème

        AfxMessageBox(TextError);
    }

    m_pEdit->SetFocus();

    return;
}


void CTestCBotDoc::OnChangeEdit1()
{
    SetModifiedFlag();
    m_bModified = TRUE;
}

BOOL CTestCBotDoc::Compile()
{
    m_pEdit->GetWindowText(m_DocText);

    CString         TextError;
    int             code, start, end;

    if ( m_pProg == NULL ) m_pProg = new CBotProgram();

    char    buffer[100];
    strcpy(buffer, "le pointeur à passer pour voir");

    if (m_bModified && !m_pProg->Compile(m_DocText, m_Liste, (void*)buffer))
    {
        m_pProg->GetError(code, start, end);
        delete m_pProg;
        m_pProg = NULL;

        m_pEdit->SetSel( start, end );
        m_pEdit->SetFocus();                // met en évidence la partie avec problème

        TextError = CBotProgram::GivErrorText( code );
        AfxMessageBox( TextError );

        m_pEdit->SetFocus();
        m_bModified = FALSE;
        return FALSE;
    }

    if ( m_pProg->GetPosition( "TheTest", start, end) )
    {
        m_pEdit->SetSel( start, end );
        m_pEdit->SetFocus();                // met en évidence la partie avec problème
    }

    m_bModified = FALSE;
    return TRUE;
}



static int compt = 0;
// routine retournant le "pointeur" à un autre object
BOOL rRetObject( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
    pResult->SetPointer( NULL );
    compt+=45671;
    if (compt&0x11) return TRUE;

    CBotVar* pAutre = CBotVar::Create("autre", CBotTypResult( CBotTypClass, "object" ));
    pAutre->SetUserPtr( (void*)2 );
    pResult->SetPointer( pAutre );

    if (!pResult->IsElemOfClass("object"))
        return TRUE;

    delete pAutre;
    return TRUE;
}

CBotTypResult cRetObject( CBotVar* &pVar, void* pUser )
{
    return CBotTypResult( CBotTypPointer, "object");
}

BOOL roRadar( CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception )
{
    pResult->SetPointer( NULL );
    compt+=45671;
    if (compt&0x11) return TRUE;

    CBotVar* pAutre = CBotVar::Create("autre", CBotTypResult( CBotTypClass, "object" ));
    pAutre->SetUserPtr( (void*)2 );
    pResult->SetPointer( pAutre );

    if (!pResult->IsElemOfClass("object"))
        return TRUE;

    delete pAutre;
    return TRUE;
}

CBotTypResult coRadar( CBotVar* pThis, CBotVar* &pVar )
{
    void*   pUser = pThis->GivUserPtr();
    return CBotTypResult( CBotTypPointer, "object");
}

BOOL rMove( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
    if ( test < 12 )
    {
        test++;
        return FALSE;
    }
    return TRUE;
}

CBotTypResult cMove( CBotVar* &pVar, void* pUser )
{
    return CBotTypResult( 0 );
}

BOOL rTurn( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
    return TRUE;
}

CBotTypResult cTurn( CBotVar* &pVar, void* pUser )
{
    return CBotTypResult( 0 );
}

BOOL rRadar( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
    pResult->SetPointer( NULL );

    if ( pVar ) pVar->debug();

    compt+=45671;
    if (compt&0x11)
    {
        return FALSE; // TRUE;
    }

    CBotVar* pAutre = CBotVar::Create("autre", CBotTypResult( CBotTypClass, "object" ));
    pAutre->SetUserPtr( (void*)2 );
    pResult->SetPointer( pAutre );

    if (!pResult->IsElemOfClass("object"))
        return TRUE;

    delete pAutre;
    return TRUE;
}

CBotTypResult cRadar( CBotVar* &pVar, void* pUser )
{
    return CBotTypResult( CBotTypPointer, "object");
}

// routine retournant le "pointeur" à un autre object
BOOL rTEST( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
    test = 1 ;
    if ( pVar == NULL ) return TRUE;

    test = pVar->GivValInt();
    if ( test == 5 )
    {
        pVar = pVar->GivNext();
        pVar->SetUserPtr( OBJECTDELETED );
    }
    return TRUE;
}

CBotTypResult cTEST( CBotVar* &pVar, void* pUser )
{
    return CBotTypResult( 0 );
}

// routine retournant le "pointeur" à un autre object
BOOL rF( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
    if ( pResult == NULL )  return TRUE;
    pResult->SetValInt(3);
    return TRUE;
}

CBotTypResult cF( CBotVar* &pVar, void* pUser )
{
    return CBotTypResult( CBotTypFloat );
}

/////////////////////////////////////////////////////////////////

// Compilation d'une procédure avec un "point".

CBotTypResult   cPoint(CBotVar* &var, void* user)
{
    if ( var == 0 )  return CBotTypResult( CBotErrLowParam );

    if ( var->GivType() <= CBotTypDouble )
    {
        var = var->GivNext();
        if ( var == 0 )  return CBotTypResult( CBotErrLowParam );
        if ( var->GivType() > CBotTypDouble )  return CBotTypResult( CBotErrBadNum );
        var = var->GivNext();
        if ( var == 0 )  return CBotTypResult( CBotErrLowParam );
        if ( var->GivType() > CBotTypDouble )  return CBotTypResult( CBotErrBadNum );
        var = var->GivNext();
        return  CBotTypResult( 0 );
    }

    if ( var->GivType() == CBotTypClass )
    {
        if ( !var->IsElemOfClass("point") )  return CBotTypResult( CBotErrBadParam );
        var = var->GivNext();
        return CBotTypResult( 0 );
    }

    return CBotTypResult( CBotErrBadParam );
}

// Donne un paramètre de type "point".
#define UNIT    1


CBotTypResult   cSpace(CBotVar* &var, void* user)
{
    CBotTypResult       ret;

    if ( var == 0 )  return CBotTypResult( CBotTypIntrinsic, "point" );
    ret = cPoint(var, user);
    if ( !ret.Eq(0) )  return ret;

    if ( var == 0 )  return CBotTypIntrinsic;
    if ( var->GivType() > CBotTypDouble )  return CBotTypResult( CBotErrBadNum );
    var = var->GivNext();

    if ( var == 0 )  return CBotTypIntrinsic;
    if ( var->GivType() > CBotTypDouble )  return CBotTypResult( CBotErrBadNum );
    var = var->GivNext();

    if ( var == 0 )  return CBotTypIntrinsic;
    if ( var->GivType() > CBotTypDouble )  return CBotTypResult( CBotErrBadNum );
    var = var->GivNext();

    if ( var != 0 )  return CBotErrOverParam;
    return CBotTypResult( CBotTypIntrinsic, "point" );
}

// Instruction "space(center, rMin, rMax, dist)".

BOOL rSpace(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CBotVar*    pSub;
    float       rMin, rMax, dist;

    rMin =  5.0f*UNIT;
    rMax = 50.0f*UNIT;
    dist =  4.0f*UNIT;

    if ( var == 0 )
    {
//      center = pThis->RetPosition(0);
    }
    else
    {
        if ( var != 0 )
        {
            rMin = var->GivValFloat()*UNIT;
            var = var->GivNext();

            if ( var != 0 )
            {
                rMax = var->GivValFloat()*UNIT;
                var = var->GivNext();

                if ( var != 0 )
                {
                    dist = var->GivValFloat()*UNIT;
                    var = var->GivNext();
                }
            }
        }
    }

    if ( result != 0 )
    {
        pSub = result->GivItemList();
        if ( pSub != 0 )
        {
            pSub->SetValFloat(1);
            pSub = pSub->GivNext();  // "y"
            pSub->SetValFloat(2);
            pSub = pSub->GivNext();  // "z"
//          pSub->SetValFloat(3);
        }
    }
    return TRUE;
}
//////////////////////////////////////////////////////////////


void CTestCBotDoc::OnTest()
{
    CBotProgram::DefineNum("WingedGrabber", 1);
    CBotProgram::DefineNum("TrackedGrabber", 2);
    CBotProgram::DefineNum("WheeledGrabber", 3);
    CBotProgram::DefineNum("LeggedGrabber", 4);
    CBotProgram::DefineNum("WingedShooter", 5);
    CBotProgram::DefineNum("TrackedShooter", 6);
    CBotProgram::DefineNum("WheeledShooter", 7);
    CBotProgram::DefineNum("LeggedShooter", 8);
    CBotProgram::DefineNum("WingedOrgaShooter", 9);
    CBotProgram::DefineNum("TrackedOrgaShooter", 10);
    CBotProgram::DefineNum("WheeledOrgaShooter", 11);
    CBotProgram::DefineNum("LeggedOrgaShooter", 12);
    CBotProgram::DefineNum("WingedSniffer", 13);
    CBotProgram::DefineNum("TrackedSniffer", 14);
    CBotProgram::DefineNum("WheeledSniffer", 14);
    CBotProgram::DefineNum("LeggedSniffer", 15);
    CBotProgram::DefineNum("Thumper", 16);
    CBotProgram::DefineNum("PhazerShooter", 17);
    CBotProgram::DefineNum("Recycler", 18);
    CBotProgram::DefineNum("Shielder", 19);
    CBotProgram::DefineNum("Subber", 20);
    CBotProgram::DefineNum("Me", 21);

    CBotProgram::DefineNum("TypeMarkPath", 111);

    OnFileSave();

//  CPerformDlg dlg;
//  dlg.m_Script = m_DocText;
//  dlg.DoModal();

    // défini la routine RetObject
    CBotProgram::AddFunction( "Radar", rRetObject, cRetObject );

    // ajoute une routine pour cette classe
    CBotProgram::AddFunction("Space", rSpace, cSpace);

    // défini la routine Test
    CBotProgram::AddFunction( "TEST", rTEST, cTEST );
    CBotProgram::AddFunction( "F", rF, cF );

    CBotProgram::AddFunction( "goto", rMove, cMove );
    CBotProgram::AddFunction( "fire", rTurn, cTurn );
    CBotProgram::AddFunction( "radar", rRadar, cRadar );

    // crée une instance de la classe "Bot" pour ce robot
    CBotVar*    pThisRobot = CBotVar::Create( "", CBotTypResult(CBotTypClass, "object") );
    pThisRobot->SetUserPtr( (void*)1 );
    pThisRobot->SetIdent( 1234 );

    delete m_pProg;
    // crée un objet programme associé à cette instance
    m_pProg = new CBotProgram(pThisRobot);

    // compile le programme
    CString         TextError;
    int             code, start, end;

    m_pEdit->GetWindowText(m_DocText);
    if (!m_pProg->Compile(m_DocText, m_Liste, (void*) 44))
    {
        m_pProg->GetError(code, start, end);
        delete m_pProg;
        m_pProg = NULL;

        delete pThisRobot;

        m_pEdit->SetSel( start, end );
        m_pEdit->SetFocus();                // met en évidence la partie avec problème

        TextError = CBotProgram::GivErrorText( code );
        AfxMessageBox( TextError );

        m_pEdit->SetFocus();
        return;
    }

    // exécute pour voir
    m_pProg->Start(m_Liste[0]);

    int mode = -1;

    if ( mode >= 0 ) {

    // sauve et restore à chaque pas possible
    while (!m_pProg->Run(NULL, 1))
    {
        const char* FunctionName;
        int         start1, end1;
        m_pProg->GetRunPos(FunctionName, start1, end1);
        if ( end1 <= 0 )
            m_pProg->GetRunPos(FunctionName, start1, end1);
        m_pEdit->SetSel(start1, end1);

if ( mode == 0 ) continue;

        FILE*   pf;
        pf =    fOpen( "TEST.CBO", "wb" );
        CBotClass::SaveStaticState(pf);
        m_pProg->SaveState(pf);
        fClose(pf);

if ( mode == 2 ) if (!m_pProg->Compile(m_DocText, m_Liste, (void*) 44))
    {
        m_pProg->GetError(code, start, end);
        delete m_pProg;
        m_pProg = NULL;

        delete pThisRobot;

        m_pEdit->SetSel( start, end );
        m_pEdit->SetFocus();                // met en évidence la partie avec problème

        TextError = CBotProgram::GivErrorText( code );
        AfxMessageBox( TextError );

        m_pEdit->SetFocus();
        return;
    }

        pf =    fOpen( "TEST.CBO", "rb" );
        CBotClass::RestoreStaticState(pf);
        m_pProg->RestoreState(pf);
        fClose(pf);

        int         start2, end2;
        m_pProg->GetRunPos(FunctionName, start2, end2);
        if ( end2 <= 0 )
            m_pProg->GetRunPos(FunctionName, start2, end2);

        if ( start1 != start2 || end1 != end2 )
            m_pProg->GetRunPos(FunctionName, start2, end2);
        m_pEdit->SetSel(start2, end2);
    }

    if (m_pProg->GetError(code, start, end))
    {
        m_pEdit->SetSel(start, end);
        TextError = CBotProgram::GivErrorText(code);
        AfxMessageBox(TextError);
    }
    return;}

    while (!m_pProg->Run(NULL, 0))
    {
        const char* FunctionName;
        int         start, end;
        m_pProg->GetRunPos(FunctionName, start, end);
        m_pEdit->SetSel(start, end);

        if ( FunctionName == NULL ) continue;
        CString info (FunctionName);
        CString sep  (":\n");

        int level = 0;
        const char* Name;
        while ( TRUE )
        {
            CBotVar* pVar = m_pProg->GivStackVars(Name, level--);
            if ( Name != FunctionName ) break;
            if ( pVar == NULL ) continue;
//          pVar->Maj(NULL, FALSE);
            while ( pVar != NULL )
            {
                info += sep;
                info += pVar->GivName() + CBotString(" = ") + pVar->GivValString();
                sep = ", ";
                pVar = pVar->GivNext();
            }
            sep = "\n";
        }
        if ( IDOK != AfxMessageBox(info, MB_OKCANCEL) ) break;

        if ( test == 1 )
        {
            test = 0;
            FILE*   pf;
            pf =    fOpen( "TEST.CBO", "wb" );
            m_pProg->SaveState(pf);
            fClose(pf);
        }

        if ( test == 2 )
        {
            test = 0;
            FILE*   pf;
            pf =    fOpen( "TEST.CBO", "rb" );
            m_pProg->RestoreState(pf);
            fClose(pf);
        }

        if ( test == 12 )
        {
            test = 0;
            FILE*   pf;
            pf =    fOpen( "TEST.CBO", "wb" );
            m_pProg->SaveState(pf);
            fClose(pf);

            pf =    fOpen( "TEST.CBO", "rb" );
            m_pProg->RestoreState(pf);
            fClose(pf);

            test = 13;
        }
    }

    if (m_pProg->GetError(code, start, end))
    {
        m_pEdit->SetSel(start, end);
        TextError = CBotProgram::GivErrorText(code);
        AfxMessageBox(TextError);
    }

    delete pThisRobot;
}

