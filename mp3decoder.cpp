// Mp3Decoder.cpp: implementation of the CMp3Decoder class.
//
//////////////////////////////////////////////////////////////////////

#include "mp3decoder.h"
#ifdef __cplusplus
extern "C" {
#endif
#include <mad.h>	//mpeg library
#ifdef __cplusplus
}
#endif
#ifdef _WIN32
#include <process.h>
#else
#include <stdlib.h>
#endif
#include <math.h>
#include <stdlib.h>

#include "liboddcast/liboddcast.h"

#define READSIZE	2000	// JCBUG i forget if we need a READSIZE, and if so, why this size

extern FILE	*outToEncoder;
extern oddcastGlobals          *g[];
extern oddcastGlobals          gMain;


///////////////////////////////////////////////////
// Private methods

void mp3dec_init(MP3_DECODE_ST* pdecode)
{
	if (!pdecode) {
		return;
	}
	pdecode->m_mp3BufPos = 0;
	mad_stream_init(&pdecode->stream);
	mad_frame_init(&pdecode->frame);
	mad_synth_init(&pdecode->synth);

}
long mp3dec_feed_stream(MP3_DECODE_ST* pdecode, char *pdata, long size)
{
	if (!pdecode) {
		return 0;
	}
	if (pdecode->m_mp3BufPos+size > MP3BUFSIZE)
	{
		return ERR_MP3DECODER_BUFFER_FULL;
	}

	memcpy(&pdecode->m_pmp3Buf[pdecode->m_mp3BufPos], pdata, size);
	pdecode->m_mp3BufPos += size;
	return ERR_OK;
}

long mp3dec_get_stream_size(MP3_DECODE_ST* pdecode)
{
	return pdecode->m_mp3BufPos;
}

static signed int scale(mad_fixed_t sample)
{
	// round 
/*
	sample += (1L << (MAD_F_FRACBITS - 16));
*/

	// clip 
	if (sample >= MAD_F_ONE)
		sample = MAD_F_ONE - 1;
	else if (sample < -MAD_F_ONE)
		sample = -MAD_F_ONE;

	// quantize 
	return sample >> (MAD_F_FRACBITS + 1 - 16);
}



int handle_mp3_output(MP3_DECODE_ST *pdecode)
{
	unsigned int nchannels, nsamples;
	float	*samples;
	struct mad_pcm *pcm;
	int	srate;
	int i = 0;

	mad_fixed_t const *left_ch, *right_ch;
	static short lastSample = 0;
	static signed int sample;

	pcm = &pdecode->synth.pcm;

	nchannels = pcm->channels;
	nsamples  = pcm->length;
	left_ch   = pcm->samples[0];
	right_ch  = pcm->samples[1];
	srate	  = pcm->samplerate;

	long	in_samplerate = pdecode->sampleRate;
	long	in_nch = pdecode->nch;

	samples = (float *)malloc(nsamples*nchannels*sizeof(float));

	// Send it to liboddcast (cross fingers)
	//fprintf(stdout, "Read %d/%d/%d - Sending %d/%d/%d", pdecode->bitRate/1000, pdecode->sampleRate, pdecode->nch, getCurrentBitrate(), getCurrentSamplerate(), getCurrentChannels());
	int	samplecount = 0;
	for (i=0;i<(int)nsamples;i++) {
		int sample;

		sample = *left_ch++;
		samples[samplecount] = (float)mad_f_todouble(sample);
		samplecount++;

		if (nchannels == 2) 
		{
			sample = *right_ch++;
			samples[samplecount] = (float)mad_f_todouble(sample);
			samplecount++;
		}
/*
		else {
			samples[samplecount] = 0.0;
			samplecount++;
		}
*/
	}

	int ret = 1;
    for(i = 0; i < gMain.gNumEncoders; i++) {
		//fprintf(stdout, "%d samples, %d channels, %d rate\n", nsamples, nchannels, srate);
        handle_output(g[i], samples, nsamples, nchannels, srate);
    }
	if (samples) {
		free(samples);
	}
	return ret;

}


int mp3dec_decode(MP3_DECODE_ST* pdecode)
{
	struct mad_stream* pstream = &pdecode->stream;
	struct mad_frame* pframe = &pdecode->frame;
	struct mad_synth* psynth = &pdecode->synth;
	int ret = 1;

	mad_stream_buffer(pstream, (const unsigned char*)pdecode->m_pmp3Buf, pdecode->m_mp3BufPos);
	
	while(1)
	{
		long ret = mad_frame_decode(pframe, pstream);
		if (pstream->error != MAD_ERROR_NONE) {
			if (pstream->error == MAD_ERROR_BUFLEN ||
				!MAD_RECOVERABLE(pstream->error))
				break;
		}
		
		if (pstream->error == MAD_ERROR_NONE) {
			mad_synth_frame(psynth, pframe);
			pdecode->sampleRate = pframe->header.samplerate;
			pdecode->bitRate = pframe->header.bitrate;
			switch (pframe->header.mode) {
				case MAD_MODE_SINGLE_CHANNEL:
					pdecode->nch = 1;
					break;
				case MAD_MODE_DUAL_CHANNEL:
				case MAD_MODE_JOINT_STEREO:
				case MAD_MODE_STEREO:
					pdecode->nch = 2;
					break;
			}
			ret = handle_mp3_output(pdecode);
		}
	}

	pstream->error = MAD_ERROR_NONE;
	
	if (!pstream->next_frame) {
		;
	}

	long bufferleft = (long)&(pdecode->m_pmp3Buf[pdecode->m_mp3BufPos]) - 
					  (long)pstream->next_frame;
	if (bufferleft > 0) {
		;
	}

	// move the mp3buf to the begining
	memmove(pdecode->m_pmp3Buf, 
			&pdecode->m_pmp3Buf[pdecode->m_mp3BufPos-bufferleft], 
			bufferleft);
	pdecode->m_mp3BufPos = bufferleft;

	return ret;
}

