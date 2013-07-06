@echo OFF
SETLOCAL

call %FUNCTIONS% :stopOSPL

call %FUNCTIONS% :deleteDBFFiles

@echo off

call %FUNCTIONS% :startOSPL

echo "=== Launching HelloWorld "

start CMD /C HelloWorldDataSubscriber.exe ^> subResult.txt

%SLEEP2% >NUL

call HelloWorldDataPublisher.exe > pubResult.txt

%SLEEP5% >NUL

call %FUNCTIONS% :helloworldCheckResult

call %FUNCTIONS% :stopOSPL