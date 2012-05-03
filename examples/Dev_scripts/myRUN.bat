call "C:\Program Files\PrismTech\OpenSpliceDDS\V5.3.0p3\HDE\x86.win32/release.bat"
rem TAO
set TAO_ROOT=C:\Devel\TAO161_0-Windows-vc8-inline-32bit-OpenSpliceDDS_v5.1_UNSUPPORTED_100324
set PATH=%TAO_ROOT%\bin;%PATH%
rem JacORB
set JACORB_HOME=C:\Program Files\PrismTech\OpenFusionV4
set PATH=%JACORB_HOME%\bin;%PATH%
call RUN.bat
