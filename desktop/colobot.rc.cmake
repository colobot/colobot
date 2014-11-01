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
			VALUE "FileDescription", "Colobot: Gold Edition - Colonize with Bots\0"
			VALUE "ProductName", "Colobot: Gold Edition\0"
			VALUE "CompanyName", "TerranovaTeam\0"
			VALUE "LegalCopyright", "Copyright (c) 2012-2014 TerranovaTeam\0"
			VALUE "FileVersion", "@COLOBOT_VERSION_FULL@\0"
			VALUE "ProductVersion", "@COLOBOT_VERSION_FULL@\0"
			END
		END
		BLOCK "VarFileInfo"
		BEGIN
			VALUE "Translation", 0x409, 1200
		END
END
