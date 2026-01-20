@echo off
setlocal enabledelayedexpansion
title Building Todo List...

set OUTPUT=TodoApp.exe
set FLAGS=-std=c++17 -D_WIN32_WINNT=0x0601 -I. -Icore -Iui
set LIBS=-municode -lgdiplus -lgdi32 -luser32 -lcomctl32 -lole32 -mwindows -static -static-libgcc -static-libstdc++

echo BUILDING TODO LIST PROJECT
echo.

set SOURCES=main.cpp core/TaskManager.cpp ui/Renderer.cpp

set OBJECTS=
echo [1/2] Compiling source files...

for %%F in (%SOURCES%) do (
    echo [COMPILE] %%F...
    g++ -c %%F -o "%%~nF.o" %FLAGS%
    if !ERRORLEVEL! neq 0 (
        echo.
        echo [!] Error during compilation of %%F
        pause
        exit /b !ERRORLEVEL!
    )
    set OBJECTS=!OBJECTS! "%%~nF.o"
)

echo.
echo [2/2] Linking objects into executable...
echo [LINK] Creating %OUTPUT%...

g++ -o %OUTPUT% %OBJECTS% %LIBS%

if %ERRORLEVEL% == 0 (
    echo.
    echo [SUCCESS] Build completed: %OUTPUT%

    del *.o
    
    echo Running TodoApp...
    start %OUTPUT%
) else (
    echo.
    echo [FAILED] Build failed! Check errors above.
    pause
)