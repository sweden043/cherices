@echo off
set SABINE_ROOT=C:\VXSABINE
set SABINE_SRCDIR=C:\\VXSABINE
set BIN_DIR=c:\cnxt\arm\arm-coff\bin
set COMPILER_PATH=c:\cnxt\arm\lib\gcc-lib\arm-coff\27B32A~1.9-9
SET CONFIG=SPARTA
SET DEFAULT_CONFIG=SPARTA
set CONFIG_ROOT=K:\SABINE\CONFIGS
set CPU=ARMARCH4
set WIND_BASE=c:\tornado
set WIND_HOST_TYPE=x86-win32
set WIND_ARCH=arm
set INCLUDE=.;%SABINE_ROOT%\HWLIB;%SABINE_ROOT%\vxincl;%SABINE_ROOT%\include;%SABINE_ROOT%\vxkal;%SABINE_ROOT%\vxbsp;..\include;%WIND_BASE%\target;%WIND_BASE%\target\h\private;%WIND_BASE%\target\h;%WIND_BASE%\target\h\rpc;%WIND_BASE%\target\config\all;%WIND_BASE%\target\h\drv\netif;%WIND_BASE%\target\src\config;%WIND_BASE%\target\src\drv;%WIND_BASE%\target\h\arch\arm;%WIND_BASE%\host\x86-win32\lib\gcc-lib\arm-wrs-vxworks\2.9-010413\include;
set RTOS=VXWORKS
cls
SET ARM_TOOLKIT=VXW
SET ARM_VERSION=220

set PATH=%BIN_DIR%;%COMPILER_PATH%;%WIND_BASE%\host\x86-win32\arm-wrs-vxworks\bin;%PATH%;

echo ***********************************************
echo * Environment set for CX24430/1/2/3 VXW builds*
echo ***********************************************
echo.


