echo OFF
SETLOCAL

echo "===== calling Listener for sac ====="

set EXAMPLE_LANG=sac

call %FUNCTIONS% :runListener

call %FUNCTIONS% :listenerCheckResults >> run.log
