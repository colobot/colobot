// progress.h

#ifndef _PROGRESS_H_
#define    _PROGRESS_H_


#include "control.h"


class CD3DEngine;



class CProgress : public CControl
{
public:
   CProgress(CInstanceManager* iMan);
   ~CProgress();

   BOOL    Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);

   BOOL    EventProcess(const Event &event);

   void    Draw();

   void    SetTotal(int total);
   int     RetTotal();

   void    SetProgress(int progress);
   int     RetProgress();

protected:

protected:
   int     m_total;
   int     m_progress;
};


#endif //_PROGRESS_H_
