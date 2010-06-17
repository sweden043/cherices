###############################################################
#    DEFAULTS 
###############################################################

!ifndef SDC_COMMON_MAK
SDC_COMMON_MAK=SDC_COMMON_MAK_INCLUDED

################################################
# Has the default image type been overridden ? #
################################################
!if "$(IMAGE_TYPE)" == ""
IMAGE_TYPE=GENERIC
!endif

###############################################
# Default options - don't change this section #
###############################################
!if "$(VERSION)"==""
VERSION=USE
!endif

!if "$(CUSTOMER)"==""
CUSTOMER=CNXT
!endif

!if "$(TYPE)"==""
TYPE=LOCAL
!endif

!if "$(DEBUG)"==""
DEBUG=YES
!endif

!if "$(DEBUG)"=="YES"
!if "$(TRACE_OUTPUT)"==""
TRACE_OUTPUT=YES
!endif
!else # DEBUG != YES
TRACE_OUTPUT=NO
!endif

!if "$(VERBOSE)"==""
VERBOSE=NO
!endif

!if "$(LOG)"==""
LOG=YES
!endif

!if "$(PACKING)" == ""
PACKING=1
!endif

!if "$(VERBOSE)" == "YES"
# If verbose is YES then we will echo the commands we will run
AT =   
!else
#else we will keep the commands "quiet"
AT = @
!endif

# When building ASM files with GNU, convert the ARM
# version and build with gas?
!if "$(ARM2ASM)" == ""
ARMASM2GNUASM=NO
!endif

!if "$(DEBUG)" == "YES"
DEBUGDIR = DEBUG
!else
DEBUGDIR = RELEASE
!endif

!if "$(MODE)"==""
MODE=THUMB
!endif

!if "$(DEBUG)"=="YES"
!if "$(TRACE_OUTPUT)"==""
TRACE_OUTPUT=YES
!endif
!else # DEBUG != YES
TRACE_OUTPUT=NO
!endif

!if "$(SDCAPPS_ROOT)" == ""
SDCAPPS_ROOT = $(SABINE_ROOT)\SDCAPPS
!endif

###############################################################
#    REMAINING DEFAULTS                        
###############################################################
INCLUDE=$(SABINE_ROOT)\include;$(EXTRA_INCLUDE_DIRS: =;);$(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)

############################
#End of protective header
############################
!endif  

