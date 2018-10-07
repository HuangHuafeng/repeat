REM x64 Native Tools Command Prompt for VS 2017
CALL "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"

SET PATH=%PATH%;C:\Qt\5.11.1\msvc2017_64\bin\;C:\Qt\Tools\QtCreator\bin\;
SET APPNAME=HaiBeiDanCi

MKDIR packages
CD packages
MKDIR %APPNAME%
CD %APPNAME%

ECHO remove all the files
DEL /Q .\*

ECHO remove the old package directory
RMDIR /S /Q .\%APPNAME%

ECHO remove the old release directory
RMDIR /S /Q .\release
RMDIR /S /Q .\debug

ECHO config and build release
qmake.exe "CONFIG += no_batch" Z:\GitHub\TextFinder\Repeat\%APPNAME%\%APPNAME%.pro -spec win32-msvc && jom.exe qmake_all
jom.exe

ECHO copy the executable to package directory
XCOPY /S /E /Y .\release\%APPNAME%.exe .\%APPNAME%\

ECHO deploy
windeployqt %APPNAME%

ECHO copy the quazip dll to package directory
COPY C:\Qt\quazip-0.7.6\quazip\release\quazip.dll .\%APPNAME%\

ECHO zip the package directory
"C:\Program Files\7-Zip\7z.exe" a ..\%APPNAME%.zip %APPNAME%

cd ..\..

PAUSE