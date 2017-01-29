// pesetas.h

#ifndef _PESETAS_H_
#define    _PESETAS_H_


#include "control.h"


class CD3DEngine;



class CPesetas : public CControl
{
public:
   CPesetas(CInstanceManager* iMan);
   ~CPesetas();

   BOOL    Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);

   BOOL    EventProcess(const Event &event);

   void    Draw();

   void    SetLevelCredit(float level);
   float   RetLevelCredit();
   void    SetLevelMission(float level);
   float   RetLevelMission();
   void    SetLevelNext(float level);
   float   RetLevelNext();

protected:
   void    DrawCell(FPOINT pos, FPOINT dim, int icon, char *text, int justif);
   void    DrawHLine(FPOINT pos, FPOINT dim, float x1, float y1, float x2);
   void    DrawVLine(FPOINT pos, FPOINT dim, float x1, float y1, float y2);
   void    DrawDot(FPOINT pos, FPOINT dim, float x, float y);

protected:
   float   m_levelCredit;
   float   m_levelMission;
   float   m_levelNext;
};


#endif //_PESETAS_H_
