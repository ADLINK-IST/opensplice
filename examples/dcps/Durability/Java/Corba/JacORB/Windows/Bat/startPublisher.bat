@echo off
rem echo === DurabilityDataPublisher

cd ..\exec
echo === DurabilityDataPublisher %*
  java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";DurabilityDataPublisher.jar DurabilityDataPublisher %*
cd ..\Bat