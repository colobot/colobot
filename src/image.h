// image.h

#ifndef _IMAGE_H_
#define	_IMAGE_H_


#include "control.h"


class CD3DEngine;



class CImage : public CControl
{
public:
	CImage(CInstanceManager* iMan);
	~CImage();

	BOOL	Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);

	BOOL	EventProcess(const Event &event);

	void	Draw();

	void	SetFilenameImage(char *name);
	char*	RetFilenameImage();

protected:

protected:
	char	m_filename[100];
};


#endif //_IMAGE_H_
