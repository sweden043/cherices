@echo off
REM
REM WARNING:  THIS BATCH FILE IS AUTOMATICALLY GENERATED AT INSTALL TIME!
REM IF YOU CHANGE ANY VALUES IN THIS BATCH FILE, THOSE CHANGES COULD BE
REM CLOBBERED IF YOU REINSTALL!
REM 

set cnxt_mpeg_WORKDRIVE=C:
set cnxt_mpeg_PROJECT1=project
set cnxt_mpeg_PROJECT2=\.
set cnxt_mpeg_RTOS= 
set cnxt_mpeg_CONFIG= 
set cnxt_mpeg_SWCONFIG=CNXT
set cnxt_mpeg_INTERNAL_BUILD=YES
set cnxt_mpeg_WINDBASE=c:\tornado220
set cnxt_mpeg_TOOLKIT=SDT
set cnxt_mpeg_TOOLVER=251
set cnxt_mpeg_DEF_DBG=T32
REM  THE FOLLOWING WILL RARELY IF EVER CHANGE
set cnxt_mpeg_MULTIICE=1
set cnxt_mpeg_MICEDIR=c:\progra~1\arm\Multi-ICE
set cnxt_mpeg_NMAKE=c:\tools2k
set cnxt_mpeg_ARMSDTDIR=c:\isiarm\arm250
set cnxt_mpeg_ARMADSDIR=c:\progra~1\arm\adsv1_2
set cnxt_mpeg_PSSROOT=c:\isiarm\pssarm.230
set cnxt_mpeg_T32=c:\t32
set cnxt_mpeg_PVCS=k:\sabine\pvcs 
set cnxt_mpeg_vcsid=bintzmf

if exist c:\cnxtmpeg\overrides.bat goto have_overrides
call %cnxt_mpeg_workdrive%\%cnxt_mpeg_project1%\%cnxt_mpeg_project2%\toolexe\mkenv1.bat %1 %2 %3 %4 %5 %6 %7
goto done

:have_overrides
call c:\cnxtmpeg\overrides.bat %1 %2 %3 %4 %5 %6 %7 

:done

