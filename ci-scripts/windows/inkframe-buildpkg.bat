@echo off
REM Inkframe portable package builder (fork of tahoma-buildpkg.bat)
REM - package folder:  toonz\build\Inkframe
REM - executable:      Inkframe.exe
REM - stuff folder:    inkframestuff
REM - OpenCV dll:      %OPENCV_DIR%\x64\vc16\bin preferred, choco paths as fallback
REM - no Inno installer step, no repo .gitkeep deletion (upstream script bug)

cd toonz\build

echo ">>> Creating Inkframe directory"
IF EXIST Inkframe rmdir /S /Q Inkframe
mkdir Inkframe

echo ">>> Copy application files"
copy /y RelWithDebInfo\*.* Inkframe
REM stale pre-rename binaries must never ship
IF EXIST Inkframe\Tahoma2D.exe del /Q Inkframe\Tahoma2D.exe
IF EXIST Inkframe\Tahoma2D.pdb del /Q Inkframe\Tahoma2D.pdb

echo ">>> Copy ThirdParty DLLs"
copy /Y ..\..\thirdparty\freeglut\bin\x64\freeglut.dll Inkframe
copy /Y ..\..\thirdparty\glew\glew-1.9.0\bin\64bit\glew32.dll Inkframe
copy /Y ..\..\thirdparty\libmypaint\dist\64\libiconv-2.dll Inkframe
copy /Y ..\..\thirdparty\libmypaint\dist\64\libintl-8.dll Inkframe
copy /Y ..\..\thirdparty\libmypaint\dist\64\libjson-c-2.dll Inkframe
copy /Y ..\..\thirdparty\libmypaint\dist\64\libmypaint-1-4-0.dll Inkframe

echo ">>> Copy OpenCV DLL"
IF DEFINED OPENCV_DIR (
   copy /Y "%OPENCV_DIR%\x64\vc16\bin\opencv_world4110.dll" Inkframe
) ELSE IF EXIST C:\tools\opencv (
   copy /Y "C:\tools\opencv\build\x64\vc16\bin\opencv_world4110.dll" Inkframe
) ELSE (
   copy /Y "C:\opencv\4110\build\x64\vc16\bin\opencv_world4110.dll" Inkframe
)

echo ">>> Copy MSVC runtime DLLs"
set VCINSTALLDIR="C:\Program Files\Microsoft Visual Studio\2022\Community\VC"
IF EXIST "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC" set VCINSTALLDIR="C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC"

set VCRUNTIME_PATH=
for /d /r ""%VCINSTALLDIR%"" %%a in (14.*) do (
    if exist "%%a\x64" (
        for /d %%b in ("%%a\x64\*") do (
            if exist "%%b\vcruntime*.dll" (
                set "VCRUNTIME_PATH=%%b"
                goto :done
            )
        )
    )
)
:done
copy /Y "%VCRUNTIME_PATH%\vcruntime140.dll" Inkframe
copy /Y "%VCRUNTIME_PATH%\vcruntime140_1.dll" Inkframe
copy /Y "%VCRUNTIME_PATH%\msvcp140.dll" Inkframe
copy /Y "%VCRUNTIME_PATH%\msvcp140_1.dll" Inkframe
copy /Y "%VCRUNTIME_PATH%\msvcp140_2.dll" Inkframe

echo ">>> Deploy Qt"
set QT_PATH=C:\Qt\5.15.2_wintab\msvc2019_64
IF EXIST ..\..\thirdparty\qt\5.15.2_wintab\msvc2019_64 set QT_PATH=..\..\thirdparty\qt\5.15.2_wintab\msvc2019_64
%QT_PATH%\bin\windeployqt.exe Inkframe\Inkframe.exe --opengl

IF EXIST ..\..\thirdparty\apps\ffmpeg\bin (
   echo ">>> Copying FFmpeg"
   IF EXIST Inkframe\ffmpeg rmdir /S /Q Inkframe\ffmpeg
   mkdir Inkframe\ffmpeg
   copy /Y ..\..\thirdparty\apps\ffmpeg\bin\ffmpeg.exe Inkframe\ffmpeg
   copy /Y ..\..\thirdparty\apps\ffmpeg\bin\ffprobe.exe Inkframe\ffmpeg
)

IF EXIST ..\..\thirdparty\apps\rhubarb (
   echo ">>> Copying Rhubarb Lip Sync"
   IF EXIST Inkframe\rhubarb rmdir /S /Q Inkframe\rhubarb
   mkdir Inkframe\rhubarb
   copy /Y ..\..\thirdparty\apps\rhubarb\rhubarb.exe Inkframe\rhubarb
   xcopy /Y /E /I ..\..\thirdparty\apps\rhubarb\res "Inkframe\rhubarb\res"
)

echo ">>> Copy stuff as inkframestuff"
xcopy /Y /E /I /Q ..\..\stuff Inkframe\inkframestuff

echo ">>> Creating Inkframe portable zip"
IF EXIST Inkframe-portable-win.zip del Inkframe-portable-win.zip
7z a Inkframe-portable-win.zip Inkframe

cd ..\..
