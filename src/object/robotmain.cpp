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

// robotmain.cpp


#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "CBot/CBotDll.h"
#include "common/struct.h"
#include "math/const.h"
#include "math/geometry.h"
#include "math/conv.h"
#include "old/d3dengine.h"
#include "old/d3dmath.h"
#include "common/language.h"
#include "common/global.h"
#include "common/event.h"
#include "common/misc.h"
#include "common/profile.h"
#include "common/iman.h"
#include "common/restext.h"
#include "old/math3d.h"
#include "old/light.h"
#include "old/particule.h"
#include "old/terrain.h"
#include "old/water.h"
#include "old/cloud.h"
#include "old/blitz.h"
#include "old/planet.h"
#include "object/object.h"
#include "object/motion/motion.h"
#include "object/motion/motiontoto.h"
#include "object/motion/motionhuman.h"
#include "physics/physics.h"
#include "object/brain.h"
#include "old/pyro.h"
#include "old/modfile.h"
#include "old/model.h"
#include "old/camera.h"
#include "object/task/task.h"
#include "object/task/taskmanip.h"
#include "object/task/taskbuild.h"
#include "object/auto/auto.h"
#include "object/auto/autobase.h"
#include "ui/displayinfo.h"
#include "ui/interface.h"
#include "ui/shortcut.h"
#include "ui/map.h"
#include "ui/label.h"
#include "ui/button.h"
#include "ui/slider.h"
#include "ui/window.h"
#include "ui/edit.h"
#include "ui/displaytext.h"
#include "old/text.h"
#include "old/sound.h"
#include "script/cbottoken.h"
#include "script/cmdtoken.h"
#include "object/mainmovie.h"
#include "ui/maindialog.h"
#include "ui/mainshort.h"
#include "ui/mainmap.h"
#include "script/script.h"
#include "object/robotmain.h"



#define CBOT_STACK  true    // saves the stack of programs CBOT
const float UNIT = 4.0f;



// Global variables.

long    g_id;               // unique identifier
long    g_build;            // constructible buildings
long    g_researchDone;         // research done
long    g_researchEnable;       // research available
float   g_unit;             // conversion factor



// Static variables

static CBotClass*   m_pClassFILE;
static CBotProgram* m_pFuncFile;
static int          m_CompteurFileOpen = 0;
static char*        m_filesDir;



// Prepares a file name.

void PrepareFilename(CBotString &filename)
{
    int         pos;

    pos = filename.ReverseFind('\\');
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

    filename = CBotString(m_filesDir) + CBotString("\\") + filename;
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
        pVar->SetValInt((long)pFile);
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

    FILE* pFile= (FILE*)pVar->GetValInt();
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
    pVar->SetValInt((long)pFile);

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

    FILE* pFile= (FILE*)pVar->GetValInt();
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

    FILE* pFile= (FILE*)pVar->GetValInt();

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

    FILE* pFile= (FILE*)pVar->GetValInt();

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

    FILE* pFile= (FILE*)pVar->GetValInt();

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




// Constructor of robot application.

CRobotMain::CRobotMain(CInstanceManager* iMan)
{
    ObjectType  type;
    float       fValue;
    int         iValue, i;
    char*       token;

    m_iMan = iMan;
    m_iMan->AddInstance(CLASS_MAIN, this);

    m_event     = (CEvent*)m_iMan->SearchInstance(CLASS_EVENT);
    m_engine    = (CD3DEngine*)m_iMan->SearchInstance(CLASS_ENGINE);
    m_light     = (CLight*)m_iMan->SearchInstance(CLASS_LIGHT);
    m_particule = (CParticule*)m_iMan->SearchInstance(CLASS_PARTICULE);
    m_water     = (CWater*)m_iMan->SearchInstance(CLASS_WATER);
    m_cloud     = (CCloud*)m_iMan->SearchInstance(CLASS_CLOUD);
    m_blitz     = (CBlitz*)m_iMan->SearchInstance(CLASS_BLITZ);
    m_planet    = (CPlanet*)m_iMan->SearchInstance(CLASS_PLANET);
    m_sound     = (CSound*)m_iMan->SearchInstance(CLASS_SOUND);

    m_interface   = new CInterface(m_iMan);
    m_terrain     = new CTerrain(m_iMan);
    m_model       = new CModel(m_iMan);
    m_camera      = new CCamera(m_iMan);
    m_displayText = new CDisplayText(m_iMan);
    m_movie       = new CMainMovie(m_iMan);
    m_dialog      = new CMainDialog(m_iMan);
    m_short       = new CMainShort(m_iMan);
    m_map         = new CMainMap(m_iMan);
    m_displayInfo = 0;

    m_engine->SetTerrain(m_terrain);
    m_filesDir = m_dialog->RetFilesDir();

    m_time = 0.0f;
    m_gameTime = 0.0f;
    m_checkEndTime = 0.0f;

    m_phase       = PHASE_NAME;
    m_cameraRank  = -1;
    m_visitLast   = EVENT_NULL;
    m_visitObject = 0;
    m_visitArrow  = 0;
    m_audioTrack  = 0;
    m_bAudioRepeat = true;
    m_delayWriteMessage = 0;
    m_selectObject = 0;
    m_infoUsed     = 0;

    m_bBeginSatCom  = false;
    m_bMovieLock    = false;
    m_bSatComLock   = false;
    m_bEditLock     = false;
    m_bEditFull     = false;
    m_bPause        = false;
    m_bHilite       = false;
    m_bFreePhoto    = false;
    m_bShowPos      = false;
    m_bSelectInsect = false;
    m_bShowSoluce   = false;
    m_bShowAll      = false;
    m_bCheatRadar   = false;
    m_bFixScene     = false;
    m_bTrainerPilot = false;
    m_bSuspend      = false;
    m_bFriendAim    = false;
    m_bResetCreate  = false;
    m_bShortCut     = true;

    m_engine->SetMovieLock(m_bMovieLock);

    m_movie->Flush();
    m_movieInfoIndex = -1;

    m_tooltipPos = Math::Point(0.0f, 0.0f);
    m_tooltipName[0] = 0;
    m_tooltipTime = 0.0f;

    m_endingWinRank   = 0;
    m_endingLostRank  = 0;
    m_bWinTerminate   = false;

    FlushDisplayInfo();

    m_fontSize  = 9.0f;
    m_windowPos = Math::Point(0.15f, 0.17f);
    m_windowDim = Math::Point(0.70f, 0.66f);

    if ( GetLocalProfileFloat("Edit", "FontSize",    fValue) )  m_fontSize    = fValue;
    if ( GetLocalProfileFloat("Edit", "WindowPos.x", fValue) )  m_windowPos.x = fValue;
    if ( GetLocalProfileFloat("Edit", "WindowPos.y", fValue) )  m_windowPos.y = fValue;
    if ( GetLocalProfileFloat("Edit", "WindowDim.x", fValue) )  m_windowDim.x = fValue;
    if ( GetLocalProfileFloat("Edit", "WindowDim.y", fValue) )  m_windowDim.y = fValue;

    m_IOPublic = false;
    m_IODim = Math::Point(320.0f/640.0f, (121.0f+18.0f*8)/480.0f);
    m_IOPos.x = (1.0f-m_IODim.x)/2.0f;  // in the middle
    m_IOPos.y = (1.0f-m_IODim.y)/2.0f;

    if ( GetLocalProfileInt  ("Edit", "IOPublic", iValue) )  m_IOPublic = iValue;
    if ( GetLocalProfileFloat("Edit", "IOPos.x",  fValue) )  m_IOPos.x  = fValue;
    if ( GetLocalProfileFloat("Edit", "IOPos.y",  fValue) )  m_IOPos.y  = fValue;
    if ( GetLocalProfileFloat("Edit", "IODim.x",  fValue) )  m_IODim.x  = fValue;
    if ( GetLocalProfileFloat("Edit", "IODim.y",  fValue) )  m_IODim.y  = fValue;

    m_short->FlushShortcuts();
    InitEye();

    m_engine->SetTracePrecision(1.0f);

    m_cameraPan  = 0.0f;
    m_cameraZoom = 0.0f;

    g_id = 0;
    g_build = 0;
    g_researchDone = 0;  // no research done
    g_researchEnable = 0;
    g_unit = 4.0f;

    m_gamerName[0] = 0;
    GetProfile()->GetLocalProfileString("Gamer", "LastName", m_gamerName, 100);
    SetGlobalGamerName(m_gamerName);
    ReadFreeParam();
    m_dialog->SetupRecall();

    for ( i=0 ; i<MAXSHOWLIMIT ; i++ )
    {
        m_showLimit[i].bUsed = false;
        m_showLimit[i].total = 0;
        m_showLimit[i].link = 0;
    }

    CBotProgram::SetTimer(100);
    CBotProgram::Init();

    for ( i=0 ; i<OBJECT_MAX ; i++ )
    {
        type = (ObjectType)i;
        token = RetObjectName(type);
        if ( token[0] != 0 )
        {
            CBotProgram::DefineNum(token, type);
        }
        token = RetObjectAlias(type);
        if ( token[0] != 0 )
        {
            CBotProgram::DefineNum(token, type);
        }
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

    CBotProgram::DefineNum("DisplayError",   TT_ERROR);
    CBotProgram::DefineNum("DisplayWarning", TT_WARNING);
    CBotProgram::DefineNum("DisplayInfo",    TT_INFO);
    CBotProgram::DefineNum("DisplayMessage", TT_MESSAGE);

    CBotProgram::DefineNum("FilterNone",        FILTER_NONE);
    CBotProgram::DefineNum("FilterOnlyLanding", FILTER_ONLYLANDING);
    CBotProgram::DefineNum("FilterOnlyFliying", FILTER_ONLYFLYING);

    // Add the class Point.
    CBotClass* bc;
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

    // Initializes the class FILE.
    InitClassFILE();

    CScript::InitFonctions();
}

// Destructor of robot application.

CRobotMain::~CRobotMain()
{
    delete m_movie;
    delete m_dialog;
    delete m_short;
    delete m_map;
    delete m_terrain;
    delete m_model;
}


// Creates the file colobot.ini at the first time.

void CRobotMain::CreateIni()
{
    int     iValue;

    // colobot.ini don't exist?
    if ( !GetLocalProfileInt("Setup", "TotoMode", iValue) )
    {
        m_dialog->SetupMemorize();
    }
}


// Changes phase.

void CRobotMain::ChangePhase(Phase phase)
{
    CEdit*          pe;
    CButton*        pb;
    D3DCOLORVALUE   color;
    Math::Point         pos, dim, ddim;
    float           ox, oy, sx, sy;
    char*           read;
    int             rank, numTry;
    bool            bLoading;

    if ( m_phase == PHASE_SIMUL )  // ends a simulation?
    {
        SaveAllScript();
        m_sound->StopMusic();
        m_camera->SetObject(0);

#if _SCHOOL
        if ( true )
#else
        if ( m_gameTime > 10.0f )  // did you play at least 10 seconds?
#endif
        {
            rank = m_dialog->RetSceneRank();
            numTry = m_dialog->RetGamerInfoTry(rank);
            m_dialog->SetGamerInfoTry(rank, numTry+1);
            m_dialog->WriteGamerInfo();
        }
    }

    if ( phase == PHASE_WIN )  // wins a simulation?
    {
        rank = m_dialog->RetSceneRank();
        m_dialog->SetGamerInfoPassed(rank, true);
        m_dialog->NextMission();  // passes to the next mission
        m_dialog->WriteGamerInfo();
    }

    DeleteAllObjects();  // removes all the current 3D Scene

    m_phase        = phase;
    m_winDelay     = 0.0f;
    m_lostDelay    = 0.0f;
    m_bBeginSatCom = false;
    m_bMovieLock   = false;
    m_bSatComLock  = false;
    m_bEditLock    = false;
    m_bFreePhoto   = false;
    m_bResetCreate = false;

    m_engine->SetMovieLock(m_bMovieLock);
    ChangePause(false);
    FlushDisplayInfo();
    m_engine->SetRankView(0);
    m_engine->FlushObject();
    color.r = color.g = color.b = color.a = 0.0f;
    m_engine->SetWaterAddColor(color);
    m_engine->SetBackground("");
    m_engine->SetBackForce(false);
    m_engine->SetFrontsizeName("");
    m_engine->SetOverColor();
    m_engine->GroundMarkDelete(0);
    SetSpeed(1.0f);
    m_terrain->SetWind(Math::Vector(0.0f, 0.0f, 0.0f));
    m_terrain->FlushBuildingLevel();
    m_terrain->FlushFlyingLimit();
    m_light->FlushLight();
    m_particule->FlushParticule();
    m_water->Flush();
    m_cloud->Flush();
    m_blitz->Flush();
    m_planet->Flush();
    m_iMan->Flush(CLASS_OBJECT);
    m_iMan->Flush(CLASS_PHYSICS);
    m_iMan->Flush(CLASS_BRAIN);
    m_iMan->Flush(CLASS_PYRO);
    m_model->StopUserAction();
    m_interface->Flush();
    ClearInterface();
    FlushNewScriptName();
    m_sound->SetListener(Math::Vector(0.0f, 0.0f, 0.0f), Math::Vector(0.0f, 0.0f, 1.0f));
    m_camera->SetType(CAMERA_DIALOG);
    m_movie->Flush();
    m_movieInfoIndex = -1;
    m_cameraPan  = 0.0f;
    m_cameraZoom = 0.0f;
    m_bShortCut = true;

    // Creates and hide the command console.
    dim.x = 200.0f/640.0f;
    dim.y =  18.0f/480.0f;
    pos.x =  50.0f/640.0f;
    pos.y = 452.0f/480.0f;
    pe = m_interface->CreateEdit(pos, dim, 0, EVENT_CMD);
    if ( pe == 0 )  return;
    pe->ClearState(STATE_VISIBLE);
    m_bCmdEdit = false;  // hidden for now

    // Creates the speedometer.
#if _TEEN
    dim.x =  30.0f/640.0f;
    dim.y =  20.0f/480.0f;
    pos.x =   4.0f/640.0f;
    pos.y = 454.0f/480.0f;
#else
    dim.x =  30.0f/640.0f;
    dim.y =  20.0f/480.0f;
    pos.x =   4.0f/640.0f;
    pos.y = 426.0f/480.0f;
#endif
    pb = m_interface->CreateButton(pos, dim, 0, EVENT_SPEED);
    if ( pb == 0 )  return;
    pb->SetState(STATE_SIMPLY);
    pb->ClearState(STATE_VISIBLE);

    m_dialog->ChangePhase(m_phase);

    dim.x = 32.0f/640.0f;
    dim.y = 32.0f/480.0f;
    ox = 3.0f/640.0f;
    oy = 3.0f/480.0f;
    sx = (32.0f+2.0f)/640.0f;
    sy = (32.0f+2.0f)/480.0f;

    if ( m_phase != PHASE_PERSO )
    {
        m_engine->SetDrawWorld(true);
        m_engine->SetDrawFront(false);
        m_bFixScene = false;
    }

    if ( m_phase == PHASE_INIT )
    {
#if _NEWLOOK
        m_engine->FreeTexture("generna.tga");
        m_engine->FreeTexture("genernb.tga");
        m_engine->FreeTexture("genernc.tga");
        m_engine->FreeTexture("genernd.tga");
#else
#if _FRENCH
#if _DEMO
        m_engine->FreeTexture("genedfa.tga");
        m_engine->FreeTexture("genedfb.tga");
        m_engine->FreeTexture("genedfc.tga");
        m_engine->FreeTexture("genedfd.tga");
#else
        m_engine->FreeTexture("generfa.tga");
        m_engine->FreeTexture("generfb.tga");
        m_engine->FreeTexture("generfc.tga");
        m_engine->FreeTexture("generfd.tga");
#endif
#endif
#if _ENGLISH
#if _DEMO
        m_engine->FreeTexture("genedea.tga");
        m_engine->FreeTexture("genedeb.tga");
        m_engine->FreeTexture("genedec.tga");
        m_engine->FreeTexture("geneded.tga");
#else
        m_engine->FreeTexture("generea.tga");
        m_engine->FreeTexture("genereb.tga");
        m_engine->FreeTexture("generec.tga");
        m_engine->FreeTexture("genered.tga");
#endif
#endif
#if _GERMAN
#if _DEMO
        m_engine->FreeTexture("genedda.tga");
        m_engine->FreeTexture("geneddb.tga");
        m_engine->FreeTexture("geneddc.tga");
        m_engine->FreeTexture("geneddd.tga");
#else
        m_engine->FreeTexture("generea.tga");
        m_engine->FreeTexture("genereb.tga");
        m_engine->FreeTexture("generec.tga");
        m_engine->FreeTexture("genered.tga");
#endif
#endif
#if _WG
#if _DEMO
        m_engine->FreeTexture("genedda.tga");
        m_engine->FreeTexture("geneddb.tga");
        m_engine->FreeTexture("geneddc.tga");
        m_engine->FreeTexture("geneddd.tga");
#else
        m_engine->FreeTexture("generda.tga");
        m_engine->FreeTexture("generdb.tga");
        m_engine->FreeTexture("generdc.tga");
        m_engine->FreeTexture("generdd.tga");
#endif
#endif
#if _POLISH
#if _DEMO
        m_engine->FreeTexture("genedpa.tga");
        m_engine->FreeTexture("genedpb.tga");
        m_engine->FreeTexture("genedpc.tga");
        m_engine->FreeTexture("genedpd.tga");
#else
        m_engine->FreeTexture("generpa.tga");
        m_engine->FreeTexture("generpb.tga");
        m_engine->FreeTexture("generpc.tga");
        m_engine->FreeTexture("generpd.tga");
#endif
#endif
#endif
    }

    if ( m_phase == PHASE_SIMUL )
    {
        m_engine->FreeTexture("inter01a.tga");
        m_engine->FreeTexture("inter01b.tga");
        m_engine->FreeTexture("inter01c.tga");
        m_engine->FreeTexture("inter01d.tga");

        read = m_dialog->RetSceneRead();
        bLoading = (read[0] != 0);

        m_map->CreateMap();
        CreateScene(m_dialog->RetSceneSoluce(), false, false);  // interactive scene
        if ( m_bMapImage )
        {
            m_map->SetFixImage(m_mapFilename);
        }

        pos.x = 620.0f/640.0f;
        pos.y = 460.0f/480.0f;
        ddim.x = 20.0f/640.0f;
        ddim.y = 20.0f/480.0f;
        m_interface->CreateButton(pos, ddim, 11, EVENT_BUTTON_QUIT);

        if ( m_bImmediatSatCom && !bLoading        &&
             m_infoFilename[SATCOM_HUSTON][0] != 0 )
        {
            StartDisplayInfo(SATCOM_HUSTON, false);  // shows the instructions
        }

        m_sound->StopMusic();
        if ( !m_bBase || bLoading )  StartMusic();
    }

    if ( m_phase == PHASE_WIN )
    {
        if ( m_endingWinRank == -1 )
        {
            ChangePhase(PHASE_TERM);
        }
        else
        {
#if _TEEN
            m_bWinTerminate = (m_endingWinRank == 900);
            m_dialog->SetSceneName("teenw");
#else
            m_bWinTerminate = (m_endingWinRank == 904);
            m_dialog->SetSceneName("win");
#endif
            m_dialog->SetSceneRank(m_endingWinRank);
            CreateScene(false, true, false);  // sets scene

            pos.x = ox+sx*1;  pos.y = oy+sy*1;
            ddim.x = dim.x*2;  ddim.y = dim.y*2;
            m_interface->CreateButton(pos, ddim, 16, EVENT_BUTTON_OK);

            if ( m_bWinTerminate )
            {
#if _TEEN
                pos.x = ox+sx*3;  pos.y = oy+sy*1;
                ddim.x = dim.x*15;  ddim.y = dim.y*2;
                pe = m_interface->CreateEdit(pos, ddim, 0, EVENT_EDIT0);
                pe->SetFontType(FONT_COLOBOT);
                pe->SetEditCap(false);
                pe->SetHiliteCap(false);
                pe->ReadText("help\\teenw.txt");
#else
                pos.x = ox+sx*3;  pos.y = oy+sy*0.2f;
                ddim.x = dim.x*15;  ddim.y = dim.y*3.0f;
                pe = m_interface->CreateEdit(pos, ddim, 0, EVENT_EDIT0);
                pe->SetGenericMode(true);
                pe->SetFontType(FONT_COLOBOT);
                pe->SetEditCap(false);
                pe->SetHiliteCap(false);
                pe->ReadText("help\\win.txt");
#endif
            }
            else
            {
                m_displayText->DisplayError(INFO_WIN, Math::Vector(0.0f,0.0f,0.0f), 15.0f, 60.0f, 1000.0f);
            }
        }
        m_sound->StopAll();
        StartMusic();
    }

    if ( m_phase == PHASE_LOST )
    {
        if ( m_endingLostRank == -1 )
        {
            ChangePhase(PHASE_TERM);
        }
        else
        {
            m_bWinTerminate = false;
            m_dialog->SetSceneName("lost");
            m_dialog->SetSceneRank(m_endingLostRank);
            CreateScene(false, true, false);  // sets scene

            pos.x = ox+sx*1;  pos.y = oy+sy*1;
            ddim.x = dim.x*2;  ddim.y = dim.y*2;
            m_interface->CreateButton(pos, ddim, 16, EVENT_BUTTON_OK);
            m_displayText->DisplayError(INFO_LOST, Math::Vector(0.0f,0.0f,0.0f), 15.0f, 60.0f, 1000.0f);
        }
        m_sound->StopAll();
        StartMusic();
    }

    if ( m_phase == PHASE_MODEL )
    {
        pos.x = ox+sx*0;  pos.y = oy+sy*0;
        m_interface->CreateButton(pos, dim, 11, EVENT_BUTTON_CANCEL);

        CreateModel();
    }

    if ( m_phase == PHASE_LOADING )
    {
        m_engine->SetMouseHide(true);
    }
    else
    {
        m_engine->SetMouseHide(false);
    }

    m_engine->LoadAllTexture();
}


// Processes an event.

bool CRobotMain::EventProcess(const Event &event)
{
    CEdit*          pe;
    CObject*        pObj;
    Event           newEvent;
    MainMovieType   type;
    int             i;

    if ( event.event == EVENT_FRAME )
    {
        if ( !m_movie->EventProcess(event) )  // end of the movie?
        {
            type = m_movie->RetStopType();
            if ( type == MM_SATCOMopen )
            {
                ChangePause(false);
                SelectObject(m_infoObject, false);  // hands over the command buttons
                m_map->ShowMap(m_bMapShow);
                m_displayText->HideText(false);
                i = m_movieInfoIndex;
                StartDisplayInfo(m_movieInfoIndex, false);
                m_movieInfoIndex = i;
            }
        }

        m_dialog->EventProcess(event);
        m_displayText->EventProcess(event);
        RemoteCamera(m_cameraPan, m_cameraZoom, event.rTime);

        m_interface->EventProcess(event);
        if ( m_displayInfo != 0 )  // current edition?
        {
            m_displayInfo->EventProcess(event);
        }
        return EventFrame(event);
    }

    // Management of the console.
#if 0
    if ( m_phase != PHASE_NAME &&
         !m_movie->IsExist()   &&
         event.event == EVENT_KEYDOWN &&
         event.param == VK_PAUSE &&
         (event.keyState&KS_CONTROL) != 0 )
#else
    if ( m_phase != PHASE_NAME &&
         !m_movie->IsExist()   &&
         event.event == EVENT_KEYDOWN &&
         event.param == VK_CANCEL )  // Ctrl+Pause ?
#endif
    {
        pe = (CEdit*)m_interface->SearchControl(EVENT_CMD);
        if ( pe == 0 )  return false;
        pe->SetState(STATE_VISIBLE);
        pe->SetFocus(true);
        if ( m_phase == PHASE_SIMUL )  ChangePause(true);
        m_bCmdEdit = true;
        return false;
    }
    if ( event.event == EVENT_KEYDOWN &&
         event.param == VK_RETURN && m_bCmdEdit )
    {
        char    cmd[50];
        pe = (CEdit*)m_interface->SearchControl(EVENT_CMD);
        if ( pe == 0 )  return false;
        pe->GetText(cmd, 50);
        pe->SetText("");
        pe->ClearState(STATE_VISIBLE);
        if ( m_phase == PHASE_SIMUL )  ChangePause(false);
        ExecuteCmd(cmd);
        m_bCmdEdit = false;
        return false;
    }

    // Management of the speed change.
    if ( event.event == EVENT_SPEED )
    {
        SetSpeed(1.0f);
    }

    if ( !m_dialog->EventProcess(event) )
    {
        if ( event.event == EVENT_MOUSEMOVE )
        {
            m_lastMousePos = event.pos;
            HiliteObject(event.pos);
        }
        return false;
    }

    if ( !m_displayText->EventProcess(event) )
    {
        return false;
    }

    if ( event.event == EVENT_MOUSEMOVE )
    {
        m_lastMousePos = event.pos;
        HiliteObject(event.pos);
    }

    if ( m_displayInfo != 0 )  // current info?
    {
        m_displayInfo->EventProcess(event);

        if ( event.event == EVENT_KEYDOWN )
        {
            if ( event.param == m_engine->RetKey(KEYRANK_HELP, 0) ||
                 event.param == m_engine->RetKey(KEYRANK_HELP, 1) ||
                 event.param == m_engine->RetKey(KEYRANK_PROG, 0) ||
                 event.param == m_engine->RetKey(KEYRANK_PROG, 1) ||
                 event.param == VK_ESCAPE )
            {
                StopDisplayInfo();
            }
        }
        if ( event.event == EVENT_OBJECT_INFOOK )
        {
            StopDisplayInfo();
        }
        return false;
    }

    // Simulation phase of the game
    if ( m_phase == PHASE_SIMUL )
    {
        UpdateInfoText();

        if ( !m_bEditFull )
        {
            m_camera->EventProcess(event);
        }

        switch( event.event )
        {
            case EVENT_KEYDOWN:
                KeyCamera(event.event, event.param);
                HiliteClear();
                if ( event.param == VK_F11 )
                {
                    m_particule->WriteWheelTrace("Savegame\\t.bmp", 256, 256, Math::Vector(16.0f, 0.0f, -368.0f), Math::Vector(140.0f, 0.0f, -248.0f));
                    return false;
                }
                if ( m_bEditLock )  // current edition?
                {
                    if ( event.param == m_engine->RetKey(KEYRANK_HELP, 0) ||
                         event.param == m_engine->RetKey(KEYRANK_HELP, 1) )
                    {
                        StartDisplayInfo(SATCOM_HUSTON, false);
                        return false;
                    }
                    if ( event.param == m_engine->RetKey(KEYRANK_PROG, 0) ||
                         event.param == m_engine->RetKey(KEYRANK_PROG, 1) )
                    {
                        StartDisplayInfo(SATCOM_PROG, false);
                        return false;
                    }
                    break;
                }
                if ( m_bMovieLock )  // current movie?
                {
                    if ( event.param == m_engine->RetKey(KEYRANK_QUIT, 0) ||
                         event.param == m_engine->RetKey(KEYRANK_QUIT, 1) ||
                         event.param == VK_ESCAPE )
                    {
                        AbortMovie();
                    }
                    return false;
                }
                if ( m_camera->RetType() == CAMERA_VISIT )
                {
                    if ( event.param == m_engine->RetKey(KEYRANK_VISIT, 0) ||
                         event.param == m_engine->RetKey(KEYRANK_VISIT, 1) )
                    {
                        StartDisplayVisit(EVENT_NULL);
                    }
                    if ( event.param == m_engine->RetKey(KEYRANK_QUIT, 0) ||
                         event.param == m_engine->RetKey(KEYRANK_QUIT, 1) ||
                         event.param == VK_ESCAPE )
                    {
                        StopDisplayVisit();
                    }
                    return false;
                }
                if ( event.param == m_engine->RetKey(KEYRANK_QUIT, 0) ||
                     event.param == m_engine->RetKey(KEYRANK_QUIT, 1) )
                {
                    if ( m_movie->IsExist() )
                    {
                        StartDisplayInfo(SATCOM_HUSTON, false);
                    }
                    else if ( m_winDelay > 0.0f )
                    {
                        ChangePhase(PHASE_WIN);
                    }
                    else if ( m_lostDelay > 0.0f )
                    {
                        ChangePhase(PHASE_LOST);
                    }
                    else
                    {
                        m_dialog->StartAbort();  // do you want to leave?
                    }
                }
                if ( event.param == VK_PAUSE )
                {
                    if ( !m_bMovieLock && !m_bEditLock && !m_bCmdEdit &&
                         m_camera->RetType() != CAMERA_VISIT &&
                         !m_movie->IsExist() )
                    {
                        ChangePause(!m_engine->RetPause());
                    }
                }
                if ( event.param == m_engine->RetKey(KEYRANK_CAMERA, 0) ||
                     event.param == m_engine->RetKey(KEYRANK_CAMERA, 1) )
                {
                    ChangeCamera();
                }
                if ( event.param == m_engine->RetKey(KEYRANK_DESEL, 0) ||
                     event.param == m_engine->RetKey(KEYRANK_DESEL, 1) )
                {
                    if ( m_bShortCut )
                    {
                        DeselectObject();
                    }
                }
                if ( event.param == m_engine->RetKey(KEYRANK_HUMAN, 0) ||
                     event.param == m_engine->RetKey(KEYRANK_HUMAN, 1) )
                {
                    SelectHuman();
                }
                if ( event.param == m_engine->RetKey(KEYRANK_NEXT, 0) ||
                     event.param == m_engine->RetKey(KEYRANK_NEXT, 1) )
                {
                    if ( m_bShortCut )
                    {
                        m_short->SelectNext();
                    }
                }
                if ( event.param == m_engine->RetKey(KEYRANK_HELP, 0) ||
                     event.param == m_engine->RetKey(KEYRANK_HELP, 1) )
                {
                    StartDisplayInfo(SATCOM_HUSTON, true);
                }
                if ( event.param == m_engine->RetKey(KEYRANK_PROG, 0) ||
                     event.param == m_engine->RetKey(KEYRANK_PROG, 1) )
                {
                    StartDisplayInfo(SATCOM_PROG, true);
                }
                if ( event.param == m_engine->RetKey(KEYRANK_VISIT, 0) ||
                     event.param == m_engine->RetKey(KEYRANK_VISIT, 1) )
                {
                    StartDisplayVisit(EVENT_NULL);
                }
                if ( event.param == m_engine->RetKey(KEYRANK_SPEED10, 0) ||
                     event.param == m_engine->RetKey(KEYRANK_SPEED10, 1) )
                {
                    SetSpeed(1.0f);
                }
                if ( event.param == m_engine->RetKey(KEYRANK_SPEED15, 0) ||
                     event.param == m_engine->RetKey(KEYRANK_SPEED15, 1) )
                {
                    SetSpeed(1.5f);
                }
                if ( event.param == m_engine->RetKey(KEYRANK_SPEED20, 0) ||
                     event.param == m_engine->RetKey(KEYRANK_SPEED20, 1) )
                {
                    SetSpeed(2.0f);
                }
                if ( event.param == m_engine->RetKey(KEYRANK_SPEED30, 0) ||
                     event.param == m_engine->RetKey(KEYRANK_SPEED30, 1) )
                {
                    SetSpeed(3.0f);
                }
                break;

            case EVENT_KEYUP:
                KeyCamera(event.event, event.param);
                break;

            case EVENT_LBUTTONDOWN:
                pObj = DetectObject(event.pos);
                if ( !m_bShortCut )  pObj = 0;
                if ( pObj != 0 && pObj->RetType() == OBJECT_TOTO )
                {
                    if ( m_displayInfo != 0 )  // current info?
                    {
                        StopDisplayInfo();
                    }
                    else
                    {
                        if ( !m_bEditLock )
                        {
                            StartDisplayInfo(SATCOM_HUSTON, true);
                        }
                    }
                }
                else
                {
                    SelectObject(pObj);
                }
                break;

            case EVENT_LBUTTONUP:
                m_cameraPan  = 0.0f;
                m_cameraZoom = 0.0f;
                break;

            case EVENT_BUTTON_QUIT:
                if ( m_movie->IsExist() )
                {
                    StartDisplayInfo(SATCOM_HUSTON, false);
                }
                else if ( m_winDelay > 0.0f )
                {
                    ChangePhase(PHASE_WIN);
                }
                else if ( m_lostDelay > 0.0f )
                {
                    ChangePhase(PHASE_LOST);
                }
                else
                {
                    m_dialog->StartAbort();  // do you want to leave?
                }
                break;

            case EVENT_OBJECT_LIMIT:
                StartShowLimit();
                break;

            case EVENT_OBJECT_DESELECT:
                if ( m_bShortCut )
                {
                    DeselectObject();
                }
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
                StartDisplayVisit(event.event);
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
                m_short->SelectShortcut(event.event);
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
        }

        EventObject(event);
        return false;
    }

    if ( m_phase == PHASE_PERSO )
    {
        EventObject(event);
    }

    if ( m_phase == PHASE_WIN  ||
         m_phase == PHASE_LOST )
    {
        EventObject(event);

        switch( event.event )
        {
            case EVENT_KEYDOWN:
                if ( event.param == VK_ESCAPE ||
                     event.param == VK_RETURN )
                {
                    if ( m_bWinTerminate )
                    {
                        ChangePhase(PHASE_INIT);
                    }
                    else
                    {
                        ChangePhase(PHASE_TERM);
                    }
                }
                break;

            case EVENT_BUTTON_OK:
                if ( m_bWinTerminate )
                {
                    ChangePhase(PHASE_INIT);
                }
                else
                {
                    ChangePhase(PHASE_TERM);
                }
                break;
        }
    }

    if ( m_phase == PHASE_MODEL )
    {
        switch( event.event )
        {
            case EVENT_KEYDOWN:
                if ( event.param == VK_ESCAPE )
                {
                    ChangePhase(PHASE_INIT);
                }
                if ( event.param == VK_HOME )
                {
                    InitEye();
                }
                break;

            case EVENT_BUTTON_CANCEL:
                ChangePhase(PHASE_INIT);
                break;
        }

        m_model->EventProcess(event);
        return false;
    }

    return true;
}



// Executes a command.

void CRobotMain::ExecuteCmd(char *cmd)
{
    if ( cmd[0] == 0 )  return;

    if ( m_phase == PHASE_SIMUL )
    {
        if ( strcmp(cmd, "winmission") == 0 )
        {
            Event       newEvent;
            m_event->MakeEvent(newEvent, EVENT_WIN);
            m_event->AddEvent(newEvent);
        }

        if ( strcmp(cmd, "lostmission") == 0 )
        {
            Event       newEvent;
            m_event->MakeEvent(newEvent, EVENT_LOST);
            m_event->AddEvent(newEvent);
        }

        if ( strcmp(cmd, "trainerpilot") == 0 )
        {
            m_bTrainerPilot = !m_bTrainerPilot;
            return;
        }

        if ( strcmp(cmd, "fly") == 0 )
        {
            Event   newEvent;

            g_researchDone |= RESEARCH_FLY;

            m_event->MakeEvent(newEvent, EVENT_UPDINTERFACE);
            m_event->AddEvent(newEvent);
            return;
        }

        if ( strcmp(cmd, "allresearch") == 0 )
        {
            Event   newEvent;

            g_researchDone = -1;  // all research are done

            m_event->MakeEvent(newEvent, EVENT_UPDINTERFACE);
            m_event->AddEvent(newEvent);
            return;
        }

        if ( strcmp(cmd, "nolimit") == 0 )
        {
            m_terrain->SetFlyingMaxHeight(280.0f);
            return;
        }

        if ( strcmp(cmd, "photo1") == 0 )
        {
            m_bFreePhoto = !m_bFreePhoto;
            if ( m_bFreePhoto )
            {
                m_camera->SetType(CAMERA_FREE);
                ChangePause(true);
            }
            else
            {
                m_camera->SetType(CAMERA_BACK);
                ChangePause(false);
            }
            return;
        }

        if ( strcmp(cmd, "photo2") == 0 )
        {
            m_bFreePhoto = !m_bFreePhoto;
            if ( m_bFreePhoto )
            {
                m_camera->SetType(CAMERA_FREE);
                ChangePause(true);
                DeselectAll();  // removes the control buttons
                m_map->ShowMap(false);
                m_displayText->HideText(true);
            }
            else
            {
                m_camera->SetType(CAMERA_BACK);
                ChangePause(false);
                m_map->ShowMap(m_bMapShow);
                m_displayText->HideText(false);
            }
            return;
        }

        if ( strcmp(cmd, "noclip") == 0 )
        {
            CObject*    object;

            object = RetSelect();
            if ( object != 0 )
            {
                object->SetClip(false);
            }
            return;
        }

        if ( strcmp(cmd, "clip") == 0 )
        {
            CObject*    object;

            object = RetSelect();
            if ( object != 0 )
            {
                object->SetClip(true);
            }
            return;
        }

        if ( strcmp(cmd, "addhusky") == 0 )
        {
            CObject*    object;

            object = RetSelect();
            if ( object != 0 )
            {
                object->SetMagnifyDamage(object->RetMagnifyDamage()*0.1f);
            }
            return;
        }

        if ( strcmp(cmd, "addfreezer") == 0 )
        {
            CObject*    object;

            object = RetSelect();
            if ( object != 0 )
            {
                object->SetRange(object->RetRange()*10.0f);
            }
            return;
        }

        if ( strcmp(cmd, "fullpower") == 0 )
        {
            CObject*    object;
            CObject*    power;
            CPhysics*   physics;

            object = RetSelect();
            if ( object != 0 )
            {
                power = object->RetPower();
                if ( power != 0 )
                {
                    power->SetEnergy(1.0f);
                }
                object->SetShield(1.0f);
                physics = object->RetPhysics();
                if ( physics != 0 )
                {
                    physics->SetReactorRange(1.0f);
                }
            }
            return;
        }

        if ( strcmp(cmd, "fullenergy") == 0 )
        {
            CObject*    object;
            CObject*    power;

            object = RetSelect();
            if ( object != 0 )
            {
                power = object->RetPower();
                if ( power != 0 )
                {
                    power->SetEnergy(1.0f);
                }
            }
            return;
        }

        if ( strcmp(cmd, "fullshield") == 0 )
        {
            CObject*    object;

            object = RetSelect();
            if ( object != 0 )
            {
                object->SetShield(1.0f);
            }
            return;
        }

        if ( strcmp(cmd, "fullrange") == 0 )
        {
            CObject*    object;
            CPhysics*   physics;

            object = RetSelect();
            if ( object != 0 )
            {
                physics = object->RetPhysics();
                if ( physics != 0 )
                {
                    physics->SetReactorRange(1.0f);
                }
            }
            return;
        }
    }

    if ( strcmp(cmd, "debugmode") == 0 )
    {
        m_engine->SetDebugMode(!m_engine->RetDebugMode());
        return;
    }

    if ( strcmp(cmd, "showstat") == 0 )
    {
        m_engine->SetShowStat(!m_engine->RetShowStat());
        return;
    }

    if ( strcmp(cmd, "invshadow") == 0 )
    {
        m_engine->SetShadow(!m_engine->RetShadow());
        return;
    }

    if ( strcmp(cmd, "invdirty") == 0 )
    {
        m_engine->SetDirty(!m_engine->RetDirty());
        return;
    }

    if ( strcmp(cmd, "invfog") == 0 )
    {
        m_engine->SetFog(!m_engine->RetFog());
        return;
    }

    if ( strcmp(cmd, "invlens") == 0 )
    {
        m_engine->SetLensMode(!m_engine->RetLensMode());
        return;
    }

    if ( strcmp(cmd, "invwater") == 0 )
    {
        m_engine->SetWaterMode(!m_engine->RetWaterMode());
        return;
    }

    if ( strcmp(cmd, "invsky") == 0 )
    {
        m_engine->SetSkyMode(!m_engine->RetSkyMode());
        return;
    }

    if ( strcmp(cmd, "invplanet") == 0 )
    {
        m_engine->SetPlanetMode(!m_engine->RetPlanetMode());
        return;
    }

    if ( strcmp(cmd, "showpos") == 0 )
    {
        m_bShowPos = !m_bShowPos;
        return;
    }

    if ( strcmp(cmd, "selectinsect") == 0 )
    {
        m_bSelectInsect = !m_bSelectInsect;
        return;
    }

    if ( strcmp(cmd, "showsoluce") == 0 )
    {
        m_bShowSoluce = !m_bShowSoluce;
        m_dialog->ShowSoluceUpdate();
        return;
    }

#if _TEEN
    if ( strcmp(cmd, "allteens") == 0 )
#else
    if ( strcmp(cmd, "allmission") == 0 )
#endif
    {
        m_bShowAll = !m_bShowAll;
        m_dialog->AllMissionUpdate();
        return;
    }

    if ( strcmp(cmd, "invradar") == 0 )
    {
        m_bCheatRadar = !m_bCheatRadar;
        return;
    }

    if ( m_phase == PHASE_SIMUL )
    {
        m_displayText->DisplayError(ERR_CMD, Math::Vector(0.0f,0.0f,0.0f));
    }
}



// Returns the type of current movie.

MainMovieType CRobotMain::RetMainMovie()
{
    return m_movie->RetType();
}


// Clears the display of instructions.

void CRobotMain::FlushDisplayInfo()
{
    int     i;

    for ( i=0 ; i<SATCOM_MAX ; i++ )
    {
        m_infoFilename[i][0] = 0;
        m_infoPos[i] = 0;
    }
    strcpy(m_infoFilename[SATCOM_OBJECT], "help\\objects.txt");
    m_infoIndex = 0;
}

// Beginning of the displaying of instructions.
// index: SATCOM_*

void CRobotMain::StartDisplayInfo(int index, bool bMovie)
{
    CObject*    pObj;
    CMotion*    motion;
    bool        bHuman;

    if ( m_bCmdEdit || m_bSatComLock )  return;

    pObj = RetSelect();
    bHuman = ( pObj != 0 && pObj->RetType() == OBJECT_HUMAN );

    if ( !m_bEditLock && bMovie && !m_movie->IsExist() && bHuman )
    {
        motion = pObj->RetMotion();
        if ( motion != 0 && motion->RetAction() == -1 )
        {
            m_movieInfoIndex = index;
            m_movie->Start(MM_SATCOMopen, 2.5f);
            ChangePause(true);
//?         m_map->ShowMap(false);
            m_infoObject = DeselectAll();  // removes the control buttons
            m_displayText->HideText(true);
            return;
        }
    }

    if ( m_movie->IsExist() )
    {
        m_movie->Stop();
        ChangePause(false);
        SelectObject(m_infoObject, false);  // hands over the command buttons
//?     m_map->ShowMap(m_bMapShow);
        m_displayText->HideText(false);
    }

    StartDisplayInfo(m_infoFilename[index], index);
}

// Beginning of the displaying of instructions.

void CRobotMain::StartDisplayInfo(char *filename, int index)
{
    CButton*    pb;
    bool        bSoluce;

    if ( m_bCmdEdit )  return;

    m_movieInfoIndex = -1;
    ClearInterface();  // removes setting evidence and tooltip

    if ( !m_bEditLock )
    {
//?     m_map->ShowMap(false);
        m_infoObject = DeselectAll();  // removes the control buttons
        m_displayText->HideText(true);
        m_sound->MuteAll(true);
    }

    pb = (CButton*)m_interface->SearchControl(EVENT_BUTTON_QUIT);
    if ( pb != 0 )
    {
        pb->ClearState(STATE_VISIBLE);
    }

    bSoluce = m_dialog->RetSceneSoluce();

    m_displayInfo = new CDisplayInfo(m_iMan);
    m_displayInfo->StartDisplayInfo(filename, index, bSoluce);

    m_infoIndex = index;
    if ( index != -1 )
    {
        m_displayInfo->SetPosition(m_infoPos[index]);
    }
}

// End of displaying of instructions.

void CRobotMain::StopDisplayInfo()
{
    CButton*    pb;

    if ( m_movieInfoIndex != -1 )  // film to read the SatCom?
    {
        m_movie->Start(MM_SATCOMclose, 2.0f);
    }

    if ( m_infoIndex != -1 )
    {
        m_infoPos[m_infoIndex] = m_displayInfo->RetPosition();
    }
    m_displayInfo->StopDisplayInfo();

    delete m_displayInfo;
    m_displayInfo = 0;

    if ( !m_bEditLock )
    {
        pb = (CButton*)m_interface->SearchControl(EVENT_BUTTON_QUIT);
        if ( pb != 0 )
        {
            pb->SetState(STATE_VISIBLE);
        }

        SelectObject(m_infoObject, false);  // gives the command buttons
//?     m_map->ShowMap(m_bMapShow);
        m_displayText->HideText(false);

        m_sound->MuteAll(false);
    }

    if ( m_infoUsed == 0 )
    {
        m_displayText->ClearText();  // removes message "see SatCom ..."
    }
    m_infoUsed ++;
}

// Returns the name of the text display.

char* CRobotMain::RetDisplayInfoName(int index)
{
    return m_infoFilename[index];
}

// Returns the name of the text display.

int CRobotMain::RetDisplayInfoPosition(int index)
{
    return m_infoPos[index];
}

// Returns the name of the text display.

void CRobotMain::SetDisplayInfoPosition(int index, int pos)
{
    m_infoPos[index] = pos;
}


// Beginning of a dialogue during the game,

void CRobotMain::StartSuspend()
{
    CButton*    pb;

    m_map->ShowMap(false);
    m_infoObject = DeselectAll();  // removes the control buttons
    m_displayText->HideText(true);

    pb = (CButton*)m_interface->SearchControl(EVENT_BUTTON_QUIT);
    if ( pb != 0 )
    {
        pb->ClearState(STATE_VISIBLE);
    }

    m_bSuspend = true;
}

// End of dialogue during the game,

void CRobotMain::StopSuspend()
{
    CButton*    pb;

    pb = (CButton*)m_interface->SearchControl(EVENT_BUTTON_QUIT);
    if ( pb != 0 )
    {
        pb->SetState(STATE_VISIBLE);
    }

    SelectObject(m_infoObject, false);  // gives the command buttons
    m_map->ShowMap(m_bMapShow);
    m_displayText->HideText(false);

    m_bSuspend = false;
}


// Returns the absolute time of the game

float CRobotMain::RetGameTime()
{
    return m_gameTime;
}



// Managing the size of the default fonts.

void CRobotMain::SetFontSize(float size)
{
    m_fontSize = size;
    SetLocalProfileFloat("Edit", "FontSize", m_fontSize);
}

float CRobotMain::RetFontSize()
{
    return m_fontSize;
}

// Managing the size of the default window.

void CRobotMain::SetWindowPos(Math::Point pos)
{
    m_windowPos = pos;
    SetLocalProfileFloat("Edit", "WindowPos.x", m_windowPos.x);
    SetLocalProfileFloat("Edit", "WindowPos.y", m_windowPos.y);
}

Math::Point CRobotMain::RetWindowPos()
{
    return m_windowPos;
}

void CRobotMain::SetWindowDim(Math::Point dim)
{
    m_windowDim = dim;
    SetLocalProfileFloat("Edit", "WindowDim.x", m_windowDim.x);
    SetLocalProfileFloat("Edit", "WindowDim.y", m_windowDim.y);
}

Math::Point CRobotMain::RetWindowDim()
{
    return m_windowDim;
}


// Managing windows open/save.

void CRobotMain::SetIOPublic(bool bMode)
{
    m_IOPublic = bMode;
    SetLocalProfileInt("Edit", "IOPublic", m_IOPublic);
}

bool CRobotMain::RetIOPublic()
{
    return m_IOPublic;
}

void CRobotMain::SetIOPos(Math::Point pos)
{
    m_IOPos = pos;
    SetLocalProfileFloat("Edit", "IOPos.x", m_IOPos.x);
    SetLocalProfileFloat("Edit", "IOPos.y", m_IOPos.y);
}

Math::Point CRobotMain::RetIOPos()
{
    return m_IOPos;
}

void CRobotMain::SetIODim(Math::Point dim)
{
    m_IODim = dim;
    SetLocalProfileFloat("Edit", "IODim.x", m_IODim.x);
    SetLocalProfileFloat("Edit", "IODim.y", m_IODim.y);
}

Math::Point CRobotMain::RetIODim()
{
    return m_IODim;
}



// Start of the visit instead of an error.

void CRobotMain::StartDisplayVisit(EventMsg event)
{
    CWindow*    pw;
    CButton*    button;
    CGroup*     group;
    Math::Vector    goal;
    Math::Point     pos, dim;
    int         i, j;

    if ( m_bEditLock )  return;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW2);
    if ( pw == 0 )  return;

    if ( event == EVENT_NULL )  // visit by keyboard shortcut?
    {
        if ( m_visitLast != EVENT_NULL )  // already a current visit?
        {
            i = m_visitLast-EVENT_DT_VISIT0;
        }
        else
        {
            i = MAXDTLINE;
        }

        // Seeks the last.
        for ( j=0 ; j<MAXDTLINE ; j++ )
        {
            i --;
            if ( i < 0 )  i = MAXDTLINE-1;

            button = (CButton*)pw->SearchControl(EventMsg(EVENT_DT_VISIT0+i));
            if ( button == 0 || !button->TestState(STATE_ENABLE) )  continue;

            group = (CGroup*)pw->SearchControl(EventMsg(EVENT_DT_GROUP0+i));
            if ( group != 0 )
            {
                event = EventMsg(EVENT_DT_VISIT0+i);
                break;
            }
        }
    }
    if ( event == EVENT_NULL )
    {
        m_sound->Play(SOUND_TZOING);  // nothing to do!
        return;
    }

    m_visitLast = event;

    ClearInterface();  // removes setting evidence and tooltip

    if ( m_camera->RetType() == CAMERA_VISIT )  // already a current visit?
    {
        m_camera->StopVisit();
        m_displayText->ClearVisit();
    }
    else
    {
        m_visitObject = DeselectAll();  // removes the control buttons
    }

    // Creates the "continue" button.
    if ( m_interface->SearchControl(EVENT_DT_END) == 0 )
    {
        pos.x = 10.0f/640.0f;
        pos.y = 10.0f/480.0f;
        dim.x = 50.0f/640.0f;
        dim.y = 50.0f/480.0f;
        m_interface->CreateButton(pos, dim, 16, EVENT_DT_END);
    }

    // Creates the arrow to show the place.
    if ( m_visitArrow != 0 )
    {
        m_visitArrow->DeleteObject();
        delete m_visitArrow;
        m_visitArrow = 0;
    }
    goal = m_displayText->RetVisitGoal(event);
    m_visitArrow = CreateObject(goal, 0.0f, 1.0f, 10.0f, OBJECT_SHOW, false, false, 0);

    m_visitPos = m_visitArrow->RetPosition(0);
    m_visitPosArrow = m_visitPos;
    m_visitPosArrow.y += m_displayText->RetVisitHeight(event);
    m_visitArrow->SetPosition(0, m_visitPosArrow);

    m_visitTime = 0.0;
    m_visitParticule = 0.0f;

    m_particule->DeleteParticule(PARTISHOW);

    m_camera->StartVisit(m_displayText->RetVisitGoal(event),
                         m_displayText->RetVisitDist(event));
    m_displayText->SetVisit(event);
    ChangePause(true);
}

// Move the arrow to visit.

void CRobotMain::FrameVisit(float rTime)
{
    Math::Vector    pos, speed;
    Math::Point     dim;
    float       level;

    if ( m_visitArrow == 0 )  return;

    // Moves the arrow.
    m_visitTime += rTime;

    pos = m_visitPosArrow;
    pos.y += 1.5f+sinf(m_visitTime*4.0f)*4.0f;
    m_visitArrow->SetPosition(0, pos);
    m_visitArrow->SetAngleY(0, m_visitTime*2.0f);

    // Manages the particles "arrows".
    m_visitParticule -= rTime;
    if ( m_visitParticule <= 0.0f )
    {
        m_visitParticule = 1.5f;

        pos = m_visitPos;
        level = m_terrain->RetFloorLevel(pos)+2.0f;
        if ( pos.y < level )  pos.y = level;  // not below the ground
        speed = Math::Vector(0.0f, 0.0f, 0.0f);
        dim.x = 30.0f;
        dim.y = dim.x;
        m_particule->CreateParticule(pos, speed, dim, PARTISHOW, 2.0f);
    }
}

// End of the visit instead of an error.

void CRobotMain::StopDisplayVisit()
{
    m_visitLast = EVENT_NULL;

    // Removes the button.
    m_interface->DeleteControl(EVENT_DT_END);

    // Removes the arrow.
    if ( m_visitArrow != 0 )
    {
        m_visitArrow->DeleteObject();
        delete m_visitArrow;
        m_visitArrow = 0;
    }

    // Removes particles "arrows".
    m_particule->DeleteParticule(PARTISHOW);

    m_camera->StopVisit();
    m_displayText->ClearVisit();
    ChangePause(false);
    if ( m_visitObject != 0 )
    {
        SelectObject(m_visitObject, false);  // gives the command buttons
        m_visitObject = 0;
    }
}



// Updates all the shortcuts.

void CRobotMain::UpdateShortcuts()
{
    m_short->UpdateShortcuts();
}

// Returns the object that default was select after the creation of a scene.

CObject* CRobotMain::RetSelectObject()
{
    if ( m_selectObject != 0 )  return m_selectObject;
    return SearchHuman();
}

// Deselects everything, and returns the object that was selected.

CObject* CRobotMain::DeselectAll()
{
    CObject*    pObj;
    CObject*    pPrev;
    int         i;

    pPrev = 0;
    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        if ( pObj->RetSelect() )  pPrev = pObj;
        pObj->SetSelect(false);
    }
    return pPrev;
}

// Selects an object, without attending to deselect the rest.

void CRobotMain::SelectOneObject(CObject* pObj, bool bDisplayError)
{
    ObjectType      type;
    CObject*        toto;
    CMotionToto*    mt;

    pObj->SetSelect(true, bDisplayError);
    m_camera->SetObject(pObj);

    type = pObj->RetType();
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
        m_camera->SetType(pObj->RetCameraType());
        m_camera->SetDist(pObj->RetCameraDist());
    }
    else
    {
        m_camera->SetType(CAMERA_BACK);
    }

    toto = SearchToto();
    if ( toto != 0 )
    {
        mt = (CMotionToto*)toto->RetMotion();
        if ( mt != 0 )
        {
            mt->SetLinkType(type);
        }
    }
}

// Selects the object aimed by the mouse.

bool CRobotMain::SelectObject(CObject* pObj, bool bDisplayError)
{
    CObject*    pPrev;

    if ( m_camera->RetType() == CAMERA_VISIT )
    {
        StopDisplayVisit();
    }

    if ( m_bMovieLock || m_bEditLock || m_bPause )  return false;
    if ( m_movie->IsExist() )  return false;
    if ( pObj == 0 || !IsSelectable(pObj) )  return false;

    pPrev = DeselectAll();

    if ( pPrev != 0 && pPrev != pObj )
    {
        pObj->AddDeselList(pPrev);
    }

    SelectOneObject(pObj, bDisplayError);
    m_short->UpdateShortcuts();
    return true;
}

// Deselects the selected object.

bool CRobotMain::DeselectObject()
{
    CObject*    pObj;
    CObject*    pPrev;

    pPrev = DeselectAll();

    if ( pPrev == 0 )
    {
        pObj = SearchHuman();
    }
    else
    {
        pObj = pPrev->SubDeselList();
    }
    if ( pObj == 0 )
    {
        pObj = SearchHuman();
    }

    if ( pObj != 0 )
    {
        SelectOneObject(pObj);
    }
    else
    {
        m_camera->SetType(CAMERA_FREE);
    }

    m_short->UpdateShortcuts();
    return true;
}

// Quickly removes all objects.

void CRobotMain::DeleteAllObjects()
{
    CPyro*      pyro;
    CObject*    pObj;
    int         i;

    // Removes all pyrotechnic effects in progress.
    while ( true )
    {
        pyro = (CPyro*)m_iMan->SearchInstance(CLASS_PYRO, 0);
        if ( pyro == 0 )  break;

        pyro->DeleteObject();
        delete pyro;
    }

    // Removes the arrow.
    if ( m_visitArrow != 0 )
    {
        m_visitArrow->DeleteObject();
        delete m_visitArrow;
        m_visitArrow = 0;
    }

    for ( i=0 ; i<MAXSHOWLIMIT ; i++ )
    {
        FlushShowLimit(i);
    }

    while ( true )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, 0);
        if ( pObj == 0 )  break;

        pObj->DeleteObject(true);  // destroys rapidly
        delete pObj;
    }
}

// Selects the human.

void CRobotMain::SelectHuman()
{
    SelectObject(SearchHuman());
}

// Returns the object human.

CObject* CRobotMain::SearchHuman()
{
    ObjectType  type;
    CObject*    pObj;
    int         i;

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        type = pObj->RetType();
        if ( type == OBJECT_HUMAN )
        {
            return pObj;
        }
    }
    return 0;
}

// Returns the object toto.

CObject* CRobotMain::SearchToto()
{
    ObjectType  type;
    CObject*    pObj;
    int         i;

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        type = pObj->RetType();
        if ( type == OBJECT_TOTO )
        {
            return pObj;
        }
    }
    return 0;
}

// Returns the nearest selectable object from a given position.

CObject* CRobotMain::SearchNearest(Math::Vector pos, CObject* pExclu)
{
    ObjectType  type;
    CObject     *pObj, *pBest;
    Math::Vector    oPos;
    float       min, dist;
    int         i;

    min = 100000.0f;
    pBest = 0;
    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        if ( pObj == pExclu )  continue;
        if ( !IsSelectable(pObj) )  continue;

        type = pObj->RetType();
        if ( type == OBJECT_TOTO )  continue;

        oPos = pObj->RetPosition(0);
        dist = Math::DistanceProjected(oPos, pos);
        if ( dist < min )
        {
            min = dist;
            pBest = pObj;
        }
    }
    return pBest;
}

// Returns the selected object.

CObject* CRobotMain::RetSelect()
{
    CObject*    pObj;
    int         i;

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        if ( pObj->RetSelect() )
        {
            return pObj;
        }
    }
    return 0;
}

CObject* CRobotMain::SearchObject(ObjectType type)
{
    CObject*    pObj;
    int         i;

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        if ( pObj->RetType() == type )
        {
            return pObj;
        }
    }
    return 0;
}

// Detects the object aimed by the mouse.

CObject* CRobotMain::DetectObject(Math::Point pos)
{
    ObjectType  type;
    CObject     *pObj, *pTarget;
    int         objRank, i, j, rank;

    objRank = m_engine->DetectObject(pos);

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        if ( !pObj->RetActif() )  continue;
        if ( pObj->RetProxyActivate() )  continue;

        pTarget = 0;
        type = pObj->RetType();
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
            pTarget = pObj;
        }
        else if ( (type == OBJECT_POWER  ||
                  type == OBJECT_ATOMIC ) &&
             pObj->RetTruck() != 0 )  // battery used?
        {
            pTarget = pObj->RetTruck();
        }
        else if ( type == OBJECT_POWER  ||
                  type == OBJECT_ATOMIC )
        {
            pTarget = pObj;
        }

        for ( j=0 ; j<OBJECTMAXPART ; j++ )
        {
            rank = pObj->RetObjectRank(j);
            if ( rank == -1 )  continue;
            if ( rank != objRank )  continue;
            return pTarget;
        }
    }
    return 0;
}

// Indicates whether an object is selectable.

bool CRobotMain::IsSelectable(CObject* pObj)
{
    ObjectType  type;

    if ( !pObj->RetSelectable() )  return false;

    type = pObj->RetType();
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

    if ( m_bSelectInsect )
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


// Deletes the selected object.

bool CRobotMain::DeleteObject()
{
    CObject*    pObj;
    CPyro*      pyro;

    pObj = RetSelect();
    if ( pObj == 0 )  return false;

    pyro = new CPyro(m_iMan);
    pyro->Create(PT_FRAGT, pObj);

    pObj->SetSelect(false);  // deselects the object
    m_camera->SetType(CAMERA_EXPLO);
    DeselectAll();
    pObj->DeleteDeselList(pObj);

    return true;
}


// Removes setting evidence of the object with the mouse hovers over.

void CRobotMain::HiliteClear()
{
    CObject*    pObj;
    int         i;

    ClearTooltip();
    m_tooltipName[0] = 0;  // really removes the tooltip

    if ( !m_bHilite )  return;

    i = -1;
    m_engine->SetHiliteRank(&i);  // nothing more selected

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        pObj->SetHilite(false);
        m_map->SetHilite(0);
        m_short->SetHilite(0);
    }

    m_bHilite = false;
}

// Highlights the object with the mouse hovers over.

void CRobotMain::HiliteObject(Math::Point pos)
{
    CObject*    pObj;
    char        name[100];
    bool        bInMap;

    if ( m_bFixScene && m_phase != PHASE_PERSO )  return;
    if ( m_bMovieLock )  return;
    if ( m_movie->IsExist() )  return;
    if ( m_engine->RetMouseHide() )  return;

    ClearInterface();  // removes setting evidence and tooltip

    pObj = m_short->DetectShort(pos);

    if ( m_dialog->RetTooltip() && m_interface->GetTooltip(pos, name) )
    {
        m_tooltipPos = pos;
        strcpy(m_tooltipName, name);
        m_tooltipTime = 0.0f;
        if ( pObj == 0 )  return;
    }

    if ( m_bSuspend )  return;

    if ( pObj == 0 )
    {
        pObj = m_map->DetectMap(pos, bInMap);
        if ( pObj == 0 )
        {
            if ( bInMap )  return;

            pObj = DetectObject(pos);

            if ( m_camera->RetType() == CAMERA_ONBOARD &&
                 m_camera->RetObject() == pObj )
            {
                return;
            }
        }
    }

    if ( pObj != 0 )
    {
        if ( m_dialog->RetTooltip() && pObj->GetTooltipName(name) )
        {
            m_tooltipPos = pos;
            strcpy(m_tooltipName, name);
            m_tooltipTime = 0.0f;
        }

        if ( IsSelectable(pObj) )
        {
            pObj->SetHilite(true);
            m_map->SetHilite(pObj);
            m_short->SetHilite(pObj);
            m_bHilite = true;
        }
    }
}

// Highlights the object with the mouse hovers over.

void CRobotMain::HiliteFrame(float rTime)
{
    if ( m_bFixScene && m_phase != PHASE_PERSO )  return;
    if ( m_bMovieLock )  return;
    if ( m_movie->IsExist() )  return;

    m_tooltipTime += rTime;

    ClearTooltip();

    if ( m_tooltipTime >= 0.2f &&
         m_tooltipName[0] != 0 )
    {
        CreateTooltip(m_tooltipPos, m_tooltipName);
    }
}

// Creates a tooltip.

void CRobotMain::CreateTooltip(Math::Point pos, char* text)
{
    CWindow*    pw;
    Math::Point     start, end, dim, offset, corner;

    corner.x = pos.x+0.022f;
    corner.y = pos.y-0.052f;

    m_engine->RetText()->DimText(text, corner, 1,
                                 SMALLFONT, NORMSTRETCH, FONT_COLOBOT,
                                 start, end);
    start.x -= 0.010f;
    start.y -= 0.002f;
    end.x   += 0.010f;
    end.y   += 0.004f;  // ch'tite (?) margin

    pos.x = start.x;
    pos.y = start.y;
    dim.x = end.x-start.x;
    dim.y = end.y-start.y;

    offset.x = 0.0f;
    offset.y = 0.0f;
    if ( pos.x+dim.x > 1.0f )  offset.x = 1.0f-(pos.x+dim.x);
    if ( pos.y       < 0.0f )  offset.y = -pos.y;

    corner.x += offset.x;
    corner.y += offset.y;
    pos.x += offset.x;
    pos.y += offset.y;

    m_interface->CreateWindows(pos, dim, 1, EVENT_TOOLTIP);

    pw = (CWindow*)m_interface->SearchControl(EVENT_TOOLTIP);
    if ( pw != 0 )
    {
        pw->SetState(STATE_SHADOW);
        pw->SetTrashEvent(false);

        pos.y -= m_engine->RetText()->RetHeight(SMALLFONT, FONT_COLOBOT)/2.0f;
        pw->CreateLabel(pos, dim, -1, EVENT_LABEL2, text);
    }
}

// Clears the previous tooltip.

void CRobotMain::ClearTooltip()
{
    m_interface->DeleteControl(EVENT_TOOLTIP);
}


// Displays help for an object.

void CRobotMain::HelpObject()
{
    CObject*    pObj;
    char*       filename;

    pObj = RetSelect();
    if ( pObj == 0 )  return;

    filename = RetHelpFilename(pObj->RetType());
    if ( filename[0] == 0 )  return;

    StartDisplayInfo(filename, -1);
}


// Change the mode of the camera.

void CRobotMain::ChangeCamera()
{
    CObject*    pObj;
    ObjectType  oType;
    CameraType  type;
    int         i;

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        if ( pObj->RetSelect() )
        {
            if ( pObj->RetCameraLock() )  return;

            oType = pObj->RetType();
            type = pObj->RetCameraType();

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

            if ( oType == OBJECT_MOBILEdr )  // designer?
            {
                     if ( type == CAMERA_PLANE   )  type = CAMERA_BACK;
                else if ( type == CAMERA_BACK    )  type = CAMERA_PLANE;
            }
            else if ( pObj->RetTrainer() )  // trainer?
            {
                     if ( type == CAMERA_ONBOARD )  type = CAMERA_FIX;
                else if ( type == CAMERA_FIX     )  type = CAMERA_PLANE;
                else if ( type == CAMERA_PLANE   )  type = CAMERA_BACK;
                else if ( type == CAMERA_BACK    )  type = CAMERA_ONBOARD;
            }
            else
            {
                     if ( type == CAMERA_ONBOARD )  type = CAMERA_BACK;
                else if ( type == CAMERA_BACK    )  type = CAMERA_ONBOARD;
            }

            pObj->SetCameraType(type);
            m_camera->SetType(type);
        }
    }
}

// Remote control the camera using the arrow keys.

void CRobotMain::KeyCamera(EventMsg event, long param)
{
    CObject*    pObj;

    if ( event == EVENT_KEYUP )
    {
        if ( param == m_engine->RetKey(KEYRANK_LEFT, 0) ||
             param == m_engine->RetKey(KEYRANK_LEFT, 1) )
        {
            m_cameraPan = 0.0f;
        }

        if ( param == m_engine->RetKey(KEYRANK_RIGHT, 0) ||
             param == m_engine->RetKey(KEYRANK_RIGHT, 1) )
        {
            m_cameraPan = 0.0f;
        }

        if ( param == m_engine->RetKey(KEYRANK_UP, 0) ||
             param == m_engine->RetKey(KEYRANK_UP, 1) )
        {
            m_cameraZoom = 0.0f;
        }

        if ( param == m_engine->RetKey(KEYRANK_DOWN, 0) ||
             param == m_engine->RetKey(KEYRANK_DOWN, 1) )
        {
            m_cameraZoom = 0.0f;
        }
    }

    if ( m_phase != PHASE_SIMUL )  return;
    if ( m_bEditLock )  return;  // current edition?
    if ( m_bTrainerPilot )  return;

    pObj = RetSelect();
    if ( pObj == 0 )  return;
    if ( !pObj->RetTrainer() )  return;

    if ( event == EVENT_KEYDOWN )
    {
        if ( param == m_engine->RetKey(KEYRANK_LEFT, 0) ||
             param == m_engine->RetKey(KEYRANK_LEFT, 1) )
        {
            m_cameraPan = -1.0f;
        }

        if ( param == m_engine->RetKey(KEYRANK_RIGHT, 0) ||
             param == m_engine->RetKey(KEYRANK_RIGHT, 1) )
        {
            m_cameraPan = 1.0f;
        }

        if ( param == m_engine->RetKey(KEYRANK_UP, 0) ||
             param == m_engine->RetKey(KEYRANK_UP, 1) )
        {
            m_cameraZoom = -1.0f;
        }

        if ( param == m_engine->RetKey(KEYRANK_DOWN, 0) ||
             param == m_engine->RetKey(KEYRANK_DOWN, 1) )
        {
            m_cameraZoom = 1.0f;
        }
    }
}

// Panned with the camera if a button is pressed.

void CRobotMain::RemoteCamera(float pan, float zoom, float rTime)
{
    float   value;

    if ( pan != 0.0f )
    {
        value = m_camera->RetRemotePan();
        value += pan*rTime*1.5f;
        m_camera->SetRemotePan(value);
    }

    if ( zoom != 0.0f )
    {
        value = m_camera->RetRemoteZoom();
        value += zoom*rTime*0.3f;
        m_camera->SetRemoteZoom(value);
    }
}



// Cancels the current movie.

void CRobotMain::AbortMovie()
{
    CObject*    pObj;
    CAuto*      automat;
    int         i;

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        automat = pObj->RetAuto();
        if ( automat != 0 )
        {
            automat->Abort();
        }
    }

    m_engine->SetMouseHide(false);
}



// Updates the text information.

void CRobotMain::UpdateInfoText()
{
    CObject*    pObj;
    Math::Vector    pos;
    char        info[100];

    if ( m_bShowPos )
    {
        pObj = RetSelect();
        if ( pObj != 0 )
        {
            pos = pObj->RetPosition(0);
            sprintf(info, "Pos = %.2f ; %.2f", pos.x/g_unit, pos.z/g_unit);
            m_engine->SetInfoText(4, info);
        }
    }
}


// Initializes the view.

void CRobotMain::InitEye()
{
    if ( m_phase == PHASE_SIMUL )
    {
        m_camera->Init(Math::Vector( 0.0f, 10.0f, 0.0f),
                       Math::Vector(10.0f,  5.0f, 0.0f), 0.0f);
    }

    if ( m_phase == PHASE_MODEL )
    {
        m_model->InitView();
    }
}

// Advances the entire scene.

bool CRobotMain::EventFrame(const Event &event)
{
    ObjectType  type;
    CObject     *pObj, *toto;
    CPyro*      pPyro;
    CWindow*    pw;
    CMap*       pm;
    int         i;

    m_time += event.rTime;
    if ( !m_bMovieLock ) m_gameTime += event.rTime;

    if ( !m_bImmediatSatCom && !m_bBeginSatCom &&
         m_gameTime > 0.1f && m_phase == PHASE_SIMUL )
    {
        m_displayText->DisplayError(INFO_BEGINSATCOM, Math::Vector(0.0f,0.0f,0.0f));
        m_bBeginSatCom = true;  // message appears
    }

    m_water->EventProcess(event);
    m_cloud->EventProcess(event);
    m_blitz->EventProcess(event);
    m_planet->EventProcess(event);

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW1);
    if ( pw == 0 )
    {
        pm = 0;
    }
    else
    {
        pm = (CMap*)pw->SearchControl(EVENT_OBJECT_MAP);
        if ( pm != 0 )  pm->FlushObject();
    }

    toto = 0;
    if ( !m_bFreePhoto )
    {
        // Advances all the robots, but not toto.
        for ( i=0 ; i<1000000 ; i++ )
        {
            pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
            if ( pObj == 0 )  break;
            if ( pm != 0 )  pm->UpdateObject(pObj);
            if ( pObj->RetTruck() != 0 )  continue;
            type = pObj->RetType();
            if ( type == OBJECT_TOTO )
            {
                toto = pObj;
            }
            else
            {
                pObj->EventProcess(event);
            }
        }
        // Advances all objects transported by robots.
        for ( i=0 ; i<1000000 ; i++ )
        {
            pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
            if ( pObj == 0 )  break;
            if ( pObj->RetTruck() == 0 )  continue;
            pObj->EventProcess(event);
        }

        // Advances pyrotechnic effects.
        for ( i=0 ; i<1000000 ; i++ )
        {
            pPyro = (CPyro*)m_iMan->SearchInstance(CLASS_PYRO, i);
            if ( pPyro == 0 )  break;

            pPyro->EventProcess(event);
            if ( pPyro->IsEnded() != ERR_CONTINUE )
            {
                pPyro->DeleteObject();
                delete pPyro;
            }
        }
    }

    // The camera follows the object, because its position
    // may depend on the selected object (CAMERA_ONBOARD or CAMERA_BACK).
    if ( m_phase == PHASE_SIMUL && !m_bEditFull )
    {
        m_camera->EventProcess(event);

        if ( m_engine->RetFog() )
        {
            m_camera->SetOverBaseColor(m_particule->RetFogColor(m_engine->RetEyePt()));
        }
    }
    if ( m_phase == PHASE_PERSO ||
         m_phase == PHASE_WIN   ||
         m_phase == PHASE_LOST  )
    {
        m_camera->EventProcess(event);
    }

    // Advances toto following the camera, because its position depends on the camera.
    if ( toto != 0 )
    {
        toto->EventProcess(event);
    }

    // Advances model.
    if ( m_phase == PHASE_MODEL )
    {
        m_model->ViewMove(event, 2.0f);
        m_model->UpdateView();
        m_model->EventProcess(event);
    }

    HiliteFrame(event.rTime);

    // Moves the film indicator.
    if ( m_bMovieLock && !m_bEditLock )  // movie in progress?
    {
        CControl*   pc;
        Math::Point     pos, dim;
        float       zoom;

        pc = m_interface->SearchControl(EVENT_OBJECT_MOVIELOCK);
        if ( pc != 0 )
        {
            dim.x = 32.0f/640.0f;
            dim.y = 32.0f/480.0f;
            pos.x = 20.0f/640.0f;
            pos.y = (480.0f-24.0f)/480.0f;

            zoom = 1.0f+sinf(m_time*6.0f)*0.1f;  // 0.9 .. 1.1
            dim.x *= zoom;
            dim.y *= zoom;
            pos.x -= dim.x/2.0f;
            pos.y -= dim.y/2.0f;

            pc->SetPos(pos);
            pc->SetDim(dim);
        }
    }

    // Moves edition indicator.
    if ( m_bEditLock || m_bPause )  // edition in progress?
    {
        CControl*   pc;
        Math::Point     pos, dim;
        float       zoom;

        pc = m_interface->SearchControl(EVENT_OBJECT_EDITLOCK);
        if ( pc != 0 )
        {
            if ( m_bEditFull || m_bEditLock )
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

                zoom = 1.0f+sinf(m_time*6.0f)*0.1f;  // 0.9 .. 1.1
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
    if ( m_camera->RetType() == CAMERA_VISIT )
    {
        FrameVisit(event.rTime);
    }

    // Moves the boundaries.
    FrameShowLimit(event.rTime);

    if ( m_phase == PHASE_SIMUL )
    {
        if ( !m_bEditLock && m_checkEndTime+1.0f < m_time )
        {
            m_checkEndTime = m_time;
            CheckEndMission(true);
        }

        if ( m_winDelay > 0.0f && !m_bEditLock )
        {
            m_winDelay -= event.rTime;
            if ( m_winDelay <= 0.0f )
            {
                if ( m_bMovieLock )
                {
                    m_winDelay = 1.0f;
                }
                else
                {
                    Event       newEvent;
                    m_event->MakeEvent(newEvent, EVENT_WIN);
                    m_event->AddEvent(newEvent);
                }
            }
        }

        if ( m_lostDelay > 0.0f && !m_bEditLock )
        {
            m_lostDelay -= event.rTime;
            if ( m_lostDelay <= 0.0f )
            {
                if ( m_bMovieLock )
                {
                    m_winDelay = 1.0f;
                }
                else
                {
                    Event       newEvent;
                    m_event->MakeEvent(newEvent, EVENT_LOST);
                    m_event->AddEvent(newEvent);
                }
            }
        }
    }

    if ( m_delayWriteMessage > 0 )
    {
        m_delayWriteMessage --;
        if ( m_delayWriteMessage == 0 )
        {
            m_displayText->DisplayError(INFO_WRITEOK, Math::Vector(0.0f,0.0f,0.0f));
        }
    }

    return S_OK;
}

// Makes the event for all robots.

bool CRobotMain::EventObject(const Event &event)
{
    CObject*    pObj;
    int         i;

    if ( m_bFreePhoto )  return S_OK;

    m_bResetCreate = false;

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        pObj->EventProcess(event);
    }

    if ( m_bResetCreate )
    {
        ResetCreate();
    }

    return S_OK;
}


// Calculates the point of arrival of the camera.

Math::Vector CRobotMain::LookatPoint(Math::Vector eye, float angleH, float angleV,
                                  float length)
{
    Math::Vector    lookat;

    lookat = eye;
    lookat.z += length;

    RotatePoint(eye, angleH, angleV, lookat);
    return lookat;
}



char* SkipNum(char *p)
{
    while ( *p == ' ' || *p == '.' || *p == '-' || (*p >= '0' && *p <= '9') )
    {
        p++;
    }
    return p;
}

// Conversion of units.

void CRobotMain::Convert()
{
    FILE*           file = NULL;
    FILE*           fileNew = NULL;
    char            line[500];
    char            lineNew[500];
    char            s[200];
    char*           base;
    char*           p;
    int             rank;
    Math::Vector        pos;
    float           value;

    base = m_dialog->RetSceneName();
    rank = m_dialog->RetSceneRank();

    m_dialog->BuildSceneName(line, base, rank);
    file = fopen(line, "r");
    if ( file == NULL )  return;

    strcpy(line+strlen(line)-4, ".new");
    fileNew = fopen(line, "w");
    if ( fileNew == NULL )  return;

    while ( fgets(line, 500, file) != NULL )
    {
        strcpy(lineNew, line);

        if ( Cmd(line, "DeepView") )
        {
            p = strstr(line, "air=");
            if ( p != 0 )
            {
                value = OpFloat(line, "air", 500.0f);
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
            if ( p != 0 )
            {
                value = OpFloat(line, "water", 100.0f);
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

        if ( Cmd(line, "TerrainGenerate") )
        {
            p = strstr(line, "vision=");
            if ( p != 0 )
            {
                value = OpFloat(line, "vision", 500.0f);
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

        if ( Cmd(line, "CreateObject") ||
             Cmd(line, "CreateSpot")   )
        {
            p = strstr(line, "pos=");
            if ( p != 0 )
            {
                pos = OpPos(line, "pos");
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

        if ( Cmd(line, "EndMissionTake") )
        {
            p = strstr(line, "pos=");
            if ( p != 0 )
            {
                pos = OpPos(line, "pos");
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
            if ( p != 0 )
            {
                value = OpFloat(line, "dist", 32.0f);
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

        if ( Cmd(line, "Camera") )
        {
            p = strstr(line, "pos=");
            if ( p != 0 )
            {
                pos = OpPos(line, "pos");
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
            if ( p != 0 )
            {
                value = OpFloat(line, "h", 32.0f);
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

// Load the scene for the character.

void CRobotMain::ScenePerso()
{
    CObject*    pObj;

    DeleteAllObjects();  // removes all the current 3D Scene
    m_engine->FlushObject();
    m_terrain->FlushRelief();  // all flat
    m_terrain->FlushBuildingLevel();
    m_terrain->FlushFlyingLimit();
    m_light->FlushLight();
    m_particule->FlushParticule();
    m_iMan->Flush(CLASS_OBJECT);
    m_iMan->Flush(CLASS_PHYSICS);
    m_iMan->Flush(CLASS_BRAIN);
    m_iMan->Flush(CLASS_PYRO);

    m_dialog->SetSceneName("perso");
    m_dialog->SetSceneRank(0);
    CreateScene(false, true, false);  // sets scene

    m_engine->SetDrawWorld(false);  // does not draw anything on the interface
    m_engine->SetDrawFront(true);  // draws on the human interface
    pObj = SearchHuman();
    if ( pObj != 0 )
    {
        CMotionHuman*   mh;

        pObj->SetDrawFront(true);  // draws the interface

        mh = (CMotionHuman*)pObj->RetMotion();
        if ( mh != 0 )
        {
            mh->StartDisplayPerso();
        }
    }
}

// Creates the whole stage.

void CRobotMain::CreateScene(bool bSoluce, bool bFixScene, bool bResetObject)
{
    CObject*        pObj;
    CObject*        pSel;
    CMotion*        motion;
    FILE*           file = NULL;
    char            line[500];
    char            name[200];
    char            dir[100];
    char            op[100];
    char*           read;
    char*           stack;
    char*           base;
    D3DCOLORVALUE   color;
    Math::Vector        pos;
    int             rank, obj, i, rankObj, rankGadget;

//? Convert();

    base  = m_dialog->RetSceneName();
    rank  = m_dialog->RetSceneRank();
    read  = m_dialog->RetSceneRead();
    stack = m_dialog->RetStackRead();
    m_dialog->SetUserDir(base, rank);

    m_bFixScene = bFixScene;

    g_id = 0;
    m_bBase = false;

    if ( !bResetObject )
    {
        g_build = 0;
        g_researchDone = 0;  // no research done
        g_researchEnable = 0;

        FlushDisplayInfo();
        m_terrain->LevelFlush();
        m_audioTrack = 0;
        m_bAudioRepeat = true;
        m_displayText->SetDelay(1.0f);
        m_displayText->SetEnable(true);
        m_bImmediatSatCom = false;
        m_endingWinRank   = 0;
        m_endingLostRank  = 0;
        m_endTakeTotal = 0;
        m_endTakeResearch = 0;
        m_endTakeWinDelay = 2.0f;
        m_endTakeLostDelay = 2.0f;
        m_obligatoryTotal = 0;
        m_prohibitedTotal = 0;
        m_bMapShow = true;
        m_bMapImage = false;
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
    }

    m_dialog->BuildSceneName(line, base, rank);
    file = fopen(line, "r");
    if ( file == NULL )  return;

    rankObj = 0;
    rankGadget = 0;
    pSel = 0;

    while ( fgets(line, 500, file) != NULL )
    {
        for ( i=0 ; i<500 ; i++ )
        {
            if ( line[i] == '\t' )  line[i] = ' ';  // replace tab by space
            if ( line[i] == '/' && line[i+1] == '/' )
            {
                line[i] = 0;
                break;
            }
        }

        sprintf(op, "Title.%c", RetLanguageLetter());
        if ( Cmd(line, op) && !bResetObject )
        {
            OpString(line, "text", m_title);
        }

        sprintf(op, "Resume.%c", RetLanguageLetter());
        if ( Cmd(line, op) && !bResetObject )
        {
            OpString(line, "text", m_resume);
        }

        sprintf(op, "ScriptName.%c", RetLanguageLetter());
        if ( Cmd(line, op) && !bResetObject )
        {
            OpString(line, "text", m_scriptName);
        }

        if ( Cmd(line, "ScriptFile") && !bResetObject )
        {
            OpString(line, "name", m_scriptFile);
        }

        if ( Cmd(line, "Instructions") && !bResetObject )
        {
            OpString(line, "name", name);
//?         sprintf(m_infoFilename[SATCOM_HUSTON], "help\\%s", name);
            UserDir(m_infoFilename[SATCOM_HUSTON], name, "help");

            m_bImmediatSatCom = OpInt(line, "immediat", 0);
        }

        if ( Cmd(line, "Satellite") && !bResetObject )
        {
            OpString(line, "name", name);
//?         sprintf(m_infoFilename[SATCOM_SAT], "help\\%s", name);
            UserDir(m_infoFilename[SATCOM_SAT], name, "help");
        }

        if ( Cmd(line, "Loading") && !bResetObject )
        {
            OpString(line, "name", name);
//?         sprintf(m_infoFilename[SATCOM_LOADING], "help\\%s", name);
            UserDir(m_infoFilename[SATCOM_LOADING], name, "help");
        }

        if ( Cmd(line, "HelpFile") && !bResetObject )
        {
            OpString(line, "name", name);
//?         sprintf(m_infoFilename[SATCOM_PROG], "help\\%s", name);
            UserDir(m_infoFilename[SATCOM_PROG], name, "help");
        }
        if ( Cmd(line, "SoluceFile") && !bResetObject )
        {
            OpString(line, "name", name);
//?         sprintf(m_infoFilename[SATCOM_SOLUCE], "help\\%s", name);
            UserDir(m_infoFilename[SATCOM_SOLUCE], name, "help");
        }

        if ( Cmd(line, "EndingFile") && !bResetObject )
        {
            m_endingWinRank  = OpInt(line, "win",  0);
            m_endingLostRank = OpInt(line, "lost", 0);
        }

        if ( Cmd(line, "MessageDelay") && !bResetObject )
        {
            m_displayText->SetDelay(OpFloat(line, "factor", 1.0f));
        }

        if ( Cmd(line, "Audio") && !bResetObject )
        {
            m_audioTrack = OpInt(line, "track", 0);
            m_bAudioRepeat = OpInt(line, "repeat", 1);
        }

        if ( Cmd(line, "AmbiantColor") && !bResetObject )
        {
            m_engine->SetAmbiantColor(OpColor(line, "air",   0x88888888), 0);
            m_engine->SetAmbiantColor(OpColor(line, "water", 0x88888888), 1);
        }

        if ( Cmd(line, "FogColor") && !bResetObject )
        {
            m_engine->SetFogColor(OpColor(line, "air",   0x88888888), 0);
            m_engine->SetFogColor(OpColor(line, "water", 0x88888888), 1);
        }

        if ( Cmd(line, "VehicleColor") && !bResetObject )
        {
            m_colorNewBot = RetColor(OpColor(line, "color", 0x88888888));
        }

        if ( Cmd(line, "InsectColor") && !bResetObject )
        {
            m_colorNewAlien = RetColor(OpColor(line, "color", 0x88888888));
        }

        if ( Cmd(line, "GreeneryColor") && !bResetObject )
        {
            m_colorNewGreen = RetColor(OpColor(line, "color", 0x88888888));
        }

        if ( Cmd(line, "DeepView") && !bResetObject )
        {
            m_engine->SetDeepView(OpFloat(line, "air",   500.0f)*UNIT, 0, true);
            m_engine->SetDeepView(OpFloat(line, "water", 100.0f)*UNIT, 1, true);
        }

        if ( Cmd(line, "FogStart") && !bResetObject )
        {
            m_engine->SetFogStart(OpFloat(line, "air",   0.5f), 0);
            m_engine->SetFogStart(OpFloat(line, "water", 0.5f), 1);
        }

        if ( Cmd(line, "SecondTexture") && !bResetObject )
        {
            m_engine->SetSecondTexture(OpInt(line, "rank", 1));
        }

        if ( Cmd(line, "Background") && !bResetObject )
        {
            OpString(line, "image", name);
            UserDir(dir, name, "");
            m_engine->SetBackground(dir,
                                    OpColor(line, "up",        0x00000000),
                                    OpColor(line, "down",      0x00000000),
                                    OpColor(line, "cloudUp",   0x00000000),
                                    OpColor(line, "cloudDown", 0x00000000),
                                    OpInt(line, "full", 0));
        }

        if ( Cmd(line, "Planet") && !bResetObject )
        {
            Math::Vector    ppos, uv1, uv2;

            ppos  = OpPos(line, "pos");
            uv1   = OpPos(line, "uv1");
            uv2   = OpPos(line, "uv2");
            OpString(line, "image", name);
            UserDir(dir, name, "");
            m_planet->Create(OpInt(line, "mode", 0),
                             Math::Point(ppos.x, ppos.z),
                             OpFloat(line, "dim", 0.2f),
                             OpFloat(line, "speed", 0.0f),
                             OpFloat(line, "dir", 0.0f),
                             dir,
                             Math::Point(uv1.x, uv1.z),
                             Math::Point(uv2.x, uv2.z));
        }

        if ( Cmd(line, "FrontsizeName") && !bResetObject )
        {
            OpString(line, "image", name);
            UserDir(dir, name, "");
            m_engine->SetFrontsizeName(dir);
        }

        if ( Cmd(line, "Global") && !bResetObject )
        {
            g_unit = OpFloat(line, "unitScale", 4.0f);
            m_engine->SetTracePrecision(OpFloat(line, "traceQuality", 1.0f));
            m_bShortCut = OpInt(line, "shortcut", 1);
        }

        if ( Cmd(line, "TerrainGenerate") && !bResetObject )
        {
            m_terrain->Generate(OpInt(line, "mosaic", 20),
                                OpInt(line, "brick", 3),
                                OpFloat(line, "size", 20.0f),
                                OpFloat(line, "vision", 500.0f)*UNIT,
                                OpInt(line, "depth", 2),
                                OpFloat(line, "hard", 0.5f));
        }

        if ( Cmd(line, "TerrainWind") && !bResetObject )
        {
            m_terrain->SetWind(OpPos(line, "speed"));
        }

        if ( Cmd(line, "TerrainRelief") && !bResetObject )
        {
            OpString(line, "image", name);
            UserDir(dir, name, "textures");
            m_terrain->ReliefFromBMP(dir, OpFloat(line, "factor", 1.0f), OpInt(line, "border", 1));
        }

        if ( Cmd(line, "TerrainReliefDXF") && !bResetObject )
        {
            OpString(line, "image", name);
            UserDir(dir, name, "textures");
            m_terrain->ReliefFromDXF(dir, OpFloat(line, "factor", 1.0f));
        }

        if ( Cmd(line, "TerrainResource") && !bResetObject )
        {
            OpString(line, "image", name);
            UserDir(dir, name, "textures");
            m_terrain->ResFromBMP(dir);
        }

        if ( Cmd(line, "TerrainWater") && !bResetObject )
        {
            OpString(line, "image", name);
            UserDir(dir, name, "");
            pos.x = OpFloat(line, "moveX", 0.0f);
            pos.y = OpFloat(line, "moveY", 0.0f);
            pos.z = pos.x;
            m_water->Create(OpTypeWater(line, "air",   WATER_TT),
                            OpTypeWater(line, "water", WATER_TT),
                            dir,
                            RetColor(OpColor(line, "diffuse", 0xffffffff)),
                            RetColor(OpColor(line, "ambiant", 0xffffffff)),
                            OpFloat(line, "level", 100.0f)*UNIT,
                            OpFloat(line, "glint", 1.0f),
                            pos);
            m_colorNewWater = RetColor(OpColor(line, "color", RetColor(m_colorRefWater)));
            m_colorShiftWater = OpFloat(line, "brightness", 0.0f);
        }

        if ( Cmd(line, "TerrainLava") && !bResetObject )
        {
            m_water->SetLava(OpInt(line, "mode", 0));
        }

        if ( Cmd(line, "TerrainCloud") && !bResetObject )
        {
            OpString(line, "image", name);
            UserDir(dir, name, "");
            m_cloud->Create(dir,
                            RetColor(OpColor(line, "diffuse", 0xffffffff)),
                            RetColor(OpColor(line, "ambiant", 0xffffffff)),
                            OpFloat(line, "level", 500.0f)*UNIT);
        }

        if ( Cmd(line, "TerrainBlitz") && !bResetObject )
        {
            m_blitz->Create(OpFloat(line, "sleep", 0.0f),
                            OpFloat(line, "delay", 3.0f),
                            OpFloat(line, "magnetic", 50.0f)*UNIT);
        }

        if ( Cmd(line, "TerrainInitTextures") && !bResetObject )
        {
            int     dx, dy, tt[100];
            char*   op;

            OpString(line, "image", name);
            AddExt(name, ".tga");
            dx = OpInt(line, "dx", 1);
            dy = OpInt(line, "dy", 1);
            op = SearchOp(line, "table");
            for ( i=0 ; i<dx*dy ; i++ )
            {
                tt[i] = GetInt(op, i, 0);
            }

            if ( strstr(name, "%user%") != 0 )
            {
                CopyFileListToTemp(name, tt, dx*dy);
            }

            m_terrain->InitTextures(name, tt, dx, dy);
        }

        if ( Cmd(line, "TerrainInit") && !bResetObject )
        {
            m_terrain->LevelInit(OpInt(line, "id", 1));
        }

        if ( Cmd(line, "TerrainMaterial") && !bResetObject )
        {
            OpString(line, "image", name);
            AddExt(name, ".tga");
            if ( strstr(name, "%user%") != 0 )
            {
                CopyFileToTemp(name);
            }

            m_terrain->LevelMaterial(OpInt(line, "id", 0),
                                     name,
                                     OpFloat(line, "u", 0.0f),
                                     OpFloat(line, "v", 0.0f),
                                     OpInt(line, "up",    1),
                                     OpInt(line, "right", 1),
                                     OpInt(line, "down",  1),
                                     OpInt(line, "left",  1),
                                     OpFloat(line, "hard", 0.5f));
        }

        if ( Cmd(line, "TerrainLevel") && !bResetObject )
        {
            int     id[50];
            char*   op;

            op = SearchOp(line, "id");
            i = 0;
            while ( true )
            {
                id[i] = GetInt(op, i, 0);
                if ( id[i++] == 0 )  break;
            }

            m_terrain->LevelGenerate(id,
                                     OpFloat(line, "min", 0.0f)*UNIT,
                                     OpFloat(line, "max", 100.0f)*UNIT,
                                     OpFloat(line, "slope", 5.0f),
                                     OpFloat(line, "freq", 100.0f),
                                     OpPos(line, "center")*g_unit,
                                     OpFloat(line, "radius", 0.0f)*g_unit);
        }

        if ( Cmd(line, "TerrainCreate") && !bResetObject )
        {
            m_terrain->CreateObjects(true);
        }

        if ( Cmd(line, "BeginObject") )
        {
            InitEye();
            SetMovieLock(false);
            if ( !m_bFixScene )
            {
//?             CreateObject(Math::Vector(0.0f, 0.0f, 0.0f), 0.0f, 0.0f, OBJECT_TOTO);
            }

            if ( read[0] != 0 )  // loading file ?
            {
                pSel = IOReadScene(read, stack);
            }
        }

        if ( Cmd(line, "CreateObject") && read[0] == 0 )
        {
            CObject*    pObj;
            CBrain*     pBrain;
            CAuto*      pAuto;
            CPyro*      pyro;
            ObjectType  type;
            PyroType    pType;
            CameraType  cType;
            Info        info;
            float       dir;
            char        op[20];
            char        text[100];
            char*       p;
            int         run, gadget;

            type = OpTypeObject(line, "type", OBJECT_NULL);

            gadget = OpInt(line, "gadget", -1);
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
            if ( gadget != 0 )  // is this a gadget?
            {
                if ( !TestGadgetQuantity(rankGadget++) )  continue;
            }

            pos = OpPos(line, "pos")*g_unit;
            dir = OpFloat(line, "dir", 0.0f)*Math::PI;
            pObj = CreateObject(pos, dir,
                                OpFloat(line, "z", 1.0f),
                                OpFloat(line, "h", 0.0f),
                                type,
                                OpFloat(line, "power", 1.0f),
                                OpInt(line, "trainer", 0),
                                OpInt(line, "toy", 0),
                                OpInt(line, "option", 0));

            if ( pObj != 0 )
            {
                pObj->SetDefRank(rankObj);

                if ( type == OBJECT_BASE )  m_bBase = true;

                cType = OpCamera(line, "camera");
                if ( cType != CAMERA_NULL )
                {
                    pObj->SetCameraType(cType);
                }
                pObj->SetCameraDist(OpFloat(line, "cameraDist", 50.0f));
                pObj->SetCameraLock(OpInt(line, "cameraLock", 0));

                pType = OpPyro(line, "pyro");
                if ( pType != PT_NULL )
                {
                    pyro = new CPyro(m_iMan);
                    pyro->Create(pType, pObj);
                }

                // Puts information in terminal (OBJECT_INFO).
                for ( i=0 ; i<OBJECTMAXINFO ; i++ )
                {
                    sprintf(op, "info%d", i+1);
                    OpString(line, op, text);
                    if ( text[0] == 0 )  break;
                    p = strchr(text, '=');
                    if ( p == 0 )  break;
                    *p = 0;
                    strcpy(info.name, text);
                    sscanf(p+1, "%f", &info.value);
                    pObj->SetInfo(i, info);
                }

                // Sets the parameters of the command line.
                p = SearchOp(line, "cmdline");
                for ( i=0 ; i<OBJECTMAXCMDLINE ; i++ )
                {
                    float   value;
                    value = GetFloat(p, i, NAN);
                    if ( value == NAN )  break;
                    pObj->SetCmdLine(i, value);
                }

                if ( OpInt(line, "select", 0) == 1 )
                {
                    pSel = pObj;
                }

                pObj->SetSelectable(OpInt(line, "selectable", 1));
                pObj->SetEnable(OpInt(line, "enable", 1));
                pObj->SetProxyActivate(OpInt(line, "proxyActivate", 0));
                pObj->SetProxyDistance(OpFloat(line, "proxyDistance", 15.0f)*g_unit);
                pObj->SetRange(OpFloat(line, "range", 30.0f));
                pObj->SetShield(OpFloat(line, "shield", 1.0f));
                pObj->SetMagnifyDamage(OpFloat(line, "magnifyDamage", 1.0f));
                pObj->SetClip(OpInt(line, "clip", 1));
                pObj->SetCheckToken(OpInt(line, "checkToken", 1));
                pObj->SetManual(OpInt(line, "manual", 0));

                motion = pObj->RetMotion();
                if ( motion != 0 )
                {
                    p = SearchOp(line, "param");
                    for ( i=0 ; i<10 ; i++ )
                    {
                        float   value;
                        value = GetFloat(p, i, NAN);
                        if ( value == NAN )  break;
                        motion->SetParam(i, value);
                    }
                }

                run = -1;
                pBrain = pObj->RetBrain();
                if ( pBrain != 0 )
                {
                    for ( i=0 ; i<10 ; i++ )
                    {
                        sprintf(op, "script%d", i+1);  // script1..script10
                        OpString(line, op, name);
#if _SCHOOL
                        if ( !m_dialog->RetSoluce4() && i == 3 )  continue;
#endif
                        if ( name[0] != 0 )
                        {
                            pBrain->SetScriptName(i, name);
                        }
                    }

                    i = OpInt(line, "run", 0);
                    if ( i != 0 )
                    {
                        run = i-1;
                        pBrain->SetScriptRun(run);
                    }
                }
                pAuto = pObj->RetAuto();
                if ( pAuto != 0 )
                {
                    type = OpTypeObject(line, "autoType", OBJECT_NULL);
                    pAuto->SetType(type);
                    for ( i=0 ; i<5 ; i++ )
                    {
                        sprintf(op, "autoValue%d", i+1);  // autoValue1..autoValue5
                        pAuto->SetValue(i, OpFloat(line, op, 0.0f));
                    }
                    OpString(line, "autoString", name);
                    pAuto->SetString(name);

                    i = OpInt(line, "run", -1);
                    if ( i != -1 )
                    {
                        if ( i != PARAM_FIXSCENE &&
                             !m_dialog->RetMovies() )  i = 0;
                        pAuto->Start(i);  // starts the film
                    }
                }

                OpString(line, "soluce", name);
                if ( bSoluce && pBrain != 0 && name[0] != 0 )
                {
                    pBrain->SetSoluceName(name);
                }

                pObj->SetResetPosition(pObj->RetPosition(0));
                pObj->SetResetAngle(pObj->RetAngle(0));
                pObj->SetResetRun(run);

                if ( OpInt(line, "reset", 0) == 1 )
                {
                    pObj->SetResetCap(RESET_MOVE);
                }
            }

            rankObj ++;
        }

        if ( Cmd(line, "CreateFog") && !bResetObject )
        {
            ParticuleType   type;
            Math::Point         dim;
            float           height, ddim, delay;

            type = (ParticuleType)(PARTIFOG0+OpInt(line, "type", 0));
            pos = OpPos(line, "pos")*g_unit;
            height = OpFloat(line, "height", 1.0f)*g_unit;
            ddim = OpFloat(line, "dim", 50.0f)*g_unit;
            delay = OpFloat(line, "delay", 2.0f);
            m_terrain->MoveOnFloor(pos);
            pos.y += height;
            dim.x = ddim;
            dim.y = dim.x;
            m_particule->CreateParticule(pos, Math::Vector(0.0f, 0.0f, 0.0f), dim, type, delay, 0.0f, 0.0f);
        }

        if ( Cmd(line, "CreateLight") && !bResetObject )
        {
            D3DTypeObj  type;

            color.r = 0.5f;
            color.g = 0.5f;
            color.b = 0.5f;
            color.a = 1.0f;
            obj = CreateLight(OpDir(line, "dir"),
                              OpColorValue(line, "color", color));

            type = OpTypeTerrain(line, "type", TYPENULL);
            if ( type == TYPETERRAIN )
            {
                m_light->SetLightIncluType(obj, TYPETERRAIN);
            }
            if ( type == TYPEQUARTZ )
            {
                m_light->SetLightIncluType(obj, TYPEQUARTZ);
            }
            if ( type == TYPEMETAL )
            {
                m_light->SetLightIncluType(obj, TYPEMETAL);
            }
            if ( type == TYPEFIX )
            {
                m_light->SetLightExcluType(obj, TYPETERRAIN);
            }
        }
        if ( Cmd(line, "CreateSpot") && !bResetObject )
        {
            D3DTypeObj  type;

            color.r = 0.5f;
            color.g = 0.5f;
            color.b = 0.5f;
            color.a = 1.0f;
            obj = CreateSpot(OpDir(line, "pos")*g_unit,
                             OpColorValue(line, "color", color));

            type = OpTypeTerrain(line, "type", TYPENULL);
            if ( type == TYPETERRAIN )
            {
                m_light->SetLightIncluType(obj, TYPETERRAIN);
            }
            if ( type == TYPEQUARTZ )
            {
                m_light->SetLightIncluType(obj, TYPEQUARTZ);
            }
            if ( type == TYPEMETAL )
            {
                m_light->SetLightIncluType(obj, TYPEMETAL);
            }
            if ( type == TYPEFIX )
            {
                m_light->SetLightExcluType(obj, TYPETERRAIN);
            }
        }

        if ( Cmd(line, "GroundSpot") && !bResetObject )
        {
            rank = m_engine->GroundSpotCreate();
            if ( rank != -1 )
            {
                m_engine->SetObjectGroundSpotPos(rank, OpPos(line, "pos")*g_unit);
                m_engine->SetObjectGroundSpotRadius(rank, OpFloat(line, "radius", 10.0f)*g_unit);
                m_engine->SetObjectGroundSpotColor(rank, RetColor(OpColor(line, "color", 0x88888888)));
                m_engine->SetObjectGroundSpotSmooth(rank, OpFloat(line, "smooth", 1.0f));
                m_engine->SetObjectGroundSpotMinMax(rank, OpFloat(line, "min", 0.0f)*g_unit,
                                                          OpFloat(line, "max", 0.0f)*g_unit);
            }
        }

        if ( Cmd(line, "WaterColor") && !bResetObject )
        {
            color.r = 0.0f;
            color.g = 0.0f;
            color.b = 0.0f;
            color.a = 1.0f;
            m_engine->SetWaterAddColor(OpColorValue(line, "color", color));
        }

        if ( Cmd(line, "MapColor") && !bResetObject )
        {
            m_map->FloorColorMap(RetColor(OpColor(line, "floor", 0x88888888)),
                                 RetColor(OpColor(line, "water", 0x88888888)));
            m_bMapShow = OpInt(line, "show", 1);
            m_map->ShowMap(m_bMapShow);
            m_map->SetToy(OpInt(line, "toyIcon", 0));
            m_bMapImage = OpInt(line, "image", 0);
            if ( m_bMapImage )
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
        if ( Cmd(line, "MapZoom") && !bResetObject )
        {
            m_map->ZoomMap(OpFloat(line, "factor", 2.0f));
            m_map->MapEnable(OpInt(line, "enable", 1));
        }

        if ( Cmd(line, "MaxFlyingHeight") && !bResetObject )
        {
            m_terrain->SetFlyingMaxHeight(OpFloat(line, "max", 280.0f)*g_unit);
        }
        if ( Cmd(line, "AddFlyingHeight") && !bResetObject )
        {
            m_terrain->AddFlyingLimit(OpPos(line, "center")*g_unit,
                                      OpFloat(line, "extRadius", 20.0f)*g_unit,
                                      OpFloat(line, "intRadius", 10.0f)*g_unit,
                                      OpFloat(line, "maxHeight", 200.0f));
        }

        if ( Cmd(line, "Camera") )
        {
            m_camera->Init(OpDir(line, "eye")*g_unit,
                           OpDir(line, "lookat")*g_unit,
                           bResetObject?0.0f:OpFloat(line, "delay", 0.0f));

            if ( OpInt(line, "fadeIn", 0) == 1 )
            {
                m_camera->StartOver(OE_FADEINw, Math::Vector(0.0f, 0.0f, 0.0f), 1.0f);
            }
            m_camera->SetFixDirection(OpFloat(line, "fixDirection", 0.25f)*Math::PI);
        }

        if ( Cmd(line, "EndMissionTake") && !bResetObject )
        {
            i = m_endTakeTotal;
            if ( i < 10 )
            {
                m_endTake[i].pos  = OpPos(line, "pos")*g_unit;
                m_endTake[i].dist = OpFloat(line, "dist", 8.0f)*g_unit;
                m_endTake[i].type = OpTypeObject(line, "type", OBJECT_NULL);
                m_endTake[i].min  = OpInt(line, "min", 1);
                m_endTake[i].max  = OpInt(line, "max", 9999);
                m_endTake[i].lost = OpInt(line, "lost", -1);
                m_endTake[i].bImmediat = OpInt(line, "immediat", 0);
                OpString(line, "message", m_endTake[i].message);
                m_endTakeTotal ++;
            }
        }
        if ( Cmd(line, "EndMissionDelay") && !bResetObject )
        {
            m_endTakeWinDelay  = OpFloat(line, "win",  2.0f);
            m_endTakeLostDelay = OpFloat(line, "lost", 2.0f);
        }
        if ( Cmd(line, "EndMissionResearch") && !bResetObject )
        {
            m_endTakeResearch |= OpResearch(line, "type");
        }

        if ( Cmd(line, "ObligatoryToken") && !bResetObject )
        {
            i = m_obligatoryTotal;
            if ( i < 100 )
            {
                OpString(line, "text", m_obligatoryToken[i]);
                m_obligatoryTotal ++;
            }
        }

        if ( Cmd(line, "ProhibitedToken") && !bResetObject )
        {
            i = m_prohibitedTotal;
            if ( i < 100 )
            {
                OpString(line, "text", m_prohibitedToken[i]);
                m_prohibitedTotal ++;
            }
        }

        if ( Cmd(line, "EnableBuild") && !bResetObject )
        {
            g_build |= OpBuild(line, "type");
        }

        if ( Cmd(line, "EnableResearch") && !bResetObject )
        {
            g_researchEnable |= OpResearch(line, "type");
        }
        if ( Cmd(line, "DoneResearch") && read[0] == 0 && !bResetObject )  // not loading file?
        {
            g_researchDone |= OpResearch(line, "type");
        }

        if ( Cmd(line, "NewScript") && !bResetObject )
        {
            OpString(line, "name", name);
            AddNewScriptName(OpTypeObject(line, "type", OBJECT_NULL), name);
        }
    }

    fclose(file);

    if ( read[0] == 0 )
    {
        CompileScript(bSoluce);  // compiles all scripts
    }

    if ( strcmp(base, "scene") == 0 && !bResetObject )  // mission?
    {
        WriteFreeParam();
    }
    if ( strcmp(base, "free") == 0 && !bResetObject )  // free play?
    {
        g_researchDone = m_freeResearch;

        g_build = m_freeBuild;
        g_build &= ~BUILD_RESEARCH;
        g_build &= ~BUILD_LABO;
        g_build |= BUILD_FACTORY;
        g_build |= BUILD_GFLAT;
        g_build |= BUILD_FLAG;
    }

    if ( !bResetObject )
    {
        ChangeColor();  // changes the colors of texture
        m_short->SetMode(false);  // vehicles?
    }

    CreateShortcuts();
    m_map->UpdateMap();
    m_engine->TimeInit();
    m_engine->FlushPressKey();
    m_time = 0.0f;
    m_gameTime = 0.0f;
    m_checkEndTime = 0.0f;
    m_infoUsed = 0;

    m_selectObject = pSel;

    if ( !m_bBase     &&  // no main base?
         !m_bFixScene )   // interractive scene?
    {
        if ( pSel == 0 )
        {
            pObj = SearchHuman();
        }
        else
        {
            pObj = pSel;
        }
        if ( pObj != 0 )
        {
            SelectObject(pObj);
            m_camera->SetObject(pObj);
//?         m_camera->SetType(CAMERA_BACK);
            m_camera->SetType(pObj->RetCameraType());
        }
    }
    if ( m_bFixScene )
    {
        m_camera->SetType(CAMERA_SCRIPT);
    }

    if ( read[0] != 0 && pSel != 0 )  // loading file?
    {
        pos = pSel->RetPosition(0);
        m_camera->Init(pos, pos, 0.0f);
        m_camera->FixCamera();

        SelectObject(pSel);
        m_camera->SetObject(pSel);

        m_bBeginSatCom = true;  // message already displayed
    }
    m_dialog->SetSceneRead("");
    m_dialog->SetStackRead("");
}

// Creates an object of decoration mobile or stationary.

CObject* CRobotMain::CreateObject(Math::Vector pos, float angle, float zoom, float height,
                                  ObjectType type, float power,
                                  bool bTrainer, bool bToy,
                                  int option)
{
    CObject*    pObject = 0;
    CAuto*      automat;

    if ( type == OBJECT_NULL )  return 0;

    if ( type == OBJECT_HUMAN ||
         type == OBJECT_TECH  )
    {
        bTrainer = false;  // necessarily
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
        pObject = new CObject(m_iMan);
        pObject->CreateBuilding(pos, angle, height, type, power);

        automat = pObject->RetAuto();
        if ( automat != 0 )
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
        pObject = new CObject(m_iMan);
        pObject->CreateResource(pos, angle, type, power);
    }
    else
    if ( type == OBJECT_FLAGb ||
         type == OBJECT_FLAGr ||
         type == OBJECT_FLAGg ||
         type == OBJECT_FLAGy ||
         type == OBJECT_FLAGv )
    {
        pObject = new CObject(m_iMan);
        pObject->CreateFlag(pos, angle, type);
    }
    else
    if ( type == OBJECT_BARRIER0 ||
         type == OBJECT_BARRIER1 ||
         type == OBJECT_BARRIER2 ||
         type == OBJECT_BARRIER3 ||
         type == OBJECT_BARRIER4 )
    {
        pObject = new CObject(m_iMan);
        pObject->CreateBarrier(pos, angle, height, type);
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
        pObject = new CObject(m_iMan);
        pObject->CreatePlant(pos, angle, height, type);
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
        pObject = new CObject(m_iMan);
        pObject->CreateMushroom(pos, angle, height, type);
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
        pObject = new CObject(m_iMan);
        pObject->SetOption(option);
        pObject->CreateTeen(pos, angle, zoom, height, type);
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
        pObject = new CObject(m_iMan);
        pObject->CreateQuartz(pos, angle, height, type);
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
        pObject = new CObject(m_iMan);
        pObject->CreateRoot(pos, angle, height, type);
    }
    else
    if ( type == OBJECT_HOME1 )
    {
        pObject = new CObject(m_iMan);
        pObject->CreateHome(pos, angle, height, type);
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
        pObject = new CObject(m_iMan);
        pObject->CreateRuin(pos, angle, height, type);
    }
    else
    if ( type == OBJECT_APOLLO1 ||
         type == OBJECT_APOLLO3 ||
         type == OBJECT_APOLLO4 ||
         type == OBJECT_APOLLO5 )
    {
        pObject = new CObject(m_iMan);
        pObject->CreateApollo(pos, angle, type);
    }
    else
    if ( type == OBJECT_MOTHER ||
         type == OBJECT_ANT    ||
         type == OBJECT_SPIDER ||
         type == OBJECT_BEE    ||
         type == OBJECT_WORM   )
    {
        pObject = new CObject(m_iMan);
        pObject->CreateInsect(pos, angle, type);  // no eggs
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
        pObject = new CObject(m_iMan);
        pObject->SetOption(option);
        pObject->CreateVehicle(pos, angle, type, power, bTrainer, bToy);
    }

    if ( m_bFixScene && type == OBJECT_HUMAN )
    {
        CMotion*    motion;

        motion = pObject->RetMotion();
        if ( m_phase == PHASE_WIN  )  motion->SetAction(MHS_WIN,  0.4f);
        if ( m_phase == PHASE_LOST )  motion->SetAction(MHS_LOST, 0.5f);
    }

    return pObject;
}


// Creates the editable model.

void CRobotMain::CreateModel()
{
    Math::Vector        direction;
    D3DCOLORVALUE   color;

    m_engine->SetAmbiantColor(0xC0C0C0C0);  // gray
    m_engine->SetBackground("", 0x80808080, 0x80808080, 0x80808080, 0x80808080);
    m_engine->SetFogColor(0x80808080);
    m_engine->SetDeepView(500.0f, 0);
    m_engine->SetDeepView(100.0f, 1);
    m_engine->SetFogStart(0.5f);

    m_model->StartUserAction();

    direction = Math::Vector(1.0f, -1.0f, 1.0f);
    color.r = 0.7f;
    color.g = 0.7f;
    color.b = 0.7f;  // white
    CreateLight(direction, color);

    direction = Math::Vector(-1.0f, -1.0f, 1.0f);
    color.r = 0.7f;
    color.g = 0.7f;
    color.b = 0.7f;  // white
    CreateLight(direction, color);

    direction = Math::Vector(1.0f, -1.0f, -1.0f);
    color.r = 0.7f;
    color.g = 0.7f;
    color.b = 0.7f;  // white
    CreateLight(direction, color);

    direction = Math::Vector(-1.0f, -1.0f, -1.0f);
    color.r = 0.7f;
    color.g = 0.7f;
    color.b = 0.7f;  // white
    CreateLight(direction, color);

    direction = Math::Vector(0.0f, 1.0f, 0.0f);
    color.r = 0.7f;
    color.g = 0.7f;
    color.b = 0.7f;  // white
    CreateLight(direction, color);

    InitEye();

    m_engine->TimeInit();
    m_time = 0.0f;
    m_gameTime = 0.0f;
    m_checkEndTime = 0.0f;
}


// Creates a directional light.

int CRobotMain::CreateLight(Math::Vector direction, D3DCOLORVALUE color)
{
    D3DLIGHT7   light;
    int         obj;

    if ( direction.x == 0.0f &&
         direction.y == 0.0f &&
         direction.z == 0.0f )
    {
        direction.y = -1.0f;
    }

    ZeroMemory(&light, sizeof(D3DLIGHT7));
    light.dltType      = D3DLIGHT_DIRECTIONAL;
    light.dcvDiffuse.r = color.r;
    light.dcvDiffuse.g = color.g;
    light.dcvDiffuse.b = color.b;
    light.dvDirection  = VEC_TO_D3DVEC(direction);
    obj = m_light->CreateLight();
    m_light->SetLight(obj, light);

    return obj;
}

// Creates a light spot.

int CRobotMain::CreateSpot(Math::Vector pos, D3DCOLORVALUE color)
{
    D3DLIGHT7   light;
    int         obj;

    if ( !m_engine->RetLightMode() )  return -1;

    pos.y += m_terrain->RetFloorLevel(pos);

    ZeroMemory(&light, sizeof(D3DLIGHT7));
    light.dltType        = D3DLIGHT_SPOT;
    light.dcvDiffuse.r   = color.r;
    light.dcvDiffuse.g   = color.g;
    light.dcvDiffuse.b   = color.b;
    light.dvPosition     = VEC_TO_D3DVEC(pos);
    light.dvDirection    = D3DVECTOR(0.0f, -1.0f, 0.0f);
    light.dvRange        = D3DLIGHT_RANGE_MAX;
    light.dvFalloff      = 1.0f;
    light.dvTheta        = 10.0f*Math::PI/180.0f;
    light.dvPhi          = 90.0f*Math::PI/180.0f;
    light.dvAttenuation0 = 2.0f;
    light.dvAttenuation1 = 0.0f;
    light.dvAttenuation2 = 0.0f;
    obj = m_light->CreateLight();
    m_light->SetLight(obj, light);

    return obj;
}


// Change the colors and textures.

void CRobotMain::ChangeColor()
{
    D3DCOLORVALUE   colorRef1, colorNew1, colorRef2, colorNew2;
    Math::Point         ts, ti;
    Math::Point         exclu[6];
    char            name[100];
    int             face;
    float           tolerance;

    ts = Math::Point(0.0f, 0.0f);
    ti = Math::Point(1.0f, 1.0f);  // the entire image

    colorRef1.a = 0.0f;
    colorRef2.a = 0.0f;

    colorRef1.r = 206.0f/256.0f;
    colorRef1.g = 206.0f/256.0f;
    colorRef1.b = 204.0f/256.0f;  // ~white
    colorNew1 = m_dialog->RetGamerColorCombi();
    colorRef2.r = 255.0f/256.0f;
    colorRef2.g = 132.0f/256.0f;
    colorRef2.b =   1.0f/256.0f;  // orange
    colorNew2 = m_dialog->RetGamerColorBand();
    exclu[0] = Math::Point(192.0f/256.0f,   0.0f/256.0f);
    exclu[1] = Math::Point(256.0f/256.0f,  64.0f/256.0f);  // crystals + cylinders
    exclu[2] = Math::Point(208.0f/256.0f, 224.0f/256.0f);
    exclu[3] = Math::Point(256.0f/256.0f, 256.0f/256.0f);  // SatCom screen
    exclu[4] = Math::Point(0.0f, 0.0f);
    exclu[5] = Math::Point(0.0f, 0.0f);  // terminator
    m_engine->ChangeColor("human.tga", colorRef1, colorNew1, colorRef2, colorNew2, 0.30f, 0.01f, ts, ti, exclu);

    face = RetGamerFace();
    if ( face == 0 )  // normal?
    {
        colorRef1.r =  90.0f/256.0f;
        colorRef1.g =  95.0f/256.0f;
        colorRef1.b =  85.0f/256.0f;  // black
        tolerance = 0.15f;
    }
    if ( face == 1 )  // bald?
    {
        colorRef1.r =  74.0f/256.0f;
        colorRef1.g =  58.0f/256.0f;
        colorRef1.b =  46.0f/256.0f;  // brown
        tolerance = 0.20f;
    }
    if ( face == 2 )  // carlos?
    {
        colorRef1.r =  70.0f/256.0f;
        colorRef1.g =  40.0f/256.0f;
        colorRef1.b =   8.0f/256.0f;  // brown
        tolerance = 0.30f;
    }
    if ( face == 3 )  // blonde?
    {
        colorRef1.r =  74.0f/256.0f;
        colorRef1.g =  16.0f/256.0f;
        colorRef1.b =   0.0f/256.0f;  // yellow
        tolerance = 0.20f;
    }
    colorNew1 = m_dialog->RetGamerColorHair();
    colorRef2.r = 0.0f;
    colorRef2.g = 0.0f;
    colorRef2.b = 0.0f;
    colorNew2.r = 0.0f;
    colorNew2.g = 0.0f;
    colorNew2.b = 0.0f;
    sprintf(name, "face%.2d.tga", face+1);
    exclu[0] = Math::Point(105.0f/256.0f, 47.0f/166.0f);
    exclu[1] = Math::Point(153.0f/256.0f, 79.0f/166.0f);  // blue canister
    exclu[2] = Math::Point(0.0f, 0.0f);
    exclu[3] = Math::Point(0.0f, 0.0f);  // terminator
    m_engine->ChangeColor(name, colorRef1, colorNew1, colorRef2, colorNew2, tolerance, 0.00f, ts, ti, exclu);

    colorRef2.r = 0.0f;
    colorRef2.g = 0.0f;
    colorRef2.b = 0.0f;
    colorNew2.r = 0.0f;
    colorNew2.g = 0.0f;
    colorNew2.b = 0.0f;

    m_engine->ChangeColor("base1.tga",   m_colorRefBot, m_colorNewBot, colorRef2, colorNew2, 0.10f, -1.0f, ts, ti, 0, 0, true);
    m_engine->ChangeColor("convert.tga", m_colorRefBot, m_colorNewBot, colorRef2, colorNew2, 0.10f, -1.0f, ts, ti, 0, 0, true);
    m_engine->ChangeColor("derrick.tga", m_colorRefBot, m_colorNewBot, colorRef2, colorNew2, 0.10f, -1.0f, ts, ti, 0, 0, true);
    m_engine->ChangeColor("factory.tga", m_colorRefBot, m_colorNewBot, colorRef2, colorNew2, 0.10f, -1.0f, ts, ti, 0, 0, true);
    m_engine->ChangeColor("lemt.tga",    m_colorRefBot, m_colorNewBot, colorRef2, colorNew2, 0.10f, -1.0f, ts, ti, 0, 0, true);
    m_engine->ChangeColor("roller.tga",  m_colorRefBot, m_colorNewBot, colorRef2, colorNew2, 0.10f, -1.0f, ts, ti, 0, 0, true);
    m_engine->ChangeColor("search.tga",  m_colorRefBot, m_colorNewBot, colorRef2, colorNew2, 0.10f, -1.0f, ts, ti, 0, 0, true);

    exclu[0] = Math::Point(  0.0f/256.0f, 160.0f/256.0f);
    exclu[1] = Math::Point(256.0f/256.0f, 256.0f/256.0f);  // pencils
    exclu[2] = Math::Point(0.0f, 0.0f);
    exclu[3] = Math::Point(0.0f, 0.0f);  // terminator
    m_engine->ChangeColor("drawer.tga",  m_colorRefBot, m_colorNewBot, colorRef2, colorNew2, 0.10f, -1.0f, ts, ti, exclu, 0, true);

    exclu[0] = Math::Point(237.0f/256.0f, 176.0f/256.0f);
    exclu[1] = Math::Point(256.0f/256.0f, 220.0f/256.0f);  // blue canister
    exclu[2] = Math::Point(106.0f/256.0f, 150.0f/256.0f);
    exclu[3] = Math::Point(130.0f/256.0f, 214.0f/256.0f);  // safe location
    exclu[4] = Math::Point(0.0f, 0.0f);
    exclu[5] = Math::Point(0.0f, 0.0f);  // terminator
    m_engine->ChangeColor("subm.tga",    m_colorRefBot, m_colorNewBot, colorRef2, colorNew2, 0.10f, -1.0f, ts, ti, exclu, 0, true);

    exclu[0] = Math::Point(128.0f/256.0f, 160.0f/256.0f);
    exclu[1] = Math::Point(256.0f/256.0f, 256.0f/256.0f);  // SatCom
    exclu[2] = Math::Point(0.0f, 0.0f);
    exclu[3] = Math::Point(0.0f, 0.0f);  // terminator
    m_engine->ChangeColor("ant.tga",     m_colorRefAlien, m_colorNewAlien, colorRef2, colorNew2, 0.50f, -1.0f, ts, ti, exclu);
    m_engine->ChangeColor("mother.tga",  m_colorRefAlien, m_colorNewAlien, colorRef2, colorNew2, 0.50f, -1.0f, ts, ti);

    m_engine->ChangeColor("plant.tga",   m_colorRefGreen, m_colorNewGreen, colorRef2, colorNew2, 0.50f, -1.0f, ts, ti);

    // PARTIPLOUF0 and PARTIDROP :
    ts = Math::Point(0.500f, 0.500f);
    ti = Math::Point(0.875f, 0.750f);
    m_engine->ChangeColor("effect00.tga", m_colorRefWater, m_colorNewWater, colorRef2, colorNew2, 0.20f, -1.0f, ts, ti, 0, m_colorShiftWater, true);

    // PARTIFLIC :
    ts = Math::Point(0.00f, 0.75f);
    ti = Math::Point(0.25f, 1.00f);
    m_engine->ChangeColor("effect02.tga", m_colorRefWater, m_colorNewWater, colorRef2, colorNew2, 0.20f, -1.0f, ts, ti, 0, m_colorShiftWater, true);
}

// Updates the number of unnecessary objects.

bool CRobotMain::TestGadgetQuantity(int rank)
{
    float       percent;
    int         *table;

    static int table10[10] = {0,1,0,0,0,0,0,0,0,0};
    static int table20[10] = {0,1,0,0,0,1,0,0,0,0};
    static int table30[10] = {0,1,0,1,0,1,0,0,0,0};
    static int table40[10] = {0,1,0,1,0,1,0,1,0,0};
    static int table50[10] = {0,1,0,1,0,1,0,1,0,1};
    static int table60[10] = {0,1,0,1,1,1,0,1,0,1};
    static int table70[10] = {0,1,0,1,1,1,0,1,1,1};
    static int table80[10] = {0,1,1,1,1,1,0,1,1,1};
    static int table90[10] = {0,1,1,1,1,1,1,1,1,1};

    percent = m_engine->RetGadgetQuantity();
    if ( percent == 0.0f )  return false;
    if ( percent == 1.0f )  return true;

         if ( percent <= 0.15f )  table = table10;
    else if ( percent <= 0.25f )  table = table20;
    else if ( percent <= 0.35f )  table = table30;
    else if ( percent <= 0.45f )  table = table40;
    else if ( percent <= 0.55f )  table = table50;
    else if ( percent <= 0.65f )  table = table60;
    else if ( percent <= 0.75f )  table = table70;
    else if ( percent <= 0.85f )  table = table80;
    else                          table = table90;

    return table[rank%10];
}



// Calculates the distance to the nearest object.

float CRobotMain::SearchNearestObject(Math::Vector center, CObject *exclu)
{
    CObject*    pObj;
    ObjectType  type;
    Math::Vector    oPos;
    float       min, dist, oRadius;
    int         i, j;

    min = 100000.0f;
    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        if ( !pObj->RetActif() )  continue;  // inactive?
        if ( pObj->RetTruck() != 0 )  continue;  // object carries?
        if ( pObj == exclu )  continue;

        type = pObj->RetType();

        if ( type == OBJECT_BASE )
        {
            oPos = pObj->RetPosition(0);
            if ( oPos.x != center.x ||
                 oPos.z != center.z )
            {
                dist = Math::Distance(center, oPos)-80.0f;
                if ( dist < 0.0f )  dist = 0.0f;
                min = Math::Min(min, dist);
                continue;
            }
        }

        if ( type == OBJECT_STATION   ||
             type == OBJECT_REPAIR    ||
             type == OBJECT_DESTROYER )
        {
            oPos = pObj->RetPosition(0);
            dist = Math::Distance(center, oPos)-8.0f;
            if ( dist < 0.0f )  dist = 0.0f;
            min = Math::Min(min, dist);
        }

        j = 0;
        while ( pObj->GetCrashSphere(j++, oPos, oRadius) )
        {
            dist = Math::Distance(center, oPos)-oRadius;
            if ( dist < 0.0f )  dist = 0.0f;
            min = Math::Min(min, dist);
        }
    }
    return min;
}

// Calculates a free space.

bool CRobotMain::FreeSpace(Math::Vector &center, float minRadius, float maxRadius,
                           float space, CObject *exclu)
{
    Math::Vector    pos;
    Math::Point     p;
    float       radius, ia, angle, dist, flat;

    if ( minRadius < maxRadius )  // from internal to external?
    {
        for ( radius=minRadius ; radius<=maxRadius ; radius+=space )
        {
            ia = space/radius;
            for ( angle=0.0f ; angle<Math::PI*2.0f ; angle+=ia )
            {
                p.x = center.x+radius;
                p.y = center.z;
                p = Math::RotatePoint(Math::Point(center.x, center.z), angle, p);
                pos.x = p.x;
                pos.z = p.y;
                pos.y = 0.0f;
                m_terrain->MoveOnFloor(pos, true);
                dist = SearchNearestObject(pos, exclu);
                if ( dist >= space )
                {
                    flat = m_terrain->RetFlatZoneRadius(pos, dist/2.0f);
                    if ( flat >= dist/2.0f )
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
        for ( radius=maxRadius ; radius>=minRadius ; radius-=space )
        {
            ia = space/radius;
            for ( angle=0.0f ; angle<Math::PI*2.0f ; angle+=ia )
            {
                p.x = center.x+radius;
                p.y = center.z;
                p = Math::RotatePoint(Math::Point(center.x, center.z), angle, p);
                pos.x = p.x;
                pos.z = p.y;
                pos.y = 0.0f;
                m_terrain->MoveOnFloor(pos, true);
                dist = SearchNearestObject(pos, exclu);
                if ( dist >= space )
                {
                    flat = m_terrain->RetFlatZoneRadius(pos, dist/2.0f);
                    if ( flat >= dist/2.0f )
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

// Calculates the maximum radius of a free space.

float CRobotMain::RetFlatZoneRadius(Math::Vector center, float maxRadius,
                                    CObject *exclu)
{
    float   dist;

    dist = SearchNearestObject(center, exclu);
    if ( dist == 0.0f )  return 0.0f;
    if ( dist < maxRadius )
    {
        maxRadius = dist;
    }
    return m_terrain->RetFlatZoneRadius(center, maxRadius);
}


// Hides buildable area when a cube of metal is taken up.

void CRobotMain::HideDropZone(CObject* metal)
{
    if ( m_showLimit[1].bUsed         &&
         m_showLimit[1].link == metal )
    {
        FlushShowLimit(1);
    }

    if ( m_showLimit[2].bUsed         &&
         m_showLimit[2].link == metal )
    {
        FlushShowLimit(2);
    }
}

// Shows the buildable area when a cube of metal is deposited.

void CRobotMain::ShowDropZone(CObject* metal, CObject* truck)
{
    CObject*    pObj;
    ObjectType  type;
    Math::Vector    center, oPos;
    float       oMax, tMax, dist, oRadius, radius;
    int         i, j;

    if ( metal == 0 )  return;

    center = metal->RetPosition(0);

    // Calculates the maximum radius possible depending on other items.
    oMax = 30.0f;  // radius to build the biggest building
    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        if ( !pObj->RetActif() )  continue;  // inactive?
        if ( pObj->RetTruck() != 0 )  continue;  // object carried?
        if ( pObj == metal )  continue;
        if ( pObj == truck )  continue;

        type = pObj->RetType();
        if ( type == OBJECT_BASE )
        {
            oPos = pObj->RetPosition(0);
            dist = Math::Distance(center, oPos)-80.0f;
            oMax = Math::Min(oMax, dist);
        }
        else
        {
            j = 0;
            while ( pObj->GetCrashSphere(j++, oPos, oRadius) )
            {
                dist = Math::Distance(center, oPos)-oRadius;
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
            j = 0;
            while ( pObj->GetCrashSphere(j++, oPos, oRadius) )
            {
                dist = Math::Distance(center, oPos)-oRadius-BUILDMARGIN;
                oMax = Math::Min(oMax, dist);
            }
        }
    }

    // Calculates the maximum possible radius depending on terrain.
    if ( oMax >= 2.0f )
    {
        tMax = m_terrain->RetFlatZoneRadius(center, 30.0f);
    }
    else
    {
        tMax = 0.0f;
    }

    radius = Math::Min(oMax, tMax);
    if ( radius >= 2.0f )
    {
        SetShowLimit(1, PARTILIMIT2, metal, center, radius, 10.0f);
    }
}

// Erases the boundaries shown.

void CRobotMain::FlushShowLimit(int i)
{
    int     j;

    if ( m_showLimit[i].link != 0 )
    {
        m_showLimit[i].link->StopShowLimit();
    }

    for ( j=0 ; j<m_showLimit[i].total ; j++ )
    {
        if ( m_showLimit[i].parti[j] == 0 )  continue;

        m_particule->DeleteParticule(m_showLimit[i].parti[j]);
        m_showLimit[i].parti[j] = 0;
    }

    m_showLimit[i].total = 0;
    m_showLimit[i].link = 0;
    m_showLimit[i].bUsed = false;
}

// Specifies the boundaries to show.

void CRobotMain::SetShowLimit(int i, ParticuleType parti, CObject *pObj,
                              Math::Vector pos, float radius, float duration)
{
    Math::Point dim;
    float   dist;
    int     j;

    FlushShowLimit(i);  // erases the current boundaries

    if ( radius <= 0.0f )  return;

    if ( radius <= 50.0f )
    {
        dim = Math::Point(0.3f, 0.3f);
        dist = 2.5f;
    }
    else
    {
        dim = Math::Point(1.5f, 1.5f);
        dist = 10.0f;
    }

    m_showLimit[i].bUsed = true;
    m_showLimit[i].link = pObj;
    m_showLimit[i].pos = pos;
    m_showLimit[i].radius = radius;
    m_showLimit[i].duration = duration;
    m_showLimit[i].total = (int)((radius*2.0f*Math::PI)/dist);
    if ( m_showLimit[i].total > MAXSHOWPARTI )  m_showLimit[i].total = MAXSHOWPARTI;
    m_showLimit[i].time = 0.0f;

    for ( j=0 ; j<m_showLimit[i].total ; j++ )
    {
        m_showLimit[i].parti[j] = m_particule->CreateParticule(pos, Math::Vector(0.0f, 0.0f, 0.0f), dim, parti, duration);
    }
}

// Adjusts the boundaries to show.

void CRobotMain::AdjustShowLimit(int i, Math::Vector pos)
{
    m_showLimit[i].pos = pos;
}

// Mount the boundaries of the selected object.

void CRobotMain::StartShowLimit()
{
    CObject*    pObj;

    pObj = RetSelect();
    if ( pObj == 0 )  return;

    pObj->StartShowLimit();
}

// Advances the boundaries shown.

void CRobotMain::FrameShowLimit(float rTime)
{
    Math::Vector    pos;
    Math::Point     center, rotate;
    float       angle, factor, speed;
    int         i, j;

    if ( m_engine->RetPause() )  return;

    for ( i=0 ; i<MAXSHOWLIMIT ; i++ )
    {
        if ( !m_showLimit[i].bUsed )  continue;

        m_showLimit[i].time += rTime;

        if ( m_showLimit[i].time >= m_showLimit[i].duration )
        {
            FlushShowLimit(i);
            continue;
        }

        if ( m_showLimit[i].time < 1.0f )
        {
            factor = m_showLimit[i].time;
        }
        else if ( m_showLimit[i].time > m_showLimit[i].duration-1.0f )
        {
            factor = m_showLimit[i].duration-m_showLimit[i].time;
        }
        else
        {
            factor = 1.0f;
        }

        speed = 0.4f-m_showLimit[i].radius*0.001f;
        if ( speed < 0.1f )  speed = 0.1f;
        angle = m_showLimit[i].time*speed;

        for ( j=0 ; j<m_showLimit[i].total ; j++ )
        {
            if ( m_showLimit[i].parti[j] == 0 )  continue;

            center.x = m_showLimit[i].pos.x;
            center.y = m_showLimit[i].pos.z;
            rotate.x = center.x+m_showLimit[i].radius*factor;
            rotate.y = center.y;
            rotate = Math::RotatePoint(center, angle, rotate);

            pos.x = rotate.x;
            pos.z = rotate.y;
            pos.y = 0.0f;
            m_terrain->MoveOnFloor(pos, true);
            if ( m_showLimit[i].radius <= 50.0f )  pos.y += 0.5f;
            else                                   pos.y += 2.0f;
            m_particule->SetPosition(m_showLimit[i].parti[j], pos);
//?         m_particule->SetAngle(m_showLimit[i].parti[j], angle-Math::PI/2.0f);

            angle += (2.0f*Math::PI)/m_showLimit[i].total;
        }
    }
}



// Returns a pointer to the last backslash in a filename.

char* SearchLastDir(char *filename)
{
    char*   p = filename;

    while ( *p++ != 0 );
    p --;  // ^on the zero terminator

    while ( p != filename )
    {
        if ( *(--p) == '\\' )  return p;
    }
    return 0;
}


// Compiles all scripts of robots.

void CRobotMain::CompileScript(bool bSoluce)
{
    CObject*    pObj;
    CBrain*     brain;
    int         i, j, nbError, lastError, run;
    char*       name;

    nbError = 0;
    do
    {
        lastError = nbError;
        nbError = 0;
        for ( i=0 ; i<1000000 ; i++ )
        {
            pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
            if ( pObj == 0 )  break;
            if ( pObj->RetTruck() != 0 )  continue;

            brain = pObj->RetBrain();
            if ( brain == 0 )  continue;

            for ( j=0 ; j<10 ; j++ )
            {
                if ( brain->RetCompile(j) )  continue;

                name = brain->RetScriptName(j);
                if ( name[0] != 0 )
                {
                    brain->ReadProgram(j, name);
                    if ( !brain->RetCompile(j) )  nbError++;
                }
            }

            LoadOneScript(pObj, nbError);
        }
    }
    while ( nbError > 0 && nbError != lastError );

    // Load all solutions.
    if ( bSoluce )
    {
        for ( i=0 ; i<1000000 ; i++ )
        {
            pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
            if ( pObj == 0 )  break;
            if ( pObj->RetTruck() != 0 )  continue;

            brain = pObj->RetBrain();
            if ( brain == 0 )  continue;

            name = brain->RetSoluceName();
            if ( name[0] != 0 )
            {
                brain->ReadSoluce(name);  // load solution
            }
        }
    }

    // Start all programs according to the command "run".
    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;
        if ( pObj->RetTruck() != 0 )  continue;

        brain = pObj->RetBrain();
        if ( brain == 0 )  continue;

        run = brain->RetScriptRun();
        if ( run != -1 )
        {
            brain->RunProgram(run);  // starts the program
        }
    }
}

// Load all programs of the robot.

void CRobotMain::LoadOneScript(CObject *pObj, int &nbError)
{
    ObjectType  type;
    CBrain*     brain;
    char        filename[_MAX_FNAME];
    char*       name;
    int         rank, i, objRank;

    brain = pObj->RetBrain();
    if ( brain == 0 )  return;

    if ( !IsSelectable(pObj) )  return;

    type = pObj->RetType();
    if ( type == OBJECT_HUMAN )  return;

    objRank = pObj->RetDefRank();
    if ( objRank == -1 )  return;

    name = m_dialog->RetSceneName();
    rank = m_dialog->RetSceneRank();

    for ( i=0 ; i<BRAINMAXSCRIPT ; i++ )
    {
        if ( brain->RetCompile(i) )  continue;
//?     if ( brain->ProgramExist(i) )  continue;

        sprintf(filename, "%s\\%s\\%c%.3d%.3d%.1d.txt",
                    RetSavegameDir(), m_gamerName, name[0], rank, objRank, i);
        brain->ReadProgram(i, filename);
        if ( !brain->RetCompile(i) )  nbError++;
    }
}

// Load all programs of the robot.

void CRobotMain::LoadFileScript(CObject *pObj, char* filename, int objRank,
                                int &nbError)
{
    ObjectType  type;
    CBrain*     brain;
    char        fn[_MAX_FNAME];
    char*       ldir;
    char*       name;
    int         rank, i;

    if ( objRank == -1 )  return;

    brain = pObj->RetBrain();
    if ( brain == 0 )  return;

    type = pObj->RetType();
    if ( type == OBJECT_HUMAN )  return;

    name = m_dialog->RetSceneName();
    rank = m_dialog->RetSceneRank();

    strcpy(fn, filename);
    ldir = SearchLastDir(fn);
    if ( ldir == 0 )  return;

    for ( i=0 ; i<BRAINMAXSCRIPT ; i++ )
    {
        if ( brain->RetCompile(i) )  continue;
//?     if ( brain->ProgramExist(i) )  continue;

        sprintf(ldir, "\\prog%.3d%.1d.txt", objRank, i);
        brain->ReadProgram(i, fn);
        if ( !brain->RetCompile(i) )  nbError++;
    }
}

// Saves all programs of all the robots.

void CRobotMain::SaveAllScript()
{
    CObject*    pObj;
    int         i;

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        SaveOneScript(pObj);
    }
}

// Saves all programs of the robot.
// If a program does not exist, the corresponding file is destroyed.

void CRobotMain::SaveOneScript(CObject *pObj)
{
    ObjectType  type;
    CBrain*     brain;
    char        filename[_MAX_FNAME];
    char*       name;
    int         rank, i, objRank;

    brain = pObj->RetBrain();
    if ( brain == 0 )  return;

    if ( !IsSelectable(pObj) )  return;

    type = pObj->RetType();
    if ( type == OBJECT_HUMAN )  return;

    objRank = pObj->RetDefRank();
    if ( objRank == -1 )  return;

    name = m_dialog->RetSceneName();
    rank = m_dialog->RetSceneRank();

    for ( i=0 ; i<BRAINMAXSCRIPT ; i++ )
    {
        sprintf(filename, "%s\\%s\\%c%.3d%.3d%.1d.txt",
                    RetSavegameDir(), m_gamerName, name[0], rank, objRank, i);
        brain->WriteProgram(i, filename);
    }
}

// Saves all programs of the robot.
// If a program does not exist, the corresponding file is destroyed.

void CRobotMain::SaveFileScript(CObject *pObj, char* filename, int objRank)
{
    ObjectType  type;
    CBrain*     brain;
    char        fn[_MAX_FNAME];
    char*       ldir;
    char*       name;
    int         rank, i;

    if ( objRank == -1 )  return;

    brain = pObj->RetBrain();
    if ( brain == 0 )  return;

    type = pObj->RetType();
    if ( type == OBJECT_HUMAN )  return;

    name = m_dialog->RetSceneName();
    rank = m_dialog->RetSceneRank();

    strcpy(fn, filename);
    ldir = SearchLastDir(fn);
    if ( ldir == 0 )  return;

    for ( i=0 ; i<BRAINMAXSCRIPT ; i++ )
    {
        sprintf(ldir, "\\prog%.3d%.1d.txt", objRank, i);
        brain->WriteProgram(i, fn);
    }
}

// Saves the stack of the program in execution of a robot.

bool CRobotMain::SaveFileStack(CObject *pObj, FILE *file, int objRank)
{
    ObjectType  type;
    CBrain*     brain;

    if ( objRank == -1 )  return true;

    brain = pObj->RetBrain();
    if ( brain == 0 )  return true;

    type = pObj->RetType();
    if ( type == OBJECT_HUMAN )  return true;

    return brain->WriteStack(file);
}

// Resumes the execution stack of the program in a robot.

bool CRobotMain::ReadFileStack(CObject *pObj, FILE *file, int objRank)
{
    ObjectType  type;
    CBrain*     brain;

    if ( objRank == -1 )  return true;

    brain = pObj->RetBrain();
    if ( brain == 0 )  return true;

    type = pObj->RetType();
    if ( type == OBJECT_HUMAN )  return true;

    return brain->ReadStack(file);
}


// Empty the list.

bool CRobotMain::FlushNewScriptName()
{
    int     i;

    for ( i=0 ; i<MAXNEWSCRIPTNAME ; i++ )
    {
        m_newScriptName[i].bUsed = false;
    }
    return true;
}

// Adds a script name.

bool CRobotMain::AddNewScriptName(ObjectType type, char *name)
{
    int     i;

    for ( i=0 ; i<MAXNEWSCRIPTNAME ; i++ )
    {
        if ( !m_newScriptName[i].bUsed )
        {
            m_newScriptName[i].bUsed = true;
            m_newScriptName[i].type = type;
            strcpy(m_newScriptName[i].name, name);
            return true;
        }
    }
    return false;
}

// Seeks a script name for a given type.

char*  CRobotMain::RetNewScriptName(ObjectType type, int rank)
{
    int     i;

    for ( i=0 ; i<MAXNEWSCRIPTNAME ; i++ )
    {
        if ( m_newScriptName[i].bUsed &&
             (m_newScriptName[i].type == type        ||
              m_newScriptName[i].type == OBJECT_NULL ) )
        {
            if ( rank == 0 )  return m_newScriptName[i].name;
            else              rank --;
        }
    }

    return 0;
}


// Seeks if an object occupies in a spot, to prevent a backup of the game.

bool CRobotMain::IsBusy()
{
    CObject*    pObj;
    CBrain*     pBrain;
//? CAuto*      pAuto;
    int         i;

    if ( m_CompteurFileOpen > 0 )  return true;

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        pBrain = pObj->RetBrain();
        if ( pBrain != 0 )
        {
            if ( pBrain->IsBusy() )  return true;
        }

//?     pAuto = pObj->RetAuto();
//?     if ( pAuto != 0 )
//?     {
//?         if ( pAuto->RetBusy() )  return true;
//?     }
    }
    return false;
}

// Writes an object into the backup file.

void CRobotMain::IOWriteObject(FILE *file, CObject* pObj, char *cmd)
{
    Math::Vector    pos;
    CBrain*     pBrain;
    char        line[3000];
    char        name[100];
    int         run, i;

    if ( pObj->RetType() == OBJECT_FIX  )  return;

    strcpy(line, cmd);

    sprintf(name, " type=%s", GetTypeObject(pObj->RetType()));
    strcat(line, name);

    sprintf(name, " id=%d", pObj->RetID());
    strcat(line, name);

    pos = pObj->RetPosition(0)/g_unit;
    sprintf(name, " pos=%.2f;%.2f;%.2f", pos.x, pos.y, pos.z);
    strcat(line, name);

    pos = pObj->RetAngle(0)/(Math::PI/180.0f);
    sprintf(name, " angle=%.2f;%.2f;%.2f", pos.x, pos.y, pos.z);
    strcat(line, name);

    pos = pObj->RetZoom(0);
    sprintf(name, " zoom=%.2f;%.2f;%.2f", pos.x, pos.y, pos.z);
    strcat(line, name);

    for ( i=1 ; i<OBJECTMAXPART ; i++ )
    {
        if ( pObj->RetObjectRank(i) == -1 )  continue;

        pos = pObj->RetPosition(i);
        if ( pos.x != 0.0f || pos.y != 0.0f || pos.z != 0.0f )
        {
            pos /= g_unit;
            sprintf(name, " p%d=%.2f;%.2f;%.2f", i, pos.x, pos.y, pos.z);
            strcat(line, name);
        }

        pos = pObj->RetAngle(i);
        if ( pos.x != 0.0f || pos.y != 0.0f || pos.z != 0.0f )
        {
            pos /= (Math::PI/180.0f);
            sprintf(name, " a%d=%.2f;%.2f;%.2f", i, pos.x, pos.y, pos.z);
            strcat(line, name);
        }

        pos = pObj->RetZoom(i);
        if ( pos.x != 1.0f || pos.y != 1.0f || pos.z != 1.0f )
        {
            sprintf(name, " z%d=%.2f;%.2f;%.2f", i, pos.x, pos.y, pos.z);
            strcat(line, name);
        }
    }

    sprintf(name, " trainer=%d", pObj->RetTrainer());
    strcat(line, name);

    sprintf(name, " option=%d", pObj->RetOption());
    strcat(line, name);

    if ( pObj == m_infoObject )  // selects object?
    {
        sprintf(name, " select=1");
        strcat(line, name);
    }

    pObj->Write(line);

    if ( pObj->RetType() == OBJECT_BASE )
    {
        sprintf(name, " run=3");  // stops and open (PARAM_FIXSCENE)
        strcat(line, name);
    }

    pBrain = pObj->RetBrain();
    if ( pBrain != 0 )
    {
        run = pBrain->RetProgram();
        if ( run != -1 )
        {
            sprintf(name, " run=%d", run+1);
            strcat(line, name);
        }
    }

    strcat(line, "\n");
    fputs(line, file);
}

// Saves the current game.

bool CRobotMain::IOWriteScene(char *filename, char *filecbot, char *info)
{
    FILE*       file;
    char        line[500];
    char*       name;
    CObject     *pObj, *pPower, *pFret;
    float       sleep, delay, magnetic, progress;
    int         i, objRank;
    long        version;

    file = fopen(filename, "w");
    if ( file == NULL )  return false;

    sprintf(line, "Title text=\"%s\"\n", info);
    fputs(line, file);

    sprintf(line, "Version maj=%d min=%d\n", 0, 1);
    fputs(line, file);

    name = m_dialog->RetSceneName();
    if ( strcmp(name, "user") == 0 )
    {
        sprintf(line, "Mission base=\"%s\" rank=%.3d dir=\"%s\"\n", name, m_dialog->RetSceneRank(), m_dialog->RetSceneDir());
    }
    else
    {
        sprintf(line, "Mission base=\"%s\" rank=%.3d\n", name, m_dialog->RetSceneRank());
    }
    fputs(line, file);

    sprintf(line, "Map zoom=%.2f\n", m_map->RetZoomMap());
    fputs(line, file);

    sprintf(line, "DoneResearch bits=%d\n", g_researchDone);
    fputs(line, file);

    if ( m_blitz->GetStatus(sleep, delay, magnetic, progress) )
    {
        sprintf(line, "BlitzMode sleep=%.2f delay=%.2f magnetic=%.2f progress=%.2f\n", sleep, delay, magnetic/g_unit, progress);
        fputs(line, file);
    }

    objRank = 0;
    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        if ( pObj->RetType() == OBJECT_TOTO )  continue;
        if ( pObj->RetType() == OBJECT_FIX  )  continue;
        if ( pObj->RetTruck() != 0 )  continue;
        if ( pObj->RetBurn() )  continue;
        if ( pObj->RetDead() )  continue;
        if ( pObj->RetExplo() )  continue;

        pPower = pObj->RetPower();
        pFret  = pObj->RetFret();

        if ( pFret != 0 )  // object transported?
        {
            IOWriteObject(file, pFret, "CreateFret");
        }

        if ( pPower != 0 )  // battery transported?
        {
            IOWriteObject(file, pPower, "CreatePower");
        }

        IOWriteObject(file, pObj, "CreateObject");

        SaveFileScript(pObj, filename, objRank++);
    }
    fclose(file);

#if CBOT_STACK
    // Writes the file of stacks of execution.
    file = fOpen(filecbot, "wb");
    if ( file == NULL )  return false;

    version = 1;
    fWrite(&version, sizeof(long), 1, file);  // version of COLOBOT
    version = CBotProgram::GetVersion();
    fWrite(&version, sizeof(long), 1, file);  // version of CBOT

    objRank = 0;
    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        if ( pObj->RetType() == OBJECT_TOTO )  continue;
        if ( pObj->RetType() == OBJECT_FIX  )  continue;
        if ( pObj->RetTruck() != 0 )  continue;
        if ( pObj->RetBurn() )  continue;
        if ( pObj->RetDead() )  continue;

        if ( !SaveFileStack(pObj, file, objRank++) )  break;
    }
    CBotClass::SaveStaticState(file);
    fClose(file);
#endif

    m_delayWriteMessage = 4;  // displays message in 3 frames
    return true;
}

// Resumes the game.

CObject* CRobotMain::IOReadObject(char *line, char* filename, int objRank)
{
    CObject*    pObj;
//? CBrain*     pBrain;
    CAuto*      pAuto;
    Math::Vector    pos, dir, zoom;
    ObjectType  type;
    int         id, run, trainer, toy, option, i;
    char        op[10];

    pos  = OpDir(line, "pos")*g_unit;
    dir  = OpDir(line, "angle")*(Math::PI/180.0f);
    zoom = OpDir(line, "zoom");
    type = OpTypeObject(line, "type", OBJECT_NULL);
    id = OpInt(line, "id", 0);
    if ( type == OBJECT_NULL )  return 0;
    trainer = OpInt(line, "trainer", 0);
    toy = OpInt(line, "toy", 0);
    option = OpInt(line, "option", 0);
    pObj = CreateObject(pos, dir.y, 1.0f, 0.0f, type, 0.0f, trainer, toy, option);
    pObj->SetDefRank(objRank);
    pObj->SetPosition(0, pos);
    pObj->SetAngle(0, dir);
    pObj->SetID(id);
    if ( g_id < id )  g_id = id;

    if ( zoom.x != 0.0f || zoom.y != 0.0f || zoom.z != 0.0f )
    {
        pObj->SetZoom(0, zoom);
    }

    for ( i=1 ; i<OBJECTMAXPART ; i++ )
    {
        if ( pObj->RetObjectRank(i) == -1 )  continue;

        sprintf(op, "p%d", i);
        pos  = OpDir(line, op);
        if ( pos.x != 0.0f || pos.y != 0.0f || pos.z != 0.0f )
        {
            pObj->SetPosition(i, pos*g_unit);
        }

        sprintf(op, "a%d", i);
        dir  = OpDir(line, op);
        if ( dir.x != 0.0f || dir.y != 0.0f || dir.z != 0.0f )
        {
            pObj->SetAngle(i, dir*(Math::PI/180.0f));
        }

        sprintf(op, "z%d", i);
        zoom = OpDir(line, op);
        if ( zoom.x != 0.0f || zoom.y != 0.0f || zoom.z != 0.0f )
        {
            pObj->SetZoom(i, zoom);
        }
    }

    if ( type == OBJECT_BASE )  m_bBase = true;

    pObj->Read(line);

#if CBOT_STACK
#else
    LoadFileScript(pObj, filename, objRank, i);
#endif

    run = OpInt(line, "run", -1);
    if ( run != -1 )
    {
#if CBOT_STACK
#else
        pBrain = pObj->RetBrain();
        if ( pBrain != 0 )
        {
            pBrain->RunProgram(run-1);  // starts the program
        }
#endif

        pAuto = pObj->RetAuto();
        if ( pAuto != 0 )
        {
            pAuto->Start(run);  // starts the film
        }
    }

    return pObj;
}

// Resumes some part of the game.

CObject* CRobotMain::IOReadScene(char *filename, char *filecbot)
{
    FILE*       file;
    CObject     *pObj, *pPower, *pFret, *pSel;
    char        line[3000];
    float       sleep, delay, progress, magnetic;
    int         i, objRank, nbError, lastError;
    long        version;

    m_bBase = false;

    file = fopen(filename, "r");
    if ( file == NULL )  return 0;

    pFret   = 0;
    pPower  = 0;
    pSel    = 0;
    objRank = 0;
    while ( fgets(line, 3000, file) != NULL )
    {
        for ( i=0 ; i<3000 ; i++ )
        {
            if ( line[i] == '\t' )  line[i] = ' ';  // replace tab by space
            if ( line[i] == '/' && line[i+1] == '/' )
            {
                line[i] = 0;
                break;
            }
        }

        if ( Cmd(line, "Map") )
        {
            m_map->ZoomMap(OpFloat(line, "zoom", 1.0f));
        }

        if ( Cmd(line, "DoneResearch") )
        {
            g_researchDone = OpInt(line, "bits", 0);
        }

        if ( Cmd(line, "BlitzMode") )
        {
            sleep = OpFloat(line, "sleep", 0.0f);
            delay = OpFloat(line, "delay", 3.0f);
            magnetic = OpFloat(line, "magnetic", 50.0f)*g_unit;
            progress = OpFloat(line, "progress", 0.0f);
            m_blitz->SetStatus(sleep, delay, magnetic, progress);
        }

        if ( Cmd(line, "CreateFret") )
        {
            pFret = IOReadObject(line, filename, -1);
        }

        if ( Cmd(line, "CreatePower") )
        {
            pPower = IOReadObject(line, filename, -1);
        }

        if ( Cmd(line, "CreateObject") )
        {
            pObj = IOReadObject(line, filename, objRank++);

            if ( OpInt(line, "select", 0) )
            {
                pSel = pObj;
            }

            if ( pFret != 0 )
            {
                CTaskManip* task;

                pObj->SetFret(pFret);
                task = new CTaskManip(m_iMan, pObj);
                task->Start(TMO_AUTO, TMA_GRAB);  // holds the object!
                delete task;
            }

            if ( pPower != 0 )
            {
                pObj->SetPower(pPower);
                pPower->SetTruck(pObj);
            }

            pFret  = 0;
            pPower = 0;
        }
    }
    fclose(file);

#if CBOT_STACK
    // Compiles scripts.
    nbError = 0;
    do
    {
        lastError = nbError;
        nbError = 0;
        for ( i=0 ; i<1000000 ; i++ )
        {
            pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
            if ( pObj == 0 )  break;
            if ( pObj->RetTruck() != 0 )  continue;

            objRank = pObj->RetDefRank();
            if ( objRank == -1 )  continue;

            LoadFileScript(pObj, filename, objRank, nbError);
        }
    }
    while ( nbError > 0 && nbError != lastError );

    // Reads the file of stacks of execution.
    file = fOpen(filecbot, "rb");
    if ( file != NULL )
    {
        fRead(&version, sizeof(long), 1, file);  // version of COLOBOT
        if ( version == 1 )
        {
            fRead(&version, sizeof(long), 1, file);  // version of CBOT
            if ( version == CBotProgram::GetVersion() )
            {
                objRank = 0;
                for ( i=0 ; i<1000000 ; i++ )
                {
                    pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
                    if ( pObj == 0 )  break;

                    if ( pObj->RetType() == OBJECT_TOTO )  continue;
                    if ( pObj->RetType() == OBJECT_FIX  )  continue;
                    if ( pObj->RetTruck() != 0 )  continue;
                    if ( pObj->RetBurn() )  continue;
                    if ( pObj->RetDead() )  continue;

                    if ( !ReadFileStack(pObj, file, objRank++) )  break;
                }
            }
        }
        CBotClass::RestoreStaticState(file);
        fClose(file);
    }
#endif

    return pSel;
}


// Writes the global parameters for free play.

void CRobotMain::WriteFreeParam()
{
    FILE*   file;
    char    filename[_MAX_FNAME];
    char    line[100];

    m_freeResearch |= g_researchDone;
    m_freeBuild    |= g_build;

    if ( m_gamerName[0] == 0 )  return;

    sprintf(filename, "%s\\%s\\research.gam", RetSavegameDir(), m_gamerName);
    file = fopen(filename, "w");
    if ( file == NULL )  return;

    sprintf(line, "research=%d build=%d\n", m_freeResearch, m_freeBuild);
    fputs(line, file);
    fclose(file);
}

// Reads the global parameters for free play.

void CRobotMain::ReadFreeParam()
{
    FILE*   file;
    char    filename[_MAX_FNAME];
    char    line[100];

    m_freeResearch = 0;
    m_freeBuild    = 0;

    if ( m_gamerName[0] == 0 )  return;

    sprintf(filename, "%s\\%s\\research.gam", RetSavegameDir(), m_gamerName);
    file = fopen(filename, "r");
    if ( file == NULL )  return;

    if ( fgets(line, 100, file) != NULL )
    {
        sscanf(line, "research=%d build=%d\n", &m_freeResearch, &m_freeBuild);
    }

    fclose(file);
}


// Resets all objects to their original position.

void CRobotMain::ResetObject()
{
#if 0
    CObject*    pObj;
    CObject*    pTruck;
    CAuto*      pAuto;
    CBrain*     brain;
    CPyro*      pyro;
    ResetCap    cap;
    Math::Vector    pos, angle;
    int         i;

    // Removes all pyrotechnic effects in progress.
    while ( true )
    {
        pyro = (CPyro*)m_iMan->SearchInstance(CLASS_PYRO, 0);
        if ( pyro == 0 )  break;

        pyro->DeleteObject();
        delete pyro;
    }

    // Removes all bullets in progress.
    m_particule->DeleteParticule(PARTIGUN1);
    m_particule->DeleteParticule(PARTIGUN2);
    m_particule->DeleteParticule(PARTIGUN3);
    m_particule->DeleteParticule(PARTIGUN4);

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        cap = pObj->RetResetCap();
        if ( cap == RESET_NONE )  continue;

        if ( cap == RESET_DELETE )
        {
            pTruck = pObj->RetTruck();
            if ( pTruck != 0 )
            {
                pTruck->SetFret(0);
                pObj->SetTruck(0);
            }
            pObj->DeleteObject();
            delete pObj;
            i --;
            continue;
        }

        pAuto = pObj->RetAuto();
        if ( pAuto != 0 )
        {
            pAuto->Abort();
        }

        if ( pObj->RetEnable() )  // object still active?
        {
            brain = pObj->RetBrain();
            if ( brain != 0 )
            {
                pos   = pObj->RetResetPosition();
                angle = pObj->RetResetAngle();

                if ( pos   == pObj->RetPosition(0) &&
                     angle == pObj->RetAngle(0)    )  continue;
                brain->StartTaskReset(pos, angle);
                continue;
            }
        }

        pObj->SetEnable(true);  // active again

        pos   = pObj->RetResetPosition();
        angle = pObj->RetResetAngle();

        if ( pos   == pObj->RetPosition(0) &&
             angle == pObj->RetAngle(0)    )  continue;

        pyro = new CPyro(m_iMan);
        pyro->Create(PT_RESET, pObj);

        brain = pObj->RetBrain();
        if ( brain != 0 )
        {
            brain->RunProgram(pObj->RetResetRun());
        }
    }
#else
    m_bResetCreate = true;
#endif
}

// Resets all objects to their original position.

void CRobotMain::ResetCreate()
{
    CObject*    pObj;
    CPyro*      pyro;
    ResetCap    cap;
    int         i;

    SaveAllScript();

    // Removes all bullets in progress.
    m_particule->DeleteParticule(PARTIGUN1);
    m_particule->DeleteParticule(PARTIGUN2);
    m_particule->DeleteParticule(PARTIGUN3);
    m_particule->DeleteParticule(PARTIGUN4);

    DeselectAll();  // removes the control buttons
    DeleteAllObjects();  // removes all the current 3D Scene

    m_particule->FlushParticule();
    m_terrain->FlushBuildingLevel();
    m_iMan->Flush(CLASS_OBJECT);
    m_iMan->Flush(CLASS_PHYSICS);
    m_iMan->Flush(CLASS_BRAIN);
    m_iMan->Flush(CLASS_PYRO);
    m_camera->SetType(CAMERA_DIALOG);

    CreateScene(m_dialog->RetSceneSoluce(), false, true);

    if ( !RetNiceReset() )  return;

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        cap = pObj->RetResetCap();
        if ( cap == RESET_NONE )  continue;

        pyro = new CPyro(m_iMan);
        pyro->Create(PT_RESET, pObj);
    }
}

// Checks if the mission is over.

Error CRobotMain::CheckEndMission(bool bFrame)
{
    CObject*    pObj;
    Math::Vector    bPos, oPos;
    ObjectType  type;
    int         t, i, nb;

    for ( t=0 ; t<m_endTakeTotal ; t++ )
    {
        if ( m_endTake[t].message[0] != 0 )  continue;

        bPos = m_endTake[t].pos;
        bPos.y = 0.0f;

        nb = 0;
        for ( i=0 ; i<1000000 ; i++ )
        {
            pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
            if ( pObj == 0 )  break;

            // Do not use RetActif () because an invisible worm (underground)
            // should be regarded as existing here!
            if ( pObj->RetLock() )  continue;
            if ( pObj->RetRuin() )  continue;
            if ( !pObj->RetEnable() )  continue;

            type = pObj->RetType();
            if ( type == OBJECT_SCRAP2 ||
                 type == OBJECT_SCRAP3 ||
                 type == OBJECT_SCRAP4 ||
                 type == OBJECT_SCRAP5 )  // wastes?
            {
                type = OBJECT_SCRAP1;
            }
            if ( type != m_endTake[t].type )  continue;

            if ( pObj->RetTruck() == 0 )
            {
                oPos = pObj->RetPosition(0);
            }
            else
            {
                oPos = pObj->RetTruck()->RetPosition(0);
            }
            oPos.y = 0.0f;
            if ( Math::DistanceProjected(oPos, bPos) <= m_endTake[t].dist )
            {
                nb ++;
            }
        }

        if ( nb <= m_endTake[t].lost )
        {
            if ( m_endTake[t].type == OBJECT_HUMAN )
            {
                if ( m_lostDelay == 0.0f )
                {
                    m_lostDelay = 0.1f;  // lost immediately
                    m_winDelay  = 0.0f;
                }
                m_displayText->SetEnable(false);
                return INFO_LOSTq;
            }
            else
            {
                if ( m_lostDelay == 0.0f )
                {
                    m_displayText->DisplayError(INFO_LOST, Math::Vector(0.0f,0.0f,0.0f));
                    m_lostDelay = m_endTakeLostDelay;  // lost in 6 seconds
                    m_winDelay  = 0.0f;
                }
                m_displayText->SetEnable(false);
                return INFO_LOST;
            }
        }
        if ( nb < m_endTake[t].min ||
             nb > m_endTake[t].max )
        {
            m_displayText->SetEnable(true);
            return ERR_MISSION_NOTERM;
        }
        if ( m_endTake[t].bImmediat )
        {
            if ( m_winDelay == 0.0f )
            {
                m_winDelay  = m_endTakeWinDelay;  // wins in x seconds
                m_lostDelay = 0.0f;
            }
            m_displayText->SetEnable(false);
            return ERR_OK;  // mission ended
        }
    }

    if ( m_endTakeResearch != 0 )
    {
        if ( m_endTakeResearch != (m_endTakeResearch&g_researchDone) )
        {
            m_displayText->SetEnable(true);
            return ERR_MISSION_NOTERM;
        }
    }

    if ( m_endTakeWinDelay == -1.0f )
    {
        m_winDelay  = 1.0f;  // wins in one second
        m_lostDelay = 0.0f;
        m_displayText->SetEnable(false);
        return ERR_OK;  // mission ended
    }

    if ( bFrame && m_bBase )  return ERR_MISSION_NOTERM;

    if ( m_winDelay == 0.0f )
    {
        m_displayText->DisplayError(INFO_WIN, Math::Vector(0.0f,0.0f,0.0f));
        m_winDelay  = m_endTakeWinDelay;  // wins in two seconds
        m_lostDelay = 0.0f;
    }
    m_displayText->SetEnable(false);
    return ERR_OK;  // mission ended
}

// Checks if the mission is finished after displaying a message.

void CRobotMain::CheckEndMessage(char *message)
{
    int     t;

    for ( t=0 ; t<m_endTakeTotal ; t++ )
    {
        if ( m_endTake[t].message[0] == 0 )  continue;

        if ( strcmp(m_endTake[t].message, message) == 0 )
        {
            m_displayText->DisplayError(INFO_WIN, Math::Vector(0.0f,0.0f,0.0f));
            m_winDelay  = m_endTakeWinDelay;  // wins in 2 seconds
            m_lostDelay = 0.0f;
        }
    }
}


// Returns the number of instructions required.

int CRobotMain::RetObligatoryToken()
{
    return m_obligatoryTotal;
}

// Returns the name of a required instruction.

char* CRobotMain::RetObligatoryToken(int i)
{
    return m_obligatoryToken[i];
}

// Checks if an instruction is part of the obligatory list.

int CRobotMain::IsObligatoryToken(char *token)
{
    int     i;

    for ( i=0 ; i<m_obligatoryTotal ; i++ )
    {
        if ( strcmp(token, m_obligatoryToken[i]) == 0 )
        {
            return i;
        }
    }
    return -1;
}

// Checks if an instruction is not part of the banned list.

bool CRobotMain::IsProhibitedToken(char *token)
{
    int     i;

    for ( i=0 ; i<m_prohibitedTotal ; i++ )
    {
        if ( strcmp(token, m_prohibitedToken[i]) == 0 )
        {
            return false;
        }
    }
    return true;
}


// Indicates whether it is possible to control a driving robot.

bool CRobotMain::RetTrainerPilot()
{
    return m_bTrainerPilot;
}

// Indicates whether the scene is fixed, without interaction.

bool CRobotMain::RetFixScene()
{
    return m_bFixScene;
}


char* CRobotMain::RetTitle()
{
    return m_title;
}

char* CRobotMain::RetResume()
{
    return m_resume;
}

char* CRobotMain::RetScriptName()
{
    return m_scriptName;
}

char* CRobotMain::RetScriptFile()
{
    return m_scriptFile;
}


bool CRobotMain::RetGlint()
{
    return m_dialog->RetGlint();
}

bool CRobotMain::RetSoluce4()
{
    return m_dialog->RetSoluce4();
}

bool CRobotMain::RetMovies()
{
    return m_dialog->RetMovies();
}

bool CRobotMain::RetNiceReset()
{
    return m_dialog->RetNiceReset();
}

bool CRobotMain::RetHimselfDamage()
{
    return m_dialog->RetHimselfDamage();
}

bool CRobotMain::RetShowSoluce()
{
    return m_bShowSoluce;
}

bool CRobotMain::RetSceneSoluce()
{
    if ( m_infoFilename[SATCOM_SOLUCE][0] == 0 )  return false;
    return m_dialog->RetSceneSoluce();
}

bool CRobotMain::RetShowAll()
{
    return m_bShowAll;
}

bool CRobotMain::RetCheatRadar()
{
    return m_bCheatRadar;
}

char* CRobotMain::RetSavegameDir()
{
    return m_dialog->RetSavegameDir();
}

char* CRobotMain::RetPublicDir()
{
    return m_dialog->RetPublicDir();
}

char* CRobotMain::RetFilesDir()
{
    return m_dialog->RetFilesDir();
}


// Change the player's name.

void CRobotMain::SetGamerName(char *name)
{
    strcpy(m_gamerName, name);
    SetGlobalGamerName(m_gamerName);
    ReadFreeParam();
}

// Getes the player's name.

char* CRobotMain::RetGamerName()
{
    return m_gamerName;
}


// Returns the representation to use for the player.

int CRobotMain::RetGamerFace()
{
    return m_dialog->RetGamerFace();
}

// Returns the representation to use for the player.

int CRobotMain::RetGamerGlasses()
{
    return m_dialog->RetGamerGlasses();
}

// Returns the mode with just the head.

bool CRobotMain::RetGamerOnlyHead()
{
    return m_dialog->RetGamerOnlyHead();
}

// Returns the angle of presentation.

float CRobotMain::RetPersoAngle()
{
    return m_dialog->RetPersoAngle();
}


// Changes on the pause mode.

void CRobotMain::ChangePause(bool bPause)
{
    m_bPause = bPause;
    m_engine->SetPause(m_bPause);

    m_sound->MuteAll(m_bPause);
    CreateShortcuts();
    if ( m_bPause )  HiliteClear();
}


// Changes game speed

void CRobotMain::SetSpeed(float speed)
{
    CButton*    pb;
    char        text[10];

    m_engine->SetSpeed(speed);

    pb = (CButton*)m_interface->SearchControl(EVENT_SPEED);
    if ( pb != 0 )
    {
        if ( speed == 1.0f )
        {
            pb->ClearState(STATE_VISIBLE);
        }
        else
        {
            sprintf(text, "x%.1f", speed);
            pb->SetName(text);
            pb->SetState(STATE_VISIBLE);
        }
    }
}

float CRobotMain::RetSpeed()
{
    return m_engine->RetSpeed();
}


// Creates interface shortcuts to the units.

bool CRobotMain::CreateShortcuts()
{
    if ( m_phase != PHASE_SIMUL )  return false;
    if ( !m_bShortCut )  return false;
    return m_short->CreateShortcuts();
}

// Updates the map.

void CRobotMain::UpdateMap()
{
    m_map->UpdateMap();
}

// Indicates whether the mini-map is visible.

bool CRobotMain::RetShowMap()
{
    return m_map->RetShowMap() && m_bMapShow;
}


// Management of the lock mode for movies.

void CRobotMain::SetMovieLock(bool bLock)
{
    m_bMovieLock = bLock;
    m_engine->SetMovieLock(m_bMovieLock);

    CreateShortcuts();
    m_map->ShowMap(!m_bMovieLock && m_bMapShow);
    if ( m_bMovieLock )  HiliteClear();
    m_engine->SetMouseHide(m_bMovieLock);
}

bool CRobotMain::RetMovieLock()
{
    return m_bMovieLock;
}

bool CRobotMain::RetInfoLock()
{
    return ( m_displayInfo != 0 );  // info in progress?
}

// Management of the blocking of the call of SatCom.

void CRobotMain::SetSatComLock(bool bLock)
{
    m_bSatComLock = bLock;
}

bool CRobotMain::RetSatComLock()
{
    return m_bSatComLock;
}

// Management of the lock mode for the edition.

void CRobotMain::SetEditLock(bool bLock, bool bEdit)
{
    m_bEditLock = bLock;

    CreateShortcuts();

    // Do not remove the card if it contains a still image.
    if ( !bLock || !m_map->RetFixImage() )
    {
        m_map->ShowMap(!m_bEditLock && m_bMapShow);
    }

    m_displayText->HideText(bLock);
    m_engine->FlushPressKey();

    if ( m_bEditLock )
    {
        HiliteClear();
    }
    else
    {
        m_bEditFull = false;
    }
}

bool CRobotMain::RetEditLock()
{
    return m_bEditLock;
}

// Management of the fullscreen mode during editing.

void CRobotMain::SetEditFull(bool bFull)
{
    m_bEditFull = bFull;
}

bool CRobotMain::RetEditFull()
{
    return m_bEditFull;
}


bool CRobotMain::RetFreePhoto()
{
    return m_bFreePhoto;
}


// Indicates whether mouse is on an friend object, on which we should not shoot.

void CRobotMain::SetFriendAim(bool bFriend)
{
    m_bFriendAim = bFriend;
}

bool CRobotMain::RetFriendAim()
{
    return m_bFriendAim;
}


// Management of the precision of drawing the ground.

void CRobotMain::SetTracePrecision(float factor)
{
    m_engine->SetTracePrecision(factor);
}

float CRobotMain::RetTracePrecision()
{
    return m_engine->RetTracePrecision();
}


// Starts music with a mission.

void CRobotMain::StartMusic()
{
    if ( m_audioTrack != 0 )
    {
        m_sound->StopMusic();
        m_sound->PlayMusic(m_audioTrack, m_bAudioRepeat);
    }
}

// Removes hilite and tooltip.

void CRobotMain::ClearInterface()
{
    HiliteClear();  // removes setting evidence
    m_tooltipName[0] = 0;  // really removes the tooltip
}


