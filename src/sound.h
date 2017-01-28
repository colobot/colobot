// sound.h


#include <dsound.h>


#define MAXFILES	200
#define MAXSOUND	32
#define MAXVOLUME	20
#define MAXOPER		4

class CInstanceManager;
class CD3DEngine;


enum Sound
{
	SOUND_CLICK			= 0,
	SOUND_BOUM			= 1,
	SOUND_EXPLO			= 2,
	SOUND_STEPs			= 3,	// smooth
	SOUND_ERROR			= 4,
	SOUND_ENERGY		= 5,
	SOUND_PLOUF			= 6,
	SOUND_BLUP			= 7,
	SOUND_WARNING		= 8,
	SOUND_BURN			= 9,
	SOUND_TZOING		= 10,
	SOUND_MANIP			= 11,
	SOUND_PSHHH			= 12,
	SOUND_NUCLEAR		= 13,
	SOUND_OPEN			= 14,
	SOUND_CLOSE			= 15,
	SOUND_MESSAGE		= 16,
	SOUND_BOUMm			= 17,	// metal
	SOUND_BOUMv			= 18,	// vegetal
	SOUND_EXPLOl		= 19,	// little
	SOUND_EXPLOlp		= 20,	// little power
	SOUND_EXPLOp		= 21,	// power
	SOUND_STEPh			= 22,	// hard
	SOUND_STEPm			= 23,	// metal
	SOUND_WHEELg		= 24,	// roue sur gravier
	SOUND_CHOCm			= 25,	// choc métalique
	SOUND_CHOCa			= 26,	// choc arbre
	SOUND_CHOCo			= 27,	// choc objet
	SOUND_FALLo1		= 28,	// objet tombe
	SOUND_FALLo2		= 29,	// objet tombe
	SOUND_FALLo3		= 30,	// objet tombe
	SOUND_BLUPIshibi	= 31,
	SOUND_BLUPIouaaa	= 32,
	SOUND_BLUPIhic		= 33,
	SOUND_BLUPIoups		= 34,
	SOUND_BLUPIpfiou	= 35,
	SOUND_BLUPInon		= 36,
	SOUND_BLUPIpousse	= 37,
	SOUND_BLUPIeffort	= 38,
	SOUND_BLUPIaie		= 39,
	SOUND_BLUPIhhuu		= 40,
	SOUND_BLUPIohhh		= 41,
	SOUND_BLUPIgrrr		= 42,
	SOUND_BLUPIpeur		= 43,
	SOUND_BLUPIslurp	= 44,
	SOUND_BLUPIouaou	= 45,
	SOUND_BLUPIblibli	= 46,
	SOUND_CRAZY			= 47,
	SOUND_GLASS			= 48,
	SOUND_TRAX			= 49,
	SOUND_TUTUTU		= 50,
	SOUND_CLOWN			= 51,
	SOUND_FLIC1			= 52,
	SOUND_FLIC2			= 53,
	SOUND_FLIC3			= 54,
	SOUND_JET			= 55,
	SOUND_PERFO2		= 56,
	SOUND_PERFO1		= 57,
	SOUND_GUN			= 58,
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
	float					baseAmplitude;
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
	CD3DEngine*				m_engine;

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

