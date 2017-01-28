// mainmap.h

#ifndef _MAINMAP_H_
#define	_MAINMAP_H_


class CInstanceManager;
class CEvent;
class CD3DEngine;
class CInterface;
class CObject;



class CMainMap
{
public:
	CMainMap(CInstanceManager* iMan);
	~CMainMap();

	void		UpdateMap();
	void		CreateMap();
	void		FloorColorMap(D3DCOLORVALUE floor, D3DCOLORVALUE water);
	void		ShowMap(BOOL bShow);
	void		DimMap();
	float		RetZoomMap();
	void		ZoomMap(float zoom);
	void		ZoomMap();
	void		MapEnable(BOOL bEnable);
	BOOL		RetShowMap();
	CObject*	DetectMap(FPOINT pos, BOOL &bInMap);
	void		SetHilite(CObject* pObj);

protected:
	void		CenterMap();

protected:
	CInstanceManager* m_iMan;
	CEvent*			m_event;
	CD3DEngine*		m_engine;
	CInterface*		m_interface;

	int				m_mapMode;
};


#endif //_MAINMAP_H_
