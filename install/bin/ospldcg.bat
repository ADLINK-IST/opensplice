CALL "%~dp0\..\release.bat"
@ echo off

if "%OSPL_HOME%"=="" goto error

set CLASSPATH_TMP=%OSPL_HOME%\jar\ospldcg.jar

set CLASSPATH_TMP=%CLASSPATH_TMP:\=/%
set OSPL_HOME_TMP=%OSPL_HOME:\=/%

@java -DOSPL_HOME="%OSPL_HOME_TMP%" -classpath "%CLASSPATH_TMP%" DCG.Control.DCGStarter %*
goto end

:error
  echo "OSPL_HOME environnment variable not defined"
goto end

:end

