// gamerfile.h

#ifndef _GAMERFILE_H_
#define	_GAMERFILE_H_



class CInstanceManager;



#define MAXGAMERFILE	500
#define MAXFILENAME		20
#define MAXSELECT		5

typedef struct
{
	char	puzzle[MAXFILENAME+2];
	char	numTry;
	char	bPassed;
	float	totalTime;
}
GamerFile;



class CGamerFile
{
public:
	CGamerFile(CInstanceManager* iMan);
	~CGamerFile();

	void	Flush();

	BOOL	Read(char *filename);
	BOOL	Write();
	BOOL	Delete(char *puzzle);

	void	RetSelect(int i, char *filename);
	void	SetSelect(int i, char *filename);

	int		RetNumTry(char *puzzle);
	BOOL	SetNumTry(char *puzzle, int numTry);

	BOOL	RetPassed(char *puzzle);
	BOOL	SetPassed(char *puzzle, BOOL bPassed);

	float	RetTotalTime(char *puzzle);
	BOOL	SetTotalTime(char *puzzle, float totalTime);

protected:
	int		SearchIndex(char *puzzle);
	int		CreateIndex(char *puzzle);

protected:
	CInstanceManager* m_iMan;

	char		m_filename[200];
	BOOL		m_bWriteToDo;
	char		m_select[MAXSELECT][MAXFILENAME+2];
	GamerFile	m_gamerFile[MAXGAMERFILE];
};


#endif //_GAMERFILE_H_
