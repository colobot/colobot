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

// ClassFile.cpp
//
// definition of methods for class FILE



// Static variables

static CBotClass*	m_pClassFILE;
static CBotProgram*	m_pFuncFile;
static int			m_CompteurFileOpen = 0;



// Prepares a file name.

void PrepareFilename(CBotString &filename)  //DD!
{
	int			pos;

	pos = filename.ReverseFind('\\');
	if ( pos > 0 )
	{
		filename = filename.Mid(pos+1);		// remove the records (files)??
	}

	pos = filename.ReverseFind('/');
	if ( pos > 0 )
	{
		filename = filename.Mid(pos+1);		// also those with /
	}

	pos = filename.ReverseFind(':');
	if ( pos > 0 )
	{
		filename = filename.Mid(pos+1);		// also removes the drive letter C:
	}

	filename = CBotString("files\\") + filename;
}


// constructor of the class
// gets the filename as a parameter

// execution
bool rfconstruct (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception) 
{
	CBotString	mode;

	// accepts no parameters
	if ( pVar == NULL ) return TRUE;

	// must be a string
	if ( pVar->GivType() != CBotTypString ) { Exception = CBotErrBadString; return FALSE; }

	CBotString	filename = pVar->GivValString();
	PrepareFilename(filename);  //DR

	// there may be a second parameter
	pVar = pVar->GivNext();
	if ( pVar != NULL )
	{
		// recovers the mode
		mode = pVar->GivValString();
		if ( mode != "r" && mode != "w" ) { Exception = CBotErrBadParam; return FALSE; }

		// no third parameter, only two or one possible
		if ( pVar->GivNext() != NULL ) { Exception = CBotErrOverParam; return FALSE; } 
	}

	// save the file name
	pVar = pThis->GivItem("filename");
	pVar->SetValString(filename);

	if ( ! mode.IsEmpty() )
	{
		// open the called file
		FILE*	pFile = fopen( filename, mode );
		if ( pFile == NULL ) { Exception = CBotErrFileOpen; return FALSE; }

		m_CompteurFileOpen ++;

		// save the handle of file
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

	// must be a string
	if ( pVar->GivType() != CBotTypString )
						return CBotTypResult( CBotErrBadString );

	// there may be a second parameter
	pVar = pVar->GivNext();
	if ( pVar != NULL )
	{
		// must be a string
		if ( pVar->GivType() != CBotTypString )
							return CBotTypResult( CBotErrBadString );
		// no third parameter
		if ( pVar->GivNext() != NULL ) return CBotTypResult( CBotErrOverParam );
	}

	// le résultat est de type void (constructeur)
	return CBotTypResult( 0 );
}


// destructor of the class

// execution
bool rfdestruct (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception)
{
	// retrieves the element "handle"
	pVar = pThis->GivItem("handle");

	// not open? no problem
	if ( pVar->GivInit() != IS_DEF) return TRUE;

	FILE* pFile= (FILE*)pVar->GivValInt();
	fclose(pFile);
	m_CompteurFileOpen --;

	pVar->SetInit(IS_NAN);

	return TRUE;
}


// FILE :: open method
// get the r / w mode as a parameter

// execution
bool rfopen (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception)
{
	// there must be a parameter
	if ( pVar == NULL ) { Exception = CBotErrLowParam; return FALSE; }

	// must be a string
	if ( pVar->GivType() != CBotTypString ) { Exception = CBotErrBadString; return FALSE; }

	// there may be a second parameter
	if ( pVar->GivNext() != NULL )
	{
		// in this case the first parameter is the file name
		CBotString	filename = pVar->GivValString();
		PrepareFilename(filename);  //DR

		// saves the file name
		CBotVar* pVar2 = pThis->GivItem("filename");
		pVar2->SetValString(filename);

		// next parameter is the mode
		pVar = pVar -> GivNext();
	}

	CBotString	mode = pVar->GivValString();
	if ( mode != "r" && mode != "w" ) { Exception = CBotErrBadParam; return FALSE; }

	// No third parameter
	if ( pVar->GivNext() != NULL ) { Exception = CBotErrOverParam; return FALSE; }

	// retrieves the element "handle"
	pVar = pThis->GivItem("handle");

	// which must not be initialized
	if ( pVar->GivInit() == IS_DEF) { Exception = CBotErrFileOpen; return FALSE; }

	// contains filename
	pVar = pThis->GivItem("filename");
	CBotString	filename = pVar->GivValString();

	PrepareFilename(filename);  //DD! (if the name was assigned by h.filename = "...";

	// open requsted file
	FILE*	pFile = fopen( filename, mode );
	if ( pFile == NULL )  //DR
	{
		pResult->SetValInt(FALSE);  //DR
		return TRUE;  //DR
	}

	m_CompteurFileOpen ++;

	// saves the handle of file
	pVar = pThis->GivItem("handle");
	pVar->SetValInt((long)pFile);

	pResult->SetValInt(TRUE);  //DR
	return TRUE;
}

// compilation
CBotTypResult cfopen (CBotVar* pThis, CBotVar* &pVar)
{
	// there must be a parameter
	if ( pVar == NULL ) return CBotTypResult( CBotErrLowParam );

	// must be a string
	if ( pVar->GivType() != CBotTypString )
						return CBotTypResult( CBotErrBadString );

	// there may be a second parameter
	pVar = pVar->GivNext();
	if ( pVar != NULL )
	{
		// must be a string
		if ( pVar->GivType() != CBotTypString )
						return CBotTypResult( CBotErrBadString );

		// no third parameter
		if ( pVar->GivNext() != NULL ) return CBotTypResult( CBotErrOverParam );
	}

	// the result is of type bool
	return CBotTypResult(CBotTypBoolean);  //DR
}

	
// FILE :: close method

// execution
bool rfclose (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception)
{
	// it should not be any parameter
	if ( pVar != NULL ) return CBotErrOverParam;

	// retrieves the element "handle"
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
	// it should not be any parameter
	if ( pVar != NULL ) return CBotTypResult( CBotErrOverParam );

	// function returns a result "void"
	return CBotTypResult( 0 );
}

// FILE :: writeln method

// execution
bool rfwrite (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception)
{
	// there must be a parameter
	if ( pVar == NULL ) { Exception = CBotErrLowParam; return FALSE; }

	// must be a string
	if ( pVar->GivType() != CBotTypString ) { Exception = CBotErrBadString; return FALSE; }

	CBotString param = pVar->GivValString();

	//retrieves the element "handle"
	pVar = pThis->GivItem("handle");

	if ( pVar->GivInit() != IS_DEF) { Exception = CBotErrNotOpen; return FALSE; }

	FILE* pFile= (FILE*)pVar->GivValInt();
	
	int res = fputs(param+CBotString("\n"), pFile);

	// on error throws an exception
	if ( res < 0 ) { Exception = CBotErrWrite; return FALSE; }

	return TRUE;
}

// compilation
CBotTypResult cfwrite (CBotVar* pThis, CBotVar* &pVar)
{
	// there must be a parameter
	if ( pVar == NULL ) return CBotTypResult( CBotErrLowParam );

	// must be a string
	if ( pVar->GivType() != CBotTypString ) return CBotTypResult( CBotErrBadString );

	// no other parameter
	if ( pVar->GivNext() != NULL ) return CBotTypResult( CBotErrOverParam );

	// function returns "void" result
	return CBotTypResult( 0 );
}

// FILE :: readln method

// execution
bool rfread (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception)
{
	// there shouldn't be any parameter
	if ( pVar != NULL ) { Exception = CBotErrOverParam; return FALSE; }

	//retrieves the element "handle"
	pVar = pThis->GivItem("handle");

	if ( pVar->GivInit() != IS_DEF) { Exception = CBotErrNotOpen; return FALSE; }

	FILE* pFile= (FILE*)pVar->GivValInt();

	char	chaine[2000];
	int		i;
	for ( i = 0 ; i < 2000 ; i++ ) chaine[i] = 0;

	fgets(chaine, 1999, pFile);

	for ( i = 0 ; i < 2000 ; i++ ) if (chaine[i] == '\n') chaine[i] = 0;

	// on error throws an exception
	if ( ferror(pFile) ) { Exception = CBotErrRead; return FALSE; }

	pResult->SetValString( chaine );

	return TRUE;
}

// compilation
CBotTypResult cfread (CBotVar* pThis, CBotVar* &pVar)
{
	// there shouldn't be any parameter
	if ( pVar != NULL ) return CBotTypResult( CBotErrOverParam );

	// function return "string" result
	return CBotTypResult( CBotTypString );
}
// FILE :: readln method


// execution
bool rfeof (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception)
{
	// there shouldn't be any parameter
	if ( pVar != NULL ) { Exception = CBotErrOverParam; return FALSE; }

	// retrieves the element "handle"
	pVar = pThis->GivItem("handle");

	if ( pVar->GivInit() != IS_DEF) { Exception = CBotErrNotOpen; return FALSE; }

	FILE* pFile= (FILE*)pVar->GivValInt();

	pResult->SetValInt( feof( pFile ) );

	return TRUE;
}

// compilation
CBotTypResult cfeof (CBotVar* pThis, CBotVar* &pVar)
{
	// there shouldn't be any parameter
	if ( pVar != NULL ) return CBotTypResult( CBotErrOverParam );

	// function return boolean result
	return CBotTypResult( CBotTypBoolean );
}





void InitClassFILE()
{
// creates a class for file management
// the usage is as follows:
// file canal( "NomFichier.txt" )
// canal.open( "r" );	// open reading
// s = canal.readln( ); // reads a line
// canal.close();		// closes the file

	// create class FILE
	m_pClassFILE	= new CBotClass("file", NULL);
	// add the component ".filename"
	m_pClassFILE->AddItem("filename", CBotTypString);
	// add the component ".handle"
	m_pClassFILE->AddItem("handle", CBotTypInt, PR_PRIVATE);

	// define a constructor and destructor
	m_pClassFILE->AddFunction("file", rfconstruct, cfconstruct );
	m_pClassFILE->AddFunction("~file", rfdestruct, NULL );

	// defined associated methods
	m_pClassFILE->AddFunction("open", rfopen, cfopen );
	m_pClassFILE->AddFunction("close", rfclose, cfclose );
	m_pClassFILE->AddFunction("writeln", rfwrite, cfwrite );
	m_pClassFILE->AddFunction("readln", rfread, cfread );
	m_pClassFILE->AddFunction("eof", rfeof, cfeof );

	m_pFuncFile = new CBotProgram( );
	CBotStringArray ListFonctions;
	m_pFuncFile->Compile( "public file openfile(string name, string mode) {return new file(name, mode);}", ListFonctions);
	m_pFuncFile->SetIdent(-2);	// restoreState as a special identifier for this function
}

