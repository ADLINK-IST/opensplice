@echo off
cd ..\exec
java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";OwnershipDataPublisher.jar OwnershipDataPublisher %*
cd ..\Bat
