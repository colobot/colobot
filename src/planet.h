// planet.h

#ifndef _PLANET_H_
#define	_PLANET_H_


class CInstanceManager;
class CD3DEngine;



#define MAXPLANET	10

typedef struct
{
	char		bUsed;		// TRUE -> planète existe
	FPOINT		start;		// position initiale en degrés
	FPOINT		angle;		// position actuelle en degrés
	float		dim;		// dimensions (0..1)
	float		speed;		// vitesse
	float		dir;		// direction dans le ciel
	char		name[20];	// nom de la texture
	FPOINT		uv1, uv2;	// mapping de la texture
	char		bTGA;		// texture .TGA
}
Planet;




class CPlanet
{
public:
	CPlanet(CInstanceManager* iMan, CD3DEngine* engine);
	~CPlanet();

	void		Flush();
	BOOL		EventProcess(const Event &event);
	BOOL		Create(int mode, FPOINT start, float dim, float speed, float dir, char *name, FPOINT uv1, FPOINT uv2);
	BOOL		PlanetExist();
	void		LoadTexture();
	void		Draw();
	void		SetMode(int mode);
	int			RetMode();

protected:
	BOOL		EventFrame(const Event &event);

protected:
	CInstanceManager*	m_iMan;
	CD3DEngine*			m_engine;

	float			m_time;
	int				m_mode;
	Planet			m_planet[2][MAXPLANET];
	BOOL			m_bPlanetExist;
};


#endif //_PLANET_H_
