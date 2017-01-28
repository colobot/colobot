// sound.h


#include <dsound.h>


#define MAXFILES	200
#define MAXSOUND	32
#define MAXVOLUME	20
#define MAXOPER		4

class CInstanceManager;


enum Sound
{
	SOUND_CLICK		= 0,
	SOUND_BOUM		= 1,
	SOUND_EXPLO		= 2,
	SOUND_FLY		= 3,
	SOUND_STEPs		= 4,	// smooth
	SOUND_MOTOR1	= 5,
	SOUND_MOTOR2	= 6,
	SOUND_ERROR		= 7,
	SOUND_ENERGY	= 8,
	SOUND_PLOUF		= 9,
	SOUND_BLUP		= 10,
	SOUND_WARNING	= 11,
	SOUND_STATION	= 12,
	SOUND_RESEARCH	= 13,
	SOUND_BURN		= 14,
	SOUND_TZOING	= 15,
	SOUND_GGG		= 16,
	SOUND_MANIP		= 17,
	SOUND_SWIM		= 18,
	SOUND_RADAR		= 19,
	SOUND_ALARM		= 20,	// alarme énergie
	SOUND_PSHHH		= 21,
	SOUND_NUCLEAR	= 22,
	SOUND_INFO		= 23,
	SOUND_OPEN		= 24,
	SOUND_CLOSE		= 25,
	SOUND_MESSAGE	= 26,
	SOUND_BOUMm		= 27,	// metal
	SOUND_BOUMv		= 28,	// vegetal
	SOUND_TAKE		= 29,
	SOUND_EXPLOl	= 30,	// little
	SOUND_EXPLOlp	= 31,	// little power
	SOUND_EXPLOp	= 32,	// power
	SOUND_STEPh		= 33,	// hard
	SOUND_STEPm		= 34,	// metal
	SOUND_POWERON	= 35,
	SOUND_POWEROFF	= 36,
	SOUND_WAYPOINT	= 37,
	SOUND_JOSTLE	= 38,
	SOUND_FINDING	= 39,	// trouvé un objet caché
	SOUND_FIREp		= 40,	// tir avec phazer
	SOUND_WHEEL		= 41,	// roue avec pneu
	SOUND_WHEELb	= 42,	// roue sur la jante
	SOUND_HOME1		= 43,
	SOUND_HORN		= 44,
	SOUND_BREAK1	= 45,
	SOUND_BREAK2	= 46,
	SOUND_STARTREADY= 47,
	SOUND_START3	= 48,
	SOUND_START2	= 49,
	SOUND_START1	= 50,
	SOUND_STARTGO	= 51,
	SOUND_SPRINKLE	= 52,
	SOUND_RECORDgall= 53,	// record du circuit
	SOUND_RECORDgone= 54,	// record du tour
	SOUND_RECORDlone= 55,	// meilleur tour
	SOUND_TOUCH		= 56,
	SOUND_MOTOR3	= 57,
	SOUND_MOTOR4	= 58,
	SOUND_MOTOR5	= 59,
	SOUND_MOTOR6	= 60,
	SOUND_EXPLOi	= 61,
	SOUND_HELICO	= 62,
	SOUND_LAP4		= 63,
	SOUND_LAP3		= 64,
	SOUND_LAP2		= 65,
	SOUND_LAP1		= 66,
	SOUND_BOT1c		= 67,
	SOUND_BOT1p		= 68,
	SOUND_BOT2c		= 69,
	SOUND_BOT2p		= 70,
	SOUND_BOT3c		= 71,
	SOUND_BOT3p		= 72,
	SOUND_BOT4c		= 73,
	SOUND_BOT4p		= 74,
	SOUND_EVIL1		= 75,
	SOUND_WHEELg	= 76,	// roue sur gravier
	SOUND_CHOCm		= 77,	// choc métalique
	SOUND_CHOCa		= 78,	// choc arbre
	SOUND_FALLg		= 79,	// tombe dans le gravier
	SOUND_CHOCo		= 80,	// choc objet
	SOUND_FALLo1	= 81,	// objet tombe
	SOUND_FALLo2	= 82,	// objet tombe
	SOUND_FALLo3	= 83,	// objet tombe
	SOUND_MOTORs	= 84,	// démarrage moteur
};

enum SoundNext
{
	SOPER_CONTINUE	= 1,
	SOPER_STOP		= 2,
	SOPER_LOOP		= 3,
};

typedef struct
{
	char		bUsed;
	float		finalAmplitude;
	float		finalFrequency;
	float		totalTime;
	float		currentTime;
	SoundNext	nextOper;
}
SoundOper;

typedef struct
{
	char					bUsed;			// buffer utilisé ?
	char					bMute;			// silence ?
	Sound					type;			// SOUND_*
	int						priority;		// si grand -> important
	D3DVECTOR				pos;			// position dans l'espace
	unsigned short			uniqueStamp;	// marqueur unique
	LPDIRECTSOUNDBUFFER		soundBuffer;
	LPDIRECTSOUND3DBUFFER	soundBuffer3D;
	float					startAmplitude;
	float					changeAmplitude;
	float					currentAmplitude;
	float					startFrequency;
	float					changeFrequency;
	float					currentFrequency;
	int						initFrequency;
	float					volume;			// 2D: volume 1..0 selon position
	float					pan;			// 2D: pan -1..+1 selon position
	SoundOper				oper[MAXOPER];
}
SoundChannel;



class CSound
{
public:
	CSound(CInstanceManager* iMan);
	~CSound();

	void	SetDebugMode(BOOL bMode);
	BOOL	Create(HWND hWnd, BOOL b3D);
	void	CacheAll();

	void	SetState(BOOL bState);
	BOOL	RetEnable();

	void	SetCDpath(char *path);
	void	SetAudioTrack(BOOL bAudio);

	void	SetSound3D(BOOL bMode);
	BOOL	RetSound3D();
	BOOL	RetSound3DCap();

	void	SetComments(BOOL bMode);
	BOOL	RetComments();

	void	SetWater(BOOL bWater);
	BOOL	RetWater();

	void	SetAudioVolume(int volume);
	int		RetAudioVolume();
	void	SetMidiVolume(int volume);
	int		RetMidiVolume();

	void	SetListener(D3DVECTOR eye, D3DVECTOR lookat);
	void	FrameMove(float rTime);

	int		Play(Sound sound, float amplitude=1.0f, float frequency=1.0f, BOOL bLoop=FALSE);
	int		Play(Sound sound, D3DVECTOR pos, float amplitude=1.0f, float frequency=1.0f, BOOL bLoop=FALSE);
	BOOL	FlushEnvelope(int channel);
	BOOL	AddEnvelope(int channel, float amplitude, float frequency, float time, SoundNext oper);
	BOOL	Position(int channel, D3DVECTOR pos);
	BOOL	Amplitude(int channel, float amplitude);
	BOOL	Frequency(int channel, float frequency);
	float	RetAmplitude(int channel);
	float	RetFrequency(int channel);
	BOOL	Stop(int channel);
	BOOL	StopAll();
	BOOL	MuteAll(BOOL bMute);

	BOOL	PlayMusic(int rank, BOOL bRepeat);
	BOOL	RestartMusic();
	void	SuspendMusic();
	void	StopMusic();
	BOOL	IsPlayingMusic();
	void	AdaptVolumeMusic();

protected:
	BOOL	CheckChannel(int &channel);
	BOOL	CreateSoundBuffer(int channel, DWORD size, DWORD freq, DWORD bitsPerSample, DWORD blkAlign, BOOL bStereo);
	BOOL	ReadData(LPDIRECTSOUNDBUFFER lpDSB, Sound sound, DWORD size);
	BOOL	CreateBuffer(int channel, Sound sound);
	void	ComputeVolumePan2D(int channel, const D3DVECTOR &pos);
	BOOL	ReadFile(Sound sound, char *metaname, char *filename);
	int		RetPriority(Sound sound);
	BOOL	SearchFreeBuffer(Sound sound, int &channel, BOOL &bAlreadyLoaded);
	void	OperNext(int channel);
	BOOL	PlayAudioTrack(int rank);

protected:
	CInstanceManager*		m_iMan;

	HWND					m_hWnd;
	BOOL					m_bEnable;
	BOOL					m_bState;
	BOOL					m_bAudioTrack;
	BOOL					m_ctrl3D;
	BOOL					m_bComments;
	BOOL					m_bWater;
	BOOL					m_bDebugMode;
	LPDIRECTSOUND			m_lpDS;
	LPDIRECTSOUND3DLISTENER	m_listener;
	SoundChannel			m_channel[MAXSOUND];
	char*					m_files[MAXFILES];
	UINT					m_MidiDeviceID;
	int						m_MIDIMusic;
	BOOL					m_bRepeatMusic;
	int						m_audioVolume;
	int						m_midiVolume;
	int						m_lastMidiVolume;
	D3DVECTOR				m_eye;
	D3DVECTOR				m_lookat;
	float					m_lastTime;
	float					m_playTime;
	int						m_uniqueStamp;
	int						m_maxSound;
	char					m_CDpath[100];
};

