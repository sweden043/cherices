;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  ;;
;;                       SOFTWARE FILE;MODULE HEADER                        ;;
;;                    Conexant Systems Inc. (c) 1998-2002                   ;;
;;                               Austin, TX                                 ;;
;;                            All Rights Reserved                           ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
 ; Filename:       PCMSRC.s
 ;
 ;
 ; Description:    Assembly Optimized Sample Rate Conversion routines
 ;
 ;
 ; Author:         Senthil Veluswamy
 ;
 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; $Header: pcmsrcs.s, 7, 8/1/03 3:41:18 PM, $
 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;
;; Aliases ;;
;;;;;;;;;;;;;
TAPS           EQU      8
HALF_TAPS      EQU      TAPS>>1

;;;;;;;;;;;;;;;;;
;; Global Data ;;
;;;;;;;;;;;;;;;;;
   AREA |PCMSRCGlobData|, DATA
      EXPORT MFADIndex
      EXPORT SFADIndex
MFADIndex      DCD   0x00000000
SFADIndex      DCD   0x00000000

;;;;;;;;;;;;;;;;;
;; Static Data ;;
;;;;;;;;;;;;;;;;;
   AREA |PCMSRCStatData|, DATA
SRCStats
PrevLeftValue  DCD   0x00000000
PrevRightValue DCD   0x00000000
PrevValue      DCD   0x00000000

   AREA |PCMSRCZiStatData|, NOINIT
prev_data      %     4*TAPS
prev_left_data %     4*TAPS
prev_right_data %    4*TAPS

;;;;;;;;;;;;;;;
;; Functions ;;
;;;;;;;;;;;;;;;
   IF :DEF: |ads$version|
      AREA |PCMSRC|, CODE, READONLY
   ELSE
      AREA |PCMSRC|, CODE, READONLY, INTERWORK
   ENDIF

   IF (:DEF: NON_VXWORKS)
   IMPORT __rt_sdiv
   ENDIF ; (:DEF: NON_VXWORKS)
   EXPORT MonoLinearInterpolate
   EXPORT StereoLinearInterpolate
   EXPORT MonoFilterAndDecimate
   EXPORT StereoFilterAndDecimate

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  MonoLinearInterpolate()                                          ;;
;;                                                                   ;;
;;  PARAMETERS:  r0 - Pointer to a set of Mono PCM samples to be     ;;
;;                      interpolated                                 ;;
;;               r1 - Pointer to the interpolated PCM samples        ;;
;;               r2 - Number of Samples to be interpolated           ;;
;;               r3 - the factor by which to interpolate             ;;
;;                                                                   ;;
;;  DESCRIPTION: This routine Performs a linear interpolation on a   ;;
;;               set of mono PCM sampless                            ;;
;;                                                                   ;;
;;  RETURNS:     Nothing.                                            ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;void MonoLinearInterpolate(PCM20 * InputBuffer, PCM20 * OutputBuffer,
;   int InputSamples, int Interpolator)
;{
MonoLinearInterpolate

      stmfd    sp!, {r4-r11, lr}       ; Save register values. 
                                       ; These will be used as follows: 
                                       ; r0 - scratch
                                       ; r1 - scratch, j (loop index)
                                       ; r2 - scratch
                                       ; r3 - scratch
                                       ; r4 - Interpolator
                                       ; r5 - Static Base
                                       ; r6 - PrevValue
                                       ; r7 - Step
                                       ; r8 - Temp
                                       ; r9 - Input Pointer
                                       ; r10 - Output Pointer
                                       ; r11 - Final Output Pointer

;   PCM20 Step;
;   PCM20 Temp;
;   int j;
;   PCM20 *InputPtr, *OutputPtr;
;   PCM20 *FinalOutputPtr;
;   static PCM20 PrevValue = 0;

;   InputPtr = InputBuffer;
      mov      r9, r0

;   OutputPtr = OutputBuffer;
      mov      r10, r1

      mov      r4, r3                  ; Save the Interpolator

;   FinalOutputPtr = &OutputBuffer[((InputSamples * Interpolator) - 1)];
      mul      r3, r2, r4
      sub      r3, r3, #1
      mov      r0, r3, lsl #2
      add      r11, r1, r0

      ldr      r5, =PrevValue          ; Get saved PrevValue
      ldr      r6, [r5]

0
;   while(OutputPtr < FinalOutputPtr)
;   {
      cmp      r10, r11                ; While loop Start
      bge      %FT9                    ; While loop End

;      Step = *InputPtr - PrevValue;
      ldr      r0, [r9]
      sub      r7, r0, r6

;      Step /= Interpolator;
      mov      r0, r4
      mov      r1, r7
      IF (:DEF: NON_VXWORKS)
      bl       __rt_sdiv
      ELSE ; (:DEF: NON_VXWORKS)
      bl       __udivsi3
      ENDIF ; (:DEF: NON_VXWORKS)
      mov      r7, r0

;      Temp = PrevValue + Step;
      add      r8, r6, r7

;      for(j=0; j < Interpolator; j++)
;      {
      mov      r1, #0                  ; j=0
1
      cmp      r1, r4                  ; For loop Start
      bge      %FT2                    ; For loop End

;         *OutputPtr++ = Temp;
      stmia    r10!, {r8}

;         Temp += Step;
      add      r8, r8, r7

      add      r1, r1, #1              ; j++
      b        %BT1                    ; Go back to the For Loop Start
;      }

2
;      PrevValue = *InputPtr++;
      ldmia   r9!, {r6}

      b        %BT0                    ; Go back to the While Loop Start
;   }

9
      str      r6, [r5]                ; Save PrevValue

      ldmfd    sp!, {r4-r11, lr}       ; Finished!
      bx       lr
;}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  StereoLinearInterpolate()                                        ;;
;;                                                                   ;;
;;  PARAMETERS:  r0 - Pointer to a set of Stereo PCM samples to be   ;;
;;                      interpolated                                 ;;
;;               r1 - Pointer to the interpolated PCM samples        ;;
;;               r2 - Number of Samples to be interpolated           ;;
;;               r3 - the factor by which to interpolate             ;;
;;                                                                   ;;
;;  DESCRIPTION: This routine Performs a linear interpolation on a   ;;
;;               set of stereo PCM sampless                          ;;
;;                                                                   ;;
;;  RETURNS:     Nothing.                                            ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;void StereoLinearInterpolate(PCM20 * InputBuffer, PCM20 * OutputBuffer,
;   int InputSamples, int Interpolator)
;{
StereoLinearInterpolate

      stmfd    sp!, {r4-r11, lr}       ; Save register values. 
                                       ; These will be used as follows: 
                                       ; r0 - scratch, Static Base
                                       ; r1 - scratch, j (loop index)
                                       ; r2 - scratch, Left Temp
                                       ; r3 - scratch, Right Temp
                                       ; r4 - Interpolator
                                       ; r5 - PrevLeftValue
                                       ; r6 - PrevRightValue
                                       ; r7 - Left Step
                                       ; r8 - Right Step 
                                       ; r9 - Input Pointer
                                       ; r10 - Output Pointer
                                       ; r11 - Final Output Pointer

;   PCM20 LeftStep;
;   PCM20 RightStep;
;   PCM20 LeftTemp;
;   PCM20 RightTemp;
;   int j;
;   PCM20 *InputPtr, *OutputPtr;
;   PCM20 *FinalOutputPtr;
;   static PCM20 PrevLeftValue = 0;
;   static PCM20 PrevRightValue = 0;
;
;   InputPtr = InputBuffer;
      mov      r9, r0

;   OutputPtr = OutputBuffer;
      mov      r10, r1

      mov      r4, r3                  ; Save the Interpolator

;   FinalOutputPtr = &OutputBuffer[((InputSamples * Interpolator) - 1)];
      mul      r3, r2, r4
      sub      r3, r3, #1
      mov      r0, r3, lsl #2
      add      r11, r1, r0

      ldr      r0, =SRCStats           ; Get saved PrevLeftValue 
      ldmia    r0, {r5, r6}            ; & PrevRightValue

0
;   while(OutputPtr <= FinalOutputPtr)
;   {
      cmp      r10, r11                ; While loop Start
      bhi      %FT9                    ; While Loop Done

;      LeftStep = *InputPtr - PrevLeftValue;
      ldr      r0, [r9]
      sub      r7, r0, r5

;      LeftStep /= Interpolator;
      mov      r0, r4
      mov      r1, r7
      IF (:DEF: NON_VXWORKS)
      bl       __rt_sdiv
      ELSE ; (:DEF: NON_VXWORKS)
      bl       __udivsi3
      ENDIF ; (:DEF: NON_VXWORKS)
      mov      r7, r0

;      RightStep = *(InputPtr + 1) - PrevRightValue;
      ldr      r0, [r9, #4]
      sub      r8, r0, r6

;      RightStep /= Interpolator;
      mov      r0, r4
      mov      r1, r8
      IF (:DEF: NON_VXWORKS)
      bl       __rt_sdiv
      ELSE ; (:DEF: NON_VXWORKS)
      bl       __udivsi3
      ENDIF ; (:DEF: NON_VXWORKS)
      mov      r8, r0

;      LeftTemp = PrevLeftValue + LeftStep;
      add      r2, r5, r7

;      RightTemp = PrevRightValue + RightStep;
      add      r3, r6, r8

;      for(j=0; j < Interpolator; j++)
;      {
      mov      r1, #0                  ; j=0
1
      cmp      r1, r4                  ; For Loop Start
      bge      %FT2                    ; For Loop Done

;         *OutputPtr++ = LeftTemp;
;         *OutputPtr++ = RightTemp;
      stmia    r10!, {r2, r3}

;         LeftTemp += LeftStep;
      add      r2, r2, r7
;         RightTemp += RightStep;
      add      r3, r3, r8

      add      r1, r1, #1              ; j++
      b        %BT1                    ; Go back to the For Loop Start
;      }

2
;      PrevLeftValue = *InputPtr++;
;      PrevRightValue = *InputPtr++;
      ldmia    r9!, {r5, r6}           ; Get next Prev Values

      b        %BT0                    ; Go back to the While Loop Start
;   }

9
      ldr      r0, =SRCStats           ; Save PrevLeftValue & PrevRightValue
      stmia    r0, {r5, r6}

      ldmfd    sp!, {r4-r11, lr}       ; Finished!
      bx       lr
;}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  MonoFilterAndDecimate()                                          ;;
;;                                                                   ;;
;;  PARAMETERS:  r0 - Pointer to a set of Mono PCM samples to be     ;;
;;                      filtered & decimated                         ;;
;;               r1 - Pointer to the filtered & decimated PCM samples;;
;;               r2 - Number of samples to be filtered & decimated   ;;
;;               r3 - the factor by which to decimate                ;;
;;               stack: PCMFilterKernel - The Filter to be used      ;;
;;                                                                   ;;
;;  DESCRIPTION: Performs an FIR filter and decimation on a set of   ;;
;;                mono PCM samples.                                  ;;
;;                                                                   ;;
;;  RETURNS:     Nothing.                                            ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;void MonoFilterAndDecimate( PCM20 * InputPtr, 
;                            PCM20 * OutputPtr,
;                            int InputSamples, 
;                            int Decimator,
;                            int32 *PCMFilterKernel)
;{
MonoFilterAndDecimate

      stmfd    sp!, {r3-r11, lr}       ; Save register values. 
                                       ; These will be used as follows: 
                                       ; r0 - InputPtr
                                       ; r1 - OutputPtr
                                       ; r2 - InputSamples
                                       ; r3 - scratch
                                       ; r4 - scratch
                                       ; r5 - scratch, j
                                       ; r6 - scratch, k
                                       ; r7 - i
                                       ; r8 - &PCMFilterKernel
                                       ; r9 - prev_data
                                       ; r10 - Result
                                       ; r11 - MFADIndex

;   int i;
;   int j, k;
;   PCM20 Result;

      ldr      r8, [sp, #40]           ; Load the Filter base
      ldr      r9, =prev_data          ; Load the static base

      ldr      r4, =MFADIndex          ; Load the MFADIndex
      ldr      r11, [r4]

;   while(InputSamples)
;   {
0
      cmp      r2, #0                  ; While Loop Start
      ble      %FT9                    ; While Loop Done

;      i = Decimator;
      ldr      r7, [sp]
;      mov      r7, r3

;      while((i > 0) && (InputSamples > 0))
;      {
1
      cmp      r7, #0                  ; While Loop 2 Start
      ble      %FT2                    ; While Loop 2 Done
      cmp      r2, #0
      ble      %FT2

;         prev_data[MFADIndex++] = *InputPtr++;
      ldr      r4, [r0], #4
      str      r4, [r9, r11, lsl #2]
      add      r11, r11, #1

;         if(MFADIndex >= TAPS)
;         {
;            MFADIndex = 0;
;         }
      cmp      r11, #TAPS
      movge    r11, #0

;         i--;
      sub      r7, r7, #1

;         InputSamples--;
      sub      r2, r2, #1

      b        %BT1                    ; Go back to Start of While Loop 2
;      };

2
;      /* if we ran out of input samples before collecting enough points for
;         decimation, break out of the loop and finish this when we have more
;         data */
;      if((InputSamples == 0) && (i != 0))
;         break; 
      cmp      r2, #0
      bne      %FT3                    ; Continue
      cmp      r7, #0
      bne      %FT9                    ; Go to the End

3
;      Result = 0;
      mov      r10, #0

;      i = 0;
      mov      r7, #0

;      j = MFADIndex - 1; /* index is one ahead of last data entered */
      sub      r5, r11, #1
;      k = MFADIndex;
      mov      r6, r11

;      do
;      {
4                                      ; Start of Do Loop
;         if (j < 0)
;         {
;            j += TAPS;
;         }
      cmp      r5, #0
      addlt    r5, r5, #TAPS

;         if (k >= TAPS)
;         {
;            k -= TAPS;
;         }
      cmp      r6, #TAPS
      subge    r6, r6, #TAPS

;         Result += ((prev_data[j] + prev_data[k]) * PCMFilterKernel[i]) >> 11;
      ldr      r3, [r9, r5, lsl #2]    ; prev_data[j]
      ldr      r4, [r9, r6, lsl #2]    ; prev_data[k]
      add      r3, r3, r4
      ldr      r4, [r8, r7, lsl #2]    ; PCMFilterKernel[i]
      mul      r4, r3, r4
      mov      r4, r4, asr #11
      add      r10, r10, r4

;         j--;
      sub      r5, r5, #1
;         k++;
      add      r6, r6, #1
;         i++;
      add      r7, r7, #1

      cmp      r7, #HALF_TAPS          ; Do Loop Check
      blt      %BT4                    ; Go back to the Do Loop Start
;      } while(i < HALF_TAPS);

      ldr      r5, =0x7FFFF
      ldr      r6, =0xFFF80000

;      if(Result > 524287)
;      {
;         Result = 524287;
      cmp      r10, r5
      movgt    r10, r5
      bgt      %FT5
;      }
;      else if (Result < -524288)
;      {
;         Result = -524288;
      cmp      r10, r6
      movlt    r10, r6
;      }

5
;      *OutputPtr++ = Result;
      str      r10, [r1], #4

      b        %BT0                    ; Go back to the While loop Start
;   }

9
      add      sp, sp, #4              ; discard temp store

      ldmfd    sp!, {r4-r11, lr}       ; Finished!
      bx       lr
;}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  StereoFilterAndDecimate()                                        ;;
;;                                                                   ;;
;;  PARAMETERS:  r0 - Pointer to a set of Stereo PCM samples to be   ;;
;;                      filtered & decimated                         ;;
;;               r1 - Pointer to the filtered & decimated PCM samples;;
;;               r2 - Number of samples to be filtered & decimated   ;;
;;               r3 - the factor by which to decimate                ;;
;;               stack: PCMFilterKernel - The Filter to be used      ;;
;;                                                                   ;;
;;  DESCRIPTION: Performs an FIR filter and decimation on a set of   ;;
;;                stereo PCM sampless                                ;;
;;                                                                   ;;
;;  RETURNS:     Nothing.                                            ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;void StereoFilterAndDecimate( PCM20 * InputPtr, 
;                              PCM20 * OutputPtr,
;                              int InputSamples, 
;                              int Decimator,
;                              int32 *PCMFilterKernel)
;{
StereoFilterAndDecimate

      stmfd    sp!, {r0-r1, r3-r11, lr}
                                       ; Save register values. 
                                       ; These will be used as follows: 
                                       ; r0 - InputPtr, &PCMFilterKernel
                                       ; r1 - OutputPtr, PCMFilterKernel[i]
                                       ; r2 - InputSamples
                                       ; r3 - scratch
                                       ; r4 - scratch
                                       ; r5 - i
                                       ; r6 - scratch, j
                                       ; r7 - prev_left_data
                                       ; r8 - prev_right_data
                                       ; r9 - LeftResult
                                       ; r10 - RightResult
                                       ; r11 - SFADIndex, k

;   int i;
;   int j, k;
;   PCM20 LeftResult;
;   PCM20 RightResult;

      ldr      r7, =prev_left_data     ; Load the static base values
      ldr      r8, =prev_right_data

;   while(InputSamples)
;   {
0
      cmp      r2, #0                  ; While Loop Start
      ble      %FT9                    ; While Loop Done

;      i = Decimator;
      ldr      r5, [sp, #8]

      ldr      r0, [sp]                ; InputPtr
      ldr      r6, =SFADIndex          ; &SFADIndex
      ldr      r11, [r6]               ; Load SFADIndex

;      while((i > 0) && (InputSamples > 0))
;      {
1
      cmp      r5, #0                  ; While Loop 2 Start
      ble      %FT2                    ; While Loop 2 Done
      cmp      r2, #0
      ble      %FT2

;         prev_left_data[SFADIndex] = *InputPtr++;
;         prev_right_data[SFADIndex] = *InputPtr++;
      ldmia    r0!, {r3, r4}
      str      r3, [r7, r11, lsl #2]
      str      r4, [r8, r11, lsl #2]

;         InputSamples-= 2;
      sub      r2, r2, #2

;         i--;
      sub      r5, r5, #1

;         SFADIndex++;
      add      r11, r11, #1

;         if(SFADIndex >= TAPS)
;         {
;            SFADIndex = 0;
;         }
      cmp      r11, #TAPS
      movge    r11, #0

      b        %BT1                    ; Go back to Start of While Loop 2
;      };

2
      str      r11, [r6]               ; writeback SFADIndex
      str      r0, [sp]                ; Writeback InputPtr
      ldr      r0, [sp, #48]           ; &PCMFilterKernel

;      /* if we ran out of input samples before collecting enough points for
;         decimation, break out of the loop and finish this when we have more
;         data */
;      if((InputSamples == 0) && (i != 0))
;         break; 
      cmp      r2, #0
      bne      %FT3                    ; Continue
      cmp      r5, #0
      bne      %FT9                    ; Go to the End

3
;      LeftResult = 0;
      mov      r9, #0
;      RightResult = 0;
      mov      r10, #0

;      i = 0;
      mov      r5, #0
;      j = SFADIndex - 1; /* index is one ahead of last data entered */
      sub      r6, r11, #1
;      k = SFADIndex;

;      do
;      {
4                                      ; Start of Do Loop
;         if (j < 0)
;         {
;            j += TAPS;
;         }
      cmp      r6, #0
      addlt    r6, r6, #TAPS

;         if (k >= TAPS)
;         {
;            k -= TAPS;
;         }
      cmp      r11, #TAPS
      subge    r11, r11, #TAPS

;         LeftResult += ((prev_left_data[j] + prev_left_data[k])
;            * PCMFilterKernel[i]) >> 11;
      ldr      r3, [r7, r6, lsl #2]    ; prev_left_data[j]
      ldr      r4, [r7, r11, lsl #2]   ; prev_left_data[k]
      add      r3, r3, r4
      ldr      r1, [r0, r5, lsl #2]    ; PCMFilterKernel[i]
      mul      r4, r3, r1
      mov      r4, r4, asr #11
      add      r9, r9, r4

;         RightResult += ((prev_right_data[j] + prev_right_data[k])
;            * PCMFilterKernel[i]) >> 11;
      ldr      r3, [r8, r6, lsl #2]    ; prev_right_data[j]
      ldr      r4, [r8, r11, lsl #2]   ; prev_right_data[k]
      add      r3, r3, r4
      mul      r4, r3, r1
      mov      r4, r4, asr #11
      add      r10, r10, r4

;         j--;
      sub      r6, r6, #1
;         k++;
      add      r11, r11, #1
;         i++;
      add      r5, r5, #1

      cmp      r5, #HALF_TAPS          ; Do Loop Check
      blt      %BT4                    ; Go back to the Do Loop Start
;      } while(i < HALF_TAPS);

      ldr      r5, =0x7FFFF
      ldr      r6, =0xFFF80000

;      if(LeftResult > 524287)
;      {
;         LeftResult = 524287;
      cmp      r9, r5
      movgt    r9, r5
      bgt      %FT5
;      }
;      else if (LeftResult < -524288)
;      {
;         LeftResult = -524288;
      cmp      r9, r6
      movlt    r9, r6
;      }

5
;      if(RightResult > 524287)
;      {
;         RightResult = 524287;
      cmp      r10, r5
      movgt    r10, r5
      bgt      %FT6
;      }
;      else if (RightResult < -524288)
;      {
;         RightResult = -524288;
      cmp      r10, r6
      movlt    r10, r6
;      }

6
;      *OutputPtr++ = LeftResult;
;      *OutputPtr++ = RightResult;
      ldr      r1, [sp, #4]
      stmia    r1!, {r9, r10}
      str      r1, [sp, #4]

      b        %BT0                    ; Go back to the While loop Start
;   }

9
      add      sp, sp, #12             ; discard temp store

      ldmfd    sp!, {r4-r11, lr}       ; Finished!
      bx       lr
;}

   END

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
 ; Modifications:
 ; $Log: 
 ;  7    mpeg      1.6         8/1/03 3:41:18 PM      Senthil Veluswamy SCR(s) 
 ;        7109 :
 ;        __rt_sdiv is a non-vxworks assembler function. Wrapped it for non-VxW
 ;         builds and added a VxW funtion to help the ASM-GNU convertor 
 ;        generate a VxW compliant assembly file.
 ;        
 ;  6    mpeg      1.5         12/3/02 3:12:40 PM     Senthil Veluswamy SCR(s) 
 ;        5058 :
 ;        Exported globals
 ;        
 ;  5    mpeg      1.4         11/27/02 12:48:48 PM   Senthil Veluswamy SCR(s) 
 ;        5001 :
 ;        Fixed function header comments
 ;        
 ;  4    mpeg      1.3         11/25/02 12:41:32 PM   Senthil Veluswamy SCR(s) 
 ;        5001 :
 ;        Completed optimized routines for MonoLinearInterpolate, 
 ;        StereoLinearInterpolate, MonoFilterAndDecimate, 
 ;        StereoFilterAndDecimate. 
 ;        
 ;  3    mpeg      1.2         11/23/02 4:41:00 PM    Senthil Veluswamy SCR(s) 
 ;        5001 :
 ;        Optimized assembly routine for StereoFilterAndDecimate
 ;        
 ;  2    mpeg      1.1         11/20/02 6:01:18 PM    Senthil Veluswamy SCR(s) 
 ;        5001 :
 ;        Fixed up the comment string
 ;        
 ;  1    mpeg      1.0         11/20/02 5:59:02 PM    Senthil Veluswamy 
 ; $
 ; 
 ;    Rev 1.6   01 Aug 2003 14:41:18   velusws
 ; SCR(s) 7109 :
 ; __rt_sdiv is a non-vxworks assembler function. Wrapped it for non-VxW builds and added a VxW funtion to help the ASM-GNU convertor generate a VxW compliant assembly file.
 ; 
 ;    Rev 1.5   03 Dec 2002 15:12:40   velusws
 ; SCR(s) 5058 :
 ; Exported globals
 ; 
 ;    Rev 1.4   27 Nov 2002 12:48:48   velusws
 ; SCR(s) 5001 :
 ; Fixed function header comments
 ; 
 ;    Rev 1.3   25 Nov 2002 12:41:32   velusws
 ; SCR(s) 5001 :
 ; Completed optimized routines for MonoLinearInterpolate, StereoLinearInterpolate, MonoFilterAndDecimate, StereoFilterAndDecimate. 
 ; 
 ;    Rev 1.2   23 Nov 2002 16:41:00   velusws
 ; SCR(s) 5001 :
 ; Optimized assembly routine for StereoFilterAndDecimate
 ; 
 ;    Rev 1.1   20 Nov 2002 18:01:18   velusws
 ; SCR(s) 5001 :
 ; Fixed up the comment string
 ; 
 ;    Rev 1.0   20 Nov 2002 17:59:02   velusws
 ; SCR(s) 5001 :
 ; Assembly Optimized Sample Rate Conversion routines
 ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
