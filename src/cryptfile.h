// cryptfile.h

#ifndef _CRYPTFILE_H_
#define	_CRYPTFILE_H_




class CCryptFile
{
public:
	CCryptFile();
	~CCryptFile();

	BOOL	Open(char *filename, char *mode, BOOL bWriteCrypt=TRUE);
	BOOL	GetLine(char *buffer, int max);
	BOOL	PutLine(char *buffer);
	BOOL	Close();

protected:

protected:
	char			m_filename[100];
	char			m_mode;
	BOOL			m_bCrypt;
	BOOL			m_bWriteCrypt;
	unsigned char*	m_data;
	int				m_length;
	int				m_seek;
};


#endif //_CRYPTFILE_H_
