:: @echo off


REM **************************************************************************
REM **************************************************************************
REM                   BEGIN PHASE 1   
REM **************************************************************************
REM **************************************************************************
:machine_specific_SETup_complete

REM      ****************************************************************     
REM         SET up default build parameters (these are neither machine
REM         nor install dependent)
REM      ****************************************************************     
SET        cnxt_mpeg_EXTRAPATH=
SET        cnxt_mpeg_LOG=YES
SET        cnxt_mpeg_INTERNAL_BUILD=NO
SET        cnxt_mpeg_VERBOSE=NO
SET        cnxt_mpeg_FLASHDIR= %CNXT_MPEGROOT%\TOOLEXE 

REM      ****************************************************************     
REM         The OVERRIDE variables let a user run their "custom" scripts
REM         to SET up their custom environments the way they are used to.
REM      ****************************************************************     
if NOT "%cnxt_OVERRIDE_WORKDRIVE%"      == ""  SET cnxt_mpeg_WORKDRIVE=%cnxt_OVERRIDE_WORKDRIVE%
if NOT "%cnxt_OVERRIDE_PROJECT1%"       == ""  SET cnxt_mpeg_PROJECT1=%cnxt_OVERRIDE_PROJECT1%
if NOT "%cnxt_OVERRIDE_PROJECT2%"       == ""  SET cnxt_mpeg_PROJECT2=%cnxt_OVERRIDE_PROJECT2%
if NOT "%cnxt_OVERRIDE_RTOS%"           == ""  SET cnxt_mpeg_RTOS=%cnxt_OVERRIDE_RTOS%
if NOT "%cnxt_OVERRIDE_CONFIG%"         == ""  SET cnxt_mpeg_CONFIG=%cnxt_OVERRIDE_CONFIG%
if NOT "%cnxt_OVERRIDE_SWCONFIG%"       == ""  SET cnxt_mpeg_SWCONFIG=%cnxt_OVERRIDE_SWCONFIG%
if NOT "%cnxt_OVERRIDE_INTERNAL_BUILD%" == ""  SET cnxt_mpeg_INTERNAL_BUILD=%cnxt_OVERRIDE_INTERNAL_BUILD%
if NOT "%cnxt_OVERRIDE_LOG%"            == ""  SET cnxt_mpeg_LOG=%cnxt_OVERRIDE_LOG%
if NOT "%cnxt_OVERRIDE_FLASHDIR%"       == ""  SET cnxt_mpeg_FLASHDIR=%cnxt_OVERRIDE_FLASHDIR%
if NOT "%cnxt_OVERRIDE_TOOLKIT%"        == ""  SET cnxt_mpeg_TOOLKIT=%cnxt_OVERRIDE_TOOLKIT%
if NOT "%cnxt_OVERRIDE_TOOLVER%"        == ""  SET cnxt_mpeg_TOOLVER=%cnxt_OVERRIDE_TOOLVER%
REM  WINDBASE can be overriden to support switching between various versions of Tornado
if NOT  "%cnxt_OVERRIDE_WINDBASE%"      == "" SET cnxt_mpeg_WINDBASE        = %cnxt_OVERRIDE_WINDBASE%


REM      ****************************************************************     
REM         Check the environment.  Everything should be defined at
REM         this point
REM      ****************************************************************     

SET error_param=
if "%cnxt_mpeg_WORKDRIVE%"      == "" SET error_param=workdrive
if "%cnxt_mpeg_PROJECT1%"       == "" SET error_param=project1 %error_param%
if "%cnxt_mpeg_PROJECT2%"       == "" SET error_param=project2 %error_param%
if "%cnxt_mpeg_CONFIG%"         == "" SET error_param=config %error_param%
if "%cnxt_mpeg_SWCONFIG%"       == "" SET error_param=swconfig %error_param%
if "%cnxt_mpeg_INTERNAL_BUILD%" == "" SET error_param=internal_build %error_param%
if "%cnxt_mpeg_LOG%"            == "" SET error_param=log %error_param%
if "%cnxt_mpeg_FLASHDIR%"       == "" SET error_param=flashdir %error_param%
if "%cnxt_mpeg_TOOLKIT%"        == "" SET error_param=toolkit %error_param%
if "%cnxt_mpeg_TOOLVER%"        == "" SET error_param=toolver %error_param%
if "%cnxt_mpeg_WINDBASE%"       == "" SET error_param=windbase %error_param%
if "%cnxt_mpeg_VERBOSE%"        == "" SET error_param=verbose %error_param%

if "%error_param%" == "" goto finalize_environment

@echo ************************************************************ 
@echo   ERROR: The following environment variables were blank:  
@echo   %error_param%
@echo   see mk_mach_env.bat and mkenv.bat to troubleshoot...
@echo ************************************************************ 
SET error_param=
goto exit_error







REM **************************************************************************
REM **************************************************************************
REM                   BEGIN PHASE 2   
REM **************************************************************************
REM **************************************************************************
:finalize_environment

REM      ****************************************************************     
REM        SET up variables that are neither RTOS or toolchain specific
REM        nor install dependent)
REM      ****************************************************************     
REM      NOTE THE TRAILING BACKSLASH!
SET        cnxt_MPEGINSTALL_DIR=%cnxt_WORKDRIVE%\%cnxt_PROJECT1%\%cnxt_PROJECT2%\
SET        CNXT_MPEGROOT= %cnxt_WORKDRIVE%\%cnxt_PROJECT1%\%cnxt_PROJECT2%
SET        CONFIG=%cnxt_mpeg_CONFIG%
SET        SWCONFIG=%cnxt_mpeg_SWCONFIG%
REM         SABINE_ROOT is depricated
SET        SABINE_ROOT=%CNXT_MPEGROOT%
SET        CNXT_MPEG_SOURCESEARCH1=%cnxt_WORKDRIVE%\\%cnxt_PROJECT1%\\%cnxt_PROJECT2%
SET        CNXT_MPEG_SOURCESEARCH2=%CNXT_MPEGROOT%
SET        PATH=%CNXT_MPEGROOT%\toolexe;%path%

REM      ****************************************************************     
REM         SETup environment variables that define the toolkit
REM      ****************************************************************     
:toolkit_specific_SETup

if "%cnxt_mpeg_TOOLKIT%"=="ADS" goto ads_SETup
if "%cnxt_mpeg_TOOLKIT%"=="ads" goto ads_SETup
if "%cnxt_mpeg_TOOLKIT%"=="SDT" goto sdt_SETup
if "%cnxt_mpeg_TOOLKIT%"=="sdt" goto sdt_SETup
if "%cnxt_mpeg_TOOLKIT%"=="VXW" goto vxw_SETup
if "%cnxt_mpeg_TOOLKIT%"=="vxw" goto vxw_SETup
@echo ************************************************************ 
@echo   ERROR: Unrecognized TOOLKIT.  Use all caps or all lower
@echo   when specifying
@echo ************************************************************ 
goto exit_error

:ads_SETup
SET ARM_VERSION=12
SET ARM_TOOLKIT=ADS
SET ARMCONF=%cnxt_mpeg_ARMADSDIR%\bin
SET ARMDLL=%cnxt_mpeg_ARMADSDIR%\bin
SET ARMHOME=%cnxt_mpeg_ARMADSDIR%
SET ARMINC=%cnxt_mpeg_ARMADSDIR%\include
SET ARMLIB=%cnxt_mpeg_ARMADSDIR%\lib
SET SEMIHOST_LIB=%cnxt_mpeg_ARMADSDIR%\lib
SET PATH=%cnxt_mpeg_ARMADSDIR%\bin;%path%
REM The following line only works on WinNT and greater
REM SET CNXT_MPEGROOT_SRC=%CNXT_MPEGROOT:\=\\%
goto toolkit_SETup_done

:sdt_SETup
SET ARM_VERSION=251
SET ARM_TOOLKIT=SDT
SET ARMCONF=%cnxt_mpeg_ARMSDTDIR%\bin
SET ARMDLL=%cnxt_mpeg_ARMSDTDIR%\bin
SET ARMHOME=%cnxt_mpeg_ARMSDTDIR%
SET ARMINC=%cnxt_mpeg_ARMSDTDIR%\include
SET ARMLIB=%cnxt_mpeg_ARMSDTDIR%\lib\embedded
SET SEMIHOST_LIB=%cnxt_mpeg_ARMSDTDIR%\lib
SET PATH=%cnxt_mpeg_ARMSDTDIR%\bin;%path%
REM The following line only works on WinNT and greater
REM SET CNXT_MPEGROOT_SRC=%CNXT_MPEGROOT:\=\\%
goto toolkit_SETup_done

:vxw_SETup
SET ARM_TOOLKIT=VXW
SET ARM_VERSION=%cnxt_mpeg_TOOLVER% 
SET WIND_BASE=cnxt_mpeg_WINDBASE
SET BIN_DIR=%WIND_BASE%\host\x86-win32\bin
goto toolkit_SETup_done

:toolkit_SETup_done
goto rtos_specific_SETup

REM      ****************************************************************     
REM         SETup environment variables that define the RTOS
REM      ****************************************************************     
:rtos_specific_SETup

REM These paths should always be first!
SET INCLUDE=.;..\INCLUDE;%CNXT_MPEGROOT%\INCLUDE;%CNXT_MPEGROOT%\OTVINCL
SET LIB=%CNXT_MPEGROOT%\lib

if "%cnxt_mpeg_RTOS%" == "PSOS"      goto PSOS
if "%cnxt_mpeg_RTOS%" == "psos"      goto PSOS
if "%cnxt_mpeg_RTOS%" == "NUP"       goto NUP
if "%cnxt_mpeg_RTOS%" == "nup"       goto NUP
if "%cnxt_mpeg_RTOS%" == "ucos"      goto ucos
if "%cnxt_mpeg_RTOS%" == "UCOS"      goto UCOS
if "%cnxt_mpeg_RTOS%" == "VXWORKS"   goto VXWORKS
if "%cnxt_mpeg_RTOS%" == "VXW"       goto VXWORKS
if "%cnxt_mpeg_RTOS%" == "vxworks"   goto VXWORKS
if "%cnxt_mpeg_RTOS%" == "vxw"       goto VXWORKS

@echo ************************************************************ 
@echo   ERROR: Unrecognized RTOS.  Use all caps or all lower
@echo   when specifying
@echo ************************************************************ 
goto exit_error

:PSOS
SET PSS_ROOT=%cnxt_mpeg_PSSROOT%
SET PSS_BSP=%CNXT_MPEGROOT%\psosbsp
SET BSP_TYPE=32l
SET INCLUDE=%INCLUDE%;%CNXT_MPEGROOT%\psosincl;%CNXT_MPEGROOT%\psosbsp
SET INCLUDE=%INCLUDE%;%PSS_ROOT%\include;%PSS_ROOT%\include\sys
SET LIB=%PSS_ROOT%\sys\os;%PSS_ROOT%\sys\libc;%LIB%
goto rtos_SETup_done

:NUP
SET INCLUDE=%CNXT_MPEGROOT%\nupbsp;%CNXT_MPEGROOT%\nupincl
SET INCLUDE=%INCLUDE%;%ARMINC%
goto rtos_SETup_done


:VXWORKS
SET CPU=ARMARCH4
SET WIND_HOST_TYPE=x86-win32
SET PATH=%BIN_DIR%;%PATH%
SET WIND_ARCH=arm
SET INCLUDE=%INCLUDE%;%CNXT_MPEGROOT%\hwlib;%CNXT_MPEGROOT%\vxincl
SET INCLUDE=%INCLUDE%;%SABINE_ROOT%\vxbsp;%SABINE_ROOT%\vxkal
SET INCLUDE=%INCLUDE%;%WIND_BASE%\TARGET
SET INCLUDE=%INCLUDE%;%WIND_BASE%\TARGET\H\private;
SET INCLUDE=%INCLUDE%;%WIND_BASE%\TARGET\H;%WIND_BASE%\TARGET\H\RPC
SET INCLUDE=%INCLUDE%;%WIND_BASE%\TARGET\CONFIG\ALL;%WIND_BASE%\TARGET\H\DRV\NETIF
SET INCLUDE=%INCLUDE%;%WIND_BASE%\TARGET\SRC\CONFIG;%WIND_BASE%\TARGET\SRC\DRV
SET INCLUDE=%INCLUDE%;%WIND_BASE%\TARGET\H\ARCH\%WIND_ARCH%;
SET INCLUDE=%INCLUDE%;%WIND_BASE%\HOST\x86-win32\lib\gcc-lib\arm-wrs-vxworks\2.9-010413\include
goto rtos_SETup_done

:rtos_SETup_done

REM      ****************************************************************     
REM         SET environment to use remaining tools
REM      ****************************************************************     

SET PATH=c:\cnxtmpeg;%cnxt_mpeg_NMAKE%;%path%;%cnxt_mpeg_T32%;%cnxt_mpeg_MICEDIR%
SET T32=%cnxt_mpeg_T32%

goto exit_clean






REM **************************************************************************
REM **************************************************************************
REM                   BEGIN PHASE 3 - script cleanup   
REM **************************************************************************
REM **************************************************************************
:exit_clean
@echo Script completed successfully!
goto clean_house

:exit_error
@echo Error encountered, aborintg

:clean_house
if NOT "%MKENV_DEBUG%"==1  SET  cnxt_mpeg_WORKDRIVE=
if NOT "%MKENV_DEBUG%"==1  SET  cnxt_mpeg_PROJECT1=
if NOT "%MKENV_DEBUG%"==1  SET  cnxt_mpeg_PROJECT2=
if NOT "%MKENV_DEBUG%"==1  SET  cnxt_mpeg_RTOS=
if NOT "%MKENV_DEBUG%"==1  SET  cnxt_mpeg_CONFIG=
if NOT "%MKENV_DEBUG%"==1  SET  cnxt_mpeg_SWCONFIG=
if NOT "%MKENV_DEBUG%"==1  SET  cnxt_mpeg_INTERNAL_BUILD=
if NOT "%MKENV_DEBUG%"==1  SET  cnxt_mpeg_WINDBASE=
if NOT "%MKENV_DEBUG%"==1  SET  cnxt_mpeg_TOOLKIT=
if NOT "%MKENV_DEBUG%"==1  SET  cnxt_mpeg_TOOLVER=
if NOT "%MKENV_DEBUG%"==1  SET  cnxt_mpeg_MULTIICE=
if NOT "%MKENV_DEBUG%"==1  SET  cnxt_mpeg_MICEDIR=
if NOT "%MKENV_DEBUG%"==1  SET  cnxt_mpeg_PRIMDEBUG=
if NOT "%MKENV_DEBUG%"==1  SET  cnxt_mpeg_NMAKE=
if NOT "%MKENV_DEBUG%"==1  SET  cnxt_mpeg_ARMSDTDIR=
if NOT "%MKENV_DEBUG%"==1  SET  cnxt_mpeg_ARMADSDIR=
if NOT "%MKENV_DEBUG%"==1  SET  cnxt_mpeg_PSSROOT=
if NOT "%MKENV_DEBUG%"==1  SET  cnxt_mpeg_T32=


exit /b

