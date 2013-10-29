id ICON "@CMAKE_CURRENT_SOURCE_DIR@/colobot.ico"

1 VERSIONINFO
FILEVERSION @COLOBOT_VERSION_4COMMAS@
PRODUCTVERSION @COLOBOT_VERSION_4COMMAS@
BEGIN
	BLOCK "StringFileInfo"
	BEGIN
		BLOCK "040904b0"
		BEGIN
			VALUE "OriginalFilename", "colobot.exe\0"
			VALUE "InternalName", "colobot\0"
			VALUE "FileDescription", "Colobot - Colonize with Bots\0"
			VALUE "ProductName", "Colobot\0"
			VALUE "CompanyName", "Polish Portal of Colobot\0"
			VALUE "LegalCopyright", "Copyright (c) 2012-2013 Polish Portal of Colobot\0"
			VALUE "FileVersion", "@COLOBOT_VERSION_FULL@\0"
			VALUE "ProductVersion", "@COLOBOT_VERSION_FULL@\0"
			END
		END
		BLOCK "VarFileInfo"
		BEGIN
			VALUE "Translation", 0x409, 1200
		END
END
