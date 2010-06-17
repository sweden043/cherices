#ifndef _gen_aud_h_
#define _gen_aud_h_


typedef void (*gen_aud_ISRNotify_t)(void);
typedef void (*gen_aud_ASRCNotify_t)(unsigned int SampleRate);
typedef bool (*mixer_volume_func_t)(unsigned char left, unsigned char right);    

#endif
