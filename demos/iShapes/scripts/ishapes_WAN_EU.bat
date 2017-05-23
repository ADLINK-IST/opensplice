@echo off
set OSPL_HOME=%~dp0
set OSPL_TARGET=
set OSPL_URI=file://.\ishapes-eu.xml
set /p PARTITION= "Enter your email address (the same used to log on the vortex demo): "
start ishapes.exec %PARTITION%
exit
