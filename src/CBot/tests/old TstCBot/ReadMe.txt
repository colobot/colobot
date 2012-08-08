========================================================================
       MICROSOFT FOUNDATION CLASS LIBRARY : TstCBot
========================================================================


AppWizard has created this TstCBot application for you.  This application
not only demonstrates the basics of using the Microsoft Foundation classes
but is also a starting point for writing your application.

This file contains a summary of what you will find in each of the files that
make up your TstCBot application.

TstCBot.h
    This is the main header file for the application.  It includes other
    project specific headers (including Resource.h) and declares the
    CTstCBotApp application class.

TstCBot.cpp
    This is the main application source file that contains the application
    class CTstCBotApp.

TstCBot.rc
    This is a listing of all of the Microsoft Windows resources that the
    program uses.  It includes the icons, bitmaps, and cursors that are stored
    in the RES subdirectory.  This file can be directly edited in Microsoft
	Developer Studio.

res\TstCBot.ico
    This is an icon file, which is used as the application's icon.  This
    icon is included by the main resource file TstCBot.rc.

res\TstCBot.rc2
    This file contains resources that are not edited by Microsoft 
	Developer Studio.  You should place all resources not
	editable by the resource editor in this file.

TstCBot.clw
    This file contains information used by ClassWizard to edit existing
    classes or add new classes.  ClassWizard also uses this file to store
    information needed to create and edit message maps and dialog data
    maps and to create prototype member functions.

/////////////////////////////////////////////////////////////////////////////

For the main frame window:

MainFrm.h, MainFrm.cpp
    These files contain the frame class CMainFrame, which is derived from
    CFrameWnd and controls all SDI frame features.


/////////////////////////////////////////////////////////////////////////////

AppWizard creates one document type and one view:

TstCBotDoc.h, TstCBotDoc.cpp - the document
    These files contain your CTstCBotDoc class.  Edit these files to
    add your special document data and to implement file saving and loading
    (via CTstCBotDoc::Serialize).

TstCBotView.h, TstCBotView.cpp - the view of the document
    These files contain your CTstCBotView class.
    CTstCBotView objects are used to view CTstCBotDoc objects.



/////////////////////////////////////////////////////////////////////////////
Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named TstCBot.pch and a precompiled types file named StdAfx.obj.

Resource.h
    This is the standard header file, which defines new resource IDs.
    Microsoft Developer Studio reads and updates this file.

/////////////////////////////////////////////////////////////////////////////
Other notes:

AppWizard uses "TODO:" to indicate parts of the source code you
should add to or customize.

If your application uses MFC in a shared DLL, and your application is 
in a language other than the operating system's current language, you
will need to copy the corresponding localized resources MFC40XXX.DLL
from the Microsoft Visual C++ CD-ROM onto the system or system32 directory,
and rename it to be MFCLOC.DLL.  ("XXX" stands for the language abbreviation.
For example, MFC40DEU.DLL contains resources translated to German.)  If you
don't do this, some of the UI elements of your application will remain in the
language of the operating system.

/////////////////////////////////////////////////////////////////////////////
