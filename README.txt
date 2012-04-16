EN: MinGW development branch

Status

This branch will serve as the place for continued work in running the project under CMake and MinGW.
The branch will be strictly tied to the master, but nevertheless separate so as not to break anything.

Instructions for compiling and running with MinGW.

1. Download and install MinGW and MSYS:

2. When installing, select all available components
3. In the file src/CMakeLists.txt, change "set(DXSDK_DIR "c:/dxsdk") to the directory, where you have DirectX SDK (version 8.1)
   (the slashes must be in this form: /, not \)
4. Run MinGW console from the shortcut in menu start
5. Change to the directory where you have the Colobot sources: "cd /c/where/the/sources/are"
6. "cmake -G 'MSYS Makefiles' ."
7. "make"
8. Everything should compile without errors
9. Copy bin/colobot.exe, bin/CBot/libCBot.dll and from the directory, where MinGW is installed
   bin/libgcc_s_dw2-1.dll and bin/libstdc++-6.dll to where you have the data files colobot*.dat
10. Run colobot.exe with -nocd option


PL: Gałąź rozwojowa MinGW

Status

Gałąź ta posłuży za miejsce do kolejnych prac nad uruchomieniem projektu pod MinGW.
Będzie ścisle związana z główną, ale mimo wszystko osobna, żeby niczego nie zepsuć.

Instrukcja jak skompilować i uruchomić pod MinGW

1. Ściągamy i instalujemy MinGW i MSYS:
   http://sourceforge.net/projects/mingw/files/Installer/mingw-get-inst/mingw-get-inst-20111118/
2. Przy instalacji zaznaczamy wszystkie komponenty do instalacji
3. W pliku src/CMakeLists.txt zmieniamy set(DXSDK_DIR "c:/dxsdk") na katalog, gdzie jest zainstalowany DirectX SDK (w wersji 8.1)
   (slashe mają być właśnie w takiej postaci: / a nie \)
4. Uruchamiamy MinGW console ze skrótu w menu start
5. Przechodzimy do katalogu, gdzie są źródła: "cd /c/tam/gdzie/sa/zrodla"
6. "cmake -G 'MSYS Makefiles' ."
7. "make"
8. Wszystko powinno się skomplikować bez błędów
9. Kopiujemy bin/colobot.exe, bin/CBot/libCBot.dll i z katalogu, gdzie jest zainstalowany MinGW
   bin/libgcc_s_dw2-1.dll i bin/libstdc++-6.dll do miejsca, gdzie mamy pliki z danymi colobot*.dat
10. Odpalamy colobot.exe z -nocd
