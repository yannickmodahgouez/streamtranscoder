// Mp3Decoder.h: interface for the CMp3Decoder class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MP3DECODER_H__F97A547C_9B74_439D_A931_D4044B99A6BE__INCLUDED_)
#define AFX_MP3DECODER_H__F97A547C_9B74_439D_A931_D4044B99A6BE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/*
#include <strstream>
*/
#include "mad.h"

#define ERR_OK	0
#define MP3BUFSIZE (0x100000)	// 1M how much space we can hold
#define ERR_MP3DECODER_BUFFER_FULL -1
#define MIN_MP3_STREAM_SIZE	(0x1) // 64k - how much we should process at a time

typedef struct tagMP3_DECODE_ST
{
	char m_pmp3Buf[MP3BUFSIZE];
	long m_mp3BufPos;
	struct mad_stream stream;
	struct mad_frame frame;
	struct mad_synth synth;
	int	sampleRate;
	long	bitRate;
	int	nch;
} MP3_DECODE_ST;

// Prototypes
#ifdef __cplusplus
extern "C" {
#endif
void mp3dec_init(MP3_DECODE_ST* pdecode);
long mp3dec_feed_stream(MP3_DECODE_ST* pdecode, char *pdata, long size);
int mp3dec_decode(MP3_DECODE_ST* pdecode);
long mp3dec_get_stream_size(MP3_DECODE_ST* pdecode);
#ifdef __cplusplus
}
#endif


#endif // !defined(AFX_MP3DECODER_H__F97A547C_9B74_439D_A931_D4044B99A6BE__INCLUDED_)
