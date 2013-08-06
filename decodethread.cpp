#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "cbuffer.h"
#include "mp3decoder.h"
#include "vorbisdecoder.h"
#include "liboddcast/liboddcast.h"

#ifdef WIN32
#define strncasecmp _strnicmp
#endif

#define MP3_FORMAT  1 
#define OGGVORBIS_FORMAT  2

extern CBUFFER	sourceCBuffer;

extern void inputMetadataCallback(void *gbl, void *pValue);
extern oddcastGlobals          *g[];
extern oddcastGlobals          gMain;
extern int sourceMedia;
extern pthread_mutex_t encoders_mutex;

#if 0
int main (void)
{
    char buf[4096];
    size_t  bytes;
    devorb h;


    devorb_init (&h);
    while (1)
    {
        bytes = fread (buf,1,4096,stdin);
        if (bytes == 0)
            break;
        printf ("fread %d\n", bytes);
        devorb_read (&h, buf, bytes);
        while (devorb_decode (&h))
        {
            printf ("samples %d\n", h.samples);
            //  h.vi.channels
            //  h.vi.rate
            //  h.pcm[0][0..]  mono, or left samples
            //  h.pcm[1][0..]  right
        }
    }
    devorb_free (&h);
    printf ("finished\n");
    return 0;
}
#endif


MP3_DECODE_ST mp3Decoder;
devorb 		  vorbisDecoder;

int decoderLoop = 1;
void * startDecodeThread(void *url) {

	LogMessage(&gMain, LOG_DEBUG, "Starting decode thread");

	char	currentMetadata[2046] = "";
	
	if (sourceMedia == MP3_FORMAT) {
		/* MP3 */
		mp3dec_init(&mp3Decoder);
	}
	if (sourceMedia == OGGVORBIS_FORMAT) {
		/* Ogg Vorbis */
    	devorb_init (&vorbisDecoder);
	}
	decoderLoop = 1;
	while (decoderLoop) {
		int	read = 0;
		long inbuffer = 0;
		char *pStreamData = NULL;
		
        inbuffer = cbuffer_get_used(&sourceCBuffer);

		if (decoderLoop) {
			if (inbuffer > 0) {
				pStreamData = (char*)malloc(inbuffer);
				 
				if (cbuffer_extract(&sourceCBuffer, pStreamData, inbuffer)) {
	//				fprintf(stdout, "Extracted %d from the circular buffer\n", inbuffer);

					pthread_mutex_lock(&encoders_mutex);
					if (sourceMedia == MP3_FORMAT) {

						mp3dec_feed_stream(&mp3Decoder, pStreamData, inbuffer);
						if (mp3dec_get_stream_size(&mp3Decoder) > MIN_MP3_STREAM_SIZE) {
							if (!mp3dec_decode(&mp3Decoder)) {
								pthread_mutex_unlock(&encoders_mutex);
								LogMessage(&gMain, LOG_ERROR, "Problem with mp3 decoding.");
								pthread_exit((void *)1);
								return 0;
							}
						}

						/* MP3 */
					}
					if (sourceMedia == OGGVORBIS_FORMAT) {
						/* Vorbis */
						devorb_read (&vorbisDecoder, pStreamData, inbuffer);
						while (devorb_decode (&vorbisDecoder)) {
							if (vorbisDecoder.headers_changed) {
								char	StreamTitle[2046] = "";
								char	Artist[1024] = "";
								char	Title[1024] = "";

								char **ptr=vorbisDecoder.vc.user_comments;

								while(*ptr){
									char *pData = *ptr;
									if (!strncasecmp(pData, "ARTIST=", strlen("ARTIST="))) {
										strncpy(Artist, pData + strlen("ARTIST="), sizeof(Artist)-1);
									}
									if (!strncasecmp(pData, "TITLE=", strlen("TITLE="))) {
										strncpy(Title, pData + strlen("TITLE="), sizeof(Title)-1);
									}
									++ptr;
								}

								sprintf(StreamTitle, "%s - %s", Artist, Title);

								inputMetadataCallback(&gMain, StreamTitle);

								for(int i = 0; i < gMain.gNumEncoders; i++) {
									setCurrentSongTitle(g[i], StreamTitle);
								}
								vorbisDecoder.headers_changed = 0;
							}
						}
					}
					pthread_mutex_unlock(&encoders_mutex);
				}
				if (pStreamData) {
					free(pStreamData);
				}
			}
			else {
	#ifdef WIN32
				Sleep(100);
	#else
				usleep(250);
	#endif
			}
		}
	}
	LogMessage(&gMain, LOG_DEBUG, "Breaking out of decoder loop.");
	
	pthread_exit((void *)1);
	return 0;
}
