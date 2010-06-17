;****************************************************************************
;*                            Conexant Systems                              *
;****************************************************************************
;*                                                                          *
;* Filename:       os.s                                                     *
;*                                                                          *
;* Description:    OS specific data structures and routines.                *
;*                                                                          *
;* Author:         Tim Ross                                                 *
;*                                                                          *
;* Copyright Conexant Systems, 1999                                         *
;* All Rights Reserved.                                                     *
;*                                                                          *
;****************************************************************************

;******************
;* Include Files  *
;******************

;**********************
;* Local Definitions  *
;**********************

;************************
;* External Definitions *
;************************

;***************
;* Global Data *
;***************
    KEEP
    AREA |OSData|, DATA, READWRITE

    EXPORT anchor
    EXPORT rom_anchor
    EXPORT cp_anchor    
anchor
                DCD 0
rom_anchor
                DCD 0
cp_anchor
                DCD 0


    END
