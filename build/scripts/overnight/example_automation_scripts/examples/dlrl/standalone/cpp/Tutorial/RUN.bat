@ECHO OFF

call %FUNCTIONS% :deleteDBFFiles

call %FUNCTIONS% :startOSPL

ECHO RUN
start CMD /C ./WhiteListedMessageBoard -name family ^> messageBoard.log

sleep 4

ECHO RUN EDITOR
start CMD /C ./WhiteListEditor -name family -add 1,mam 2,dad 3,John ^> editor.log

sleep 4


ECHO RUN VIEWER
start CMD /C ./WhiteListViewer ^> viewer.log

sleep 4

"%OSPL_HOME%\examples\dcps\Tutorial\cpp\standalone\Chatter.exe" 1 mam >> %LOGFILE%

sleep 4

"%OSPL_HOME%\examples\dcps\Tutorial\cpp\standalone\Chatter.exe" 6 fred >> %LOGFILE%

sleep 4

ECHO start Chatter with terminate message
"%OSPL_HOME%\examples\dcps\Tutorial\cpp\standalone\Chatter.exe" -1 >> %LOGFILE%


GOTO end

:error
ECHO An error occurred, exiting example ...

:end

call %FUNCTIONS% :stopOSPL