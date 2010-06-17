@echo off

SET SABINE_ROOT=C:\VXSABINE
call %SABINE_ROOT%\toolexe\vxw_211_bld.bat

rem application to do
if "%1" == "" goto USAGE
rem CONFIG
if "%2" == "" goto USAGE
rem SWCONFIG
if "%3" == "" goto USAGE

SET TGTKERNEL=%SABINE_ROOT%\%1\%2\%1\%RTOS%\%3\debug\use\%1.cof
SET TGTSVR=ts

REM Start the target server.
start tgtsvr.exe ts -B wdbmice -c %TGTKERNEL% -m 4000000 -Mc %2 -MnoStart -Md c:\temp\debug.log -Ml 0
echo Press a key once the target server is loaded...
pause > nul:

REM Load the application to IRD.
wtxtcl %SABINE_ROOT%\toolexe\load_and_wait.tcl

REM Launch the Tornado debugger.
start tornado
goto END

:USAGE
echo Usage:     vxw_dbg [application name] config swconfig
echo   where config =
echo      SPARTA
echo      WABASHDVT
echo 	  KLONMDP
echo      etc...
echo   swconfig = 
echo      GENWAB
echo config names are case sensitive.
echo Example:   vxw_dbg watchtv SPARTA GENWAB

:END

