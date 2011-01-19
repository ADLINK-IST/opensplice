@echo off
rem echo === WaitSetDataPublisher

cd ..\exec
SET SLEEP2=ping 127.0.0.1 -n 2
%SLEEP2% > nul
if /I "%1" == "" (
  java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";WaitSetDataPublisher.jar WaitSetDataPublisher
) else (
  java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";WaitSetDataPublisher.jar WaitSetDataPublisher > ..\Bat\%1
)

cd ..\Bat
