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
// * along with this program. If not, see  http://www.gnu.org/licenses/.////////////////////////////////////////////////////////////////////////
// Librairie pour l'interprétation du language CBOT
// pour le jeu COLOBOT
//

//#include "stdafx.h"

#include <windows.h>
#include <stdio.h>

#define DllExport    __declspec( dllexport )

#define CBOTVERSION 104

////////////////////////////////////////////////////////////////////////
// quelques classes définies par ailleurs

class CBotToken;        // programme transformé en "jetons"
class CBotStack;        // pile pour l'exécution
class CBotClass;        // classe d'object
class CBotInstr;        // instruction à exécuter
class CBotFunction;     // les fonctions user
class CBotVar;          // les variables
class CBotVarClass;     // une instance de classe
class CBotVarPointer;   // pointeur à une instance de classe
class CBotCall;         // les fonctions
class CBotCallMethode;  // les méthodes
class CBotDefParam;     // liste de paramètres
class CBotCStack;


////////////////////////////////////////////////////////////////////////
// Gestion des variables
////////////////////////////////////////////////////////////////////////

// ces types sont calqués sur les types Java
// ne pas changer l'ordre de ces types

enum CBotType
{
    CBotTypVoid         = 0,                // fonction retournant void
    CBotTypByte         = 1,    //n         // nombre entier ( 8 bits)
    CBotTypShort        = 2,    //n         // nombre entier (16 bits)
    CBotTypChar         = 3,    //n         // caractère "unicode" (16 bits)
    CBotTypInt          = 4,                // nombre entier (32 bits)
    CBotTypLong         = 5,    //n         // nombre entier (64 bits)
    CBotTypFloat        = 6,                // nombre décimal (32 bits)
    CBotTypDouble       = 7,    //n         // nombre décimal (64 bits)
    CBotTypBoolean      = 8,                // true ou false exclusivement
    CBotTypString       = 9,                // chaine de caractère

    CBotTypArrayPointer = 10,               // un tableau de variables
    CBotTypArrayBody    = 11,               // idem mais crée l'instance

    CBotTypPointer      = 12,               // pointeur à une instance
    CBotTypNullPointer  = 13,               // pointeur null est spécial

    CBotTypClass        = 15,               // instance d'une classe
    CBotTypIntrinsic    = 16                // instance d'une classe intrinsèque
};
            //n = non encore implémenté

// pour SetUserPtr lors de la suppression d'un objet
#define OBJECTDELETED ((void*)-1)
// valeur mise avant initialisation
#define OBJECTCREATED ((void*)-2)


// classe permettant de définir le type complet d'un résultat
class CBotTypResult
{
private:
    int             m_type;
    CBotTypResult*  m_pNext;    // pour les types de types
    CBotClass*      m_pClass;   // pour les dérivés de classe
    int             m_limite;   // limitation des tableaux
    friend class    CBotVarClass;
    friend class    CBotVarPointer;

public:
    // divers constructeurs selon les besoins
    DllExport
                CBotTypResult(int type);
                // pour les types simples (CBotTypInt à CBotTypString)
    DllExport
                CBotTypResult(int type, const char* name);
                // pour les types pointeur et classe intrinsic
    DllExport
                CBotTypResult(int type, CBotClass* pClass);
                // idem à partir de l'instance d'une classe
    DllExport
                CBotTypResult(int type, CBotTypResult elem);
                // pour les tableaux de variables

    DllExport
                CBotTypResult(const CBotTypResult& typ);
                // pour les assignations
    DllExport
                CBotTypResult();
                // pour par défaut
    DllExport
                ~CBotTypResult();

    DllExport
    int         GivType(int mode = 0) const;
                // rend le type CBotTyp* du résultat

    void        SetType(int n);
                // modifie le type

    DllExport
    CBotClass*  GivClass() const;
                // rend le pointeur à la classe (pour les CBotTypClass, CBotTypPointer)

    DllExport
    int         GivLimite() const;
                // rend la taille limite du tableau (CBotTypArray)

    DllExport
    void        SetLimite(int n);
                // fixe une limite au tableau

    void        SetArray(int* max );
                // idem avec une liste de dimension (tableaux de tableaux)

    DllExport
    CBotTypResult& GivTypElem() const;
                // rend le type des éléments du tableau (CBotTypArray)

    DllExport
    BOOL        Compare(const CBotTypResult& typ) const;
                // compare si les types sont compatibles
    DllExport
    BOOL        Eq(int type) const;
                // compare le type

    DllExport
    CBotTypResult&
                operator=(const CBotTypResult& src);
                // copie un type complet dans un autre
};

/*
// pour définir un résultat en sortie, utiliser par exemple

    // pour rendre un simple Float
    return CBotTypResult( CBotTypFloat );


    // pour rendre un tableau de string
    return CBotTypResult( CBotTypArray, CBotTypResult( CBotTypString ) );

    // pour rendre un tableau de tableau de "point"
    CBotTypResult   typPoint( CBotTypIntrinsic, "point" );
    CBotTypResult   arrPoint( CBotTypArray, typPoint );
    return  CBotTypResult( CBotTypArray, arrPoint );
*/


////////////////////////////////////////////////////////////////////////
// Gestion des erreurs compilation et exécution
////////////////////////////////////////////////////////////////////////

// voici la liste des erreurs pouvant être retournées par le module
// pour la compilation

#define CBotErrOpenPar          5000    // manque la parenthèse ouvrante
#define CBotErrClosePar         5001    // manque la parenthèse fermante
#define CBotErrNotBoolean       5002    // l'expression doit être un boolean
#define CBotErrUndefVar         5003    // variable non déclarée
#define CBotErrBadLeft          5004    // assignation impossible ( 5 = ... )
#define CBotErrNoTerminator     5005    // point-virgule attendu
#define CBotErrCaseOut          5006    // case en dehors d'un switch
//  CBotErrNoTerm           5007, plus utile
#define CBotErrCloseBlock       5008    // manque " } "
#define CBotErrElseWhitoutIf    5009    // else sans if correspondant
#define CBotErrOpenBlock        5010    // manque " { "
#define CBotErrBadType1         5011    // mauvais type pour l'assignation
#define CBotErrRedefVar         5012    // redéfinition de la variable
#define CBotErrBadType2         5013    // 2 opérandes de type incompatibles
#define CBotErrUndefCall        5014    // routine inconnue
#define CBotErrNoDoubleDots     5015    // " : " attendu
//  CBotErrWhile            5016, plus utile
#define CBotErrBreakOutside     5017    // break en dehors d'une boucle
#define CBotErrUndefLabel       5019    // label inconnu
#define CBotErrLabel            5018    // label ne peut se mettre ici
#define CBotErrNoCase           5020    // manque " case "
#define CBotErrBadNum           5021    // nombre attendu
#define CBotErrVoid             5022    // " void " pas possible ici
#define CBotErrNoType           5023    // déclaration de type attendue
#define CBotErrNoVar            5024    // nom de variable attendu
#define CBotErrNoFunc           5025    // nom de fonction attendu
#define CBotErrOverParam        5026    // trop de paramètres
#define CBotErrRedefFunc        5027    // cette fonction existe déjà
#define CBotErrLowParam         5028    // pas assez de paramètres
#define CBotErrBadParam         5029    // mauvais types de paramètres
#define CBotErrNbParam          5030    // mauvais nombre de paramètres
#define CBotErrUndefItem        5031    // élément n'existe pas dans la classe
#define CBotErrUndefClass       5032    // variable n'est pas une classe
#define CBotErrNoConstruct      5033    // pas de constructeur approprié
#define CBotErrRedefClass       5034    // classe existe déjà
#define CBotErrCloseIndex       5035    // " ] " attendu
#define CBotErrReserved         5036    // mot réservé (par un DefineNum)
#define CBotErrBadNew           5037    // mauvais paramètre pour new
#define CBotErrOpenIndex        5038    // " [ " attendu
#define CBotErrBadString        5039    // chaîne de caractère attendue
#define CBotErrBadIndex         5040    // mauvais type d'index "[ false ]"
#define CBotErrPrivate          5041    // élément protégé
#define CBotErrNoPublic         5042    // manque le mot "public"

// voici la liste des erreurs pouvant être retournées par le module
// pour l'exécution

#define CBotErrZeroDiv          6000    // division par zéro
#define CBotErrNotInit          6001    // variable non initialisée
#define CBotErrBadThrow         6002    // throw d'une valeur négative
#define CBotErrNoRetVal         6003    // fonction n'a pas retourné de résultat
#define CBotErrNoRun            6004    // Run() sans fonction active
#define CBotErrUndefFunc        6005    // appel d'une fonction qui n'existe plus
#define CBotErrNotClass         6006    // cette classe n'existe pas
#define CBotErrNull             6007    // pointeur null
#define CBotErrNan              6008    // calcul avec un NAN
#define CBotErrOutArray         6009    // index hors du tableau
#define CBotErrStackOver        6010    // dépassement de la pile
#define CBotErrDeletedPtr       6011    // pointeur à un objet détruit

#define CBotErrFileOpen         6012    // ouverture du fichier impossible
#define CBotErrNotOpen          6013    // canal pas ouvert
#define CBotErrRead             6014    // erreur à la lecture
#define CBotErrWrite            6015    // erreur à l'écriture

// d'autres valeurs peuvent être rendues
// par exemple les exceptions rendues par les routines externes
// et les " throw " avec un nombre quelconque.


////////////////////////////////////////////////////////////////////////
// définie une classe pour l'utilisation des strings
// car CString fait partie de MFC pas utilisé ici.
//
// ( toutes les fonctions ne sont pas encore implémentées )

class CBotString
{
private:
    char*           m_ptr;                      // pointeur à la chaine
    int             m_lg;                       // longueur de la chaine
    static
    HINSTANCE       m_hInstance;

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
    void            Empty();
    DllExport
    BOOL            IsEmpty() const;
    DllExport
    int             GivLength();
    DllExport
    int             Find(const char c);
    DllExport
    int             Find(LPCTSTR lpsz);
    DllExport
    int             ReverseFind(const char c);
    DllExport
    int             ReverseFind(LPCTSTR lpsz);
    DllExport
    BOOL            LoadString(UINT id);
    DllExport
    CBotString      Mid(int nFirst, int nCount) const;
    DllExport
    CBotString      Mid(int nFirst) const;
    DllExport
    CBotString      Left(int nCount) const;
    DllExport
    CBotString      Right(int nCount) const;

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
    BOOL            operator==(const CBotString& str);
    DllExport
    BOOL            operator==(const char* p);
    DllExport
    BOOL            operator!=(const CBotString& str);
    DllExport
    BOOL            operator!=(const char* p);
    DllExport
    BOOL            operator>(const CBotString& str);
    DllExport
    BOOL            operator>(const char* p);
    DllExport
    BOOL            operator>=(const CBotString& str);
    DllExport
    BOOL            operator>=(const char* p);
    DllExport
    BOOL            operator<(const CBotString& str);
    DllExport
    BOOL            operator<(const char* p);
    DllExport
    BOOL            operator<=(const CBotString& str);
    DllExport
    BOOL            operator<=(const char* p);

    DllExport
                    operator LPCTSTR() const;           // as a C string

    int             Compare(LPCTSTR lpsz) const;

    DllExport
    CBotString      Mid(int start, int lg=-1);

    DllExport
    void            MakeUpper();
    DllExport
    void            MakeLower();
};


// idem avec la gestion en tableau

class CBotStringArray : public CBotString
{
private:
    int             m_nSize;                    // nombre d'éléments
    int             m_nMaxSize;                 // taille réservée
    CBotString*     m_pData;                    // ^aux données

public:
    DllExport
                    CBotStringArray();
    DllExport
                    ~CBotStringArray();
    DllExport
    void            SetSize(int nb);
    DllExport
    int             GivSize();
    DllExport
    void            Add(const CBotString& str);
    DllExport
    CBotString&     operator[](int nIndex);

    DllExport
    CBotString&     ElementAt(int nIndex);
};

// différents mode pour GetPosition
enum CBotGet
{
    GetPosExtern    = 1,
    GetPosNom       = 2,
    GetPosParam     = 3,
    GetPosBloc      = 4
};

////////////////////////////////////////////////////////////////////
// classe principale gérant un programme CBot
//

class CBotProgram
{
private:
    CBotFunction*   m_Prog;         // les fonctions définies par l'utilisateur
    CBotFunction*   m_pRun;         // la fonction de base pour l'exécution
    CBotClass*      m_pClass;       // les classes définies dans cette partie
    CBotStack*      m_pStack;       // la pile d'exécution
    CBotVar*        m_pInstance;    // instance de la classe parent
    friend class    CBotFunction;

    int             m_ErrorCode;
    int             m_ErrorStart;
    int             m_ErrorEnd;

    long            m_Ident;        // identificateur associé

public:
    static
    CBotString      m_DebugVarStr;  // a fin de debug
    BOOL            m_bDebugDD;     // idem déclanchable par robot

    BOOL            m_bCompileClass;

public:
    DllExport
    static
    void            Init();
    //              initialise le module (défini les mots clefs pour les erreurs)
    //              doit être fait une fois (et une seule) au tout début
    DllExport
    static
    void            Free();
    //              libère les zones mémoires statiques

    DllExport
    static
    int             GivVersion();
    //              donne la version de la librairie CBOT


    DllExport
                    CBotProgram();
    DllExport
                    CBotProgram(CBotVar* pInstance);
    DllExport
                    ~CBotProgram();

    DllExport
    BOOL            Compile( const char* program, CBotStringArray& ListFonctions, void* pUser = NULL);
    //              compile le programme donné en texte
    //              retourne FALSE s'il y a une erreur à la compilation
    //              voir GetCompileError() pour récupérer l'erreur
    //              ListFonctions retourne le nom des fonctions déclarées extern
    //              pUser permet de passer un pointeur pour les routines définies par AddFunction

    DllExport
    void            SetIdent(long n);
    //              associe un identificateur avec l'instance CBotProgram

    DllExport
    long            GivIdent();
    //              redonne l'identificateur

    DllExport
    int             GivError();
    DllExport
    BOOL            GetError(int& code, int& start, int& end);
    DllExport
    BOOL            GetError(int& code, int& start, int& end, CBotProgram* &pProg);
    //              si TRUE
    //              donne l'erreur trouvée à la compilation
    //              ou à l'exécution
    //              start et end délimite le bloc où se trouve l'erreur
    //              pProg permet de savoir dans quel "module" s'est produite l'erreur d'exécution
    DllExport
    static
    CBotString      GivErrorText(int code);


    DllExport
    BOOL            Start(const char* name);
    //              définie quelle fonction doit être exécutée
    //              retourne FALSE si la fontion name n'est pas trouvée
    //              le programme ne fait rien, il faut appeller Run() pour cela

    DllExport
    BOOL            Run(void* pUser = NULL, int timer = -1);
    //              exécute le programme
    //              retourne FALSE si le programme a été suspendu
    //              retourne TRUE si le programme s'est terminé avec ou sans erreur
    //              timer = 0 permet de faire une avance pas à pas

    DllExport
    BOOL            GetRunPos(const char* &FunctionName, int &start, int &end);
    //              donne la position dans le programme en exécution
    //              retourne FALSE si on n'est pas en exécution (programme terminé)
    //              FunctionName est un pointeur rendu sur le nom de la fonction
    //              start et end la position dans le texte du token en traitement

    DllExport
    CBotVar*        GivStackVars(const char* &FunctionName, int level);
    //              permet d'obtenir le pointeur aux variables sur la pile d'exécution
    //              level est un paramètre d'entrée, 0 pour le dernier niveau, -1, -2, etc pour les autres niveau
    //              la valeur retournée (CBotVar*) est une liste de variable (ou NULL)
    //                  qui peut être traité que la liste des paramètres reçu par une routine
    //              FunctionName donne le nom de la fonction où se trouvent ces variables
    //              FunctionName == NULL signifiant qu'on est plus dans le programme (selon level)

    DllExport
    void            Stop();
    //              arrête l'exécution du programme
    //              quitte donc le mode "suspendu"

    DllExport
    static
    void            SetTimer(int n);
    //              défini le nombre de pas (parties d'instructions) à faire
    //              dans Run() avant de rendre la main "FALSE"

    DllExport
    static
    BOOL            AddFunction(const char* name,
                                BOOL rExec (CBotVar* pVar, CBotVar* pResult, int& Exception, void* pUser),
                                CBotTypResult rCompile (CBotVar* &pVar, void* pUser));
    //              cet appel permet d'ajouter de manière externe (**)
    //              une nouvelle fonction utilisable par le programme CBot

    DllExport
    static
    BOOL            DefineNum(const char* name, long val);

    DllExport
    BOOL            SaveState(FILE* pf);
    //              sauvegarde l'état d'exécution dans le fichier
    //              le fichier doit avoir été ouvert avec l'appel fopen de cette dll
    //              sinon le système plante
    DllExport
    BOOL            RestoreState(FILE* pf);
    //              rétablie l'état de l'exécution depuis le fichier
    //              le programme compilé doit évidemment être identique

    DllExport
    BOOL            GetPosition(const char* name, int& start, int& stop,
                                CBotGet modestart = GetPosExtern,
                                CBotGet modestop  = GetPosBloc);
    //              donne la position d'une routine dans le texte d'origine
    //              le mode permet de choisir l'élément à trouver pour le début et la fin
    //              voir les modes ci-dessus dans CBotGet


    CBotFunction*   GivFunctions();
};


///////////////////////////////////////////////////////////////////////////////
// routines pour la gestion d'un fichier (FILE*)
    DllExport
    FILE*       fOpen(const char* name, const char* mode);
    DllExport
    int         fClose(FILE* filehandle);
    DllExport
    size_t      fWrite(const void *buffer, size_t elemsize, size_t length, FILE* filehandle);
    DllExport
    size_t      fRead(void *buffer, size_t elemsize, size_t length, FILE* filehandle);


#if 0
/*
(**) Note:
     Pour définir une fonction externe, il faut procéder ainsi:

    a) définir une routine pour la compilation
        cette routine reçois la liste des paramètres (sans valeurs)
        et retourne soit un type de résultat (CBotTyp... ou 0 = void)
        soit un numéro d'erreur
    b) définir une routine pour l'exécution
        cette rourine reCoit la liste des paramètres (avec valeurs),
        une variable pour stocker le résultat (selon le type donné à la compilation)

    Par exemple, une routine qui calcule la moyenne d'une liste de paramètres */

int cMoyenne(CBotVar* &pVar, CBotString& ClassName)
{
    if ( pVar == NULL ) return 6001;    // il n'y a aucun paramètre !

    while ( pVar != NULL )
    {
        if ( pVar->GivType() > CBotTypDouble ) return 6002;     // ce n'est pas un nombre
        pVar = pVar -> GivNext();
    }

    return CBotTypFloat;        // le type du résultat pourrait dépendre des paramètres !
}


BOOL rMoyenne(CBotVar* pVar, CBotVar* pResult, int& Exception)
{
    float total = 0;
    int   nb      = 0;
    while (pVar != NULL)
    {
        total += pVar->GivValFloat();
        pVar = pVar->GivNext();
        nb++;
    }
    pResult->SetValFloat(total/nb);             // retourne la valeur moyenne

    return TRUE;                                // opération totalement terminée
}

#endif

/////////////////////////////////////////////////////////////////////////////////
// Classe pour la gestion des variables

// les méthodes marquées DllExport
// peuvent être utile à l'exterieur du module
// ( il n'est pour l'instant pas prévu de pouvoir créer ces objets en externe )

// résultats pour GivInit()
#define IS_UNDEF    0       // variable indéfinie
#define IS_DEF      1       // variable définie
#define IS_NAN      999     // variable définie comme étant not a number

// type de variable SetPrivate / IsPrivate
#define PR_PUBLIC   0       // variable publique
#define PR_READ     1       // read only
#define PR_PROTECT  2       // protected (héritage)
#define PR_PRIVATE  3       // strictement privée

class CBotVar
{
protected:
    CBotToken*      m_token;                    // le token correspondant

    CBotVar*        m_next;                     // liste de variables
    friend class    CBotStack;
    friend class    CBotCStack;
    friend class    CBotInstrCall;
    friend class    CBotProgram;

    CBotTypResult   m_type;                     // type de valeur

    int             m_binit;                    // pas initialisée ?
    CBotVarClass*   m_pMyThis;                  // ^élément this correspondant
    void*           m_pUserPtr;                 // ^données user s'il y a lieu
    BOOL            m_bStatic;                  // élément static (dans une classe)
    int             m_mPrivate;                 // élément public, protected ou private ?

    CBotInstr*      m_InitExpr;                 // expression pour le contenu initial
    CBotInstr*      m_LimExpr;                  // liste des limites pour un tableau
    friend class    CBotClass;
    friend class    CBotVarClass;
    friend class    CBotVarPointer;
    friend class    CBotVarArray;

    long            m_ident;                    // identificateur unique
    static long     m_identcpt;                 // compteur

public:
                    CBotVar();
virtual             ~CBotVar( );                        // destructeur


/*  DllExport
    static
    CBotVar*        Create( const char* name, int type, const char* ClassName = NULL);
    //              crée une variable selon son type,*/

    DllExport
    static
    CBotVar*        Create( const char* name, CBotTypResult type);
    //              idem à partir du type complet

    DllExport
    static
    CBotVar*        Create( const char* name, CBotClass* pClass);
    //              idem pour une instance d'une classe connue

    static
    CBotVar*        Create( const CBotToken* name, int type );
    static
    CBotVar*        Create( const CBotToken* name, CBotTypResult type );

    static
    CBotVar*        Create( const char* name, int type, CBotClass* pClass);

    static
    CBotVar*        Create( CBotVar* pVar );


    DllExport
    void            SetUserPtr(void* pUser);
    //              associe un pointeur utilisateur à une instance

    DllExport
    virtual void    SetIdent(long UniqId);
    //              associe un identificateur unique à une instance
    //              ( c'est à l'utilisateur de s'assurer que l'id est unique)

    DllExport
    void*           GivUserPtr();
    //              rend le pointeur associé à la variable

    DllExport
    CBotString      GivName();                  // le nom de la variable, s'il est connu
    ////////////////////////////////////////////////////////////////////////////////////
    void            SetName(const char* name);  // change le nom de la variable

    DllExport
    int             GivType(int mode = 0);      // rend le type de base (int) de la variable
    ////////////////////////////////////////////////////////////////////////////////////////

    DllExport
    CBotTypResult   GivTypResult(int mode = 0); // rend le type complet de la variable


    CBotToken*      GivToken();
    void            SetType(CBotTypResult& type);

    DllExport
    void            SetInit(int bInit);         // met la variable dans l'état IS_UNDEF, IS_DEF, IS_NAN

    DllExport
    int             GivInit();                  // donne l'état de la variable

    DllExport
    void            SetStatic(BOOL bStatic);
    DllExport
    BOOL            IsStatic();

    DllExport
    void            SetPrivate(int mPrivate);
    DllExport
    BOOL            IsPrivate(int mode = PR_PROTECT);
    DllExport
    int             GivPrivate();

    virtual
    void            ConstructorSet();

    void            SetVal(CBotVar* var);       // remprend une valeur

    DllExport
    virtual
    CBotVar*        GivItem(const char* name);  // rend un élément d'une classe selon son nom (*)
    virtual
    CBotVar*        GivItemRef(int nIdent);     // idem à partir du n° ref

    DllExport
    virtual
    CBotVar*        GivItem(int row, BOOL bGrow = FALSE);

    DllExport
    virtual
    CBotVar*        GivItemList();              // donne la liste des éléments

    DllExport
    CBotVar*        GivStaticVar();             // rend le pointeur à la variable si elle est statique

    DllExport
    BOOL            IsElemOfClass(const char* name);
                                                // dit si l'élément appartient à la classe "name"
                                                // rend TRUE si l'objet est d'une classe fille

    DllExport
    CBotVar*        GivNext();                  // prochaine variable dans la liste (paramètres)
    ////////////////////////////////////////////////////////////////////////////////////////////

    void            AddNext(CBotVar* pVar);     // ajoute dans une liste

    virtual
    void            Copy(CBotVar* pSrc, BOOL bName = TRUE); // fait une copie de la variable

    DllExport
    virtual void    SetValInt(int val, const char* name = NULL);
                                                // initialise avec une valeur entière (#)
    /////////////////////////////////////////////////////////////////////////////////

    DllExport
    virtual void    SetValFloat(float val);     // initialise avec une valeur réelle (#)
    ////////////////////////////////////////////////////////////////////////////////

    DllExport
    virtual void    SetValString(const char* p);// initialise avec une valeur chaîne (#)
    ////////////////////////////////////////////////////////////////////////////////

    DllExport
    virtual int     GivValInt();                // demande la valeur entière (#)
    ////////////////////////////////////////////////////////////////////////

    DllExport
    virtual float   GivValFloat();              // demande la valeur réelle (#)
    ///////////////////////////////////////////////////////////////////////

    virtual
    CBotString      GivValString();             // demande la valeur chaîne (#)
    ///////////////////////////////////////////////////////////////////////

    virtual void    SetClass(CBotClass* pClass);
    virtual
    CBotClass*      GivClass();

    virtual void    SetPointer(CBotVar* p);
    virtual
    CBotVarClass*   GivPointer();
//  virtual void    SetIndirection(CBotVar* pVar);

    virtual void    Add(CBotVar* left, CBotVar* right); // addition
    virtual void    Sub(CBotVar* left, CBotVar* right); // soustraction
    virtual void    Mul(CBotVar* left, CBotVar* right); // multiplication
    virtual int     Div(CBotVar* left, CBotVar* right); // division
    virtual int     Modulo(CBotVar* left, CBotVar* right);  // reste de division
    virtual void    Power(CBotVar* left, CBotVar* right);   // puissance

    virtual BOOL    Lo(CBotVar* left, CBotVar* right);
    virtual BOOL    Hi(CBotVar* left, CBotVar* right);
    virtual BOOL    Ls(CBotVar* left, CBotVar* right);
    virtual BOOL    Hs(CBotVar* left, CBotVar* right);
    virtual BOOL    Eq(CBotVar* left, CBotVar* right);
    virtual BOOL    Ne(CBotVar* left, CBotVar* right);

    virtual void    And(CBotVar* left, CBotVar* right);
    virtual void    Or(CBotVar* left, CBotVar* right);
    virtual void    XOr(CBotVar* left, CBotVar* right);
    virtual void    ASR(CBotVar* left, CBotVar* right);
    virtual void    SR(CBotVar* left, CBotVar* right);
    virtual void    SL(CBotVar* left, CBotVar* right);

    virtual void    Neg();
    virtual void    Not();
    virtual void    Inc();
    virtual void    Dec();


    virtual BOOL    Save0State(FILE* pf);
    virtual BOOL    Save1State(FILE* pf);
    static  BOOL    RestoreState(FILE* pf, CBotVar* &pVar);

    DllExport
    void            debug();

//  virtual
//  CBotVar*        GivMyThis();

    DllExport
    virtual
    void            Maj(void* pUser = NULL, BOOL bContinue = TRUE);

    void            SetUniqNum(long n);
    long            GivUniqNum();
    static long     NextUniqNum();
};

/* NOTE (#)
    les méthodes    SetValInt() SetValFloat() et SetValString()
    ne peuvent êtes appellées qu'avec des objets respectivement entier, réelle ou chaîne
    toujours s'assurer du type de la variable avant d'appeller ces méthodes

    if ( pVar->GivType() == CBotInt() ) pVar->SetValFloat( 3.3 ); // plante !!

    les méthodes    GivValInt(), GivValFloat() et GivValString()
    font des conversions de valeur,
    GivValString() fonctionne sur des nombres (rend la chaîne correspondante)
    par contre il ne faut pas faire de GivValInt() avec une variable de type chaîne !
*/



////////////////////////////////////////////////////////////////////////
// Gestion des classes
////////////////////////////////////////////////////////////////////////

// classe pour définir de nouvelle classes dans le language CBOT
// par exemple pour définir la classe CPoint (x,y)

class CBotClass
{
private:
    static
    CBotClass*      m_ExClass;      // liste des classes existante à un moment donné
    CBotClass*      m_ExNext;       // pour cette liste générale
    CBotClass*      m_ExPrev;       // pour cette liste générale

private:
    CBotClass*      m_pParent;      // classe parent
    CBotString      m_name;         // nom de cette classe-ci
    int             m_nbVar;        // nombre de variables dans la chaîne
    CBotVar*        m_pVar;         // contenu de la classe
    BOOL            m_bIntrinsic;   // classe intrinsèque
    CBotClass*      m_next;         // chaine les classe
    CBotCallMethode* m_pCalls;      // liste des méthodes définie en externe
    CBotFunction*   m_pMethod;      // liste des méthodes compilées
    void            (*m_rMaj) ( CBotVar* pThis, void* pUser );
    friend class    CBotVarClass;
    int             m_cptLock;      // pour Lock / UnLock
    int             m_cptOne;       // pour réentrance Lock
    CBotProgram*    m_ProgInLock[5];// processus en attente pour synchro

public:
    BOOL            m_IsDef;        // marque si est définie ou pas encore

    DllExport
                    CBotClass( const char* name,
                               CBotClass* pParent, BOOL bIntrinsic = FALSE );       // constructeur
    //              Dès qu'une classe est créée, elle est connue
    //              partout dans CBot
    //              le mode intrinsic donne une classe qui n'est pas gérée par des pointeurs

    DllExport
                    ~CBotClass( );                          // destructeur

    DllExport
    BOOL            AddFunction(const char* name,
                                BOOL rExec (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception),
                                CBotTypResult rCompile (CBotVar* pThis, CBotVar* &pVar));
    //              cet appel permet d'ajouter de manière externe (**)
    //              une nouvelle méthode utilisable par les objets de cette classe

    DllExport
    BOOL            AddUpdateFunc( void rMaj ( CBotVar* pThis, void* pUser ) );
    //              défini la routine qui sera appellée pour mettre à jour les élements de la classe

    DllExport
    BOOL            AddItem(CBotString name, CBotTypResult type, int mPrivate = PR_PUBLIC);
    //              ajoute un élément à la classe
//  DllExport
//  BOOL            AddItem(CBotString name, CBotClass* pClass);
    //              idem pour des éléments appartenant à pClass
    DllExport
    BOOL            AddItem(CBotVar* pVar);
    //              idem en passant le pointeur à une instance d'une variable
    //              l'objet est pris tel quel, il ne faut donc pas le détruire



    // idem en donnant un élément de type CBotVar
    void            AddNext(CBotClass* pClass);

    DllExport
    CBotString      GivName();                  // rend le nom de la classe
    DllExport
    CBotClass*      GivParent();                // donne la classe père (ou NULL)

    // dit si une classe est dérivée (Extends) d'une autre
    // rend TRUE aussi si les classes sont identiques
    DllExport
    BOOL            IsChildOf(CBotClass* pClass);

    static
    CBotClass*      Find(CBotToken* &pToken);   // trouve une classe d'après son nom

    DllExport
    static
    CBotClass*      Find(const char* name);

    CBotVar*        GivVar();                   // rend la liste des variables
    CBotVar*        GivItem(const char* name);  // l'une des variables selon son nom
    CBotVar*        GivItemRef(int nIdent);

    CBotTypResult   CompileMethode(const char* name, CBotVar* pThis, CBotVar** ppParams,
                                   CBotCStack* pStack, long& nIdent);

    BOOL            ExecuteMethode(long& nIdent, const char* name, CBotVar* pThis, CBotVar** ppParams, CBotVar* &pResult, CBotStack* &pStack, CBotToken* pToken);
    void            RestoreMethode(long& nIdent, const char* name, CBotVar* pThis, CBotVar** ppParams, CBotStack* &pStack);

    // compile une classe déclarée par l'utilisateur
    static
    CBotClass*      Compile(CBotToken* &p, CBotCStack* pStack);
    static
    CBotClass*      Compile1(CBotToken* &p, CBotCStack* pStack);

    BOOL            CompileDefItem(CBotToken* &p, CBotCStack* pStack, BOOL bSecond);

    BOOL            IsIntrinsic();
    void            Purge();
    static
    void            Free();

    DllExport
    static
    BOOL            SaveStaticState(FILE* pf);

    DllExport
    static
    BOOL            RestoreStaticState(FILE* pf);

    BOOL            Lock(CBotProgram* p);
    void            Unlock();
    static
    void            FreeLock(CBotProgram* p);

    BOOL            CheckCall(CBotToken* &pToken, CBotDefParam* pParam);

};

#define MAXDEFNUM           1000                // nombre limite des DefineNum

/////////////////////////////////////////////////////////////////////////////////////
// gestion des jetons (tokens)

#define TokenTypKeyWord     1                   // un mot clef du language (voir TokenKeyWord)
#define TokenTypNum         2                   // un nombre
#define TokenTypString      3                   // une chaine
#define TokenTypVar         4                   // un nom de variable
#define TokenTypDef         5                   // une valeur selon DefineNum

#define TokenKeyWord        2000                // les mots clefs du langage
#define TokenKeyDeclare     2100                // mots clefs pour déclarations (int, float,..)
#define TokenKeyVal         2200                // les mots représentant une "valeur" (true, false, null, nan)
#define TokenKeyOp          2300                // les opérateurs


class CBotToken
{
private:
    static
    CBotStringArray m_ListKeyWords;             // liste des mots clefs du language
    static
    int             m_ListIdKeyWords[200];      // les codes correspondants

    static
    CBotStringArray m_ListKeyDefine;            // les noms définis par un DefineNum
    static
    long            m_ListKeyNums[MAXDEFNUM];   // les valeurs associées

private:
    CBotToken*      m_next;                     // suivant dans la liste
    CBotToken*      m_prev;
    int             m_type;                     // type de Token
    long            m_IdKeyWord;                // numéro du mot clef si c'en est un
                                                // ou valeur du "define"

    CBotString      m_Text;                     // mot trouvé comme token
    CBotString      m_Sep;                      // séparateurs qui suivent

    int             m_start;                    // position dans le texte d'origine (programme)
    int             m_end;                      // itou pour la fin du token

    static
    int             GivKeyWords(const char* w); // est-ce un mot clef ?
    static
    BOOL            GivKeyDefNum(const char* w, CBotToken* &token);

    static
    void            LoadKeyWords();             // fait la liste des mots clefs

public:
                    CBotToken();
                    CBotToken(const CBotToken* pSrc);
                    CBotToken(const CBotString& mot, const CBotString& sep, int start=0, int end=0);
                    CBotToken(const char* mot, const char* sep = NULL);
                                                // constructeur
                    ~CBotToken();               // destructeur

    DllExport
    int             GivType();                  // rend le type du token

    DllExport
    CBotString&     GivString();                // rend la chaine correspondant à ce token

    DllExport
    CBotString&     GivSep();                   // rend le séparateur suivant le token

    DllExport
    int             GivStart();                 // position du début dans le texte
    DllExport
    int             GivEnd();                   // position de fin dans le texte

    DllExport
    CBotToken*      GivNext();                  // rend le suivant dans la liste
    DllExport
    CBotToken*      GivPrev();                  // rend le Précédent dans la liste

    DllExport
    static
    CBotToken*      CompileTokens(const char* p, int& error);
                                                // transforme tout le programme
    DllExport
    static
    void            Delete(CBotToken* pToken);  // libère la liste


    // fonctions non utiles en export
    static
    BOOL            DefineNum(const char* name, long val);
    void            SetString(const char* name);

    void            SetPos(int start, int end);
    long            GivIdKey();
    void            AddNext(CBotToken* p);      // ajoute un token (une copie)

    static
    CBotToken*      NextToken(char* &program, int& error, BOOL first = FALSE);
                                                // trouve le prochain token
    const CBotToken&
                    operator=(const CBotToken& src);

    static
    void            Free();
};



#if 0
////////////////////////////////////////////////////////////////////////
// Exemples d'utilisation
// Définition de classes et de fonctions


// définie la classe globale CPoint
// --------------------------------
    m_pClassPoint   = new CBotClass("CPoint", NULL);
    // ajoute le composant ".x"
    m_pClassPoint->AddItem("x", CBotTypResult(CBotTypFloat));
    // ajoute le composant ".y"
    m_pClassPoint->AddItem("y", CBotTypResult(CBotTypFloat));
    // le joueur peut alors utiliser les instructions
    // CPoint position; position.x = 12; position.y = -13.6

// définie la classe CColobotObject
// --------------------------------
// cette classe gère tous les objets dans le monde de COLOBOT
// le programme utilisateur "main" appartient à cette classe
    m_pClassObject  = new CBotClass("CColobotObject", m_pClassBase);
    // ajoute le composant ".position"
    m_pClassObject->AddItem("position", m_pClassPoint);
    // ajoute le composant ".type"
    m_pClassObject->AddItem("type", CBotTypResult(CBotTypShort));
    // ajoute une définition de constante
    m_pClassObject->AddConst("ROBOT", CBotTypShort, 1);         // ROBOT équivalent à la valeur 1
    // ajoute la routine FIND
    m_pClassObject->AddFunction( rCompFind, rDoFind );
    // le joueur peut maintenant utiliser les instructions
    // CColobotObject chose; chose = FIND( ROBOT )



// définie la classe CColobotRobot dérivée de CColobotObject
// ---------------------------------------------------------
// les programmes "main" associés aux robots font partie de cette classe
    m_pClassRobot   = new CBotClass("CColobotRobot", m_pClassObject);
    // ajoute la routine GOTO
    m_pClassRobot->AddFunction( rCompGoto, rDoGoto );
    // le joueur peut maintenant faire
    // GOTO( FIND ( ROBOT ) );


// crée une instance de la classe Robot
// ------------------------------------
// par exemple un nouveau robot qui vient d'être fabriqué
    CBotVar*    m_pMonRobot = new CBotVar("MonRobot", m_pClassRobot);

// compile le programme main pour ce robot-là
// ------------------------------------------
    CString LeProgramme( "void main() {GOTO(0, 0); return 0;}" );
    if ( !m_pMonRobot->Compile( LeProgramme ) ) {gestion d'erreur...};

// construit une pile pour l'interpréteur
// --------------------------------------
    CBotStack*  pStack = new CBotStack(NULL);

// exécute le programme main
// -------------------------
    while( FALSE = m_pMonRobot->Execute( "main", pStack ))
    {
        // programme suspendu
        // on pourrait passer la main à un autre (en sauvegardant pStack pour ce robot-là)
    };
    // programme "main" terminé !




// routine implémentant l'instruction GOTO( CPoint pos )
BOOL rDoGoto( CBotVar* pVar, CBotVar* pResult, int& exception )
{
    if (pVar->GivType() != CBotTypeClass ||
        pVar->IsElemOfClas("CPoint") ) { exception = 6522; return FALSE; )
        // le paramètre n'est pas de la bonne classe ?
        // NB en fait ce contrôle est déjà fait par la routine pour la compilation

    m_PosToGo.Copy( pVar );             // garde la position à atteindre (object type CBotVar)

    // ou alors
    CBotVar*    temp;
    temp = pVar->GivItem("x");          // trouve forcément pour un object de type "CPoint"
    ASSERT (temp != NULL && temp->GivType() == CBotTypFloat);
    m_PosToGo.x = temp->GivValFloat();

    temp = pVar->GivItem("y");          // trouve forcément pour un object de type "CPoint"
    ASSERT (temp != NULL && temp->GivType() == CBotTypFloat);
    m_PosToGo.y = temp->GivValFloat();

    return (m_CurentPos == m_PosToGo);  // rend TRUE si la position est atteinte
                                        // rend FALSE s'il faut patienter encore
}

#endif