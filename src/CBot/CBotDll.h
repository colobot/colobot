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

//#include "stdafx.h"

#include <windows.h>        
#include <stdio.h>

#define DllExport    __declspec( dllexport )

#define	CBOTVERSION	104

////////////////////////////////////////////////////////////////////////
// quelques classes d�finies par ailleurs

class CBotToken;		// programme transform� en "jetons"		
class CBotStack;		// pile pour l'ex�cution
class CBotClass;		// classe d'object
class CBotInstr;		// instruction � ex�cuter
class CBotFunction;		// les fonctions user
class CBotVar;			// les variables
class CBotVarClass;		// une instance de classe
class CBotVarPointer;	// pointeur � une instance de classe
class CBotCall;			// les fonctions
class CBotCallMethode;	// les m�thodes
class CBotDefParam;		// liste de param�tres


////////////////////////////////////////////////////////////////////////
// Gestion des variables
////////////////////////////////////////////////////////////////////////

// ces types sont calqu�s sur les types Java
// ne pas changer l'ordre de ces types

enum CBotType
{
	CBotTypVoid			= 0,				// fonction retournant void
	CBotTypByte			= 1,	//n			// nombre entier ( 8 bits)
	CBotTypShort		= 2,	//n			// nombre entier (16 bits)
	CBotTypChar			= 3,	//n			// caract�re "unicode" (16 bits)
	CBotTypInt			= 4,				// nombre entier (32 bits)
	CBotTypLong			= 5,	//n			// nombre entier (64 bits)
	CBotTypFloat		= 6,				// nombre d�cimal (32 bits)
	CBotTypDouble		= 7,	//n			// nombre d�cimal (64 bits)
	CBotTypBoolean		= 8,				// true ou false exclusivement
	CBotTypString		= 9,				// chaine de caract�re

	CBotTypArrayPointer	= 10,				// un tableau de variables
	CBotTypArrayBody	= 11,				// idem mais cr�e l'instance

	CBotTypPointer		= 12,				// pointeur � une instance
	CBotTypNullPointer  = 13,				// pointeur null est sp�cial

	CBotTypClass		= 15,				// instance d'une classe
	CBotTypIntrinsic	= 16				// instance d'une classe intrins�que
};
			//n = non encore impl�ment�

// pour SetUserPtr lors de la suppression d'un objet
#define OBJECTDELETED ((void*)-1)
// valeur mise avant initialisation
#define OBJECTCREATED ((void*)-2)


// classe permettant de d�finir le type complet d'un r�sultat
class CBotTypResult
{
private:
	int				m_type;
	CBotTypResult*	m_pNext;	// pour les types de types
	CBotClass*		m_pClass;	// pour les d�riv�s de classe
	int				m_limite;	// limitation des tableaux 
	friend class	CBotVarClass;
	friend class	CBotVarPointer;	

public:
	// divers constructeurs selon les besoins
	DllExport
				CBotTypResult(int type);
				// pour les types simples (CBotTypInt � CBotTypString)
	DllExport
				CBotTypResult(int type, const char* name);
				// pour les types pointeur et classe intrinsic
	DllExport
				CBotTypResult(int type, CBotClass* pClass);
				// idem � partir de l'instance d'une classe
	DllExport
				CBotTypResult(int type, CBotTypResult elem);
				// pour les tableaux de variables

	DllExport
				CBotTypResult(CBotTypResult& typ);
				// pour les assignations
	DllExport
				CBotTypResult();
				// pour par d�faut
	DllExport
				~CBotTypResult();

	DllExport
	int			GivType(int mode = 0);
				// rend le type CBotTyp* du r�sultat

	void		SetType(int n);
				// modifie le type

	DllExport
	CBotClass*	GivClass();
				// rend le pointeur � la classe (pour les CBotTypClass, CBotTypPointer)

	DllExport
	int			GivLimite();
				// rend la taille limite du tableau (CBotTypArray)

	DllExport
	void		SetLimite(int n);
				// fixe une limite au tableau

	void		SetArray(int* max );
				// idem avec une liste de dimension (tableaux de tableaux)

	DllExport
	CBotTypResult& GivTypElem();
				// rend le type des �l�ments du tableau (CBotTypArray)

	DllExport
	BOOL		Compare(CBotTypResult& typ);
				// compare si les types sont compatibles
	DllExport
	BOOL		Eq(int type);
				// compare le type

	DllExport
	CBotTypResult&
				operator=(const CBotTypResult& src);
				// copie un type complet dans un autre
};

/*
// pour d�finir un r�sultat en sortie, utiliser par exemple

	// pour rendre un simple Float
	return CBotTypResult( CBotTypFloat );


	// pour rendre un tableau de string
	return CBotTypResult( CBotTypArray, CBotTypResult( CBotTypString ) );

	// pour rendre un tableau de tableau de "point"
	CBotTypResult	typPoint( CBotTypIntrinsic, "point" );
	CBotTypResult	arrPoint( CBotTypArray, typPoint );
	return	CBotTypResult( CBotTypArray, arrPoint );
*/


////////////////////////////////////////////////////////////////////////
// Gestion des erreurs compilation et ex�cution
////////////////////////////////////////////////////////////////////////

// voici la liste des erreurs pouvant �tre retourn�es par le module
// pour la compilation

#define	CBotErrOpenPar			5000	// manque la parenth�se ouvrante
#define	CBotErrClosePar			5001	// manque la parenth�se fermante
#define	CBotErrNotBoolean		5002	// l'expression doit �tre un boolean
#define	CBotErrUndefVar			5003	// variable non d�clar�e
#define	CBotErrBadLeft			5004	// assignation impossible ( 5 = ... )
#define	CBotErrNoTerminator		5005	// point-virgule attendu
#define	CBotErrCaseOut			5006	// case en dehors d'un switch
//	CBotErrNoTerm			5007, plus utile
#define	CBotErrCloseBlock		5008	// manque " } "
#define	CBotErrElseWhitoutIf	5009	// else sans if correspondant
#define	CBotErrOpenBlock		5010	// manque " { "
#define	CBotErrBadType1			5011	// mauvais type pour l'assignation
#define	CBotErrRedefVar			5012	// red�finition de la variable
#define	CBotErrBadType2			5013	// 2 op�randes de type incompatibles
#define	CBotErrUndefCall		5014	// routine inconnue
#define	CBotErrNoDoubleDots		5015	// " : " attendu
//	CBotErrWhile			5016, plus utile
#define	CBotErrBreakOutside		5017	// break en dehors d'une boucle
#define	CBotErrUndefLabel		5019	// label inconnu
#define	CBotErrLabel			5018	// label ne peut se mettre ici
#define	CBotErrNoCase			5020	// manque " case "
#define	CBotErrBadNum			5021	// nombre attendu
#define	CBotErrVoid				5022	// " void " pas possible ici
#define	CBotErrNoType			5023	// d�claration de type attendue
#define	CBotErrNoVar			5024	// nom de variable attendu
#define	CBotErrNoFunc			5025	// nom de fonction attendu
#define	CBotErrOverParam		5026	// trop de param�tres
#define	CBotErrRedefFunc		5027	// cette fonction existe d�j�
#define	CBotErrLowParam			5028	// pas assez de param�tres
#define	CBotErrBadParam			5029	// mauvais types de param�tres
#define	CBotErrNbParam			5030	// mauvais nombre de param�tres
#define	CBotErrUndefItem		5031	// �l�ment n'existe pas dans la classe
#define	CBotErrUndefClass		5032	// variable n'est pas une classe
#define	CBotErrNoConstruct		5033	// pas de constructeur appropri�
#define	CBotErrRedefClass		5034	// classe existe d�j�
#define	CBotErrCloseIndex		5035	// " ] " attendu
#define	CBotErrReserved			5036	// mot r�serv� (par un DefineNum)
#define CBotErrBadNew			5037	// mauvais param�tre pour new
#define CBotErrOpenIndex	    5038	// " [ " attendu
#define CBotErrBadString		5039	// cha�ne de caract�re attendue
#define CBotErrBadIndex			5040	// mauvais type d'index "[ false ]"
#define CBotErrPrivate			5041	// �l�ment prot�g�
#define CBotErrNoPublic			5042	// manque le mot "public"

// voici la liste des erreurs pouvant �tre retourn�es par le module
// pour l'ex�cution

#define	CBotErrZeroDiv			6000	// division par z�ro
#define	CBotErrNotInit			6001	// variable non initialis�e
#define	CBotErrBadThrow			6002	// throw d'une valeur n�gative
#define	CBotErrNoRetVal			6003	// fonction n'a pas retourn� de r�sultat
#define	CBotErrNoRun			6004	// Run() sans fonction active
#define	CBotErrUndefFunc		6005	// appel d'une fonction qui n'existe plus
#define CBotErrNotClass			6006	// cette classe n'existe pas
#define CBotErrNull				6007	// pointeur null
#define CBotErrNan				6008	// calcul avec un NAN
#define CBotErrOutArray			6009	// index hors du tableau
#define CBotErrStackOver		6010	// d�passement de la pile
#define CBotErrDeletedPtr		6011	// pointeur � un objet d�truit

#define CBotErrFileOpen			6012	// ouverture du fichier impossible
#define CBotErrNotOpen			6013	// canal pas ouvert
#define CBotErrRead				6014	// erreur � la lecture
#define CBotErrWrite			6015	// erreur � l'�criture

// d'autres valeurs peuvent �tre rendues 
// par exemple les exceptions rendues par les routines externes
// et les " throw " avec un nombre quelconque.


////////////////////////////////////////////////////////////////////////
// d�finie une classe pour l'utilisation des strings
// car CString fait partie de MFC pas utilis� ici.
//
// ( toutes les fonctions ne sont pas encore impl�ment�es )

class CBotString
{
private:
	char*			m_ptr;						// pointeur � la chaine
	int				m_lg;						// longueur de la chaine
	static 
	HINSTANCE		m_hInstance;

public:
	DllExport
					CBotString();
	DllExport
					CBotString(const char* p);
	DllExport
					CBotString(const CBotString& p);
	DllExport
					~CBotString();

	DllExport
	void			Empty();
	DllExport
	BOOL			IsEmpty();
	DllExport
	int				GivLength();
	DllExport
	int				Find(const char c);
	DllExport
	int				Find(LPCTSTR lpsz);
	DllExport
	int				ReverseFind(const char c);
	DllExport
	int				ReverseFind(LPCTSTR lpsz);
	DllExport
	BOOL			LoadString(UINT id);
	DllExport
	CBotString		Mid(int nFirst, int nCount) const;
	DllExport
	CBotString		Mid(int nFirst) const;
	DllExport
	CBotString		Left(int nCount) const;
	DllExport
	CBotString		Right(int nCount) const;

	DllExport
	const CBotString&
					operator=(const CBotString& stringSrc);
	DllExport
	const CBotString&
					operator=(const char ch);
	DllExport
	const CBotString&
					operator=(const char* pString);
	DllExport
	const CBotString&
					operator+(const CBotString& str);
	DllExport
	friend CBotString 
					operator+(const CBotString& string, LPCTSTR lpsz);

	DllExport
	const CBotString&
					operator+=(const char ch);
	DllExport
	const CBotString& 
					operator+=(const CBotString& str);
	DllExport
	BOOL			operator==(const CBotString& str);
	DllExport
	BOOL			operator==(const char* p);
	DllExport
	BOOL			operator!=(const CBotString& str);
	DllExport
	BOOL			operator!=(const char* p);
	DllExport
	BOOL			operator>(const CBotString& str);
	DllExport
	BOOL			operator>(const char* p);
	DllExport
	BOOL			operator>=(const CBotString& str);
	DllExport
	BOOL			operator>=(const char* p);
	DllExport
	BOOL			operator<(const CBotString& str);
	DllExport
	BOOL			operator<(const char* p);
	DllExport
	BOOL			operator<=(const CBotString& str);
	DllExport
	BOOL			operator<=(const char* p);

	DllExport
					operator LPCTSTR() const;           // as a C string

	int				Compare(LPCTSTR lpsz) const;

	DllExport
	CBotString		Mid(int start, int lg=-1);

	DllExport
	void			MakeUpper();
	DllExport
	void			MakeLower();
};


// idem avec la gestion en tableau

class CBotStringArray : public CBotString
{
private:
	int				m_nSize;					// nombre d'�l�ments
	int				m_nMaxSize;					// taille r�serv�e
	CBotString*		m_pData;					// ^aux donn�es

public:
	DllExport
					CBotStringArray();
	DllExport
					~CBotStringArray();
	DllExport
	void			SetSize(int nb);
	DllExport
	int				GivSize();
	DllExport
	void			Add(const CBotString& str);
	DllExport
	CBotString&		operator[](int nIndex);

	DllExport
	CBotString&		ElementAt(int nIndex);
};

// diff�rents mode pour GetPosition
enum CBotGet
{
	GetPosExtern	= 1,
	GetPosNom		= 2,
	GetPosParam		= 3,
	GetPosBloc		= 4
};

////////////////////////////////////////////////////////////////////
// classe principale g�rant un programme CBot
//

class CBotProgram
{
private:
	CBotFunction*	m_Prog;			// les fonctions d�finies par l'utilisateur
	CBotFunction*	m_pRun;			// la fonction de base pour l'ex�cution
	CBotClass*		m_pClass;		// les classes d�finies dans cette partie
	CBotStack*		m_pStack;		// la pile d'ex�cution
	CBotVar*		m_pInstance;	// instance de la classe parent
	friend class	CBotFunction;

	int				m_ErrorCode;
	int				m_ErrorStart;
	int				m_ErrorEnd;

	long			m_Ident;		// identificateur associ�

public:
	static
	CBotString		m_DebugVarStr;	// a fin de debug
	BOOL			m_bDebugDD;		// idem d�clanchable par robot

	BOOL			m_bCompileClass;

public:
	DllExport
	static
	void			Init();
	//				initialise le module (d�fini les mots clefs pour les erreurs)
	//				doit �tre fait une fois (et une seule) au tout d�but
	DllExport
	static
	void			Free();
	//				lib�re les zones m�moires statiques

	DllExport
	static
	int				GivVersion();
	//				donne la version de la librairie CBOT


	DllExport
					CBotProgram();
	DllExport
					CBotProgram(CBotVar* pInstance);
	DllExport
					~CBotProgram();

	DllExport
	BOOL			Compile( const char* program, CBotStringArray& ListFonctions, void* pUser = NULL);
	//				compile le programme donn� en texte
	//				retourne FALSE s'il y a une erreur � la compilation
	//				voir GetCompileError() pour r�cup�rer l'erreur
	//				ListFonctions retourne le nom des fonctions d�clar�es extern
	//				pUser permet de passer un pointeur pour les routines d�finies par AddFunction

	DllExport
	void			SetIdent(long n);
	//				associe un identificateur avec l'instance CBotProgram

	DllExport
	long			GivIdent();
	//				redonne l'identificateur

	DllExport
	int				GivError();
	DllExport
	BOOL			GetError(int& code, int& start, int& end);
	DllExport
	BOOL			GetError(int& code, int& start, int& end, CBotProgram* &pProg);
	//				si TRUE
	//				donne l'erreur trouv�e � la compilation
	//				ou � l'ex�cution
	//				start et end d�limite le bloc o� se trouve l'erreur
	//				pProg permet de savoir dans quel "module" s'est produite l'erreur d'ex�cution
	DllExport
	static
	CBotString		GivErrorText(int code);


	DllExport
	BOOL			Start(const char* name);
	//				d�finie quelle fonction doit �tre ex�cut�e
	//				retourne FALSE si la fontion name n'est pas trouv�e
	//				le programme ne fait rien, il faut appeller Run() pour cela

	DllExport
	BOOL			Run(void* pUser = NULL, int timer = -1);
	//				ex�cute le programme
	//				retourne FALSE si le programme a �t� suspendu
	//				retourne TRUE si le programme s'est termin� avec ou sans erreur
	//				timer = 0 permet de faire une avance pas � pas

	DllExport
	BOOL			GetRunPos(const char* &FunctionName, int &start, int &end);
	//				donne la position dans le programme en ex�cution
	//				retourne FALSE si on n'est pas en ex�cution (programme termin�)
	//				FunctionName est un pointeur rendu sur le nom de la fonction
	//				start et end la position dans le texte du token en traitement

	DllExport
	CBotVar*		GivStackVars(const char* &FunctionName, int level);
	//				permet d'obtenir le pointeur aux variables sur la pile d'ex�cution
	//				level est un param�tre d'entr�e, 0 pour le dernier niveau, -1, -2, etc pour les autres niveau
	//				la valeur retourn�e (CBotVar*) est une liste de variable (ou NULL)
	//					qui peut �tre trait� que la liste des param�tres re�u par une routine
	//				FunctionName donne le nom de la fonction o� se trouvent ces variables
	//				FunctionName == NULL signifiant qu'on est plus dans le programme (selon level)

	DllExport
	void			Stop();
	//				arr�te l'ex�cution du programme
	//				quitte donc le mode "suspendu"

	DllExport
	static
	void			SetTimer(int n);
	//				d�fini le nombre de pas (parties d'instructions) � faire
	//				dans Run() avant de rendre la main "FALSE"

	DllExport
	static
	BOOL			AddFunction(const char* name, 
								BOOL rExec (CBotVar* pVar, CBotVar* pResult, int& Exception, void* pUser), 
								CBotTypResult rCompile (CBotVar* &pVar, void* pUser));
	//				cet appel permet d'ajouter de mani�re externe (**)
	//				une nouvelle fonction utilisable par le programme CBot

	DllExport
	static
	BOOL			DefineNum(const char* name, long val);

	DllExport
	BOOL			SaveState(FILE* pf);
	//				sauvegarde l'�tat d'ex�cution dans le fichier
	//				le fichier doit avoir �t� ouvert avec l'appel fopen de cette dll
	//				sinon le syst�me plante
	DllExport
	BOOL			RestoreState(FILE* pf);
	//				r�tablie l'�tat de l'ex�cution depuis le fichier
	//				le programme compil� doit �videmment �tre identique

	DllExport
	BOOL			GetPosition(const char* name, int& start, int& stop, 
								CBotGet modestart = GetPosExtern, 
								CBotGet modestop  = GetPosBloc);
	//				donne la position d'une routine dans le texte d'origine
	//				le mode permet de choisir l'�l�ment � trouver pour le d�but et la fin
	//				voir les modes ci-dessus dans CBotGet


	CBotFunction*	GivFunctions();
};


///////////////////////////////////////////////////////////////////////////////
// routines pour la gestion d'un fichier (FILE*)
	DllExport
	FILE*		fOpen(const char* name, const char* mode);
	DllExport
	int			fClose(FILE* filehandle);
	DllExport
	size_t		fWrite(const void *buffer, size_t elemsize, size_t length, FILE* filehandle);
	DllExport
	size_t		fRead(void *buffer, size_t elemsize, size_t length, FILE* filehandle);


#if 0
/*
(**) Note:
	 Pour d�finir une fonction externe, il faut proc�der ainsi:

	a) d�finir une routine pour la compilation
		cette routine re�ois la liste des param�tres (sans valeurs)
		et retourne soit un type de r�sultat (CBotTyp... ou 0 = void)
		soit un num�ro d'erreur
	b) d�finir une routine pour l'ex�cution
		cette rourine reCoit la liste des param�tres (avec valeurs),
		une variable pour stocker le r�sultat (selon le type donn� � la compilation)

	Par exemple, une routine qui calcule la moyenne d'une liste de param�tres */

int	cMoyenne(CBotVar* &pVar, CBotString& ClassName)
{
	if ( pVar == NULL ) return 6001;	// il n'y a aucun param�tre !

	while ( pVar != NULL )
	{
		if ( pVar->GivType() > CBotTypDouble ) return 6002;		// ce n'est pas un nombre
		pVar = pVar -> GivNext();
	}

	return CBotTypFloat;		// le type du r�sultat pourrait d�pendre des param�tres !
}


BOOL rMoyenne(CBotVar* pVar, CBotVar* pResult, int& Exception)
{
	float total = 0;
	int   nb	  = 0;
	while (pVar != NULL)
	{
		total += pVar->GivValFloat();
		pVar = pVar->GivNext();
		nb++;
	}
	pResult->SetValFloat(total/nb);				// retourne la valeur moyenne

	return TRUE;								// op�ration totalement termin�e
}

#endif

/////////////////////////////////////////////////////////////////////////////////
// Classe pour la gestion des variables

// les m�thodes marqu�es DllExport 
// peuvent �tre utile � l'exterieur du module
// ( il n'est pour l'instant pas pr�vu de pouvoir cr�er ces objets en externe )

// r�sultats pour GivInit()
#define	IS_UNDEF	0		// variable ind�finie
#define IS_DEF		1		// variable d�finie
#define	IS_NAN		999		// variable d�finie comme �tant not a number

// type de variable SetPrivate / IsPrivate
#define PR_PUBLIC	0		// variable publique
#define	PR_READ		1		// read only
#define PR_PROTECT	2		// protected (h�ritage)
#define PR_PRIVATE	3		// strictement priv�e

class CBotVar
{
protected:
	CBotToken*		m_token;					// le token correspondant

	CBotVar*		m_next;						// liste de variables
	friend class	CBotStack;
	friend class	CBotCStack;
	friend class	CBotInstrCall;
	friend class	CBotProgram;

	CBotTypResult	m_type;						// type de valeur

	int				m_binit;					// pas initialis�e ?
	CBotVarClass*	m_pMyThis;					// ^�l�ment this correspondant
	void*			m_pUserPtr;					// ^donn�es user s'il y a lieu
	BOOL			m_bStatic;					// �l�ment static (dans une classe)
	int				m_mPrivate;					// �l�ment public, protected ou private ?

	CBotInstr*		m_InitExpr;					// expression pour le contenu initial
	CBotInstr*		m_LimExpr;					// liste des limites pour un tableau
	friend class	CBotClass;
	friend class	CBotVarClass;
	friend class	CBotVarPointer;
	friend class	CBotVarArray;

	long			m_ident;					// identificateur unique
	static long		m_identcpt;					// compteur

public:
					CBotVar();
virtual				~CBotVar( );						// destructeur


/*	DllExport
	static
	CBotVar*		Create( const char* name, int type, const char* ClassName = NULL);
	//				cr�e une variable selon son type,*/

	DllExport
	static
	CBotVar*		Create( const char* name, CBotTypResult type);
	//				idem � partir du type complet

	DllExport
	static
	CBotVar*		Create( const char* name, CBotClass* pClass);
	//				idem pour une instance d'une classe connue

	static
	CBotVar*		Create( const CBotToken* name, int type );
	static
	CBotVar*		Create( const CBotToken* name, CBotTypResult type );

	static
	CBotVar*		Create( const char* name, int type, CBotClass* pClass);

	static
	CBotVar*		Create( CBotVar* pVar );


	DllExport
	void			SetUserPtr(void* pUser);
	//				associe un pointeur utilisateur � une instance

	DllExport
	virtual void	SetIdent(long UniqId);
	//				associe un identificateur unique � une instance
	//				( c'est � l'utilisateur de s'assurer que l'id est unique)

	DllExport
	void*			GivUserPtr();
	//				rend le pointeur associ� � la variable

	DllExport
	CBotString		GivName();					// le nom de la variable, s'il est connu
	////////////////////////////////////////////////////////////////////////////////////
	void			SetName(const char* name);	// change le nom de la variable

	DllExport
	int				GivType(int mode = 0);		// rend le type de base (int) de la variable
	////////////////////////////////////////////////////////////////////////////////////////

	DllExport
	CBotTypResult	GivTypResult(int mode = 0);	// rend le type complet de la variable


	CBotToken*		GivToken();
	void			SetType(CBotTypResult& type);

	DllExport
	void			SetInit(int bInit);			// met la variable dans l'�tat IS_UNDEF, IS_DEF, IS_NAN

	DllExport
	int				GivInit();					// donne l'�tat de la variable

	DllExport
	void			SetStatic(BOOL bStatic);
	DllExport
	BOOL			IsStatic();

	DllExport
	void			SetPrivate(int mPrivate);
	DllExport
	BOOL			IsPrivate(int mode = PR_PROTECT);
	DllExport
	int				GivPrivate();

	virtual
	void			ConstructorSet();

	void			SetVal(CBotVar* var);		// remprend une valeur

	DllExport
	virtual
	CBotVar*		GivItem(const char* name);	// rend un �l�ment d'une classe selon son nom (*)
	virtual
	CBotVar*		GivItemRef(int nIdent);		// idem � partir du n� ref

	DllExport
	virtual
	CBotVar*		GivItem(int row, BOOL bGrow = FALSE); 

	DllExport
	virtual
	CBotVar*		GivItemList();				// donne la liste des �l�ments

	DllExport
	CBotVar*		GivStaticVar();				// rend le pointeur � la variable si elle est statique

	DllExport
	BOOL			IsElemOfClass(const char* name);
												// dit si l'�l�ment appartient � la classe "name"
												// rend TRUE si l'objet est d'une classe fille

	DllExport
	CBotVar*		GivNext();					// prochaine variable dans la liste (param�tres)
	////////////////////////////////////////////////////////////////////////////////////////////

	void			AddNext(CBotVar* pVar);		// ajoute dans une liste

	virtual
	void			Copy(CBotVar* pSrc, BOOL bName = TRUE);	// fait une copie de la variable

	DllExport
	virtual void	SetValInt(int val, const char* name = NULL);
												// initialise avec une valeur enti�re (#)
	/////////////////////////////////////////////////////////////////////////////////

	DllExport
	virtual void	SetValFloat(float val);		// initialise avec une valeur r�elle (#)
	////////////////////////////////////////////////////////////////////////////////

	DllExport
	virtual void	SetValString(const char* p);// initialise avec une valeur cha�ne (#)
	////////////////////////////////////////////////////////////////////////////////

	DllExport
	virtual int		GivValInt();				// demande la valeur enti�re (#)
	////////////////////////////////////////////////////////////////////////

	DllExport
	virtual float	GivValFloat();				// demande la valeur r�elle (#)
	///////////////////////////////////////////////////////////////////////

	virtual
	CBotString 		GivValString();				// demande la valeur cha�ne (#)
	///////////////////////////////////////////////////////////////////////

	virtual void	SetClass(CBotClass* pClass);
	virtual
	CBotClass*		GivClass();

	virtual void	SetPointer(CBotVar* p);
	virtual
	CBotVarClass*	GivPointer();
//	virtual void	SetIndirection(CBotVar* pVar);

	virtual void	Add(CBotVar* left, CBotVar* right);	// addition
	virtual void	Sub(CBotVar* left, CBotVar* right);	// soustraction
	virtual void	Mul(CBotVar* left, CBotVar* right);	// multiplication
	virtual int 	Div(CBotVar* left, CBotVar* right);	// division
	virtual int		Modulo(CBotVar* left, CBotVar* right);	// reste de division
	virtual void	Power(CBotVar* left, CBotVar* right);	// puissance

	virtual BOOL	Lo(CBotVar* left, CBotVar* right);
	virtual BOOL	Hi(CBotVar* left, CBotVar* right);
	virtual BOOL	Ls(CBotVar* left, CBotVar* right);
	virtual BOOL	Hs(CBotVar* left, CBotVar* right);
	virtual BOOL	Eq(CBotVar* left, CBotVar* right);
	virtual BOOL	Ne(CBotVar* left, CBotVar* right);

	virtual void	And(CBotVar* left, CBotVar* right);
	virtual void	Or(CBotVar* left, CBotVar* right);
	virtual void	XOr(CBotVar* left, CBotVar* right);
	virtual void	ASR(CBotVar* left, CBotVar* right);
	virtual void	SR(CBotVar* left, CBotVar* right);
	virtual void	SL(CBotVar* left, CBotVar* right);

	virtual void	Neg();
	virtual void	Not();
	virtual void	Inc();
	virtual void	Dec();


	virtual BOOL	Save0State(FILE* pf);
	virtual BOOL	Save1State(FILE* pf);
	static	BOOL	RestoreState(FILE* pf, CBotVar* &pVar);

	DllExport
	void			debug();

//	virtual
//	CBotVar*		GivMyThis();

	DllExport
	virtual
	void			Maj(void* pUser = NULL, BOOL bContinue = TRUE);

	void			SetUniqNum(long n);
	long			GivUniqNum();
	static long		NextUniqNum();
};

/* NOTE (#)
	les m�thodes	SetValInt() SetValFloat() et SetValString()
	ne peuvent �tes appell�es qu'avec des objets respectivement entier, r�elle ou cha�ne
	toujours s'assurer du type de la variable avant d'appeller ces m�thodes

	if ( pVar->GivType() == CBotInt() ) pVar->SetValFloat( 3.3 ); // plante !!

	les m�thodes	GivValInt(), GivValFloat() et GivValString()
	font des conversions de valeur,
	GivValString() fonctionne sur des nombres (rend la cha�ne correspondante)
	par contre il ne faut pas faire de GivValInt() avec une variable de type cha�ne !
*/



////////////////////////////////////////////////////////////////////////
// Gestion des classes
////////////////////////////////////////////////////////////////////////

// classe pour d�finir de nouvelle classes dans le language CBOT
// par exemple pour d�finir la classe CPoint (x,y)

class CBotClass
{
private:
	static
	CBotClass*		m_ExClass;		// liste des classes existante � un moment donn�
	CBotClass*		m_ExNext;		// pour cette liste g�n�rale
	CBotClass*		m_ExPrev;		// pour cette liste g�n�rale

private:
	CBotClass*		m_pParent;		// classe parent
	CBotString		m_name;			// nom de cette classe-ci
	int				m_nbVar;		// nombre de variables dans la cha�ne
	CBotVar*		m_pVar;			// contenu de la classe
	BOOL			m_bIntrinsic;	// classe intrins�que
	CBotClass*		m_next;			// chaine les classe
	CBotCallMethode* m_pCalls;		// liste des m�thodes d�finie en externe
	CBotFunction*	m_pMethod;		// liste des m�thodes compil�es
	void			(*m_rMaj) ( CBotVar* pThis, void* pUser );
	friend class	CBotVarClass;
	int				m_cptLock;		// pour Lock / UnLock
	int				m_cptOne;		// pour r�entrance Lock
	CBotProgram*	m_ProgInLock[5];// processus en attente pour synchro

public:
	BOOL			m_IsDef;		// marque si est d�finie ou pas encore

	DllExport
					CBotClass( const char* name,
							   CBotClass* pParent, BOOL bIntrinsic = FALSE );		// constructeur
	//				D�s qu'une classe est cr��e, elle est connue
	//				partout dans CBot
	//				le mode intrinsic donne une classe qui n'est pas g�r�e par des pointeurs

	DllExport
					~CBotClass( );							// destructeur

	DllExport
	BOOL			AddFunction(const char* name, 
								BOOL rExec (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception), 
								CBotTypResult rCompile (CBotVar* pThis, CBotVar* &pVar));
	//				cet appel permet d'ajouter de mani�re externe (**)
	//				une nouvelle m�thode utilisable par les objets de cette classe

	DllExport
	BOOL			AddUpdateFunc( void rMaj ( CBotVar* pThis, void* pUser ) );
	//				d�fini la routine qui sera appell�e pour mettre � jour les �lements de la classe

	DllExport
	BOOL			AddItem(CBotString name, CBotTypResult type, int mPrivate = PR_PUBLIC);
	//				ajoute un �l�ment � la classe
//	DllExport
//	BOOL			AddItem(CBotString name, CBotClass* pClass);
	//				idem pour des �l�ments appartenant � pClass
	DllExport
	BOOL			AddItem(CBotVar* pVar);
	//				idem en passant le pointeur � une instance d'une variable
	//				l'objet est pris tel quel, il ne faut donc pas le d�truire



	// idem en donnant un �l�ment de type CBotVar
	void			AddNext(CBotClass* pClass);

	DllExport
	CBotString		GivName();					// rend le nom de la classe
	DllExport
	CBotClass*		GivParent();				// donne la classe p�re (ou NULL)

	// dit si une classe est d�riv�e (Extends) d'une autre
	// rend TRUE aussi si les classes sont identiques
	DllExport
	BOOL			IsChildOf(CBotClass* pClass);

	static
	CBotClass*		Find(CBotToken* &pToken);	// trouve une classe d'apr�s son nom

	DllExport
	static
	CBotClass*		Find(const char* name);

	CBotVar*		GivVar();					// rend la liste des variables
	CBotVar*		GivItem(const char* name);	// l'une des variables selon son nom
	CBotVar*		GivItemRef(int nIdent);

	CBotTypResult	CompileMethode(const char* name, CBotVar* pThis, CBotVar** ppParams, 
								   CBotCStack* pStack, long& nIdent);

	BOOL			ExecuteMethode(long& nIdent, const char* name, CBotVar* pThis, CBotVar** ppParams, CBotVar* &pResult, CBotStack* &pStack, CBotToken* pToken);
	void			RestoreMethode(long& nIdent, const char* name, CBotVar* pThis, CBotVar** ppParams, CBotStack* &pStack);

	// compile une classe d�clar�e par l'utilisateur
	static
	CBotClass*		Compile(CBotToken* &p, CBotCStack* pStack);
	static
	CBotClass*		Compile1(CBotToken* &p, CBotCStack* pStack);

	BOOL			CompileDefItem(CBotToken* &p, CBotCStack* pStack, BOOL bSecond);
	
	BOOL			IsIntrinsic();
	void			Purge();
	static
	void			Free();

	DllExport
	static
	BOOL			SaveStaticState(FILE* pf);

	DllExport
	static
	BOOL			RestoreStaticState(FILE* pf);

	BOOL			Lock(CBotProgram* p);
	void			Unlock();
	static
	void			FreeLock(CBotProgram* p);

	BOOL			CheckCall(CBotToken* &pToken, CBotDefParam* pParam);

};

#define	MAXDEFNUM			1000				// nombre limite des DefineNum

/////////////////////////////////////////////////////////////////////////////////////
// gestion des jetons (tokens)

#define	TokenTypKeyWord		1					// un mot clef du language (voir TokenKeyWord)
#define	TokenTypNum			2					// un nombre
#define TokenTypString		3					// une chaine
#define TokenTypVar			4					// un nom de variable
#define TokenTypDef			5					// une valeur selon DefineNum

#define TokenKeyWord		2000				// les mots clefs du langage
#define TokenKeyDeclare		2100				// mots clefs pour d�clarations (int, float,..)
#define TokenKeyVal			2200				// les mots repr�sentant une "valeur" (true, false, null, nan)
#define TokenKeyOp			2300				// les op�rateurs


class CBotToken
{
private:
	static
	CBotStringArray	m_ListKeyWords;				// liste des mots clefs du language
	static
	int				m_ListIdKeyWords[200];		// les codes correspondants

	static
	CBotStringArray	m_ListKeyDefine;			// les noms d�finis par un DefineNum
	static
	long			m_ListKeyNums[MAXDEFNUM];	// les valeurs associ�es

private:
	CBotToken*		m_next;						// suivant dans la liste
	CBotToken*		m_prev;
	int				m_type;						// type de Token
	long			m_IdKeyWord;				// num�ro du mot clef si c'en est un
												// ou valeur du "define"

	CBotString		m_Text;						// mot trouv� comme token
	CBotString		m_Sep;						// s�parateurs qui suivent

	int				m_start;					// position dans le texte d'origine (programme)
	int				m_end;						// itou pour la fin du token

	static
	int				GivKeyWords(const char* w);	// est-ce un mot clef ?
	static
	BOOL			GivKeyDefNum(const char* w, CBotToken* &token);

	static
	void			LoadKeyWords();				// fait la liste des mots clefs

public:
					CBotToken();
					CBotToken(const CBotToken* pSrc);
					CBotToken(CBotString& mot, CBotString& sep, int start=0, int end=0);
					CBotToken(const char* mot, const char* sep = NULL);
												// constructeur
					~CBotToken();				// destructeur

	DllExport
	int				GivType();					// rend le type du token

	DllExport
	CBotString&		GivString();				// rend la chaine correspondant � ce token

	DllExport
	CBotString&		GivSep();					// rend le s�parateur suivant le token

	DllExport
	int				GivStart();					// position du d�but dans le texte
	DllExport
	int				GivEnd();					// position de fin dans le texte

	DllExport
	CBotToken*		GivNext();					// rend le suivant dans la liste
	DllExport
	CBotToken*		GivPrev();					// rend le Pr�c�dent dans la liste

	DllExport
	static
	CBotToken*		CompileTokens(const char* p, int& error);
												// transforme tout le programme
	DllExport
	static
	void			Delete(CBotToken* pToken);	// lib�re la liste


	// fonctions non utiles en export
	static
	BOOL			DefineNum(const char* name, long val);
	void			SetString(const char* name);

	void			SetPos(int start, int end);
	long			GivIdKey();
	void			AddNext(CBotToken* p);		// ajoute un token (une copie)

	static
	CBotToken*		NextToken(char* &program, int& error, BOOL first = FALSE);
												// trouve le prochain token
	const CBotToken&
					operator=(const CBotToken& src);

	static
	void			Free();
};



#if 0
////////////////////////////////////////////////////////////////////////
// Exemples d'utilisation
// D�finition de classes et de fonctions


// d�finie la classe globale CPoint
// --------------------------------
	m_pClassPoint	= new CBotClass("CPoint", NULL);
	// ajoute le composant ".x"
	m_pClassPoint->AddItem("x", CBotTypResult(CBotTypFloat));
	// ajoute le composant ".y"
	m_pClassPoint->AddItem("y", CBotTypResult(CBotTypFloat));
	// le joueur peut alors utiliser les instructions
	// CPoint position; position.x = 12; position.y = -13.6

// d�finie la classe CColobotObject
// --------------------------------
// cette classe g�re tous les objets dans le monde de COLOBOT
// le programme utilisateur "main" appartient � cette classe
	m_pClassObject	= new CBotClass("CColobotObject", m_pClassBase);
	// ajoute le composant ".position"
	m_pClassObject->AddItem("position", m_pClassPoint);
	// ajoute le composant ".type"
	m_pClassObject->AddItem("type", CBotTypResult(CBotTypShort));
	// ajoute une d�finition de constante
	m_pClassObject->AddConst("ROBOT", CBotTypShort, 1);			// ROBOT �quivalent � la valeur 1
	// ajoute la routine FIND
	m_pClassObject->AddFunction( rCompFind, rDoFind );
	// le joueur peut maintenant utiliser les instructions
	// CColobotObject chose; chose = FIND( ROBOT )



// d�finie la classe CColobotRobot d�riv�e de CColobotObject
// ---------------------------------------------------------
// les programmes "main" associ�s aux robots font partie de cette classe
	m_pClassRobot	= new CBotClass("CColobotRobot", m_pClassObject);
	// ajoute la routine GOTO
	m_pClassRobot->AddFunction( rCompGoto, rDoGoto );
	// le joueur peut maintenant faire
	// GOTO( FIND ( ROBOT ) );


// cr�e une instance de la classe Robot
// ------------------------------------
// par exemple un nouveau robot qui vient d'�tre fabriqu�
	CBotVar*	m_pMonRobot = new CBotVar("MonRobot", m_pClassRobot);

// compile le programme main pour ce robot-l�
// ------------------------------------------
	CString LeProgramme( "void main() {GOTO(0, 0); return 0;}" );
	if ( !m_pMonRobot->Compile( LeProgramme ) ) {gestion d'erreur...};

// construit une pile pour l'interpr�teur
// --------------------------------------
	CBotStack*	pStack = new CBotStack(NULL);

// ex�cute le programme main
// -------------------------
	while( FALSE = m_pMonRobot->Execute( "main", pStack ))
	{
		// programme suspendu
		// on pourrait passer la main � un autre (en sauvegardant pStack pour ce robot-l�)
	};
	// programme "main" termin� !




// routine impl�mentant l'instruction GOTO( CPoint pos )
BOOL rDoGoto( CBotVar* pVar, CBotVar* pResult, int& exception )
{
	if (pVar->GivType() != CBotTypeClass ||
		pVar->IsElemOfClas("CPoint") ) { exception = 6522; return FALSE; )
		// le param�tre n'est pas de la bonne classe ?
		// NB en fait ce contr�le est d�j� fait par la routine pour la compilation

	m_PosToGo.Copy( pVar );				// garde la position � atteindre (object type CBotVar)

	// ou alors
	CBotVar*	temp;
	temp = pVar->GivItem("x");			// trouve forc�ment pour un object de type "CPoint"
	ASSERT (temp != NULL && temp->GivType() == CBotTypFloat);
	m_PosToGo.x = temp->GivValFloat();

	temp = pVar->GivItem("y");			// trouve forc�ment pour un object de type "CPoint"
	ASSERT (temp != NULL && temp->GivType() == CBotTypFloat);
	m_PosToGo.y = temp->GivValFloat();

	return (m_CurentPos == m_PosToGo);	// rend TRUE si la position est atteinte
										// rend FALSE s'il faut patienter encore
}

#endif
