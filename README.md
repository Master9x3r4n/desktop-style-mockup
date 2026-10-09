# OS MARQUEE OPERATOR
- Claro, Stephen Jakobb
- Infante, Gabriel
- Omandac, Karl Deejay
- Placer, Paul John

A desktop-style OS shell mock-up built with GLFW + OpenGL 3.3 + Dear ImGui. 
It simulates the visual experience of a minimal OS
desktop that is redrawn every frame by an immediate-mode UI.

This project was developed using **C++**.

## Features
- N/A

## Requirements
  - Windows 10/11
  - A C++17 compiler. GCC 8 or newer (MinGW-w64 via MSYS2 recommended) or
    MSVC (Visual Studio Build Tools). Old compilers such as the GCC 4.9
    bundled with Dev-C++ will NOT work.
  - CMake 3.16 or newer
  - Git (CMake downloads GLFW, Dear ImGui and stb using FetchContent)
  - An internet connection on the first configure
  - A GPU/driver that supports OpenGL 3.3
  - Visual Studio Code with these extensions:
      * C/C++ (Microsoft)
      * CMake Tools (Microsoft)

## Setup
MSYS2 MinGW-w64 (GCC)
  1. Install MSYS2 from https://www.msys2.org/ (default path C:\msys64).
  2. Open the "MSYS2 UCRT64" terminal and run:
       pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-gdb mingw-w64-ucrt-x86_64-ninja
  3. Add C:\msys64\ucrt64\bin to your Windows PATH (above any Dev-Cpp entry).
  4. Open a NEW terminal and run "g++ --version". It must report GCC 8 or
     newer.

 Visual Studio Build Tools (MSVC)
  1. Install "Build Tools for Visual Studio" with the
     "Desktop development with C++" workload.

Install CMake (https://cmake.org/download/)

## Build and Run
  1. Open VS Code, then File > Open Folder and select the project folder
     (the one containing CMakeLists.txt).
  2. Press Ctrl+Shift+P and run "CMake: Scan for Kits".
  3. Press Ctrl+Shift+P and run "CMake: Select a Kit". Choose the modern
     compiler (e.g. GCC ... ucrt64, or Visual Studio Build Tools amd64).
     Do not choose the Dev-Cpp compiler.
  4. Press Ctrl+Shift+P and run "CMake: Configure". The first run downloads
     GLFW, Dear ImGui and stb (about a minute).
  5. Build with the "Build" button in the bottom status bar, or press F7.
  6. Run:
       - Run without debugging: the play button in the bottom status bar,
         or Shift+F5.
       - Run with debugging: the bug button in the bottom status bar,
         or Ctrl+F5.
     Make sure the selected launch target is "myapp" (not "all").

The executable is created at build/myapp.exe (MinGW/Ninja) or
build/Debug/myapp.exe (Visual Studio generator).
 
IMPORTANT: Use the CMake buttons in the bottom status bar. Do NOT use the
top-right "Run Code" / "Run C/C++ File" button, because it compiles main.cpp
alone without the ImGui include paths and will fail.

## Project Structure
```
CMakeLists.txt
src/
  Common.h                # Scales (px), constants, shared Desktop state
  Wallpaper.h / .cpp      # Wallpaper texture loading & procedural fallback
  DesktopOverlay.h / .cpp # Top-right clock and PWR button
  Taskbar.h / .cpp        # Bottom taskbar & Start button
  StartMenu.h / .cpp      # Start menu popup
  Apps/
    FileExplorer.h / .cpp # Virtual filesystem and File Explorer window
    SystemInfo.h / .cpp   # System Info window
    TaskManager.h / .cpp  # Task Manager window & mock process metrics
  main.cpp                # App initialization and render loop
```

## Usage
  - Click Files, System Info or Task Manager on the taskbar to open or close
    each window. Windows can be dragged, resized and closed with their own
    X button.
  - To use a custom wallpaper, place a PNG at wallpaper/wallpaper.png (in
    the project root, next to CMakeLists.txt) and restart the program.
  - To shut down, click the red PWR button in the bottom-left corner.




