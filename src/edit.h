// edit.h

#ifndef _EDIT_H_
#define	_EDIT_H_


#include "control.h"


class CD3DEngine;
class CScroll;



#define EDITSTUDIOMAX	20000		// nb max de caractères pour éditer CBOT
#define EDITLINEMAX		1000		// nb max total de lignes
#define EDITIMAGEMAX	50			// nb max total de lignes avec images
#define EDITLINKMAX		100			// nb max de liens
#define EDITHISTORYMAX	50			// nb max de niveaux concervés

#define EDITUNDOMAX		20			// nb max de undo successifs

typedef struct
{
	char*	text;			// texte original
	int		len;			// longueur du texte
	int		cursor1;		// offset curseur
	int		cursor2;		// offset curseur
	int		lineFirst;		// première ligne affichée.

}
EditUndo;

enum OperUndo
{
	OPERUNDO_SPEC	= 0,	// opération spéciale
	OPERUNDO_INSERT	= 1,	// insertion de caractères
	OPERUNDO_DELETE	= 2,	// suppression de caractères
};

typedef struct
{
	char	name[40];		// nom de l'image (sans diagram\)
	float	offset;			// offset vertical (v texture)
	float	height;			// hauteur de la tranche (dv texture)
	float	width;			// largeur
}
ImageLine;




class CEdit : public CControl
{
public:
	CEdit(CInstanceManager* iMan);
	~CEdit();

	BOOL		Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);

	void		SetPos(FPOINT pos);
	void		SetDim(FPOINT dim);

	BOOL		EventProcess(const Event &event);
	void		Draw();

	void		SetText(char *text, BOOL bNew=TRUE);
	void		GetText(char *buffer, int max);
	char*		RetText();
	int			RetTextLength();

	BOOL		ReadText(char *filename, int addSize=0);
	BOOL		WriteText(char *filename);

	void		SetMaxChar(int max);
	int			RetMaxChar();

	void		SetEditCap(BOOL bMode);
	BOOL		RetEditCap();

	void		SetHiliteCap(BOOL bEnable);
	BOOL		RetHiliteCap();

	void		SetInsideScroll(BOOL bInside);
	BOOL		RetInsideScroll();

	void		SetSoluceMode(BOOL bSoluce);
	BOOL		RetSoluceMode();

	void		SetGenericMode(BOOL bGeneric);
	BOOL		RetGenericMode();

	void		SetAutoIndent(BOOL bMode);
	BOOL		RetAutoIndent();

	void		SetCursor(int cursor1, int cursor2);
	void		GetCursor(int &cursor1, int &cursor2);

	void		SetFirstLine(int rank);
	int			RetFirstLine();
	void		ShowSelect();

	void		SetDisplaySpec(BOOL bDisplay);
	BOOL		RetDisplaySpec();

	void		SetMultiFont(BOOL bMulti);
	BOOL		RetMultiFont();

	BOOL		Cut();
	BOOL		Copy();
	BOOL		Paste();
	BOOL		Undo();

	void		SetFontSize(float size);

	BOOL		ClearFormat();
	BOOL		SetFormat(int cursor1, int cursor2, int format);

protected:
	void		SendModifEvent();
	void		MouseDoubleClick(FPOINT mouse);
	void		MouseClick(FPOINT mouse);
	void		MouseMove(FPOINT mouse);
	void		MouseRelease(FPOINT mouse);
	int			MouseDetect(FPOINT mouse);
	void		MoveAdjust();

	void		DrawImage(FPOINT pos, char *name, float width, float offset, float height, int nbLine);
	void		DrawBack(FPOINT pos, FPOINT dim);
	void		DrawPart(FPOINT pos, FPOINT dim, int icon);

	void		FreeImage();
	void		LoadImage(char *name);
	void		Scroll(int pos, BOOL bAdjustCursor);
	void		Scroll();
	void		MoveChar(int move, BOOL bWord, BOOL bSelect);
	void		MoveLine(int move, BOOL bWord, BOOL bSelect);
	void		MoveHome(BOOL bWord, BOOL bSelect);
	void		MoveEnd(BOOL bWord, BOOL bSelect);
	void		ColumnFix();
	void		Insert(char character);
	void		InsertOne(char character);
	void		Delete(int dir);
	void		DeleteOne(int dir);
	int			IndentCompute();
	int			IndentTabCount();
	void		IndentTabAdjust(int number);
	BOOL		Shift(BOOL bLeft);
	BOOL		MinMaj(BOOL bMaj);
	void		Justif();
	int			RetCursorLine(int cursor);

	void		UndoFlush();
	void		UndoMemorize(OperUndo oper);
	BOOL		UndoRecall();

protected:
	CScroll*	m_scroll;			// ascenseur vertical à droite

	int			m_maxChar;			// lg max du buffer m_text
	char*		m_text;				// texte (sans zéro terminateur)
	char*		m_format;			// format des caractères
	int			m_len;				// longueur utilisée dans m_text
	int			m_cursor1;			// offset curseur
	int			m_cursor2;			// offset curseur

	BOOL		m_bMulti;			// TRUE -> multi lignes
	BOOL		m_bEdit;			// TRUE -> éditable
	BOOL		m_bHilite;			// TRUE -> hilitable
	BOOL		m_bInsideScroll;	// TRUE -> ascenseur dans le cadre
	BOOL		m_bDisplaySpec;		// TRUE -> affiche les caractères spéciaux
	BOOL		m_bMultiFont;		// TRUE -> plusieurs fontes possible
	BOOL		m_bSoluce;			// TRUE -> montre les liens-solution
	BOOL		m_bGeneric;			// TRUE -> générique qui défile
	BOOL		m_bAutoIndent;		// TRUE -> indentation automatique
	float		m_lineHeight;		// hauteur d'une ligne
	float		m_lineAscent;		// hauteur au-dessus de la ligne de base
	float		m_lineDescent;		// hauteur au-dessous de la ligne de base
	int			m_lineVisible;		// nb total de ligne affichables
	int			m_lineFirst;		// première ligne affichée
	int			m_lineTotal;		// nb lignes utilisées (ds m_lineOffset)
	int			m_lineOffset[EDITLINEMAX];
	char		m_lineIndent[EDITLINEMAX];
	int			m_imageTotal;
	ImageLine	m_image[EDITIMAGEMAX];
	float		m_time;				// temps absolu
	float		m_timeBlink;
	float		m_timeLastClick;
	float		m_timeLastScroll;
	FPOINT		m_mouseFirstPos;
	FPOINT		m_mouseLastPos;
	float		m_column;

	BOOL		m_bCapture;

	BOOL		m_bUndoForce;
	OperUndo	m_undoOper;
	EditUndo	m_undo[EDITUNDOMAX];
};


#endif //_EDIT_H_
