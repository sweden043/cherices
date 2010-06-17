/***************************************************************************
*                                                                          
* IMPORTANT :
*
* Beginning with version 4.3 of Nucleus NET. This file is obsolete and should 
* no longer be used. This file and TCP_ERRS.C have been replaced by NERRS.H 
* and NERRS.C. The only purpose of this file is to provide backwards 
* compatability for those applications, drivers, etc. that made use of 
* the old TCP_ERRS.H and TCP_ERRS.C files.
*                                                                          
*                                                                          
****************************************************************************/
#ifndef TCP_ERRS_H
#define TCP_ERRS_H

#include "target.h"
#include "nerrs.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif /* _cplusplus */

#define NU_Tcp_Log_Error(a, b, c, d)    NERRS_Log_Error((b), (c), (d));
#define NU_Tcp_Clear_All_Errors         NERRS_Clear_All_Errors
#define NU_Tcp_Error_String             NERRS_Error_String



#define TCP_RECOVERABLE         NERR_RECOVERABLE
#define UDP_RECOVERABLE         NERR_RECOVERABLE
#define IP_RAW_RECOVERABLE      NERR_RECOVERABLE
#define TCP_TRIVIAL             NERR_INFORMATIONAL
#define TCP_SEVERE              NERR_SEVERE
#define TCP_FATAL               NERR_FATAL

#ifdef          __cplusplus
}
#endif /* _cplusplus */


#endif  /* TCPERRS_H */
