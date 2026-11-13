@echo off
g++ main.cpp -o csma_sim.exe
if %ERRORLEVEL% equ 0 (
    echo Build successful! Run: csma_sim.exe
) else (
    echo Build failed!
)
