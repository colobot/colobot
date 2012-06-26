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

//-----------------------------------------------------------------------------
// File: D3DRes.h
//
// Desc: Resource definitions required by the CD3DApplication class.
//       Any application using the CD3DApplication class must include resources
//       with the following identifiers.
//
// Copyright (c) 1999 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef D3DRES_H
#define D3DRES_H


#define IDI_MAIN_ICON          101 // Application icon
#define IDR_MAIN_ACCEL         113 // Keyboard accelerator
#define IDR_MENU               141 // Application menu
#define IDR_POPUP              142 // Popup menu
#define IDD_ABOUT              143 // About dialog box
#define IDD_CHANGEDEVICE       144 // "Change Device" dialog box
#define IDC_CURSORHAND         149
#define IDC_CURSORSCROLLL      150
#define IDC_CURSORSCROLLR      151
#define IDC_CURSORSCROLLU      152
#define IDC_CURSORSCROLLD      153
#define IDC_CURSORTARGET       154

#define IDC_DEVICE_COMBO      1000 // Device combobox for "Change Device" dlg
#define IDC_MODE_COMBO        1001 // Mode combobox for "Change Device" dlg
#define IDC_WINDOWED_CHECKBOX 1012 // Checkbox for windowed-mode
#define IDC_STEREO_CHECKBOX   1013 // Checkbox for stereo modes
#define IDC_FULLSCREEN_TEXT   1014 // Group box text label

#define IDM_ABOUT            40001 // Command to invoke About dlg
#define IDM_CHANGEDEVICE     40002 // Command to invoke "Change Device" dlg
#define IDM_TOGGLEFULLSCREEN 40003 // Command to toggle fullscreen mode
#define IDM_TOGGLESTART      40004 // Command to toggle frame animation
#define IDM_SINGLESTEP       40005 // Command to single step frame animation
#define IDM_EXIT             40006 // Command to exit the application




#endif // D3DRES_H
