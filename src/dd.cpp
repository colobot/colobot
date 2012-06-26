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

// Compilation of a procedure with a "dot".

int cPoint(CBotVar* &var, CBotString& retClass, void* user)
{
    if ( var == 0 )  return CBotErrLowParam;

    if ( var->GivType() <= CBotTypDouble )
    {
        var = var->GivNext();
        if ( var == 0 )  return CBotErrLowParam;
        if ( var->GivType() > CBotTypDouble )  return CBotErrBadNum;
        var = var->GivNext();
        if ( var == 0 )  return CBotErrLowParam;
        if ( var->GivType() > CBotTypDouble )  return CBotErrBadNum;
        var = var->GivNext();
        return 0;
    }

    if ( var->GivType() == CBotTypClass )
    {
        if ( !var->IsElemOfClass("point") )  return CBotErrBadParam;
        var = var->GivNext();
        return 0;
    }

    return CBotErrBadParam;
}

// Gives a parameter of type "point".

BOOL GetPoint(CBotVar* &var, int& exception, D3DVECTOR& pos)
{
    CBotVar     *pX, *pY, *pZ;

    if ( var->GivType() <= CBotTypDouble )
    {
        pos.x = var->GivValFloat()*UNIT;
        var = var->GivNext();

        pos.z = var->GivValFloat()*UNIT;
        var = var->GivNext();

        pos.y = var->GivValFloat()*UNIT;
        var = var->GivNext();
    }
    else
    {
        pX = var->GivItem("x");
        if ( pX == NULL )
        {
            exception = CBotErrUndefItem;  return TRUE;
        }
        pos.x = pX->GivValFloat()*UNIT;

        pY = var->GivItem("y");
        if ( pY == NULL )
        {
            exception = CBotErrUndefItem;  return TRUE;
        }
        pos.z = pY->GivValFloat()*UNIT;  // attention y -> z !

        pZ = var->GivItem("z");
        if ( pZ == NULL )
        {
            exception = CBotErrUndefItem;  return TRUE;
        }
        pos.y = pZ->GivValFloat()*UNIT;  // attention z -> y !

        var = var->GivNext();
    }
    return TRUE;
}



// Compilation of the instruction "space(center, rMin, rMax, dist)".

int cSpace(CBotVar* &var, CBotString& retClass, void* user)
{
    int     ret;

    retClass = "point";

    if ( var == 0 )  return CBotTypIntrinsic;
    ret = cPoint(var, retClass, user);
    if ( ret != 0 )  return ret;

    if ( var == 0 )  return CBotTypIntrinsic;
    if ( var->GivType() > CBotTypDouble )  return CBotErrBadNum;
    var = var->GivNext();

    if ( var == 0 )  return CBotTypIntrinsic;
    if ( var->GivType() > CBotTypDouble )  return CBotErrBadNum;
    var = var->GivNext();

    if ( var == 0 )  return CBotTypIntrinsic;
    if ( var->GivType() > CBotTypDouble )  return CBotErrBadNum;
    var = var->GivNext();

    if ( var != 0 )  return CBotErrOverParam;
    return CBotTypIntrinsic;
}

// Instruction "space(center, rMin, rMax, dist)".

BOOL rSpace(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = ((CObject*)user)->RetRunScript();
    CObject*    pThis = (CObject*)user;
    CBotVar*    pSub;
    D3DVECTOR   center;
    float       rMin, rMax, dist;

    rMin =  5.0f*UNIT;
    rMax = 50.0f*UNIT;
    dist =  4.0f*UNIT;

    if ( var == 0 )
    {
        center = pThis->RetPosition(0);
    }
    else
    {
        if ( !GetPoint(var, exception, center) )  return TRUE;

        if ( var != 0 )
        {
            rMin = var->GivValFloat()*UNIT;
            var = var->GivNext();

            if ( var != 0 )
            {
                rMax = var->GivValFloat()*UNIT;
                var = var->GivNext();

                if ( var != 0 )
                {
                    dist = var->GivValFloat()*UNIT;
                    var = var->GivNext();
                }
            }
        }
    }
    script->m_main->FreeSpace(center, rMin, rMax, dist, pThis);

    if ( result != 0 )
    {
        pSub = result->GivItemList();
        if ( pSub != 0 )
        {
            pSub->SetValFloat(center.x/UNIT);
            pSub = pSub->GivNext();  // "y"
            pSub->SetValFloat(center.z/UNIT);
            pSub = pSub->GivNext();  // "z"
            pSub->SetValFloat(center.y/UNIT);
        }
    }
    return TRUE;
}
