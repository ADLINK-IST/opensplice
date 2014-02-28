@ECHO OFF

REM "TS" means "TouchStones"

REM This script launches the STAX "./run_test.xml" script.

REM Set how long the test will pass in seconds:
SET TEST_DURATION=20

REM Set the name for the host:
SET MASTER_HOST_NAME='ws06'
SET SLAVE_HOST_NAME='vm-dds-rhel53x64'

REM Set the OSPL location on the hosts:
SET MASTER_OSPL_HOME='C:\\PrismTech\\OpenSpliceDDS\\V5.3.0\\HDE\\x86.win32'
SET SLAVE_OSPL_HOME='/home/skharin/PrismTech/OpenSpliceDDS/V5.3.0p3/HDE/x86_64.linux'

REM Set the location where the logs will be stored on the hosts:
SET MASTER_LOG_DIR='C:\\Cygwin\\home\\skharin\\ospli\\testsuite\\tests\\TouchStones'
SET SLAVE_LOG_DIR='/home/skharin/ospli/testsuite/tests/TouchStones'

REM Set the location of the DDS TouchStones binaries on the hosts:
SET MASTER_TS_BIN='C:\\DDS-TouchStone\\dds-touchstone\\v1.3\\bin\\windows'
SET SLAVE_TS_BIN='/home/skharin/dds-touchstone/v1.3/bin/my'

REM Set the language of the DDS TouchStones on the hosts:
SET MASTER_TS_LANG='C'
SET SLAVE_TS_LANG='C'

REM Set the number of the DDS TouchStones on the hosts:
SET MASTER_TS_NUMBER=1
SET SLAVE_TS_NUMBER=1

REM Set the group id of the DDS TouchStones on the hosts:
SET MASTER_TS_GROUP_ID=1001
SET SLAVE_TS_GROUP_ID=2001

REM Set the start application id of the DDS TouchStones on the hosts:
SET MASTER_TS_START_APP_ID=1
SET SLAVE_TS_START_APP_ID=1

REM Set the location where all logs from both hosts and results will be stored on the master host:
SET MASTER_RESULT_DIR='C:\\log'

REM Set the location of the OSPL config on the master host which will be used for testing on both hosts:
SET MASTER_OSPL_CONFIG='C:\\Cygwin\\home\\skharin\\ospli\\testsuite\\tests\\TouchStones\\ospl.broadcast.xml'

REM Set the DDS TouchStone recorder script on the master host which will be used for testing:
SET MASTER_RECORDER_SCRIPT='C:\\DDS-TouchStone\\dds-touchstone\\v1.3\\examples\\simple\\latency\\simple_latency_recording.dat'

REM Start:
STAF local STAX EXECUTE FILE %~dp0\\run_test.xml JOBNAME "DDS ToushStones" FUNCTION touchstone_main ARGS "[%TEST_DURATION%, %MASTER_HOST_NAME%, %SLAVE_HOST_NAME%, %MASTER_OSPL_HOME%, %SLAVE_OSPL_HOME%, %MASTER_LOG_DIR%, %SLAVE_LOG_DIR%, %MASTER_TS_BIN%, %SLAVE_TS_BIN%, %MASTER_TS_LANG%, %SLAVE_TS_LANG%, %MASTER_TS_NUMBER%, %SLAVE_TS_NUMBER%, %MASTER_TS_GROUP_ID%, %SLAVE_TS_GROUP_ID%, %MASTER_RESULT_DIR%, %MASTER_OSPL_CONFIG%, %MASTER_RECORDER_SCRIPT%, %MASTER_TS_START_APP_ID%, %SLAVE_TS_START_APP_ID%]" WAIT RETURNRESULT DETAILS