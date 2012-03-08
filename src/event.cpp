// event.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>

#include "struct.h"
#include "iman.h"
#include "event.h"




// Constructeur de l'objet.

CEvent::CEvent(CInstanceManager* iMan)
{
	m_iMan = iMan;
	m_iMan->AddInstance(CLASS_EVENT, this);

	Flush();
}

// Destructeur de l'objet.

CEvent::~CEvent()
{
}


// Vide le fifo des événements.

void CEvent::Flush()
{
	m_head = 0;
	m_tail = 0;
	m_total = 0;
}

// Fabrique un événement.

void CEvent::MakeEvent(Event &event, EventMsg msg)
{
	ZeroMemory(&event, sizeof(Event));
	event.event = msg;
}

// Ajoute un événement dans le fifo.

BOOL CEvent::AddEvent(const Event &event)
{
	if ( m_total >= MAXEVENT )  return FALSE;

	m_fifo[m_head++] = event;
	if ( m_head >= MAXEVENT )  m_head = 0;
	m_total ++;

	return TRUE;
}

// Enlève un événement du fifo.

BOOL CEvent::GetEvent(Event &event)
{
	if ( m_head == m_tail )  return FALSE;

	event = m_fifo[m_tail++];
	if ( m_tail >= MAXEVENT )  m_tail = 0;
	m_total --;

	return TRUE;
}

