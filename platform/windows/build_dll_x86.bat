@echo off
setlocal

rem Edit this path if your WinLibs x86 install lives somewhere else.
set "GXX_X86=C:\winlibs32\mingw32\bin\g++.exe"

rem Repo root is two levels up from this script (platform\windows\).
set "REPO_ROOT=%~dp0..\.."
pushd "%REPO_ROOT%" || goto :fail

set "CXXFLAGS=-std=c++17 -O2 -Iinclude -DSHARPVOX_FIXED_POINT_SYNTH -DSHARPVOX_BUILD_DLL -static -static-libgcc -static-libstdc++"
set "SRCS=src\*.cpp platform\lib\SharpVox.cpp platform\windows\sharpvox_win.cpp"

if not exist "%GXX_X86%" (
    echo ERROR: x86 compiler not found at %GXX_X86%
    echo Edit GXX_X86 near the top of this script to point at your WinLibs x86 g++.exe
    goto :fail
)

if not exist "platform\windows\dist\x86" mkdir "platform\windows\dist\x86"

echo Building x86 DLL...
"%GXX_X86%" %CXXFLAGS% -shared %SRCS% -o platform\windows\dist\x86\sharpvox.dll -Wl,--out-implib,platform/windows/dist/x86/sharpvox.lib
if errorlevel 1 goto :fail

echo.
echo Build succeeded: platform\windows\dist\x86\sharpvox.dll
popd
exit /b 0

:fail
echo.
echo Build FAILED.
popd
exit /b 1
