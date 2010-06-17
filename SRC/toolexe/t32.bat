@echo  off
SET TRACE32=C:\T32
CALL C:\REPLACE\TOOLS\DEBUG\vxw_bld.bat > nul:

if "%SABINE_ROOT%"=="" goto usage
if "%RTOS%"=="" goto usage
if "%TRACE32%"=="" goto usage
if "%1"=="" goto usage

SET CNXT_INSTALL_BASE=

if EXIST %TRACE32%\config.bak goto nopatchconfig
echo Backing up %TRACE32%\config.t32 to %TRACE32%\config.bak
echo Grabbing latest config.t32 from %CNXT_INSTALL_BASE%\TOOLEXE

move %TRACE32%\config.t32 %TRACE32%\config.bak > nul:
copy %CNXT_INSTALL_BASE%\TOOLS\DEBUG\config.t32 %TRACE32% > nul:

:nopatchconfig

if EXIST %TRACE32%\t32.bak goto nopatcht32cmm
echo Backing up %TRACE32%\t32.cmm to %TRACE32%\t32.bak
echo Grabbing latest t32.cmm from %CNXT_INSTALL_BASE%\TOOLEXE
move %TRACE32%\t32.cmm %TRACE32%\t32.bak > nul:
copy %CNXT_INSTALL_BASE%\TOOLS\DEBUG\t32.cmm %TRACE32% > nul:
:nopatcht32cmm

if "%2"=="" goto determinehwconfig
set CONFIG=%2
goto havehwconfig
:determinehwconfig
if "%CONFIG%"=="" goto usage
:havehwconfig

if "%3"=="" goto determineswconfig
set SWCONFIG=%3
goto haveswconfig
:determineswconfig
if "%SWCONFIG%"=="" set SWCONFIG=CNXT
echo WARNING: SWCONFIG undefined.  Using default of CNXT.
echo.
:haveswconfig

:prepare
set SCRIPT_FN=%1.cmm
if /I "%1"=="CODELDR" (
	set IMGSUF=_rom
	set rtos=NOOS
) else (
	set IMGSUF=_ram
)
set IMGEXT=axf

if EXIST %SCRIPT_FN% del %SCRIPT_FN%
REM 
echo. > %SCRIPT_FN%


if /I "%CONFIG%"=="sparta_na" goto core920
if /I "%CONFIG%"=="sparta_euro" goto core920
if /I "%CONFIG%"=="broncovxwb" goto core920
if /I "%CONFIG%"=="milano_na" goto core920
if /I "%CONFIG%"=="milano_euro" goto core920
if /I "%CONFIG%"=="bronco" goto core920
if /I "%CONFIG%"=="eureka" goto core920

echo  b::system.cpu arm940t                   >> %SCRIPT_FN%
if /I "%CONFIG%"=="broncovxwa" (
	echo  b::system.multicore irpost 4            >> %SCRIPT_FN%
	echo  b::system.multicore drpost 1            >> %SCRIPT_FN%
 ) else (
	echo  b::system.multicore irpost 0            >> %SCRIPT_FN%
	echo  b::system.multicore drpost 0            >> %SCRIPT_FN%
 )

goto main

:core920
echo  b::sys.cpu arm920t                   >> %SCRIPT_FN%
if /I "%CONFIG%"=="bronco" (
	echo  b::system.multicore irpost 4            >> %SCRIPT_FN%
	echo  b::system.multicore drpost 1            >> %SCRIPT_FN%
    goto main
) 

if /I "%CONFIG%"=="eureka" (
    echo  b::system.multicore irpost 4            >> %SCRIPT_FN%
	echo  b::system.multicore drpost 1            >> %SCRIPT_FN%
    goto main
) 

echo  b::sys.multicore irpost 8            >> %SCRIPT_FN%
echo  b::sys.multicore drpost 2            >> %SCRIPT_FN%

goto main

:main
echo  b::map.bonchip 20000000--2fffffff       >> %SCRIPT_FN%
echo  b::system.mode.attach                   >> %SCRIPT_FN%
echo  if state.run()                          >> %SCRIPT_FN%
echo  (                                       >> %SCRIPT_FN%
echo      b::break.                           >> %SCRIPT_FN%
echo  )                                       >> %SCRIPT_FN%

echo  b::D.A r:7ffe00 mov pc,r14              >> %SCRIPT_FN%
echo  b::D.PROLOG.target 0x7ffe00--0x7fffff   >> %SCRIPT_FN%
echo  b::D.PROLOG.ON                          >> %SCRIPT_FN%

echo  b::symbol.spath + %SABINE_ROOT%\VXBSP          >> %SCRIPT_FN%
echo  b::symbol.spath + %SABINE_ROOT%\VXKAL          >> %SCRIPT_FN%
echo  b::symbol.spath + %SABINE_ROOT%\PSOSBSP        >> %SCRIPT_FN%
echo  b::symbol.spath + %SABINE_ROOT%\PSOSKAL        >> %SCRIPT_FN%
echo  b::symbol.spath + %SABINE_ROOT%\NUPBSP         >> %SCRIPT_FN%
echo  b::symbol.spath + %SABINE_ROOT%\NUPKAL         >> %SCRIPT_FN%
echo  b::symbol.spath + %SABINE_ROOT%\STARTUP        >> %SCRIPT_FN% 
echo  b::symbol.spath + %SABINE_ROOT%\HWLIB          >> %SCRIPT_FN% 
echo  b::symbol.spath + %SABINE_ROOT%\WATCHTV        >> %SCRIPT_FN% 
echo  b::symbol.spath + %SABINE_ROOT%\DEMUX          >> %SCRIPT_FN% 
echo  b::symbol.spath + %SABINE_ROOT%\DEMOD          >> %SCRIPT_FN% 
echo  b::symbol.spath + %SABINE_ROOT%\DEMOD_CX24430  >> %SCRIPT_FN% 
echo  b::symbol.spath + %SABINE_ROOT%\VIDEO          >> %SCRIPT_FN% 
echo  b::symbol.spath + %SABINE_ROOT%\AUDCOMN        >> %SCRIPT_FN% 
echo  b::symbol.spath + %SABINE_ROOT%\IIC            >> %SCRIPT_FN% 
echo  b::symbol.spath + %SABINE_ROOT%\FLASH          >> %SCRIPT_FN% 

rem if NOT "%RTOS%"=="VXWORKS" goto notvxw
rem set imgsuf=
if "%ARM_VERSION%"=="211" set IMGEXT=cof
if "%ARM_VERSION%"=="296" set IMGEXT=elf
rem :notvxw


echo  b::d.load %sabine_root%\%1\%config%\%1\%rtos%\%SWCONFIG%\debug\use\%1%IMGSUF%.%IMGEXT%   >> %SCRIPT_FN%
if "%RTOS%"=="VXWORKS" if "%ARM_VERSION%"=="211" echo  b::r.s PC _sysInit >> %SCRIPT_FN%
REM   Show source code
echo  b::data.list                                                        >> %SCRIPT_FN%
REM   Show registers
echo  b::R                                                                >> %SCRIPT_FN%
echo  b::PRINT "READY!"                                                   >> %SCRIPT_FN%
echo  enddo                                                               >> %SCRIPT_FN%

if "%3" == "no_run" echo Generating %SCRIPT_FN% but not launching T32...
if "%3" == "no_run" goto cleanup

:launch
start %TRACE32%\t32marm.exe -c %TRACE32%\config.t32 %SCRIPT_FN%

:cleanup
set imgsuf=
set imgext=
set SCRIPT_FN=
goto end

:usage
echo  Lauterbach TRACE-32 Debug script generator/Launcher ver 1.2
echo.
echo  Usage:
echo     t32 APP_NAME [ CONFIG SWCONFIG ] [ no_run ]
echo.
echo  Where:
echo     APP_NAME - is the application to debug (ex. watchtv)
echo     CONFIG   - will default tot he value defined in your environment
echo                or can be overridden on the command line.  The parameter
echo                specifies the HWCONFIG to use.
echo     SWCONFIG - will default to CNXT or use the value defined in your 
echo                environment or can be overridden on the command line.
echo                If specified it must be specified along with HWCONFIG.
echo     no_run   - if specified, it must be the fourth parameter (ie. you must
echo                specify a CONFIG and SWCONFIG) and will stop the debugger 
echo                from launching automatically.
echo.
echo.
echo.
echo  In order for this batch file to function properly, the following
echo  environment variables must be set:
echo.
echo   SABINE_ROOT = path of your build tree
echo        CONFIG = which box you're building for.  The following values
echo                 (case sensitive) imply an ARM920 core on TAP2:
echo                    athens
echo                    sparta_na
echo                    sparta_euro_na
echo                    sparta_na_nocm
echo                    sparta_euro_na_nocm
echo          RTOS = which RTOS you're building for
echo       TRACE32 = directory where Lauterbach's TRACE32 is installed
echo.
echo  The first parameter must be the name of the app to debug
echo.

:end

