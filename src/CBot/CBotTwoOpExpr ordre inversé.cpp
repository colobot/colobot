///////////////////////////////////////////////////
// expression du genre Opérande1 + Opérande2
//					   Opérande1 > Opérande2

#include "CBot.h"

// divers constructeurs

CBotTwoOpExpr::CBotTwoOpExpr()
{
	m_leftop	=
	m_rightop	= NULL;			// NULL pour pouvoir faire delete sans autre
	name = "CBotTwoOpExpr";		// debug
}

CBotTwoOpExpr::~CBotTwoOpExpr()
{
	delete	m_leftop;
	delete	m_rightop;
}

// type d'opérandes acceptés par les opérations
#define		ENTIER		((1<<CBotTypByte)|(1<<CBotTypShort)|(1<<CBotTypChar)|(1<<CBotTypInt)|(1<<CBotTypLong))
#define		FLOTANT		((1<<CBotTypFloat)|(1<<CBotTypDouble))
#define		BOOLEEN		(1<<CBotTypBoolean)
#define		CHAINE		(1<<CBotTypString)
#define		CLASSE		(1<<CBotTypClass)

// liste des opérations (précéance)
static int	ListOp[] = 
{
	BOOLEEN,				ID_LOG_OR, 0,
	BOOLEEN,				ID_LOG_AND, 0,
	BOOLEEN|ENTIER,			ID_OR, 0,
	ENTIER,					ID_XOR, 0,
	BOOLEEN|ENTIER,			ID_AND, 0,
	BOOLEEN|ENTIER|FLOTANT,	ID_EQ, 
	BOOLEEN|ENTIER|FLOTANT,	ID_NE, 0,
	ENTIER|FLOTANT,			ID_HI,
	ENTIER|FLOTANT,			ID_LO,
	ENTIER|FLOTANT,			ID_HS,
	ENTIER|FLOTANT,			ID_LS, 0,
	ENTIER,					ID_SR,
	ENTIER,					ID_SL,
	ENTIER,					ID_ASR, 0,
	ENTIER|FLOTANT|CHAINE,	ID_ADD,
	ENTIER|FLOTANT,			ID_SUB, 0,
	ENTIER|FLOTANT,			ID_MUL, 
	ENTIER|FLOTANT,			ID_DIV, 
	ENTIER|FLOTANT,			ID_MODULO, 0,
	0,
};

BOOL IsInList( int val, int* list, int& typemasque )
{
	while (TRUE)
	{
		if ( *list == 0 ) return FALSE;
		typemasque = *list++;
		if ( *list++ == val ) return TRUE;
	}
}

BOOL TypeOk( int type, int test )
{
	while (TRUE)
	{
		if ( type == 0 ) return (test & 1);
		type--; test /= 2;
	}
}

// compile une instruction de type A op B

CBotInstr* CBotTwoOpExpr::Compile(CBotToken* &p, CBotCStack* pStack, int* pOperations)
{
	int	typemasque;

	if ( pOperations == NULL ) pOperations = ListOp;
	int* pOp = pOperations;
	while ( *pOp++ != 0 );										// suite de la table

	CBotCStack* pStk = pStack->TokenStack();					// un bout de pile svp

	// cherche des instructions qui peuvent convenir à gauche de l'opération
	CBotInstr*	left = CBotParExpr::Compile( p, pStk ) ;		// expression (...) à gauche

	if (left == NULL) return pStack->Return(NULL, pStk);		// si erreur, la transmet


	CBotToken*	pp = p;
	int	TypeOp = pp->GetType();									// type d'opération
	p = p->Next();												// saute le token de l'opération 

	// cherche des instructions qui peuvent convenir à droite

	CBotInstr*	right = (*pOp == 0) ?
						CBotParExpr::Compile( p, pStk ) :		// expression (...) à droite
						CBotTwoOpExpr::Compile( p, pStk, pOp );	// expression A op B à droite

	if (right == NULL) 	return pStack->Return(left, pStk);		// pas d'opérande à droite ?

	// est-ce qu'on a l'opération prévue entre les deux ?
	if ( IsInList( TypeOp, pOperations, typemasque ) )
	{
		CBotTwoOpExpr* inst = new CBotTwoOpExpr();				// élément pour opération
		inst->SetToken(pp);										// mémorise l'opération

		int			 type1, type2;
		type1 = pStk->GetType();								// de quel type le premier opérande ?

		inst->m_rightop = right;
		{
			// il y a un second opérande acceptable

			type2 = pStk->GetType();							// de quel type le résultat ?

			// quel est le type du résultat ?
			int	TypeRes = MAX( type1, type2 );
			if (!TypeOk( TypeRes, typemasque )) type1 = 99;		// erreur de type

			switch ( TypeOp )
			{
			case ID_LOG_OR:
			case ID_LOG_AND:
			case ID_EQ:
			case ID_NE:
			case ID_HI:
			case ID_LO:
			case ID_HS:
			case ID_LS:
				TypeRes = CBotTypBoolean;
			}
			if ( TypeCompatible (type1, type2) ||				// les résultats sont-ils compatibles
			// cas particulier pour les concaténation de chaînes
				 (TypeOp == ID_ADD && (type1 == CBotTypString || type2 == CBotTypString)))
			{
				// si ok, enregistre l'opérande dans l'objet
				inst->m_leftop = left;
				// met une variable sur la pile pour avoir le type de résultat
				pStk->SetVar(new CBotVar(NULL, TypeRes));
				// et rend l'object à qui l'a demandé
				return pStack->Return(inst, pStk);
			}
			pStk->SetError(TX_BAD2TYPE, &inst->m_token);
		}

		// en cas d'erreur, libère les éléments
		delete left;
		delete inst;
		// et transmet l'erreur qui se trouve sur la pile
		return pStack->Return(NULL, pStk);
	}

	// si on n'a pas affaire à une opération + ou -
	// rend à qui l'a demandé, l'opérande (de gauche) trouvé
	// à la place de l'objet "addition"
	return pStack->Return(left, pStk);
}




// fait l'opération d'addition ou de soustraction

BOOL CBotTwoOpExpr::Execute(CBotStack* &pStack)
{
	CBotStack* pStk1 = pStack->AddStack();			// ajoute un élément à la pile
													// ou le retrouve en cas de reprise
//	if ( pStk1 == EOX ) return TRUE;


	// selon la reprise, on peut être dans l'un des 2 états

	if ( pStk1->GetState() == 0 &&					// 1er état, évalue l'opérande de gauche
		!m_leftop->Execute(pStk1) ) return FALSE;	// interrompu ici ?

	// passe à l'étape suivante
	pStk1->SetState(1);								// prêt pour la suite

	// pour les OU et ET logique, n'évalue pas la seconde expression si pas nécessaire
	if ( GetTokenType() == ID_LOG_AND && pStk1->GetVal() == FALSE )
	{
		CBotVar*	res = CBotVar::Create( NULL, CBotTypBoolean);
		res->SetValInt(FALSE);
		pStk1->SetVar(res);
		return pStack->Return(pStk1);				// transmet le résultat
	}
	if ( GetTokenType() == ID_LOG_OR && pStk1->GetVal() == TRUE )
	{
		CBotVar*	res = CBotVar::Create( NULL, CBotTypBoolean);
		res->SetValInt(TRUE);
		pStk1->SetVar(res);
		return pStack->Return(pStk1);				// transmet le résultat
	}

	// demande un peu plus de stack pour ne pas toucher le résultat de gauche
	// qui se trouve sur la pile, justement.

	CBotStack* pStk2 = pStk1->AddStack();			// ajoute un élément à la pile
													// ou le retrouve en cas de reprise

	// 2e état, évalue l'opérande de droite
	if ( !m_rightop->Execute(pStk2) ) return FALSE; // interrompu ici ?

	int		type1 = pStk1->GetType();				// de quels types les résultats ?
	int		type2 = pStk2->GetType();

	// crée une variable temporaire pour y mettre le résultat
	// quel est le type du résultat ?
	int	TypeRes = MAX(type1, type2);
	switch ( GetTokenType() )
	{
	case ID_LOG_OR:
	case ID_LOG_AND:
	case ID_EQ:
	case ID_NE:
	case ID_HI:
	case ID_LO:
	case ID_HS:
	case ID_LS:
		TypeRes = CBotTypBoolean;
	}
	CBotVar*	result = CBotVar::Create( NULL, TypeRes);
	CBotVar*	temp   = CBotVar::Create( NULL, MAX(type1, type2) );

	int	err = 0;
	// fait l'opération selon la demande
	switch (GetTokenType())
	{
	case ID_ADD:
		result->Add(pStk1->GetVar(), pStk2->GetVar());		// additionne
		break;
	case ID_SUB:
		result->Sub(pStk1->GetVar(), pStk2->GetVar());		// soustrait
		break;
	case ID_MUL:
		result->Mul(pStk1->GetVar(), pStk2->GetVar());		// multiplie
		break;
	case ID_DIV:
		err = result->Div(pStk1->GetVar(), pStk2->GetVar());// divise
		break;
	case ID_MODULO:
		err = result->Modulo(pStk1->GetVar(), pStk2->GetVar());// reste de division
		break;
	case ID_LO:
		temp->Lo(pStk1->GetVar(), pStk2->GetVar());			// inférieur
		result->SetValInt(temp->GetValInt());				// converti le résultat
		break;
	case ID_HI:
		temp->Hi(pStk1->GetVar(), pStk2->GetVar());			// supérieur
		result->SetValInt(temp->GetValInt());				// converti le résultat
		break;
	case ID_LS:
		temp->Ls(pStk1->GetVar(), pStk2->GetVar());			// inférieur ou égal
		result->SetValInt(temp->GetValInt());				// converti le résultat
		break;
	case ID_HS:
		temp->Hs(pStk1->GetVar(), pStk2->GetVar());			// supérieur ou égal
		result->SetValInt(temp->GetValInt());				// converti le résultat
		break;
	case ID_EQ:
		temp->Eq(pStk1->GetVar(), pStk2->GetVar());			// égal
		result->SetValInt(temp->GetValInt());				// converti le résultat
		break;
	case ID_NE:
		temp->Ne(pStk1->GetVar(), pStk2->GetVar());			// différent
		result->SetValInt(temp->GetValInt());				// converti le résultat
		break;
	case ID_LOG_AND:
	case ID_AND:
		result->And(pStk1->GetVar(), pStk2->GetVar());		// ET
		break;
	case ID_LOG_OR:
	case ID_OR:
		result->Or(pStk1->GetVar(), pStk2->GetVar());		// OU
		break;
	case ID_XOR:
		result->XOr(pStk1->GetVar(), pStk2->GetVar());		// OU exclusif
		break;
	case ID_ASR:
		result->ASR(pStk1->GetVar(), pStk2->GetVar());
		break;
	case ID_SR:
		result->SR(pStk1->GetVar(), pStk2->GetVar());
		break;
	case ID_SL:
		result->SL(pStk1->GetVar(), pStk2->GetVar());
		break;
	default:
		__asm int 3;
	}
	delete temp;

	pStk2->SetVar(result);						// met le résultat sur la pile
	if ( err ) pStk2->SetError(err, &m_token);	// et l'erreur éventuelle (division par zéro)

	pStk1->Return(pStk2);						// libère la pile
	return pStack->Return(pStk1);				// transmet le résultat
}


