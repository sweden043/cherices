/****************************************************************************/
/*                      Conexant Systems                                    */
/****************************************************************************/
/*                                                                          */
/* Filename:           HAMMING.C                                            */
/*                                                                          */
/* Description:        Hamming decoder                                      */
/*                                                                          */
/* Author:             Miles Bintz                                          */
/*                                                                          */
/* Copyright Conexant Systems, 2001                                         */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/* $Header: hamming.c, 2, 3/26/02 9:52:40 AM, Billy Jackman$ */
/****************************************************************************/

#include "hamming.h"

#define P84(x)   ( x.ham.h7 ^ x.ham.h6 ^ x.ham.h5 ^ x.ham.h4 ^ x.ham.h3 ^ x.ham.h2 ^ x.ham.h1 ^ x.ham.h0 )
#define CK84(x)  (((x.ham.h7 ^ x.ham.h5 ^ x.ham.h1 ^ x.ham.h0) << 2) | \
                  ((x.ham.h7 ^ x.ham.h3 ^ x.ham.h2 ^ x.ham.h1) << 1) | \
                  ((x.ham.h5 ^ x.ham.h4 ^ x.ham.h3 ^ x.ham.h1) << 0))

#define P2418(x)   (x.ham.h23 ^ x.ham.h22 ^ x.ham.h21 ^ x.ham.h20 ^ x.ham.h19 ^ x.ham.h18 ^ x.ham.h17 ^ x.ham.h16 ^ \
                    x.ham.h15 ^ x.ham.h14 ^ x.ham.h13 ^ x.ham.h12 ^ x.ham.h11 ^ x.ham.h10 ^  x.ham.h9 ^  x.ham.h8 ^ \
                    x.ham.h7  ^  x.ham.h6 ^  x.ham.h5 ^  x.ham.h4 ^  x.ham.h3 ^  x.ham.h2 ^  x.ham.h1 ^  x.ham.h0)
                   
#define CK2418(x)  (((x.ham.h15 ^ x.ham.h16 ^ x.ham.h17 ^ x.ham.h18 ^ x.ham.h19 ^ x.ham.h20 ^ x.ham.h21 ^ x.ham.h22) << 4) | \
                    ((x.ham.h7  ^ x.ham.h8  ^ x.ham.h9  ^ x.ham.h10 ^ x.ham.h11 ^ x.ham.h12 ^ x.ham.h13 ^ x.ham.h14) << 3) | \
                    ((x.ham.h3  ^ x.ham.h4  ^ x.ham.h5  ^ x.ham.h6  ^ x.ham.h11 ^ x.ham.h12 ^ x.ham.h13 ^ x.ham.h14 ^ x.ham.h19 ^ x.ham.h20 ^ x.ham.h21 ^ x.ham.h22)  << 2) | \
                    ((x.ham.h1  ^ x.ham.h2  ^ x.ham.h5  ^ x.ham.h6  ^ x.ham.h9  ^ x.ham.h10 ^ x.ham.h13 ^ x.ham.h14 ^ x.ham.h17 ^ x.ham.h18 ^ x.ham.h21 ^ x.ham.h22)  << 1) | \
                    ((x.ham.h0  ^ x.ham.h2  ^ x.ham.h4  ^ x.ham.h6  ^ x.ham.h8  ^ x.ham.h10 ^ x.ham.h12 ^ x.ham.h14 ^ x.ham.h16 ^ x.ham.h18 ^ x.ham.h20 ^ x.ham.h22)  << 0))
 

 
typedef union {
   struct {
       unsigned int  h0:1, h1:1, h2:1, h3:1, h4:1, h5:1, h6:1, h7:1;
   } ham;
   unsigned int byte;
} Ham84;
                
typedef union {
   struct {
       unsigned int   h0:1,  h1:1,  h2:1,  h3:1,  h4:1,  h5:1,  h6:1,  h7:1,
                      h8:1,  h9:1, h10:1, h11:1, h12:1, h13:1, h14:1, h15:1,
                     h16:1, h17:1, h18:1, h19:1, h20:1, h21:1, h22:1, h23:1;
                     
   } ham;
   unsigned int word;
} Ham2418;
                

int HammingDecode(unsigned char coding, unsigned int codeword, int *decode) {
    Ham84     h;
    Ham2418   H;
    *decode = 0xFFFFFFFF;
    
    if (coding == CODING_8_4) { 
       h.byte = codeword;
       if (P84(h)) {
          /* two errors or no error */
          if (CK84(h) == 7) { 
              *decode = (h.ham.h7 << 3) | (h.ham.h5 << 2) | (h.ham.h3 << 1) | (h.ham.h1 << 0);
              return 0;
          } else {
              return 1;
          }    
       } else { /* !P84(h) */
          /* One error */
          switch (CK84(h)) {
             case 1: /* Bit 7 has the error */
                *decode = (1 ^ h.ham.h7 << 3) | (h.ham.h5 << 2) | (h.ham.h3 << 1) | (h.ham.h1 << 0);
                return 0; 
                break;
             case 2: /* Bit 5 has the error */
                *decode = ( h.ham.h7 << 3) | (1 ^ h.ham.h5 << 2) | (h.ham.h3 << 1) | (h.ham.h1 << 0);
                return 0; 
                break;
             case 4: /* Bit 3 has the error */
                *decode = ( h.ham.h7 << 3) | (h.ham.h5 << 2) | (1 ^ h.ham.h3 << 1) | (h.ham.h1 << 0);
                return 0; 
                break;
             case 0: /* Bit 1 has the error */
                *decode = ( h.ham.h7 << 3) | (h.ham.h5 << 2) | (h.ham.h3 << 1) | (1 ^ h.ham.h1 << 0);
                return 0; 
                break;
             default:
                return 1;
          } /* switch CK84 */
       } /*  if P84(h)  */
    }
    
    if (coding == CODING_24_18) {
       H.word = codeword;
       *decode = ( (H.ham.h22 << 17) | (H.ham.h21 << 16) | (H.ham.h20 << 15) | (H.ham.h19 << 14) | 
                   (H.ham.h18 << 13) | (H.ham.h17 << 12) | (H.ham.h16 << 11) | (H.ham.h14 << 10) | 
                   (H.ham.h13 <<  9) | (H.ham.h12 <<  8) | (H.ham.h11 <<  7) | (H.ham.h10 <<  6) | 
                   (H.ham.h9  <<  5) | (H.ham.h8  <<  4) | (H.ham.h6  <<  3) | (H.ham.h5  <<  2) | 
                   (H.ham.h4  <<  1) | (H.ham.h2  ) );

       if (P2418(H)) {
          /* 2 or more errors or 0 errors */
          if (CK2418(H) == 0x1f) {
             return 0;
          } else {
             if (CK2418(H) < 24) {
                 *decode ^= (1 << CK2418(H));
                 return 0;
             } else {
                 return 1;
             }
          }
       }
    }

    return 1;
}

/****************************************************************************
 * Modifications:
 *
 * $Log: 
 *  2    mpeg      1.1         3/26/02 9:52:40 AM     Billy Jackman   SCR(s) 
 *        3446 3429 :
 *        Added parentheses to avoid compiler warnings.  Added file header and 
 *        trailer
 *        comments.
 *        
 *  1    mpeg      1.0         8/13/01 12:11:56 PM    Miles Bintz     
 * $
 * 
 *    Rev 1.1   26 Mar 2002 09:52:40   jackmaw
 * SCR(s) 3446 3429 :
 * Added parentheses to avoid compiler warnings.  Added file header and trailer
 * comments.
 *
 ****************************************************************************/
