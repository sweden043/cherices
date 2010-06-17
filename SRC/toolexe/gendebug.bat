@echo  off
REM SET TRACE32=c:\t32
REM call C:\C1.3.1.b58.E\MPEG\src\toolexe\vxw_211_bld.bat > nul:
REM
REM 
REM ************************************************************  
REM *      Generic Portion of debug script generator           *
REM ************************************************************  
if NOT "%SABINE_ROOT%"=="" echo WARNING: The SABINE_ROOT environment variable will be discontinued
if "%CNXT_MPEGROOT%"=="" SET CNXT_MPEGROOT=%SABINE_ROOT%
if "%CNXT_MPEGROOT%"=="" goto usage
if "%RTOS%"==""     goto usage
if "%TRACE32%"==""  goto usage
if "%CONFIG%"==""   goto usage
IF "%1" == ""       goto usage
IF "%1" == "."      goto usage
IF "%1" == "/?"      goto usage
IF "%1" == "?"      goto usage
IF "%1" == "H"      goto usage
IF "%1" == "-H"     goto usage
IF "%1" == "h"      goto usage
IF "%1" == "/h"      goto usage
IF "%1" == "-h"     goto usage
IF "%1" == "HELP"   goto usage
IF "%1" == "help"   goto usage
IF "%1" == "-HELP"  goto usage
IF "%1" == "-help"  goto usage
IF "%1" == "--HELP" goto usage
IF "%1" == "--help" goto usage
if "%DEBUGVER%"=="" SET DEBUGVER=DEBUG
if "%LABEL%"==""    SET LABEL=USE

:: check for config in environment or use command line
if "%2"=="" goto determinehwconfig
set CONFIG=%2
goto havehwconfig
:determinehwconfig
if "%CONFIG%"=="" goto usage
:havehwconfig

:: check for swconfig in environment or use command line
if "%3"=="" goto determineswconfig
set SWCONFIG=%3
goto haveswconfig
:determineswconfig
if "%SWCONFIG%"=="" set SWCONFIG=CNXT
echo WARNING: SWCONFIG undefined.  Using default of CNXT.
echo.
:haveswconfig

:: jump to debugger specific portion of batch file
if "%CNXT_MPEG_DEF_DBG%"=="ADW" goto genadw
if "%CNXT_MPEG_DEF_DBG%"=="adw" goto genadw
if "%CNXT_MPEG_DEF_DBG%"=="AXD" goto genaxd
if "%CNXT_MPEG_DEF_DBG%"=="axd" goto genaxd
if "%CNXT_MPEG_DEF_DBG%"=="T32" goto gent32
if "%CNXT_MPEG_DEF_DBG%"=="t32" goto gent32
if "%CNXT_MPEG_DEF_DBG%"=="TOR" goto gentor
if "%CNXT_MPEG_DEF_DBG%"=="tor" goto gentor
goto usage


REM ************************************************************  
REM *      AXD     Portion of debug script generator           *
REM ************************************************************  
:genaxd
set SCRIPT_FN=%1.scr
set IMGSUF=_ram
set IMGEXT=axf
ECHO let $top_of_memory 0x800000            > %SCRIPT_FN%
ECHO let $arm9_restart_code_address 0x80    >> %SCRIPT_FN%
ECHO let $cp_access_code_address 0x50       >> %SCRIPT_FN%
ECHO let $safe_non_vector_address 0x7ffc00  >> %SCRIPT_FN%
ECHO let $target_fpu 2                      >> %SCRIPT_FN%
ECHO spp semihosting_enabled 0              >> %SCRIPT_FN%

set RTOSPREF=%RTOS%
if "%RTOS%"=="VXWORKS" set RTOSPREF=VX

::
:: This needs to be kept within Win98 command line length limits
SET DBGDIRS=%CNXT_MPEGROOT%\%RTOSPREF%BSP;%CNXT_MPEGROOT%\%RTOSPREF%KAL;%CNXT_MPEGROOT%\STARTUP;%CNXT_MPEGROOT%\HWLIB;%CNXT_MPEGROOT%\DEMUX;%CNXT_MPEGROOT%\%1
::
::
echo ssd "%DBGDIRS%"                   >> %SCRIPT_FN%
echo ssd "%CNXT_MPEGROOT%\AUD_COMN" #3 >> %SCRIPT_FN%
echo ssd "%CNXT_MPEGROOT%\DEMOD" #3    >> %SCRIPT_FN%
echo ssd "%CNXT_MPEGROOT%\GENIR" #3    >> %SCRIPT_FN%
echo ssd "%CNXT_MPEGROOT%\HSDP" #3     >> %SCRIPT_FN%
echo ssd "%CNXT_MPEGROOT%\MIPIDLE" #3  >> %SCRIPT_FN%
echo ssd "%CNXT_MPEGROOT%\VIDEO" #3    >> %SCRIPT_FN%

if NOT "%RTOS%"=="VXWORKS" goto axdnotvxw
SET IMGSUF=_ram_vx
SET IMGEXT=bin
::
:: This value shouldn't be hardcoded but CMD.EXE/COMMNAD.COM doesn't have a way 
:: to extract this value from the config file.
SET LOADADDR=0x23c800
::
::
echo com *************************************************************** >> %SCRIPT_FN%
echo com                                                                 >> %SCRIPT_FN%
echo com If you are running VxWorks 5.5/Tornado 2.2 and would like to do >> %SCRIPT_FN%
echo com source level debugging, set the DEBUG_FLAG environment variable >> %SCRIPT_FN%
echo com to -gdwarf, clean, and rebuid                                   >> %SCRIPT_FN%
echo com                                                                 >> %SCRIPT_FN%
echo com *************************************************************** >> %SCRIPT_FN%
echo lb %CNXT_MPEGROOT%\%1\%CONFIG%\%1\%RTOS%\%SWCONFIG%\%DEBUGVER%\%LABEL%\%1%IMGSUF%.%IMGEXT%,%LOADADDR%  >> %SCRIPT_FN%
goto launchaxd


:axdnotvxw
echo load %CNXT_MPEGROOT%\%1\%CONFIG%\%1\%RTOS%\%SWCONFIG%\%DEBUGVER%\%LABEL%\%1%IMGSUF%.%IMGEXT% >> %SCRIPT_FN%
goto launchaxd



REM ************************************************************  
REM *      ADW     Portion of debug script generator           *
REM ************************************************************  
:genadw
set SCRIPT_FN=%1.scr
set IMGSUF=_ram
set IMGEXT=axf
echo $echo=0                             > %SCRIPT_FN%
echo $top_of_memory=0x800000            >> %SCRIPT_FN%
echo $arm9_restart_code_address=0x80    >> %SCRIPT_FN%
echo $cp_access_code_address=0x50       >> %SCRIPT_FN%
echo $safe_non_vector_address=0x7ffc00  >> %SCRIPT_FN%
echo $semihosting_enabled=0             >> %SCRIPT_FN%

set RTOSPREF=%RTOS%
if "%RTOS%"=="VXWORKS" set RTOSPREF=VX

::
:: This needs to be kept within Win98 command line length limits
SET DBGDIRS=%CNXT_MPEGROOT%\%RTOSPREF%BSP;%CNXT_MPEGROOT%\%RTOSPREF%KAL;%CNXT_MPEGROOT%\STARTUP;%CNXT_MPEGROOT%\HWLIB;%CNXT_MPEGROOT%\DEMUX;%CNXT_MPEGROOT%\%1
::
::

if NOT %CMDEXTVERSION%==2 goto short_shell
:: Win98 can't handle environment variables this large
SET DBGDIRS=%DBGDIRS%;%CNXT_MPEGROOT%\AUD_COMN
SET DBGDIRS=%DBGDIRS%;%CNXT_MPEGROOT%\DEMOD
SET DBGDIRS=%DBGDIRS%;%CNXT_MPEGROOT%\GENIR
SET DBGDIRS=%DBGDIRS%;%CNXT_MPEGROOT%\HSDP
SET DBGDIRS=%DBGDIRS%;%CNXT_MPEGROOT%\MIPIDLE
SET DBGDIRS=%DBGDIRS%;%CNXT_MPEGROOT%\VIDEO
:short_shell

echo $sourcedir="%DBGDIRS%" >> %SCRIPT_FN%
::
:: A little magic to change backslash to double-backslash (assuming
:: the debug search path is the only thing in the file with backslashes)
ren %SCRIPT_FN% %SCRIPT_FN%.old
c:\cnxtmpeg\sed -e s/\\/\\\\/g < %SCRIPT_FN%.old > %SCRIPT_FN%
del -f %SCRIPT_FN%.old > nul:
::

if NOT "%RTOS%"=="VXWORKS" goto adwnotvxw
SET IMGSUF=_ram_vx
SET IMGEXT=bin
::
:: This value shouldn't be hardcoded but CMD.EXE/COMMNAD.COM doesn't have a way 
:: to extract this value from the config file.
SET LOADADDR=0x23c800
::
::
echo com *************************************************************** >> %SCRIPT_FN%
echo com                                                                 >> %SCRIPT_FN%
echo com If you are running VxWorks 5.5/Tornado 2.2 and would like to do >> %SCRIPT_FN%
echo com source level debugging, set the DEBUG_FLAG environment variable >> %SCRIPT_FN%
echo com to -gdwarf, clean, and rebuid                                   >> %SCRIPT_FN%
echo com                                                                 >> %SCRIPT_FN%
echo com *************************************************************** >> %SCRIPT_FN%
echo get %CNXT_MPEGROOT%\%1\%CONFIG%\%1\%RTOS%\%SWCONFIG%\%DEBUGVER%\%LABEL%\%1%IMGSUF%.%IMGEXT% %LOADADDR%  >> %SCRIPT_FN%
goto launchadw
:adwnotvxw

echo load %CNXT_MPEGROOT%\%1\%CONFIG%\%1\%RTOS%\%SWCONFIG%\%DEBUGVER%\%LABEL%\%1%IMGSUF%.%IMGEXT% >> %SCRIPT_FN%
goto launchadw


REM ************************************************************  
REM *      TRACE32 Portion of debug script generator           *
REM ************************************************************  
:gent32

if EXIST %TRACE32%\config.bak goto nopatchconfig
echo Backing up %TRACE32%\config.t32 to %TRACE32%\config.bak
echo Grabbing latest config.t32 from %CNXT_MPEGROOT%\TOOLEXE
move %TRACE32%\config.t32 %TRACE32%\config.bak > nul:
copy %sabine_root%\toolexe\config.t32 %TRACE32% > nul:
:nopatchconfig

if EXIST %TRACE32%\t32.bak goto nopatcht32cmm
echo Backing up %TRACE32%\t32.cmm to %TRACE32%\t32.bak
echo Grabbing latest t32.cmm from %CNXT_MPEGROOT%\TOOLEXE
move %TRACE32%\t32.cmm %TRACE32%\t32.bak > nul:
copy %sabine_root%\toolexe\t32.cmm %TRACE32% > nul:
:nopatcht32cmm

:prepare
set SCRIPT_FN=%1.cmm
set IMGSUF=_ram
set IMGEXT=axf

if EXIST %SCRIPT_FN% del %SCRIPT_FN%
REM 
echo. > %SCRIPT_FN%

::  Errorlevel 1 = "errorlevel >= 1" 
grep -iq CHIP_NAME %CNXT_MPEGROOT%\configs\%CONFIG%.CFG
if ERRORLEVEL 1 goto nochipname

grep -i CHIP_NAME %CNXT_MPEGROOT%\configs\%CONFIG%.CFG | grep -iq WABASH
if NOT ERRORLEVEL 1 goto iswabash

grep -i CHIP_NAME %CNXT_MPEGROOT%\configs\%CONFIG%.CFG | grep -iq HONDO
if NOT ERRORLEVEL 1 goto ishondo

grep -i CHIP_NAME %CNXT_MPEGROOT%\configs\%CONFIG%.CFG | grep -iq COLORADO
if NOT ERRORLEVEL 1 goto iscolorado

grep -i CHIP_NAME %CNXT_MPEGROOT%\configs\%CONFIG%.CFG | grep -iq BRAZOS
if NOT ERRORLEVEL 1 goto isbrazos

:nochipname
echo ERROR: Could not determine chip type from config file:
echo    %CNXT_MPEGROOT%\config\%CONFIG%.CFG
goto end

:isbrazos
echo  b::system.cpu arm940t                   >> %SCRIPT_FN%
echo  b::system.multicore irpost 4            >> %SCRIPT_FN%
echo  b::system.multicore drpost 1            >> %SCRIPT_FN%
goto main

:iscolorado
:ishondo
echo  b::system.cpu arm940t                   >> %SCRIPT_FN%
echo  b::system.multicore irpost 0            >> %SCRIPT_FN%
echo  b::system.multicore drpost 0            >> %SCRIPT_FN%
goto main

:iswabash
echo  b::sys.cpu arm920t                   >> %SCRIPT_FN%
echo  b::sys.multicore irpost 8            >> %SCRIPT_FN%
echo  b::sys.multicore drpost 2            >> %SCRIPT_FN%
echo  b::system.JTAGCLOCK 10M              >> %SCRIPT_FN%
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

echo  b::symbol.spath + %CNXT_MPEGROOT%\VXBSP          >> %SCRIPT_FN%
echo  b::symbol.spath + %CNXT_MPEGROOT%\VXKAL          >> %SCRIPT_FN%
echo  b::symbol.spath + %CNXT_MPEGROOT%\PSOSBSP        >> %SCRIPT_FN%
echo  b::symbol.spath + %CNXT_MPEGROOT%\PSOSKAL        >> %SCRIPT_FN%
echo  b::symbol.spath + %CNXT_MPEGROOT%\NUPBSP         >> %SCRIPT_FN%
echo  b::symbol.spath + %CNXT_MPEGROOT%\NUPKAL         >> %SCRIPT_FN%
echo  b::symbol.spath + %CNXT_MPEGROOT%\STARTUP        >> %SCRIPT_FN% 
echo  b::symbol.spath + %CNXT_MPEGROOT%\HWLIB          >> %SCRIPT_FN% 
echo  b::symbol.spath + %CNXT_MPEGROOT%\WATCHTV        >> %SCRIPT_FN% 
echo  b::symbol.spath + %CNXT_MPEGROOT%\DEMUX          >> %SCRIPT_FN% 
echo  b::symbol.spath + %CNXT_MPEGROOT%\DEMOD          >> %SCRIPT_FN% 
echo  b::symbol.spath + %CNXT_MPEGROOT%\DEMOD_CX24430  >> %SCRIPT_FN% 
echo  b::symbol.spath + %CNXT_MPEGROOT%\VIDEO          >> %SCRIPT_FN% 
echo  b::symbol.spath + %CNXT_MPEGROOT%\AUDCOMN        >> %SCRIPT_FN% 
echo  b::symbol.spath + %CNXT_MPEGROOT%\IIC            >> %SCRIPT_FN% 
echo  b::symbol.spath + %CNXT_MPEGROOT%\FLASH          >> %SCRIPT_FN% 

if NOT "%RTOS%"=="VXWORKS" goto t32notvxw
SET IMGSUF=
if "%ARM_VERSION%"=="211" set IMGEXT=cof
if "%ARM_VERSION%"=="220" set IMGEXT=elf
:t32notvxw


echo  b::d.load %sabine_root%\%1\%config%\%1\%rtos%\%SWCONFIG%\debug\use\%1%IMGSUF%.%IMGEXT%   >> %SCRIPT_FN%
if "%RTOS%"=="VXWORKS" if "%ARM_VERSION%"=="211" echo  b::r.s PC _sysInit >> %SCRIPT_FN%
REM   Show source code
echo  b::data.list                                                        >> %SCRIPT_FN%
REM   Show registers
echo  b::R                                                                >> %SCRIPT_FN%
echo  b::PRINT "READY!"                                                   >> %SCRIPT_FN%
echo  enddo                                                               >> %SCRIPT_FN%


REM ********************************************************************** 
REM     Scripts are generated, launch the debuggers, clean up, exit...
REM ********************************************************************** 
:launcht32
if "%4" == "no_run" echo Generating %SCRIPT_FN% but not launching T32...
if "%4" == "no_run" goto cleanup
start %TRACE32%\t32marm.exe -c %TRACE32%\config.t32 %SCRIPT_FN%
goto cleanup

:launchadw
if "%4" == "no_run" echo Generating %SCRIPT_FN% but not launching ADW...
if "%4" == "no_run" goto cleanup
start adw -script %SCRIPT_FN%
goto cleanup

:launchaxd
if "%4" == "no_run" echo Generating %SCRIPT_FN% but not launching ADW...
if "%4" == "no_run" goto cleanup
start axd -script %SCRIPT_FN%
goto cleanup

:cleanup
set IMGSUF=
set IMGEXT=
set SCRIPT_FN=
set LOADADDR=
set RTOSPREF=
set DBGDIR=
goto end

:usage
cls
echo  Multi-Debugger Script Generator/Launcher  v1.3
echo    (current support=ADW, AXD, Tornado, Trace32)
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
echo  CNXT_MPEGROOT = path of your build tree
echo         CONFIG = which box you're building/debugging 
echo           RTOS = which RTOS you're building/debugging
echo        TRACE32 = if using TRACE32, the directory where Lauterbach's 
echo                  TRACE32 is installed.
echo       DEBUGVER = Override the default of DEBUG.  Valid values are:
echo                    DEBUG (default)
echo                    RELEASE 
echo   CNXT_MPEG_DEF_DBG = Default debugger.  Valid values are:
echo                        ADW
echo                        AXD
echo                        T32
echo                        TOR
echo.
echo  The first parameter must be the name of the app to debug
echo.

:end

