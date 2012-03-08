// profile.cpp

#define STRICT
#define D3D_OVERLOADS

#include <stdio.h>
#include <d3d.h>
#include <stdlib.h>

#include "language.h"
#include "struct.h"
#include "profile.h"



static char g_filename[100];



BOOL InitCurrentDirectory()
{
#if _SCHOOL
	_fullpath(g_filename, "ceebot.ini", 100);
#else
	_fullpath(g_filename, "colobot.ini", 100);
#endif
	return TRUE;
}


BOOL SetProfileString(char* section, char* key, char* string)
{
	WritePrivateProfileString(section, key, string, g_filename);
	return TRUE;
}

BOOL GetProfileString(char* section, char* key, char* buffer, int max)
{
	int		nb;

	nb = GetPrivateProfileString(section, key, "", buffer, max, g_filename);
	if ( nb == 0 )
	{
		buffer[0] = 0;
		return FALSE;
	}
	return TRUE;
}


BOOL SetProfileInt(char* section, char* key, int value)
{
	char	s[20];

	sprintf(s, "%d", value);
	WritePrivateProfileString(section, key, s, g_filename);
	return TRUE;
}

BOOL GetProfileInt(char* section, char* key, int &value)
{
	char	s[20];
	int		nb;

	nb = GetPrivateProfileString(section, key, "", s, 20, g_filename);
	if ( nb == 0 )
	{
		value = 0;
		return FALSE;
	}
	sscanf(s, "%d", &value);
	return TRUE;
}


BOOL SetProfileFloat(char* section, char* key, float value)
{
	char	s[20];

	sprintf(s, "%.2f", value);
	WritePrivateProfileString(section, key, s, g_filename);
	return TRUE;
}

BOOL GetProfileFloat(char* section, char* key, float &value)
{
	char	s[20];
	int		nb;

	nb = GetPrivateProfileString(section, key, "", s, 20, g_filename);
	if ( nb == 0 )
	{
		value = 0.0f;
		return FALSE;
	}
	sscanf(s, "%f", &value);
	return TRUE;
}


