#include "vorbisdecoder.h"
#include "liboddcast/liboddcast.h"

extern oddcastGlobals          *g[];
extern oddcastGlobals          gMain;


int handle_vorbis_output(devorb *devorb);

int devorb_init (devorb *devorb)
{
    memset (devorb, 0, sizeof (*devorb));
    ogg_sync_init (&devorb->oy);
    ogg_stream_init (&devorb->os, 0);
    return 1;
}


void devorb_free (devorb *devorb)
{
    ogg_stream_clear (&devorb->os);


    if (devorb->vorbis_ready)
    {
        vorbis_block_clear (&devorb->vb);
        vorbis_dsp_clear (&devorb->vd);
        vorbis_comment_clear (&devorb->vc);
        vorbis_info_clear (&devorb->vi);
        devorb->vorbis_ready = 0;
    }
    ogg_sync_clear (&devorb->oy);
}



// insert ogg data for decoding
int devorb_read (devorb *devorb, const char *buf, size_t len)
{
    int ret = 0;
    char *oggbuf = ogg_sync_buffer (&devorb->oy, len);


    if (oggbuf)
    {
        memcpy (oggbuf, buf, len);
        ogg_sync_wrote (&devorb->oy, len);
        ret = 1;
    }
    return ret;
}



static int add_page (devorb *devorb)
{
    ogg_page page;


    if (ogg_sync_pageout (&devorb->oy, &page) > 0)
    {
        if (ogg_page_bos (&page))
        {
            ogg_stream_clear (&devorb->os);


            if (devorb->vorbis_ready)
            {
                vorbis_block_clear (&devorb->vb);
                vorbis_dsp_clear (&devorb->vd);
                vorbis_comment_clear (&devorb->vc);
                vorbis_info_clear (&devorb->vi);
                devorb->vorbis_ready = 0;
            }


            ogg_stream_init (&devorb->os, ogg_page_serialno (&page));
            vorbis_info_init (&devorb->vi);
            vorbis_comment_init (&devorb->vc);


            devorb->process_headers = 3;
            devorb->failed = 0;
        }
        ogg_stream_pagein (&devorb->os, &page);
        return 1;
    }
    return 0;
}



static int process_block (devorb *devorb)
{
    int samples;


    if (devorb->vorbis_ready == 0)
        return 0;


    samples = vorbis_synthesis_pcmout (&devorb->vd, &devorb->pcm);


    if (samples > 0)
    {
        devorb->samples = samples;
        return 1;
    }
    return 0;
}



static int process_packets (devorb *devorb)
{
    ogg_packet op;


    while (1)
    {
        int ret;


        if (process_block (devorb))
            return 1;


        ret = ogg_stream_packetout (&devorb->os, &op);


        if (ret < 1)
            return 0;
        if (devorb->process_headers)
        {
            if (vorbis_synthesis_headerin (&devorb->vi, &devorb->vc, &op) < 0)
            {
                printf ("header failed\n");
                devorb->failed = 1;
                return -1;
            }
            devorb->process_headers--;
            if (devorb->process_headers == 0)
            {
                vorbis_synthesis_init (&devorb->vd, &devorb->vi);
                vorbis_block_init (&devorb->vd, &devorb->vb);
                devorb->vorbis_ready = 1;
                devorb->headers_changed = 1;
            }
            continue;
        }


        if (vorbis_synthesis (&devorb->vb, &op) == 0)
            vorbis_synthesis_blockin (&devorb->vd, &devorb->vb);
    }
}



// process packets, produces float arrays in pcm -1.0 to +1.0
int devorb_decode (devorb *devorb)
{
    if (devorb->samples)
    {
        vorbis_synthesis_read (&devorb->vd, devorb->samples);
        devorb->samples = 0;
        devorb->pcm = NULL;
    }
    while (1)
    {
        if (process_packets (devorb)) {
			handle_vorbis_output(devorb);
            return 1;
		}
        if (add_page (devorb))
            continue;
        return 0;
    }
}

int handle_vorbis_output(devorb *devorb)
{
	unsigned int nchannels, nsamples;
	float	*samples;
	float	*left_ch;
	float	*right_ch;
	int i = 0;

	nchannels = devorb->vi.channels;
	nsamples  = devorb->samples;
	left_ch   = devorb->pcm[0];
	right_ch  = devorb->pcm[1];
	int srate 	  = devorb->vi.rate;

	samples = (float *)malloc(nsamples*nchannels*sizeof(float));

	int	samplecount = 0;
	for (i=0;i<(int)nsamples;i++) {

		samples[samplecount] = (float)(devorb->pcm[0][i]);
		samplecount++;

		if (nchannels == 2) {
			samples[samplecount] = (float)(devorb->pcm[1][i]);
			samplecount++;
		}
	}

	int ret = 1;
    for(i = 0; i < gMain.gNumEncoders; i++) {
        handle_output(g[i], samples, nsamples, nchannels, srate);
    }
	if (samples) {
		free(samples);
	}
	return ret;

}


// simple usage
//
//
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
