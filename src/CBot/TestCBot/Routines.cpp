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

////////////////////////////////////////////////////////////////////
// routine show()
// utilisable depuis le programme écrit en CBot

// exécution
BOOL    rShow( CBotVar* pVar, CBotVar* pResult, int& Exception, void* pUser )
{
    CString     s;

    while ( pVar != NULL )
    {
        CString ss;
        ss.LoadString( TX_TYPENAMES + pVar->GivType() );
        s += ss + " ";

        ss = pVar->GivName();
        if (ss.IsEmpty()) ss = "<sans nom>";
        s += ss + " = ";

        s += pVar->GivValString();
        s += "\n";
        pVar = pVar->GivNext();
    }

    AfxMessageBox(s, MB_OK|MB_ICONINFORMATION);

    return  TRUE;           // pas d'interruption
}

CBotTypResult   cShow( CBotVar* &pVar, void* pUser)
{
    if ( pVar == NULL ) return CBotTypResult(5028);
    return CBotTypResult(0);        // tous paramètres acceptés, void en retour
}


////////////////////////////////////////////////////////////////////
// routine print()
// utilisable depuis le programme écrit en CBot

// exécution
BOOL    rPrintLn( CBotVar* pVar, CBotVar* pResult, int& Exception, void* pUser )
{
    CString     s;

    CTestCBotApp* pApp = (CTestCBotApp*)AfxGetApp();
    CEdit*       pEdit = pApp->m_pConsole;

    if (pEdit == NULL) return TRUE;
    pEdit->GetWindowText(s);

    while ( pVar != NULL )
    {
        if ( !s.IsEmpty() ) s += " ";
        s += pVar->GivValString();
        pVar = pVar->GivNext();
    }
    s += "\r\n";

    pEdit->SetWindowText(s);
    pEdit->SetSel(s.GetLength(), s.GetLength());
    pEdit->SetFocus();
    return  TRUE;           // pas d'interruption
}

BOOL    rPrint( CBotVar* pVar, CBotVar* pResult, int& Exception, void* pUser )
{
    CString     s;

    CTestCBotApp* pApp = (CTestCBotApp*)AfxGetApp();
    CEdit*       pEdit = pApp->m_pConsole;

    if (pEdit == NULL) return TRUE;
    pEdit->GetWindowText(s);

    while ( pVar != NULL )
    {
        if ( !s.IsEmpty() ) s += " ";
        s += pVar->GivValString();
        pVar = pVar->GivNext();
    }

    pEdit->SetWindowText(s);
    pEdit->SetSel(s.GetLength(), s.GetLength());
    pEdit->SetFocus();
    return  TRUE;           // pas d'interruption
}

CBotTypResult   cPrint( CBotVar* &pVar, void* pUser)
{
    return CBotTypResult(0);                // tous paramètres acceptés, un entier en retour
}


//////////////////////////////////////////////////////////////////
// class CPoint pour essayer

// exécution
BOOL    rCPoint( CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception )
{
    CString     s;

    if ( pVar == NULL )return TRUE;         // constructeur sans paramètres est ok

    CBotVar*    pX = pThis->GivItem("x");
    pX->SetValFloat( pVar->GivValFloat() );
    pVar    = pVar->GivNext();

    CBotVar*    pY = pThis->GivItem("y");
    pY->SetValFloat( pVar->GivValFloat() );
    pVar    = pVar->GivNext();

    return  TRUE;           // pas d'interruption
}

CBotTypResult cCPoint( CBotVar* pThis, CBotVar* &pVar)
{
    // ok si aucun paramètres !
    if ( pVar == NULL ) return CBotTypResult(0);

    // paramètre de type numérique svp
    if ( pVar->GivType() > CBotTypDouble ) return CBotTypResult(5011);
    pVar    = pVar->GivNext();

    // il doit y avoir un second paramètre
    if ( pVar == NULL ) return 5028;
    // également de type numérique
    if ( pVar->GivType() > CBotTypDouble )return CBotTypResult(5011);
    pVar    = pVar->GivNext();

    // et pas plus de 2 paramètres svp
    if ( pVar != NULL ) return CBotTypResult(5026);

    return CBotTypResult(0);        // cette fonction retourne void
}


