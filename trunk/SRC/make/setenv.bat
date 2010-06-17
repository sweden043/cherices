@echo off
goto skipusage
:USAGE
echo setenv rtos config swconfig toolset [project] [setup [app] ]
echo     Where rtos is:
echo              vxw
echo              psos
echo              psos25
echo              nup
echo              ucos
echo              noos
echo.
echo     config is:
echo              [sparta : milano]_[na : euro]
echo              klon[dike : mdm : mdp : scm]
echo              bronco[_vxwa : _vxwb : mdm ]
echo.
echo     swconfig is:
echo              gen
echo              .....
echo.
echo     toolset is:
echo              ADS
echo              SDT
echo              WRGCC
echo              GNUGCC
echo.
echo     project if specified, overrides the default "current-date"
echo        subdirectory directory (x:\dir\mm.dd.yyyy)
echo.
echo     setup forces a new build tree to be created in the default 
echo        project directory.  Unless overridden, the current date is
echo        used as the default project subdirectory.
echo.
echo     specifying an app causes a GET of that app.
echo.
goto realend

:skipusage

if "%1" == "" goto USAGE
if "%2" == "" goto USAGE
if "%3" == "" goto USAGE
if "%4" == "" goto USAGE

set RTOS=
if /I "%1" == "vxw"      set RTOS=VXWORKS
if /I "%1" == "nup"      set RTOS=NUP
if /I "%1" == "psos"     set RTOS=PSOS
if /I "%1" == "psos25"   set RTOS=PSOS25
if /I "%1" == "ucos"     set RTOS=UCOS
if /I "%1" == "noos"     set RTOS=NOOS

if "%RTOS%" == "" goto USAGE

set CONFIG=%2
set DEFAULT_CONFIG=%2
set SWCONFIG=%3

if /I "%4" == "ADS"      set ARM_TOOLKIT=ADS
if /I "%4" == "SDT"      set ARM_TOOLKIT=SDT
if /I "%4" == "WRGCC"    set ARM_TOOLKIT=WRGCC
if /I "%4" == "GNUGCC"   set ARM_TOOLKIT=GNUGCC

shift
shift
shift
shift

REM The following variables control the location of your build tree
REM \/ \/ \/  NEEDS REVIEWING !!!
set WORK_DRIVE=c:
REM \/ \/ \/  NEEDS REVIEWING !!!
set PROJECT=%WORK_DRIVE%\project
REM \/ \/ \/  NEEDS REVIEWING !!!
set ENVDATE=%DATE:~4,10%
REM \/ \/ \/  NEEDS REVIEWING !!!
set ENVDATE=%ENVDATE:/=.%

REM Machine and user specific settings
set PROMPT=$_$C$M$F$+$_$P$G
REM \/ \/ \/  NEEDS REVIEWING !!!
set HOME=c:\home\bintzmf
REM \/ \/ \/  NEEDS REVIEWING !!!
set VCSID=bintzmf

set PVCS_ROOT=k:\sabine\pvcs
REM \/ \/ \/  NEEDS REVIEWING !!!
set TRACE32=c:\t32

REM \/ \/ \/  NEEDS REVIEWING !!!
REM (oops, need to add back support for other version of psos)
set PSS_ROOT=c:\isiarm\pssarm.230
set PSS25_ROOT=c:\isiarm\pssarm.251

REM \/ \/ \/  NEEDS REVIEWING !!!
set SDT_PATH=c:\isiarm\arm250
set SDT_BIN_PATH=%SDT_PATH%\bin
set SDT_LIB_PATH=%SDT_PATH%\lib\embedded
set SDT_INC=%SDT_PATH%\include

REM \/ \/ \/  NEEDS REVIEWING !!!
set ADS_PATH=c:\progra~1\arm\adsv1_2
set ADS_BIN_PATH=%ADS_PATH%\bin
set ADS_LIB_PATH=%ADS_PATH%\lib
set ADS_INC=%ADS_PATH%\include

REM \/ \/ \/  NEEDS REVIEWING !!!
set WIND_BASE=c:\tornado220
set WIND_HOST_TYPE=x86-win32
set WIND_ARCH=ARM
set WR_BIN_PATH=%WIND_BASE%\HOST\%WIND_HOST_TYPE%\bin


REM Build constants
set LM_LIC_DOMAIN=austin.conexant.com

set LM_LICENSE_FILE=8228@pia.%LM_LIC_DOMAIN%;8228@pombe.%LM_LIC_DOMAIN%;8228@bir.%LM_LIC_DOMAIN%;8224@pia.%LM_LIC_DOMAIN%;8224@pombe;8224@bir.%LM_LIC_DOMAIN% 

REM \/ \/ \/  NEEDS REVIEWING !!!
set INTERNAL_BUILD=YES
REM \/ \/ \/  NEEDS REVIEWING !!!
set CNXT_MPEG_DEF_DBG=T32
set CPU=ARMARCH4
REM \/ \/ \/  NEEDS REVIEWING !!!
set SABINE_ROOT=%PROJECT%\%envdate%
REM \/ \/ \/  NEEDS REVIEWING !!!
set SABINE_SRCDIR=%SABINE_ROOT:\=\\%
set CONFIG_ROOT=%SABINE_ROOT%\configs

REM NOTE! The path to the wind-river compiler is in the path.  This MUST be
REM present or else it doesn't run properly (calling the application with the
REM entire path on the command line is NOT sufficient)
REM \/ \/ \/  NEEDS REVIEWING !!!
set PATH=%WR_BIN_PATH%;%SystemRoot%\system32;%SystemRoot%;%SystemRoot%\System32\Wbem;c:\t32;
REM The following is specific to miles' machine, but some of this may
REM apply to yours as well.
set PATH=%PATH%;c:\tools2k;c:\tools2k\rem;c:\tools2k\xvi;%WR_BIN_PATH%;c:\progra~1\winzip;c:\progra~1\arm\multi-ice;c:\progra~1\vim\vim60;c:\t32;c:\cygwin\bin;
REM PVCS tools...
set PATH=%PATH%;p:\nt

goto %ARM_TOOLKIT%

:SDT
SET ARM_VERSION=251
goto TOOLSDONE

:ADS
SET ARM_VERSION=12
goto TOOLSDONE

:WRGCC
set ARM_VERSION=296
goto TOOLSDONE

:TOOLSDONE

if "%1" == "" goto end
if /I "%1" == "setup" goto setup

:proj_override
set SABINE_ROOT=%PROJECT%\%1
set SABINE_SRCDIR=%SABINE_ROOT:\=\\%
set CONFIG_ROOT=%SABINE_ROOT%\configs
set PATH=%SABINE_ROOT%\TOOLEXE;%PATH%
shift

:setup
if /I "%1"=="setup" goto setup2
goto end

:setup2
getincl=YES

%WORK_DRIVE%
cd %PROJECT%
mkdir %SABINE_ROOT%
cd %SABINE_ROOT%
if exist include set getincl=NO
mkdir make
cd make
echo vcsdir=k:\sabine\pvcs\make > vcs.cfg
get -vUSE *.??v
cd ..
if not exist configs (
    mkdir configs
    cd configs
    echo vcsdir=k:\sabine\pvcs\configs > vcs.cfg
    get -vUSE *.cfv
    cd ..
)
if not exist toolexe (
        mkdir toolexe
    cd toolexe
    echo vcsdir=k:\sabine\pvcs\toolexe > vcs.cfg
    get -vUSE *.??v
    cd ..
)
if "%2" == "" goto end
mkdir %2
cd %2
echo vcsdir=k:\sabine\pvcs\%2 > vcs.cfg
get makefile
nmake TYPE=GET GET_INCLUDE=%GETINCL%

:end
cd /d %SABINE_ROOT%
color 1e
title RTOS=%RTOS% CFG=%CONFIG% SWCFG=%SWCONFIG% TKIT=%ARM_TOOLKIT% SAB_RT=%SABINE_ROOT%

:realend

