@echo OFF
rem delete all spaces and blank lines
rem usage supp_spaces.bat source_file dest_file
if [%2]==[] GOTO usage
del /F /Q %2
FOR /F "delims=" %%i in ('TYPE %1') DO call :cmd "%%i" %2
GOTO end
:cmd
set line=%~1
echo %line: =% >> %2
GOTO end
:usage
echo *** Error ***
echo     usage : supp_spaces.bat source_file dest_file
:end