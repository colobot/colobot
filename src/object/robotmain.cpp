// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
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


#include "object/robotmain.h"

#include "CBot/CBotDll.h"

#include "app/app.h"

#include "common/event.h"
#include "common/global.h"
#include "common/iman.h"
#include "common/logger.h"
#include "common/misc.h"
#include "common/profile.h"
#include "common/restext.h"

#include "graphics/engine/camera.h"
#include "graphics/engine/cloud.h"
#include "graphics/engine/engine.h"
#include "graphics/engine/lightman.h"
#include "graphics/engine/lightning.h"
#include "graphics/engine/modelmanager.h"
#include "graphics/engine/particle.h"
#include "graphics/engine/planet.h"
#include "graphics/engine/pyro.h"
#include "graphics/engine/terrain.h"
#include "graphics/engine/text.h"
#include "graphics/engine/water.h"

#include "math/const.h"
#include "math/geometry.h"

#include "object/auto/auto.h"
#include "object/auto/autobase.h"
#include "object/brain.h"
#include "object/mainmovie.h"
#include "object/motion/motion.h"
#include "object/motion/motionhuman.h"
#include "object/motion/motiontoto.h"
#include "object/object.h"
#include "object/task/task.h"
#include "object/task/taskbuild.h"
#include "object/task/taskmanip.h"

#include "physics/physics.h"

#include "script/cbottoken.h"
#include "script/cmdtoken.h"
#include "script/script.h"

#include "sound/sound.h"

#include "ui/button.h"
#include "ui/displayinfo.h"
#include "ui/displaytext.h"
#include "ui/edit.h"
#include "ui/interface.h"
#include "ui/label.h"
#include "ui/maindialog.h"
#include "ui/mainmap.h"
#include "ui/mainshort.h"
#include "ui/map.h"
#include "ui/shortcut.h"
#include "ui/slider.h"
#include "ui/window.h"


template<> CRobotMain* CSingleton<CRobotMain>::m_instance = nullptr;


// TODO: remove once using std::string
const int MAX_FNAME = 255;



#define CBOT_STACK  true    // saves the stack of programs CBOT
const float UNIT = 4.0f;



// Global variables.

long    g_id;               // unique identifier
int     g_build;            // constructible buildings
int     g_researchDone;         // research done
long    g_researchEnable;       // research available
float   g_unit;             // conversion factor



// Static variables

static CBotClass*   m_pClassFILE;
static CBotProgram* m_pFuncFile;
static int          m_CompteurFileOpen = 0;
static std::string  m_filesDir;



// Prepares a file name.

void PrepareFilename(CBotString &filename)
{
    int pos = filename.ReverseFind('/');
    if ( pos > 0 )
    {
        filename = filename.Mid(pos+1);  // Remove files with
    }

    pos = filename.ReverseFind('/');
    if ( pos > 0 )
    {
        filename = filename.Mid(pos+1);  // also with /
    }

    pos = filename.ReverseFind(':');
    if ( pos > 0 )
    {
        filename = filename.Mid(pos+1);  // also removes the drive letter C:
    }

    filename = CBotString(m_filesDir.c_str()) + CBotString("/") + filename;
}


// constructor of the class
// get the filename as a parameter

// execution
bool rfconstruct (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception)
{
    CBotString  mode;

    // accepts no parameters
    if ( pVar == NULL ) return true;

    // must be a character string
    if ( pVar->GetType() != CBotTypString ) { Exception = CBotErrBadString; return false; }

    CBotString  filename = pVar->GetValString();
    PrepareFilename(filename);

    // there may be a second parameter
    pVar = pVar->GetNext();
    if ( pVar != NULL )
    {
        // recover mode
        mode = pVar->GetValString();
        if ( mode != "r" && mode != "w" ) { Exception = CBotErrBadParam; return false; }

        // no third parameter
        if ( pVar->GetNext() != NULL ) { Exception = CBotErrOverParam; return false; }
    }

    // saves the file name
    pVar = pThis->GetItem("filename");
    pVar->SetValString(filename);

    if ( ! mode.IsEmpty() )
    {
        // opens the requested file
        FILE*   pFile = fopen( filename, mode );
        if ( pFile == NULL ) { Exception = CBotErrFileOpen; return false; }

        m_CompteurFileOpen ++;

        // save the channel file
        pVar = pThis->GetItem("handle");
        pVar->SetValInt(reinterpret_cast<long>(pFile));
    }

    return true;
}

// compilation
CBotTypResult cfconstruct (CBotVar* pThis, CBotVar* &pVar)
{
    // accepts no parameters
    if ( pVar == NULL ) return CBotTypResult( 0 );

    // must be a character string
    if ( pVar->GetType() != CBotTypString )
                        return CBotTypResult( CBotErrBadString );

    // there may be a second parameter
    pVar = pVar->GetNext();
    if ( pVar != NULL )
    {
        // which must be a string
        if ( pVar->GetType() != CBotTypString )
                            return CBotTypResult( CBotErrBadString );
        // no third parameter
        if ( pVar->GetNext() != NULL ) return CBotTypResult( CBotErrOverParam );
    }

    // the result is void (constructor)
    return CBotTypResult( 0 );
}


// destructor of the class

// execution
bool rfdestruct (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception)
{
    // retrieve the item "handle"
    pVar = pThis->GetItem("handle");

    // don't open? no problem :)
    if ( pVar->GetInit() != IS_DEF) return true;

    FILE* pFile= reinterpret_cast<FILE*>(pVar->GetValInt());
    fclose(pFile);
    m_CompteurFileOpen --;

    pVar->SetInit(IS_NAN);

    return true;
}


// process FILE :: open
// get the r/w mode as a parameter

// execution
bool rfopen (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception)
{
    // there must be a parameter
    if ( pVar == NULL ) { Exception = CBotErrLowParam; return false; }

    // which must be a character string
    if ( pVar->GetType() != CBotTypString ) { Exception = CBotErrBadString; return false; }

    // There may be a second parameter
    if ( pVar->GetNext() != NULL )
    {
        // if the first parameter is the file name
        CBotString  filename = pVar->GetValString();
        PrepareFilename(filename);

        // saves the file name
        CBotVar* pVar2 = pThis->GetItem("filename");
        pVar2->SetValString(filename);

        // next parameter is the mode
        pVar = pVar -> GetNext();
    }

    CBotString  mode = pVar->GetValString();
    if ( mode != "r" && mode != "w" ) { Exception = CBotErrBadParam; return false; }

    // no third parameter
    if ( pVar->GetNext() != NULL ) { Exception = CBotErrOverParam; return false; }

    // retrieve the item "handle"
    pVar = pThis->GetItem("handle");

    // which must not be initialized
    if ( pVar->GetInit() == IS_DEF) { Exception = CBotErrFileOpen; return false; }

    // file contains the name
    pVar = pThis->GetItem("filename");
    CBotString  filename = pVar->GetValString();

    PrepareFilename(filename);  // if the name was h.filename attribute = "...";

    // opens the requested file
    FILE*   pFile = fopen( filename, mode );
    if ( pFile == NULL )
    {
        pResult->SetValInt(false);
        return true;
    }

    m_CompteurFileOpen ++;

    // Registered the channel file
    pVar = pThis->GetItem("handle");
    pVar->SetValInt(reinterpret_cast<long>(pFile));

    pResult->SetValInt(true);
    return true;
}

// compilation
CBotTypResult cfopen (CBotVar* pThis, CBotVar* &pVar)
{
    // there must be a parameter
    if ( pVar == NULL ) return CBotTypResult( CBotErrLowParam );

    // which must be a string
    if ( pVar->GetType() != CBotTypString )
                        return CBotTypResult( CBotErrBadString );

    // there may be a second parameter
    pVar = pVar->GetNext();
    if ( pVar != NULL )
    {
        // which must be a string
        if ( pVar->GetType() != CBotTypString )
                        return CBotTypResult( CBotErrBadString );

        // no third parameter
        if ( pVar->GetNext() != NULL ) return CBotTypResult( CBotErrOverParam );
    }

    // the result is bool
    return CBotTypResult(CBotTypBoolean);
}


// process FILE :: close

// execeution
bool rfclose (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception)
{
    // it shouldn't be any parameters
    if ( pVar != NULL ) return CBotErrOverParam;

    // retrieve the item "handle"
    pVar = pThis->GetItem("handle");

    if ( pVar->GetInit() != IS_DEF) { Exception = CBotErrNotOpen; return false; }

    FILE* pFile= reinterpret_cast<FILE*>(pVar->GetValInt());
    fclose(pFile);
    m_CompteurFileOpen --;

    pVar->SetInit(IS_NAN);

    return true;
}

// compilation
CBotTypResult cfclose (CBotVar* pThis, CBotVar* &pVar)
{
    // it shouldn't be any parameters
    if ( pVar != NULL ) return CBotTypResult( CBotErrOverParam );

    // function returns a result "void"
    return CBotTypResult( 0 );
}

// process FILE :: writeln

// execution
bool rfwrite (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception)
{
    // there must be a parameter
    if ( pVar == NULL ) { Exception = CBotErrLowParam; return false; }

    // which must be a character string
    if ( pVar->GetType() != CBotTypString ) { Exception = CBotErrBadString; return false; }

    CBotString param = pVar->GetValString();

    // retrieve the item "handle"
    pVar = pThis->GetItem("handle");

    if ( pVar->GetInit() != IS_DEF) { Exception = CBotErrNotOpen; return false; }

    FILE* pFile= reinterpret_cast<FILE*>(pVar->GetValInt());

    int res = fputs(param+CBotString("\n"), pFile);

    // if an error occurs generate an exception
    if ( res < 0 ) { Exception = CBotErrWrite; return false; }

    return true;
}

// compilation
CBotTypResult cfwrite (CBotVar* pThis, CBotVar* &pVar)
{
    // there must be a parameter
    if ( pVar == NULL ) return CBotTypResult( CBotErrLowParam );

    // which must be a character string
    if ( pVar->GetType() != CBotTypString ) return CBotTypResult( CBotErrBadString );

    // no other parameter
    if ( pVar->GetNext() != NULL ) return CBotTypResult( CBotErrOverParam );

    // the function returns a void result
    return CBotTypResult( 0 );
}

// process FILE :: readln

// execution
bool rfread (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception)
{
    // it shouldn't be any parameters
    if ( pVar != NULL ) { Exception = CBotErrOverParam; return false; }

    // retrieve the item "handle"
    pVar = pThis->GetItem("handle");

    if ( pVar->GetInit() != IS_DEF) { Exception = CBotErrNotOpen; return false; }

    FILE* pFile= reinterpret_cast<FILE*>(pVar->GetValInt());

    char    chaine[2000];
    int     i;
    for ( i = 0 ; i < 2000 ; i++ ) chaine[i] = 0;

    fgets(chaine, 1999, pFile);

    for ( i = 0 ; i < 2000 ; i++ ) if (chaine[i] == '\n') chaine[i] = 0;

    // if an error occurs generate an exception
    if ( ferror(pFile) ) { Exception = CBotErrRead; return false; }

    pResult->SetValString( chaine );

    return true;
}

// compilation
CBotTypResult cfread (CBotVar* pThis, CBotVar* &pVar)
{
    // it should not be any parameter
    if ( pVar != NULL ) return CBotTypResult( CBotErrOverParam );

    // function returns a result "string"
    return CBotTypResult( CBotTypString );
}
// process FILE :: readln


// execution
bool rfeof (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception)
{
    // it should not be any parameter
    if ( pVar != NULL ) { Exception = CBotErrOverParam; return false; }

    // retrieve the item "handle"
    pVar = pThis->GetItem("handle");

    if ( pVar->GetInit() != IS_DEF) { Exception = CBotErrNotOpen; return false; }

    FILE* pFile= reinterpret_cast<FILE*>(pVar->GetValInt());

    pResult->SetValInt( feof( pFile ) );

    return true;
}

// compilation
CBotTypResult cfeof (CBotVar* pThis, CBotVar* &pVar)
{
    // it shouldn't be any parameter
    if ( pVar != NULL ) return CBotTypResult( CBotErrOverParam );

    // the function returns a boolean result
    return CBotTypResult( CBotTypBoolean );
}





void InitClassFILE()
{
// create a class for file management
// the use is as follows:
// file canal( "NomFichier.txt" )
// canal.open( "r" );   // open for read
// s = canal.readln( ); // reads a line
// canal.close();   // close the file

    // create the class FILE
    m_pClassFILE    = new CBotClass("file", NULL);
    // adds the component ".filename"
    m_pClassFILE->AddItem("filename", CBotTypString);
    // adds the component ".handle"
    m_pClassFILE->AddItem("handle", CBotTypInt, PR_PRIVATE);

    // define a constructor and a destructor
    m_pClassFILE->AddFunction("file", rfconstruct, cfconstruct );
    m_pClassFILE->AddFunction("~file", rfdestruct, NULL );

    // end of the methods associated
    m_pClassFILE->AddFunction("open", rfopen, cfopen );
    m_pClassFILE->AddFunction("close", rfclose, cfclose );
    m_pClassFILE->AddFunction("writeln", rfwrite, cfwrite );
    m_pClassFILE->AddFunction("readln", rfread, cfread );
    m_pClassFILE->AddFunction("eof", rfeof, cfeof );

    m_pFuncFile = new CBotProgram( );
    CBotStringArray ListFonctions;
    m_pFuncFile->Compile( "public file openfile(string name, string mode) {return new file(name, mode);}", ListFonctions);
    m_pFuncFile->SetIdent(-2);  // restoreState in special identifier for this function
}





// Compilation of class "point".

CBotTypResult cPoint(CBotVar* pThis, CBotVar* &var)
{
    if ( !pThis->IsElemOfClass("point") )  return CBotTypResult(CBotErrBadNum);

    if ( var == NULL )  return CBotTypResult(0);  // ok if no parameter

    // First parameter (x):
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    // Second parameter (y):
    if ( var == NULL )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    // Third parameter (z):
    if ( var == NULL )  // only 2 parameters?
    {
        return CBotTypResult(0);  // this function returns void
    }

    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();
    if ( var != NULL )  return CBotTypResult(CBotErrOverParam);

    return CBotTypResult(0);  // this function returns void
}

//Execution of the class "point".

bool rPoint(CBotVar* pThis, CBotVar* var, CBotVar* pResult, int& Exception)
{
    CBotVar     *pX, *pY, *pZ;

    if ( var == NULL )  return true;  // constructor with no parameters is ok

    if ( var->GetType() > CBotTypDouble )
    {
        Exception = CBotErrBadNum;  return false;
    }

    pX = pThis->GetItem("x");
    if ( pX == NULL )
    {
        Exception = CBotErrUndefItem;  return false;
    }
    pX->SetValFloat( var->GetValFloat() );
    var = var->GetNext();

    if ( var == NULL )
    {
        Exception = CBotErrLowParam;  return false;
    }

    if ( var->GetType() > CBotTypDouble )
    {
        Exception = CBotErrBadNum;  return false;
    }

    pY = pThis->GetItem("y");
    if ( pY == NULL )
    {
        Exception = CBotErrUndefItem;  return false;
    }
    pY->SetValFloat( var->GetValFloat() );
    var = var->GetNext();

    if ( var == NULL )
    {
        return true;  // ok with only two parameters
    }

    pZ = pThis->GetItem("z");
    if ( pZ == NULL )
    {
        Exception = CBotErrUndefItem;  return false;
    }
    pZ->SetValFloat( var->GetValFloat() );
    var = var->GetNext();

    if ( var != NULL )
    {
        Exception = CBotErrOverParam;  return false;
    }

    return  true;  // no interruption
}




//! Constructor of robot application
CRobotMain::CRobotMain(CApplication* app)
{
    m_app = app;

    m_eventQueue = m_app->GetEventQueue();
    m_sound      = m_app->GetSound();

    m_engine     = Gfx::CEngine::GetInstancePointer();
    m_lightMan   = m_engine->GetLightManager();
    m_particle   = m_engine->GetParticle();
    m_water      = m_engine->GetWater();
    m_cloud      = m_engine->GetCloud();
    m_lightning  = m_engine->GetLightning();
    m_planet     = m_engine->GetPlanet();

    m_interface   = new Ui::CInterface();
    m_terrain     = new Gfx::CTerrain();
    m_camera      = new Gfx::CCamera();
    m_displayText = new Ui::CDisplayText();
    m_movie       = new CMainMovie();
    m_dialog      = new Ui::CMainDialog();
    m_short       = new Ui::CMainShort();
    m_map         = new Ui::CMainMap();
    m_displayInfo = nullptr;

    m_engine->SetTerrain(m_terrain);
    m_filesDir = m_dialog->GetFilesDir();

    m_time = 0.0f;
    m_gameTime = 0.0f;
    m_checkEndTime = 0.0f;

    m_phase       = PHASE_NAME;
    m_cameraRank  = -1;
    m_visitLast   = EVENT_NULL;
    m_visitObject = 0;
    m_visitArrow  = 0;
    m_audioTrack  = 0;
    m_audioRepeat = true;
    m_delayWriteMessage = 0;
    m_selectObject = 0;
    m_infoUsed     = 0;

    m_beginObject         = false;
    m_terrainGenerate     = false;
    m_terrainInit         = false;
    m_terrainInitTextures = false;
    m_terrainCreate       = false;

    m_version      = 1;
    m_retroStyle   = false;
    m_immediatSatCom = false;
    m_beginSatCom  = false;
    m_lockedSatCom = false;
    m_movieLock    = false;
    m_satComLock   = false;
    m_editLock     = false;
    m_editFull     = false;
    m_pause        = false;
    m_hilite       = false;
    m_freePhoto    = false;
    m_showPos      = false;
    m_selectInsect = false;
    m_showSoluce   = false;
    #ifdef NDEBUG
    m_showAll      = false;
    #else
    m_showAll      = true; // for development
    #endif
    m_cheatRadar   = false;
    m_fixScene     = false;
    m_trainerPilot = false;
    m_suspend      = false;
    m_friendAim    = false;
    m_resetCreate  = false;
    m_shortCut     = true;

    m_engine->SetMovieLock(m_movieLock);

    m_movie->Flush();
    m_movieInfoIndex = -1;

    m_tooltipPos = Math::Point(0.0f, 0.0f);
    m_tooltipName[0] = 0;
    m_tooltipTime = 0.0f;

    m_endingWinRank   = 0;
    m_endingLostRank  = 0;
    m_winTerminate   = false;

    m_joystickDeadzone = 0.2f;
    SetDefaultInputBindings();

    FlushDisplayInfo();

    m_fontSize  = 9.0f;
    m_windowPos = Math::Point(0.15f, 0.17f);
    m_windowDim = Math::Point(0.70f, 0.66f);

    // TODO: profile
    // float fValue;
    // int iValue;

    // if (GetLocalProfileFloat("Edit", "FontSize",    fValue)) m_fontSize    = fValue;
    // if (GetLocalProfileFloat("Edit", "WindowPos.x", fValue)) m_windowPos.x = fValue;
    // if (GetLocalProfileFloat("Edit", "WindowPos.y", fValue)) m_windowPos.y = fValue;
    // if (GetLocalProfileFloat("Edit", "WindowDim.x", fValue)) m_windowDim.x = fValue;
    // if (GetLocalProfileFloat("Edit", "WindowDim.y", fValue)) m_windowDim.y = fValue;

    m_IOPublic = false;
    m_IODim = Math::Point(320.0f/640.0f, (121.0f+18.0f*8)/480.0f);
    m_IOPos.x = (1.0f-m_IODim.x)/2.0f;  // in the middle
    m_IOPos.y = (1.0f-m_IODim.y)/2.0f;

    /* TODO: profile
    if (GetLocalProfileInt  ("Edit", "IOPublic", iValue)) m_IOPublic = iValue;
    if (GetLocalProfileFloat("Edit", "IOPos.x",  fValue)) m_IOPos.x  = fValue;
    if (GetLocalProfileFloat("Edit", "IOPos.y",  fValue)) m_IOPos.y  = fValue;
    if (GetLocalProfileFloat("Edit", "IODim.x",  fValue)) m_IODim.x  = fValue;
    if (GetLocalProfileFloat("Edit", "IODim.y",  fValue)) m_IODim.y  = fValue; */

    m_short->FlushShortcuts();
    InitEye();

    m_engine->SetTracePrecision(1.0f);

    m_cameraPan  = 0.0f;
    m_cameraZoom = 0.0f;

    g_id = 0;
    g_build = 0;
    g_researchDone = 0;  // no research done
    g_researchEnable = 0;
    g_unit = UNIT;

    m_gamerName[0] = 0;
    /* TODO: profile
    GetLocalProfileString("Gamer", "LastName", m_gamerName, 100); */
    SetGlobalGamerName(m_gamerName);
    ReadFreeParam();
    m_dialog->SetupRecall();

    for (int i = 0; i < MAXSHOWLIMIT; i++)
    {
        m_showLimit[i].used = false;
        m_showLimit[i].total = 0;
        m_showLimit[i].link = 0;
    }

    CBotProgram::SetTimer(100);
    CBotProgram::Init();

    for (int i = 0; i < OBJECT_MAX; i++)
    {
        ObjectType type = static_cast<ObjectType>(i);
        const char* token = GetObjectName(type);
        if (token[0] != 0)
            CBotProgram::DefineNum(token, type);

        token = GetObjectAlias(type);
        if (token[0] != 0)
            CBotProgram::DefineNum(token, type);
    }

    CBotProgram::DefineNum("White",      0);
    CBotProgram::DefineNum("Black",      1);
    CBotProgram::DefineNum("Gray",       2);
    CBotProgram::DefineNum("LightGray",  3);
    CBotProgram::DefineNum("Red",        4);
    CBotProgram::DefineNum("Pink",       5);
    CBotProgram::DefineNum("Purple",     6);
    CBotProgram::DefineNum("Orange",     7);
    CBotProgram::DefineNum("Yellow",     8);
    CBotProgram::DefineNum("Beige",      9);
    CBotProgram::DefineNum("Brown",      10);
    CBotProgram::DefineNum("Skin",       11);
    CBotProgram::DefineNum("Green",      12);
    CBotProgram::DefineNum("LightGreen", 13);
    CBotProgram::DefineNum("Blue",       14);
    CBotProgram::DefineNum("LightBlue",  15);
    CBotProgram::DefineNum("BlackArrow", 16);
    CBotProgram::DefineNum("RedArrow",   17);

    CBotProgram::DefineNum("Metal",   OM_METAL);
    CBotProgram::DefineNum("Plastic", OM_PLASTIC);

    CBotProgram::DefineNum("InFront",    TMA_FFRONT);
    CBotProgram::DefineNum("Behind",     TMA_FBACK);
    CBotProgram::DefineNum("EnergyCell", TMA_POWER);

    CBotProgram::DefineNum("DisplayError",   Ui::TT_ERROR);
    CBotProgram::DefineNum("DisplayWarning", Ui::TT_WARNING);
    CBotProgram::DefineNum("DisplayInfo",    Ui::TT_INFO);
    CBotProgram::DefineNum("DisplayMessage", Ui::TT_MESSAGE);

    CBotProgram::DefineNum("FilterNone",        FILTER_NONE);
    CBotProgram::DefineNum("FilterOnlyLanding", FILTER_ONLYLANDING);
    CBotProgram::DefineNum("FilterOnlyFliying", FILTER_ONLYFLYING);

    CBotProgram::DefineNum("ExploNone", 0);
    CBotProgram::DefineNum("ExploBoum", EXPLO_BOUM);
    CBotProgram::DefineNum("ExploBurn", EXPLO_BURN);
    CBotProgram::DefineNum("ExploWater", EXPLO_WATER);

    CBotProgram::DefineNum("PolskiPortalColobota", 1337);

    CBotClass* bc;

    // Add the class Point.
    bc = new CBotClass("point", NULL, true);  // intrinsic class
    bc->AddItem("x", CBotTypFloat);
    bc->AddItem("y", CBotTypFloat);
    bc->AddItem("z", CBotTypFloat);
    bc->AddFunction("point", rPoint, cPoint);

    // Adds the class Object.
    bc = new CBotClass("object", NULL);
    bc->AddItem("category",    CBotTypResult(CBotTypInt), PR_READ);
    bc->AddItem("position",    CBotTypResult(CBotTypClass, "point"), PR_READ);
    bc->AddItem("orientation", CBotTypResult(CBotTypFloat), PR_READ);
    bc->AddItem("pitch",       CBotTypResult(CBotTypFloat), PR_READ);
    bc->AddItem("roll",        CBotTypResult(CBotTypFloat), PR_READ);
    bc->AddItem("energyLevel", CBotTypResult(CBotTypFloat), PR_READ);
    bc->AddItem("shieldLevel", CBotTypResult(CBotTypFloat), PR_READ);
    bc->AddItem("temperature", CBotTypResult(CBotTypFloat), PR_READ);
    bc->AddItem("altitude",    CBotTypResult(CBotTypFloat), PR_READ);
    bc->AddItem("lifeTime",    CBotTypResult(CBotTypFloat), PR_READ);
    bc->AddItem("material",    CBotTypResult(CBotTypInt), PR_READ);
    bc->AddItem("energyCell",  CBotTypResult(CBotTypPointer, "object"), PR_READ);
    bc->AddItem("load",        CBotTypResult(CBotTypPointer, "object"), PR_READ);
    bc->AddItem("id",          CBotTypResult(CBotTypInt), PR_READ);

    // Initializes the class FILE.
    InitClassFILE();

    CScript::InitFonctions();
}

//! Destructor of robot application
CRobotMain::~CRobotMain()
{
    delete m_displayText;
    m_displayText = nullptr;

    delete m_interface;
    m_interface = nullptr;

    delete m_terrain;
    m_terrain = nullptr;

    delete m_camera;
    m_camera = nullptr;

    delete m_displayText;
    m_displayText = nullptr;

    delete m_movie;
    m_movie = nullptr;

    delete m_dialog;
    m_dialog = nullptr;

    delete m_short;
    m_short = nullptr;

    delete m_map;
    m_map = nullptr;

    m_app = nullptr;
}

Gfx::CCamera* CRobotMain::GetCamera()
{
    return m_camera;
}

Gfx::CTerrain* CRobotMain::GetTerrain()
{
    return m_terrain;
}

Ui::CInterface* CRobotMain::GetInterface()
{
    return m_interface;
}

Ui::CDisplayText* CRobotMain::GetDisplayText()
{
    return m_displayText;
}


//! Creates the file colobot.ini at the first time
void CRobotMain::CreateIni()
{
    /* TODO: profile
    int iValue;
    // colobot.ini doesn't exist?
    if (!GetLocalProfileInt("Setup", "TotoMode", iValue))
        m_dialog->SetupMemorize();*/
}

void CRobotMain::SetDefaultInputBindings()
{
    for (int i = 0; i < INPUT_SLOT_MAX; i++)
    {
        m_inputBindings[i].primary = m_inputBindings[i].secondary = KEY_INVALID;
    }

    for (int i = 0; i < JOY_AXIS_SLOT_MAX; i++)
    {
        m_joyAxisBindings[i].axis = AXIS_INVALID;
        m_joyAxisBindings[i].invert = false;
    }

    m_inputBindings[INPUT_SLOT_LEFT   ].primary   = KEY(LEFT);
    m_inputBindings[INPUT_SLOT_RIGHT  ].primary   = KEY(RIGHT);
    m_inputBindings[INPUT_SLOT_UP     ].primary   = KEY(UP);
    m_inputBindings[INPUT_SLOT_DOWN   ].primary   = KEY(DOWN);
    m_inputBindings[INPUT_SLOT_GUP    ].primary   = VIRTUAL_KMOD(SHIFT);
    m_inputBindings[INPUT_SLOT_GDOWN  ].primary   = VIRTUAL_KMOD(CTRL);
    m_inputBindings[INPUT_SLOT_CAMERA ].primary   = KEY(SPACE);
    m_inputBindings[INPUT_SLOT_CAMERA ].secondary = VIRTUAL_JOY(2);
    m_inputBindings[INPUT_SLOT_DESEL  ].primary   = KEY(KP0);
    m_inputBindings[INPUT_SLOT_DESEL  ].secondary = VIRTUAL_JOY(6);
    m_inputBindings[INPUT_SLOT_ACTION ].primary   = KEY(RETURN);
    m_inputBindings[INPUT_SLOT_ACTION ].secondary = VIRTUAL_JOY(1);
    m_inputBindings[INPUT_SLOT_NEAR   ].primary   = KEY(KP_PLUS);
    m_inputBindings[INPUT_SLOT_NEAR   ].secondary = VIRTUAL_JOY(5);
    m_inputBindings[INPUT_SLOT_AWAY   ].primary   = KEY(KP_MINUS);
    m_inputBindings[INPUT_SLOT_AWAY   ].secondary = VIRTUAL_JOY(4);
    m_inputBindings[INPUT_SLOT_NEXT   ].primary   = KEY(TAB);
    m_inputBindings[INPUT_SLOT_NEXT   ].secondary = VIRTUAL_JOY(3);
    m_inputBindings[INPUT_SLOT_HUMAN  ].primary   = KEY(HOME);
    m_inputBindings[INPUT_SLOT_HUMAN  ].secondary = VIRTUAL_JOY(7);
    m_inputBindings[INPUT_SLOT_QUIT   ].primary   = KEY(ESCAPE);
    m_inputBindings[INPUT_SLOT_HELP   ].primary   = KEY(F1);
    m_inputBindings[INPUT_SLOT_PROG   ].primary   = KEY(F2);
    m_inputBindings[INPUT_SLOT_CBOT   ].primary   = KEY(F3);
    m_inputBindings[INPUT_SLOT_VISIT  ].primary   = KEY(KP_PERIOD);
    m_inputBindings[INPUT_SLOT_SPEED10].primary   = KEY(F4);
    m_inputBindings[INPUT_SLOT_SPEED15].primary   = KEY(F5);
    m_inputBindings[INPUT_SLOT_SPEED20].primary   = KEY(F6);

    m_joyAxisBindings[JOY_AXIS_SLOT_X].axis = 0;
    m_joyAxisBindings[JOY_AXIS_SLOT_Y].axis = 1;
    m_joyAxisBindings[JOY_AXIS_SLOT_Z].axis = 2;
}

void CRobotMain::SetInputBinding(InputSlot slot, InputBinding binding)
{
    unsigned int index = static_cast<unsigned int>(slot);
    assert(index >= 0 && index < INPUT_SLOT_MAX);
    m_inputBindings[index] = binding;
}

const InputBinding& CRobotMain::GetInputBinding(InputSlot slot)
{
    unsigned int index = static_cast<unsigned int>(slot);
    assert(index >= 0 && index < INPUT_SLOT_MAX);
    return m_inputBindings[index];
}

void CRobotMain::SetJoyAxisBinding(JoyAxisSlot slot, JoyAxisBinding binding)
{
    unsigned int index = static_cast<unsigned int>(slot);
    assert(index >= 0 && index < JOY_AXIS_SLOT_MAX);
    m_joyAxisBindings[index] = binding;
}

const JoyAxisBinding& CRobotMain::GetJoyAxisBinding(JoyAxisSlot slot)
{
    unsigned int index = static_cast<unsigned int>(slot);
    assert(index >= 0 && index < JOY_AXIS_SLOT_MAX);
    return m_joyAxisBindings[index];
}

void CRobotMain::SetJoystickDeadzone(float zone)
{
    m_joystickDeadzone = zone;
}

float CRobotMain::GetJoystickDeadzone()
{
    return m_joystickDeadzone;
}

void CRobotMain::ResetKeyStates()
{
    m_keyMotion = Math::Vector(0.0f, 0.0f, 0.0f);
    m_joyMotion = Math::Vector(0.0f, 0.0f, 0.0f);
}

//! Changes phase
void CRobotMain::ChangePhase(Phase phase)
{
    if (m_phase == PHASE_SIMUL)  // ends a simulation?
    {
        SaveAllScript();
        m_sound->StopMusic();
        m_camera->SetControllingObject(0);

/* TODO: #if _SCHOOL
        if ( true )
#else*/
        if (m_gameTime > 10.0f)  // did you play at least 10 seconds?
        {
            int rank = m_dialog->GetSceneRank();
            int numTry = m_dialog->GetGamerInfoTry(rank);
            m_dialog->SetGamerInfoTry(rank, numTry+1);
            m_dialog->WriteGamerInfo();
        }
    }

    if (phase == PHASE_WIN)  // wins a simulation?
    {
        int rank = m_dialog->GetSceneRank();
        m_dialog->SetGamerInfoPassed(rank, true);
        m_dialog->NextMission();  // passes to the next mission
        m_dialog->WriteGamerInfo();
    }

    m_app->SetLowCPU(true); // doesn't use much CPU in interface phases

    DeleteAllObjects();  // removes all the current 3D Scene

    m_phase        = phase;
    m_winDelay     = 0.0f;
    m_lostDelay    = 0.0f;
    m_beginSatCom = false;
    m_movieLock   = false;
    m_satComLock  = false;
    m_editLock    = false;
    m_freePhoto   = false;
    m_resetCreate = false;

    m_engine->SetMovieLock(m_movieLock);
    ChangePause(false);
    FlushDisplayInfo();
    m_engine->SetRankView(0);
    m_engine->DeleteAllObjects();
    Gfx::CModelManager::GetInstancePointer()->DeleteAllModelCopies();
    m_engine->SetWaterAddColor(Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f));
    m_engine->SetBackground("");
    m_engine->SetBackForce(false);
    m_engine->SetForegroundName("");
    m_engine->SetOverColor();
    m_engine->DeleteGroundMark(0);
    SetSpeed(1.0f);
    m_terrain->SetWind(Math::Vector(0.0f, 0.0f, 0.0f));
    m_terrain->FlushBuildingLevel();
    m_terrain->FlushFlyingLimit();
    m_lightMan->FlushLights();
    m_particle->FlushParticle();
    m_water->Flush();
    m_cloud->Flush();
    m_lightning->Flush();
    m_planet->Flush();
    m_interface->Flush();
    ClearInterface();
    FlushNewScriptName();
    m_sound->SetListener(Math::Vector(0.0f, 0.0f, 0.0f), Math::Vector(0.0f, 0.0f, 1.0f));
    m_camera->SetType(Gfx::CAM_TYPE_DIALOG);
    m_movie->Flush();
    m_movieInfoIndex = -1;
    m_cameraPan  = 0.0f;
    m_cameraZoom = 0.0f;
    m_shortCut = true;

    CInstanceManager* iMan = CInstanceManager::GetInstancePointer();
    iMan->Flush(CLASS_OBJECT);
    iMan->Flush(CLASS_PHYSICS);
    iMan->Flush(CLASS_BRAIN);
    iMan->Flush(CLASS_PYRO);

    Math::Point dim, pos;

    // Creates and hide the command console.
    dim.x = 200.0f/640.0f;
    dim.y =  18.0f/480.0f;
    pos.x =  50.0f/640.0f;
    pos.y = 452.0f/480.0f;
    Ui::CEdit* pe = static_cast<Ui::CEdit*>(m_interface->CreateEdit(pos, dim, 0, EVENT_CMD));
    if (pe == nullptr) return;
    pe->ClearState(Ui::STATE_VISIBLE);
    m_cmdEdit = false;  // hidden for now

    // Creates the speedometer.
/* TODO: #if _TEEN
    dim.x =  30.0f/640.0f;
    dim.y =  20.0f/480.0f;
    pos.x =   4.0f/640.0f;
    pos.y = 454.0f/480.0f;
#else*/
    dim.x =  30.0f/640.0f;
    dim.y =  20.0f/480.0f;
    pos.x =   4.0f/640.0f;
    pos.y = 426.0f/480.0f;

    Ui::CButton* pb = m_interface->CreateButton(pos, dim, 0, EVENT_SPEED);
    if (pb == nullptr) return;
    pb->SetState(Ui::STATE_SIMPLY);
    pb->ClearState(Ui::STATE_VISIBLE);

    m_dialog->ChangePhase(m_phase);

    dim.x = 32.0f/640.0f;
    dim.y = 32.0f/480.0f;
    float ox = 3.0f/640.0f;
    float oy = 3.0f/480.0f;
    float sx = (32.0f+2.0f)/640.0f;
    float sy = (32.0f+2.0f)/480.0f;

    if (m_phase != PHASE_PERSO)
    {
        m_engine->SetDrawWorld(true);
        m_engine->SetDrawFront(false);
        m_fixScene = false;
    }

    if (m_phase == PHASE_INIT)
    {
        m_engine->DeleteTexture("generic.png");
    }

    if (m_phase == PHASE_SIMUL)
    {
        m_engine->DeleteTexture("interface.png");

        m_app->SetLowCPU(false); // high CPU for simulation

        bool loading = (m_dialog->GetSceneRead()[0] != 0);

        m_map->CreateMap();
        CreateScene(m_dialog->GetSceneSoluce(), false, false);  // interactive scene
        if (m_mapImage)
            m_map->SetFixImage(m_mapFilename);

        /*Math::Point ddim;

        pos.x = 620.0f/640.0f;
        pos.y = 460.0f/480.0f;
        ddim.x = 20.0f/640.0f;
        ddim.y = 20.0f/480.0f;
        m_interface->CreateButton(pos, ddim, 11, EVENT_BUTTON_QUIT);*/

        if (m_immediatSatCom && !loading  &&
            m_infoFilename[SATCOM_HUSTON][0] != 0)
            StartDisplayInfo(SATCOM_HUSTON, false);  // shows the instructions

        m_sound->StopMusic();
        if (!m_base || loading) StartMusic();
    }

    if (m_phase == PHASE_WIN)
    {
        m_sound->StopAll();
        if (m_endingWinRank == -1)
        {
            ChangePhase(PHASE_TERM);
        }
        else
        {
/* TODO: #if _TEEN
            m_winTerminate = (m_endingWinRank == 900);
            m_dialog->SetSceneName("teenw");
#else*/
            m_winTerminate = (m_endingWinRank == 904);
            m_dialog->SetSceneName("win");

            m_dialog->SetSceneRank(m_endingWinRank);
            CreateScene(false, true, false);  // sets scene

            pos.x = ox+sx*1;  pos.y = oy+sy*1;
            Math::Point ddim;
            ddim.x = dim.x*2;  ddim.y = dim.y*2;
            m_interface->CreateButton(pos, ddim, 16, EVENT_BUTTON_OK);

            if (m_winTerminate)
            {
/* TODO: #if _TEEN
                pos.x = ox+sx*3;  pos.y = oy+sy*1;
                ddim.x = dim.x*15;  ddim.y = dim.y*2;
                pe = m_interface->CreateEdit(pos, ddim, 0, EVENT_EDIT0);
                pe->SetFontType(FONT_COLOBOT);
                pe->SetEditCap(false);
                pe->SetHiliteCap(false);
                pe->ReadText("help/teenw.txt");
#else*/

                pos.x = ox+sx*3;  pos.y = oy+sy*0.2f;
                ddim.x = dim.x*15;  ddim.y = dim.y*3.0f;
                pe = m_interface->CreateEdit(pos, ddim, 0, EVENT_EDIT0);
                pe->SetGenericMode(true);
                pe->SetFontType(Gfx::FONT_COLOBOT);
                pe->SetEditCap(false);
                pe->SetHighlightCap(false);
                pe->ReadText(std::string("help/") + m_app->GetLanguageChar() + std::string("/win.txt"));
            }
            else
            {
                m_displayText->DisplayError(INFO_WIN, Math::Vector(0.0f,0.0f,0.0f), 15.0f, 60.0f, 1000.0f);
            }
            StartMusic();
        }
    }

    if (m_phase == PHASE_LOST)
    {
        m_sound->StopAll();
        if (m_endingLostRank == -1)
        {
            ChangePhase(PHASE_TERM);
        }
        else
        {
            m_winTerminate = false;
            m_dialog->SetSceneName("lost");
            m_dialog->SetSceneRank(m_endingLostRank);
            CreateScene(false, true, false);  // sets scene

            pos.x = ox+sx*1;  pos.y = oy+sy*1;
            Math::Point ddim;
            ddim.x = dim.x*2;  ddim.y = dim.y*2;
            m_interface->CreateButton(pos, ddim, 16, EVENT_BUTTON_OK);
            m_displayText->DisplayError(INFO_LOST, Math::Vector(0.0f,0.0f,0.0f), 15.0f, 60.0f, 1000.0f);
            
            StartMusic();
        }
    }

    if (m_phase == PHASE_LOADING)
        m_app->SetMouseMode(MOUSE_NONE);
    else
        m_app->SetMouseMode(MOUSE_ENGINE);

    m_engine->LoadAllTextures();
}

//! Processes an event
bool CRobotMain::EventProcess(Event &event)
{
    /* Motion vector management */

    if (event.type == EVENT_KEY_DOWN)
    {
        if (event.key.key == GetInputBinding(INPUT_SLOT_UP   ).primary)    m_keyMotion.y =  1.0f;
        if (event.key.key == GetInputBinding(INPUT_SLOT_UP   ).secondary)  m_keyMotion.y =  1.0f;
        if (event.key.key == GetInputBinding(INPUT_SLOT_DOWN ).primary)    m_keyMotion.y = -1.0f;
        if (event.key.key == GetInputBinding(INPUT_SLOT_DOWN ).secondary)  m_keyMotion.y = -1.0f;
        if (event.key.key == GetInputBinding(INPUT_SLOT_LEFT ).primary)    m_keyMotion.x = -1.0f;
        if (event.key.key == GetInputBinding(INPUT_SLOT_LEFT ).secondary)  m_keyMotion.x = -1.0f;
        if (event.key.key == GetInputBinding(INPUT_SLOT_RIGHT).primary)    m_keyMotion.x =  1.0f;
        if (event.key.key == GetInputBinding(INPUT_SLOT_RIGHT).secondary)  m_keyMotion.x =  1.0f;
        if (event.key.key == GetInputBinding(INPUT_SLOT_GUP  ).primary)    m_keyMotion.z =  1.0f;
        if (event.key.key == GetInputBinding(INPUT_SLOT_GUP  ).secondary)  m_keyMotion.z =  1.0f;
        if (event.key.key == GetInputBinding(INPUT_SLOT_GDOWN).primary)    m_keyMotion.z = -1.0f;
        if (event.key.key == GetInputBinding(INPUT_SLOT_GDOWN).secondary)  m_keyMotion.z = -1.0f;
    }
    else if (event.type == EVENT_KEY_UP)
    {
        if (event.key.key == GetInputBinding(INPUT_SLOT_UP   ).primary)    m_keyMotion.y =  0.0f;
        if (event.key.key == GetInputBinding(INPUT_SLOT_UP   ).secondary)  m_keyMotion.y =  0.0f;
        if (event.key.key == GetInputBinding(INPUT_SLOT_DOWN ).primary)    m_keyMotion.y =  0.0f;
        if (event.key.key == GetInputBinding(INPUT_SLOT_DOWN ).secondary)  m_keyMotion.y =  0.0f;
        if (event.key.key == GetInputBinding(INPUT_SLOT_LEFT ).primary)    m_keyMotion.x =  0.0f;
        if (event.key.key == GetInputBinding(INPUT_SLOT_LEFT ).secondary)  m_keyMotion.x =  0.0f;
        if (event.key.key == GetInputBinding(INPUT_SLOT_RIGHT).primary)    m_keyMotion.x =  0.0f;
        if (event.key.key == GetInputBinding(INPUT_SLOT_RIGHT).secondary)  m_keyMotion.x =  0.0f;
        if (event.key.key == GetInputBinding(INPUT_SLOT_GUP  ).primary)    m_keyMotion.z =  0.0f;
        if (event.key.key == GetInputBinding(INPUT_SLOT_GUP  ).secondary)  m_keyMotion.z =  0.0f;
        if (event.key.key == GetInputBinding(INPUT_SLOT_GDOWN).primary)    m_keyMotion.z =  0.0f;
        if (event.key.key == GetInputBinding(INPUT_SLOT_GDOWN).secondary)  m_keyMotion.z =  0.0f;
    }
    else if (event.type == EVENT_JOY_AXIS)
    {
        if (event.joyAxis.axis == GetJoyAxisBinding(JOY_AXIS_SLOT_X).axis)
        {
            m_joyMotion.x = Math::Neutral(event.joyAxis.value / 32768.0f, m_joystickDeadzone);
            if (GetJoyAxisBinding(JOY_AXIS_SLOT_X).invert)
                m_joyMotion.x *= -1.0f;
        }

        if (event.joyAxis.axis == GetJoyAxisBinding(JOY_AXIS_SLOT_Y).axis)
        {
            m_joyMotion.y = Math::Neutral(event.joyAxis.value / 32768.0f, m_joystickDeadzone);
            if (GetJoyAxisBinding(JOY_AXIS_SLOT_Y).invert)
                m_joyMotion.y *= -1.0f;
        }

        if (event.joyAxis.axis == GetJoyAxisBinding(JOY_AXIS_SLOT_Z).axis)
        {
            m_joyMotion.z = Math::Neutral(event.joyAxis.value / 32768.0f, m_joystickDeadzone);
            if (GetJoyAxisBinding(JOY_AXIS_SLOT_Z).invert)
                m_joyMotion.z *= -1.0f;
        }
    }

    event.motionInput = Math::Clamp(m_joyMotion + m_keyMotion, Math::Vector(-1.0f, -1.0f, -1.0f), Math::Vector(1.0f, 1.0f, 1.0f));



    if (event.type == EVENT_FRAME)
    {
        if (!m_movie->EventProcess(event))  // end of the movie?
        {
            MainMovieType type = m_movie->GetStopType();
            if (type == MM_SATCOMopen)
            {
                ChangePause(false);
                SelectObject(m_infoObject, false);  // hands over the command buttons
                m_map->ShowMap(m_mapShow);
                m_displayText->HideText(false);
                int i = m_movieInfoIndex;
                StartDisplayInfo(m_movieInfoIndex, false);
                m_movieInfoIndex = i;
            }
        }

        m_dialog->EventProcess(event);
        m_displayText->EventProcess(event);
        RemoteCamera(m_cameraPan, m_cameraZoom, event.rTime);

        m_interface->EventProcess(event);
        if (m_displayInfo != nullptr)  // current edition?
            m_displayInfo->EventProcess(event);

        return EventFrame(event);
    }

    // Management of the console.
    if (m_phase != PHASE_NAME &&
        !m_movie->IsExist()   &&
        !m_movieLock && !m_editLock && !m_engine->GetPause() &&
        event.type == EVENT_KEY_DOWN &&
        event.key.key == KEY(PAUSE))  // Pause ?
    {
        Ui::CEdit* pe = static_cast<Ui::CEdit*>(m_interface->SearchControl(EVENT_CMD));
        if (pe == nullptr) return false;
        pe->SetState(Ui::STATE_VISIBLE);
        pe->SetFocus(true);
        if (m_phase == PHASE_SIMUL) ChangePause(true);
        m_cmdEdit = true;
        return false;
    }
    if (event.type == EVENT_KEY_DOWN &&
        event.key.key == KEY(RETURN) && m_cmdEdit)
    {
        char cmd[50];
        Ui::CEdit* pe = static_cast<Ui::CEdit*>(m_interface->SearchControl(EVENT_CMD));
        if (pe == nullptr) return false;
        pe->GetText(cmd, 50);
        pe->SetText("");
        pe->ClearState(Ui::STATE_VISIBLE);
        if (m_phase == PHASE_SIMUL) ChangePause(false);
        ExecuteCmd(cmd);
        m_cmdEdit = false;
        return false;
    }

    // Management of the speed change.
    if (event.type == EVENT_SPEED)
        SetSpeed(1.0f);

    if (!m_dialog->EventProcess(event))
    {
        if (event.type == EVENT_MOUSE_MOVE)
        {
            m_lastMousePos = event.mousePos;
            HiliteObject(event.mousePos);
        }
        return false;
    }

    if (!m_displayText->EventProcess(event))
        return false;

    if (event.type == EVENT_MOUSE_MOVE)
    {
        m_lastMousePos = event.mousePos;
        HiliteObject(event.mousePos);
    }

    if (m_displayInfo != nullptr)  // current info?
    {
        m_displayInfo->EventProcess(event);

        if (event.type == EVENT_KEY_DOWN)
        {
            if (event.key.key == GetInputBinding(INPUT_SLOT_HELP).primary ||
                event.key.key == GetInputBinding(INPUT_SLOT_HELP).secondary ||
                event.key.key == GetInputBinding(INPUT_SLOT_PROG).primary ||
                event.key.key == GetInputBinding(INPUT_SLOT_PROG).secondary ||
                event.key.key == KEY(ESCAPE))
            {
                StopDisplayInfo();
            }
        }

        if (event.type == EVENT_OBJECT_INFOOK)
            StopDisplayInfo();

        return false;
    }

    CObject* obj;

    // Simulation phase of the game
    if (m_phase == PHASE_SIMUL)
    {
        UpdateInfoText();

        if (!m_editFull)
            m_camera->EventProcess(event);

        switch (event.type)
        {
            case EVENT_KEY_DOWN:
                KeyCamera(event.type, event.key.key);
                HiliteClear();
                if (event.key.key == KEY(F11))
                {
                    m_particle->WriteWheelTrace("Savegame/t.png", 256, 256, Math::Vector(16.0f, 0.0f, -368.0f), Math::Vector(140.0f, 0.0f, -248.0f));
                    return false;
                }
                if (m_editLock)  // current edition?
                {
                    if (event.key.key == GetInputBinding(INPUT_SLOT_HELP).primary ||
                        event.key.key == GetInputBinding(INPUT_SLOT_HELP).secondary)
                    {
                        StartDisplayInfo(SATCOM_HUSTON, false);
                        return false;
                    }
                    if (event.key.key == GetInputBinding(INPUT_SLOT_PROG).primary ||
                        event.key.key == GetInputBinding(INPUT_SLOT_PROG).secondary)
                    {
                        StartDisplayInfo(SATCOM_PROG, false);
                        return false;
                    }
                    break;
                }
                if (m_movieLock)  // current movie?
                {
                    if (event.key.key == GetInputBinding(INPUT_SLOT_QUIT).primary ||
                        event.key.key == GetInputBinding(INPUT_SLOT_QUIT).secondary ||
                        event.key.key == KEY(ESCAPE))
                    {
                        AbortMovie();
                    }
                    return false;
                }
                if (m_camera->GetType() == Gfx::CAM_TYPE_VISIT)
                {
                    if (event.key.key == GetInputBinding(INPUT_SLOT_VISIT).primary ||
                        event.key.key == GetInputBinding(INPUT_SLOT_VISIT).secondary)
                    {
                        StartDisplayVisit(EVENT_NULL);
                    }
                    if (event.key.key == GetInputBinding(INPUT_SLOT_QUIT).primary ||
                        event.key.key == GetInputBinding(INPUT_SLOT_QUIT).secondary ||
                        event.key.key == KEY(ESCAPE))
                    {
                        StopDisplayVisit();
                    }
                    return false;
                }
                if (event.key.key == GetInputBinding(INPUT_SLOT_QUIT).primary ||
                    event.key.key == GetInputBinding(INPUT_SLOT_QUIT).secondary)
                {
                    if (m_movie->IsExist())
                        StartDisplayInfo(SATCOM_HUSTON, false);
                    else if (m_winDelay > 0.0f)
                        ChangePhase(PHASE_WIN);
                    else if (m_lostDelay > 0.0f)
                        ChangePhase(PHASE_LOST);
                    else if (!m_cmdEdit) {
                        m_dialog->StartAbort();  // do you want to leave?
                    }
                }
                if (event.key.key == KEY(PAUSE))
                {
                    if (!m_movieLock && !m_editLock && !m_cmdEdit &&
                        m_camera->GetType() != Gfx::CAM_TYPE_VISIT &&
                        !m_movie->IsExist())
                    {
                        ChangePause(!m_engine->GetPause());
                    }
                }
                if (event.key.key == GetInputBinding(INPUT_SLOT_CAMERA).primary ||
                    event.key.key == GetInputBinding(INPUT_SLOT_CAMERA).secondary)
                {
                    ChangeCamera();
                }
                if (event.key.key == GetInputBinding(INPUT_SLOT_DESEL).primary ||
                    event.key.key == GetInputBinding(INPUT_SLOT_DESEL).secondary)
                {
                    if (m_shortCut)
                        DeselectObject();
                }
                if (event.key.key == GetInputBinding(INPUT_SLOT_HUMAN).primary ||
                    event.key.key == GetInputBinding(INPUT_SLOT_HUMAN).secondary)
                {
                    SelectHuman();
                }
                if (event.key.key == GetInputBinding(INPUT_SLOT_NEXT).primary ||
                    event.key.key == GetInputBinding(INPUT_SLOT_NEXT).secondary)
                {
                    if (m_shortCut)
                        m_short->SelectNext();
                }
                if (event.key.key == GetInputBinding(INPUT_SLOT_HELP).primary ||
                    event.key.key == GetInputBinding(INPUT_SLOT_HELP).secondary)
                {
                    StartDisplayInfo(SATCOM_HUSTON, true);
                }
                if (event.key.key == GetInputBinding(INPUT_SLOT_PROG).primary ||
                    event.key.key == GetInputBinding(INPUT_SLOT_PROG).secondary)
                {
                    StartDisplayInfo(SATCOM_PROG, true);
                }
                if (event.key.key == GetInputBinding(INPUT_SLOT_VISIT).primary ||
                    event.key.key == GetInputBinding(INPUT_SLOT_VISIT).secondary)
                {
                    StartDisplayVisit(EVENT_NULL);
                }
                if (event.key.key == GetInputBinding(INPUT_SLOT_SPEED10).primary ||
                    event.key.key == GetInputBinding(INPUT_SLOT_SPEED10).secondary)
                {
                    SetSpeed(1.0f);
                }
                if (event.key.key == GetInputBinding(INPUT_SLOT_SPEED15).primary ||
                    event.key.key == GetInputBinding(INPUT_SLOT_SPEED15).secondary)
                {
                    SetSpeed(1.5f);
                }
                if (event.key.key == GetInputBinding(INPUT_SLOT_SPEED20).primary ||
                    event.key.key == GetInputBinding(INPUT_SLOT_SPEED20).secondary)
                {
                    SetSpeed(2.0f);
                }
                break;

            case EVENT_KEY_UP:
                KeyCamera(event.type, event.key.key);
                break;

            case EVENT_MOUSE_BUTTON_DOWN:
                if (event.mouseButton.button != MOUSE_BUTTON_LEFT) // only left mouse button
                    break;

                obj = DetectObject(event.mousePos);
                if (!m_shortCut) obj = nullptr;
                if (obj != nullptr && obj->GetType() == OBJECT_TOTO)
                {
                    if (m_displayInfo != nullptr)  // current info?
                    {
                        StopDisplayInfo();
                    }
                    else
                    {
                        if (!m_editLock)
                            StartDisplayInfo(SATCOM_HUSTON, true);
                    }
                }
                else
                    SelectObject(obj);
                break;

            case EVENT_MOUSE_BUTTON_UP:
                if (event.mouseButton.button != MOUSE_BUTTON_LEFT) // only left mouse button
                    break;

                m_cameraPan  = 0.0f;
                m_cameraZoom = 0.0f;
                break;

            case EVENT_BUTTON_QUIT:
                if (m_movie->IsExist())
                    StartDisplayInfo(SATCOM_HUSTON, false);
                else if (m_winDelay > 0.0f)
                    ChangePhase(PHASE_WIN);
                else if (m_lostDelay > 0.0f)

                    ChangePhase(PHASE_LOST);
                else
                    m_dialog->StartAbort();  // do you want to leave?
                break;

            case EVENT_OBJECT_LIMIT:
                StartShowLimit();
                break;

            case EVENT_OBJECT_DESELECT:
                if (m_shortCut)
                    DeselectObject();
                break;

            case EVENT_OBJECT_HELP:
                HelpObject();
                break;

            case EVENT_OBJECT_CAMERA:
                ChangeCamera();
                break;

            case EVENT_OBJECT_CAMERAleft:
                m_cameraPan = -1.0f;
                break;
            case EVENT_OBJECT_CAMERAright:
                m_cameraPan = 1.0f;
                break;
            case EVENT_OBJECT_CAMERAnear:
                m_cameraZoom = -1.0f;
                break;
            case EVENT_OBJECT_CAMERAaway:
                m_cameraZoom = 1.0f;
                break;

            case EVENT_OBJECT_DELETE:
                m_dialog->StartDeleteObject();  // do you want to destroy it?
                break;

            case EVENT_OBJECT_BHELP:
                StartDisplayInfo(SATCOM_HUSTON, true);
                break;

            case EVENT_OBJECT_SOLUCE:
                StartDisplayInfo(SATCOM_SOLUCE, true);
                break;

            case EVENT_OBJECT_MAPZOOM:
                m_map->ZoomMap();
                break;

            case EVENT_DT_VISIT0:
            case EVENT_DT_VISIT1:
            case EVENT_DT_VISIT2:
            case EVENT_DT_VISIT3:
            case EVENT_DT_VISIT4:
                StartDisplayVisit(event.type);
                break;

            case EVENT_DT_END:
                StopDisplayVisit();
                break;

            case EVENT_OBJECT_SHORTCUT00:
            case EVENT_OBJECT_SHORTCUT01:
            case EVENT_OBJECT_SHORTCUT02:
            case EVENT_OBJECT_SHORTCUT03:
            case EVENT_OBJECT_SHORTCUT04:
            case EVENT_OBJECT_SHORTCUT05:
            case EVENT_OBJECT_SHORTCUT06:
            case EVENT_OBJECT_SHORTCUT07:
            case EVENT_OBJECT_SHORTCUT08:
            case EVENT_OBJECT_SHORTCUT09:
            case EVENT_OBJECT_SHORTCUT10:
            case EVENT_OBJECT_SHORTCUT11:
            case EVENT_OBJECT_SHORTCUT12:
            case EVENT_OBJECT_SHORTCUT13:
            case EVENT_OBJECT_SHORTCUT14:
            case EVENT_OBJECT_SHORTCUT15:
            case EVENT_OBJECT_SHORTCUT16:
            case EVENT_OBJECT_SHORTCUT17:
            case EVENT_OBJECT_SHORTCUT18:
            case EVENT_OBJECT_SHORTCUT19:
                m_short->SelectShortcut(event.type);
                break;

            case EVENT_OBJECT_MOVIELOCK:
                AbortMovie();
                break;

            case EVENT_WIN:
                ChangePhase(PHASE_WIN);
                break;

            case EVENT_LOST:
                ChangePhase(PHASE_LOST);
                break;

            default:
                break;
        }

        EventObject(event);
        return false;
    }

    if (m_phase == PHASE_PERSO)
        EventObject(event);

    if (m_phase == PHASE_WIN  ||
        m_phase == PHASE_LOST)
    {
        EventObject(event);

        switch (event.type)
        {
            case EVENT_KEY_DOWN:
                if (event.key.key == KEY(ESCAPE) ||
                    event.key.key == KEY(RETURN))
                {
                    if (m_winTerminate)
                        ChangePhase(PHASE_INIT);
                    else
                        ChangePhase(PHASE_TERM);
                }
                break;

            case EVENT_BUTTON_OK:
                if (m_winTerminate)
                    ChangePhase(PHASE_INIT);
                else
                    ChangePhase(PHASE_TERM);
                break;

            default:
                break;
        }
    }

    return true;
}



//! Executes a command
void CRobotMain::ExecuteCmd(char *cmd)
{
    if (cmd[0] == 0) return;

    if (m_phase == PHASE_SIMUL)
    {
        if (strcmp(cmd, "winmission") == 0)
            m_eventQueue->AddEvent(Event(EVENT_WIN));

        if (strcmp(cmd, "lostmission") == 0)
            m_eventQueue->AddEvent(Event(EVENT_LOST));

        if (strcmp(cmd, "trainerpilot") == 0)
        {
            m_trainerPilot = !m_trainerPilot;
            return;
        }

        if (strcmp(cmd, "fly") == 0)
        {
            g_researchDone |= RESEARCH_FLY;

            m_eventQueue->AddEvent(Event(EVENT_UPDINTERFACE));
            return;
        }

        if (strcmp(cmd, "allresearch") == 0)
        {
            g_researchDone = -1;  // all research are done

            m_eventQueue->AddEvent(Event(EVENT_UPDINTERFACE));
            return;
        }

        if (strcmp(cmd, "nolimit") == 0)
        {
            m_terrain->SetFlyingMaxHeight(280.0f);
            return;
        }

        if (strcmp(cmd, "photo1") == 0)
        {
            m_freePhoto = !m_freePhoto;
            if (m_freePhoto)
            {
                m_camera->SetType(Gfx::CAM_TYPE_FREE);
                ChangePause(true);
            }
            else
            {
                m_camera->SetType(Gfx::CAM_TYPE_BACK);
                ChangePause(false);
            }
            return;
        }

        if (strcmp(cmd, "photo2") == 0)
        {
            m_freePhoto = !m_freePhoto;
            if (m_freePhoto)
            {
                m_camera->SetType(Gfx::CAM_TYPE_FREE);
                ChangePause(true);
                DeselectAll();  // removes the control buttons
                m_map->ShowMap(false);
                m_displayText->HideText(true);
            }
            else
            {
                m_camera->SetType(Gfx::CAM_TYPE_BACK);
                ChangePause(false);
                m_map->ShowMap(m_mapShow);
                m_displayText->HideText(false);
            }
            return;
        }

        if (strcmp(cmd, "noclip") == 0)
        {
            CObject* object = GetSelect();
            if (object != nullptr)
                object->SetClip(false);
            return;
        }

        if (strcmp(cmd, "clip") == 0)
        {
            CObject* object = GetSelect();
            if (object != nullptr)
                object->SetClip(true);
            return;
        }

        if (strcmp(cmd, "addhusky") == 0)
        {
            CObject* object = GetSelect();
            if (object != nullptr)
                object->SetMagnifyDamage(object->GetMagnifyDamage()*0.1f);
            return;
        }

        if (strcmp(cmd, "addfreezer") == 0)
        {
            CObject* object = GetSelect();
            if (object != nullptr)
                object->SetRange(object->GetRange()*10.0f);
            return;
        }
        
        if (strcmp(cmd, "\155\157\157") == 0)
        {
            // VGhpcyBpcyBlYXN0ZXItZWdnIGFuZCBzbyBpdCBzaG91bGQgYmUgb2JmdXNjYXRlZCEgRG8gbm90
            // IGNsZWFuLXVwIHRoaXMgY29kZSEK
            GetLogger()->Info(" _________________________\n");
            GetLogger()->Info("< \x50\x6F\x6C\x73\x6B\x69 \x50\x6F\x72\x74\x61\x6C C\x6F\x6C\x6F\x62\x6F\x74\x61! \x3E\n");
            GetLogger()->Info(" -------------------------\n");
            GetLogger()->Info("        \x5C\x20\x20\x20\x5E\x5F\x5F\x5E\n");
            GetLogger()->Info("        \x20\x5C\x20\x20\x28\x6F\x6F\x29\x5C\x5F\x5F\x5F\x5F\x5F\x5F\x5F\n");
            GetLogger()->Info("            \x28\x5F\x5F\x29\x5C   \x20\x20\x20\x20\x29\x5C\x2F\x5C\n");
            GetLogger()->Info("            \x20\x20\x20\x20\x7C|\x2D\x2D\x2D\x2D\x77\x20\x7C\n");
            GetLogger()->Info("          \x20\x20    \x7C\x7C\x20\x20\x20\x20 ||\n");
        }

        if (strcmp(cmd, "fullpower") == 0)
        {
            CObject* object = GetSelect();
            if (object != nullptr)
            {
                CObject* power = object->GetPower();
                if (power != nullptr)
                    power->SetEnergy(1.0f);

                object->SetShield(1.0f);
                CPhysics* physics = object->GetPhysics();
                if (physics != nullptr)
                    physics->SetReactorRange(1.0f);
            }
            return;
        }

        if (strcmp(cmd, "fullenergy") == 0)
        {
            CObject* object = GetSelect();
            if (object != nullptr)
            {
                CObject* power = object->GetPower();
                if (power != nullptr)
                    power->SetEnergy(1.0f);
            }
            return;
        }

        if (strcmp(cmd, "fullshield") == 0)
        {
            CObject* object = GetSelect();
            if (object != nullptr)
                object->SetShield(1.0f);
            return;
        }

        if (strcmp(cmd, "fullrange") == 0)
        {
            CObject* object = GetSelect();
            if (object != nullptr)
            {
                CPhysics* physics = object->GetPhysics();
                if (physics != nullptr)
                    physics->SetReactorRange(1.0f);
            }
            return;
        }
    }

    if (strcmp(cmd, "debugmode") == 0)
    {
        m_app->SetDebugMode(!m_app->GetDebugMode());
        return;
    }

    if (strcmp(cmd, "showstat") == 0)
    {
        m_engine->SetShowStats(!m_engine->GetShowStats());
        return;
    }

    if (strcmp(cmd, "invshadow") == 0)
    {
        m_engine->SetShadow(!m_engine->GetShadow());
        return;
    }

    if (strcmp(cmd, "invdirty") == 0)
    {
        m_engine->SetDirty(!m_engine->GetDirty());
        return;
    }

    if (strcmp(cmd, "invfog") == 0)
    {
        m_engine->SetFog(!m_engine->GetFog());
        return;
    }

    if (strcmp(cmd, "invlens") == 0)
    {
        m_engine->SetLensMode(!m_engine->GetLensMode());
        return;
    }

    if (strcmp(cmd, "invwater") == 0)
    {
        m_engine->SetWaterMode(!m_engine->GetWaterMode());
        return;
    }

    if (strcmp(cmd, "invsky") == 0)
    {
        m_engine->SetSkyMode(!m_engine->GetSkyMode());
        return;
    }

    if (strcmp(cmd, "invplanet") == 0)
    {
        m_engine->SetPlanetMode(!m_engine->GetPlanetMode());
        return;
    }

    if (strcmp(cmd, "showpos") == 0)
    {
        m_showPos = !m_showPos;
        return;
    }

    if (strcmp(cmd, "selectinsect") == 0)
    {
        m_selectInsect = !m_selectInsect;
        return;
    }

    if (strcmp(cmd, "showsoluce") == 0)
    {
        m_showSoluce = !m_showSoluce;
        m_dialog->ShowSoluceUpdate();
        return;
    }

/* TODO: #if _TEEN
    if (strcmp(cmd, "allteens") == 0)
#else*/
    if (strcmp(cmd, "allmission") == 0)
    {
        m_showAll = !m_showAll;
        m_dialog->AllMissionUpdate();
        return;
    }

    if (strcmp(cmd, "invradar") == 0)
    {
        m_cheatRadar = !m_cheatRadar;
        return;
    }

    if (strcmp(cmd, "speed4") == 0) {
        SetSpeed(4.0f);
        UpdateSpeedLabel();
	return;
    }
    if (strcmp(cmd, "speed8") == 0) {
        SetSpeed(8.0f);
        UpdateSpeedLabel();
	return;
    }

    if (m_phase == PHASE_SIMUL)
        m_displayText->DisplayError(ERR_CMD, Math::Vector(0.0f,0.0f,0.0f));
}



//! Returns the type of current movie
MainMovieType CRobotMain::GetMainMovie()
{
    return m_movie->GetType();
}


//! Clears the display of instructions
void CRobotMain::FlushDisplayInfo()
{
    for (int i = 0; i < SATCOM_MAX; i++)
    {
        m_infoFilename[i][0] = 0;
        m_infoPos[i] = 0;
    }
    strcpy(m_infoFilename[SATCOM_OBJECT], "help/") + m_app->GetLanguageChar() + std::string("/objects.txt");
    m_infoIndex = 0;
}

//! Beginning of the displaying of instructions.
//! index: SATCOM_*
void CRobotMain::StartDisplayInfo(int index, bool movie)
{
    if (m_cmdEdit || m_satComLock || m_lockedSatCom) return;

    CObject* obj = GetSelect();
    bool human = obj != nullptr && obj->GetType() == OBJECT_HUMAN;

    if (!m_editLock && movie && !m_movie->IsExist() && human)
    {
        CMotion* motion = obj->GetMotion();
        if (motion != nullptr && motion->GetAction() == -1)
        {
            m_movieInfoIndex = index;
            m_movie->Start(MM_SATCOMopen, 2.5f);
            ChangePause(true);
            m_infoObject = DeselectAll();  // removes the control buttons
            m_displayText->HideText(true);
            return;
        }
    }

    if (m_movie->IsExist())
    {
        m_movie->Stop();
        ChangePause(false);
        SelectObject(m_infoObject, false);  // hands over the command buttons
        m_displayText->HideText(false);
    }

    StartDisplayInfo(m_infoFilename[index], index);
}

//! Beginning of the displaying of instructions
void CRobotMain::StartDisplayInfo(const char *filename, int index)
{
    if (m_cmdEdit) return;

    m_movieInfoIndex = -1;
    ClearInterface();  // removes setting evidence and tooltip

    if (!m_editLock)
    {
        m_infoObject = DeselectAll();  // removes the control buttons
        m_displayText->HideText(true);
        m_sound->MuteAll(true);
    }

    Ui::CButton* pb = static_cast<Ui::CButton*>(m_interface->SearchControl(EVENT_BUTTON_QUIT));
    if (pb != nullptr)
    {
        pb->ClearState(Ui::STATE_VISIBLE);
    }

    bool soluce = m_dialog->GetSceneSoluce();

    m_displayInfo = new Ui::CDisplayInfo();
    m_displayInfo->StartDisplayInfo(filename, index, soluce);

    m_infoIndex = index;
    if (index != -1)
        m_displayInfo->SetPosition(m_infoPos[index]);
}

//! End of displaying of instructions
void CRobotMain::StopDisplayInfo()
{
    if (m_cmdEdit) return;
    
    if (m_movieInfoIndex != -1)  // film to read the SatCom?
        m_movie->Start(MM_SATCOMclose, 2.0f);

    if (m_infoIndex != -1)
        m_infoPos[m_infoIndex] = m_displayInfo->GetPosition();

    m_displayInfo->StopDisplayInfo();

    delete m_displayInfo;
    m_displayInfo = nullptr;

    if (!m_editLock)
    {
        Ui::CButton* pb = static_cast<Ui::CButton*>(m_interface->SearchControl(EVENT_BUTTON_QUIT));
        if (pb != nullptr)
            pb->SetState(Ui::STATE_VISIBLE);

        SelectObject(m_infoObject, false);  // gives the command buttons
        m_displayText->HideText(false);

        m_sound->MuteAll(false);
    }

    if (m_infoUsed == 0)
        m_displayText->ClearText();  // removes message "see SatCom ..."
    m_infoUsed ++;
}

//! Returns the name of the text display
char* CRobotMain::GetDisplayInfoName(int index)
{
    return m_infoFilename[index];
}

//! Returns the name of the text display
int CRobotMain::GetDisplayInfoPosition(int index)
{
    return m_infoPos[index];
}

//! Returns the name of the text display
void CRobotMain::SetDisplayInfoPosition(int index, int pos)
{
    m_infoPos[index] = pos;
}


//! Beginning of a dialogue during the game
void CRobotMain::StartSuspend()
{
    m_map->ShowMap(false);
    m_infoObject = DeselectAll();  // removes the control buttons
    m_displayText->HideText(true);

    Ui::CButton* pb = static_cast<Ui::CButton*>(m_interface->SearchControl(EVENT_BUTTON_QUIT));
    if (pb != nullptr)
        pb->ClearState(Ui::STATE_VISIBLE);

    m_suspend = true;
}

//! End of dialogue during the game
void CRobotMain::StopSuspend()
{
    Ui::CButton* pb = static_cast<Ui::CButton*>(m_interface->SearchControl(EVENT_BUTTON_QUIT));
    if (pb != nullptr)
        pb->SetState(Ui::STATE_VISIBLE);

    SelectObject(m_infoObject, false);  // gives the command buttons
    m_map->ShowMap(m_mapShow);
    m_displayText->HideText(false);

    m_suspend = false;
}


//! Returns the absolute time of the game
float CRobotMain::GetGameTime()
{
    return m_gameTime;
}



//! Managing the size of the default fonts
void CRobotMain::SetFontSize(float size)
{
    m_fontSize = size;
    /* TODO: profile
    SetLocalProfileFloat("Edit", "FontSize", m_fontSize); */
}

float CRobotMain::GetFontSize()
{
    return m_fontSize;
}

//! Managing the size of the default window
void CRobotMain::SetWindowPos(Math::Point pos)
{
    m_windowPos = pos;
    /* TODO: profile
    SetLocalProfileFloat("Edit", "WindowPos.x", m_windowPos.x);
    SetLocalProfileFloat("Edit", "WindowPos.y", m_windowPos.y); */
}

Math::Point CRobotMain::GetWindowPos()
{
    return m_windowPos;
}

void CRobotMain::SetWindowDim(Math::Point dim)
{
    m_windowDim = dim;
    /* TODO: profile
    SetLocalProfileFloat("Edit", "WindowDim.x", m_windowDim.x);
    SetLocalProfileFloat("Edit", "WindowDim.y", m_windowDim.y); */
}

Math::Point CRobotMain::GetWindowDim()
{
    return m_windowDim;
}


//! Managing windows open/save
void CRobotMain::SetIOPublic(bool mode)
{
    m_IOPublic = mode;
    /* TODO: profile
    SetLocalProfileInt("Edit", "IOPublic", m_IOPublic); */
}

bool CRobotMain::GetIOPublic()
{
    return m_IOPublic;
}

void CRobotMain::SetIOPos(Math::Point pos)
{
    m_IOPos = pos;
    /* TODO: profile
    SetLocalProfileFloat("Edit", "IOPos.x", m_IOPos.x);
    SetLocalProfileFloat("Edit", "IOPos.y", m_IOPos.y); */
}

Math::Point CRobotMain::GetIOPos()
{
    return m_IOPos;
}

void CRobotMain::SetIODim(Math::Point dim)
{
    m_IODim = dim;
    /* TODO: profile
    SetLocalProfileFloat("Edit", "IODim.x", m_IODim.x);
    SetLocalProfileFloat("Edit", "IODim.y", m_IODim.y); */
}

Math::Point CRobotMain::GetIODim()
{
    return m_IODim;
}



//! Start of the visit instead of an error
void CRobotMain::StartDisplayVisit(EventType event)
{
    if (m_editLock) return;

    Ui::CWindow* pw = static_cast<Ui::CWindow*>(m_interface->SearchControl(EVENT_WINDOW2));
    if (pw == nullptr) return;

    if (event == EVENT_NULL)  // visit by keyboard shortcut?
    {
        int i;
        if (m_visitLast != EVENT_NULL)  // already a current visit?
            i = m_visitLast-EVENT_DT_VISIT0;
        else
            i = Ui::MAXDTLINE;

        // Seeks the last.
        for (int j = 0; j < Ui::MAXDTLINE; j++)
        {
            i --;
            if (i < 0) i = Ui::MAXDTLINE-1;

            Ui::CButton* button = static_cast<Ui::CButton*>(pw->SearchControl(static_cast<EventType>(EVENT_DT_VISIT0+i)));
            if (button == nullptr || !button->TestState(Ui::STATE_ENABLE)) continue;

            Ui::CGroup* group = static_cast<Ui::CGroup*>(pw->SearchControl(static_cast<EventType>(EVENT_DT_GROUP0+i)));
            if (group != nullptr)
            {
                event = static_cast<EventType>(EVENT_DT_VISIT0+i);
                break;
            }
        }
    }
    if (event == EVENT_NULL)
    {
        m_sound->Play(SOUND_TZOING);  // nothing to do!
        return;
    }

    m_visitLast = event;

    ClearInterface();  // removes setting evidence and tooltip

    if (m_camera->GetType() == Gfx::CAM_TYPE_VISIT)  // already a current visit?
    {
        m_camera->StopVisit();
        m_displayText->ClearVisit();
    }
    else
    {
        m_visitObject = DeselectAll();  // removes the control buttons
    }

    // Creates the "continue" button.
    if (m_interface->SearchControl(EVENT_DT_END) == 0)
    {
        Math::Point pos, dim;
        pos.x = 10.0f/640.0f;
        pos.y = 10.0f/480.0f;
        dim.x = 50.0f/640.0f;
        dim.y = 50.0f/480.0f;
        m_interface->CreateButton(pos, dim, 16, EVENT_DT_END);
    }

    // Creates the arrow to show the place.
    if (m_visitArrow != 0)
    {
        m_visitArrow->DeleteObject();
        delete m_visitArrow;
        m_visitArrow = 0;
    }

    Math::Vector goal = m_displayText->GetVisitGoal(event);
    m_visitArrow = CreateObject(goal, 0.0f, 1.0f, 10.0f, OBJECT_SHOW, false, false, 0);

    m_visitPos = m_visitArrow->GetPosition(0);
    m_visitPosArrow = m_visitPos;
    m_visitPosArrow.y += m_displayText->GetVisitHeight(event);
    m_visitArrow->SetPosition(0, m_visitPosArrow);

    m_visitTime = 0.0;
    m_visitParticle = 0.0f;

    m_particle->DeleteParticle(Gfx::PARTISHOW);

    m_camera->StartVisit(m_displayText->GetVisitGoal(event),
                         m_displayText->GetVisitDist(event));
    m_displayText->SetVisit(event);
    ChangePause(true);
}

//! Move the arrow to visit
void CRobotMain::FrameVisit(float rTime)
{
    if (m_visitArrow == 0) return;

    // Moves the arrow.
    m_visitTime += rTime;

    Math::Vector pos = m_visitPosArrow;
    pos.y += 1.5f+sinf(m_visitTime*4.0f)*4.0f;
    m_visitArrow->SetPosition(0, pos);
    m_visitArrow->SetAngleY(0, m_visitTime*2.0f);

    // Manages the particles "arrows".
    m_visitParticle -= rTime;
    if (m_visitParticle <= 0.0f)
    {
        m_visitParticle = 1.5f;

        pos = m_visitPos;
        float level = m_terrain->GetFloorLevel(pos)+2.0f;
        if (pos.y < level) pos.y = level;  // not below the ground
        Math::Vector speed(0.0f, 0.0f, 0.0f);
        Math::Point dim;
        dim.x = 30.0f;
        dim.y = dim.x;
        m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISHOW, 2.0f);
    }
}

//! End of the visit instead of an error
void CRobotMain::StopDisplayVisit()
{
    m_visitLast = EVENT_NULL;

    // Removes the button.
    m_interface->DeleteControl(EVENT_DT_END);

    // Removes the arrow.
    if (m_visitArrow != nullptr)
    {
        m_visitArrow->DeleteObject();
        delete m_visitArrow;
        m_visitArrow = nullptr;
    }

    // Removes particles "arrows".
    m_particle->DeleteParticle(Gfx::PARTISHOW);

    m_camera->StopVisit();
    m_displayText->ClearVisit();
    ChangePause(false);
    if (m_visitObject != 0)
    {
        SelectObject(m_visitObject, false);  // gives the command buttons
        m_visitObject = 0;
    }
}



//! Updates all the shortcuts
void CRobotMain::UpdateShortcuts()
{
    m_short->UpdateShortcuts();
}

//! Returns the object that default was select after the creation of a scene
CObject* CRobotMain::GetSelectObject()
{
    if (m_selectObject != nullptr) return m_selectObject;
    return SearchHuman();
}

//! Deselects everything, and returns the object that was selected
CObject* CRobotMain::DeselectAll()
{
    CInstanceManager* iMan = CInstanceManager::GetInstancePointer();
    CObject* prev = nullptr;
    for (int i = 0; i < 1000000; i++)
    {
        CObject* obj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, i));
        if (obj == nullptr) break;

        if (obj->GetSelect()) prev = obj;
        obj->SetSelect(false);
    }
    return prev;
}

//! Selects an object, without attending to deselect the rest
void CRobotMain::SelectOneObject(CObject* obj, bool displayError)
{
    obj->SetSelect(true, displayError);
    m_camera->SetControllingObject(obj);

    ObjectType type = obj->GetType();
    if ( type == OBJECT_HUMAN    ||
         type == OBJECT_MOBILEfa ||
         type == OBJECT_MOBILEta ||
         type == OBJECT_MOBILEwa ||
         type == OBJECT_MOBILEia ||
         type == OBJECT_MOBILEfc ||
         type == OBJECT_MOBILEtc ||
         type == OBJECT_MOBILEwc ||
         type == OBJECT_MOBILEic ||
         type == OBJECT_MOBILEfi ||
         type == OBJECT_MOBILEti ||
         type == OBJECT_MOBILEwi ||
         type == OBJECT_MOBILEii ||
         type == OBJECT_MOBILEfs ||
         type == OBJECT_MOBILEts ||
         type == OBJECT_MOBILEws ||
         type == OBJECT_MOBILEis ||
         type == OBJECT_MOBILErt ||
         type == OBJECT_MOBILErc ||
         type == OBJECT_MOBILErr ||
         type == OBJECT_MOBILErs ||
         type == OBJECT_MOBILEsa ||
         type == OBJECT_MOBILEft ||
         type == OBJECT_MOBILEtt ||
         type == OBJECT_MOBILEwt ||
         type == OBJECT_MOBILEit ||
         type == OBJECT_MOBILEdr ||
         type == OBJECT_APOLLO2  )
    {
        m_camera->SetType(obj->GetCameraType());
        m_camera->SetDist(obj->GetCameraDist());
    }
    else
    {
        m_camera->SetType(Gfx::CAM_TYPE_BACK);
    }

    CObject* toto = SearchToto();
    if (toto != nullptr)
    {
        CMotionToto* mt = static_cast<CMotionToto*>(toto->GetMotion());
        if (mt != nullptr)
            mt->SetLinkType(type);
    }
}

//! Selects the object aimed by the mouse
bool CRobotMain::SelectObject(CObject* obj, bool displayError)
{
    if (m_camera->GetType() == Gfx::CAM_TYPE_VISIT)
        StopDisplayVisit();

    if (m_movieLock || m_editLock || m_pause) return false;
    if (m_movie->IsExist()) return false;
    if (obj == nullptr || !IsSelectable(obj)) return false;

    CObject* prev = DeselectAll();

    if (prev != nullptr && prev != obj)
       obj->AddDeselList(prev);

    SelectOneObject(obj, displayError);
    m_short->UpdateShortcuts();
    return true;
}

//! Deselects the selected object
bool CRobotMain::DeselectObject()
{
    CObject* obj = nullptr;
    CObject* prev = DeselectAll();

    if (prev == nullptr)
        obj = SearchHuman();
    else
        obj = prev->SubDeselList();

    if (obj == nullptr)
        obj = SearchHuman();

    if (obj != nullptr)
        SelectOneObject(obj);
    else
        m_camera->SetType(Gfx::CAM_TYPE_FREE);

    m_short->UpdateShortcuts();
    return true;
}

//! Quickly removes all objects
void CRobotMain::DeleteAllObjects()
{
    CInstanceManager* iMan = CInstanceManager::GetInstancePointer();

    // Removes all pyrotechnic effects in progress.
    while (true)
    {
        Gfx::CPyro* pyro = static_cast<Gfx::CPyro*>(iMan->SearchInstance(CLASS_PYRO, 0));
        if (pyro == nullptr) break;

        pyro->DeleteObject();
        delete pyro;
    }

    // Removes the arrow.
    if (m_visitArrow != nullptr)
    {
        m_visitArrow->DeleteObject();
        delete m_visitArrow;
        m_visitArrow = nullptr;
    }

    for (int i = 0; i < MAXSHOWLIMIT; i++)
        FlushShowLimit(i);

    while (true)
    {
        CObject* obj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, 0));
        if (obj == nullptr) break;

        obj->DeleteObject(true);  // destroys rapidly
        delete obj;
    }
}

//! Selects the human
void CRobotMain::SelectHuman()
{
    SelectObject(SearchHuman());
}

//! Returns the object human
CObject* CRobotMain::SearchHuman()
{
    CInstanceManager* iMan = CInstanceManager::GetInstancePointer();
    for (int i = 0; i < 1000000; i++)
    {
        CObject* obj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, i));
        if (obj == 0)  break;

        ObjectType type = obj->GetType();
        if (type == OBJECT_HUMAN)
            return obj;
    }
    return 0;
}

//! Returns the object toto
CObject* CRobotMain::SearchToto()
{
    CInstanceManager* iMan = CInstanceManager::GetInstancePointer();
    for (int i = 0; i < 1000000; i++)
    {
        CObject* obj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, i));
        if (obj == nullptr) break;

        ObjectType type = obj->GetType();
        if (type == OBJECT_TOTO)
            return obj;
    }
    return nullptr;
}

//! Returns the nearest selectable object from a given position
CObject* CRobotMain::SearchNearest(Math::Vector pos, CObject* exclu)
{
    CInstanceManager* iMan = CInstanceManager::GetInstancePointer();
    float min = 100000.0f;
    CObject* best = 0;
    for (int i = 0; i < 1000000; i++)
    {
        CObject* obj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, i));
        if (obj == nullptr) break;

        if (obj == exclu) continue;
        if (!IsSelectable(obj)) continue;

        ObjectType type = obj->GetType();
        if (type == OBJECT_TOTO) continue;

        Math::Vector oPos = obj->GetPosition(0);
        float dist = Math::DistanceProjected(oPos, pos);
        if (dist < min)
        {
            min = dist;
            best = obj;
        }
    }
    return best;
}

//! Returns the selected object
CObject* CRobotMain::GetSelect()
{
    CInstanceManager* iMan = CInstanceManager::GetInstancePointer();
    for (int i = 0; i < 1000000; i++)
    {
        CObject* obj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, i));
        if (obj == nullptr) break;

        if (obj->GetSelect())
            return obj;
    }
    return nullptr;
}

CObject* CRobotMain::SearchObject(ObjectType type)
{
    CInstanceManager* iMan = CInstanceManager::GetInstancePointer();
    for (int i = 0; i < 1000000; i++)
    {
        CObject* obj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, i));
        if (obj == nullptr) break;

        if (obj->GetType() == type)
            return obj;
    }
    return nullptr;
}

//! Detects the object aimed by the mouse
CObject* CRobotMain::DetectObject(Math::Point pos)
{
    int objRank = m_engine->DetectObject(pos);
    CInstanceManager* iMan = CInstanceManager::GetInstancePointer();

    for (int i = 0; i < 1000000; i++)
    {
        CObject* obj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, i));
        if (obj == nullptr) break;

        if (!obj->GetActif()) continue;
        if (obj->GetProxyActivate()) continue;

        CObject* target = nullptr;
        ObjectType type = obj->GetType();
        if ( type == OBJECT_PORTICO      ||
             type == OBJECT_BASE         ||
             type == OBJECT_DERRICK      ||
             type == OBJECT_FACTORY      ||
             type == OBJECT_REPAIR       ||
             type == OBJECT_DESTROYER    ||
             type == OBJECT_STATION      ||
             type == OBJECT_CONVERT      ||
             type == OBJECT_TOWER        ||
             type == OBJECT_RESEARCH     ||
             type == OBJECT_RADAR        ||
             type == OBJECT_INFO         ||
             type == OBJECT_ENERGY       ||
             type == OBJECT_LABO         ||
             type == OBJECT_NUCLEAR      ||
             type == OBJECT_PARA         ||
             type == OBJECT_SAFE         ||
             type == OBJECT_HUSTON       ||
             type == OBJECT_TARGET1      ||
             type == OBJECT_TARGET2      ||
             type == OBJECT_START        ||
             type == OBJECT_END          ||
             type == OBJECT_STONE        ||
             type == OBJECT_URANIUM      ||
             type == OBJECT_BULLET       ||
             type == OBJECT_METAL        ||
             type == OBJECT_BBOX         ||
             type == OBJECT_KEYa         ||
             type == OBJECT_KEYb         ||
             type == OBJECT_KEYc         ||
             type == OBJECT_KEYd         ||
             type == OBJECT_TNT          ||
             type == OBJECT_SCRAP1       ||
             type == OBJECT_SCRAP2       ||
             type == OBJECT_SCRAP3       ||
             type == OBJECT_SCRAP4       ||
             type == OBJECT_SCRAP5       ||
             type == OBJECT_BOMB         ||
             type == OBJECT_BAG          ||
             type == OBJECT_WAYPOINT     ||
             type == OBJECT_FLAGb        ||
             type == OBJECT_FLAGr        ||
             type == OBJECT_FLAGg        ||
             type == OBJECT_FLAGy        ||
             type == OBJECT_FLAGv        ||
             type == OBJECT_MARKPOWER    ||
             type == OBJECT_MARKSTONE    ||
             type == OBJECT_MARKURANIUM  ||
             type == OBJECT_MARKKEYa     ||
             type == OBJECT_MARKKEYb     ||
             type == OBJECT_MARKKEYc     ||
             type == OBJECT_MARKKEYd     ||
             type == OBJECT_HUMAN        ||
             type == OBJECT_TECH         ||
             type == OBJECT_TOTO         ||
             type == OBJECT_MOBILEfa     ||
             type == OBJECT_MOBILEta     ||
             type == OBJECT_MOBILEwa     ||
             type == OBJECT_MOBILEia     ||
             type == OBJECT_MOBILEfc     ||
             type == OBJECT_MOBILEtc     ||
             type == OBJECT_MOBILEwc     ||
             type == OBJECT_MOBILEic     ||
             type == OBJECT_MOBILEfi     ||
             type == OBJECT_MOBILEti     ||
             type == OBJECT_MOBILEwi     ||
             type == OBJECT_MOBILEii     ||
             type == OBJECT_MOBILEfs     ||
             type == OBJECT_MOBILEts     ||
             type == OBJECT_MOBILEws     ||
             type == OBJECT_MOBILEis     ||
             type == OBJECT_MOBILErt     ||
             type == OBJECT_MOBILErc     ||
             type == OBJECT_MOBILErr     ||
             type == OBJECT_MOBILErs     ||
             type == OBJECT_MOBILEsa     ||
             type == OBJECT_MOBILEtg     ||
             type == OBJECT_MOBILEft     ||
             type == OBJECT_MOBILEtt     ||
             type == OBJECT_MOBILEwt     ||
             type == OBJECT_MOBILEit     ||
             type == OBJECT_MOBILEdr     ||
             type == OBJECT_MOTHER       ||
             type == OBJECT_ANT          ||
             type == OBJECT_SPIDER       ||
             type == OBJECT_BEE          ||
             type == OBJECT_WORM         ||
             type == OBJECT_EGG          ||
             type == OBJECT_RUINmobilew1 ||
             type == OBJECT_RUINmobilew2 ||
             type == OBJECT_RUINmobilet1 ||
             type == OBJECT_RUINmobilet2 ||
             type == OBJECT_RUINmobiler1 ||
             type == OBJECT_RUINmobiler2 ||
             type == OBJECT_RUINfactory  ||
             type == OBJECT_RUINdoor     ||
             type == OBJECT_RUINsupport  ||
             type == OBJECT_RUINradar    ||
             type == OBJECT_RUINconvert  ||
             type == OBJECT_RUINbase     ||
             type == OBJECT_RUINhead     ||
             type == OBJECT_APOLLO1      ||
             type == OBJECT_APOLLO2      ||
             type == OBJECT_APOLLO3      ||
             type == OBJECT_APOLLO4      ||
             type == OBJECT_APOLLO5      )
        {
            target = obj;
        }
        else if ((type == OBJECT_POWER ||
                  type == OBJECT_ATOMIC) &&
                 obj->GetTruck() != nullptr)  // battery used?
        {
            target = obj->GetTruck();
        }
        else if (type == OBJECT_POWER ||
                 type == OBJECT_ATOMIC)
        {
            target = obj;
        }

        for (int j = 0; j < OBJECTMAXPART; j++)
        {
            int rank = obj->GetObjectRank(j);
            if (rank == -1) continue;
            if (rank != objRank) continue;
            return target;
        }
    }
    return 0;
}

//! Indicates whether an object is selectable
bool CRobotMain::IsSelectable(CObject* obj)
{
    if (!obj->GetSelectable()) return false;

    ObjectType type = obj->GetType();
    if ( type == OBJECT_HUMAN    ||
         type == OBJECT_TOTO     ||
         type == OBJECT_MOBILEfa ||
         type == OBJECT_MOBILEta ||
         type == OBJECT_MOBILEwa ||
         type == OBJECT_MOBILEia ||
         type == OBJECT_MOBILEfc ||
         type == OBJECT_MOBILEtc ||
         type == OBJECT_MOBILEwc ||
         type == OBJECT_MOBILEic ||
         type == OBJECT_MOBILEfi ||
         type == OBJECT_MOBILEti ||
         type == OBJECT_MOBILEwi ||
         type == OBJECT_MOBILEii ||
         type == OBJECT_MOBILEfs ||
         type == OBJECT_MOBILEts ||
         type == OBJECT_MOBILEws ||
         type == OBJECT_MOBILEis ||
         type == OBJECT_MOBILErt ||
         type == OBJECT_MOBILErc ||
         type == OBJECT_MOBILErr ||
         type == OBJECT_MOBILErs ||
         type == OBJECT_MOBILEsa ||
         type == OBJECT_MOBILEft ||
         type == OBJECT_MOBILEtt ||
         type == OBJECT_MOBILEwt ||
         type == OBJECT_MOBILEit ||
         type == OBJECT_MOBILEdr ||
         type == OBJECT_APOLLO2  ||
         type == OBJECT_BASE     ||
         type == OBJECT_DERRICK  ||
         type == OBJECT_FACTORY  ||
         type == OBJECT_REPAIR   ||
         type == OBJECT_DESTROYER||
         type == OBJECT_STATION  ||
         type == OBJECT_CONVERT  ||
         type == OBJECT_TOWER    ||
         type == OBJECT_RESEARCH ||
         type == OBJECT_RADAR    ||
         type == OBJECT_INFO     ||
         type == OBJECT_ENERGY   ||
         type == OBJECT_LABO     ||
         type == OBJECT_NUCLEAR  ||
         type == OBJECT_PARA     ||
         type == OBJECT_SAFE     ||
         type == OBJECT_HUSTON   )
    {
        return true;
    }

    if (m_selectInsect)
    {
        if ( type == OBJECT_MOTHER   ||
             type == OBJECT_ANT      ||
             type == OBJECT_SPIDER   ||
             type == OBJECT_BEE      ||
             type == OBJECT_WORM     ||
             type == OBJECT_MOBILEtg )
        {
            return true;
        }
    }

    return false;
}


//! Deletes the selected object
bool CRobotMain::DeleteObject()
{
    CObject* obj = GetSelect();
    if (obj == nullptr) return false;

    Gfx::CPyro* pyro = new Gfx::CPyro();
    pyro->Create(Gfx::PT_FRAGT, obj);

    obj->SetSelect(false);  // deselects the object
    m_camera->SetType(Gfx::CAM_TYPE_EXPLO);
    DeselectAll();
    obj->DeleteDeselList(obj);

    return true;
}


//! Removes setting evidence of the object with the mouse hovers over
void CRobotMain::HiliteClear()
{
    ClearTooltip();
    m_tooltipName[0] = 0;  // really removes the tooltip

    if (!m_hilite) return;

    int rank = -1;
    m_engine->SetHighlightRank(&rank);  // nothing more selected

    CInstanceManager* iMan = CInstanceManager::GetInstancePointer();

    for (int i = 0; i < 1000000; i++)
    {
        CObject* obj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, i));
        if (obj == nullptr) break;

        obj->SetHilite(false);
        m_map->SetHighlight(0);
        m_short->SetHighlight(0);
    }

    m_hilite = false;
}

//! Highlights the object with the mouse hovers over
void CRobotMain::HiliteObject(Math::Point pos)
{
    if (m_fixScene && m_phase != PHASE_PERSO) return;
    if (m_movieLock) return;
    if (m_movie->IsExist()) return;
    if (m_app->GetMouseMode() == MOUSE_NONE) return;

    ClearInterface();  // removes setting evidence and tooltip

    CObject* obj = m_short->DetectShort(pos);

    std::string nameStr;
    if (m_dialog->GetTooltip() && m_interface->GetTooltip(pos, nameStr))
    {
        m_tooltipPos = pos;
        strcpy(m_tooltipName, nameStr.c_str());
        m_tooltipTime = 0.0f;
        if (obj == nullptr) return;
    }

    if (m_suspend) return;

    if (obj == nullptr)
    {
        bool inMap = false;
        obj = m_map->DetectMap(pos, inMap);
        if (obj == nullptr)
        {
            if (inMap)  return;

            obj = DetectObject(pos);

            if ((m_camera->GetType() == Gfx::CAM_TYPE_ONBOARD) &&
                (m_camera->GetControllingObject() == obj))
                return;
        }
    }

    char name[100];
    if (obj != nullptr)
    {
        if (m_dialog->GetTooltip() && obj->GetTooltipName(name))
        {
            m_tooltipPos = pos;
            strcpy(m_tooltipName, name);
            m_tooltipTime = 0.0f;
        }

        if (IsSelectable(obj))
        {
            obj->SetHilite(true);
            m_map->SetHighlight(obj);
            m_short->SetHighlight(obj);
            m_hilite = true;
        }
    }
}

//! Highlights the object with the mouse hovers over
void CRobotMain::HiliteFrame(float rTime)
{
    if (m_fixScene && m_phase != PHASE_PERSO) return;
    if (m_movieLock) return;
    if (m_movie->IsExist()) return;

    m_tooltipTime += rTime;

    ClearTooltip();

    if (m_tooltipTime >= 0.2f &&
        m_tooltipName[0] != 0)
    {
        CreateTooltip(m_tooltipPos, m_tooltipName);
    }
}

//! Creates a tooltip
void CRobotMain::CreateTooltip(Math::Point pos, const char* text)
{
    Math::Point corner;
    corner.x = pos.x+0.022f;
    corner.y = pos.y-0.052f;

    Math::Point start, end;

    m_engine->GetText()->SizeText(text, Gfx::FONT_COLOBOT, Gfx::FONT_SIZE_SMALL,
                                  corner, Gfx::TEXT_ALIGN_LEFT,
                                  start, end);

    start.x -= 0.010f;
    start.y -= 0.002f;
    end.x   += 0.010f;
    end.y   += 0.004f;  // ch'tite (?) margin

    pos.x = start.x;
    pos.y = start.y;

    Math::Point dim;
    dim.x = end.x-start.x;
    dim.y = end.y-start.y;

    Math::Point offset;
    offset.x = 0.0f;
    offset.y = 0.0f;
    if (pos.x+dim.x > 1.0f) offset.x = 1.0f-(pos.x+dim.x);
    if (pos.y       < 0.0f) offset.y = -pos.y;

    corner.x += offset.x;
    corner.y += offset.y;
    pos.x += offset.x;
    pos.y += offset.y;

    m_interface->CreateWindows(pos, dim, 1, EVENT_TOOLTIP);

    Ui::CWindow* pw = static_cast<Ui::CWindow*>(m_interface->SearchControl(EVENT_TOOLTIP));
    if (pw != nullptr)
    {
        pw->SetState(Ui::STATE_SHADOW);
        pw->SetTrashEvent(false);

        pos.y -= m_engine->GetText()->GetHeight(Gfx::FONT_COLOBOT, Gfx::FONT_SIZE_SMALL) / 2.0f;
        pw->CreateLabel(pos, dim, -1, EVENT_LABEL2, text);
    }
}

//! Clears the previous tooltip
void CRobotMain::ClearTooltip()
{
    m_interface->DeleteControl(EVENT_TOOLTIP);
}


//! Displays help for an object
void CRobotMain::HelpObject()
{
    CObject* obj = GetSelect();
    if (obj == nullptr) return;

    const char* filename = GetHelpFilename(obj->GetType()).c_str();
    if (filename[0] == 0) return;

    StartDisplayInfo(filename, -1);
}


//! Change the mode of the camera
void CRobotMain::ChangeCamera()
{
    CInstanceManager* iMan = CInstanceManager::GetInstancePointer();

    for (int i = 0; i < 1000000; i++)
    {
        CObject* obj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, i));
        if (obj == nullptr) break;

        if (obj->GetSelect())
        {
            if (obj->GetCameraLock()) return;

            ObjectType oType = obj->GetType();
            Gfx::CameraType type = obj->GetCameraType();

            if ( oType != OBJECT_MOBILEfa &&
                 oType != OBJECT_MOBILEta &&
                 oType != OBJECT_MOBILEwa &&
                 oType != OBJECT_MOBILEia &&
                 oType != OBJECT_MOBILEfc &&
                 oType != OBJECT_MOBILEtc &&
                 oType != OBJECT_MOBILEwc &&
                 oType != OBJECT_MOBILEic &&
                 oType != OBJECT_MOBILEfi &&
                 oType != OBJECT_MOBILEti &&
                 oType != OBJECT_MOBILEwi &&
                 oType != OBJECT_MOBILEii &&
                 oType != OBJECT_MOBILEfs &&
                 oType != OBJECT_MOBILEts &&
                 oType != OBJECT_MOBILEws &&
                 oType != OBJECT_MOBILEis &&
                 oType != OBJECT_MOBILErt &&
                 oType != OBJECT_MOBILErc &&
                 oType != OBJECT_MOBILErr &&
                 oType != OBJECT_MOBILErs &&
                 oType != OBJECT_MOBILEsa &&
                 oType != OBJECT_MOBILEtg &&
                 oType != OBJECT_MOBILEft &&
                 oType != OBJECT_MOBILEtt &&
                 oType != OBJECT_MOBILEwt &&
                 oType != OBJECT_MOBILEit &&
                 oType != OBJECT_MOBILEdr &&
                 oType != OBJECT_APOLLO2  )  return;

            if (oType == OBJECT_MOBILEdr)  // designer?
            {
                     if (type == Gfx::CAM_TYPE_PLANE  )  type = Gfx::CAM_TYPE_BACK;
                else if (type == Gfx::CAM_TYPE_BACK   )  type = Gfx::CAM_TYPE_PLANE;
            }
            else if (obj->GetTrainer())  // trainer?
            {
                     if (type == Gfx::CAM_TYPE_ONBOARD)  type = Gfx::CAM_TYPE_FIX;
                else if (type == Gfx::CAM_TYPE_FIX    )  type = Gfx::CAM_TYPE_PLANE;
                else if (type == Gfx::CAM_TYPE_PLANE  )  type = Gfx::CAM_TYPE_BACK;
                else if (type == Gfx::CAM_TYPE_BACK   )  type = Gfx::CAM_TYPE_ONBOARD;
            }
            else
            {
                     if (type == Gfx::CAM_TYPE_ONBOARD)  type = Gfx::CAM_TYPE_BACK;
                else if (type == Gfx::CAM_TYPE_BACK   )  type = Gfx::CAM_TYPE_ONBOARD;
            }

            obj->SetCameraType(type);
            m_camera->SetType(type);
        }
    }
}

//! Remote control the camera using the arrow keys
void CRobotMain::KeyCamera(EventType type, unsigned int key)
{
    // TODO: rewrite key handling to input bindings

    if (type == EVENT_KEY_UP)
    {
        if (key == GetInputBinding(INPUT_SLOT_LEFT).primary ||
            key == GetInputBinding(INPUT_SLOT_LEFT).secondary)
        {
            m_cameraPan = 0.0f;
        }

        if (key == GetInputBinding(INPUT_SLOT_RIGHT).primary ||
            key == GetInputBinding(INPUT_SLOT_RIGHT).secondary)
        {
            m_cameraPan = 0.0f;
        }

        if (key == GetInputBinding(INPUT_SLOT_UP).primary ||
            key == GetInputBinding(INPUT_SLOT_UP).secondary)
        {
            m_cameraZoom = 0.0f;
        }

        if (key == GetInputBinding(INPUT_SLOT_DOWN).primary ||
            key == GetInputBinding(INPUT_SLOT_DOWN).secondary)
        {
            m_cameraZoom = 0.0f;
        }
    }

    if (m_phase != PHASE_SIMUL) return;
    if (m_editLock) return;  // current edition?
    if (m_trainerPilot) return;

    CObject* obj = GetSelect();
    if (obj == nullptr) return;
    if (!obj->GetTrainer()) return;

    if (type == EVENT_KEY_DOWN)
    {
        if (key == GetInputBinding(INPUT_SLOT_LEFT).primary ||
            key == GetInputBinding(INPUT_SLOT_LEFT).secondary)
        {
            m_cameraPan = -1.0f;
        }

        if (key == GetInputBinding(INPUT_SLOT_RIGHT).primary ||
            key == GetInputBinding(INPUT_SLOT_RIGHT).secondary)
        {
            m_cameraPan = 1.0f;
        }

        if (key == GetInputBinding(INPUT_SLOT_UP).primary ||
            key == GetInputBinding(INPUT_SLOT_UP).secondary)
        {
            m_cameraZoom = -1.0f;
        }

        if (key == GetInputBinding(INPUT_SLOT_DOWN).primary ||
            key == GetInputBinding(INPUT_SLOT_DOWN).secondary)
        {
            m_cameraZoom = 1.0f;
        }
    }
}

//! Panned with the camera if a button is pressed
void CRobotMain::RemoteCamera(float pan, float zoom, float rTime)
{
    if (pan != 0.0f)
    {
        float value = m_camera->GetRemotePan();
        value += pan*rTime*1.5f;
        m_camera->SetRemotePan(value);
    }

    if (zoom != 0.0f)
    {
        float value = m_camera->GetRemoteZoom();
        value += zoom*rTime*0.3f;
        m_camera->SetRemoteZoom(value);
    }
}



//! Cancels the current movie
void CRobotMain::AbortMovie()
{
    CInstanceManager* iMan = CInstanceManager::GetInstancePointer();

    for (int i = 0; i < 1000000; i++)
    {
        CObject* obj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, i));
        if (obj == nullptr) break;

        CAuto* automat = obj->GetAuto();
        if (automat != 0)
            automat->Abort();
    }

    m_app->SetMouseMode(MOUSE_ENGINE);
}



//! Updates the text information
void CRobotMain::UpdateInfoText()
{
    if (m_showPos)
    {
        CObject* obj = GetSelect();
        if (obj != nullptr)
        {
            Math::Vector pos = obj->GetPosition(0);
            char info[100];
            sprintf(info, "Pos = %.2f ; %.2f", pos.x/g_unit, pos.z/g_unit);
            //TODO: m_engine->SetInfoText(4, info);
        }
    }
}


//! Initializes the view
void CRobotMain::InitEye()
{
    if (m_phase == PHASE_SIMUL)
        m_camera->Init(Math::Vector( 0.0f, 10.0f, 0.0f),
                       Math::Vector(10.0f,  5.0f, 0.0f), 0.0f);
}

//! Advances the entire scene
bool CRobotMain::EventFrame(const Event &event)
{
    m_time += event.rTime;
    if (!m_movieLock) m_gameTime += event.rTime;

    if (!m_immediatSatCom && !m_beginSatCom &&
         m_gameTime > 0.1f && m_phase == PHASE_SIMUL)
    {
        m_displayText->DisplayError(INFO_BEGINSATCOM, Math::Vector(0.0f,0.0f,0.0f));
        m_beginSatCom = true;  // message appears
    }

    m_water->EventProcess(event);
    m_cloud->EventProcess(event);
    m_lightning->EventProcess(event);
    m_planet->EventProcess(event);

    Ui::CMap* pm = nullptr;
    Ui::CWindow* pw = static_cast<Ui::CWindow*>(m_interface->SearchControl(EVENT_WINDOW1));
    if (pw == nullptr)
    {
        pm = nullptr;
    }
    else
    {
        pm = static_cast<Ui::CMap*>(pw->SearchControl(EVENT_OBJECT_MAP));
        if (pm != nullptr) pm->FlushObject();
    }

    CInstanceManager* iMan = CInstanceManager::GetInstancePointer();

    CObject* toto = nullptr;
    if (!m_freePhoto)
    {
        // Advances all the robots, but not toto.
        for (int i = 0; i < 1000000; i++)
        {
            CObject* obj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, i));
            if (obj == nullptr) break;
            if (pm != nullptr) pm->UpdateObject(obj);
            if (obj->GetTruck() != nullptr)  continue;
            ObjectType type = obj->GetType();
            if (type == OBJECT_TOTO)
                toto = obj;
            else
                obj->EventProcess(event);
        }
        // Advances all objects transported by robots.
        for (int i = 0; i < 1000000; i++)
        {
            CObject* obj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, i));
            if (obj == nullptr) break;
            if (obj->GetTruck() == nullptr) continue;
            obj->EventProcess(event);
        }

        // Advances pyrotechnic effects.
        for (int i = 0; i < 1000000; i++)
        {
            Gfx::CPyro* pyro = static_cast<Gfx::CPyro*>(iMan->SearchInstance(CLASS_PYRO, i));
            if (pyro == nullptr) break;

            pyro->EventProcess(event);
            if (pyro->IsEnded() != ERR_CONTINUE)
            {
                pyro->DeleteObject();
                delete pyro;
            }
        }
    }

    // The camera follows the object, because its position
    // may depend on the selected object (Gfx::CAM_TYPE_ONBOARD or Gfx::CAM_TYPE_BACK).
    if (m_phase == PHASE_SIMUL && !m_editFull)
    {
        m_camera->EventProcess(event);

        if (m_engine->GetFog())
            m_camera->SetOverBaseColor(m_particle->GetFogColor(m_engine->GetEyePt()));
    }
    if (m_phase == PHASE_PERSO ||
        m_phase == PHASE_WIN   ||
        m_phase == PHASE_LOST)
    {
        m_camera->EventProcess(event);
    }

    // Advances toto following the camera, because its position depends on the camera.
    if (toto != nullptr)
        toto->EventProcess(event);

    HiliteFrame(event.rTime);

    // Moves the film indicator.
    if (m_movieLock && !m_editLock)  // movie in progress?
    {
        Ui::CControl* pc = m_interface->SearchControl(EVENT_OBJECT_MOVIELOCK);
        if (pc != nullptr)
        {
            Math::Point pos, dim;

            dim.x = 32.0f/640.0f;
            dim.y = 32.0f/480.0f;
            pos.x = 20.0f/640.0f;
            pos.y = (480.0f-24.0f)/480.0f;

            float zoom = 1.0f+sinf(m_time*6.0f)*0.1f;  // 0.9 .. 1.1
            dim.x *= zoom;
            dim.y *= zoom;
            pos.x -= dim.x/2.0f;
            pos.y -= dim.y/2.0f;

            pc->SetPos(pos);
            pc->SetDim(dim);
        }
    }

    // Moves edition indicator.
    if (m_editLock || m_pause)  // edition in progress?
    {
        Ui::CControl* pc = m_interface->SearchControl(EVENT_OBJECT_EDITLOCK);
        if (pc != nullptr)
        {
            Math::Point pos, dim;

            if (m_editFull || m_editLock)
            {
                dim.x = 10.0f/640.0f;
                dim.y = 10.0f/480.0f;
                pos.x = -20.0f/640.0f;
                pos.y = -20.0f/480.0f;  // invisible!
            }
            else
            {
                dim.x = 32.0f/640.0f;
                dim.y = 32.0f/480.0f;
                pos.x = 20.0f/640.0f;
                pos.y = (480.0f-24.0f)/480.0f;

                float zoom = 1.0f+sinf(m_time*6.0f)*0.1f;  // 0.9 .. 1.1
                dim.x *= zoom;
                dim.y *= zoom;
                pos.x -= dim.x/2.0f;
                pos.y -= dim.y/2.0f;
            }
            pc->SetPos(pos);
            pc->SetDim(dim);
        }
    }

    // Will move the arrow to visit.
    if (m_camera->GetType() == Gfx::CAM_TYPE_VISIT)
        FrameVisit(event.rTime);

    // Moves the boundaries.
    FrameShowLimit(event.rTime);

    if (m_phase == PHASE_SIMUL)
    {
        if (!m_editLock && m_checkEndTime+1.0f < m_time)
        {
            m_checkEndTime = m_time;
            CheckEndMission(true);
        }

        if (m_winDelay > 0.0f && !m_editLock)
        {
            m_winDelay -= event.rTime;
            if (m_winDelay <= 0.0f)
            {
                if (m_movieLock)
                    m_winDelay = 1.0f;
                else
                    m_eventQueue->AddEvent(Event(EVENT_WIN));
            }
        }

        if (m_lostDelay > 0.0f && !m_editLock)
        {
            m_lostDelay -= event.rTime;
            if (m_lostDelay <= 0.0f)
            {
                if (m_movieLock)
                    m_winDelay = 1.0f;
                else
                    m_eventQueue->AddEvent(Event(EVENT_LOST));
            }
        }
    }

    if (m_delayWriteMessage > 0)
    {
        m_delayWriteMessage --;
        if (m_delayWriteMessage == 0)
        {
            m_displayText->DisplayError(INFO_WRITEOK, Math::Vector(0.0f,0.0f,0.0f));
        }
    }

    return true;
}

//! Makes the event for all robots
bool CRobotMain::EventObject(const Event &event)
{
    if (m_freePhoto) return true;

    m_resetCreate = false;

    CInstanceManager* iMan = CInstanceManager::GetInstancePointer();

    for (int i = 0; i < 1000000; i++)
    {
        CObject* obj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, i));
        if (obj == nullptr) break;

        obj->EventProcess(event);
    }

    if (m_resetCreate)
        ResetCreate();

    return true;
}


//! Calculates the point of arrival of the camera
Math::Vector CRobotMain::LookatPoint(Math::Vector eye, float angleH, float angleV,
                                  float length)
{
    Math::Vector lookat = eye;
    lookat.z += length;

    RotatePoint(eye, angleH, angleV, lookat);
    return lookat;
}



char* SkipNum(char *p)
{
    while (*p == ' ' || *p == '.' || *p == '-' || (*p >= '0' && *p <= '9'))
    {
        p++;
    }
    return p;
}

//! Conversion of units
void CRobotMain::Convert()
{
    char* base = m_dialog->GetSceneName();
    int rank = m_dialog->GetSceneRank();

    //TODO change line to string
    char line[500];
    std::string tempLine;

    m_dialog->BuildSceneName(tempLine, base, rank);
    strcpy(line, tempLine.c_str());
    FILE* file = fopen(line, "r");
    if (file == NULL) return;

    strcpy(line+strlen(line)-4, ".new");
    FILE* fileNew = fopen(line, "w");
    if (fileNew == NULL) return;

    char lineNew[500];
    char s[200];

    while (fgets(line, 500, file) != NULL)
    {
        strcpy(lineNew, line);

        if (Cmd(line, "DeepView"))
        {
            char* p = strstr(line, "air=");
            if (p != 0)
            {
                float value = OpFloat(line, "air", 500.0f);
                value /= g_unit;
                p[0] = 0;
                p = SkipNum(p+4);
                strcpy(lineNew, line);
                strcat(lineNew, "air=");
                sprintf(s, "%.2f", value);
                strcat(lineNew, s);
                strcat(lineNew, " ");
                strcat(lineNew, p);
            }
            strcpy(line, lineNew);

            p = strstr(line, "water=");
            if (p != 0)
            {
                float value = OpFloat(line, "water", 100.0f);
                value /= g_unit;
                p[0] = 0;
                p = SkipNum(p+6);
                strcpy(lineNew, line);
                strcat(lineNew, "water=");
                sprintf(s, "%.2f", value);
                strcat(lineNew, s);
                strcat(lineNew, " ");
                strcat(lineNew, p);
            }
            strcpy(line, lineNew);
        }

        if (Cmd(line, "TerrainGenerate"))
        {
            char* p = strstr(line, "vision=");
            if (p != 0)
            {
                float value = OpFloat(line, "vision", 500.0f);
                value /= g_unit;
                p[0] = 0;
                p = SkipNum(p+7);
                strcpy(lineNew, line);
                strcat(lineNew, "vision=");
                sprintf(s, "%.2f", value);
                strcat(lineNew, s);
                strcat(lineNew, " ");
                strcat(lineNew, p);
            }
        }

        if (Cmd(line, "CreateObject") ||
            Cmd(line, "CreateSpot"))
        {
            char* p = strstr(line, "pos=");
            if (p != 0)
            {
                Math::Vector pos = OpPos(line, "pos");
                pos.x /= g_unit;
                pos.y /= g_unit;
                pos.z /= g_unit;
                p[0] = 0;
                p = SkipNum(p+4);
                p = SkipNum(p+1);
                strcpy(lineNew, line);
                strcat(lineNew, "pos=");
                sprintf(s, "%.2f", pos.x);
                strcat(lineNew, s);
                strcat(lineNew, ";");
                sprintf(s, "%.2f", pos.z);
                strcat(lineNew, s);
                strcat(lineNew, " ");
                strcat(lineNew, p);
            }
        }

        if (Cmd(line, "EndMissionTake"))
        {
            char* p = strstr(line, "pos=");
            if (p != 0)
            {
                Math::Vector pos = OpPos(line, "pos");
                pos.x /= g_unit;
                pos.y /= g_unit;
                pos.z /= g_unit;
                p[0] = 0;
                p = SkipNum(p+4);
                p = SkipNum(p+1);
                strcpy(lineNew, line);
                strcat(lineNew, "pos=");
                sprintf(s, "%.2f", pos.x);
                strcat(lineNew, s);
                strcat(lineNew, ";");
                sprintf(s, "%.2f", pos.z);
                strcat(lineNew, s);
                strcat(lineNew, " ");
                strcat(lineNew, p);
            }
            strcpy(line, lineNew);

            p = strstr(line, "dist=");
            if (p != 0)
            {
                float value = OpFloat(line, "dist", 32.0f);
                value /= g_unit;
                p[0] = 0;
                p = SkipNum(p+5);
                strcpy(lineNew, line);
                strcat(lineNew, "dist=");
                sprintf(s, "%.2f", value);
                strcat(lineNew, s);
                strcat(lineNew, " ");
                strcat(lineNew, p);
            }
            strcpy(line, lineNew);
        }

        if (Cmd(line, "Camera"))
        {
            char* p = strstr(line, "pos=");
            if (p != 0)
            {
                Math::Vector pos = OpPos(line, "pos");
                pos.x /= g_unit;
                pos.y /= g_unit;
                pos.z /= g_unit;
                p[0] = 0;
                p = SkipNum(p+4);
                p = SkipNum(p+1);
                strcpy(lineNew, line);
                strcat(lineNew, "pos=");
                sprintf(s, "%.2f", pos.x);
                strcat(lineNew, s);
                strcat(lineNew, ";");
                sprintf(s, "%.2f", pos.z);
                strcat(lineNew, s);
                strcat(lineNew, " ");
                strcat(lineNew, p);
            }
            strcpy(line, lineNew);

            p = strstr(line, "h=");
            if (p != 0)
            {
                float value = OpFloat(line, "h", 32.0f);
                value /= g_unit;
                p[0] = 0;
                p = SkipNum(p+2);
                strcpy(lineNew, line);
                strcat(lineNew, "h=");
                sprintf(s, "%.2f", value);
                strcat(lineNew, s);
                strcat(lineNew, " ");
                strcat(lineNew, p);
            }
            strcpy(line, lineNew);
        }

        fputs(lineNew, fileNew);
    }

    fclose(fileNew);
    fclose(file);
}

//! Load the scene for the character
void CRobotMain::ScenePerso()
{
    DeleteAllObjects();  // removes all the current 3D Scene
    m_engine->DeleteAllObjects();
    Gfx::CModelManager::GetInstancePointer()->DeleteAllModelCopies();
    m_terrain->FlushRelief();  // all flat
    m_terrain->FlushBuildingLevel();
    m_terrain->FlushFlyingLimit();
    m_lightMan->FlushLights();
    m_particle->FlushParticle();

    CInstanceManager* iMan = CInstanceManager::GetInstancePointer();
    iMan->Flush(CLASS_OBJECT);
    iMan->Flush(CLASS_PHYSICS);
    iMan->Flush(CLASS_BRAIN);
    iMan->Flush(CLASS_PYRO);

    m_dialog->SetSceneName("perso");
    m_dialog->SetSceneRank(0);
    CreateScene(false, true, false);  // sets scene

    m_engine->SetDrawWorld(false);  // does not draw anything on the interface
    m_engine->SetDrawFront(true);  // draws on the human interface
    CObject* obj = SearchHuman();
    if (obj != nullptr)
    {
        obj->SetDrawFront(true);  // draws the interface

        CMotionHuman* mh = static_cast<CMotionHuman*>(obj->GetMotion());
        if (mh != nullptr)
            mh->StartDisplayPerso();
    }
}

//! Creates the whole scene
void CRobotMain::CreateScene(bool soluce, bool fixScene, bool resetObject)
{
    char*       base  = m_dialog->GetSceneName();
    int         rank  = m_dialog->GetSceneRank();
    const char* read  = m_dialog->GetSceneRead().c_str();
    const char* stack = m_dialog->GetStackRead().c_str();
    m_dialog->SetUserDir(base, rank);

    /*
     * TODO: original code relying on UserDir() was removed.
     * A new way of providing custom data file paths will need to be devised.
     */

    m_fixScene = fixScene;

    g_id = 0;
    m_base = false;

    if (!resetObject)
    {
        g_build = 0;
        g_researchDone = 0;  // no research done
        g_researchEnable = 0;

        FlushDisplayInfo();
        m_terrain->FlushMaterials();
        m_audioTrack = 0;
        m_audioRepeat = true;
        m_displayText->SetDelay(1.0f);
        m_displayText->SetEnable(true);
        m_immediatSatCom = false;
        m_lockedSatCom = false;
        m_endingWinRank   = 0;
        m_endingLostRank  = 0;
        m_endTakeTotal = 0;
        m_endTakeResearch = 0;
        m_endTakeWinDelay = 2.0f;
        m_endTakeLostDelay = 2.0f;
        m_obligatoryTotal = 0;
        m_prohibitedTotal = 0;
        m_mapShow = true;
        m_mapImage = false;
        m_mapFilename[0] = 0;

        m_colorRefBot.r =  10.0f/256.0f;
        m_colorRefBot.g = 166.0f/256.0f;
        m_colorRefBot.b = 254.0f/256.0f;  // blue
        m_colorRefBot.a = 0.0f;
        m_colorNewBot = m_colorRefBot;

        m_colorRefAlien.r = 135.0f/256.0f;
        m_colorRefAlien.g = 170.0f/256.0f;
        m_colorRefAlien.b =  13.0f/256.0f;  // green
        m_colorRefAlien.a = 0.0f;
        m_colorNewAlien = m_colorRefAlien;

        m_colorRefGreen.r = 135.0f/256.0f;
        m_colorRefGreen.g = 170.0f/256.0f;
        m_colorRefGreen.b =  13.0f/256.0f;  // green
        m_colorRefGreen.a = 0.0f;
        m_colorNewGreen = m_colorRefGreen;

        m_colorRefWater.r =  25.0f/256.0f;
        m_colorRefWater.g = 255.0f/256.0f;
        m_colorRefWater.b = 240.0f/256.0f;  // cyan
        m_colorRefWater.a = 0.0f;
        m_colorNewWater = m_colorRefWater;

        m_dialog->BuildResumeName(m_title, base, rank);
        m_dialog->BuildResumeName(m_resume, base, rank);
        GetResource(RES_TEXT, RT_SCRIPT_NEW, m_scriptName);
        m_scriptFile[0] = 0;

        m_beginObject         = false;
        m_terrainGenerate     = false;
        m_terrainInit         = false;
        m_terrainInitTextures = false;
        m_terrainCreate       = false;

        m_version             = 1;
        m_retroStyle          = false;
    }

    char line[500];
    char name[200];
    char dir[100];
    char op[100];
    char filename[500];
    int lineNum = 0;

    memset(line, 0, 500);
    memset(name, 0, 200);
    memset(dir, 0, 100);
    memset(op, 0, 100);
    memset(filename, 0, 500);
    std::string tempLine;
    m_dialog->BuildSceneName(tempLine, base, rank);
    strcpy(filename, tempLine.c_str());
    FILE* file = fopen(filename, "r");
    if (file == NULL) return;

    int rankObj = 0;
    int rankGadget = 0;
    CObject* sel = 0;

    SetNumericLocale();

    while (fgets(line, 500, file) != NULL)
    {
        lineNum++;
        for (int i = 0; i < 500; i++)
        {
            if (line[i] == '\t' ) line[i] = ' ';  // replace tab by space
            if (line[i] == '/' && line[i+1] == '/')
            {
                line[i] = 0;
                break;
            }
        }

        if (Cmd(line, "MissionFile") && !resetObject)
           m_version = OpInt(line, "version", 1);

        // TODO: Fallback to an non-localized entry
        sprintf(op, "Title.%c", m_app->GetLanguageChar());
        if (Cmd(line, op) && !resetObject)
            OpString(line, "text", m_title);

        sprintf(op, "Resume.%c", m_app->GetLanguageChar());
        if (Cmd(line, op) && !resetObject)
            OpString(line, "text", m_resume);

        sprintf(op, "ScriptName.%c", m_app->GetLanguageChar());
        if (Cmd(line, op) && !resetObject)
            OpString(line, "text", m_scriptName);

        if (Cmd(line, "ScriptFile") && !resetObject)
            OpString(line, "name", m_scriptFile);

        if (Cmd(line, "Instructions") && !resetObject)
        {
            OpString(line, "name", name);
            std::string path = m_app->GetDataFilePath(DIR_HELP, name);
            strcpy(m_infoFilename[SATCOM_HUSTON], path.c_str());

            m_immediatSatCom = OpInt(line, "immediat", 0);
            if(m_version >= 2) m_beginSatCom = m_lockedSatCom = OpInt(line, "lock", 0);
        }

        if (Cmd(line, "Satellite") && !resetObject)
        {
            OpString(line, "name", name);
            std::string path = m_app->GetDataFilePath(DIR_HELP, name);
            strcpy(m_infoFilename[SATCOM_SAT], path.c_str());
        }

        if (Cmd(line, "Loading") && !resetObject)
        {
            OpString(line, "name", name);
            std::string path = m_app->GetDataFilePath(DIR_HELP, name);
            strcpy(m_infoFilename[SATCOM_LOADING], path.c_str());
        }

        if (Cmd(line, "HelpFile") && !resetObject)
        {
            OpString(line, "name", name);
            std::string path = m_app->GetDataFilePath(DIR_HELP, name);
            strcpy(m_infoFilename[SATCOM_PROG], path.c_str());
        }
        if (Cmd(line, "SoluceFile") && !resetObject)
        {
            OpString(line, "name", name);
            std::string path = m_app->GetDataFilePath(DIR_HELP, name);
            strcpy(m_infoFilename[SATCOM_SOLUCE], path.c_str());
        }

        if (Cmd(line, "EndingFile") && !resetObject)
        {
            m_endingWinRank  = OpInt(line, "win",  0);
            m_endingLostRank = OpInt(line, "lost", 0);
        }

        if (Cmd(line, "MessageDelay") && !resetObject)
        {
            m_displayText->SetDelay(OpFloat(line, "factor", 1.0f));
        }

        if (Cmd(line, "Audio") && !resetObject)
        {
            m_audioTrack = OpInt(line, "track", 0);
            m_audioRepeat = OpInt(line, "repeat", 1);
        }

        if (Cmd(line, "AmbientColor") && !resetObject)
        {
            m_engine->SetAmbientColor(OpColor(line, "air",   Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f)), 0);
            m_engine->SetAmbientColor(OpColor(line, "water", Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f)), 1);
        }

        if (Cmd(line, "FogColor") && !resetObject)
        {
            m_engine->SetFogColor(OpColor(line, "air",   Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f)), 0);
            m_engine->SetFogColor(OpColor(line, "water", Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f)), 1);
        }

        if (Cmd(line, "VehicleColor") && !resetObject)
            m_colorNewBot = OpColor(line, "color", Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f));

        if (Cmd(line, "InsectColor") && !resetObject)
            m_colorNewAlien = OpColor(line, "color", Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f));

        if (Cmd(line, "GreeneryColor") && !resetObject)
            m_colorNewGreen = OpColor(line, "color", Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f));

        if (Cmd(line, "DeepView") && !resetObject)
        {
            m_engine->SetDeepView(OpFloat(line, "air",   500.0f)*g_unit, 0, true);
            m_engine->SetDeepView(OpFloat(line, "water", 100.0f)*g_unit, 1, true);
        }

        if (Cmd(line, "FogStart") && !resetObject)
        {
            m_engine->SetFogStart(OpFloat(line, "air",   0.5f), 0);
            m_engine->SetFogStart(OpFloat(line, "water", 0.5f), 1);
        }

        if (Cmd(line, "SecondTexture") && !resetObject)
            m_engine->SetSecondTexture(OpInt(line, "rank", 1));

        if (Cmd(line, "Background") && !resetObject)
        {
            OpString(line, "image", name);
            m_engine->SetBackground(name,
                                    OpColor(line, "up",        Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f)),
                                    OpColor(line, "down",      Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f)),
                                    OpColor(line, "cloudUp",   Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f)),
                                    OpColor(line, "cloudDown", Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f)),
                                    OpInt(line, "full", 0));
        }

        if (Cmd(line, "Planet") && !resetObject)
        {
            Math::Vector    ppos, uv1, uv2;

            ppos  = OpPos(line, "pos");
            uv1   = OpPos(line, "uv1");
            uv2   = OpPos(line, "uv2");
            OpString(line, "image", name);
            m_planet->Create(OpInt(line, "mode", 0),
                             Math::Point(ppos.x, ppos.z),
                             OpFloat(line, "dim", 0.2f),
                             OpFloat(line, "speed", 0.0f),
                             OpFloat(line, "dir", 0.0f),
                             name,
                             Math::Point(uv1.x, uv1.z),
                             Math::Point(uv2.x, uv2.z),
                             strstr(name, "planet") != nullptr // TODO: add transparent op or modify textures
                            );
        }

        if (Cmd(line, "ForegroundName") && !resetObject)
        {
            OpString(line, "image", name);
            m_engine->SetForegroundName(name);
        }

        if (((m_version == 1 && Cmd(line, "Global")) || (m_version >= 2 && Cmd(line, "Mission"))) && !resetObject)
        {
            g_unit = OpFloat(line, "unitScale", 4.0f);
            m_engine->SetTracePrecision(OpFloat(line, "traceQuality", 1.0f));
            m_shortCut = OpInt(line, "shortcut", 1);
            if(m_version >= 2) {
                m_retroStyle = OpInt(line, "retro", 0);
                if(m_retroStyle) GetLogger()->Info("Retro mode enabled.\n");
            }
        }

        if (Cmd(line, "TerrainGenerate") && !resetObject)
        {
            if(m_terrainCreate) {
                GetLogger()->Error("Syntax error in file '%s' (line %d): TerrainGenerate after TerrainCreate\n", filename, lineNum);
                continue;
            }

            if(m_terrainInit) {
                GetLogger()->Error("Syntax error in file '%s' (line %d): TerrainGenerate after TerrainInit\n", filename, lineNum);
                continue;
            }

            m_terrain->Generate(OpInt(line, "mosaic", 20),
                                OpInt(line, "brick", 3),
                                OpFloat(line, "size", 20.0f),
                                OpFloat(line, "vision", 500.0f)*g_unit,
                                OpInt(line, "depth", 2),
                                OpFloat(line, "hard", 0.5f));

            m_terrainGenerate = true;
        }

        if (Cmd(line, "TerrainWind") && !resetObject) {
            if(m_terrainCreate) {
                GetLogger()->Error("Syntax error in file '%s' (line %d): TerrainWind after TerrainCreate\n", filename, lineNum);
                continue;
            }

            if(m_terrainInit) {
                GetLogger()->Error("Syntax error in file '%s' (line %d): TerrainWind after TerrainInit\n", filename, lineNum);
                continue;
            }

            if(!m_terrainGenerate) {
                GetLogger()->Error("Syntax error in file '%s' (line %d): TerrainWind before TerrainGenerate\n", filename, lineNum);
                continue;
            }

            m_terrain->SetWind(OpPos(line, "speed"));
        }

        if (Cmd(line, "TerrainRelief") && !resetObject)
        {
            if(m_terrainCreate) {
                GetLogger()->Error("Syntax error in file '%s' (line %d): TerrainRelief after TerrainCreate\n", filename, lineNum);
                continue;
            }

            if(m_terrainInit) {
                GetLogger()->Error("Syntax error in file '%s' (line %d): TerrainRelief after TerrainInit\n", filename, lineNum);
                continue;
            }

            if(!m_terrainGenerate) {
                GetLogger()->Error("Syntax error in file '%s' (line %d): TerrainRelief before TerrainGenerate\n", filename, lineNum);
                continue;
            }

            OpString(line, "image", name);
            m_terrain->LoadRelief(name, OpFloat(line, "factor", 1.0f), OpInt(line, "border", 1));
        }

        if (Cmd(line, "TerrainResource") && !resetObject)
        {
            if(m_terrainCreate) {
                GetLogger()->Error("Syntax error in file '%s' (line %d): TerrainResource after TerrainCreate\n", filename, lineNum);
                continue;
            }

            if(m_terrainInit) {
                GetLogger()->Error("Syntax error in file '%s' (line %d): TerrainResource after TerrainInit\n", filename, lineNum);
                continue;
            }

            if(!m_terrainGenerate) {
                GetLogger()->Error("Syntax error in file '%s' (line %d): TerrainResource before TerrainGenerate\n", filename, lineNum);
                continue;
            }

            OpString(line, "image", name);
            m_terrain->LoadResources(name);
        }

        if (Cmd(line, "TerrainWater") && !resetObject)
        {
            OpString(line, "image", name);
            Math::Vector pos;
            pos.x = OpFloat(line, "moveX", 0.0f);
            pos.y = OpFloat(line, "moveY", 0.0f);
            pos.z = pos.x;
            m_water->Create(OpTypeWater(line, "air",   Gfx::WATER_TT),
                            OpTypeWater(line, "water", Gfx::WATER_TT),
                            name,
                            OpColor(line, "diffuse", Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f)),
                            OpColor(line, "ambient", Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f)),
                            OpFloat(line, "level", 100.0f)*g_unit,
                            OpFloat(line, "glint", 1.0f),
                            pos);
            m_colorNewWater = OpColor(line, "color", m_colorRefWater);
            m_colorShiftWater = OpFloat(line, "brightness", 0.0f);
        }

        if (Cmd(line, "TerrainLava") && !resetObject)
            m_water->SetLava(OpInt(line, "mode", 0));

        if (Cmd(line, "TerrainCloud") && !resetObject)
        {
            OpString(line, "image", name);
            m_cloud->Create(name,
                            OpColor(line, "diffuse", Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f)),
                            OpColor(line, "ambient", Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f)),
                            OpFloat(line, "level", 500.0f) * g_unit);
        }

        if (Cmd(line, "TerrainBlitz") && !resetObject)
        {
            m_lightning->Create(OpFloat(line, "sleep", 0.0f),
                            OpFloat(line, "delay", 3.0f),
                            OpFloat(line, "magnetic", 50.0f) * g_unit);
        }

        if (Cmd(line, "TerrainInitTextures") && !resetObject)
        {
            if(m_terrainInit) {
                GetLogger()->Error("Syntax error in file '%s' (line %d): TerrainInitTextures and TerrainInit at same time\n", filename, lineNum);
                continue;
            }

            OpString(line, "image", name);
            AddExt(name, ".png");
            int dx = OpInt(line, "dx", 1);
            int dy = OpInt(line, "dy", 1);
            char* op = SearchOp(line, "table");
            int tt[100];
            for (int i = 0; i < dx*dy; i++)
                tt[i] = GetInt(op, i, 0);

            if (strstr(name, "%user%") != 0)
                CopyFileListToTemp(name, tt, dx*dy);

            m_terrain->InitTextures(name, tt, dx, dy);

            m_terrainInitTextures = true;
        }

        if (Cmd(line, "TerrainInit") && !resetObject) {
            if(m_terrainInitTextures) {
                GetLogger()->Error("Syntax error in file '%s' (line %d): TerrainInit and TerrainInitTextures at same time\n", filename, lineNum);
                continue;
            }

            m_terrain->InitMaterials(OpInt(line, "id", 1));
            m_terrainInit = true;
        }

        if (Cmd(line, "TerrainMaterial") && !resetObject)
        {
            if(m_terrainCreate) {
                GetLogger()->Error("Syntax error in file '%s' (line %d): TerrainMaterial after TerrainCreate\n", filename, lineNum);
                continue;
            }

            if(m_terrainInit) {
                GetLogger()->Error("Syntax error in file '%s' (line %d): TerrainMaterial after TerrainInit\n", filename, lineNum);
                continue;
            }

            if(m_terrainInitTextures) {
                GetLogger()->Error("Syntax error in file '%s' (line %d): TerrainMaterial and TerrainInitTextures at same time\n", filename, lineNum);
                continue;
            }

            OpString(line, "image", name);
            AddExt(name, ".png");
            if (strstr(name, "%user%") != 0) {
                GetProfile().CopyFileToTemp(std::string(name));
            }

            m_terrain->AddMaterial(OpInt(line, "id", 0),
                                   name,
                                   Math::Point(OpFloat(line, "u", 0.0f),
                                               OpFloat(line, "v", 0.0f)),
                                   OpInt(line, "up",    1),
                                   OpInt(line, "right", 1),
                                   OpInt(line, "down",  1),
                                   OpInt(line, "left",  1),
                                   OpFloat(line, "hard", 0.5f));
        }

        if (Cmd(line, "TerrainLevel") && !resetObject)
        {
            if(m_terrainCreate) {
                GetLogger()->Error("Syntax error in file '%s' (line %d): TerrainLevel after TerrainCreate\n", filename, lineNum);
                continue;
            }

            if(!m_terrainInit) {
                GetLogger()->Error("Syntax error in file '%s' (line %d): TerrainLevel before TerrainInit\n", filename, lineNum);
                continue;
            }

            if(m_terrainInitTextures) {
                GetLogger()->Error("Syntax error in file '%s' (line %d): TerrainLevel and TerrainInitTextures at same time\n", filename, lineNum);
                continue;
            }

            if(!m_terrainGenerate) {
                GetLogger()->Error("Syntax error in file '%s' (line %d): TerrainLevel before TerrainGenerate\n", filename, lineNum);
                continue;
            }

            char* op = SearchOp(line, "id");
            int id[50];
            int i = 0;
            while (i < 50)
            {
                id[i] = GetInt(op, i, 0);
                if (id[i++] == 0) break;
            }

            m_terrain->GenerateMaterials(id,
                                         OpFloat(line, "min", 0.0f)*g_unit,
                                         OpFloat(line, "max", 100.0f)*g_unit,
                                         OpFloat(line, "slope", 5.0f),
                                         OpFloat(line, "freq", 100.0f),
                                         OpPos(line, "center")*g_unit,
                                         OpFloat(line, "radius", 0.0f)*g_unit);
        }

        if (Cmd(line, "TerrainCreate") && !resetObject) {
            m_terrain->CreateObjects();
            m_terrainCreate = true;
        }

        if (Cmd(line, "BeginObject"))
        {
            InitEye();
            SetMovieLock(false);

            if (read[0] != 0)  // loading file ?
                sel = IOReadScene(read, stack);

            m_beginObject = true;
        }

        if (Cmd(line, "CreateObject") && read[0] == 0)
        {
            if (!m_beginObject) {
                GetLogger()->Error("Syntax error in file '%s' (line %d): CreateObject before BeginObject\n", filename, lineNum);
                continue;
            }

            ObjectType type = OpTypeObject(line, "type", OBJECT_NULL);

            int gadget = OpInt(line, "gadget", -1);
            if ( gadget == -1 )
            {
                gadget = 0;
                if ( type == OBJECT_TECH ||
                     (type >= OBJECT_PLANT0  &&
                      type <= OBJECT_PLANT19 ) ||
                     (type >= OBJECT_TREE0   &&
                      type <= OBJECT_TREE9   ) ||
                     (type >= OBJECT_TEEN0   &&
                      type <= OBJECT_TEEN49  ) ||
                     (type >= OBJECT_QUARTZ0 &&
                      type <= OBJECT_QUARTZ9 ) ||
                     (type >= OBJECT_ROOT0   &&
                      type <= OBJECT_ROOT4   ) )  // not ROOT5!
                {
                    if ( type != OBJECT_TEEN11 &&  // lamp?
                         type != OBJECT_TEEN12 &&  // coke?
                         type != OBJECT_TEEN20 &&  // wall?
                         type != OBJECT_TEEN21 &&  // wall?
                         type != OBJECT_TEEN22 &&  // wall?
                         type != OBJECT_TEEN26 &&  // lamp?
                         type != OBJECT_TEEN28 &&  // bottle?
                         type != OBJECT_TEEN34 )   // stone?
                    {
                        gadget = 1;
                    }
                }
            }
            if (gadget != 0)  // is this a gadget?
            {
                if (!TestGadgetQuantity(rankGadget++)) continue;
            }

            Math::Vector pos = OpPos(line, "pos")*g_unit;
            float dir = OpFloat(line, "dir", 0.0f)*Math::PI;
            bool trainer = OpInt(line, "trainer", 0);
            CObject* obj = CreateObject(pos, dir,
                                        OpFloat(line, "z", 1.0f),
                                        OpFloat(line, "h", 0.0f),
                                        type,
                                        OpFloat(line, "power", 1.0f),
                                        trainer,
                                        OpInt(line, "toy", 0),
                                        OpInt(line, "option", 0));

            if (obj != nullptr)
            {
                obj->SetDefRank(rankObj);

                if (type == OBJECT_BASE) m_base = true;

                Gfx::CameraType cType = OpCamera(line, "camera");
                if (cType != Gfx::CAM_TYPE_NULL)
                    obj->SetCameraType(cType);

                obj->SetCameraDist(OpFloat(line, "cameraDist", 50.0f));
                obj->SetCameraLock(OpInt(line, "cameraLock", 0));

                Gfx::PyroType pType = OpPyro(line, "pyro");
                if (pType != Gfx::PT_NULL)
                {
                    Gfx::CPyro* pyro = new Gfx::CPyro();
                    pyro->Create(pType, obj);
                }

                // Puts information in terminal (OBJECT_INFO).
                for (int i = 0; i < OBJECTMAXINFO; i++)
                {
                    sprintf(op, "info%d", i+1);
                    char text[100];
                    OpString(line, op, text);
                    if (text[0] == 0)  break;
                    char* p = strchr(text, '=');
                    if (p == 0) break;
                    *p = 0;
                    Info info;
                    strcpy(info.name, text);
                    sscanf(p+1, "%f", &info.value);
                    obj->SetInfo(i, info);
                }

                // Sets the parameters of the command line.
                char* p = SearchOp(line, "cmdline");
                for (int i = 0; i < OBJECTMAXCMDLINE; i++)
                {
                    float value = GetFloat(p, i, NAN);
                    if (value == NAN) break;
                    obj->SetCmdLine(i, value);
                }

                if (OpInt(line, "select", 0) == 1)
                {
                    sel = obj;
                }

                obj->SetSelectable(OpInt(line, "selectable", 1));
                obj->SetEnable(OpInt(line, "enable", 1));
                obj->SetProxyActivate(OpInt(line, "proxyActivate", 0));
                obj->SetProxyDistance(OpFloat(line, "proxyDistance", 15.0f)*g_unit);
                obj->SetRange(OpFloat(line, "range", 30.0f));
                obj->SetShield(OpFloat(line, "shield", 1.0f));
                obj->SetMagnifyDamage(OpFloat(line, "magnifyDamage", 1.0f));
                obj->SetClip(OpInt(line, "clip", 1));
                obj->SetCheckToken(m_version >= 2 ? trainer : OpInt(line, "manual", 1));
                obj->SetManual(m_version >= 2 ? !trainer : OpInt(line, "manual", 0));

                if(m_version >= 2) {
                    Math::Vector zoom = OpDir(line, "zoom");
                    if (zoom.x != 0.0f || zoom.y != 0.0f || zoom.z != 0.0f)
                        obj->SetZoom(0, zoom);
                }

                CMotion* motion = obj->GetMotion();
                if (motion != nullptr)
                {
                    p = SearchOp(line, "param");
                    for (int i = 0; i < 10; i++)
                    {
                        float   value;
                        value = GetFloat(p, i, NAN);
                        if (value == NAN) break;
                        motion->SetParam(i, value);
                    }
                }

                int run = -1;
                CBrain* brain = obj->GetBrain();
                if (brain != nullptr)
                {
                    for (int i = 0; i < 10; i++)
                    {
                        sprintf(op, "script%d", i+1);  // script1..script10
                        OpString(line, op, name);
/* TODO: #if _SCHOOL
                        if ( !m_dialog->GetSoluce4() && i == 3 )  continue;
#endif*/
                        if (name[0] != 0)
                            brain->SetScriptName(i, name);

                    }

                    int i = OpInt(line, "run", 0);
                    if (i != 0)
                    {
                        run = i-1;
                        brain->SetScriptRun(run);
                    }
                }
                CAuto* automat = obj->GetAuto();
                if (automat != nullptr)
                {
                    type = OpTypeObject(line, "autoType", OBJECT_NULL);
                    automat->SetType(type);
                    for (int i = 0; i < 5; i++)
                    {
                        sprintf(op, "autoValue%d", i+1);  // autoValue1..autoValue5
                        automat->SetValue(i, OpFloat(line, op, 0.0f));
                    }
                    OpString(line, "autoString", name);
                    automat->SetString(name);

                    int i = OpInt(line, "run", -1);
                    if (i != -1)
                    {
                        if (i != PARAM_FIXSCENE &&
                            !m_dialog->GetMovies()) i = 0;
                        automat->Start(i);  // starts the film
                    }
                }

                OpString(line, "soluce", name);
                if (soluce && brain != 0 && name[0] != 0)
                    brain->SetSoluceName(name);

                obj->SetResetPosition(obj->GetPosition(0));
                obj->SetResetAngle(obj->GetAngle(0));
                obj->SetResetRun(run);

                if (OpInt(line, "reset", 0) == 1)
                    obj->SetResetCap(RESET_MOVE);
            }

            rankObj ++;
        }

        if (Cmd(line, "CreateFog") && !resetObject)
        {
            Gfx::ParticleType type = static_cast<Gfx::ParticleType>((Gfx::PARTIFOG0+OpInt(line, "type", 0)));
            Math::Vector pos = OpPos(line, "pos")*g_unit;
            float height = OpFloat(line, "height", 1.0f)*g_unit;
            float ddim = OpFloat(line, "dim", 50.0f)*g_unit;
            float delay = OpFloat(line, "delay", 2.0f);
            m_terrain->AdjustToFloor(pos);
            pos.y += height;
            Math::Point dim;
            dim.x = ddim;
            dim.y = dim.x;
            m_particle->CreateParticle(pos, Math::Vector(0.0f, 0.0f, 0.0f), dim, type, delay, 0.0f, 0.0f);
        }

        if (Cmd(line, "CreateLight") && !resetObject)
        {
            Gfx::EngineObjectType  type;

            int lightRank = CreateLight(OpDir(line, "dir"),
                                        OpColor(line, "color", Gfx::Color(0.5f, 0.5f, 0.5f, 1.0f)));

            type = OpTypeTerrain(line, "type", Gfx::ENG_OBJTYPE_NULL);
            if (type == Gfx::ENG_OBJTYPE_TERRAIN)
                m_lightMan->SetLightIncludeType(lightRank, Gfx::ENG_OBJTYPE_TERRAIN);

            if (type == Gfx::ENG_OBJTYPE_QUARTZ)
                m_lightMan->SetLightIncludeType(lightRank, Gfx::ENG_OBJTYPE_QUARTZ);

            if (type == Gfx::ENG_OBJTYPE_METAL)
                m_lightMan->SetLightIncludeType(lightRank, Gfx::ENG_OBJTYPE_METAL);

            if (type == Gfx::ENG_OBJTYPE_FIX)
                m_lightMan->SetLightExcludeType(lightRank, Gfx::ENG_OBJTYPE_TERRAIN);
        }
        if (Cmd(line, "CreateSpot") && !resetObject)
        {
            Gfx::EngineObjectType  type;

            int rankLight = CreateSpot(OpDir(line, "pos")*g_unit,
                                       OpColor(line, "color", Gfx::Color(0.5f, 0.5f, 0.5f, 1.0f)));

            type = OpTypeTerrain(line, "type", Gfx::ENG_OBJTYPE_NULL);
            if (type == Gfx::ENG_OBJTYPE_TERRAIN)
                m_lightMan->SetLightIncludeType(rankLight, Gfx::ENG_OBJTYPE_TERRAIN);

            if (type == Gfx::ENG_OBJTYPE_QUARTZ)
                m_lightMan->SetLightIncludeType(rankLight, Gfx::ENG_OBJTYPE_QUARTZ);

            if (type == Gfx::ENG_OBJTYPE_METAL)
                m_lightMan->SetLightIncludeType(rankLight, Gfx::ENG_OBJTYPE_METAL);

            if (type == Gfx::ENG_OBJTYPE_FIX)
                m_lightMan->SetLightExcludeType(rankLight, Gfx::ENG_OBJTYPE_TERRAIN);
        }

        if (Cmd(line, "GroundSpot") && !resetObject)
        {
            rank = m_engine->CreateGroundSpot();
            if (rank != -1)
            {
                m_engine->SetObjectGroundSpotPos(rank, OpPos(line, "pos")*g_unit);
                m_engine->SetObjectGroundSpotRadius(rank, OpFloat(line, "radius", 10.0f)*g_unit);
                m_engine->SetObjectGroundSpotColor(rank, OpColor(line, "color", Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f)));
                m_engine->SetObjectGroundSpotSmooth(rank, OpFloat(line, "smooth", 1.0f));
                m_engine->SetObjectGroundSpotMinMax(rank, OpFloat(line, "min", 0.0f)*g_unit,
                                                          OpFloat(line, "max", 0.0f)*g_unit);
            }
        }

        if (Cmd(line, "WaterColor") && !resetObject)
            m_engine->SetWaterAddColor(OpColor(line, "color", Gfx::Color(0.0f, 0.0f, 0.0f, 1.0f)));

        if (Cmd(line, "MapColor") && !resetObject)
        {
            m_map->FloorColorMap(OpColor(line, "floor", Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f)),
                                 OpColor(line, "water", Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f)));
            m_mapShow = OpInt(line, "show", 1);
            m_map->ShowMap(m_mapShow);
            m_map->SetToy(OpInt(line, "toyIcon", 0));
            m_mapImage = OpInt(line, "image", 0);
            if (m_mapImage)
            {
                Math::Vector    offset;
                OpString(line, "filename", m_mapFilename);
                offset = OpPos(line, "offset");
                m_map->SetFixParam(OpFloat(line, "zoom", 1.0f),
                                   offset.x, offset.z,
                                   OpFloat(line, "angle", 0.0f)*Math::PI/180.0f,
                                   OpInt(line, "mode", 0),
                                   OpInt(line, "debug", 0));
            }
        }
        if (Cmd(line, "MapZoom") && !resetObject)
        {
            m_map->ZoomMap(OpFloat(line, "factor", 2.0f));
            m_map->MapEnable(OpInt(line, "enable", 1));
        }

        if (Cmd(line, "MaxFlyingHeight") && !resetObject)
        {
            m_terrain->SetFlyingMaxHeight(OpFloat(line, "max", 280.0f)*g_unit);
        }
        if (Cmd(line, "AddFlyingHeight") && !resetObject)
        {
            m_terrain->AddFlyingLimit(OpPos(line, "center")*g_unit,
                                      OpFloat(line, "extRadius", 20.0f)*g_unit,
                                      OpFloat(line, "intRadius", 10.0f)*g_unit,
                                      OpFloat(line, "maxHeight", 200.0f));
        }

        if (Cmd(line, "Camera"))
        {
            m_camera->Init(OpDir(line, "eye")*g_unit,
                           OpDir(line, "lookat")*g_unit,
                           resetObject?0.0f:OpFloat(line, "delay", 0.0f));

            if (OpInt(line, "fadeIn", 0) == 1)
                m_camera->StartOver(Gfx::CAM_OVER_EFFECT_FADEIN_WHITE, Math::Vector(0.0f, 0.0f, 0.0f), 1.0f);

            m_camera->SetFixDirection(OpFloat(line, "fixDirection", 0.25f)*Math::PI);
        }

        if (Cmd(line, "EndMissionTake") && !resetObject)
        {
            int i = m_endTakeTotal;
            if (i < 10)
            {
                m_endTake[i].pos  = OpPos(line, "pos")*g_unit;
                m_endTake[i].dist = OpFloat(line, "dist", 8.0f)*g_unit;
                m_endTake[i].type = OpTypeObject(line, "type", OBJECT_NULL);
                m_endTake[i].min  = OpInt(line, "min", 1);
                m_endTake[i].max  = OpInt(line, "max", 9999);
                m_endTake[i].lost = OpInt(line, "lost", -1);
                m_endTake[i].immediat = OpInt(line, "immediat", 0);
                OpString(line, "message", m_endTake[i].message);
                m_endTakeTotal ++;
            }
        }
        if (Cmd(line, "EndMissionDelay") && !resetObject)
        {
            m_endTakeWinDelay  = OpFloat(line, "win",  2.0f);
            m_endTakeLostDelay = OpFloat(line, "lost", 2.0f);
        }
        if (Cmd(line, "EndMissionResearch") && !resetObject)
        {
            m_endTakeResearch |= OpResearch(line, "type");
        }

        if (Cmd(line, "ObligatoryToken") && !resetObject)
        {
            int i = m_obligatoryTotal;
            if (i < 100)
            {
                OpString(line, "text", m_obligatoryToken[i]);
                m_obligatoryTotal ++;
            }
        }

        if (Cmd(line, "ProhibitedToken") && !resetObject)
        {
            int i = m_prohibitedTotal;
            if (i < 100)
            {
                OpString(line, "text", m_prohibitedToken[i]);
                m_prohibitedTotal ++;
            }
        }

        if (Cmd(line, "EnableBuild") && !resetObject)
            g_build |= OpBuild(line, "type");

        if (Cmd(line, "EnableResearch") && !resetObject)
            g_researchEnable |= OpResearch(line, "type");

        if (Cmd(line, "DoneResearch") && read[0] == 0 && !resetObject)  // not loading file?
            g_researchDone |= OpResearch(line, "type");

        if (Cmd(line, "NewScript") && !resetObject)
        {
            OpString(line, "name", name);
            AddNewScriptName(OpTypeObject(line, "type", OBJECT_NULL), name);
        }
    }

    fclose(file);

    if (read[0] == 0)
        CompileScript(soluce);  // compiles all scripts

    if (strcmp(base, "scene") == 0 && !resetObject)  // mission?
        WriteFreeParam();

    if (strcmp(base, "free") == 0 && !resetObject)  // free play?
    {
        g_researchDone = m_freeResearch;

        g_build = m_freeBuild;
        g_build &= ~BUILD_RESEARCH;
        g_build &= ~BUILD_LABO;
        g_build |= BUILD_FACTORY;
        g_build |= BUILD_GFLAT;
        g_build |= BUILD_FLAG;
    }

    if (!resetObject)
    {
        ChangeColor();  // changes the colors of texture
        m_short->SetMode(false);  // vehicles?
    }

    CreateShortcuts();
    m_map->UpdateMap();
    // TODO: m_engine->TimeInit(); ??
    m_app->ResetKeyStates();
    m_time = 0.0f;
    m_gameTime = 0.0f;
    m_checkEndTime = 0.0f;
    m_infoUsed = 0;

    m_selectObject = sel;

    if (!m_base     &&  // no main base?
        !m_fixScene)    // interractive scene?
    {
        CObject* obj;
        if (sel == nullptr)
            obj = SearchHuman();
        else
            obj = sel;

        if (obj != nullptr)
        {
            SelectObject(obj);
            m_camera->SetControllingObject(obj);
            m_camera->SetType(obj->GetCameraType());
        }
    }

    if (m_fixScene)
        m_camera->SetType(Gfx::CAM_TYPE_SCRIPT);

    if (read[0] != 0 && sel != 0)  // loading file?
    {
        Math::Vector pos = sel->GetPosition(0);
        m_camera->Init(pos, pos, 0.0f);
        m_camera->FixCamera();

        SelectObject(sel);
        m_camera->SetControllingObject(sel);

        m_beginSatCom = true;  // message already displayed
    }
    m_dialog->SetSceneRead("");
    m_dialog->SetStackRead("");

    RestoreNumericLocale();
}

//! Creates an object of decoration mobile or stationary
CObject* CRobotMain::CreateObject(Math::Vector pos, float angle, float zoom, float height,
                                  ObjectType type, float power,
                                  bool trainer, bool toy,
                                  int option)
{
    CObject* object = nullptr;

    if ( type == OBJECT_NULL ) return nullptr;

    if ( type == OBJECT_HUMAN ||
         type == OBJECT_TECH  )
    {
        trainer = false;  // necessarily
    }

    if ( type == OBJECT_PORTICO  ||
         type == OBJECT_BASE     ||
         type == OBJECT_DERRICK  ||
         type == OBJECT_FACTORY  ||
         type == OBJECT_STATION  ||
         type == OBJECT_CONVERT  ||
         type == OBJECT_REPAIR   ||
         type == OBJECT_DESTROYER||
         type == OBJECT_TOWER    ||
         type == OBJECT_NEST     ||
         type == OBJECT_RESEARCH ||
         type == OBJECT_RADAR    ||
         type == OBJECT_INFO     ||
         type == OBJECT_ENERGY   ||
         type == OBJECT_LABO     ||
         type == OBJECT_NUCLEAR  ||
         type == OBJECT_PARA     ||
         type == OBJECT_SAFE     ||
         type == OBJECT_HUSTON   ||
         type == OBJECT_TARGET1  ||
         type == OBJECT_TARGET2  ||
         type == OBJECT_START    ||
         type == OBJECT_END      )
    {
        object = new CObject();
        object->CreateBuilding(pos, angle, height, type, power);

        CAuto* automat = object->GetAuto();
        if (automat != nullptr)
        {
            automat->Init();
        }
    }
    else
    if ( type == OBJECT_FRET        ||
         type == OBJECT_STONE       ||
         type == OBJECT_URANIUM     ||
         type == OBJECT_METAL       ||
         type == OBJECT_POWER       ||
         type == OBJECT_ATOMIC      ||
         type == OBJECT_BULLET      ||
         type == OBJECT_BBOX        ||
         type == OBJECT_KEYa        ||
         type == OBJECT_KEYb        ||
         type == OBJECT_KEYc        ||
         type == OBJECT_KEYd        ||
         type == OBJECT_TNT         ||
         type == OBJECT_SCRAP1      ||
         type == OBJECT_SCRAP2      ||
         type == OBJECT_SCRAP3      ||
         type == OBJECT_SCRAP4      ||
         type == OBJECT_SCRAP5      ||
         type == OBJECT_BOMB        ||
         type == OBJECT_WAYPOINT    ||
         type == OBJECT_SHOW        ||
         type == OBJECT_WINFIRE     ||
         type == OBJECT_BAG         ||
         type == OBJECT_MARKPOWER   ||
         type == OBJECT_MARKSTONE   ||
         type == OBJECT_MARKURANIUM ||
         type == OBJECT_MARKKEYa    ||
         type == OBJECT_MARKKEYb    ||
         type == OBJECT_MARKKEYc    ||
         type == OBJECT_MARKKEYd    ||
         type == OBJECT_EGG         )
    {
        object = new CObject();
        object->CreateResource(pos, angle, type, power);
    }
    else
    if ( type == OBJECT_FLAGb ||
         type == OBJECT_FLAGr ||
         type == OBJECT_FLAGg ||
         type == OBJECT_FLAGy ||
         type == OBJECT_FLAGv )
    {
        object = new CObject();
        object->CreateFlag(pos, angle, type);
    }
    else
    if ( type == OBJECT_BARRIER0 ||
         type == OBJECT_BARRIER1 ||
         type == OBJECT_BARRIER2 ||
         type == OBJECT_BARRIER3 ||
         type == OBJECT_BARRIER4 )
    {
        object = new CObject();
        object->CreateBarrier(pos, angle, height, type);
    }
    else
    if ( type == OBJECT_PLANT0  ||
         type == OBJECT_PLANT1  ||
         type == OBJECT_PLANT2  ||
         type == OBJECT_PLANT3  ||
         type == OBJECT_PLANT4  ||
         type == OBJECT_PLANT5  ||
         type == OBJECT_PLANT6  ||
         type == OBJECT_PLANT7  ||
         type == OBJECT_PLANT8  ||
         type == OBJECT_PLANT9  ||
         type == OBJECT_PLANT10 ||
         type == OBJECT_PLANT11 ||
         type == OBJECT_PLANT12 ||
         type == OBJECT_PLANT13 ||
         type == OBJECT_PLANT14 ||
         type == OBJECT_PLANT15 ||
         type == OBJECT_PLANT16 ||
         type == OBJECT_PLANT17 ||
         type == OBJECT_PLANT18 ||
         type == OBJECT_PLANT19 ||
         type == OBJECT_TREE0   ||
         type == OBJECT_TREE1   ||
         type == OBJECT_TREE2   ||
         type == OBJECT_TREE3   ||
         type == OBJECT_TREE4   ||
         type == OBJECT_TREE5   ||
         type == OBJECT_TREE6   ||
         type == OBJECT_TREE7   ||
         type == OBJECT_TREE8   ||
         type == OBJECT_TREE9   )
    {
        object = new CObject();
        object->CreatePlant(pos, angle, height, type);
    }
    else
    if ( type == OBJECT_MUSHROOM0 ||
         type == OBJECT_MUSHROOM1 ||
         type == OBJECT_MUSHROOM2 ||
         type == OBJECT_MUSHROOM3 ||
         type == OBJECT_MUSHROOM4 ||
         type == OBJECT_MUSHROOM5 ||
         type == OBJECT_MUSHROOM6 ||
         type == OBJECT_MUSHROOM7 ||
         type == OBJECT_MUSHROOM8 ||
         type == OBJECT_MUSHROOM9 )
    {
        object = new CObject();
        object->CreateMushroom(pos, angle, height, type);
    }
    else
    if ( type == OBJECT_TEEN0  ||
         type == OBJECT_TEEN1  ||
         type == OBJECT_TEEN2  ||
         type == OBJECT_TEEN3  ||
         type == OBJECT_TEEN4  ||
         type == OBJECT_TEEN5  ||
         type == OBJECT_TEEN6  ||
         type == OBJECT_TEEN7  ||
         type == OBJECT_TEEN8  ||
         type == OBJECT_TEEN9  ||
         type == OBJECT_TEEN10 ||
         type == OBJECT_TEEN11 ||
         type == OBJECT_TEEN12 ||
         type == OBJECT_TEEN13 ||
         type == OBJECT_TEEN14 ||
         type == OBJECT_TEEN15 ||
         type == OBJECT_TEEN16 ||
         type == OBJECT_TEEN17 ||
         type == OBJECT_TEEN18 ||
         type == OBJECT_TEEN19 ||
         type == OBJECT_TEEN20 ||
         type == OBJECT_TEEN21 ||
         type == OBJECT_TEEN22 ||
         type == OBJECT_TEEN23 ||
         type == OBJECT_TEEN24 ||
         type == OBJECT_TEEN25 ||
         type == OBJECT_TEEN26 ||
         type == OBJECT_TEEN27 ||
         type == OBJECT_TEEN28 ||
         type == OBJECT_TEEN29 ||
         type == OBJECT_TEEN30 ||
         type == OBJECT_TEEN31 ||
         type == OBJECT_TEEN32 ||
         type == OBJECT_TEEN33 ||
         type == OBJECT_TEEN34 ||
         type == OBJECT_TEEN35 ||
         type == OBJECT_TEEN36 ||
         type == OBJECT_TEEN37 ||
         type == OBJECT_TEEN38 ||
         type == OBJECT_TEEN39 ||
         type == OBJECT_TEEN40 ||
         type == OBJECT_TEEN41 ||
         type == OBJECT_TEEN42 ||
         type == OBJECT_TEEN43 ||
         type == OBJECT_TEEN44 ||
         type == OBJECT_TEEN45 ||
         type == OBJECT_TEEN46 ||
         type == OBJECT_TEEN47 ||
         type == OBJECT_TEEN48 ||
         type == OBJECT_TEEN49 )
    {
        object = new CObject();
        object->SetOption(option);
        object->CreateTeen(pos, angle, zoom, height, type);
    }
    else
    if ( type == OBJECT_QUARTZ0 ||
         type == OBJECT_QUARTZ1 ||
         type == OBJECT_QUARTZ2 ||
         type == OBJECT_QUARTZ3 ||
         type == OBJECT_QUARTZ4 ||
         type == OBJECT_QUARTZ5 ||
         type == OBJECT_QUARTZ6 ||
         type == OBJECT_QUARTZ7 ||
         type == OBJECT_QUARTZ8 ||
         type == OBJECT_QUARTZ9 )
    {
        object = new CObject();
        object->CreateQuartz(pos, angle, height, type);
    }
    else
    if ( type == OBJECT_ROOT0 ||
         type == OBJECT_ROOT1 ||
         type == OBJECT_ROOT2 ||
         type == OBJECT_ROOT3 ||
         type == OBJECT_ROOT4 ||
         type == OBJECT_ROOT5 ||
         type == OBJECT_ROOT6 ||
         type == OBJECT_ROOT7 ||
         type == OBJECT_ROOT8 ||
         type == OBJECT_ROOT9 )
    {
        object = new CObject();
        object->CreateRoot(pos, angle, height, type);
    }
    else
    if ( type == OBJECT_HOME1 )
    {
        object = new CObject();
        object->CreateHome(pos, angle, height, type);
    }
    else
    if ( type == OBJECT_RUINmobilew1 ||
         type == OBJECT_RUINmobilew2 ||
         type == OBJECT_RUINmobilet1 ||
         type == OBJECT_RUINmobilet2 ||
         type == OBJECT_RUINmobiler1 ||
         type == OBJECT_RUINmobiler2 ||
         type == OBJECT_RUINfactory  ||
         type == OBJECT_RUINdoor     ||
         type == OBJECT_RUINsupport  ||
         type == OBJECT_RUINradar    ||
         type == OBJECT_RUINconvert  ||
         type == OBJECT_RUINbase     ||
         type == OBJECT_RUINhead     )
    {
        object = new CObject();
        object->CreateRuin(pos, angle, height, type);
    }
    else
    if ( type == OBJECT_APOLLO1 ||
         type == OBJECT_APOLLO3 ||
         type == OBJECT_APOLLO4 ||
         type == OBJECT_APOLLO5 )
    {
        object = new CObject();
        object->CreateApollo(pos, angle, type);
    }
    else
    if ( type == OBJECT_MOTHER ||
         type == OBJECT_ANT    ||
         type == OBJECT_SPIDER ||
         type == OBJECT_BEE    ||
         type == OBJECT_WORM   )
    {
        object = new CObject();
        object->CreateInsect(pos, angle, type);  // no eggs
    }
    else
    if ( type == OBJECT_HUMAN    ||
         type == OBJECT_TECH     ||
         type == OBJECT_TOTO     ||
         type == OBJECT_MOBILEfa ||
         type == OBJECT_MOBILEta ||
         type == OBJECT_MOBILEwa ||
         type == OBJECT_MOBILEia ||
         type == OBJECT_MOBILEfc ||
         type == OBJECT_MOBILEtc ||
         type == OBJECT_MOBILEwc ||
         type == OBJECT_MOBILEic ||
         type == OBJECT_MOBILEfi ||
         type == OBJECT_MOBILEti ||
         type == OBJECT_MOBILEwi ||
         type == OBJECT_MOBILEii ||
         type == OBJECT_MOBILEfs ||
         type == OBJECT_MOBILEts ||
         type == OBJECT_MOBILEws ||
         type == OBJECT_MOBILEis ||
         type == OBJECT_MOBILErt ||
         type == OBJECT_MOBILErc ||
         type == OBJECT_MOBILErr ||
         type == OBJECT_MOBILErs ||
         type == OBJECT_MOBILEsa ||
         type == OBJECT_MOBILEtg ||
         type == OBJECT_MOBILEft ||
         type == OBJECT_MOBILEtt ||
         type == OBJECT_MOBILEwt ||
         type == OBJECT_MOBILEit ||
         type == OBJECT_MOBILEdr ||
         type == OBJECT_APOLLO2  )
    {
        object = new CObject();
        object->SetOption(option);
        object->CreateVehicle(pos, angle, type, power, trainer, toy);
    }

    if (m_fixScene && type == OBJECT_HUMAN)
    {
        CMotion* motion = object->GetMotion();
        if (m_phase == PHASE_WIN ) motion->SetAction(MHS_WIN,  0.4f);
        if (m_phase == PHASE_LOST) motion->SetAction(MHS_LOST, 0.5f);
    }

    return object;
}

//! Creates a directional light
int CRobotMain::CreateLight(Math::Vector direction, Gfx::Color color)
{
    if (direction.x == 0.0f &&
        direction.y == 0.0f &&
        direction.z == 0.0f)
    {
        direction.y = -1.0f;
    }

    Gfx::Light light;
    light.type = Gfx::LIGHT_DIRECTIONAL;
    light.diffuse = color;
    light.ambient = color * 0.1f;
    light.direction  = direction;
    int obj = m_lightMan->CreateLight(Gfx::LIGHT_PRI_HIGH);
    m_lightMan->SetLight(obj, light);

    return obj;
}

//! Creates a light spot
int CRobotMain::CreateSpot(Math::Vector pos, Gfx::Color color)
{
    if (!m_engine->GetLightMode()) return -1;

    pos.y += m_terrain->GetFloorLevel(pos);

    Gfx::Light light;
    light.type          = Gfx::LIGHT_SPOT;
    light.diffuse       = color;
    light.ambient       = color * 0.1f;
    light.position      = pos;
    light.direction     = Math::Vector(0.0f, -1.0f, 0.0f);
    light.spotIntensity = 1.0f;
    light.spotAngle     = 90.0f*Math::PI/180.0f;
    light.attenuation0  = 2.0f;
    light.attenuation1  = 0.0f;
    light.attenuation2  = 0.0f;
    int obj = m_lightMan->CreateLight(Gfx::LIGHT_PRI_HIGH);
    m_lightMan->SetLight(obj, light);

    return obj;
}


//! Change the colors and textures
void CRobotMain::ChangeColor()
{
    Math::Point ts = Math::Point(0.0f, 0.0f);
    Math::Point ti = Math::Point(1.0f, 1.0f);  // the entire image

    Gfx::Color colorRef1, colorNew1, colorRef2, colorNew2;

    colorRef1.a = 0.0f;
    colorRef2.a = 0.0f;

    colorRef1.r = 206.0f/256.0f;
    colorRef1.g = 206.0f/256.0f;
    colorRef1.b = 204.0f/256.0f;  // ~white
    colorNew1 = m_dialog->GetGamerColorCombi();
    colorRef2.r = 255.0f/256.0f;
    colorRef2.g = 132.0f/256.0f;
    colorRef2.b =   1.0f/256.0f;  // orange
    colorNew2 = m_dialog->GetGamerColorBand();

    Math::Point exclu[6];
    exclu[0] = Math::Point(192.0f/256.0f,   0.0f/256.0f);
    exclu[1] = Math::Point(256.0f/256.0f,  64.0f/256.0f);  // crystals + cylinders
    exclu[2] = Math::Point(208.0f/256.0f, 224.0f/256.0f);
    exclu[3] = Math::Point(256.0f/256.0f, 256.0f/256.0f);  // SatCom screen
    exclu[4] = Math::Point(0.0f, 0.0f);
    exclu[5] = Math::Point(0.0f, 0.0f);  // terminator
    m_engine->ChangeTextureColor("human.png", colorRef1, colorNew1, colorRef2, colorNew2, 0.30f, 0.01f, ts, ti, exclu);

    float tolerance;

    int face = GetGamerFace();
    if (face == 0)  // normal?
    {
        colorRef1.r =  90.0f/256.0f;
        colorRef1.g =  95.0f/256.0f;
        colorRef1.b =  85.0f/256.0f;  // black
        tolerance = 0.15f;
    }
    if (face == 1)  // bald?
    {
        colorRef1.r =  74.0f/256.0f;
        colorRef1.g =  58.0f/256.0f;
        colorRef1.b =  46.0f/256.0f;  // brown
        tolerance = 0.20f;
    }
    if (face == 2)  // carlos?
    {
        colorRef1.r =  70.0f/256.0f;
        colorRef1.g =  40.0f/256.0f;
        colorRef1.b =   8.0f/256.0f;  // brown
        tolerance = 0.30f;
    }
    if (face == 3)  // blonde?
    {
        colorRef1.r =  74.0f/256.0f;
        colorRef1.g =  16.0f/256.0f;
        colorRef1.b =   0.0f/256.0f;  // yellow
        tolerance = 0.20f;
    }
    colorNew1 = m_dialog->GetGamerColorHair();
    colorRef2.r = 0.0f;
    colorRef2.g = 0.0f;
    colorRef2.b = 0.0f;
    colorNew2.r = 0.0f;
    colorNew2.g = 0.0f;
    colorNew2.b = 0.0f;

    char name[100];
    sprintf(name, "face%.2d.png", face+1);
    exclu[0] = Math::Point(105.0f/256.0f, 47.0f/166.0f);
    exclu[1] = Math::Point(153.0f/256.0f, 79.0f/166.0f);  // blue canister
    exclu[2] = Math::Point(0.0f, 0.0f);
    exclu[3] = Math::Point(0.0f, 0.0f);  // terminator
    m_engine->ChangeTextureColor(name, colorRef1, colorNew1, colorRef2, colorNew2, tolerance, 0.00f, ts, ti, exclu);

    colorRef2.r = 0.0f;
    colorRef2.g = 0.0f;
    colorRef2.b = 0.0f;
    colorNew2.r = 0.0f;
    colorNew2.g = 0.0f;
    colorNew2.b = 0.0f;

    m_engine->ChangeTextureColor("base1.png",   m_colorRefBot, m_colorNewBot, colorRef2, colorNew2, 0.10f, -1.0f, ts, ti, 0, 0, true);
    m_engine->ChangeTextureColor("convert.png", m_colorRefBot, m_colorNewBot, colorRef2, colorNew2, 0.10f, -1.0f, ts, ti, 0, 0, true);
    m_engine->ChangeTextureColor("derrick.png", m_colorRefBot, m_colorNewBot, colorRef2, colorNew2, 0.10f, -1.0f, ts, ti, 0, 0, true);
    m_engine->ChangeTextureColor("factory.png", m_colorRefBot, m_colorNewBot, colorRef2, colorNew2, 0.10f, -1.0f, ts, ti, 0, 0, true);
    m_engine->ChangeTextureColor("lemt.png",    m_colorRefBot, m_colorNewBot, colorRef2, colorNew2, 0.10f, -1.0f, ts, ti, 0, 0, true);
    m_engine->ChangeTextureColor("roller.png",  m_colorRefBot, m_colorNewBot, colorRef2, colorNew2, 0.10f, -1.0f, ts, ti, 0, 0, true);
    m_engine->ChangeTextureColor("search.png",  m_colorRefBot, m_colorNewBot, colorRef2, colorNew2, 0.10f, -1.0f, ts, ti, 0, 0, true);

    exclu[0] = Math::Point(  0.0f/256.0f, 160.0f/256.0f);
    exclu[1] = Math::Point(256.0f/256.0f, 256.0f/256.0f);  // pencils
    exclu[2] = Math::Point(0.0f, 0.0f);
    exclu[3] = Math::Point(0.0f, 0.0f);  // terminator
    m_engine->ChangeTextureColor("drawer.png",  m_colorRefBot, m_colorNewBot, colorRef2, colorNew2, 0.10f, -1.0f, ts, ti, exclu, 0, true);

    exclu[0] = Math::Point(237.0f/256.0f, 176.0f/256.0f);
    exclu[1] = Math::Point(256.0f/256.0f, 220.0f/256.0f);  // blue canister
    exclu[2] = Math::Point(106.0f/256.0f, 150.0f/256.0f);
    exclu[3] = Math::Point(130.0f/256.0f, 214.0f/256.0f);  // safe location
    exclu[4] = Math::Point(0.0f, 0.0f);
    exclu[5] = Math::Point(0.0f, 0.0f);  // terminator
    m_engine->ChangeTextureColor("subm.png",    m_colorRefBot, m_colorNewBot, colorRef2, colorNew2, 0.10f, -1.0f, ts, ti, exclu, 0, true);

    exclu[0] = Math::Point(128.0f/256.0f, 160.0f/256.0f);
    exclu[1] = Math::Point(256.0f/256.0f, 256.0f/256.0f);  // SatCom
    exclu[2] = Math::Point(0.0f, 0.0f);
    exclu[3] = Math::Point(0.0f, 0.0f);  // terminator
    m_engine->ChangeTextureColor("ant.png",     m_colorRefAlien, m_colorNewAlien, colorRef2, colorNew2, 0.50f, -1.0f, ts, ti, exclu);
    m_engine->ChangeTextureColor("mother.png",  m_colorRefAlien, m_colorNewAlien, colorRef2, colorNew2, 0.50f, -1.0f, ts, ti);

    m_engine->ChangeTextureColor("plant.png",   m_colorRefGreen, m_colorNewGreen, colorRef2, colorNew2, 0.50f, -1.0f, ts, ti);

    // PARTIPLOUF0 and PARTIDROP :
    ts = Math::Point(0.500f, 0.500f);
    ti = Math::Point(0.875f, 0.750f);
    m_engine->ChangeTextureColor("effect00.png", m_colorRefWater, m_colorNewWater, colorRef2, colorNew2, 0.20f, -1.0f, ts, ti, 0, m_colorShiftWater, true);

    // PARTIFLIC :
    ts = Math::Point(0.00f, 0.75f);
    ti = Math::Point(0.25f, 1.00f);
    m_engine->ChangeTextureColor("effect02.png", m_colorRefWater, m_colorNewWater, colorRef2, colorNew2, 0.20f, -1.0f, ts, ti, 0, m_colorShiftWater, true);
}

//! Updates the number of unnecessary objects
bool CRobotMain::TestGadgetQuantity(int rank)
{
    static int table10[10] = {0,1,0,0,0,0,0,0,0,0};
    static int table20[10] = {0,1,0,0,0,1,0,0,0,0};
    static int table30[10] = {0,1,0,1,0,1,0,0,0,0};
    static int table40[10] = {0,1,0,1,0,1,0,1,0,0};
    static int table50[10] = {0,1,0,1,0,1,0,1,0,1};
    static int table60[10] = {0,1,0,1,1,1,0,1,0,1};
    static int table70[10] = {0,1,0,1,1,1,0,1,1,1};
    static int table80[10] = {0,1,1,1,1,1,0,1,1,1};
    static int table90[10] = {0,1,1,1,1,1,1,1,1,1};

    float percent = m_engine->GetGadgetQuantity();
    if (percent == 0.0f) return false;
    if (percent == 1.0f) return true;

    int *table;
         if (percent <= 0.15f) table = table10;
    else if (percent <= 0.25f) table = table20;
    else if (percent <= 0.35f) table = table30;
    else if (percent <= 0.45f) table = table40;
    else if (percent <= 0.55f) table = table50;
    else if (percent <= 0.65f) table = table60;
    else if (percent <= 0.75f) table = table70;
    else if (percent <= 0.85f) table = table80;
    else                       table = table90;

    return table[rank%10];
}



//! Calculates the distance to the nearest object
float CRobotMain::SearchNearestObject(Math::Vector center, CObject *exclu)
{
    CInstanceManager* iMan = CInstanceManager::GetInstancePointer();

    float min = 100000.0f;
    for (int i = 0; i < 1000000; i++)
    {
        CObject* obj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, i));
        if (obj == nullptr)  break;

        if (!obj->GetActif()) continue;  // inactive?
        if (obj->GetTruck() != nullptr) continue;  // object carries?
        if (obj == exclu)  continue;

        ObjectType type = obj->GetType();

        if (type == OBJECT_BASE)
        {
            Math::Vector oPos = obj->GetPosition(0);
            if (oPos.x != center.x ||
                oPos.z != center.z)
            {
                float dist = Math::Distance(center, oPos)-80.0f;
                if (dist < 0.0f) dist = 0.0f;
                min = Math::Min(min, dist);
                continue;
            }
        }

        if (type == OBJECT_STATION   ||
            type == OBJECT_REPAIR    ||
            type == OBJECT_DESTROYER)
        {
            Math::Vector oPos = obj->GetPosition(0);
            float dist = Math::Distance(center, oPos)-8.0f;
            if (dist < 0.0f) dist = 0.0f;
            min = Math::Min(min, dist);
        }

        int j = 0;
        Math::Vector oPos;
        float oRadius;
        while (obj->GetCrashSphere(j++, oPos, oRadius))
        {
            float dist = Math::Distance(center, oPos)-oRadius;
            if (dist < 0.0f) dist = 0.0f;
            min = Math::Min(min, dist);
        }
    }
    return min;
}

//! Calculates a free space
bool CRobotMain::FreeSpace(Math::Vector &center, float minRadius, float maxRadius,
                           float space, CObject *exclu)
{
    if (minRadius < maxRadius)  // from internal to external?
    {
        for (float radius = minRadius; radius <= maxRadius; radius += space)
        {
            float ia = space/radius;
            for (float angle = 0.0f; angle < Math::PI*2.0f; angle += ia)
            {
                Math::Point p;
                p.x = center.x+radius;
                p.y = center.z;
                p = Math::RotatePoint(Math::Point(center.x, center.z), angle, p);
                Math::Vector pos;
                pos.x = p.x;
                pos.z = p.y;
                pos.y = 0.0f;
                m_terrain->AdjustToFloor(pos, true);
                float dist = SearchNearestObject(pos, exclu);
                if (dist >= space)
                {
                    float flat = m_terrain->GetFlatZoneRadius(pos, dist/2.0f);
                    if (flat >= dist/2.0f)
                    {
                        center = pos;
                        return true;
                    }
                }
            }
        }
    }
    else    // from external to internal?
    {
        for (float radius=maxRadius; radius >= minRadius; radius -= space)
        {
            float ia = space/radius;
            for (float angle=0.0f ; angle<Math::PI*2.0f ; angle+=ia )
            {
                Math::Point p;
                p.x = center.x+radius;
                p.y = center.z;
                p = Math::RotatePoint(Math::Point(center.x, center.z), angle, p);
                Math::Vector pos;
                pos.x = p.x;
                pos.z = p.y;
                pos.y = 0.0f;
                m_terrain->AdjustToFloor(pos, true);
                float dist = SearchNearestObject(pos, exclu);
                if (dist >= space)
                {
                    float flat = m_terrain->GetFlatZoneRadius(pos, dist/2.0f);
                    if (flat >= dist/2.0f)
                    {
                        center = pos;
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

//! Calculates the maximum radius of a free space
float CRobotMain::GetFlatZoneRadius(Math::Vector center, float maxRadius,
                                    CObject *exclu)
{
    float dist = SearchNearestObject(center, exclu);
    if (dist == 0.0f) return 0.0f;
    if (dist < maxRadius)
        maxRadius = dist;

    return m_terrain->GetFlatZoneRadius(center, maxRadius);
}


//! Hides buildable area when a cube of metal is taken up
void CRobotMain::HideDropZone(CObject* metal)
{
    if (m_showLimit[1].used         &&
        m_showLimit[1].link == metal)
    {
        FlushShowLimit(1);
    }

    if (m_showLimit[2].used         &&
        m_showLimit[2].link == metal)
    {
        FlushShowLimit(2);
    }
}

//! Shows the buildable area when a cube of metal is deposited
void CRobotMain::ShowDropZone(CObject* metal, CObject* truck)
{
    if (metal == nullptr) return;

    Math::Vector center = metal->GetPosition(0);

    CInstanceManager* iMan = CInstanceManager::GetInstancePointer();

    // Calculates the maximum radius possible depending on other items.
    float oMax = 30.0f;  // radius to build the biggest building
    float tMax;
    for (int i = 0; i < 1000000; i++)
    {
        CObject* obj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, i));
        if (obj == nullptr) break;

        if (!obj->GetActif()) continue;  // inactive?
        if (obj->GetTruck() != nullptr) continue;  // object carried?
        if (obj == metal) continue;
        if (obj == truck) continue;

        Math::Vector oPos;
        float oRadius;

        ObjectType type = obj->GetType();
        if (type == OBJECT_BASE)
        {
            oPos = obj->GetPosition(0);
            float dist = Math::Distance(center, oPos)-80.0f;
            oMax = Math::Min(oMax, dist);
        }
        else
        {
            int j = 0;
            while (obj->GetCrashSphere(j++, oPos, oRadius))
            {
                float dist = Math::Distance(center, oPos)-oRadius;
                oMax = Math::Min(oMax, dist);
            }
        }

        if ( type == OBJECT_DERRICK  ||
             type == OBJECT_FACTORY  ||
             type == OBJECT_STATION  ||
             type == OBJECT_CONVERT  ||
             type == OBJECT_REPAIR   ||
             type == OBJECT_DESTROYER||
             type == OBJECT_TOWER    ||
             type == OBJECT_RESEARCH ||
             type == OBJECT_RADAR    ||
             type == OBJECT_ENERGY   ||
             type == OBJECT_LABO     ||
             type == OBJECT_NUCLEAR  ||
             type == OBJECT_START    ||
             type == OBJECT_END      ||
             type == OBJECT_INFO     ||
             type == OBJECT_PARA     ||
             type == OBJECT_SAFE     ||
             type == OBJECT_HUSTON   )  // building?
        {
            int j = 0;
            while (obj->GetCrashSphere(j++, oPos, oRadius))
            {
                float dist = Math::Distance(center, oPos)-oRadius-BUILDMARGIN;
                oMax = Math::Min(oMax, dist);
            }
        }
    }

    // Calculates the maximum possible radius depending on terrain.
    if (oMax >= 2.0f)
        tMax = m_terrain->GetFlatZoneRadius(center, 30.0f);
    else
        tMax = 0.0f;

    float radius = Math::Min(oMax, tMax);
    if (radius >= 2.0f)
        SetShowLimit(1, Gfx::PARTILIMIT2, metal, center, radius, 10.0f);
}

//! Erases the boundaries shown
void CRobotMain::FlushShowLimit(int i)
{
    if (m_showLimit[i].link != 0)
    {
        m_showLimit[i].link->StopShowLimit();
    }

    for (int j = 0; j < m_showLimit[i].total; j++)
    {
        if (m_showLimit[i].parti[j] == 0) continue;

        m_particle->DeleteParticle(m_showLimit[i].parti[j]);
        m_showLimit[i].parti[j] = 0;
    }

    m_showLimit[i].total = 0;
    m_showLimit[i].link = 0;
    m_showLimit[i].used = false;
}

//! Specifies the boundaries to show
void CRobotMain::SetShowLimit(int i, Gfx::ParticleType parti, CObject *obj,
                              Math::Vector pos, float radius, float duration)
{
    FlushShowLimit(i);  // erases the current boundaries

    if (radius <= 0.0f) return;

    Math::Point dim;
    float dist;
    if (radius <= 50.0f)
    {
        dim = Math::Point(0.3f, 0.3f);
        dist = 2.5f;
    }
    else
    {
        dim = Math::Point(1.5f, 1.5f);
        dist = 10.0f;
    }

    m_showLimit[i].used = true;
    m_showLimit[i].link = obj;
    m_showLimit[i].pos = pos;
    m_showLimit[i].radius = radius;
    m_showLimit[i].duration = duration;
    m_showLimit[i].total = static_cast<int>((radius*2.0f*Math::PI)/dist);
    if (m_showLimit[i].total > MAXSHOWPARTI) m_showLimit[i].total = MAXSHOWPARTI;
    m_showLimit[i].time = 0.0f;

    for (int j = 0; j < m_showLimit[i].total; j++)
    {
        m_showLimit[i].parti[j] = m_particle->CreateParticle(pos, Math::Vector(0.0f, 0.0f, 0.0f), dim, parti, duration);
    }
}

//! Adjusts the boundaries to show
void CRobotMain::AdjustShowLimit(int i, Math::Vector pos)
{
    m_showLimit[i].pos = pos;
}

//! Mount the boundaries of the selected object
void CRobotMain::StartShowLimit()
{
    CObject* obj = GetSelect();
    if (obj == nullptr) return;

    obj->StartShowLimit();
}

//! Advances the boundaries shown
void CRobotMain::FrameShowLimit(float rTime)
{
    if (m_engine->GetPause()) return;

    for (int i = 0; i < MAXSHOWLIMIT; i++)
    {
        if (!m_showLimit[i].used) continue;

        m_showLimit[i].time += rTime;

        if (m_showLimit[i].time >= m_showLimit[i].duration)
        {
            FlushShowLimit(i);
            continue;
        }

        float factor;
        if (m_showLimit[i].time < 1.0f)
            factor = m_showLimit[i].time;
        else if (m_showLimit[i].time > m_showLimit[i].duration-1.0f)
            factor = m_showLimit[i].duration-m_showLimit[i].time;
        else
            factor = 1.0f;

        float speed = 0.4f-m_showLimit[i].radius*0.001f;
        if (speed < 0.1f) speed = 0.1f;
        float angle = m_showLimit[i].time*speed;

        for (int j = 0; j < m_showLimit[i].total; j++)
        {
            if (m_showLimit[i].parti[j] == 0) continue;

            Math::Point center;
            center.x = m_showLimit[i].pos.x;
            center.y = m_showLimit[i].pos.z;
            Math::Point rotate;
            rotate.x = center.x+m_showLimit[i].radius*factor;
            rotate.y = center.y;
            rotate = Math::RotatePoint(center, angle, rotate);

            Math::Vector pos;
            pos.x = rotate.x;
            pos.z = rotate.y;
            pos.y = 0.0f;
            m_terrain->AdjustToFloor(pos, true);
            if (m_showLimit[i].radius <= 50.0f) pos.y += 0.5f;
            else                                pos.y += 2.0f;
            m_particle->SetPosition(m_showLimit[i].parti[j], pos);

            angle += (2.0f*Math::PI)/m_showLimit[i].total;
        }
    }
}



//! Returns a pointer to the last slash in a filename.
char* SearchLastDir(char *filename)
{
    char* p = filename;

    while (*p++ != 0);
    p --;  // ^on the zero terminator

    while (p != filename)
    {
        if (*(--p) == '/') return p;
    }
    return 0;
}


//! Compiles all scripts of robots
void CRobotMain::CompileScript(bool soluce)
{
    int nbError = 0;
    int lastError = 0;

    CInstanceManager* iMan = CInstanceManager::GetInstancePointer();

    do
    {
        lastError = nbError;
        nbError = 0;
        for (int i = 0; i < 1000000; i++)
        {
            CObject* obj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, i));
            if (obj == nullptr) break;
            if (obj->GetTruck() != nullptr) continue;

            CBrain* brain = obj->GetBrain();
            if (brain == nullptr) continue;

            for (int j = 0; j < 10; j++)
            {
                if (brain->GetCompile(j)) continue;

                char* name = brain->GetScriptName(j);
                if (name[0] != 0)
                {
                    brain->ReadProgram(j, name);
                    if (!brain->GetCompile(j)) nbError++;
                }
            }

            LoadOneScript(obj, nbError);
        }
    }
    while (nbError > 0 && nbError != lastError);

    // Load all solutions.
    if (soluce)
    {
        for (int i = 0; i < 1000000; i++)
        {
            CObject* obj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, i));
            if (obj == 0)  break;
            if (obj->GetTruck() != 0)  continue;

            CBrain* brain = obj->GetBrain();
            if (brain == 0)  continue;

            char* name = brain->GetSoluceName();
            if (name[0] != 0)
            {
                brain->ReadSoluce(name);  // load solution
            }
        }
    }

    // Start all programs according to the command "run".
    for (int i = 0; i < 1000000; i++)
    {
        CObject* obj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, i));
        if (obj == nullptr) break;
        if (obj->GetTruck() != nullptr) continue;

        CBrain* brain = obj->GetBrain();
        if (brain == nullptr)  continue;

        int run = brain->GetScriptRun();
        if (run != -1)
        {
            brain->RunProgram(run);  // starts the program
        }
    }
}

//! Load all programs of the robot
void CRobotMain::LoadOneScript(CObject *obj, int &nbError)
{
    CBrain* brain = obj->GetBrain();
    if (brain == nullptr) return;

    if (!IsSelectable(obj)) return;

    ObjectType type = obj->GetType();
    if (type == OBJECT_HUMAN) return;

    int objRank = obj->GetDefRank();
    if (objRank == -1) return;

    char* name = m_dialog->GetSceneName();
    int rank = m_dialog->GetSceneRank();

    for (int i = 0; i < BRAINMAXSCRIPT; i++)
    {
        if (brain->GetCompile(i)) continue;

        char filename[MAX_FNAME];
        sprintf(filename, "%s/%s/%c%.3d%.3d%.1d.txt",
                    GetSavegameDir(), m_gamerName, name[0], rank, objRank, i);
        brain->ReadProgram(i, filename);
        if (!brain->GetCompile(i)) nbError++;
    }
}

//! Load all programs of the robot
void CRobotMain::LoadFileScript(CObject *obj, const char* filename, int objRank,
                                int &nbError)
{
    if (objRank == -1) return;

    CBrain* brain = obj->GetBrain();
    if (brain == nullptr) return;

    ObjectType type = obj->GetType();
    if (type == OBJECT_HUMAN) return;


    char fn[MAX_FNAME];
    strcpy(fn, filename);
    char* ldir = SearchLastDir(fn);
    if (ldir == 0) return;

    for (int i = 0; i < BRAINMAXSCRIPT; i++)
    {
        if (brain->GetCompile(i)) continue;

        sprintf(ldir, "/prog%.3d%.1d.txt", objRank, i);
        brain->ReadProgram(i, fn);
        if (!brain->GetCompile(i)) nbError++;
    }
}

//! Saves all programs of all the robots
void CRobotMain::SaveAllScript()
{
    CInstanceManager* iMan = CInstanceManager::GetInstancePointer();

    for (int i = 0; i < 1000000; i++)
    {
        CObject* obj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, i));
        if (obj == nullptr) break;

        SaveOneScript(obj);
    }
}

//! Saves all programs of the robot.
//! If a program does not exist, the corresponding file is destroyed.
void CRobotMain::SaveOneScript(CObject *obj)
{
    CBrain* brain = obj->GetBrain();
    if (brain == nullptr) return;

    if (!IsSelectable(obj)) return;

    ObjectType type = obj->GetType();
    if (type == OBJECT_HUMAN) return;

    int objRank = obj->GetDefRank();
    if (objRank == -1) return;

    char* name = m_dialog->GetSceneName();
    int rank = m_dialog->GetSceneRank();

    for (int i = 0; i < BRAINMAXSCRIPT; i++)
    {
        char filename[MAX_FNAME];
        sprintf(filename, "%s/%s/%c%.3d%.3d%.1d.txt",
                    GetSavegameDir(), m_gamerName, name[0], rank, objRank, i);
        brain->WriteProgram(i, filename);
    }
}

//! Saves all programs of the robot.
//! If a program does not exist, the corresponding file is destroyed.
void CRobotMain::SaveFileScript(CObject *obj, const char* filename, int objRank)
{
    if (objRank == -1) return;

    CBrain* brain = obj->GetBrain();
    if (brain == nullptr) return;

    ObjectType type = obj->GetType();
    if (type == OBJECT_HUMAN) return;

    char fn[MAX_FNAME];
    strcpy(fn, filename);
    char* ldir = SearchLastDir(fn);
    if (ldir == 0) return;

    for (int i = 0; i < BRAINMAXSCRIPT; i++)
    {
        sprintf(ldir, "/prog%.3d%.1d.txt", objRank, i);
        brain->WriteProgram(i, fn);
    }
}

//! Saves the stack of the program in execution of a robot
bool CRobotMain::SaveFileStack(CObject *obj, FILE *file, int objRank)
{
    if (objRank == -1) return true;

    CBrain* brain = obj->GetBrain();
    if (brain == nullptr) return true;

    ObjectType type = obj->GetType();
    if (type == OBJECT_HUMAN) return true;

    return brain->WriteStack(file);
}

//! Resumes the execution stack of the program in a robot
bool CRobotMain::ReadFileStack(CObject *obj, FILE *file, int objRank)
{
    if (objRank == -1) return true;

    CBrain* brain = obj->GetBrain();
    if (brain == nullptr) return true;

    ObjectType type = obj->GetType();
    if (type == OBJECT_HUMAN) return true;

    return brain->ReadStack(file);
}


//! Empty the list
bool CRobotMain::FlushNewScriptName()
{
    for (int i = 0; i < MAXNEWSCRIPTNAME; i++)
        m_newScriptName[i].used = false;

    return true;
}

//! Adds a script name
bool CRobotMain::AddNewScriptName(ObjectType type, char *name)
{
    for (int i = 0; i < MAXNEWSCRIPTNAME; i++)
    {
        if (!m_newScriptName[i].used)
        {
            m_newScriptName[i].used = true;
            m_newScriptName[i].type = type;
            strcpy(m_newScriptName[i].name, name);
            return true;
        }
    }
    return false;
}

//! Seeks a script name for a given type
char*  CRobotMain::GetNewScriptName(ObjectType type, int rank)
{
    for (int i = 0; i < MAXNEWSCRIPTNAME; i++)
    {
        if (m_newScriptName[i].used &&
            (m_newScriptName[i].type == type        ||
             m_newScriptName[i].type == OBJECT_NULL))
        {
            if (rank == 0) return m_newScriptName[i].name;
            else           rank --;
        }
    }

    return 0;
}


//! Seeks if an object occupies in a spot, to prevent a backup of the game
bool CRobotMain::IsBusy()
{
    if (m_CompteurFileOpen > 0) return true;

    CInstanceManager* iMan = CInstanceManager::GetInstancePointer();

    for (int i = 0; i < 1000000; i++)
    {
        CObject* obj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, i));
        if (obj == nullptr) break;

        CBrain* brain = obj->GetBrain();
        if (brain != nullptr)
        {
            if (brain->IsBusy()) return true;
        }
    }
    return false;
}

//! Writes an object into the backup file
void CRobotMain::IOWriteObject(FILE *file, CObject* obj, const char *cmd)
{
    if (obj->GetType() == OBJECT_FIX) return;
    
    SetNumericLocale();

    char line[3000];
    char name[100];

    strcpy(line, cmd);

    sprintf(name, " type=%s", GetTypeObject(obj->GetType()));
    strcat(line, name);

    sprintf(name, " id=%d", obj->GetID());
    strcat(line, name);

    Math::Vector pos;

    pos = obj->GetPosition(0)/g_unit;
    sprintf(name, " pos=%.2f;%.2f;%.2f", pos.x, pos.y, pos.z);
    strcat(line, name);

    pos = obj->GetAngle(0)/(Math::PI/180.0f);
    sprintf(name, " angle=%.2f;%.2f;%.2f", pos.x, pos.y, pos.z);
    strcat(line, name);

    pos = obj->GetZoom(0);
    sprintf(name, " zoom=%.2f;%.2f;%.2f", pos.x, pos.y, pos.z);
    strcat(line, name);

    for (int i = 1; i < OBJECTMAXPART; i++)
    {
        if (obj->GetObjectRank(i) == -1) continue;

        pos = obj->GetPosition(i);
        if (pos.x != 0.0f || pos.y != 0.0f || pos.z != 0.0f)
        {
            pos /= g_unit;
            sprintf(name, " p%d=%.2f;%.2f;%.2f", i, pos.x, pos.y, pos.z);
            strcat(line, name);
        }

        pos = obj->GetAngle(i);
        if (pos.x != 0.0f || pos.y != 0.0f || pos.z != 0.0f)
        {
            pos /= (Math::PI/180.0f);
            sprintf(name, " a%d=%.2f;%.2f;%.2f", i, pos.x, pos.y, pos.z);
            strcat(line, name);
        }

        pos = obj->GetZoom(i);
        if (pos.x != 1.0f || pos.y != 1.0f || pos.z != 1.0f)
        {
            sprintf(name, " z%d=%.2f;%.2f;%.2f", i, pos.x, pos.y, pos.z);
            strcat(line, name);
        }
    }

    sprintf(name, " trainer=%d", obj->GetTrainer());
    strcat(line, name);

    sprintf(name, " option=%d", obj->GetOption());
    strcat(line, name);

    if (obj == m_infoObject)  // selects object?
    {
        sprintf(name, " select=1");
        strcat(line, name);
    }

    obj->Write(line);

    if (obj->GetType() == OBJECT_BASE)
    {
        sprintf(name, " run=3");  // stops and open (PARAM_FIXSCENE)
        strcat(line, name);
    }

    CBrain* brain = obj->GetBrain();
    if (brain != nullptr)
    {
        int run = brain->GetProgram();
        if (run != -1)
        {
            sprintf(name, " run=%d", run+1);
            strcat(line, name);
        }
    }

    strcat(line, "\n");
    fputs(line, file);
    
    RestoreNumericLocale();
}

//! Saves the current game
bool CRobotMain::IOWriteScene(const char *filename, const char *filecbot, char *info)
{
    FILE* file = fopen(filename, "w");
    if (file == NULL)  return false;
    
    SetNumericLocale();

    char line[500];

    sprintf(line, "Title text=\"%s\"\n", info);
    fputs(line, file);

    sprintf(line, "Version maj=%d min=%d\n", 0, 1);
    fputs(line, file);

    char* name = m_dialog->GetSceneName();
    if (strcmp(name, "user") == 0)
    {
        sprintf(line, "Mission base=\"%s\" rank=%.3d dir=\"%s\"\n", name, m_dialog->GetSceneRank(), m_dialog->GetSceneDir());
    }
    else
    {
        sprintf(line, "Mission base=\"%s\" rank=%.3d\n", name, m_dialog->GetSceneRank());
    }
    fputs(line, file);

    sprintf(line, "Map zoom=%.2f\n", m_map->GetZoomMap());
    fputs(line, file);

    sprintf(line, "DoneResearch bits=%d\n", static_cast<int>(g_researchDone));
    fputs(line, file);

    float sleep, delay, magnetic, progress;
    if (m_lightning->GetStatus(sleep, delay, magnetic, progress))
    {
        sprintf(line, "BlitzMode sleep=%.2f delay=%.2f magnetic=%.2f progress=%.2f\n", sleep, delay, magnetic/g_unit, progress);
        fputs(line, file);
    }

    CInstanceManager* iMan = CInstanceManager::GetInstancePointer();

    int objRank = 0;
    for (int i = 0; i < 1000000; i++)
    {
        CObject* obj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, i));
        if (obj == nullptr) break;

        if (obj->GetType() == OBJECT_TOTO) continue;
        if (obj->GetType() == OBJECT_FIX) continue;
        if (obj->GetTruck() != nullptr) continue;
        if (obj->GetBurn()) continue;
        if (obj->GetDead()) continue;
        if (obj->GetExplo()) continue;

        CObject* power = obj->GetPower();
        CObject* fret  = obj->GetFret();

        if (fret != nullptr)  // object transported?
            IOWriteObject(file, fret, "CreateFret");

        if (power != nullptr)  // battery transported?
            IOWriteObject(file, power, "CreatePower");

        IOWriteObject(file, obj, "CreateObject");

        SaveFileScript(obj, filename, objRank++);
    }
    fclose(file);
    
    RestoreNumericLocale();

#if CBOT_STACK
    // Writes the file of stacks of execution.
    file = fOpen(filecbot, "wb");
    if (file == NULL) return false;

    long version = 1;
    fWrite(&version, sizeof(long), 1, file);  // version of COLOBOT
    version = CBotProgram::GetVersion();
    fWrite(&version, sizeof(long), 1, file);  // version of CBOT

    objRank = 0;
    for (int i = 0; i < 1000000; i++)
    {
        CObject* obj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, i));
        if (obj == nullptr) break;

        if (obj->GetType() == OBJECT_TOTO) continue;
        if (obj->GetType() == OBJECT_FIX) continue;
        if (obj->GetTruck() != nullptr) continue;
        if (obj->GetBurn()) continue;
        if (obj->GetDead()) continue;

        if (!SaveFileStack(obj, file, objRank++))  break;
    }
    CBotClass::SaveStaticState(file);
    fClose(file);
#endif

    m_delayWriteMessage = 4;  // displays message in 3 frames
    return true;
}

//! Resumes the game
CObject* CRobotMain::IOReadObject(char *line, const char* filename, int objRank)
{
    Math::Vector pos  = OpDir(line, "pos")*g_unit;
    Math::Vector dir  = OpDir(line, "angle")*(Math::PI/180.0f);
    Math::Vector zoom = OpDir(line, "zoom");

    ObjectType type = OpTypeObject(line, "type", OBJECT_NULL);
    int id = OpInt(line, "id", 0);
    if (type == OBJECT_NULL)
        return nullptr;

    SetNumericLocale();
    
    int trainer = OpInt(line, "trainer", 0);
    int toy = OpInt(line, "toy", 0);
    int option = OpInt(line, "option", 0);

    CObject* obj = CreateObject(pos, dir.y, 1.0f, 0.0f, type, 0.0f, trainer, toy, option);
    obj->SetDefRank(objRank);
    obj->SetPosition(0, pos);
    obj->SetAngle(0, dir);
    obj->SetID(id);
    if (g_id < id) g_id = id;

    if (zoom.x != 0.0f || zoom.y != 0.0f || zoom.z != 0.0f)
        obj->SetZoom(0, zoom);

    for (int i = 1; i < OBJECTMAXPART; i++)
    {
        if (obj->GetObjectRank(i) == -1) continue;

        char op[10];
        sprintf(op, "p%d", i);
        pos = OpDir(line, op);
        if (pos.x != 0.0f || pos.y != 0.0f || pos.z != 0.0f)
        {
            obj->SetPosition(i, pos*g_unit);
        }

        sprintf(op, "a%d", i);
        dir = OpDir(line, op);
        if (dir.x != 0.0f || dir.y != 0.0f || dir.z != 0.0f)
        {
            obj->SetAngle(i, dir*(Math::PI/180.0f));
        }

        sprintf(op, "z%d", i);
        zoom = OpDir(line, op);
        if (zoom.x != 0.0f || zoom.y != 0.0f || zoom.z != 0.0f)
        {
            obj->SetZoom(i, zoom);
        }
    }

    if (type == OBJECT_BASE) m_base = true;

    obj->Read(line);

#if CBOT_STACK
#else
    LoadFileScript(obj, filename, objRank, i);
#endif

    int run = OpInt(line, "run", -1);
    if (run != -1)
    {
#if CBOT_STACK
#else
        CBrain* brain = obj->GetBrain();
        if (brain != nullptr)
            brain->RunProgram(run-1);  // starts the program
#endif

        CAuto* automat = obj->GetAuto();
        if (automat != nullptr)
            automat->Start(run);  // starts the film
    }

    RestoreNumericLocale();
    
    return obj;
}

//! Resumes some part of the game
CObject* CRobotMain::IOReadScene(const char *filename, const char *filecbot)
{
    m_base = false;

    FILE* file = fopen(filename, "r");
    if (file == NULL) return 0;
    
    SetNumericLocale();

    CObject* fret   = nullptr;
    CObject* power  = nullptr;
    CObject* sel    = nullptr;
    int objRank = 0;
    char line[3000];
    while (fgets(line, 3000, file) != NULL)
    {
        for (int i = 0; i < 3000; i++)
        {
            if (line[i] == '\t') line[i] = ' ';  // replace tab by space
            if (line[i] == '/' && line[i+1] == '/')
            {
                line[i] = 0;
                break;
            }
        }

        if (Cmd(line, "Map"))
            m_map->ZoomMap(OpFloat(line, "zoom", 1.0f));

        if (Cmd(line, "DoneResearch"))
            g_researchDone = OpInt(line, "bits", 0);

        if (Cmd(line, "BlitzMode"))
        {
            float sleep = OpFloat(line, "sleep", 0.0f);
            float delay = OpFloat(line, "delay", 3.0f);
            float magnetic = OpFloat(line, "magnetic", 50.0f)*g_unit;
            float progress = OpFloat(line, "progress", 0.0f);
            m_lightning->SetStatus(sleep, delay, magnetic, progress);
        }

        if (Cmd(line, "CreateFret"))
            fret = IOReadObject(line, filename, -1);

        if (Cmd(line, "CreatePower"))
            power = IOReadObject(line, filename, -1);

        if (Cmd(line, "CreateObject"))
        {
            CObject* obj = IOReadObject(line, filename, objRank++);

            if (OpInt(line, "select", 0))
                sel = obj;

            if (fret != nullptr)
            {
                obj->SetFret(fret);
                CTaskManip* task = new CTaskManip(obj);
                task->Start(TMO_AUTO, TMA_GRAB);  // holds the object!
                delete task;
            }

            if (power != nullptr)
            {
                obj->SetPower(power);
                power->SetTruck(obj);
            }

            fret  = nullptr;
            power = nullptr;
        }
    }
    fclose(file);

#if CBOT_STACK
    CInstanceManager* iMan = CInstanceManager::GetInstancePointer();

    // Compiles scripts.
    int nbError = 0;
    int lastError = 0;
    do
    {
        lastError = nbError;
        nbError = 0;
        for (int i = 0; i < 1000000; i++)
        {
            CObject* obj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, i));
            if (obj == nullptr) break;
            if (obj->GetTruck() != nullptr) continue;

            objRank = obj->GetDefRank();
            if (objRank == -1) continue;

            LoadFileScript(obj, filename, objRank, nbError);
        }
    }
    while (nbError > 0 && nbError != lastError);

    // Reads the file of stacks of execution.
    file = fOpen(filecbot, "rb");
    if (file != NULL)
    {
        long version;
        fRead(&version, sizeof(long), 1, file);  // version of COLOBOT
        if (version == 1)
        {
            fRead(&version, sizeof(long), 1, file);  // version of CBOT
            if (version == CBotProgram::GetVersion())
            {
                objRank = 0;
                for (int i = 0; i < 1000000; i++)
                {
                    CObject* obj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, i));
                    if (obj == nullptr) break;

                    if (obj->GetType() == OBJECT_TOTO) continue;
                    if (obj->GetType() == OBJECT_FIX) continue;
                    if (obj->GetTruck() != nullptr) continue;
                    if (obj->GetBurn()) continue;
                    if (obj->GetDead()) continue;

                    if (!ReadFileStack(obj, file, objRank++)) break;
                }
            }
        }
        CBotClass::RestoreStaticState(file);
        fClose(file);
    }
#endif

    RestoreNumericLocale();

    return sel;
}


//! Writes the global parameters for free play
void CRobotMain::WriteFreeParam()
{
    m_freeResearch |= g_researchDone;
    m_freeBuild    |= g_build;

    if (m_gamerName[0] == 0) return;

    char filename[MAX_FNAME];
    sprintf(filename, "%s/%s/research.gam", GetSavegameDir(), m_gamerName);
    FILE* file = fopen(filename, "w");
    if (file == NULL) return;

    char line[100];
    sprintf(line, "research=%d build=%d\n", m_freeResearch, m_freeBuild);
    fputs(line, file);
    fclose(file);
}

//! Reads the global parameters for free play
void CRobotMain::ReadFreeParam()
{
    m_freeResearch = 0;
    m_freeBuild    = 0;

    if (m_gamerName[0] == 0) return;

    char filename[MAX_FNAME];
    sprintf(filename, "%s/%s/research.gam", GetSavegameDir(), m_gamerName);
    FILE* file = fopen(filename, "r");
    if (file == NULL)  return;

    char line[100];
    if (fgets(line, 100, file) != NULL)
        sscanf(line, "research=%d build=%d\n", &m_freeResearch, &m_freeBuild);

    fclose(file);
}


//! Resets all objects to their original position
void CRobotMain::ResetObject()
{
// TODO: ?
#if 0
    CObject*    obj;
    CObject*    truck;
    CAuto*      objAuto;
    CBrain*     brain;
    CPyro*      pyro;
    ResetCap    cap;
    Math::Vector    pos, angle;
    int         i;

    CInstanceManager* iMan = CInstanceManager::GetInstancePointer();

    // Removes all pyrotechnic effects in progress.
    while ( true )
    {
        pyro = static_cast<CPyro*>(iMan->SearchInstance(CLASS_PYRO, 0));
        if ( pyro == 0 )  break;

        pyro->DeleteObject();
        delete pyro;
    }

    // Removes all bullets in progress.
    m_particle->DeleteParticle(PARTIGUN1);
    m_particle->DeleteParticle(PARTIGUN2);
    m_particle->DeleteParticle(PARTIGUN3);
    m_particle->DeleteParticle(PARTIGUN4);

    for ( i=0 ; i<1000000 ; i++ )
    {
        obj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, i));
        if ( obj == 0 )  break;

        cap = obj->GetResetCap();
        if ( cap == RESET_NONE )  continue;

        if ( cap == RESET_DELETE )
        {
            truck = obj->GetTruck();
            if ( truck != 0 )
            {
                truck->SetFret(0);
                obj->SetTruck(0);
            }
            obj->DeleteObject();
            delete obj;
            i --;
            continue;
        }

        objAuto = obj->GetAuto();
        if ( objAuto != 0 )
        {
            objAuto->Abort();
        }

        if ( obj->GetEnable() )  // object still active?
        {
            brain = obj->GetBrain();
            if ( brain != 0 )
            {
                pos   = obj->GetResetPosition();
                angle = obj->GetResetAngle();

                if ( pos   == obj->GetPosition(0) &&
                     angle == obj->GetAngle(0)    )  continue;
                brain->StartTaskReset(pos, angle);
                continue;
            }
        }

        obj->SetEnable(true);  // active again

        pos   = obj->GetResetPosition();
        angle = obj->GetResetAngle();

        if ( pos   == obj->GetPosition(0) &&
             angle == obj->GetAngle(0)    )  continue;

        pyro = new CPyro();
        pyro->Create(PT_RESET, obj);

        brain = obj->GetBrain();
        if ( brain != 0 )
        {
            brain->RunProgram(obj->GetResetRun());
        }
    }
#else
    m_resetCreate = true;
#endif
}

//! Resets all objects to their original position
void CRobotMain::ResetCreate()
{
    SaveAllScript();

    // Removes all bullets in progress.
    m_particle->DeleteParticle(Gfx::PARTIGUN1);
    m_particle->DeleteParticle(Gfx::PARTIGUN2);
    m_particle->DeleteParticle(Gfx::PARTIGUN3);
    m_particle->DeleteParticle(Gfx::PARTIGUN4);

    DeselectAll();  // removes the control buttons
    DeleteAllObjects();  // removes all the current 3D Scene

    m_particle->FlushParticle();
    m_terrain->FlushBuildingLevel();

    CInstanceManager* iMan = CInstanceManager::GetInstancePointer();
    iMan->Flush(CLASS_OBJECT);
    iMan->Flush(CLASS_PHYSICS);
    iMan->Flush(CLASS_BRAIN);
    iMan->Flush(CLASS_PYRO);

    m_camera->SetType(Gfx::CAM_TYPE_DIALOG);

    CreateScene(m_dialog->GetSceneSoluce(), false, true);

    if (!GetNiceReset()) return;

    for (int i = 0; i < 1000000; i++)
    {
        CObject* obj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, i));
        if (obj == nullptr) break;

        ResetCap cap = obj->GetResetCap();
        if (cap == RESET_NONE) continue;

        Gfx::CPyro* pyro = new Gfx::CPyro();
        pyro->Create(Gfx::PT_RESET, obj);
    }
}

//! Checks if the mission is over
Error CRobotMain::CheckEndMission(bool frame)
{
    CInstanceManager* iMan = CInstanceManager::GetInstancePointer();

    for (int t = 0; t < m_endTakeTotal; t++)
    {
        if (m_endTake[t].message[0] != 0) continue;

        Math::Vector bPos = m_endTake[t].pos;
        bPos.y = 0.0f;

        Math::Vector oPos;

        int nb = 0;
        for (int i = 0; i < 1000000; i++)
        {
            CObject* obj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, i));
            if (obj == nullptr) break;

            // Do not use GetActif () because an invisible worm (underground)
            // should be regarded as existing here!
            if (obj->GetLock()) continue;
            if (obj->GetRuin()) continue;
            if (!obj->GetEnable()) continue;

            ObjectType type = obj->GetType();
            if (type == OBJECT_SCRAP2 ||
                type == OBJECT_SCRAP3 ||
                type == OBJECT_SCRAP4 ||
                type == OBJECT_SCRAP5)  // wastes?
            {
                type = OBJECT_SCRAP1;
            }

            if (type != m_endTake[t].type)  continue;

            if (obj->GetTruck() == 0)
                oPos = obj->GetPosition(0);
            else
                oPos = obj->GetTruck()->GetPosition(0);

            oPos.y = 0.0f;

            if (Math::DistanceProjected(oPos, bPos) <= m_endTake[t].dist)
                nb ++;
        }

        if (nb <= m_endTake[t].lost)
        {
            if (m_endTake[t].type == OBJECT_HUMAN)
            {
                if (m_lostDelay == 0.0f)
                {
                    m_lostDelay = 0.1f;  // lost immediately
                    m_winDelay  = 0.0f;
                }
                m_displayText->SetEnable(false);
                return INFO_LOSTq;
            }
            else
            {
                if (m_lostDelay == 0.0f)
                {
                    m_displayText->DisplayError(INFO_LOST, Math::Vector(0.0f,0.0f,0.0f));
                    m_lostDelay = m_endTakeLostDelay;  // lost in 6 seconds
                    m_winDelay  = 0.0f;
                }
                m_displayText->SetEnable(false);
                return INFO_LOST;
            }
        }
        if (nb < m_endTake[t].min ||
            nb > m_endTake[t].max)
        {
            m_displayText->SetEnable(true);
            return ERR_MISSION_NOTERM;
        }
        if (m_endTake[t].immediat)
        {
            if (m_winDelay == 0.0f)
            {
                m_winDelay  = m_endTakeWinDelay;  // wins in x seconds
                m_lostDelay = 0.0f;
            }
            m_displayText->SetEnable(false);
            return ERR_OK;  // mission ended
        }
    }

    if (m_endTakeResearch != 0)
    {
        if (m_endTakeResearch != (m_endTakeResearch&g_researchDone))
        {
            m_displayText->SetEnable(true);
            return ERR_MISSION_NOTERM;
        }
    }

    if (m_endTakeWinDelay == -1.0f)
    {
        m_winDelay  = 1.0f;  // wins in one second
        m_lostDelay = 0.0f;
        m_displayText->SetEnable(false);
        return ERR_OK;  // mission ended
    }

    if (frame && m_base) return ERR_MISSION_NOTERM;

    if (m_winDelay == 0.0f)
    {
        m_displayText->DisplayError(INFO_WIN, Math::Vector(0.0f,0.0f,0.0f));
        m_winDelay  = m_endTakeWinDelay;  // wins in two seconds
        m_lostDelay = 0.0f;
    }
    m_displayText->SetEnable(false);
    return ERR_OK;  // mission ended
}

//! Checks if the mission is finished after displaying a message
void CRobotMain::CheckEndMessage(const char* message)
{
    for (int t = 0; t < m_endTakeTotal; t++)
    {
        if (m_endTake[t].message[0] == 0) continue;

        if (strcmp(m_endTake[t].message, message) == 0)
        {
            m_displayText->DisplayError(INFO_WIN, Math::Vector(0.0f,0.0f,0.0f));
            m_winDelay  = m_endTakeWinDelay;  // wins in 2 seconds
            m_lostDelay = 0.0f;
        }
    }
}


//! Returns the number of instructions required
int CRobotMain::GetObligatoryToken()
{
    return m_obligatoryTotal;
}

//! Returns the name of a required instruction
char* CRobotMain::GetObligatoryToken(int i)
{
    return m_obligatoryToken[i];
}

//! Checks if an instruction is part of the obligatory list
int CRobotMain::IsObligatoryToken(const char* token)
{
    for (int i = 0; i < m_obligatoryTotal; i++)
    {
        if (strcmp(token, m_obligatoryToken[i]) == 0)
            return i;
    }
    return -1;
}

//! Checks if an instruction is not part of the banned list
bool CRobotMain::IsProhibitedToken(const char* token)
{
    for (int i = 0; i < m_prohibitedTotal; i++)
    {
        if (strcmp(token, m_prohibitedToken[i]) == 0)
            return false;
    }
    return true;
}


//! Indicates whether it is possible to control a driving robot
bool CRobotMain::GetTrainerPilot()
{
    return m_trainerPilot;
}

//! Indicates whether the scene is fixed, without interaction
bool CRobotMain::GetFixScene()
{
    return m_fixScene;
}


char* CRobotMain::GetTitle()
{
    return m_title;
}

char* CRobotMain::GetResume()
{
    return m_resume;
}

char* CRobotMain::GetScriptName()
{
    return m_scriptName;
}

char* CRobotMain::GetScriptFile()
{
    return m_scriptFile;
}


bool CRobotMain::GetGlint()
{
    return m_dialog->GetGlint();
}

bool CRobotMain::GetSoluce4()
{
    return m_dialog->GetSoluce4();
}

bool CRobotMain::GetMovies()
{
    return m_dialog->GetMovies();
}

bool CRobotMain::GetNiceReset()
{
    return m_dialog->GetNiceReset();
}

bool CRobotMain::GetHimselfDamage()
{
    return m_dialog->GetHimselfDamage();
}

bool CRobotMain::GetShowSoluce()
{
    return m_showSoluce;
}

bool CRobotMain::GetSceneSoluce()
{
    if (m_infoFilename[SATCOM_SOLUCE][0] == 0) return false;
    return m_dialog->GetSceneSoluce();
}

bool CRobotMain::GetShowAll()
{
    return m_showAll;
}

bool CRobotMain::GetRadar()
{
    if (m_cheatRadar)
        return true;

    CInstanceManager* iMan = CInstanceManager::GetInstancePointer();

    for (int i = 0; i < 1000000; i++)
    {
        CObject* obj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, i));
        if (obj == nullptr)  break;

        ObjectType type = obj->GetType();
        if (type == OBJECT_RADAR && !obj->GetLock())
            return true;
    }
    return false;
}

const char* CRobotMain::GetSavegameDir()
{
    return m_dialog->GetSavegameDir().c_str();
}

const char* CRobotMain::GetPublicDir()
{
    return m_dialog->GetPublicDir().c_str();
}

const char* CRobotMain::GetFilesDir()
{
    return m_dialog->GetFilesDir().c_str();
}

bool CRobotMain::GetRetroMode()
{
    return m_retroStyle;
}

//! Change the player's name
void CRobotMain::SetGamerName(const char *name)
{
    strcpy(m_gamerName, name);
    SetGlobalGamerName(m_gamerName);
    ReadFreeParam();
}

//! Getes the player's name
char* CRobotMain::GetGamerName()
{
    return m_gamerName;
}


//! Returns the representation to use for the player
int CRobotMain::GetGamerFace()
{
    return m_dialog->GetGamerFace();
}

//! Returns the representation to use for the player
int CRobotMain::GetGamerGlasses()
{
    return m_dialog->GetGamerGlasses();
}

//! Returns the mode with just the head
bool CRobotMain::GetGamerOnlyHead()
{
    return m_dialog->GetGamerOnlyHead();
}

//! Returns the angle of presentation
float CRobotMain::GetPersoAngle()
{
    return m_dialog->GetPersoAngle();
}


//! Changes on the pause mode
void CRobotMain::ChangePause(bool pause)
{
    m_pause = pause;
    m_engine->SetPause(m_pause);

    m_sound->MuteAll(m_pause);
    CreateShortcuts();
    if (m_pause) HiliteClear();
}


//! Changes game speed
void CRobotMain::SetSpeed(float speed)
{
    m_app->SetSimulationSpeed(speed);
    UpdateSpeedLabel();
}

float CRobotMain::GetSpeed()
{
    return m_app->GetSimulationSpeed();
}

void CRobotMain::UpdateSpeedLabel()
{
    Ui::CButton* pb = static_cast<Ui::CButton*>(m_interface->SearchControl(EVENT_SPEED));
    float speed = m_app->GetSimulationSpeed();

    if (pb != nullptr)
    {
        if (speed == 1.0f)
        {
            pb->ClearState(Ui::STATE_VISIBLE);
        }
        else
        {
            char text[10];
            sprintf(text, "x%.1f", speed);
            pb->SetName(text);
            pb->SetState(Ui::STATE_VISIBLE);
        }
    }

}


//! Creates interface shortcuts to the units
bool CRobotMain::CreateShortcuts()
{
    if (m_phase != PHASE_SIMUL) return false;
    if (!m_shortCut) return false;
    return m_short->CreateShortcuts();
}

//! Updates the map
void CRobotMain::UpdateMap()
{
    m_map->UpdateMap();
}

//! Indicates whether the mini-map is visible
bool CRobotMain::GetShowMap()
{
    return m_map->GetShowMap() && m_mapShow;
}


//! Management of the lock mode for movies
void CRobotMain::SetMovieLock(bool lock)
{
    m_movieLock = lock;
    m_engine->SetMovieLock(m_movieLock);

    CreateShortcuts();
    m_map->ShowMap(!m_movieLock && m_mapShow);
    if (m_movieLock) HiliteClear();

    if (m_movieLock)
        m_app->SetMouseMode(MOUSE_NONE);
    else
        m_app->SetMouseMode(MOUSE_ENGINE);
}

bool CRobotMain::GetMovieLock()
{
    return m_movieLock;
}

bool CRobotMain::GetInfoLock()
{
    return m_displayInfo != nullptr;  // info in progress?
}

//! Management of the blocking of the call of SatCom
void CRobotMain::SetSatComLock(bool lock)
{
    m_satComLock = lock;
}

bool CRobotMain::GetSatComLock()
{
    return m_satComLock;
}

//! Management of the lock mode for the edition
void CRobotMain::SetEditLock(bool lock, bool edit)
{
    m_editLock = lock;

    CreateShortcuts();

    // Do not remove the card if it contains a still image.
    if (!lock || !m_map->GetFixImage())
        m_map->ShowMap(!m_editLock && m_mapShow);

    m_displayText->HideText(lock);
    m_app->ResetKeyStates();

    if (m_editLock)
        HiliteClear();
    else
        m_editFull = false;
}

bool CRobotMain::GetEditLock()
{
    return m_editLock;
}

//! Management of the fullscreen mode during editing
void CRobotMain::SetEditFull(bool full)
{
    m_editFull = full;
}

bool CRobotMain::GetEditFull()
{
    return m_editFull;
}


bool CRobotMain::GetFreePhoto()
{
    return m_freePhoto;
}


//! Indicates whether mouse is on an friend object, on which we should not shoot
void CRobotMain::SetFriendAim(bool friendAim)
{
    m_friendAim = friendAim;
}

bool CRobotMain::GetFriendAim()
{
    return m_friendAim;
}


//! Management of the precision of drawing the ground
void CRobotMain::SetTracePrecision(float factor)
{
    m_engine->SetTracePrecision(factor);
}

float CRobotMain::GetTracePrecision()
{
    return m_engine->GetTracePrecision();
}


//! Starts music with a mission
void CRobotMain::StartMusic()
{
    if (m_audioTrack != 0)
    {
        m_sound->StopMusic();
        m_sound->PlayMusic(m_audioTrack, m_audioRepeat);
    }
}

//! Removes hilite and tooltip
void CRobotMain::ClearInterface()
{
    HiliteClear();  // removes setting evidence
    m_tooltipName[0] = 0;  // really removes the tooltip
}

void CRobotMain::SetNumericLocale()
{
    char *locale = setlocale(LC_NUMERIC, nullptr);
    if (locale != nullptr)
        m_oldLocale = locale;

    setlocale(LC_NUMERIC, "C");
}

void CRobotMain::RestoreNumericLocale()
{
    setlocale(LC_NUMERIC, m_oldLocale.c_str());
}
    