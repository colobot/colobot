// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// * Copyright (C) 2012, Polish Portal of Colobot (PPC)
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see  http://www.gnu.org/licenses/.

// engine.cpp

#include "graphics/common/engine.h"

#include <GL/gl.h>
#include <GL/glu.h>


// TODO implementation

Gfx::CEngine::CEngine(CInstanceManager *iMan, CApplication *app)
{
	// TODO
}

Gfx::CEngine::~CEngine()
{
	// TODO
}

int Gfx::CEngine::Render()
{
	/* Just a hello world for now */

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glShadeModel(GL_SMOOTH);
	glDisable(GL_DEPTH_TEST);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);


	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-10.0f, 10.0f, -10.0f, 10.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//glTranslatef(0.0f, 0.0f, -6.0f);

	glBegin(GL_TRIANGLES);
	{
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex2f(-2.0f, -1.0f);
		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex2f(2.0f, -1.0f);
		glColor3f(0.0f, 0.0f, 1.0f);
		glVertex2f(0.0f, 1.5f);
	}
	glEnd();

	glFlush();

	return 1;
}
