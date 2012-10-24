@echo OFF
call "C:\Program Files\PrismTech\OpenSpliceDDS\V5.3.0p3\HDE\x86.win32\release.bat"
set TAO_ROOT=C:\Devel\TAO161_0-Windows-vc8-inline-32bit-OpenSpliceDDS_v5.1_UNSUPPORTED_100324
set PATH=%TAO_ROOT%\bin;%PATH%
echo %OSPL_HOME%
cd ..
echo %CD%

rem xcopy /Y /S examples "%OSPL_HOME%"\examples

cd "%OSPL_HOME%"\examples
echo %CD%

start NewExamples.sln
