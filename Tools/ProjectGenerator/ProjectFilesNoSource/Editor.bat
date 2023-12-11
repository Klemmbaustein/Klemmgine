@echo off
set location=%cd%
cd %~dp0
bin\Klemmgine-Editor.exe -editorPath ../..
cd %location%