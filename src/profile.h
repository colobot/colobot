// profile.h

#ifndef _PROFILE_H_
#define	_PROFILE_H_


#define STRICT
#define D3D_OVERLOADS


extern BOOL	InitCurrentDirectory();
extern BOOL	SetProfileString(char* section, char* key, char* string);
extern BOOL	GetProfileString(char* section, char* key, char* buffer, int max);
extern BOOL	SetProfileInt(char* section, char* key, int value);
extern BOOL	GetProfileInt(char* section, char* key, int &value);
extern BOOL	SetProfileFloat(char* section, char* key, float value);
extern BOOL	GetProfileFloat(char* section, char* key, float &value);


#endif //_PROFILE_H_
