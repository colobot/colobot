/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2020, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsitec.ch; http://colobot.info; http://github.com/colobot
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://gnu.org/licenses
 */

#pragma once

#include "app/modman.h"

#include "ui/maindialog.h"

#include "ui/screen/screen.h"

#include <map>

namespace Ui
{

/**
 * \class CScreenModList
 * \brief This class is the front-end for the \ref CModManager.
 *
 * \section Assumptions Assumptions
 *
 * It assumes the user is changing something in the mods folders while the screen is visible,
 * e.g. removing them or adding more. For this reason, the mods are always reloaded after the user
 * lefts the screen, possibly after asking the user if their changes should be saved. They are also reloaded
 * when the user opens the screen to avoid weird situations like "the mod is no longer there but in theory
 * it's still in the game's memory even if it's not visible on the list".
 *
 * Unsafe changes, like removing a mod which is still enabled, are a sole responsibility of the user and
 * we basically can't guarantee the game will behave properly in such cases even when they happen while
 * this screen is visible.
 *
 * \section Features UI Features
 *
 * The user can reorder mods with appropriate buttons and enable/disable them. To avoid reloading
 * the resources after every change, the changes are not immediate. The resources are reloaded in the
 * cases described above and also after using the Apply or Refresh buttons. The only technical
 * difference between them is that the Refresh button is always enabled, but Apply is only enabled
 * if the user made any changes in the list by using the UI. The reason is, again, to avoid dealing with
 * weird situations like described above.
 *
 * The UI also shows the selected mod metadata like description, version, etc.
 *
 * There is also a button which will try to open the default web browser with the Workshop website,
 * where the user can search for new mods.
 *
 * For convenience, also a button opening a saves/mods folder is provided.
 */
class CScreenModList : public CScreen
{
public:
    CScreenModList(CMainDialog* dialog, CModManager* modManager);

    void CreateInterface() override;
    bool EventProcess(const Event &event) override;

protected:
    void ApplyChanges();
    void CloseWindow();

    void UpdateModList();
    void UpdateModDetails();
    void UpdateModSummary();
    void UpdateEnableDisableButton();
    void UpdateApplyButton();
    void UpdateUpDownButtons();

protected:
    Ui::CMainDialog* m_dialog;

    CModManager* m_modManager;

    size_t m_modSelectedIndex = 0;
    bool m_changes = false;
    bool m_empty = true;
};

} // namespace Ui
