**************************
  Build System Overview
**************************

The build system uses a hierarchical system of makefiles.  This was done to eliminate redundant code within the makefiles and to isolate and clarify the purpose and type of definitions which belong in a makefile.

The following is a list of files used with a brief description of the purpose of each file:

[application]\makefile:  contains a list of drivers and libraries to include in a build.  Build switches can be hardcoded in this makefile to pass options down to all build sub-processes.  This makefile is the "entry point" for the builds.  It includes $(SABINE_ROOT)\make\topmake.mak.

$(SABINE_ROOT)\make\product.mak: This file converts the specified hardware and software config files into makefiles which can be included by the build system.  This file basically takes config options and ensures they are defined in the appropriate namespaces.

$(SABINE_ROOT)\make\topmake.mak:  Contains default definitions of all variables used in the build system.  Defines dependencies for the "all:" target.  Includes additional makefiles such as clean.mak and link.mak.  The overall execution flow for topmake is described in more detail below.

$(SABINE_ROOT)\make\envcheck.mak: performs some sanity checking on environment settings to make sure all "set" options are supported.

$(SABINE_ROOT)\make\flags.mak: a central point for including flags specific to additional toolsets.

$(SABINE_ROOT)\make\flags[toolkit].mak: defines the path and executable name for the ARM compiler, THUMB compiler, assemblers, linkers, and librarians.  Also contains a list of switches specific to each toolchain.  For every new toolkit supported, a new flags[toolkit].mak should be created and conditionally included from flags.mak

$(SABINE_ROOT)\make\cdefs.mak: contains a list of toolkit independent C definitions.

$(SABINE_ROOT)\make\adefs.mak: contains a list of ASM definitions.  This file actually has two lists which contain the same information but are syntactically different for the various asssemblers.

$(SABINE_ROOT)\make\builddirs.mak: Defines build variables which determine WHERE build output will be placed.  This makefile is included in almost all topmake subprocesses.

$(SABINE_ROOT)\make\rtos.mak: Contains definitions which are specific to a given RTOS.  These include the BSP and KAL libraries, RTOS libraies, and include directories.

$(SABINE_ROOT)\make\depend.mak:  depend.mak is called as a subprocess of topmake.  As such it has an "all:" target of its own.  It is responsible for generating dependencies for all C files in a given driver.  This dependency list will only be built/rebuit based on the modification time of the C files.

$(SABINE_ROOT)\make\rules.mak: These are the actual build inference rules.  This file tells the build system how to turn a file of extension A into a file of extension B.  That is, to compile a C file into a ARM object file, perform the following commands ....   The rules are kept as generic (ie. don't use features specific to nmake) as possible for the sake of future porting to GNU make.  This file is called as a subprocess of topmake and has an "all:" target of its own.  It is responsible for turning source code into libraries.

$(SABINE_ROOT)\make\label.mak:  contains rules for performing a label.  This may become depricated when moving to starteam.

$(SABINE_ROOT)\make\get.mak:  contains rules for performing a get.  This may become depricated when moving to starteam.

$(SABINE_ROOT)\make\clean.mak:  contains rules for performing a clean.

$(SABINE_ROOT)\make\link.mak:  contains rules for performing a link.  This is dependant on all the driver libraries.


**************************
  Makefile "philosophy"
**************************

It is important to remember the intent of a makefile.  Generally speaking, a makefile describes the optimal path for transforming a collection of files into a final product.  That is, don't do any work that has already been done.  In order to accomplish this, you must "think like a makefile".  Think of every file in terms of "dependants".

What is the desired output?  
A binary.

How do you make a binary?
By converting an elf to a binary.

How do you make an elf?
By linking many libaries and by obeying a linker script.

How do you make a linker script?
Generate it based on the drivers listed by the application and other makefiles

How do you make libraries?
By archiving arm and thumb objects.

How do you make arm objects?
By compiling or assembling source code with an arm compiler or assembler.

How do you make thumb objects?
By compiling or assembling source code with a thumb compiler or assembler.

How do you make source code?
Its provided.  HOWEVER, it may be dependent on other source code.


**************************
     "execution flow"
**************************

The "entry point" for a build system is a file called Makefile.  These files exist in the application directory.

Makefile defines a list of drivers required to make an application.

Makefile !include topmake.mak  which contains "all:" rule.

All: is dependent on the following pseudo targets (steps): 
   1. config file to .h and .mak conversion
   2. environment setup
   3. generating list of #define DRIVER_INCL_*
   4. creating output directories
   5. Creating build timestamp header
   6. cleaning and creating log files
   7. Building Drivers
   8. Linking
   9. Building extra apps and extensions

Note that #7 generates real output.  However, the driver list itself is a pseudotarget.  This implies that for every driver, some steps will be performed.  These steps include:

   7-a. make depend
     "make depend" is called for each driver.  It includes $(driver).mak which contains a list of source files.  A dependency is generated if any file in the  source list is newer than the dependency file itself.

   7-b. make rules
       "make rules" is called for each driver.  It includes $(driver.mak), the dependency file, and flags.mak.  The goal of rules.mak is to build a library.  The library is dependant on object files which are in turn dependent on source files.  A library will only be built if a source file or one of its dependants is newer than the library itself.

   8.  Linking generates binaries which are dependant on libraries.  An application will only be linked if a library is newer than the binary.  (note: since the timestamp header file is generated during every make, the KAL is rebuilt every time and thus the application is relinked every time).

**************************
   Internal variables
**************************
The following variables are set in the make process and passed on to make subprocessses.  A brief description of each variable follows.

APPNAME - application name (ex. watchtv, testh)
APPDIR - directory in which nmake was called
AUDIO_FLAGS 
ARM_VERSION
AT - "@" or "".  Controls "echo" of nmake commands.  Set to @ if VERBOSE=NO.
BSP
BOARD_FLAGS
BUFFER_FLAGS
CPU
COMBINED_ERROR_LOG - location and filename of build output log.
CONFIG - name of hardware config to use
CUSTOMER - 
DEBUG
DELCFG - yes to del generated vcs.cfg files
DEVICE_FLAGS 
DRV_INCL - variable containing list of drivers used to build application
EXTENDED_KAL - set to yes if extended KAL is being built
EXTRA_ARM_C_FLAGS
EXTRA_THUMB_C_FLAGS
EXTRA_ARM_ASM_FLAGS
EXTRA_THUMB_ASM_FLAGS
EXTRA_MAKEDEPEND_FLAGS
GEN_CMDS
FILE_FLAGS
IMAGE_TYPE
INCLUDE - set by make to appropriate include directories for given environment
MAKE_VERSION - prerequisite build system version required
MODE 
OPENTV_FLAGS
PACKING - structure packing flag.
REVISION_FLAGS
ROM_FLAGS
RTOS - PSOS, NUP, VxWorks, uCos, NOOS
SWCONFIG - swconfig file being used
TYPE - clean, get, label, or "local" (default)
UART_FLAGS
VERBOSE
VERSION - LABEL being built
VIDEO_FLAGS







