@echo off
setlocal enabledelayedexpansion
title Building Todo List...

:: Настройки выходного файла
set OUTPUT=TodoApp.exe
set FLAGS=-std=c++17 -D_WIN32_WINNT=0x0601 -I. -Icore -Iui
:: Библиотеки: gdiplus для графики, gdi32 для контекста устройства, и mwindows чтобы не было консоли
set LIBS=-municode -lgdiplus -lgdi32 -luser32 -lcomctl32 -lole32 -mwindows -static -static-libgcc -static-libstdc++

echo ======================================================
echo           BUILDING TODO LIST PROJECT
echo ======================================================
echo.

:: Список исходных файлов согласно твоей архитектуре
set SOURCES=main.cpp core/TaskManager.cpp ui/Renderer.cpp

set OBJECTS=
echo [1/2] Compiling source files...
echo ------------------------------------------------------

:: Цикл компиляции каждого файла в объектный файл (.o)
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
echo ------------------------------------------------------
echo [LINK] Creating %OUTPUT%...

:: Линковка всех собранных объектов
g++ -o %OUTPUT% %OBJECTS% %LIBS%

if %ERRORLEVEL% == 0 (
    echo.
    echo ======================================================
    echo [SUCCESS] Build completed: %OUTPUT%
    echo ======================================================
    
    :: Удаление временных объектных файлов
    del *.o
    
    echo Running TodoApp...
    start %OUTPUT%
) else (
    echo.
    echo ======================================================
    echo [FAILED] Build failed! Check errors above.
    echo ======================================================
    pause
)