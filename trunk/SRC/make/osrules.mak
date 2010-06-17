###############################################################################
#                                                                             #
# Sabine Build Support File: OSRULES.MAK                                      #
#                                                                             #
# Author: B. Bradford, 2002-01-11                                             #
#                                                                             #
# OS Macro Defintions for BUILD system commands that may vary from OS to      #
# OS.                                                                         #
#                                                                             #
###############################################################################

!if "$(WINNT)" == ""
!if "$(SYSTEMROOT)" == ""
WINNT=NO
!else
WINNT=YES
!endif
!endif


###############################################################################
# Name:           DEL_TREE                                                    #
# Description:    Delete all files in a directory tree                        #
#                                                                             #
# Windows 95/98/Me will support the deltree.exe command, with                 #
# the /Y option.                                                              #
#                                                                             #
# Windows NT (4.0, 2000, XP) will support the RMDIR command, with             #
# the /S (for sub-directories) and /Q (for QUIET mode)                        #
#                                                                             #
###############################################################################

!if "$(WINNT)" == "YES"
DEL_TREE = RMDIR /S /Q
!else
DEL_TREE = DELTREE /Y
!endif

