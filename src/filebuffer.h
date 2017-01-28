// filebuffer.h

#ifndef _FILEBUFFER_H_
#define	_FILEBUFFER_H_


class CInstanceManager;


typedef struct
{
	int		chapter;
	int		rank;
	int		index;
}
LineIndex;



class CFileBuffer
{
public:
	CFileBuffer(CInstanceManager* iMan);
	~CFileBuffer();

	void	Open();
	void	Close();
	BOOL	PutLine(int chapter, char *line);
	BOOL	GetLine(int chapter, int rank, char *line);

protected:

protected:
	CInstanceManager* m_iMan;

	int			m_total;		// lg max de m_buffer
	int			m_length;		// lg utlisée de m_buffer
	char*		m_buffer;		// lignes

	int			m_used;
	LineIndex	m_index[100];
};


#endif //_FILEBUFFER_H_
