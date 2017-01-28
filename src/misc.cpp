// misc.cpp

#define STRICT
#define D3D_OVERLOADS

#include <math.h>
#include <stdio.h>
#include <direct.h>
#include <time.h>
#include <d3d.h>

#include "struct.h"
#include "D3DEngine.h"
#include "D3DMath.h"
#include "D3DUtil.h"
#include "language.h"
#include "math3d.h"
#include "event.h"
#include "misc.h"



CMetaFile	g_metafile;

static EventMsg		g_uniqueEventMsg = EVENT_USER;
static BOOL			g_bUserDir = FALSE;
static char			g_userDir[100] = "";



// Donne un événement utilisateur unique.

EventMsg GetUniqueEventMsg()
{
	int		i;

	i = (int)g_uniqueEventMsg+1;
	g_uniqueEventMsg = (EventMsg)i;
	return g_uniqueEventMsg;
}



// Retourne une lettre non accentuée.

char RetNoAccent(char letter)
{
	if ( letter < 0 )
	{
		if ( letter == 'á' ||
			 letter == 'à' ||
			 letter == 'â' ||
			 letter == 'ä' ||
			 letter == 'ã' )  return 'a';

		if ( letter == 'é' ||
			 letter == 'è' ||
			 letter == 'ê' ||
			 letter == 'ë' )  return 'e';

		if ( letter == 'í' ||
			 letter == 'ì' ||
			 letter == 'î' ||
			 letter == 'ï' )  return 'i';

		if ( letter == 'ó' ||
			 letter == 'ò' ||
			 letter == 'ô' ||
			 letter == 'ö' ||
			 letter == 'õ' )  return 'o';

		if ( letter == 'ú' ||
			 letter == 'ù' ||
			 letter == 'û' ||
			 letter == 'ü' )  return 'u';

		if ( letter == 'ç' )  return 'c';

		if ( letter == 'ñ' )  return 'n';

		if ( letter == 'Á' ||
			 letter == 'À' ||
			 letter == 'Â' ||
			 letter == 'Ä' ||
			 letter == 'Ã' )  return 'A';

		if ( letter == 'É' ||
			 letter == 'È' ||
			 letter == 'Ê' ||
			 letter == 'Ë' )  return 'E';

		if ( letter == 'Í' ||
			 letter == 'Ì' ||
			 letter == 'Î' ||
			 letter == 'Ï' )  return 'I';

		if ( letter == 'Ó' ||
			 letter == 'Ò' ||
			 letter == 'Ô' ||
			 letter == 'Ö' ||
			 letter == 'Õ' )  return 'O';

		if ( letter == 'Ú' ||
			 letter == 'Ù' ||
			 letter == 'Û' ||
			 letter == 'Ü' )  return 'U';

		if ( letter == 'Ç' )  return 'C';

		if ( letter == 'Ñ' )  return 'N';
	}

	return letter;
}

// Retourne une lettre majuscule.

char RetToUpper(char letter)
{
	if ( letter < 0 )
	{
		if ( letter == 'á' )  return 'Á';
		if ( letter == 'à' )  return 'À';
		if ( letter == 'â' )  return 'Â';
		if ( letter == 'ä' )  return 'Ä';
		if ( letter == 'ã' )  return 'Ã';

		if ( letter == 'é' )  return 'É';
		if ( letter == 'è' )  return 'È';
		if ( letter == 'ê' )  return 'Ê';
		if ( letter == 'ë' )  return 'Ë';

		if ( letter == 'í' )  return 'Í';
		if ( letter == 'ì' )  return 'Ì';
		if ( letter == 'î' )  return 'Î';
		if ( letter == 'ï' )  return 'Ï';

		if ( letter == 'ó' )  return 'Ó';
		if ( letter == 'ò' )  return 'Ò';
		if ( letter == 'ô' )  return 'Ô';
		if ( letter == 'ö' )  return 'Ö';
		if ( letter == 'õ' )  return 'Õ';

		if ( letter == 'ú' )  return 'Ú';
		if ( letter == 'ù' )  return 'Ù';
		if ( letter == 'û' )  return 'Û';
		if ( letter == 'ü' )  return 'Ü';

		if ( letter == 'ç' )  return 'Ç';

		if ( letter == 'ñ' )  return 'Ñ';
	}

	return toupper(letter);
}

// Retourne une lettre minuscule.

char RetToLower(char letter)
{
	if ( letter < 0 )
	{
		if ( letter == 'Á' )  return 'á';
		if ( letter == 'À' )  return 'à';
		if ( letter == 'Â' )  return 'â';
		if ( letter == 'Ä' )  return 'ä';
		if ( letter == 'Ã' )  return 'ã';

		if ( letter == 'É' )  return 'é';
		if ( letter == 'È' )  return 'è';
		if ( letter == 'Ê' )  return 'ê';
		if ( letter == 'Ë' )  return 'ë';

		if ( letter == 'Í' )  return 'í';
		if ( letter == 'Ì' )  return 'ì';
		if ( letter == 'Î' )  return 'î';
		if ( letter == 'Ï' )  return 'ï';

		if ( letter == 'Ó' )  return 'ó';
		if ( letter == 'Ò' )  return 'ò';
		if ( letter == 'Ô' )  return 'ô';
		if ( letter == 'Ö' )  return 'ö';
		if ( letter == 'Õ' )  return 'õ';

		if ( letter == 'Ú' )  return 'ú';
		if ( letter == 'Ù' )  return 'ù';
		if ( letter == 'Û' )  return 'û';
		if ( letter == 'Ü' )  return 'ü';

		if ( letter == 'Ç' )  return 'ç';

		if ( letter == 'Ñ' )  return 'ñ';
	}

	return tolower(letter);
}


// Convertion "\n" en (char)10.

void RemoveEscape(char *dst, char *src)
{
	while ( *src != 0 )
	{
		if ( src[0] == '\\' && src[1] == 'n' )
		{
			*dst++ = '\n';
			src += 2;
		}
		else
		{
			*dst++ = *src++;
		}
	}
	*dst = 0;
}

// Convertion (char)10 en "\n".

void InsertEscape(char *dst, char *src)
{
	while ( *src != 0 )
	{
		if ( *src == '\n' )
		{
			*dst++ = '\\';
			*dst++ = 'n';
			src ++;
		}
		else
		{
			*dst++ = *src++;
		}
	}
	*dst = 0;
}

// Convertion "\n" ou (char)10 en espace.

void SpaceEscape(char *dst, char *src)
{
	while ( *src != 0 )
	{
		if ( src[0] == '\\' && src[1] == 'n' )
		{
			*dst++ = ' ';
			src += 2;
		}
		else if ( *src == '\n' )
		{
			*dst++ = ' ';
			src ++;
		}
		else
		{
			*dst++ = *src++;
		}
	}
	*dst = 0;
}


// Conversion du temps en chaîne.

void TimeToAscii(time_t time, char *buffer)
{
	struct tm	when;
	int			year;

	when = *localtime(&time);
	year = when.tm_year+1900;
	if ( year < 2000 )  year -= 1900;
	else                year -= 2000;
#if _FRENCH
	sprintf(buffer, "%.2d.%.2d.%.2d %.2d:%.2d",
					when.tm_mday, when.tm_mon+1, year,
					when.tm_hour, when.tm_min);
#endif
#if _DEUTSCH
	sprintf(buffer, "%.2d.%.2d.%.2d %.2d:%.2d",
					when.tm_mday, when.tm_mon+1, year,
					when.tm_hour, when.tm_min);
#endif
#if _ITALIAN
	sprintf(buffer, "%.2d.%.2d.%.2d %.2d:%.2d",
					when.tm_mday, when.tm_mon+1, year,
					when.tm_hour, when.tm_min);
#endif
#if _SPANISH
	sprintf(buffer, "%.2d.%.2d.%.2d %.2d:%.2d",
					when.tm_mday, when.tm_mon+1, year,
					when.tm_hour, when.tm_min);
#endif
#if _PORTUGUESE
	sprintf(buffer, "%.2d.%.2d.%.2d %.2d:%.2d",
					when.tm_mday, when.tm_mon+1, year,
					when.tm_hour, when.tm_min);
#endif
#if _ENGLISH
	char		format[10];
	int			hour;

	hour = when.tm_hour;  // 0..23
	if ( hour < 12 )  // matin ?
	{
		strcpy(format, "am");
	}
	else	// après-midi ?
	{
		strcpy(format, "pm");
		hour -= 12;  // 0..11
	}
	if ( hour == 0 )  hour = 12;

	sprintf(buffer, "%.2d.%.2d.%.2d %.2d:%.2d %s",
					when.tm_mon+1, when.tm_mday, year,
					hour, when.tm_min, format);
#endif
}

// Formate un temps en minutes, secondes et centièmes.

void PutTime(char *buffer, float time)
{
	float	m,s,c;

	m = time/60.0f;
	s = Mod(time, 60.0f);
	c = Mod(s, 1.0f);
	s -= c;
	sprintf(buffer, "%d:%.2d:%.2d", (int)m, (int)s, (int)(c*100));
}

// Formate un nombre en %, aligné à droite.

void Put100(char *buffer, int value)
{
	if ( value <  10 )  *buffer++ = ' ';
	if ( value < 100 )  *buffer++ = ' ';

	sprintf(buffer, "  %d %%", value);
}

// Formate un nombre de points, aligné à droite.

void PutPoints(char *buffer, int value)
{
	if ( value <  10 )  { *buffer++ = ' ';  *buffer++ = ' '; }
	if ( value < 100 )  { *buffer++ = ' ';  *buffer++ = ' '; }

	sprintf(buffer, " >  %d  <", value);
}



// Effectue une copie d'un fichier.

BOOL Xfer(char* src, char* dst)
{
	FILE	*fs, *fd;
	char	*buffer;
	int		len;

	fs = fopen(src, "rb");
	if ( fs == 0 )
	{
		return FALSE;
	}

	fd = fopen(dst, "wb");
	if ( fd == 0 )
	{
		fclose(fs);
		return FALSE;
	}

	buffer = (char*)malloc(10000);

	while ( TRUE )
	{
		len = fread(buffer, 1, 10000, fs);
		if ( len == 0 )  break;
		fwrite(buffer, 1, len, fd);
	}

	free(buffer);
	fclose(fs);
	fclose(fd);
	return TRUE;
}

// Copie un fichier dans le dossier temporaire.

BOOL CopyFileToTemp(char* filename)
{
	char	src[100];
	char	dst[100];
	char	save[100];

	UserDir(src, filename, "textures");

	strcpy(save, g_userDir);
	strcpy(g_userDir, "temp");
	UserDir(dst, filename, "textures");
	strcpy(g_userDir, save);

	_mkdir("temp");
	if ( !Xfer(src, dst) )  return FALSE;

	strcpy(filename, dst);
	return TRUE;
}

// Copie une liste de fichiers numérotés dans le dossier temporaire.

BOOL CopyFileListToTemp(char* filename, int* list, int total)
{
	char	name[100];
	char	ext[10];
	char	file[100];
	char	save[100];
	char*	p;
	int		i;

	strcpy(name, filename);
	p = strchr(name, '.');
	if ( p == 0 )
	{
		strcpy(ext, ".tga");
	}
	else
	{
		strcpy(ext, p);
		*p = 0;
	}

	for ( i=0 ; i<total ; i++ )
	{
		sprintf(file, "%s%.3d%s", name, list[i], ext);  // nameNNN.ext
		CopyFileToTemp(file);
	}

	strcpy(save, g_userDir);
	strcpy(g_userDir, "temp");
	UserDir(file, filename, "textures");
	strcpy(filename, file);
	strcpy(g_userDir, save);

	return TRUE;
}


// Ajoute une extension à un fichier, s'il n'en a pas déjà une.

void AddExt(char* filename, char* ext)
{
	if ( strchr(filename, '.') != 0 )  return;  // déjà une extension ?
	strcat(filename, ext);
}


// Spécifie le dossier utilisateur.

void UserDir(BOOL bUser, char* dir)
{
	g_bUserDir = bUser;
	strcpy(g_userDir, dir);
}

// Remplace la chaîne %user% par le dossier utilisateur.
// in:	dir = "%user%toto.txt"
//		def = "abc\"
// out:	buffer = "abc\toto.txt"

void UserDir(char* buffer, char* dir, char* def)
{
	char	ddir[100];
	char*	add;

	if ( strstr(dir, "\\") == 0 && def[0] != 0 )
	{
		sprintf(ddir, "%s\\%s", def, dir);
	}
	else
	{
		strcpy(ddir, dir);
	}
	dir = ddir;

	while ( *dir != 0 )
	{
		if ( dir[0] == '%' &&
			 dir[1] == 'u' &&
			 dir[2] == 's' &&
			 dir[3] == 'e' &&
			 dir[4] == 'r' &&
			 dir[5] == '%' )  // %user% ?
		{
			if ( g_bUserDir )  add = g_userDir;
			else               add = def;

			while ( *add != 0 )
			{
				*buffer++ = *add++;
			}
			dir += 6;  // saute %user%
			continue;
		}

		*buffer++ = *dir++;
	}
	*buffer = 0;
}


// Retourne la lettre correspondant à la langue.

char RetLanguageLetter()
{
#if _FRENCH
	return 'F';
#endif
#if _ENGLISH
	return 'E';
#endif
#if _DEUTSCH
	return 'D';
#endif
#if _ITALIAN
	return 'I';
#endif
#if _SPANISH
	return 'S';
#endif
#if _PORTUGUESE
	return 'P';
#endif
	return 'X';
}

