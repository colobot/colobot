/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include <glm/glm.hpp>

#include <filesystem>
#include <string>
#include <vector>


struct Event;

namespace Ui
{

class CEdit;
class CInterface;


/**
 * \brief File selector dialog
 *
 * \section Example Example usage
 * Create the dialog and set the type of dialog.
 * \code
 * CFileDialog* fileDialog = new CFileDialog();
 * fileDialog->SetDialogType(CFileDialog::Type::Folder);
 * \endcode
 *
 * Initial settings and start the dialog.
 * \code
 * fileDialog->SetWindowTitle("Select Player Folder");
 * fileDialog->SetBasePath("savegame");
 * fileDialog->StartDialog();
 * \endcode
 *
 * Handle events for the dialog.
 * \code
 * // first check for events sent from the dialog
 *
 * if (event.type == EVENT_DIALOG_STOP) // cancel or close
 * {
 *     fileDialog->StopDialog();
 *     delete fileDialog;
 *     return true;
 * }
 *
 * if (event.type == EVENT_DIALOG_ACTION) // ok button was pressed
 * {
 *     std::string folder = fileDialog->GetSubFolderPath();
 *     fileDialog->StopDialog();
 *     delete fileDialog;
 *     return true;
 * }
 *
 * // send the event to the dialog
 * return m_fileDialog->EventProcess(event);
 * \endcode
 *
 * \nosubgrouping
 */
class CFileDialog
{
public:

    /**
     * \brief Constructor
     */
    CFileDialog();

    /**
     * \brief Destructor
     */
    ~CFileDialog();

    //! \name Set dialog type, starting, event processing, and stopping
    //@{
    /**
     * \brief Disables other windows and creates the dialog window.
     */
    void        StartDialog();

    /**
     * \brief Enables other windows and deletes the dialog window.
     */
    void        StopDialog();

    /**
     * \brief Event processing.
     */
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

    /**
     * \brief Set the type of dialog to use.
     */
    void              SetDialogType(CFileDialog::Type type) { m_dialogtype = type; }

    /**
     * \brief Get the type of dialog.
     */
    CFileDialog::Type GetDialogType()                       { return m_dialogtype; }

    //@}

    //! \name Dialog window properties
    //@{
    /**
     * \brief Set EventType for the dialog window.
     * If not set, a unique EventType will be used.
     */
    void        SetWindowEvent(EventType type) { m_windowEvent = type; }

    /**
     * \brief Get EventType for the dialog window.
     */
    EventType   GetWindowEvent()               { return m_windowEvent; }

    /**
     * \brief Set the initial position of the window.
     */
    void        SetWindowPos(const glm::vec2& pos) { m_windowPos = pos; }

    /**
     * \brief Get the position of the window.
     */
    glm::vec2   GetWindowPos()                { return m_windowPos; }

    /**
     * \brief Set the initial size of the window.
     */
    void        SetWindowDim(const glm::vec2& dim) { m_windowDim = dim; }

    /**
     * \brief Get the size of the window.
     */
    glm::vec2   GetWindowDim()                { return m_windowDim; }

    /**
     * \brief Set the text for the title bar of the dialog.
     * This setting will override the default title text for the dialog.
     */
    void        SetWindowTitle(const std::string& name) { m_title = name; }

    //@}

    //! \name Settings for Public and Private check boxes
    //@{
    /**
     * \brief Set whether to create Public and Private check boxes.
     * \param usePublic If true, Public and Private check boxes will be added to the dialog.
     */
    void        SetUsePublicPrivate(bool usePublic);

    /**
     * \brief Set initial state for Public and Private check boxes.
     * \param bPublic If true, the Public check box will be marked.
     */
    void        SetPublic(bool bPublic);

    /**
     * \brief Get the state of Public and Private check boxes.
     * \return true if Public check box is marked and false for Private.
     */
    bool        GetPublic();

    /**
     * \brief Set the path for the folder associated with the Public check box.
     * \param dir Path to 'Public' folder.
     */
    void        SetPublicFolder(const std::filesystem::path& dir);

    /**
     * \brief Set the path for the folder associated with the Private check box.
     * \param dir Path to 'Private' folder.
     */
    void        SetPrivateFolder(const std::filesystem::path& dir);

    //@}

    //! \name Folder settings
    //@{
    /**
     * \brief Set the initial path for the folder whose contents are displayed.
     * This setting is overridden by Public/Private settings.
     */
    void        SetBasePath(const std::filesystem::path& dir);

    /**
     * \brief Get the initial path or Public/Private folder path
     */
    const std::filesystem::path& GetBasePath() const;

    /**
     * \brief Set the initial subfolder whose contents are displayed.
     * \param dir Name of a subfolder. Ex. "subfolder/anotherFolder/oneMoreFolder"
     */
    void        SetSubFolderPath(const std::filesystem::path& dir);

    /**
     * \brief Get the current subfolder shown by the dialog.
     * \return A string with a folder name and subsequent folders separated by forward slash.
     * <p>Returns empty string if the dialog is showing GetBasePath().
     */
    const std::filesystem::path& GetSubFolderPath() const;

    //@}

    //! \name File name settings
    //@{
    /**
     * \brief Set the extension that may be appended to a file name.
     * <p> If any extensions are defined, the dialog will only show files
     * with those extensions.
     * \param ext A string with an extension. Ex. ".txt"
     */
    void        SetAutoExtension(const std::filesystem::path& ext) { m_extension = ext; }

    /**
     * \brief Define extensions that will be accepted as part of a valid file name.
     * \param ext A string with an extension. Ex. ".txt"
     */
    void        AddOptionalExtension(const std::filesystem::path& ext) { m_extlist.push_back(ext); }

    /**
     * \brief Set the filename that appears in the edit box when the dialog opens.
     */
    void        SetFilename(const std::filesystem::path& filename);

    /**
     * \brief Get the filename that was selected or typed.
     * \return The filename that was typed in the edit box.
     */
    const std::filesystem::path& GetFilename() const;

    /**
     * \brief Set whether to check if a file exists when the 'Save' button is pressed,
     * and if the file exists, an "Overwrite existing file?" message is shown.
     * \param doCheck true to check if a file exists when the 'Save' button is pressed.
     */
    void        SetConfirmOverwrite(bool doCheck) { m_confirmOverwrite = doCheck; }

    //@}

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
    bool        DirectoryExists(const std::filesystem::path& name);

    bool        CheckFilename(const std::filesystem::path& name);
    bool        ActionOpen();
    bool        ActionSave(bool checkFileExist = false);

    bool        StartAskOverwrite(const std::filesystem::path& name);
    bool        StopAskOverwrite();
    bool        EventAskOverwrite(const Event &event);

    /*!
     * \brief Set the text in the file name edit box.
     * \param edit Pointer to the edit box.
     * \param filename Text to put in the edit box.
    */
    void        SetFilenameField(CEdit* edit, const std::filesystem::path& filename);

    /*!
     * \brief Get the current directory with the current sub-directory appended.
     * \param bCreate If true, the directories in question will be created.
     * \return A string with the path of current directory, plus the
     * current sub-directory if any.
     */
    std::filesystem::path SearchDirectory(bool bCreate);

private:

    CEventQueue*    m_eventQueue;
    CInterface*     m_interface;

    CFileDialog::Type  m_dialogtype = Type::None;

    // EventType for this dialog.
    // With EVENT_NULL, a unique EventType will be used.
    EventType    m_windowEvent = EVENT_NULL;

    glm::vec2    m_windowPos;
    glm::vec2    m_windowDim;
    std::string  m_title = "";

    float        m_time;
    float        m_lastTimeClickDir;
    bool         m_captureClick = false;

    bool         m_newFolderMode = false;
    bool         m_selectFolderMode = false;
    bool         m_askOverwriteMode = false;
    bool         m_confirmOverwrite = false;

    bool         m_public = false;
    std::filesystem::path m_pathPublic  = "";
    std::filesystem::path m_pathPrivate = "";
    bool         m_usePublicPrivate = false;

    std::filesystem::path m_basePath   = "";
    std::filesystem::path  m_subDirPath = "";

    std::filesystem::path  m_filename = "";

    //! The extension to add to a filename if needed
    std::filesystem::path  m_extension = "";
    //! List of extensions accepted as part of a valid file name
    std::vector<std::filesystem::path> m_extlist = {};

    //! Contents of the dirrectory
    std::vector<std::filesystem::path> m_entries = {};
    int m_dirCount = 0;
};

} // namespace Ui
