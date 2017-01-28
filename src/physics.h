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
class CRobotMain;
class CSound;

enum ParticuleType;


enum PhysicsType
{
	TYPE_RACE		= 1,
	TYPE_TANK		= 2,
	TYPE_MASS		= 3,
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
	MO_REAACCEL		= 12,
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
	D3DVECTOR	realAccel;		// accélération réelle (+/-)

	D3DVECTOR	finalInclin;	// inclinaison finale
}
Motion;

typedef struct
{
	D3DVECTOR	p[4];	// fr,fl,rl,rr
	FPOINT		min;
	FPOINT		max;
}
Corner;




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
	float		RetWheelSpeed(BOOL bFront);

	void		SetMotor(BOOL bState);
	BOOL		RetMotor();
	void		SetSilent(BOOL bState);
	BOOL		RetSilent();
	void		SetForceSlow(BOOL bState);
	BOOL		RetForceSlow();
	void		SetLand(BOOL bState);
	BOOL		RetLand();
	void		SetSwim(BOOL bState);
	BOOL		RetSwim();
	void		SetCollision(BOOL bCollision);
	BOOL		RetCollision();
	void		SetFreeze(BOOL bFreeze);
	BOOL		RetFreeze();
	BOOL		RetLight(int rank);
	void		SuspForce(int rank, float force, float front, float rTime);

	void		SetMotorSpeed(D3DVECTOR speed);
	void		SetMotorSpeedX(float speed);
	void		SetMotorSpeedY(float speed);
	void		SetMotorSpeedZ(float speed);
	void		SetMotorSpeedW(float speed);
	D3DVECTOR	RetMotorSpeed();
	float		RetMotorSpeedX();
	float		RetMotorSpeedY();
	float		RetMotorSpeedZ();
	float		RetMotorSpeedW();
	void		ForceMotorSpeedX(float speed);
	void		ForceMotorSpeedY(float speed);
	void		ForceMotorSpeedZ(float speed);
	float		RetMotorRPM();
	int			RetMotorState();

	int			CrashCornerRect(CObject *pObj, const D3DVECTOR &pos, const D3DVECTOR &angle, FPOINT &adjust, FPOINT &inter, float &chocAngle, float &priority, float &hardness, Sound &sound, float aTime, float rTime);
	BOOL		CrashValidity(CObject *pObj, FPOINT adjust);
	int			CrashCornerCircle(CObject *pObj, const D3DVECTOR &pos, const D3DVECTOR &angle, FPOINT &adjust, FPOINT &inter, float &chocAngle, float &priority, float &hardness, Sound &sound, float aTime, float rTime);
	int			CrashCornerDo(CObject *pObj, D3DVECTOR &pos, FPOINT adjust, FPOINT inter, float angle, float priority, float hardness, Sound sound, float aTime, float rTime);
	void		UpdateCorner();
	void		UpdateCorner(const D3DVECTOR &pos, const D3DVECTOR &angle);
	void		UpdateGlideSound(float rTime);

	void		CreateInterface(BOOL bSelect);
	Error		RetError();

	void		FFBCrash(float amplitude, float speed, float frequency);
	void		FFBForce(float amplitude);

protected:
	void		HornEvent(BOOL bPress);
	BOOL		EventFrame(const Event &event);
	void		WaterFrame(float aTime, float rTime);
	void		FFBFrame(float aTime, float rTime);
	BOOL		RetLock();
	void		SoundMotor(float rTime);
	void		SoundMotorFull(float rTime, ObjectType type);
	void		SoundMotorSlow(float rTime, ObjectType type);
	void		SoundMotorStop(float rTime, ObjectType type);
	void		SoundReactorFull(float rTime, ObjectType type);
	void		SoundReactorStop(float rTime, ObjectType type);
	void		MotorUpdate(float aTime, float rTime);
	void		EffectUpdate(float aTime, float rTime);
	void		UpdateMotionStruct(float rTime, Motion &motion);
	void		FloorAdapt(float aTime, float rTime, D3DVECTOR &pos, D3DVECTOR &angle);
	void		FloorAngle(const D3DVECTOR &pos, D3DVECTOR &angle);
	int			ObjectAdapt(D3DVECTOR &pos, D3DVECTOR &angle, float aTime, float rTime);
	D3DVECTOR	ChocRepulse(CObject *pObj1, CObject *pObj2, D3DVECTOR p1, D3DVECTOR p2);
	void		ChocObject(CObject *pObj, D3DVECTOR repulse);
	float		ChocSpin(D3DVECTOR obstacle, float force);
	BOOL		JostleObject(CObject* pObj, D3DVECTOR iPos, float iRad, D3DVECTOR oPos, float oRad);
	BOOL		JostleObject(CObject* pObj, float force);
	BOOL		ExploOther(ObjectType iType, CObject *pObj, ObjectType oType, float force, float hardness);
	int			ExploHimself(ObjectType iType, ObjectType oType, float force, float hardness, D3DVECTOR impact);
	void		DoorCounter(CObject *pObj, ObjectType oType);
	void		HornAction();

	void		CrashParticule(float crash, BOOL bCrash);
	void		SlideParticule(float aTime, float rTime, float fDusty, float fSound);
	void		WheelParticule(float aTime, float rTime, float force, BOOL bBrake);
	void		SuspParticule(float aTime, float rTime);
	void		GazCompute(int model, D3DMATRIX *mat, int i, float dir, float factor, D3DVECTOR &pos, D3DVECTOR &speed);
	void		MotorParticule(float aTime, float rTime);
	void		WaterParticule(float aTime, D3DVECTOR pos, ObjectType type, float floor, float advance, float turn);

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
	CRobotMain*	m_main;
	CSound*		m_sound;

	PhysicsType	m_type;			// TYPE_*
	float		m_gravity;		// force de gravitation
	float		m_time;			// temps absolu
	D3DVECTOR	m_motorSpeed;	// vitesse du moteur (-1..1)
	float		m_motorSpeedW;	// frein à main
	float		m_motorSpeedWk;	// frein à main (selon clavier)
	D3DVECTOR	m_forceSpeed;	// vitesse du moteur forcée (même si lock)
	D3DVECTOR	m_imprecisionA;	// imprécision mécanique actuelle
	D3DVECTOR	m_imprecisionH;	// imprécision mécanique souhaitée
	D3DVECTOR	m_imprecisionT;	// imprécision mécanique temps
	Motion		m_linMotion;	// mouvement linéaire
	Motion		m_cirMotion;	// mouvement circulaire
	float		m_graviSpeed;	// vitesse de chute dû à la gravitation
	float		m_graviGlu;		// colle au sol après un choc (0..1)
	float		m_centriSpeed;	// vitesse de déport dû à la force centripète
	float		m_wheelSlide;	// patinage à l'accélération
	float		m_overTurn;		// facteur de survirage
	float		m_overTurnCur;	// facteur de survirage
	float		m_overBrake;	// facteur de survirage avec frein à main
	float		m_suspTime[2];	// temps de l'oscillation de la suspension
	float		m_suspHeight[2];// hauteur actuelle des amortisseurs
	float		m_suspEnergy[2][5];// énergies de la suspension
	float		m_suspDelayed[2];// force de la suspension
	float		m_chocSpin;		// vitesse circulaire de choc
	float		m_chocAngle;
	float		m_motorAngle;
	float		m_motorRPM;		// vitesse du moteur (0..1..2)
	int			m_motorState;	// rapport de la boîte de vitesses
	BOOL		m_bMotor;		// moteur en marche
	BOOL		m_bSilent;		// moteur stoppé
	BOOL		m_bForceSlow;	// force moteur au ralenti
	BOOL		m_bBrake;		// freinage à pied en cours
	BOOL		m_bHandbrake;	// freinage à main en cours
	BOOL		m_bLand;
	BOOL		m_bSwim;
	BOOL		m_bCollision;
	BOOL		m_bObstacle;
	BOOL		m_bFreeze;
	BOOL		m_bWater;		// sous l'eau ?
	BOOL		m_bOldWater;
	int			m_repeatCollision;
	int			m_doorRank;
	float		m_linVibrationFactor;
	float		m_cirVibrationFactor;
	float		m_inclinaisonFactor;
	float		m_lastSlideParticule;
	float		m_lastMotorParticule;
	float		m_lastWaterParticule;
	float		m_lastUnderParticule;
	float		m_lastPloufParticule;
	float		m_lastFlameParticule;
	float		m_lastCrashParticule;
	float		m_lastWheelParticule;
	float		m_lastSuspParticule;
	BOOL		m_bWheelParticuleBrake;
	D3DVECTOR	m_wheelParticulePos[4];
	float		m_absorbWater;
	float		m_reactorTemperature;
	float		m_timeReactorFail;
	float		m_timeUnderWater;
	float		m_timeWheelBurn;
	float		m_timeMotorBurn;
	float		m_timeLock;
	float		m_lastSoundCollision;
	float		m_lastSoundCrash;
	float		m_lastSoundWater;
	float		m_lastSoundInsect;
	float		m_restBreakParticule;
	float		m_floorLevel;	// niveau du sol
	float		m_floorHeight;	// hauteur au-dessus du sol
	float		m_terrainHard;
	int			m_soundChannelMotor1;
	int			m_soundChannelMotor2;
	int			m_soundChannelSlide;
	int			m_soundChannelBrake;
	int			m_soundChannelBoost;
	int			m_soundChannelHorn;
	int			m_soundChannelGlide;
	float		m_soundTimeJostle;
	float		m_soundTimeBoum;
	float		m_soundTimePshhh;
	float		m_soundAmplitudeMotor1;
	float		m_soundAmplitudeMotor2;
	float		m_soundFrequencyMotor1;
	float		m_soundFrequencyMotor2;
	float		m_glideTime;
	float		m_glideVolume;
	float		m_glideAmplitude;
	D3DVECTOR	m_glideImpact;
	BOOL		m_bForceUpdate;
	BOOL		m_bLowLevel;
	BOOL		m_bHornPress;
	Corner		m_curCorner;
	Corner		m_newCorner;
	CObject*	m_lastDoorCounter;
	float		m_FFBamplitude;
	float		m_FFBspeed;
	float		m_FFBfrequency;
	float		m_FFBprogress;
	float		m_FFBforce;
	float		m_FFBbrake;
};


#endif //_PHYSICS_H_
