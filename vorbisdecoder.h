#include <stdio.h> // for fread
#include <stdlib.h>
#include <string.h>
#include <ogg/ogg.h>
#include <vorbis/codec.h>

// handle to use in app
typedef struct 
{
    int process_headers;
    int failed;


    int vorbis_ready;
    float **pcm;
    int samples;
    int headers_changed;
    ogg_sync_state oy;
    ogg_stream_state os;
    vorbis_info vi;
    vorbis_comment vc;
    vorbis_dsp_state vd;
    vorbis_block vb;


} devorb;



int devorb_init (devorb *devorb);
void devorb_free (devorb *devorb);
int devorb_read (devorb *devorb, const char *buf, size_t len);
int devorb_decode (devorb *devorb);
