@echo off

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86

pushd ..
start D:\Development\Vim\vim80\gvim.exe

cls
echo Shell Started
