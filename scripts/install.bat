call build.bat

mkdir "%ProgramFiles%\Dua"
copy "..\build\Dua.exe" "%ProgramFiles%\Dua"

call build_lib.bat
move dua.lib "%ProgramFiles%\Dua\dua.lib"

setx /M PATH "%ProgramFiles%\Dua;%PATH%"
