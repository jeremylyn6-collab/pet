@echo off
setlocal
where cl >nul 2>nul
if errorlevel 1 (
  echo Microsoft C++ Build Tools not found.
  echo Install "Desktop development with C++" from https://visualstudio.microsoft.com/visual-cpp-build-tools/
  exit /b 1
)
cl /nologo /O2 /W4 /DUNICODE /D_UNICODE cockroach.c /link /SUBSYSTEM:WINDOWS /OUT:DesktopCockroach.exe
if errorlevel 1 exit /b 1
del /q cockroach.obj 2>nul
echo.
echo Build complete: DesktopCockroach.exe
