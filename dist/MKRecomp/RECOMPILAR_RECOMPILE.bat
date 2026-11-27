@echo off
rem Recompila los juegos desde tus ROMs / Recompiles the games from your ROMs
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0build_recomp.ps1" %*
pause
