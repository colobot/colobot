Gałąź testowa MinGW

Status

Tutaj będą kontynuowane prace nad uruchomieniem projektu pod MinGW.
Gałąź będzie ścisle związana z główną, ale mimo wszystko osobna, żeby niczego nie zepsuć :)

Instrukcja jak skompilować i uruchomić pod MinGW

1. Ściągamy i instalujemy MinGW i MSYS:
   http://sourceforge.net/projects/mingw/files/Installer/mingw-get-inst/mingw-get-inst-20111118/
2. Przy instalacji zaznaczamy wszystkie komponenty do instalacji
3. W pliku src/CMakeLists.txt zmieniamy set(DXSDK_DIR "c:/dxsdk") na katalog, gdzie jest zainstalowany DirectX SDK
   (slashe mają być właśnie w takiej postaci: / a nie \)
4. Uruchamiamy MinGW console ze skrótu w menu start
5. Przechodzimy do katalogu, gdzie są źródła: "cd /c/tam/gdzie/sa/zrodla"
6. "cmake -G 'MSYS Makefiles' ."
7. "make"
8. Wszystko powinno się skomplikować bez błędów
9. Kopiujemy bin/colobot.exe, bin/CBot/libCBot.dll i z katalogu, gdzie jest zainstalowany MinGW
   bin/libgcc_s_dw2-1.dll i bin/libstdc++-6.dll do miejsca, gdzie mamy oryginalne źródła z danymi
10. Odpalamy z -nocd albo jak kto woli
