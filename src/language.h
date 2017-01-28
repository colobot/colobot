// language.h

#define _EGAMES			FALSE

#define _FULL			TRUE
#define _SCHOOL			FALSE
#define _NET			FALSE
#define _DEMO			FALSE
#define _SE				FALSE

#define _FRENCH			FALSE
#define _ENGLISH		TRUE
#define _DEUTSCH		FALSE
#define _ITALIAN		FALSE
#define _SPANISH		FALSE
#define _PORTUGUESE		FALSE

#if _ENGLISH
	#define _MPH		TRUE
#else
	#define _MPH		FALSE
#endif

#define _SOUNDTRACKS	FALSE		// toujours FALSE depuis que InitAudioTrackVolume plante sous Vista
