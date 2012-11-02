@echo off
rem echo === ContentFilteredTopicDataPublisher

cd ..\exec

if  "%1" == "" (
  java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";ContentFilteredTopicDataPublisher.jar ContentFilteredTopicDataPublisher
) else (
  java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";ContentFilteredTopicDataPublisher.jar ContentFilteredTopicDataPublisher > ..\Bat\%1
)

cd ..\Bat