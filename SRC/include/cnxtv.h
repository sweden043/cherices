
/* if KB_UP_EVENT has already been defined, 
   one of the OpenTV header files has already defined
   these things */
#ifndef KB_UP_EVENT 

#define	KB_UP_EVENT			0
#define	KB_DOWN_EVENT		1

/*
enum {
	VIDEO_PTS_MODE = 1,
	VIDEO_VBV_DELAY_MODE,
	VIDEO_NONREAL_TIME_MODE,
	VIDEO_UNKNOWN_MODE
};

typedef enum {
	DECIDE_YOURSELF = VIDEO_UNKNOWN_MODE,
	USE_PTS = VIDEO_PTS_MODE,				
	USE_VBV_DELAY = VIDEO_VBV_DELAY_MODE,    
	USE_NONREAL_TIME_MODE = VIDEO_NONREAL_TIME_MODE
} video_starting_mode;
*/

enum {
        AUDIO_PCM_FORMAT = 1,
        AUDIO_PURE_MPEG1_FORMAT,
        AUDIO_MPEG_PACKET_FORMAT,
        AUDIO_AC2_FORMAT,
        AUDIO_AC3_FORMAT
};

typedef enum {
        PCM = AUDIO_PCM_FORMAT,
        PURE_MPEG_1 = AUDIO_PURE_MPEG1_FORMAT,
        MPEG_PACKET = AUDIO_MPEG_PACKET_FORMAT,
        AC2 = AUDIO_AC2_FORMAT,
        AC3 = AUDIO_AC3_FORMAT
} audio_format;

/* The data structure of farcir_q */
typedef struct {
        void *          begin;
        int             size;           /* NOTE:It holds at most (size-1)  bytes in queue. */
        int             in;             /* Write  offset from the beginning of the buffer. */
        int             out;            /* Read offset form the beginning of the buffer. */
} farcir_q;

#endif /* ndef KB_UP_EVENT */

