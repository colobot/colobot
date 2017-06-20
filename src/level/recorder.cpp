// recorder.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "D3DEngine.h"
#include "D3DMath.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "math3d.h"
#include "recorder.h"




// Constructeur de l'objet.

CRecorder::CRecorder(CInstanceManager* iMan)
{
   m_iMan = iMan;
   m_iMan->AddInstance(CLASS_RECORDER, this, 10);

   Init();
}

// Destructeur de l'objet.

CRecorder::~CRecorder()
{
   m_iMan->DeleteInstance(CLASS_RECORDER, this);
   Flush();
}


// Prépare le premier bloc, prêt à l'emploi.

void CRecorder::Init()
{
   int     i;

   m_bloc = (RecorderBloc*)malloc(sizeof(RecorderBloc));
   ZeroMemory(m_bloc, sizeof(RecorderBloc));

   m_record = m_bloc;
   m_lastTime = -1.0f;
   m_model = 0;
   m_subModel = 0;
   m_color.r = 0.0f;
   m_color.g = 0.0f;
   m_color.b = 0.0f;
   m_color.a = 0.0f;
   m_type = 0;
   m_mission = 0;
   m_level = 0;
   ZeroMemory(m_gamer, 20);
   m_chrono = 0.0f;

   for ( i=0 ; i<10 ; i++ )
   {
       m_check[i] = 0;
   }
}

// Libère tous les blocs.

void CRecorder::Flush()
{
   RecorderBloc    *bloc, *prev;

   if ( m_bloc == 0 )  return;

   bloc = m_bloc;
   while ( bloc->next != 0 )
   {
       bloc = bloc->next;
   }

   do
   {
       prev = bloc->prev;
       free(bloc);
       bloc = prev;
   }
   while ( bloc != 0 );

   m_bloc = 0;
   m_record = 0;
}


// Gestion du modèle de la voiture.

void CRecorder::SetModel(int model)
{
   m_model = model;
}

int CRecorder::RetModel()
{
   return m_model;
}

void CRecorder::SetSubModel(int subModel)
{
   m_subModel = subModel;
}

int CRecorder::RetSubModel()
{
   return m_subModel;
}


// Gestion de la couleur de la voiture.

void CRecorder::SetColor(D3DCOLORVALUE color)
{
   m_color = color;
}

D3DCOLORVALUE CRecorder::RetColor()
{
   return m_color;
}


// Gestion du type de mission.

void CRecorder::SetType(int rank)
{
   m_type = rank;
}

int CRecorder::RetType()
{
   return m_type;
}


// Gestion du numéro de mission.

void CRecorder::SetMission(int rank)
{
   m_mission = rank;
}

int CRecorder::RetMission()
{
   return m_mission;
}


// Gestion des checksums.

void CRecorder::SetCheck(int rank, int value)
{
   m_check[rank] = value;
}

int CRecorder::RetCheck(int rank)
{
   return m_check[rank];
}


// Gestion du niveau de difficulté.

void CRecorder::SetLevel(int level)
{
   m_level = level;
}

int CRecorder::RetLevel()
{
   return m_level;
}


// Gestion du nom du joueur.

void CRecorder::SetGamer(char *name)
{
   strcpy(m_gamer, name);
}

char* CRecorder::RetGamer()
{
   return m_gamer;
}


// Gestion du temps record.

void CRecorder::SetChrono(float time)
{
   m_chrono = time;
}

float CRecorder::RetChrono()
{
   return m_chrono;
}


// Ajoute un nouvel événement.

BOOL CRecorder::Put(float time, const RecorderEvent &event)
{
   RecorderBloc*   bloc;
   int             i;

   if ( m_bloc == 0 )  return FALSE;
//?    if ( time == m_lastTime )  return TRUE;
   if ( Abs(time-m_lastTime) < 0.1f )  return TRUE;

   if ( m_record->total >= RECORDERBLOC )  // bloc courant plein ?
   {
       bloc = (RecorderBloc*)malloc(sizeof(RecorderBloc));
       ZeroMemory(bloc, sizeof(RecorderBloc));

       m_record->next = bloc;
       bloc->prev = m_record;
       m_record = bloc;
   }

   i = m_record->total;
   m_record->time[i] = time;
   m_record->event[i] = event;

   m_record->total ++;
   m_lastTime = time;
   return TRUE;
}

// Retourne un événement pour un temps quelconque.
// On calcule la valeur la plus proche par approximation linéaire
// entre les événements précédents et suivants.

BOOL CRecorder::Get(float time, RecorderEvent &event)
{
   RecorderBloc    *bloc;
   RecorderEvent   *ePrev, *eNext;
   float           tPrev, tNext, progress;
   int             i;

   if ( m_bloc == 0 )  return FALSE;

   bloc = m_bloc;
   while ( time > bloc->time[bloc->total-1] && bloc->next != 0 )
   {
       bloc = bloc->next;
   }

   for ( i=0 ; i<bloc->total-1 ; i++ )
   {
       if ( time <= bloc->time[i] )  break;
   }
   tNext = bloc->time[i];
   eNext = &bloc->event[i];

   if ( i == 0 )
   {
       if ( bloc->prev == 0 )
       {
           tPrev = bloc->time[i];
           ePrev = &bloc->event[i];
       }
       else
       {
           bloc = bloc->prev;
           tPrev = bloc->time[bloc->total-1];
           ePrev = &bloc->event[bloc->total-1];
       }
   }
   else
   {
       tPrev = bloc->time[i-1];
       ePrev = &bloc->event[i-1];
   }

   progress = Norm((time-tPrev)/(tNext-tPrev));  // 0..1

   event.position = ePrev->position+(eNext->position-ePrev->position)*progress;
   event.angle    = ePrev->angle+(eNext->angle-ePrev->angle)*progress;
   return TRUE;
}


// En-tête des fichiers.

typedef struct
{
   int             majRev;
   int             minRev;
   int             size;
   int             total;
   int             model;
   int             subModel;
   D3DCOLORVALUE   color;
   int             type;
   int             mission;
   int             check[10];
   int             level;
   char            gamer[20];
   float           chrono;
   int             reserve[100];
}
RecorderHeader;


// Ecrit tous les événements sur disque.

BOOL CRecorder::Write(char *filename)
{
   FILE*           file;
   RecorderHeader  header;
   RecorderBloc    *bloc;
   int             total, i;

   file = fopen(filename, "wb");
   if ( file == 0 )  return FALSE;

   bloc = m_bloc;
   total = 0;
   do
   {
       total ++;  // compte le nombre total de blocs
       bloc = bloc->next;
   }
   while ( bloc != 0 );

   ZeroMemory(&header, sizeof(RecorderHeader));
   header.majRev = 0;
   header.minRev = 0;
   header.size = sizeof(RecorderBloc);
   header.total = total;
   header.model = m_model;
   header.subModel = m_subModel;
   header.color = m_color;
   header.type = m_type;
   header.mission = m_mission;
   for ( i=0 ; i<10 ; i++ )
   {
       header.check[i] = m_check[i];
   }
   header.level = m_level;
   strcpy(header.gamer, m_gamer);
   header.chrono = m_chrono;
   fwrite(&header, sizeof(RecorderHeader), 1, file);  // écrit l'en-tête

   bloc = m_bloc;
   do
   {
       fwrite(bloc, sizeof(RecorderBloc), 1, file);
       bloc = bloc->next;
   }
   while ( bloc != 0 );

   fclose(file);
   return TRUE;
}

// Lit tous les événements sur disque.

BOOL CRecorder::Read(char *filename)
{
   FILE*           file;
   RecorderHeader  header;
   RecorderBloc    *bloc, *prev;
   int             i, nb, total;

   file = fopen(filename, "rb");
   if ( file == 0 )  return FALSE;

   Flush();

   nb = fread(&header, sizeof(RecorderHeader), 1, file);  // lit l'en-tête
   if ( nb == 0 )
   {
       fclose(file);
       return FALSE;
   }

   if ( header.size != sizeof(RecorderBloc) )
   {
       fclose(file);
       return FALSE;
   }
   m_model = header.model;
   m_subModel = header.subModel;
   m_color = header.color;
   m_type = header.type;
   m_mission = header.mission;
   for ( i=0 ; i<10 ; i++ )
   {
       m_check[i] = header.check[i];
   }
   m_level = header.level;
   strcpy(m_gamer, header.gamer);
   m_chrono = header.chrono;

   total = header.total;
   prev = 0;
   for ( i=0 ; i<total ; i++ )
   {
       bloc = (RecorderBloc*)malloc(sizeof(RecorderBloc));
       nb = fread(bloc, sizeof(RecorderBloc), 1, file);
       if ( nb == 0 )
       {
           fclose(file);
           Flush();
           return FALSE;
       }
       bloc->prev = 0;  // data lu forcément faux (pointeurs)
       bloc->next = 0;

       if ( i == 0 )  // premier bloc ?
       {
           m_bloc = bloc;
       }

       if ( prev != 0 )
       {
           prev->next = bloc;
           bloc->prev = prev;
       }
       prev = bloc;
   }

   fclose(file);
   return TRUE;
}

// Lit juste l'en-tête sur disque.

BOOL CRecorder::ReadHeader(char *filename)
{
   FILE*           file;
   RecorderHeader  header;
   int             i, nb;

   file = fopen(filename, "rb");
   if ( file == 0 )  return FALSE;

   Flush();

   nb = fread(&header, sizeof(RecorderHeader), 1, file);  // lit l'en-tête
   if ( nb == 0 )
   {
       fclose(file);
       return FALSE;
   }

   if ( header.size != sizeof(RecorderBloc) )
   {
       fclose(file);
       return FALSE;
   }
   m_model = header.model;
   m_subModel = header.subModel;
   m_color = header.color;
   m_type = header.type;
   m_mission = header.mission;
   for ( i=0 ; i<10 ; i++ )
   {
       m_check[i] = header.check[i];
   }
   m_level = header.level;
   strcpy(m_gamer, header.gamer);
   m_chrono = header.chrono;

   fclose(file);
   return TRUE;
}

