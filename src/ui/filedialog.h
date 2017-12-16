/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2016, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "common/event.h"
#include "math/point.h"

#include <string>
#include <vector>


struct Event;

namespace Ui
{

class CEdit;
class CInterface;


class CFileDialog
{
public:

    CFileDialog();
    ~CFileDialog();

    void        StartDialog();
    void        StopDialog();
    bool        EventProcess(const Event &event);

    /**
     * \brief Identifies the type of dialog to display.
     */
    enum class Type
    {
        None,    //!< Type was not set
        Open,    //!< Open dialog
        Save,    //!< Save dialog
        Folder,  //!< Select Folder dialog
    };

    void              SetDialogType(CFileDialog::Type type) { m_dialogtype = type; }
    CFileDialog::Type GetDialogType()                       { return m_dialogtype; }

    // Set EventType for this dialog.
    // If not set, a unique EventType will be used.
    void        SetWindowEvent(EventType type) { m_windowEvent = type; }
    EventType   GetWindowEvent()               { return m_windowEvent; }

    void        SetWindowPos(Math::Point pos) { m_windowPos = pos; }
    Math::Point GetWindowPos()                { return m_windowPos; }

    void        SetWindowDim(Math::Point dim) { m_windowDim = dim; }
    Math::Point GetWindowDim()                { return m_windowDim; }

    void        SetWindowTitle(const std::string& name) { m_title = name; }

    void        SetUsePublicPrivate(bool usePublic);
    void        SetPublic(bool bPublic);
    bool        GetPublic();

    void        SetPublicFolder(const std::string& dir);
    void        SetPrivateFolder(const std::string& dir);

    void        SetBasePath(const std::string& dir);
    std::string GetBasePath();

    void        SetSubFolderPath(const std::string& dir);
    std::string GetSubFolderPath();

    void        SetAutoExtension(const std::string& ext) { m_extension = ext; }
    void        AddOptionalExtension(const std::string& ext) { m_extlist.push_back(ext); }

    void        SetFilename(const std::string& name);
    std::string GetFilename();

    void        SetConfirmOverwrite(bool doCheck) { m_confirmOverwrite = doCheck; }

private:

    void        StartFileDialog();
    void        AdjustDialog();

    void        PopulateList();
    void        GetListChoice();
    void        SearchList(const std::string &text, bool dirOnly = false);

    void        UpdateAction();
    void        UpdatePathLabel();
    void        UpdatePublic(bool bPublic);

    void        OpenFolder();

    bool        StartNewFolderMode();
    bool        StopNewFolderMode(bool bCancel = false);
    bool        EventNewFolder(const Event &event);
    void        UpdateNewFolder();
    void        CreateNewFolder();

    bool        EventSelectFolder(const Event &event);
    void        UpdateSelectFolder();

    bool        ListItemIsFolder();
    bool        DirectoryExists(const std::string &name);

    bool        CheckFilename(const std::string& name);
    bool        ActionOpen();
    bool        ActionSave(bool checkFileExist = false);

    bool        StartAskOverwrite(const std::string& name);
    bool        StopAskOverwrite();
    bool        EventAskOverwrite(const Event &event);

    /*!
     * \brief Set the text in the file name edit box.
     * \param edit Pointer to the edit box.
     * \param filename Text to put in the edit box.
    */
    void        SetFilenameField(CEdit* edit, const std::string& filename);

    /*!
     * \brief Get the current directory with the current sub-directory appended.
     * \param bCreate If true, the directories in question will be created.
     * \return A string with the path of current directory, plus the
     * current sub-directory if any.
     */
    std::string SearchDirectory(bool bCreate);

private:

    CEventQueue*    m_eventQueue;
    CInterface*     m_interface;

    CFileDialog::Type  m_dialogtype = Type::None;

    // EventType for this dialog.
    // With EVENT_NULL, a unique EventType will be used.
    EventType    m_windowEvent = EVENT_NULL;

    Math::Point  m_windowPos;
    Math::Point  m_windowDim;
    std::string  m_title = "";

    float        m_time;
    float        m_lastTimeClickDir;
    bool         m_captureClick = false;

    bool         m_newFolderMode = false;
    bool         m_selectFolderMode = false;
    bool         m_askOverwriteMode = false;
    bool         m_confirmOverwrite = false;

    bool         m_public;
    std::string  m_pathPublic  = "";
    std::string  m_pathPrivate = "";
    bool         m_usePublicPrivate = false;

    std::string  m_basePath   = "";
    std::string  m_subDirPath = "";

    std::string  m_filename = "";

    //! The extension to add to a filename if needed
    std::string  m_extension = "";
    //! List of extensions accepted as part of a valid file name
    std::vector<std::string> m_extlist = {};
};

} // namespace Ui
