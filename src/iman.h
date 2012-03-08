// iman.h

#ifndef _IMAN_H_
#define	_IMAN_H_


#include "misc.h"



typedef struct
{
	int		totalPossible;
	int		totalUsed;
	void**	classPointer;
}
BaseClass;



class CInstanceManager
{
public:
	CInstanceManager();
	~CInstanceManager();

	void	Flush();
	void	Flush(ClassType classType);
	BOOL	AddInstance(ClassType classType, void* pointer, int max=1);
	BOOL	DeleteInstance(ClassType classType, void* pointer);
	void*	SearchInstance(ClassType classType, int rank=0);


protected:
	void	Compress(ClassType classType);

protected:
	BaseClass	m_table[CLASS_MAX];
};


#endif //_IMAN_H_
