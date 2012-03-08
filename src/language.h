// language.h

#define _FULL			TRUE		// CoLoBoT
#define _SCHOOL			FALSE		// CeeBot-A ou Teen
  #define _TEEN			FALSE		// FALSE si CeeBot-A, TRUE si CeeBot-Teen
  #define _EDU			FALSE
  #define _PERSO		FALSE
  #define _CEEBOTDEMO	FALSE
#define _NET			FALSE
#define _DEMO			FALSE		// DEMO uniquement de CoLoBoT (avec _FULL = FALSE) !

#define _FRENCH			TRUE
#define _ENGLISH		FALSE
#define _GERMAN			FALSE
#define _WG				FALSE
#define _POLISH			FALSE

#define _NEWLOOK		FALSE		// FALSE pour CoLoBoT, TRUE pour tous les CeeBot
#define _SOUNDTRACKS	FALSE		// toujours FALSE depuis que InitAudioTrackVolume plante sous Vista


// Vérifications

#if !_FULL & !_SCHOOL & !_NET & !_DEMO
-> aucune version choisie !!!
#endif

#if _SCHOOL
#if !_EDU & !_PERSO & !_CEEBOTDEMO
-> EDU ou PERSO ou CEEBOTDEMO ?
#endif
#if _EDU & _PERSO & _CEEBOTDEMO
-> pas EDU et PERSO et CEEBOTDEMO en même temps !!!
#endif
#endif
