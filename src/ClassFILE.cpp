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



// Static variables

static CBotClass*   m_pClassFILE;
static CBotProgram* m_pFuncFile;
static int          m_CompteurFileOpen = 0;
static char*        m_filesDir;



// Prepares a file name.

void PrepareFilename(CBotString &filename)
{
    int         pos;

    pos = filename.ReverseFind('\\');
    if ( pos > 0 )
    {
        filename = filename.Mid(pos+1);  // Remove files with
    }

    pos = filename.ReverseFind('/');
    if ( pos > 0 )
    {
        filename = filename.Mid(pos+1);  // also with /
    }

    pos = filename.ReverseFind(':');
    if ( pos > 0 )
    {
        filename = filename.Mid(pos+1);  // also removes the drive letter C:
    }

    filename = CBotString(m_filesDir) + CBotString("\\") + filename;
}


// constructor of the class
// get the filename as a parameter

// execution
BOOL rfconstruct (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception)
{
    CBotString  mode;

    // accepts no parameters
    if ( pVar == NULL ) return TRUE;

    // must be a character string
    if ( pVar->GivType() != CBotTypString ) { Exception = CBotErrBadString; return FALSE; }

    CBotString  filename = pVar->GivValString();
    PrepareFilename(filename);

    // there may be a second parameter
    pVar = pVar->GivNext();
    if ( pVar != NULL )
    {
        // recover mode
        mode = pVar->GivValString();
        if ( mode != "r" && mode != "w" ) { Exception = CBotErrBadParam; return FALSE; }

        // no third parameter
        if ( pVar->GivNext() != NULL ) { Exception = CBotErrOverParam; return FALSE; }
    }

    // saves the file name
    pVar = pThis->GivItem("filename");
    pVar->SetValString(filename);

    if ( ! mode.IsEmpty() )
    {
        // opens the requested file
        FILE*   pFile = fopen( filename, mode );
        if ( pFile == NULL ) { Exception = CBotErrFileOpen; return FALSE; }

        m_CompteurFileOpen ++;

        // save the channel file
        pVar = pThis->GivItem("handle");
        pVar->SetValInt((long)pFile);
    }

    return TRUE;
}

// compilation
CBotTypResult cfconstruct (CBotVar* pThis, CBotVar* &pVar)
{
    // accepts no parameters
    if ( pVar == NULL ) return CBotTypResult( 0 );

    // must be a character string
    if ( pVar->GivType() != CBotTypString )
                        return CBotTypResult( CBotErrBadString );

    // there may be a second parameter
    pVar = pVar->GivNext();
    if ( pVar != NULL )
    {
        // which must be a string
        if ( pVar->GivType() != CBotTypString )
                            return CBotTypResult( CBotErrBadString );
        // no third parameter
        if ( pVar->GivNext() != NULL ) return CBotTypResult( CBotErrOverParam );
    }

    // the result is void (constructor)
    return CBotTypResult( 0 );
}


// destructor of the class

// execution
BOOL rfdestruct (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception)
{
    // retrieve the item "handle"
    pVar = pThis->GivItem("handle");

    // don't open? no problem :)
    if ( pVar->GivInit() != IS_DEF) return TRUE;

    FILE* pFile= (FILE*)pVar->GivValInt();
    fclose(pFile);
    m_CompteurFileOpen --;

    pVar->SetInit(IS_NAN);

    return TRUE;
}


// process FILE :: open
// get the r/w mode as a parameter

// execution
BOOL rfopen (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception)
{
    // there must be a parameter
    if ( pVar == NULL ) { Exception = CBotErrLowParam; return FALSE; }

    // which must be a character string
    if ( pVar->GivType() != CBotTypString ) { Exception = CBotErrBadString; return FALSE; }

    // There may be a second parameter
    if ( pVar->GivNext() != NULL )
    {
        // if the first parameter is the file name
        CBotString  filename = pVar->GivValString();
        PrepareFilename(filename);

        // saves the file name
        CBotVar* pVar2 = pThis->GivItem("filename");
        pVar2->SetValString(filename);

        // next parameter is the mode
        pVar = pVar -> GivNext();
    }

    CBotString  mode = pVar->GivValString();
    if ( mode != "r" && mode != "w" ) { Exception = CBotErrBadParam; return FALSE; }

    // no third parameter
    if ( pVar->GivNext() != NULL ) { Exception = CBotErrOverParam; return FALSE; }

    // retrieve the item "handle"
    pVar = pThis->GivItem("handle");

    // which must not be initialized
    if ( pVar->GivInit() == IS_DEF) { Exception = CBotErrFileOpen; return FALSE; }

    // file contains the name
    pVar = pThis->GivItem("filename");
    CBotString  filename = pVar->GivValString();

    PrepareFilename(filename);  // if the name was h.filename attribute = "...";

    // opens the requested file
    FILE*   pFile = fopen( filename, mode );
    if ( pFile == NULL )
    {
        pResult->SetValInt(FALSE);
        return TRUE;
    }

    m_CompteurFileOpen ++;

    // Registered the channel file
    pVar = pThis->GivItem("handle");
    pVar->SetValInt((long)pFile);

    pResult->SetValInt(TRUE);
    return TRUE;
}

// compilation
CBotTypResult cfopen (CBotVar* pThis, CBotVar* &pVar)
{
    // there must be a parameter
    if ( pVar == NULL ) return CBotTypResult( CBotErrLowParam );

    // which must be a string
    if ( pVar->GivType() != CBotTypString )
                        return CBotTypResult( CBotErrBadString );

    // there may be a second parameter
    pVar = pVar->GivNext();
    if ( pVar != NULL )
    {
        // which must be a string
        if ( pVar->GivType() != CBotTypString )
                        return CBotTypResult( CBotErrBadString );

        // no third parameter
        if ( pVar->GivNext() != NULL ) return CBotTypResult( CBotErrOverParam );
    }

    // the result is bool
    return CBotTypResult(CBotTypBoolean);
}


// process FILE :: close

// execeution
BOOL rfclose (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception)
{
    // it shouldn't be any parameters
    if ( pVar != NULL ) return CBotErrOverParam;

    // retrieve the item "handle"
    pVar = pThis->GivItem("handle");

    if ( pVar->GivInit() != IS_DEF) { Exception = CBotErrNotOpen; return FALSE; }

    FILE* pFile= (FILE*)pVar->GivValInt();
    fclose(pFile);
    m_CompteurFileOpen --;

    pVar->SetInit(IS_NAN);

    return TRUE;
}

// compilation
CBotTypResult cfclose (CBotVar* pThis, CBotVar* &pVar)
{
    // it shouldn't be any parameters
    if ( pVar != NULL ) return CBotTypResult( CBotErrOverParam );

    // function returns a result "void"
    return CBotTypResult( 0 );
}

// process FILE :: writeln

// execution
BOOL rfwrite (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception)
{
    // there must be a parameter
    if ( pVar == NULL ) { Exception = CBotErrLowParam; return FALSE; }

    // which must be a character string
    if ( pVar->GivType() != CBotTypString ) { Exception = CBotErrBadString; return FALSE; }

    CBotString param = pVar->GivValString();

    // retrieve the item "handle"
    pVar = pThis->GivItem("handle");

    if ( pVar->GivInit() != IS_DEF) { Exception = CBotErrNotOpen; return FALSE; }

    FILE* pFile= (FILE*)pVar->GivValInt();

    int res = fputs(param+CBotString("\n"), pFile);

    // if an error occurs generate an exception
    if ( res < 0 ) { Exception = CBotErrWrite; return FALSE; }

    return TRUE;
}

// compilation
CBotTypResult cfwrite (CBotVar* pThis, CBotVar* &pVar)
{
    // there must be a parameter
    if ( pVar == NULL ) return CBotTypResult( CBotErrLowParam );

    // which must be a character string
    if ( pVar->GivType() != CBotTypString ) return CBotTypResult( CBotErrBadString );

    // no other parameter
    if ( pVar->GivNext() != NULL ) return CBotTypResult( CBotErrOverParam );

    // the function returns a void result
    return CBotTypResult( 0 );
}

// process FILE :: readln

// execution
BOOL rfread (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception)
{
    // it shouldn't be any parameters
    if ( pVar != NULL ) { Exception = CBotErrOverParam; return FALSE; }

    // retrieve the item "handle"
    pVar = pThis->GivItem("handle");

    if ( pVar->GivInit() != IS_DEF) { Exception = CBotErrNotOpen; return FALSE; }

    FILE* pFile= (FILE*)pVar->GivValInt();

    char    chaine[2000];
    int     i;
    for ( i = 0 ; i < 2000 ; i++ ) chaine[i] = 0;

    fgets(chaine, 1999, pFile);

    for ( i = 0 ; i < 2000 ; i++ ) if (chaine[i] == '\n') chaine[i] = 0;

    // if an error occurs generate an exception
    if ( ferror(pFile) ) { Exception = CBotErrRead; return FALSE; }

    pResult->SetValString( chaine );

    return TRUE;
}

// compilation
CBotTypResult cfread (CBotVar* pThis, CBotVar* &pVar)
{
    // it should not be any parameter
    if ( pVar != NULL ) return CBotTypResult( CBotErrOverParam );

    // function returns a result "string"
    return CBotTypResult( CBotTypString );
}
// process FILE :: readln


// execution
BOOL rfeof (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception)
{
    // it should not be any parameter
    if ( pVar != NULL ) { Exception = CBotErrOverParam; return FALSE; }

    // retrieve the item "handle"
    pVar = pThis->GivItem("handle");

    if ( pVar->GivInit() != IS_DEF) { Exception = CBotErrNotOpen; return FALSE; }

    FILE* pFile= (FILE*)pVar->GivValInt();

    pResult->SetValInt( feof( pFile ) );

    return TRUE;
}

// compilation
CBotTypResult cfeof (CBotVar* pThis, CBotVar* &pVar)
{
    // it shouldn't be any parameter
    if ( pVar != NULL ) return CBotTypResult( CBotErrOverParam );

    // the function returns a boolean result
    return CBotTypResult( CBotTypBoolean );
}





void InitClassFILE()
{
// create a class for file management
// the use is as follows:
// file canal( "NomFichier.txt" )
// canal.open( "r" );   // open for read
// s = canal.readln( ); // reads a line
// canal.close();   // close the file

    // create the class FILE
    m_pClassFILE    = new CBotClass("file", NULL);
    // adds the component ".filename"
    m_pClassFILE->AddItem("filename", CBotTypString);
    // adds the component ".handle"
    m_pClassFILE->AddItem("handle", CBotTypInt, PR_PRIVATE);

    // define a constructor and a destructor
    m_pClassFILE->AddFunction("file", rfconstruct, cfconstruct );
    m_pClassFILE->AddFunction("~file", rfdestruct, NULL );

    // end of the methods associated
    m_pClassFILE->AddFunction("open", rfopen, cfopen );
    m_pClassFILE->AddFunction("close", rfclose, cfclose );
    m_pClassFILE->AddFunction("writeln", rfwrite, cfwrite );
    m_pClassFILE->AddFunction("readln", rfread, cfread );
    m_pClassFILE->AddFunction("eof", rfeof, cfeof );

    m_pFuncFile = new CBotProgram( );
    CBotStringArray ListFonctions;
    m_pFuncFile->Compile( "public file openfile(string name, string mode) {return new file(name, mode);}", ListFonctions);
    m_pFuncFile->SetIdent(-2);  // restoreState in special identifier for this function
}

