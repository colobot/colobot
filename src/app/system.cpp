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

// system.cpp

#include "app/system.h"

#include "common/config.h"

#if defined(PLATFORM_WINDOWS)
#include <windows.h>
#elif defined(PLATFORM_LINUX)
#include <cstdlib>
#else
#include <iostream>
#endif



SystemDialogResult SystemDialog_Windows(SystemDialogType type, const std::string& title, const std::string& message);
SystemDialogResult SystemDialog_Linux(SystemDialogType type, const std::string& title, const std::string& message);
SystemDialogResult SystemDialog_Other(SystemDialogType type, const std::string& title, const std::string& message);

/**
 * Displays a system dialog with info, error, question etc. message.
 *
 * \param type type of dialog
 * \param message text of message (in UTF-8)
 * \param title dialog title (in UTF-8)
 * \returns result (which button was clicked)
 */
SystemDialogResult SystemDialog(SystemDialogType type, const std::string& title, const std::string& message)
{
	#if defined(PLATFORM_WINDOWS)
	return SystemDialog_Windows(type, title, message);
	#elif defined(PLATFORM_LINUX)
	return SystemDialog_Linux(type, title, message);
	#else
	return SystemDialog_Other(type, title, message);
	#endif
}



#if defined(PLATFORM_WINDOWS)

// Convert a wide Unicode string to an UTF8 string
std::string UTF8_Encode_Windows(const std::wstring &wstr)
{
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}

// Convert an UTF8 string to a wide Unicode String
std::wstring UTF8_Decode_Windows(const std::string &str)
{
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}

SystemDialogResult SystemDialog_Windows(SystemDialogType type, const std::string& title, const std::string& message)
{
	unsigned int windowsType = 0;
	std::wstring windowsMessage = UTF8_Decode_Windows(message);
	std::wstring windowsTitle = UTF8_Decode_Windows(title);

	switch (type)
	{
		case SDT_INFO:
		default:
			windowsType = MB_ICONINFORMATION|MB_OK;
			break;
		case SDT_WARNING:
			windowsType = MB_ICONWARNING|MB_OK;
			break;
		case SDT_ERROR:
			windowsType = MB_ICONERROR|MB_OK;
			break;
		case SDT_YES_NO:
			windowsType = MB_ICONQUESTION|MB_YESNO;
			break;
		case SDT_OK_CANCEL:
			windowsType = MB_ICONWARNING|MB_OKCANCEL;
			break;
	}

	switch (MessageBoxW(NULL, windowsMessage.c_str(), windowsTitle.c_str(), windowsType))
	{
		case IDOK:
			return SDR_OK;
		case IDCANCEL:
			return SDR_CANCEL;
		case IDYES:
			return SDR_YES;
		case IDNO:
			return SDR_NO;
		default:
			break;
	}

	return SDR_OK;
}

#elif defined(PLATFORM_LINUX)

SystemDialogResult SystemDialog_Linux(SystemDialogType type, const std::string& title, const std::string& message)
{
	std::string options = "";
	switch (type)
	{
		case SDT_INFO:
		default:
			options = "--info";
			break;
        case SDT_WARNING:
			options = "--warning";
			break;
        case SDT_ERROR:
			options = "--error";
			break;
        case SDT_YES_NO:
			options = "--question --ok-label=\"Yes\" --cancel-label=\"No\"";
			break;
        case SDT_OK_CANCEL:
			options = "--question --ok-label=\"OK\" --cancel-label=\"Cancel\"";
			break;
	}

	std::string command = "zenity " + options + " --text=\"" + message + "\" --title=\"" + title + "\"";
	int code = system(command.c_str());

	SystemDialogResult result = SDR_OK;
	switch (type)
	{
		case SDT_YES_NO:
			result = code ? SDR_NO : SDR_YES;
			break;
		case SDT_OK_CANCEL:
			result = code ? SDR_CANCEL : SDR_OK;
			break;
		default:
			break;
	}

	return result;
}

#else

SystemDialogResult SystemDialog_Other(SystemDialogType type, const std::string& title, const std::string& message)
{
	switch (type)
	{
		case SDT_INFO:
			std::cout << "INFO: ";
			break;
		case SDT_WARNING:
			std::cout << "WARNING:";
			break;
		case SDT_ERROR:
			std::cout << "ERROR: ";
			break;
		case SDT_YES_NO:
		case SDT_OK_CANCEL:
			std::cout << "QUESTION: ";
			break;
	}

	std::cout << message << std::endl;

	std::string line;

	SystemDialogResult result = SDR_OK;

	bool done = false;
	while (!done)
	{
		switch (type)
		{
			case SDT_INFO:
			case SDT_WARNING:
			case SDT_ERROR:
				std::cout << "Press ENTER to continue";
				break;

			case SDT_YES_NO:
				std::cout << "Type 'Y' for Yes or 'N' for No";
				break;

			case SDT_OK_CANCEL:
				std::cout << "Type 'O' for OK or 'C' for Cancel";
				break;
		}

		std::getline(std::cin, line);

		switch (type)
		{
			case SDT_INFO:
			case SDT_WARNING:
			case SDT_ERROR:
				done = true;
				break;

			case SDT_YES_NO:
				if (line == "Y" || line == "y")
				{
					result = SDR_YES;
					done = true;
				}
				else if (line == "N" || line == "n")
				{
					result = SDR_NO;
					done = true;
				}
				break;

			case SDT_OK_CANCEL:
				if (line == "O" || line == "o")
				{
					done = true;
					result = SDR_OK;
				}
				else if (line == "C" || line == "c")
				{
					done = true;
					result = SDR_CANCEL;
				}
				break;
		}
	}

	return result;
}
#endif // if defined(PLATFORM_WINDOWS)
