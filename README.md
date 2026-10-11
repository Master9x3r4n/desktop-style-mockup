# 🫃 OS MARQUEE OPERATOR
- Claro, Stephen Jakobb
- Infante, Gabriel
- Omandac, Karl Deejay
- Placer, Paul John

A desktop-style OS shell mock-up built with GLFW + OpenGL 3.3 + Dear ImGui. 
It simulates the visual experience of a minimal OS
desktop that is redrawn every frame by an immediate-mode UI.

This project was developed using **C++**.

## 🫃 Features
### CLI Startup Screen
- CLI startup screen when booting up the Mock OS for the first time
### OS Desktop 
- Desktop Style Mock OS that features a taskbar, wallpaper, widgets, and desktop icons
### Taskbar
- Taskbar that features the OS Managing Tools, Clock, Power, and Volume and Network Controls
### Start Menu
- Start Menu panel opens up when clicking the menu
- Start Menu features "Pinned Apps" which really displays all the OS Managing Tools and Applications the OS has to offer
- Displays mock data for recently used files
### OS Managing Tools
- **Settings**
  - **Taskbar**
    - Controllable Taskbar alignment **Left** or **Center**
    - Additional mock options for immersion
  - **Personalization**
    - Allows you to select **preset wallpapers** (default, grass, magic, and slop)
    - Allows you to select the accent color
    - Additional mock options for immersion
  - **System**
    - Provides mock controls for display and audio
  - **About**
    - Displays info about the Mock OS
- **File Explorer**
  - Mock File Explorer
  - Uses static dummy data for the files
  - Navigation across different folders (Desktop, Downloads, Documents, Pictures, Music, Local Disc, PC)
  - Navigatable sub folders
  - Mock folder editing buttons (does not work cause mock)
- **System Info**
  - Displays additional mock info about the OS 
- **Task Manager**
  - View mock info of currently running processes
  - View mock graph of OS current performance
### Applications
- **Terminal**
  - Command terminal that functions with working commands
  - Mostly I/O commands
- **Calculator**
  - Working simple calculator
- **Wuhdle**
  - Working wordle game that accepts any 5 letter combination of characters
  - Yes there is only **one hardcoded answer**
- **ChicaGPT**
  - Agentic AI Application that all OS designers need nowadays
  - Features message and reply system with token/message limit
  - Features AI responses from a hardcoded list
- **Reels**
  - Scrollable Reels if you get bored in SAS3000 Seminar
  - Features buttons to scroll upwards and downwards, like the reel, and mock button for comments
  - Reels are just hardcoded images
### Widgets
- **Power Button**
  - Power Button that safely calls the `glfwWindowShouldClose(window)` and closes the entire OS application
- **Date and Time**
  - Date and time display featuring the real world time clock display
- **Volume**
  - Volume adjuster that opens up a mock slider
- **Network**
  - Network panel that allows you to select a mock network to connect to

## 🫃 Requirements
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

## 🫃 Setup
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

## 🫃 Build and Run
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

## Usage
  - Click Files, System Info or Task Manager on the taskbar to open or close
    each window. Windows can be dragged, resized and closed with their own
    X button.
  - To use a custom wallpaper, place a PNG at wallpaper/wallpaper.png (in
    the project root, next to CMakeLists.txt) and restart the program.
  - To shut down, click the red PWR button in the bottom-left corner.




