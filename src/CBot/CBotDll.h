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
////////////////////////////////////////////////////////////////////////

/**
 * \file CBotDll.h
 * \brief Library for interpretation of CBOT language
 */

#pragma once

#include <stdio.h>
#include "resource.h"
#include <map>
#include <cstring>


#define    CBOTVERSION    104

////////////////////////////////////////////////////////////////////////
// forward declaration of needed classes

class CBotToken;        // program turned into "tokens
class CBotStack;        // for the execution stack
class CBotClass;        // class of object
class CBotInstr;        // instruction to be executed
class CBotFunction;     // user functions
class CBotVar;          // variables
class CBotVarClass;     // instance of class
class CBotVarPointer;   // pointer to an instance of class
class CBotCall;         // functions
class CBotCallMethode;  // methods
class CBotDefParam;     // parameter list
class CBotCStack;       // stack


////////////////////////////////////////////////////////////////////////
// Variables management
////////////////////////////////////////////////////////////////////////

/** \brief CBotType Defines known types. This types are modeled on Java types. Do not change the order of elements */
enum CBotType
{
    CBotTypVoid         = 0,
    CBotTypByte         = 1,                //n
    CBotTypShort        = 2,                //n
    CBotTypChar         = 3,                //n
    CBotTypInt          = 4,
    CBotTypLong         = 5,                //n
    CBotTypFloat        = 6,
    CBotTypDouble       = 7,                //n
    CBotTypBoolean      = 8,
    CBotTypString       = 9,

    CBotTypArrayPointer = 10,                // array of variables
    CBotTypArrayBody    = 11,                // same but creates an instance

    CBotTypPointer      = 12,                // pointer to an instance
    CBotTypNullPointer  = 13,                // null pointer is special
    CBotTypClass        = 15,
    CBotTypIntrinsic    = 16                // instance of a class intrinsic
};
//n = not implemented yet

// for SetUserPtr when deleting an object
// \TODO define own types to distinct between different states of objects
#define OBJECTDELETED (reinterpret_cast<void*>(-1))
// value set before initialization
#define OBJECTCREATED (reinterpret_cast<void*>(-2))


/** \brief CBotTypResult class to define the complete type of a result*/
class CBotTypResult
{
public:
    /**
     * \brief CBotTypResult constructor  for simple types (CBotTypInt to CBotTypString)
     * \param type type of created result, see CBotType
     */
    CBotTypResult(int type);
    // for simple types (CBotTypInt à CBotTypString)

    
    CBotTypResult(int type, const char* name);
    // for pointer types and intrinsic classes
    
    CBotTypResult(int type, CBotClass* pClass);
    // for the instance of a class
    
    CBotTypResult(int type, CBotTypResult elem);
    // for arrays of variables
    
    CBotTypResult(const CBotTypResult& typ);
    // for assignments
    
    CBotTypResult();
    // for default
    
    ~CBotTypResult();

    int            GetType(int mode = 0) const;
    // returns type CBotType* as a result

    void        SetType(int n);
    // modifies a type

    CBotClass*    GetClass() const;
    // makes the pointer to the class (for CBotTypClass, CBotTypPointer)

    int            GetLimite() const;
    // returns limit size of table (CBotTypArray)

    void        SetLimite(int n);
    // set limit to the table

    void        SetArray(int* max );
    // set limits for a list of dimensions (arrays of arrays)

    CBotTypResult& GetTypElem() const;
    // returns type of array elements (CBotTypArray)
    // rend le type des éléments du tableau (CBotTypArray)

    bool        Compare(const CBotTypResult& typ) const;
    // compares whether the types are compatible
    bool        Eq(int type) const;
    // compare type

    CBotTypResult& operator=(const CBotTypResult& src);
    // copy a complete type in another

private:
    int                m_type;
    CBotTypResult*    m_pNext;    // for the types of type
    CBotClass*        m_pClass;    // for the derivatives of class
    int                m_limite;    // limits of tables
    friend class    CBotVarClass;
    friend class    CBotVarPointer;    
};

/*
// to define a result as output, using for example

    // to return a simple Float
    return CBotTypResult( CBotTypFloat );


    // to return a string array
    return CBotTypResult( CBotTypArray, CBotTypResult( CBotTypString ) );

    // to return un array of array of "point" class
    CBotTypResult    typPoint( CBotTypIntrinsic, "point" );
    CBotTypResult    arrPoint( CBotTypArray, typPoint );
    return    CBotTypResult( CBotTypArray, arrPoint );
*/


////////////////////////////////////////////////////////////////////////
// Error Handling of compilation and execution
////////////////////////////////////////////////////////////////////////

// Here are the list of errors that can be returned by the module
// for compilation

#define    CBotErrOpenPar            5000    // missing the opening parenthesis
#define    CBotErrClosePar            5001    // missing the closing parenthesis
#define    CBotErrNotBoolean        5002    // expression must be a boolean
#define    CBotErrUndefVar            5003    // undeclared variable
#define    CBotErrBadLeft            5004    // assignment impossible ( 5 = ... )
#define    CBotErrNoTerminator        5005    // semicolon expected
#define    CBotErrCaseOut            5006    // case outside a switch
//    CBotErrNoTerm            5007, plus utile
#define    CBotErrCloseBlock        5008    // missing " } "
#define    CBotErrElseWhitoutIf    5009    // else without matching if
#define    CBotErrOpenBlock        5010    // missing " { "
#define    CBotErrBadType1            5011    // wrong type for the assignment
#define    CBotErrRedefVar            5012    // redefinition of the variable
#define    CBotErrBadType2            5013    // Two operands are incompatible
#define    CBotErrUndefCall        5014    // routine undefined
#define    CBotErrNoDoubleDots        5015    // " : " expected
//    CBotErrWhile            5016, plus utile
#define    CBotErrBreakOutside        5017    // break outside of a loop
#define    CBotErrUndefLabel        5019    // label udnefined
#define    CBotErrLabel            5018    // label ne peut se mettre ici (label can not get here)
#define    CBotErrNoCase            5020    // missing " case "
#define    CBotErrBadNum            5021    // expected number
#define    CBotErrVoid                5022    // " void " not possible here
#define    CBotErrNoType            5023    // type declaration expected
#define    CBotErrNoVar            5024    // variable name expected
#define    CBotErrNoFunc            5025    // expected function name
#define    CBotErrOverParam        5026    // too many parameters
#define    CBotErrRedefFunc        5027    // this function already exists
#define    CBotErrLowParam            5028    // not enough parameters
#define    CBotErrBadParam            5029    // wrong types of parameters
#define    CBotErrNbParam            5030    // wrong number of parameters
#define    CBotErrUndefItem        5031    // element does not exist in the class
#define    CBotErrUndefClass        5032    // variable is not a class
#define    CBotErrNoConstruct        5033    // no appropriate constructor
#define    CBotErrRedefClass        5034    // class already exists
#define    CBotErrCloseIndex        5035    // " ] " expected
#define    CBotErrReserved            5036    // reserved word (for a DefineNum)
#define CBotErrBadNew            5037    // wrong setting for new
#define CBotErrOpenIndex        5038    // " [ " expected
#define CBotErrBadString        5039    // expected string
#define CBotErrBadIndex            5040    // wrong index type "[ false ]"
#define CBotErrPrivate            5041    // protected item
#define CBotErrNoPublic            5042    // missing word "public"

// here is the list of errors that can be returned by the module
// for the execution

#define    CBotErrZeroDiv            6000    // division by zero
#define    CBotErrNotInit            6001    // uninitialized variable
#define    CBotErrBadThrow            6002    // throw a negative value
#define    CBotErrNoRetVal            6003    // function did not return results
#define    CBotErrNoRun            6004    // Run() without active function
#define    CBotErrUndefFunc        6005    // calling a function that no longer exists
#define CBotErrNotClass            6006    // this class does not exist
#define CBotErrNull                6007    // null pointer
#define CBotErrNan                6008    // calculation with a NAN
#define CBotErrOutArray            6009    // index out of array
#define CBotErrStackOver        6010    // stack overflow
#define CBotErrDeletedPtr        6011    // pointer to an object destroyed

#define CBotErrFileOpen            6012    // cannot open the file
#define CBotErrNotOpen            6013    // channel not open
#define CBotErrRead                6014    // error while reading
#define CBotErrWrite            6015    // writing error


// other values ​​may be returned
// for example exceptions returned by external routines
// and " throw " with any number.


////////////////////////////////////////////////////////////////////////
// 
// as part of MFC CString not used here.
//
// ( all functions are not implemented yet )

/** \brief CBotString Class used to work on strings */
class CBotString
{
public:
    CBotString();
    CBotString(const char* p);
    CBotString(const CBotString& p);
    ~CBotString();

    void       Empty();
    bool       IsEmpty() const;
    int        GetLength();
    int        Find(const char c);
    int        Find(const char* lpsz);
    int        ReverseFind(const char c);
    int        ReverseFind(const char* lpsz);
    bool       LoadString(unsigned int id);
    CBotString Mid(int nFirst, int nCount) const;
    CBotString Mid(int nFirst) const;
    CBotString Mid(int start, int lg=-1);
    CBotString Left(int nCount) const;
    CBotString Right(int nCount) const;
    int        Compare(const char* lpsz) const;
    void       MakeUpper();
    void       MakeLower();


    /**
     * \brief Overloaded oprators to work on CBotString classes
     */
    const CBotString& operator=(const CBotString& stringSrc);
    const CBotString& operator=(const char ch);
    const CBotString& operator=(const char* pString);
    const CBotString& operator+(const CBotString& str);
    friend CBotString operator+(const CBotString& string, const char* lpsz);

    const CBotString& operator+=(const char ch);
    const CBotString& operator+=(const CBotString& str);
    bool              operator==(const CBotString& str);
    bool              operator==(const char* p);
    bool              operator!=(const CBotString& str);
    bool              operator!=(const char* p);
    bool              operator>(const CBotString& str);
    bool              operator>(const char* p);
    bool              operator>=(const CBotString& str);
    bool              operator>=(const char* p);
    bool              operator<(const CBotString& str);
    bool              operator<(const char* p);
    bool              operator<=(const CBotString& str);
    bool              operator<=(const char* p);

                      operator const char*() const;           // as a C string


private:

    /** \brief Pointer to string */
    char* m_ptr;

    /** \brief Length of the string */
    int m_lg;

    /** \brief Keeps the string corresponding to keyword ID */
    static const std::map<EID,const char *> s_keywordString;

    /**
     * \brief MapIdToString maps given ID to its string equivalent
     * \param id Provided identifier
     * \return string if found, else NullString
     */
    static const char * MapIdToString(EID id);
};


// Class used to array management

class CBotStringArray : public CBotString
{
private:
    int                m_nSize;                    // number of elements
    int                m_nMaxSize;                    // reserved size
    CBotString*        m_pData;                    // ^data

public:
                    CBotStringArray();
                    ~CBotStringArray();
    void            SetSize(int nb);
    int             GetSize();
    void            Add(const CBotString& str);
    CBotString&        operator[](int nIndex);

    CBotString&        ElementAt(int nIndex);
};

// different modes for GetPosition
enum CBotGet
{
    GetPosExtern    = 1,
    GetPosNom        = 2,
    GetPosParam        = 3,
    GetPosBloc        = 4
};

////////////////////////////////////////////////////////////////////
// main class managing CBot program
//

class CBotProgram
{
private:
    CBotFunction*    m_Prog;            // the user-defined functions
    CBotFunction*    m_pRun;            // the basic function for the execution
    CBotClass*        m_pClass;        // classes defined in this part
    CBotStack*        m_pStack;        // execution stack
    CBotVar*        m_pInstance;    // instance of the parent class
    friend class    CBotFunction;

    int                m_ErrorCode;
    int                m_ErrorStart;
    int                m_ErrorEnd;

    long            m_Ident;        // associated identifier

public:
    static CBotString        m_DebugVarStr;    // end of a debug
    bool m_bDebugDD;        // idem déclanchable par robot \TODO ???
    bool m_bCompileClass;

public:
    static void Init();
    //                initializes the module (defined keywords for errors)
    //                should be done once (and only one) at the beginning
    static
    void            Free();
    //                frees the static memory areas

    static
    int                GetVersion();
    //                gives the version of the library CBOT


                    CBotProgram();
                    CBotProgram(CBotVar* pInstance);
                    ~CBotProgram();

    bool            Compile( const char* program, CBotStringArray& ListFonctions, void* pUser = NULL);
    //                compiles the program given in text
    //                returns false if an error at compile
    //                see GetCompileError () to retrieve the error
    //                ListFonctions returns the names of functions declared as extern
    //                pUser can pass a pointer to routines defined by AddFunction

    void            SetIdent(long n);
    //                associates an identifier with the instance CBotProgram

    long            GetIdent();
    //                gives the identifier

    int                GetError();
    bool            GetError(int& code, int& start, int& end);
    bool            GetError(int& code, int& start, int& end, CBotProgram* &pProg);
    //                if true
    //                gives the error found in the compilation
    //                or execution
    //                delimits the start and end block where the error
    //                pProg lets you know what "module" has produced runtime error
    static         CBotString        GetErrorText(int code);


    bool            Start(const char* name);
    //                defines what function should be executed
    //                returns false if the funtion name is not found
    //                the program does nothing, we must call Run () for this

    bool            Run(void* pUser = NULL, int timer = -1);
    //                executes the program
    //                returns false if the program was suspended
    //                returns true if the program ended with or without error
    //                timer = 0 allows to advance step by step

    bool            GetRunPos(const char* &FunctionName, int &start, int &end);
    //                gives the position in the executing program
    //                returns false if it is not running (program completion)
    //                FunctionName is a pointer made to the name of the function
    //                start and end position in the text of the token processing

    CBotVar*        GetStackVars(const char* &FunctionName, int level);
    //                provides the pointer to the variables on the execution stack
    //                level is an input parameter,  0 for the last level, -1, -2, etc. for the other levels
    //                the return value (CBotVar *) is a variable list (or NULL)
    //                    that can be processed as the list of parameters received by a routine
    //                FunctionName gives the name of the function where are these variables
    //                FunctionName == NULL means that is more in a program (depending on  level)

    void            Stop();
    //                stops execution of the program
    //                therefore quits "suspend" mode

    static
    void            SetTimer(int n);
    //                defines the number of steps (parts of instructions) to done  
    //                in Run() before rendering hand "false" \TODO avant de rendre la main "false"

    static
    bool            AddFunction(const char* name, 
                                bool rExec (CBotVar* pVar, CBotVar* pResult, int& Exception, void* pUser), 
                                CBotTypResult rCompile (CBotVar* &pVar, void* pUser));
    //                call this to add externally (**)
    //                a new function used by the program CBoT

    static
    bool            DefineNum(const char* name, long val);

    bool            SaveState(FILE* pf);
    //                backup the execution status in the file
    //                the file must have been opened with the fopen call this dll (\TODO this library??)
    //                if the system crashes
    bool            RestoreState(FILE* pf);
    //                restores the state of execution from file
    //                the compiled program must obviously be the same

    bool            GetPosition(const char* name, int& start, int& stop, 
                                CBotGet modestart = GetPosExtern, 
                                CBotGet modestop  = GetPosBloc);
    //                gives the position of a routine in the original text
    //                the user can select the item to find from the beginning to the end
    //                see the above modes in CBotGet


    CBotFunction*    GetFunctions();
};


///////////////////////////////////////////////////////////////////////////////
// routines for file management  (* FILE)
    FILE*        fOpen(const char* name, const char* mode);
    int            fClose(FILE* filehandle);
    size_t        fWrite(const void *buffer, size_t elemsize, size_t length, FILE* filehandle);
    size_t        fRead(void *buffer, size_t elemsize, size_t length, FILE* filehandle);


#if 0
/*
(**) Note:
     To define an external function, proceed as follows:

    a) define a routine for compilation
        this routine receive list of parameters (no values)
        and either returns a result type (CBotTyp... or 0 = void)
        or an error number
    b) define a routine for the execution
        this routine receive list of parameters (with valeurs),
        a variable to store the result (according to the given type at compile time)

    For example, a routine which calculates the mean of a parameter list */

int    cMean(CBotVar* &pVar, CBotString& ClassName)
{
    if ( pVar == NULL ) return 6001;    // there is no parameter!

    while ( pVar != NULL )
    {
        if ( pVar->GetType() > CBotTypDouble ) return 6002;        // this is not a number 
        pVar = pVar -> GetNext();
    }

    return CBotTypFloat;        // the type of the result may depend on the parameters!
}


bool rMean(CBotVar* pVar, CBotVar* pResult, int& Exception)
{
    float total = 0;
    int   nb      = 0;
    while (pVar != NULL)
    {
        total += pVar->GetValFloat();
        pVar = pVar->GetNext();
        nb++;
    }
    pResult->SetValFloat(total/nb);                // returns the mean value

    return true;                                // operation fully completed
}

#endif

/////////////////////////////////////////////////////////////////////////////////
// Class for managing variables

// may be useful to the outside of the module
// ( it is currently not expected to be able to create these objects in outer )

// results of GetInit()
#define    IS_UNDEF    0        // undefined variable
#define IS_DEF        1        // variable defined
#define    IS_NAN        999        // variable defined as not a number

// variable type SetPrivate / IsPrivate
#define PR_PUBLIC    0        // public variable
#define    PR_READ        1        // read only
#define PR_PROTECT    2        // protected (inheritance)
#define PR_PRIVATE    3        // strictly private

class CBotVar
{
protected:
    CBotToken*        m_token;                    // the corresponding token

    CBotVar*        m_next;                        // list of variables
    friend class    CBotStack;
    friend class    CBotCStack;
    friend class    CBotInstrCall;
    friend class    CBotProgram;

    CBotTypResult    m_type;                        // type of value

    int                m_binit;                    // not initialized?
    CBotVarClass*    m_pMyThis;                    // ^ corresponding this element
    void*            m_pUserPtr;                    // ^user data if necessary
    bool            m_bStatic;                    // static element (in class)
    int                m_mPrivate;                    // element public, protected or private?

    CBotInstr*        m_InitExpr;                    // expression for the original content
    CBotInstr*        m_LimExpr;                    // list of limits for a table
    friend class    CBotClass;
    friend class    CBotVarClass;
    friend class    CBotVarPointer;
    friend class    CBotVarArray;

    long            m_ident;                    // unique identifier
    static long        m_identcpt;                    // counter

public:
                    CBotVar();
virtual                ~CBotVar( );                        // destructor

    static
    CBotVar*        Create( const char* name, CBotTypResult type);
    //                creates from a complete type

    static
    CBotVar*        Create( const char* name, CBotClass* pClass);
    //                creates from one instance of a known class

    static
    CBotVar*        Create( const CBotToken* name, int type );
    static
    CBotVar*        Create( const CBotToken* name, CBotTypResult type );

    static
    CBotVar*        Create( const char* name, int type, CBotClass* pClass);

    static
    CBotVar*        Create( CBotVar* pVar );


    void            SetUserPtr(void* pUser);
    //                associate a user pointer to an instance

    virtual void    SetIdent(long UniqId);
    //                associates a unique identifier to an instance
    //                ( it is used to ensure that the id is unique)

    void*            GetUserPtr();
    //                makes the pointer associated with the variable

    CBotString        GetName();                    // the name of the variable, if known
    ////////////////////////////////////////////////////////////////////////////////////
    void            SetName(const char* name);    // changes the name of the variable

    int                GetType(int mode = 0);          // returns the base type (int) of the variable
                                                        // TODO check it    
    ////////////////////////////////////////////////////////////////////////////////////////

    CBotTypResult    GetTypResult(int mode = 0);    // returns the complete type of the variable


    CBotToken*        GetToken();
    void            SetType(CBotTypResult& type);

    void            SetInit(int bInit);            // is the variable in the state IS_UNDEF, IS_DEF, IS_NAN

    int                GetInit();                    // gives the state of the variable

    void            SetStatic(bool bStatic);
    bool            IsStatic();

    void            SetPrivate(int mPrivate);
    bool            IsPrivate(int mode = PR_PROTECT);
    int                GetPrivate();

    virtual
    void            ConstructorSet();

    void            SetVal(CBotVar* var);          // remprend une valeur
                                                    // TODO remprend value
    virtual
    CBotVar*        GetItem(const char* name);    // returns an element of a class according to its name (*)
    virtual
    CBotVar*        GetItemRef(int nIdent);        // idem à partir du n° ref
                                                    // TODO ditto from ref no.
    virtual
    CBotVar*        GetItem(int row, bool bGrow = false); 

    virtual
    CBotVar*        GetItemList();                // lists the elements

    CBotVar*        GetStaticVar();                // makes the pointer to the variable if it is static

    bool            IsElemOfClass(const char* name);
                                                // said if the element belongs to the class "name"
                                                // makes true if the object is a subclass

    CBotVar*        GetNext();                    // next variable in the list (parameters)
    ////////////////////////////////////////////////////////////////////////////////////////////

    void            AddNext(CBotVar* pVar);        // added to a list

    virtual
    void            Copy(CBotVar* pSrc, bool bName = true);    // makes a copy of the variable

    virtual void    SetValInt(int val, const char* name = NULL);
                                                // initialized with an integer value (#)
    /////////////////////////////////////////////////////////////////////////////////

    virtual void    SetValFloat(float val);        // initialized with a real value (#)
    ////////////////////////////////////////////////////////////////////////////////

    virtual void    SetValString(const char* p);// initialized with a string value (#)
    ////////////////////////////////////////////////////////////////////////////////

    virtual int        GetValInt();                // request the full value (#)
    ////////////////////////////////////////////////////////////////////////

    virtual float    GetValFloat();                // gets real value (#)
    ///////////////////////////////////////////////////////////////////////

    virtual
    CBotString         GetValString();                // request the string value (#)
    ///////////////////////////////////////////////////////////////////////

    virtual void    SetClass(CBotClass* pClass);
    virtual
    CBotClass*        GetClass();

    virtual void    SetPointer(CBotVar* p);
    virtual
    CBotVarClass*    GetPointer();
//    virtual void    SetIndirection(CBotVar* pVar);

    virtual void    Add(CBotVar* left, CBotVar* right);    // addition
    virtual void    Sub(CBotVar* left, CBotVar* right);    // subtraction
    virtual void    Mul(CBotVar* left, CBotVar* right);    // multiplication
    virtual int     Div(CBotVar* left, CBotVar* right);    // division
    virtual int     Modulo(CBotVar* left, CBotVar* right);    // remainder of division
    virtual void    Power(CBotVar* left, CBotVar* right);    // power

    virtual bool    Lo(CBotVar* left, CBotVar* right);
    virtual bool    Hi(CBotVar* left, CBotVar* right);
    virtual bool    Ls(CBotVar* left, CBotVar* right);
    virtual bool    Hs(CBotVar* left, CBotVar* right);
    virtual bool    Eq(CBotVar* left, CBotVar* right);
    virtual bool    Ne(CBotVar* left, CBotVar* right);

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


    virtual bool    Save0State(FILE* pf);
    virtual bool    Save1State(FILE* pf);
    static    bool    RestoreState(FILE* pf, CBotVar* &pVar);

    void            debug();

//    virtual
//    CBotVar*        GetMyThis();

    virtual
    void            Maj(void* pUser = NULL, bool bContinue = true);

    void            SetUniqNum(long n);
    long            GetUniqNum();
    static long        NextUniqNum();
};

/* NOTE (#)
    methods    SetValInt() SetValFloat() et SetValString()
    can be called  with objects which are respectively integer, real or string
    Always be sure of the type of the variable before calling these methods

    if ( pVar->GetType() == CBotInt() ) pVar->SetValFloat( 3.3 ); // plante !!

    methods    GetValInt(), GetValFloat() et GetValString()
    use value conversions,
    GetValString() works on numbers (makes the corresponding string)
    but do not make GetValInt () with a string variable!
*/



////////////////////////////////////////////////////////////////////////
// management of classes
////////////////////////////////////////////////////////////////////////

// class to define new classes in the language CBOT
// for example to define the class CPoint (x, y)

class CBotClass
{
private:
    static
    CBotClass*        m_ExClass;        // list of classes existing at a given time
    CBotClass*        m_ExNext;        // for this general list
    CBotClass*        m_ExPrev;        // for this general list

private:
    CBotClass*        m_pParent;        // parent class
    CBotString        m_name;            // name of this class
    int                m_nbVar;        // number of variables in the chain
    CBotVar*        m_pVar;            // content of the class
    bool            m_bIntrinsic;    // intrinsic class
    CBotClass*        m_next;            // the string class
    CBotCallMethode* m_pCalls;        // list of methods defined in external
    CBotFunction*    m_pMethod;        // compiled list of methods
    void            (*m_rMaj) ( CBotVar* pThis, void* pUser );
    friend class    CBotVarClass;
    int                m_cptLock;        // for Lock / UnLock
    int                m_cptOne;        // Lock for reentrancy
    CBotProgram*    m_ProgInLock[5];// processes waiting for sync

public:
    bool            m_IsDef;        //  mark if is set or not

                    CBotClass( const char* name,
                               CBotClass* pParent, bool bIntrinsic = false );        // constructor
    //                Once a class is created, it is known
    //                around CBoT
    //                intrinsic mode gives a class that is not managed by pointers

                    ~CBotClass( );                            // destructor

    bool            AddFunction(const char* name, 
                                bool rExec (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception), 
                                CBotTypResult rCompile (CBotVar* pThis, CBotVar* &pVar));
    //                this call allows to add as external (**)
    //                new method used by the objects of this class

    bool            AddUpdateFunc( void rMaj ( CBotVar* pThis, void* pUser ) );
    //                defines routine to be called to update the elements of the class

    bool            AddItem(CBotString name, CBotTypResult type, int mPrivate = PR_PUBLIC);
    //                adds an element to the class
//    bool            AddItem(CBotString name, CBotClass* pClass);
    //                the same for elements belonging to pClass
    bool            AddItem(CBotVar* pVar);
    //                adds an item by passing the pointer to an instance of a variable
    //                the object is taken as is, so do not destroyed



    // adds an element by giving an element of type CBotVar
    void            AddNext(CBotClass* pClass);

    CBotString        GetName();                    // gives the name of the class
    CBotClass*        GetParent();                // gives the parent class (or NULL)

    // true if a class is derived (Extends) of another
    // return true also if the classes are identical
    bool            IsChildOf(CBotClass* pClass);

    static
    CBotClass*        Find(CBotToken* &pToken);    // trouve une classe d'après son nom
                                                    // return a class by it's its name
    static
    CBotClass*        Find(const char* name);

    CBotVar*        GetVar();                    // return the list of variables
    CBotVar*        GetItem(const char* name);    // one of the variables according to its name
    CBotVar*        GetItemRef(int nIdent);

    CBotTypResult    CompileMethode(const char* name, CBotVar* pThis, CBotVar** ppParams, 
                                   CBotCStack* pStack, long& nIdent);

    bool            ExecuteMethode(long& nIdent, const char* name, CBotVar* pThis, CBotVar** ppParams, CBotVar* &pResult, CBotStack* &pStack, CBotToken* pToken);
    void            RestoreMethode(long& nIdent, const char* name, CBotVar* pThis, CBotVar** ppParams, CBotStack* &pStack);

    // compiles a class declared by the user
    static
    CBotClass*        Compile(CBotToken* &p, CBotCStack* pStack);
    static
    CBotClass*        Compile1(CBotToken* &p, CBotCStack* pStack);

    bool            CompileDefItem(CBotToken* &p, CBotCStack* pStack, bool bSecond);
    
    bool            IsIntrinsic();
    void            Purge();
    static
    void            Free();

    static
    bool            SaveStaticState(FILE* pf);

    static
    bool            RestoreStaticState(FILE* pf);

    bool            Lock(CBotProgram* p);
    void            Unlock();
    static
    void            FreeLock(CBotProgram* p);

    bool            CheckCall(CBotToken* &pToken, CBotDefParam* pParam);

};

#define    MAXDEFNUM            1000                // limited number of DefineNum

/////////////////////////////////////////////////////////////////////////////////////
// Token management (tokens)

#define TokenTypKeyWord        1                    // a keyword of the language (see TokenKeyWord)
#define TokenTypNum            2                    // number
#define TokenTypString         3                    // string
#define TokenTypVar            4                    // a variable name
#define TokenTypDef            5                    // value according DefineNum

#define TokenKeyWord        2000                // keywords of the language
#define TokenKeyDeclare        2100                // keywords of declarations (int, float,..)
#define TokenKeyVal            2200                // keywords representing the value (true, false, null, nan)
#define TokenKeyOp            2300                // operators

/** \class Responsible for token management */
class CBotToken
{
private:
    static
    CBotStringArray    m_ListKeyWords;                // list of keywords of language
    static
    int                m_ListIdKeyWords[200];        // the corresponding codes

    static
    CBotStringArray    m_ListKeyDefine;            // names defined by a DefineNum
    static
    long            m_ListKeyNums[MAXDEFNUM];    // the ​​associated values

private:
    CBotToken*        m_next;                        // following in the list
    CBotToken*        m_prev;
    int                m_type;                        // type of Token
    long            m_IdKeyWord;                // number of the keyword if it is a
                                                // or value of the "define"

    CBotString        m_Text;                        // word found as token
    CBotString        m_Sep;                        //  following separators

    int                m_start;                    // position in the original text (program)
    int                m_end;                        // the same for the end of the token
    
    /**
     * \brief Check whether given parameter is a keyword
     */
    static
    int                GetKeyWords(const char* w);    // is it a keyword?
    static
    bool            GetKeyDefNum(const char* w, CBotToken* &token);

    /**
     * \brief Loads the list of keywords
     */
    static
    void            LoadKeyWords();

public:
    /**
     * \brief Constructors
     */
                    CBotToken();
                    CBotToken(const CBotToken* pSrc);
                    CBotToken(const CBotString& mot, const CBotString& sep, int start=0, int end=0);
                    CBotToken(const char* mot, const char* sep = NULL);

    /**
     * \brief Destructor
     */
                    ~CBotToken();
    /**
     * \brief Returns the type of token
     */
    int                GetType();

    /**
     * \brief makes the string corresponding to this token
     */
    CBotString&        GetString();

    /**
     * \brief makes the following separator token
     */
    CBotString&        GetSep();

    /**
     * \brief position of the beginning in the text
     */
    int                GetStart();
    /**
     * \brief end position in the text
     */
    int                GetEnd();

    /**
     * \brief gives the next token in the list
     */
    CBotToken*        GetNext();
    /**
     * \brief gives the previous token in a list
     */
    CBotToken*        GetPrev();

    /**
     * \brief transforms the entire program 
     */
    static
    CBotToken*        CompileTokens(const char* p, int& error);
       
    /**
     * \brief releases the list
     */
    static
    void            Delete(CBotToken* pToken);    // libère la liste


    // fonctions non utiles en export
    static
    bool            DefineNum(const char* name, long val);
    void            SetString(const char* name);

    void            SetPos(int start, int end);
    long            GetIdKey();
    /**
     * \brief adds a token (a copy)
     */
    void            AddNext(CBotToken* p);

    /**
     * finds the next token
     */
    static
    CBotToken*        NextToken(char* &program, int& error, bool first = false);

    const CBotToken&
                    operator=(const CBotToken& src);

    static
    void            Free();
};



#if 0
////////////////////////////////////////////////////////////////////////
// Examples of use
// Definition classes and functions


// define the global class CPoint
// --------------------------------
    m_pClassPoint    = new CBotClass("CPoint", NULL);
    // adds the component ".x"
    m_pClassPoint->AddItem("x", CBotTypResult(CBotTypFloat));
    // adds the component ".y"
    m_pClassPoint->AddItem("y", CBotTypResult(CBotTypFloat));
    // the player can then use the instructions
    // CPoint position; position.x = 12; position.y = -13.6

// define class CColobotObject
// --------------------------------
// This class manages all the objects in the world of COLOBOT
// the  "main" user program belongs to this class
    m_pClassObject    = new CBotClass("CColobotObject", m_pClassBase);
    // adds the component ".position"
    m_pClassObject->AddItem("position", m_pClassPoint);
    // adds the component ".type"
    m_pClassObject->AddItem("type", CBotTypResult(CBotTypShort));
    // adds a definition of constant
    m_pClassObject->AddConst("ROBOT", CBotTypShort, 1);            // ROBOT equivalent to the value 1
    // adds the FIND routine
    m_pClassObject->AddFunction( rCompFind, rDoFind );
    // the player can now use the instructions
    // CColobotObject chose; chose = FIND( ROBOT )



// define class CColobotRobot derived from CColobotObject
// ---------------------------------------------------------
// programs "main" associated with robots as a part of this class
    m_pClassRobot    = new CBotClass("CColobotRobot", m_pClassObject);
    // add routine GOTO
    m_pClassRobot->AddFunction( rCompGoto, rDoGoto );
    // the player can now use
    // GOTO( FIND ( ROBOT ) );


// creates an instance of the class Robot
// ------------------------------------
// for example a new robot which has just been manufactured
    CBotVar*    m_pMonRobot = new CBotVar("MonRobot", m_pClassRobot);

// compiles the program by hand for this robot
// ------------------------------------------
    CString LeProgramme( "void main() {GOTO(0, 0); return 0;}" );
    if ( !m_pMonRobot->Compile( LeProgramme ) ) {error handling ...};

// build a stack for interpreter
// --------------------------------------
    CBotStack*    pStack = new CBotStack(NULL);

// executes the main program
// -------------------------
    while( false = m_pMonRobot->Execute( "main", pStack ))
    {
        // program suspended
        // could be pass a handle to another (safeguarding pstack for the robot one)
    };
    // programme "main" finished !




// routine that implements the GOTO (CPoint pos)
bool rDoGoto( CBotVar* pVar, CBotVar* pResult, int& exception )
{
    if (pVar->GetType() != CBotTypeClass ||
        pVar->IsElemOfClas("CPoint") ) { exception = 6522; return false; )
        // the parameter is not the right class?
        // in fact the control is done to the routine of compilation

    m_PosToGo.Copy( pVar );                // keeps the target position (object type CBotVar)

    // or so
    CBotVar*    temp;
    temp = pVar->GetItem("x");            // is necessary for the object of type CPoint
    ASSERT (temp != NULL && temp->GetType() == CBotTypFloat);
    m_PosToGo.x = temp->GetValFloat();

    temp = pVar->GetItem("y");            // is necessary for the object of type CPoint
    ASSERT (temp != NULL && temp->GetType() == CBotTypFloat);
    m_PosToGo.y = temp->GetValFloat();

    return (m_CurentPos == m_PosToGo);    // makes true if the position is reached
                                        // returns false if one had wait yet
}

#endif

