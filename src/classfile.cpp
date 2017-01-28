// ClassFile.cpp
//
// définition des méthodes pour la classe FILE



// Variables statiques

static CBotClass*	m_pClassFILE;
static CBotProgram*	m_pFuncFile;
static int			m_CompteurFileOpen = 0;
static char*		m_filesDir;



// Prépare un nom de fichier.

void PrepareFilename(CBotString &filename)
{
	int			pos;

	pos = filename.ReverseFind('\\');
	if ( pos > 0 )
	{
		filename = filename.Mid(pos+1);  // enlève les dossiers
	}

	pos = filename.ReverseFind('/');
	if ( pos > 0 )
	{
		filename = filename.Mid(pos+1);  // aussi ceux avec /
	}

	pos = filename.ReverseFind(':');
	if ( pos > 0 )
	{
		filename = filename.Mid(pos+1);  // enlève aussi la lettre d'unité C:
	}

	filename = CBotString(m_filesDir) + CBotString("\\") + filename;
}


// constructeur de la classe
// reçois le nom du fichier en paramètre

// exécution
BOOL rfconstruct (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception) 
{
	CBotString	mode;

	// accepte sans paramètre
	if ( pVar == NULL ) return TRUE;

	// qui doit être une chaîne de caractères
	if ( pVar->GivType() != CBotTypString ) { Exception = CBotErrBadString; return FALSE; }

	CBotString	filename = pVar->GivValString();
	PrepareFilename(filename);

	// il peut y avoir  un second paramètre
	pVar = pVar->GivNext();
	if ( pVar != NULL )
	{
		// récupère le mode
		mode = pVar->GivValString();
		if ( mode != "r" && mode != "w" ) { Exception = CBotErrBadParam; return FALSE; }

		// pas de 3e paramètre
		if ( pVar->GivNext() != NULL ) { Exception = CBotErrOverParam; return FALSE; } 
	}

	// enregistre le nom du fichier
	pVar = pThis->GivItem("filename");
	pVar->SetValString(filename);

	if ( ! mode.IsEmpty() )
	{
		// ouvre le ficher demandé
		FILE*	pFile = fopen( filename, mode );
		if ( pFile == NULL ) { Exception = CBotErrFileOpen; return FALSE; }

		m_CompteurFileOpen ++;

		// enregiste le canal du fichier
		pVar = pThis->GivItem("handle");
		pVar->SetValInt((long)pFile);
	}

	return TRUE;
}

// compilation
CBotTypResult cfconstruct (CBotVar* pThis, CBotVar* &pVar)
{
	// accepte sans paramètre
	if ( pVar == NULL ) return CBotTypResult( 0 );

	// qui doit être une chaine
	if ( pVar->GivType() != CBotTypString )
						return CBotTypResult( CBotErrBadString );

	// il peut y avoir un second paramètre
	pVar = pVar->GivNext();
	if ( pVar != NULL )
	{
		// qui doit être une chaine
		if ( pVar->GivType() != CBotTypString )
							return CBotTypResult( CBotErrBadString );
		// pas de 3e paramètre
		if ( pVar->GivNext() != NULL ) return CBotTypResult( CBotErrOverParam );
	}

	// le résultat est de type void (constructeur)
	return CBotTypResult( 0 );
}


// destructeur de la classe

// exécution
BOOL rfdestruct (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception)
{
	// récupère l'élément "handle"
	pVar = pThis->GivItem("handle");

	// pas ouvert ? pas de problème
	if ( pVar->GivInit() != IS_DEF) return TRUE;

	FILE* pFile= (FILE*)pVar->GivValInt();
	fclose(pFile);
	m_CompteurFileOpen --;

	pVar->SetInit(IS_NAN);

	return TRUE;
}


// méthode FILE :: open
// reçois le mode r/w en paramètre

// exécution
BOOL rfopen (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception)
{
	// il doit y avoir un paramètre
	if ( pVar == NULL ) { Exception = CBotErrLowParam; return FALSE; }

	// qui doit être une chaîne de caractères
	if ( pVar->GivType() != CBotTypString ) { Exception = CBotErrBadString; return FALSE; }

	// il peut y avoir  un second paramètre
	if ( pVar->GivNext() != NULL )
	{
		// dans ce cas le premier paramètre est le nom du fichier
		CBotString	filename = pVar->GivValString();
		PrepareFilename(filename);

		// enregistre le nom du fichier
		CBotVar* pVar2 = pThis->GivItem("filename");
		pVar2->SetValString(filename);

		// paramètre suivant est le mode
		pVar = pVar -> GivNext();
	}

	CBotString	mode = pVar->GivValString();
	if ( mode != "r" && mode != "w" ) { Exception = CBotErrBadParam; return FALSE; }

	// pas de 3e paramètre
	if ( pVar->GivNext() != NULL ) { Exception = CBotErrOverParam; return FALSE; }

	// récupère l'élément "handle"
	pVar = pThis->GivItem("handle");

	// qui doit pas être initialisé
	if ( pVar->GivInit() == IS_DEF) { Exception = CBotErrFileOpen; return FALSE; }

	// reprend le nom du fichier
	pVar = pThis->GivItem("filename");
	CBotString	filename = pVar->GivValString();

	PrepareFilename(filename);  // si le nom a été attribué par h.filename = "...";

	// ouvre le ficher demandé
	FILE*	pFile = fopen( filename, mode );
	if ( pFile == NULL )
	{
		pResult->SetValInt(FALSE);
		return TRUE;
	}

	m_CompteurFileOpen ++;

	// enregiste le canal du fichier
	pVar = pThis->GivItem("handle");
	pVar->SetValInt((long)pFile);

	pResult->SetValInt(TRUE);
	return TRUE;
}

// compilation
CBotTypResult cfopen (CBotVar* pThis, CBotVar* &pVar)
{
	// il doit y avoir un paramètre
	if ( pVar == NULL ) return CBotTypResult( CBotErrLowParam );

	// qui doit être une chaine
	if ( pVar->GivType() != CBotTypString )
						return CBotTypResult( CBotErrBadString );

	// il peut y avoir  un second paramètre
	pVar = pVar->GivNext();
	if ( pVar != NULL )
	{
		// qui doit être une chaine
		if ( pVar->GivType() != CBotTypString )
						return CBotTypResult( CBotErrBadString );

		// pas de 3e paramètre
		if ( pVar->GivNext() != NULL ) return CBotTypResult( CBotErrOverParam );
	}

	// le résultat est de type bool
	return CBotTypResult(CBotTypBoolean);
}

	
// méthode FILE :: close

// exécution
BOOL rfclose (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception)
{
	// il ne doit pas y avoir de paramètre
	if ( pVar != NULL ) return CBotErrOverParam;

	// récupère l'élément "handle"
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
	// il ne doit pas y avoir de paramètre
	if ( pVar != NULL ) return CBotTypResult( CBotErrOverParam );

	// la fonction retourne un résultat "void"
	return CBotTypResult( 0 );
}

// méthode FILE :: writeln

// exécution
BOOL rfwrite (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception)
{
	// il doit y avoir un paramètre
	if ( pVar == NULL ) { Exception = CBotErrLowParam; return FALSE; }

	// qui doit être une chaîne de caractères
	if ( pVar->GivType() != CBotTypString ) { Exception = CBotErrBadString; return FALSE; }

	CBotString param = pVar->GivValString();

	// récupère l'élément "handle"
	pVar = pThis->GivItem("handle");

	if ( pVar->GivInit() != IS_DEF) { Exception = CBotErrNotOpen; return FALSE; }

	FILE* pFile= (FILE*)pVar->GivValInt();
	
	int res = fputs(param.operator+("\n"), pFile);

	// en cas d'erreur génère une exception
	if ( res < 0 ) { Exception = CBotErrWrite; return FALSE; }

	return TRUE;
}

// compilation
CBotTypResult cfwrite (CBotVar* pThis, CBotVar* &pVar)
{
	// il doit y avoir un paramètre
	if ( pVar == NULL ) return CBotTypResult( CBotErrLowParam );

	// qui doit être une chaîne de caractères
	if ( pVar->GivType() != CBotTypString ) return CBotTypResult( CBotErrBadString );

	// pas d'autre paramètre
	if ( pVar->GivNext() != NULL ) return CBotTypResult( CBotErrOverParam );

	// la fonction retourne un résultat void
	return CBotTypResult( 0 );
}

// méthode FILE :: readln

// exécution
BOOL rfread (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception)
{
	// il ne doit pas y avoir de paramètre
	if ( pVar != NULL ) { Exception = CBotErrOverParam; return FALSE; }

	// récupère l'élément "handle"
	pVar = pThis->GivItem("handle");

	if ( pVar->GivInit() != IS_DEF) { Exception = CBotErrNotOpen; return FALSE; }

	FILE* pFile= (FILE*)pVar->GivValInt();

	char	chaine[2000];
	int		i;
	for ( i = 0 ; i < 2000 ; i++ ) chaine[i] = 0;

	fgets(chaine, 1999, pFile);

	for ( i = 0 ; i < 2000 ; i++ ) if (chaine[i] == '\n') chaine[i] = 0;

	// en cas d'erreur génère une exception
	if ( ferror(pFile) ) { Exception = CBotErrRead; return FALSE; }

	pResult->SetValString( chaine );

	return TRUE;
}

// compilation
CBotTypResult cfread (CBotVar* pThis, CBotVar* &pVar)
{
	// il ne doit pas y avoir de paramètre
	if ( pVar != NULL ) return CBotTypResult( CBotErrOverParam );

	// la fonction retourne un résultat "string"
	return CBotTypResult( CBotTypString );
}
// méthode FILE :: readln


// exécution
BOOL rfeof (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception)
{
	// il ne doit pas y avoir de paramètre
	if ( pVar != NULL ) { Exception = CBotErrOverParam; return FALSE; }

	// récupère l'élément "handle"
	pVar = pThis->GivItem("handle");

	if ( pVar->GivInit() != IS_DEF) { Exception = CBotErrNotOpen; return FALSE; }

	FILE* pFile= (FILE*)pVar->GivValInt();

	pResult->SetValInt( feof( pFile ) );

	return TRUE;
}

// compilation
CBotTypResult cfeof (CBotVar* pThis, CBotVar* &pVar)
{
	// il ne doit pas y avoir de paramètre
	if ( pVar != NULL ) return CBotTypResult( CBotErrOverParam );

	// la fonction retourne un résultat booleen
	return CBotTypResult( CBotTypBoolean );
}





void InitClassFILE()
{
// crée une classe pour la gestion des fichiers
// l'utilisation en est la suivante:
// file canal( "NomFichier.txt" )
// canal.open( "r" );	// ouvre en lecture
// s = canal.readln( ); // lit une ligne
// canal.close();		// referme le fichier

	// crée la classe FILE
	m_pClassFILE	= new CBotClass("file", NULL);
	// ajoute le composant ".filename"
	m_pClassFILE->AddItem("filename", CBotTypString);
	// ajoute le composant ".handle"
	m_pClassFILE->AddItem("handle", CBotTypInt, PR_PRIVATE);

	// défini un constructeur et un destructeur
	m_pClassFILE->AddFunction("file", rfconstruct, cfconstruct );
	m_pClassFILE->AddFunction("~file", rfdestruct, NULL );

	// défini les méthodes associées
	m_pClassFILE->AddFunction("open", rfopen, cfopen );
	m_pClassFILE->AddFunction("close", rfclose, cfclose );
	m_pClassFILE->AddFunction("writeln", rfwrite, cfwrite );
	m_pClassFILE->AddFunction("readln", rfread, cfread );
	m_pClassFILE->AddFunction("eof", rfeof, cfeof );

	m_pFuncFile = new CBotProgram( );
	CBotStringArray ListFonctions;
	m_pFuncFile->Compile( "public file openfile(string name, string mode) {return new file(name, mode);}", ListFonctions);
	m_pFuncFile->SetIdent(-2);	// identificateur spécial pour RestoreState dans cette fonction
}

