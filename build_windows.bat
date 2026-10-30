@echo off
setlocal
cd /d "%~dp0"
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
if errorlevel 1 exit /b 1
cmake --build build --config Release --parallel 4
if errorlevel 1 exit /b 1
ctest --test-dir build -C Release --output-on-failure
if errorlevel 1 exit /b 1
where ISCC.exe >nul 2>&1
if errorlevel 1 (
 if exist "%ProgramFiles(x86)%\Inno Setup 6\ISCC.exe" (
  "%ProgramFiles(x86)%\Inno Setup 6\ISCC.exe" packaging\windows\IronSqueeze.iss
 ) else (
  echo Inno Setup 6 is required to produce the installer. Plugins built successfully.
  exit /b 2
 )
) else (
 ISCC.exe packaging\windows\IronSqueeze.iss
)
