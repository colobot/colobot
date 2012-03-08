// physics.h

#ifndef _PHYSICS_H_
#define	_PHYSICS_H_


class CInstanceManager;
class CD3DEngine;
class CLight;
class CParticule;
class CTerrain;
class CWater;
class CCamera;
class CObject;
class CBrain;
class CMotion;
class CSound;


enum PhysicsType
{
	TYPE_ROLLING	= 1,
	TYPE_FLYING		= 2,
};

enum PhysicsMode
{
	MO_ADVACCEL		= 0,
	MO_RECACCEL		= 1,
	MO_STOACCEL		= 2,
	MO_MOTACCEL		= 3,
	MO_ADVSPEED		= 4,
	MO_RECSPEED		= 5,
	MO_MOTSPEED		= 6,
	MO_CURSPEED		= 7,
	MO_TERFORCE		= 8,
	MO_TERSPEED		= 9,
	MO_TERSLIDE		= 10,
	MO_REASPEED		= 11,
};


typedef struct
{
	D3DVECTOR	advanceAccel;	// accélération de départ (+)
	D3DVECTOR	recedeAccel;	// accélération de départ (+)
	D3DVECTOR	stopAccel;		// accélération d'arrêt (+)
	D3DVECTOR	motorAccel;		// accélération actuelle (+/-)

	D3DVECTOR	advanceSpeed;	// vitesse en marche avant (+)
	D3DVECTOR	recedeSpeed;	// vitesse en marche arrière (+)
	D3DVECTOR	motorSpeed;		// vitesse souhaitée (+/-)
	D3DVECTOR	currentSpeed;	// vitesse actuelle (+/-)

	D3DVECTOR	terrainForce;	// force de résistance du terrain (+)
	D3DVECTOR	terrainSpeed;	// vitesse du terrain (+/-)
	D3DVECTOR	terrainSlide;	// limite vitesse de glissement (+)

	D3DVECTOR	realSpeed;		// vitesse réelle (+/-)

	D3DVECTOR	finalInclin;	// inclinaison finale
}
Motion;




class CPhysics
{
public:
	CPhysics(CInstanceManager* iMan, CObject* object);
	~CPhysics();

	void		DeleteObject(BOOL bAll=FALSE);

	BOOL		EventProcess(const Event &event);

	void		SetBrain(CBrain* brain);
	void		SetMotion(CMotion* motion);

	void		SetType(PhysicsType type);
	PhysicsType	RetType();

	BOOL		Write(char *line);
	BOOL		Read(char *line);

	void		SetGravity(float value);
	float		RetGravity();

	float		RetFloorHeight();

	void		SetLinMotion(PhysicsMode mode, D3DVECTOR value);
	D3DVECTOR	RetLinMotion(PhysicsMode mode);
	void		SetLinMotionX(PhysicsMode mode, float value);
	void		SetLinMotionY(PhysicsMode mode, float value);
	void		SetLinMotionZ(PhysicsMode mode, float value);
	float		RetLinMotionX(PhysicsMode mode);
	float		RetLinMotionY(PhysicsMode mode);
	float		RetLinMotionZ(PhysicsMode mode);

	void		SetCirMotion(PhysicsMode mode, D3DVECTOR value);
	D3DVECTOR	RetCirMotion(PhysicsMode mode);
	void		SetCirMotionX(PhysicsMode mode, float value);
	void		SetCirMotionY(PhysicsMode mode, float value);
	void		SetCirMotionZ(PhysicsMode mode, float value);
	float		RetCirMotionX(PhysicsMode mode);
	float		RetCirMotionY(PhysicsMode mode);
	float		RetCirMotionZ(PhysicsMode mode);

	float		RetLinStopLength(PhysicsMode sMode=MO_ADVSPEED, PhysicsMode aMode=MO_STOACCEL);
	float		RetCirStopLength();
	float		RetLinMaxLength(float dir);
	float		RetLinTimeLength(float dist, float dir=1.0f);
	float		RetLinLength(float dist);

	void		SetMotor(BOOL bState);
	BOOL		RetMotor();
	void		SetLand(BOOL bState);
	BOOL		RetLand();
	void		SetSwim(BOOL bState);
	BOOL		RetSwim();
	void		SetCollision(BOOL bCollision);
	BOOL		RetCollision();
	void		SetFreeze(BOOL bFreeze);
	BOOL		RetFreeze();
	void		SetReactorRange(float range);
	float		RetReactorRange();

	void		SetMotorSpeed(D3DVECTOR speed);
	void		SetMotorSpeedX(float speed);
	void		SetMotorSpeedY(float speed);
	void		SetMotorSpeedZ(float speed);
	D3DVECTOR	RetMotorSpeed();
	float		RetMotorSpeedX();
	float		RetMotorSpeedY();
	float		RetMotorSpeedZ();

	void		CreateInterface(BOOL bSelect);
	Error		RetError();

protected:
	BOOL		EventFrame(const Event &event);
	void		WaterFrame(float aTime, float rTime);
	void		SoundMotor(float rTime);
	void		SoundMotorFull(float rTime, ObjectType type);
	void		SoundMotorSlow(float rTime, ObjectType type);
	void		SoundMotorStop(float rTime, ObjectType type);
	void		SoundReactorFull(float rTime, ObjectType type);
	void		SoundReactorStop(float rTime, ObjectType type);
	void		FrameParticule(float aTime, float rTime);
	void		MotorUpdate(float aTime, float rTime);
	void		EffectUpdate(float aTime, float rTime);
	void		UpdateMotionStruct(float rTime, Motion &motion);
	void		FloorAdapt(float aTime, float rTime, D3DVECTOR &pos, D3DVECTOR &angle);
	void		FloorAngle(const D3DVECTOR &pos, D3DVECTOR &angle);
	int			ObjectAdapt(const D3DVECTOR &pos, const D3DVECTOR &angle);
	BOOL		JostleObject(CObject* pObj, D3DVECTOR iPos, float iRad, D3DVECTOR oPos, float oRad);
	BOOL		JostleObject(CObject* pObj, float force);
	BOOL		ExploOther(ObjectType iType, CObject *pObj, ObjectType oType, float force);
	int			ExploHimself(ObjectType iType, ObjectType oType, float force);

	void		PowerParticule(float factor, BOOL bBreak);
	void		CrashParticule(float crash);
	void		MotorParticule(float aTime, float rTime);
	void		WaterParticule(float aTime, D3DVECTOR pos, ObjectType type, float floor, float advance, float turn);
	void		WheelParticule(int color, float width);

protected:
	CInstanceManager* m_iMan;
	CD3DEngine*	m_engine;
	CLight*		m_light;
	CParticule*	m_particule;
	CTerrain*	m_terrain;
	CWater*		m_water;
	CCamera*	m_camera;
	CObject*	m_object;
	CBrain*		m_brain;
	CMotion*	m_motion;
	CSound*		m_sound;

	PhysicsType	m_type;			// TYPE_*
	float		m_gravity;		// force de gravitation
	float		m_time;			// temps absolu
	D3DVECTOR	m_motorSpeed;	// vitesse du moteur (-1..1)
	Motion		m_linMotion;	// mouvement linéaire
	Motion		m_cirMotion;	// mouvement circulaire
	BOOL		m_bMotor;
	BOOL		m_bLand;
	BOOL		m_bSwim;
	BOOL		m_bCollision;
	BOOL		m_bObstacle;
	BOOL		m_bFreeze;
	int			m_repeatCollision;
	float		m_linVibrationFactor;
	float		m_cirVibrationFactor;
	float		m_inclinaisonFactor;
	float		m_lastPowerParticule;
	float		m_lastSlideParticule;
	float		m_lastMotorParticule;
	float		m_lastWaterParticule;
	float		m_lastUnderParticule;
	float		m_lastPloufParticule;
	float		m_lastFlameParticule;
	BOOL		m_bWheelParticuleBrake;
	D3DVECTOR	m_wheelParticulePos[2];
	float		m_absorbWater;
	float		m_reactorTemperature;
	float		m_reactorRange;
	float		m_timeReactorFail;
	float		m_timeUnderWater;
	float		m_lastEnergy;
	float		m_lastSoundWater;
	float		m_lastSoundInsect;
	float		m_restBreakParticule;
	float		m_floorLevel;	// niveau du sol
	float		m_floorHeight;	// hauteur au-dessus du sol
	int			m_soundChannel;
	int			m_soundChannelSlide;
	float		m_soundTimePshhh;
	float		m_soundTimeJostle;
	float		m_soundTimeBoum;
	BOOL		m_bSoundSlow;
	BOOL		m_bForceUpdate;
	BOOL		m_bLowLevel;
};


#endif //_PHYSICS_H_
