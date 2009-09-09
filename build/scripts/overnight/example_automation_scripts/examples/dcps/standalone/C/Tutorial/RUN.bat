@ECHO OFF
set SLEEP2=@ping -n 2 localhost
set SLEEP4=@ping -n 4 localhost
set SLEEP10=@ping -n 10 localhost

REM start OpenSplice

IF NOT EXIST ".\MessageBoard\MessageBoard.exe" (
   ECHO Error: MessageBoard.exe not exist
   GOTO error
)

IF NOT EXIST "Chatter\Chatter.exe" (
   ECHO Error: Chatter.exe not exist
   GOTO error
)

ospl start 

%SLEEP10% >NUL

REM start MessageBoard
start "Messageboard" .\MessageBoard\MessageBoard.exe

%SLEEP4% >NUL

ECHO start Chatter
call .\Chatter\Chatter.exe

%SLEEP2% >NUL

ECHO start Chatter with terminate message
call .\Chatter\Chatter.exe -1

%SLEEP4% >NUL

ospl stop

GOTO end

:error
ECHO An error occurred, exiting in 4 seconds...
%SLEEP4% >NUL
EXIT 1
:end
